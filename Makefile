BUILD_DIR := build

# Prefer system QEMU on Linux/Ubuntu; keep Apple Silicon path as fallback.
UNAME_S := $(shell uname -s 2>/dev/null || echo unknown)
ifeq ($(UNAME_S),Darwin)
  QEMU ?= /opt/homebrew/bin/qemu-system-x86_64
else
  QEMU ?= $(shell command -v qemu-system-x86_64 2>/dev/null || echo /usr/bin/qemu-system-x86_64)
endif

# Toolchain (override with make CC=... LD=... NASM=... if needed)
CC ?= x86_64-linux-gnu-gcc
LD ?= x86_64-linux-gnu-ld
NASM ?= nasm

CFLAGS := -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -mno-mmx -mno-sse -mno-sse2 -msoft-float -nostdlib -nostdinc -Wall -Wextra -O2 -Ikernel/include
LDFLAGS := -m elf_i386 -T kernel/linker.ld --oformat binary

.PHONY: all clean run run-headless assets

all: $(BUILD_DIR)/copperos.img

assets: $(BUILD_DIR)/assets.stamp

$(BUILD_DIR)/assets.stamp: scripts/build_assets.sh filemanager.png internetexplorer.png activitymanager.png paint.png systemsettings.png systemupdates.png calculator.png txteditor.png bootupscreen.png bootupscreenLog.png cursor.png cursorselected.png
	sh scripts/build_assets.sh
	touch $(BUILD_DIR)/assets.stamp

$(BUILD_DIR)/kernel_entry.o: kernel/arch/kernel_entry.asm
	$(NASM) -f elf32 -o $(BUILD_DIR)/kernel_entry.o kernel/arch/kernel_entry.asm

$(BUILD_DIR)/assets.o: kernel/assets/assets.asm | $(BUILD_DIR)/assets.stamp
	$(NASM) -f elf32 -o $(BUILD_DIR)/assets.o kernel/assets/assets.asm

$(BUILD_DIR)/kernel.o: kernel/kernel.c kernel/include/bootinfo.h kernel/include/types.h
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/kernel.o kernel/kernel.c

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/assets.o $(BUILD_DIR)/kernel.o kernel/linker.ld
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/assets.o

$(BUILD_DIR)/kernel_sectors.inc: $(BUILD_DIR)/kernel.bin
	# Linux/macOS compatible byte size
	# Linux: stat -c%s FILE, macOS: stat -f%z FILE
	printf 'KERNEL_SECTORS equ %d\n' $$((($$(stat -c%s $(BUILD_DIR)/kernel.bin 2>/dev/null || stat -f%z $(BUILD_DIR)/kernel.bin) + 511) / 512)) > $(BUILD_DIR)/kernel_sectors.inc

$(BUILD_DIR)/boot.bin: boot/bootloader/boot.asm $(BUILD_DIR)/kernel_sectors.inc
	$(NASM) -f bin -I . -o $(BUILD_DIR)/boot.bin boot/bootloader/boot.asm

$(BUILD_DIR)/copperos.img: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$(BUILD_DIR)/copperos.img bs=1474560 count=1
	dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/copperos.img conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/copperos.img bs=512 seek=1 conv=notrunc

run: $(BUILD_DIR)/copperos.img
	@echo "Select QEMU resolution (applied via VBE/graphics mode inside CopperOS, if supported)."
	@echo "Examples: 1280x720, 1920x1080, 1980x1080"
	@read -p "Resolution (WIDTHxHEIGHT) [1280x720]: " RES; \
		RES=$${RES:-1280x720}; \
		WIDTH=$${RES%x*}; HEIGHT=$${RES#*x}; \
		if [ -z "$$WIDTH" ] || [ -z "$$HEIGHT" ]; then echo "Bad format. Use WIDTHxHEIGHT"; exit 1; fi; \
		$(QEMU) -drive format=raw,file=$(BUILD_DIR)/copperos.img,media=disk \
		  -nic user,model=virtio-net-pci,mac=52:54:00:12:34:56 \
		  -rtc base=localtime \
		  -m 256M \
		  -display sdl,gl=off \
		  -device VGA \
		  -global isa-debugcon.iobase=0x402 \
		  -boot order=d

run-headless: $(BUILD_DIR)/copperos.img
	$(QEMU) -drive format=raw,file=$(BUILD_DIR)/copperos.img,media=disk -nic user,model=virtio-net-pci,mac=52:54:00:12:34:56 -display none -serial stdio -no-reboot -no-shutdown

$(BUILD_DIR)/copperos.iso: $(BUILD_DIR)/copperos.img
	rm -f $(BUILD_DIR)/copperos.iso
	hdiutil makehybrid -ov -o $(BUILD_DIR)/copperos.iso -iso -joliet -default-volume-name COPPEROS -eltorito-boot $(BUILD_DIR)/copperos.img -hard-disk-boot $(BUILD_DIR)

iso: $(BUILD_DIR)/copperos.iso

vbox: iso
	@echo "ISO artifact built at $(BUILD_DIR)/copperos.iso"

clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.img $(BUILD_DIR)/*.inc $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.map $(BUILD_DIR)/assets $(BUILD_DIR)/assets.stamp $(BUILD_DIR)/*.iso

