
# Swindle CH32V3x DFU Bootloader

A USB DFU (Device Firmware Update) bootloader for **CH32V3x** RISC-V microcontrollers from WCH.

It lives in the first 16 KB of flash and lets you update the main firmware over USB without any special programmer.
(its actual size is ~ 6 kB).

## How to enter DFU mode

The bootloader jumps straight to your application unless one of these is true:

1. **Reboot marker** — your application can set a magic value in RAM before rebooting to request DFU
2. **DFU button** — hold PB7 low at startup (external button to ground)
3. **No valid application** — if the CRC32 check fails or no firmware is present, DFU starts automatically

When in DFU mode the LED blinks fast so you know it's ready.

## Flashing a new application

Once the bootloader is on the chip, use `dfu-util` from any computer:

```sh
dfu-util -d 1d50:6030 -s 0x0004000:leave -D your_firmware.bin
```

Your application must be linked to run at flash offset **0x4000** (16 KB).

## Firmware header

The bootloader expects a small 12-byte header at the start of your application image:

| Offset | Content                     |
|--------|-----------------------------|
| 0x0000 | Reset instruction (jump to entry point) |
| 0x0004 | Firmware size in bytes      |
| 0x0008 | CRC32 checksum of the code  |

The CRC uses the same algorithm as GDB's `crc32`. If the size is `0x1234` and the checksum is `0x5678`, the CRC check is skipped — useful during development.

## Building the bootloader

You need a RISC-V toolchain (Clang with picolibc works well) and CMake.

```sh
cmake .. && make 
```

Edit `platformConfig.cmake` first if your toolchain is in a non-standard location.

The build produces `stage1.elf` — that's the single binary you flash onto the chip (it includes the DFU handler automatically).

## Initial flashing (first time)

To put the bootloader on a blank chip, use any RVSWD compatible debugger (swindle, WCHlink+openOCD,...):

```sh
gdb-multiarch -ex "target extended-remote /dev/ttyBmpGdb" -ex "load stage1.elf" -ex "run"
```

After that, all future updates can go through USB DFU.
