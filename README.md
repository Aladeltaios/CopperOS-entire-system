# CopperOS-entire-system
# CopperOS v1.0 Copperhead

CopperOS is a small BIOS-booting hobby OS that now uses a freestanding `C` kernel with a tiny NASM boot path and runs under `qemu-system-x86_64` on Apple Silicon.

This v1.0 Copperhead release is intentionally compact:

- BIOS boot sector written in NASM
- Freestanding 32-bit `C` kernel linked with `x86_64-elf-gcc` and `x86_64-elf-ld`
- VESA framebuffer path that targets HD `1280x720x8`, then falls back through broadly supported modes
- Off-screen frame composition reduces full-screen blink during redraws and prepares the UI for smoother animation
- Staged boot screens using the supplied `bootupscreenLog.png` and `bootupscreen.png`
- Windows 95-inspired black desktop, taskbar, Start menu, windows, and pointer
- Eight built-in apps: `File Explorer`, `Internet Explorer`, `Activity Manager`, `Paint`, `System Settings`, `System Updates`, `Calculator`, and `TXT Editor`
- First-run setup collects username, passcode, install mode, disk target, desktop theme, and network
- A welcome animation plays after setup, then the user logs in with the created passcode
- PNG assets are resized and converted at build time into bundled raw assets

## Build

```sh
make
```

## Run

```sh
make run
```

For a quick serial verification without opening a GUI window:

```sh
make run-headless
```

## Controls

**Setup creates a 4-digit passcode. Use 0-9, Backspace, Enter.**

- `I` `J` `K` `L`: move the desktop pointer
- `Tab`: cycle the selected desktop icon or Paint color
- `Enter`: click the current pointer target or open the selected icon
- `S`: open/close the Start menu on the desktop
- `1` `2` `3` `4` `5` `6` `7` `8`: open apps directly (`8` = TXT Editor)
- `Esc`: close the current app
- `W` `A` `S` `D`: move the Paint brush
- `Space`: paint in Paint
- `C`: clear the Paint canvas
- In `Internet Explorer`: type a URL, then press `Enter` to check real network stack status
- In `System Settings`: `N` changes Wi-Fi, `B` changes theme, `M` changes Start menu style, `X` minimizes all windows
- In `TXT Editor`: type text, `Enter` creates a new line, and edits are auto-saved in memory
- **Setup passcode**: `0`-`9` input digits, `Backspace` delete, `Enter` save, `Esc` back step

## Notes

- The current shipped build runs in `32-bit` architecture.
- This version targets an 8-bit VESA framebuffer and is written for 32-bit x86 graphics modes.
- The apps are still kernel-rendered built-ins, not separate user-space programs yet.
- `Internet Explorer` now accepts typed URLs but no longer fakes page loads; real browsing still needs NIC, TCP/IP, DNS, TLS, and HTML support.
- `System Updates` shows the current release status and confirms the latest version.
- First boot launches a setup wizard for username, passcode, install or try mode, disk scan, theme, and network setup.
- The Start menu includes `Reboot to Bootloader`, which opens a kernel-rendered bootloader screen with CopperOS and USB installer choices.
- `System Settings` can choose Wi-Fi, desktop theme, Start menu style, and view diagnostics.
- Build a VM boot image with `make`; build the ISO artifact with `make vbox`.
- The pointer works with keyboard + mouse input and the redraw path now renders through an off-screen buffer to prevent corruption and blinking.
