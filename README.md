# Wii-Linux Switch L4T Companion Distro
This is the source for the Alpine-based companion distro that runs on a Nintendo Switch,
to connect to your Nintendo Wii running [Wii-Linux](https://wii-linux.org), and
provide additional functionality.  **This is experimental software, and is not error-free.**

**Current version: v0.9**

# System requirements
- Common
  - Standard coreutils
  - wget
  - gzip
  - GNU tar (`tar`)
- Building the rootfs
  - BSD tar (`bsdtar`)
  - mkfs.ext4
  - zerofree
  - 7z
  - u-boot tools
    - mkimage
  - Either be on a native AArch64 system, or have QEMU binfmt set up
- Compiling the userspace utilities
  - Git
  - GNU Make
  - One of the following
    - AArch64 host with native toolchain (gcc, binutils, glibc, Linux API headers)
    - x86_64 host, and will automatically download the Linaro toolchain

# Configuration of the userspace app
Several debugging options are available:
- `DEBUG_FEATURES` - Enables in-app debugging features
- `INPUT_DEBUG`    - Enables debugging of the input subsystem

These can be enabled by setting the CFLAGS to define these, like so:  
`make DEBUG_FEATURES=1 INPUT_DEBUG=1`

## History
This project was started in 2024 by Techflash.  It initially started as the scripts that you see in the `userspace-utils/src/usb` directory.  
They don't resemble their original form anymore, but they are still loosely derived from the original scripts.  
The rest of the project (Alpine port, rootfs build script, all C code) was added over time.  

## TODO
- [ ] USB connection handling
  - [x] Basic infrastructure
  - [ ] Device type support
    - [ ] Serial communication protocol
      - [x] ACM device created
      - [x] Basic protocol established
      - [ ] Background communication support
      - [ ] Protocol specification
      - [x] Shared code betweeen client app and host app
      - [ ] Abstractable communication library
      - [x] Client app works properly on big endian clients
    - [x] RAMDisk / USB Mass Storage
    - [ ] USB Networking
      - [x] ECM device created
      - [x] MAC addresses assigned
      - [ ] Network configuration and bridging
- [ ] TUI to run on the Switch
  - [x] Menu interface
  - [ ] Input handling
    - [x] Keyboard input
    - [ ] Touch input
      - [x] Initial support
      - [x] Nag screen that touch is not supported
      - [x] Wake screen on tap
      - [ ] Support for touch input in TUI
    - [ ] Joycon input
  - [x] Features page
    - [x] RAMDisk Toggle
    - [x] Networking Toggle
  - [x] Device Info page
    - [x] Initiates handshake
    - [x] Reports host device info
    - [x] Reports target device info
  - [ ] Settings page
    - [ ] System Update Menu
      - [ ] Allows updating from the network
      - [ ] Allows updating from the target device
      - [ ] Allows updating via local storage
    - [x] Display configuration menu
  - [x] Debugging features menu
  - [x] Power controls (reboot, shutdown)
  - [x] Manages configuration file to persistently store settings
- [x] Configuration infrastructure
  - [x] Configuration format
  - [x] Helpers for managing it
- [x] Rootfs image building

## Legal
Most code is licensed under the GNU GPL v2, unless otherwise stated.  
Unless otherwise stated, all code is Copyright (C) 2024-2025 Michael "Techflash" Garofalo and contributors.  

Disk image generation code is Copyright (C) 2024 theofficialgman.  
The disk image generation code was released without a license.  It is assumed to be usable under a GPLv2 project such as this.  

The images under the `switchroot/wii-linux-switch-companion` directory are used under permission of [Tech64](https://github.com/tech64dd).  
They are licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC-BY-NC-SA) License.  
Images are Copyright (C) 2025 Tech64.
