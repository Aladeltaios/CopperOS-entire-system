# CopperOS Architecture

## Boot flow

1. BIOS loads the boot sector at `0x7C00`.
2. The boot sector reads the kernel from disk in BIOS extended-read chunks, tries to switch into a VESA linear framebuffer mode, enables A20, installs a small GDT, and jumps to protected mode.
3. A tiny assembly entry stub sets up a stack and calls the freestanding `C` kernel.
4. The kernel initializes serial logging, installs a 6x6x6 palette for the `8-bit` framebuffer, shows staged boot artwork, and then renders the CopperOS desktop.

## Runtime model

- Single address space
- Polling keyboard input
- Keyboard-driven pointer state
- Immediate-mode UI redraws
- Built-in apps rendered as desktop windows by the kernel

## Asset pipeline

The source PNG files stay in the project root. During `make`, they are resized to the target UI dimensions under `build/assets/`, converted to indexed raw sprite buffers, and bundled directly into the kernel with NASM `incbin`.
