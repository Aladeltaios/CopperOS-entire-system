# libc Placeholder

This project tree reserves `libc/` for a future freestanding C runtime once a full cross-linker toolchain is introduced.

The current bootable prototype is implemented in NASM because the local environment already provides `nasm`, `make`, and `qemu-system-x86_64`, which is enough to ship a real bootable OS image without downloading extra tools.
