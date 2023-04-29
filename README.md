
CH32V3x DFU bootloader
========================

This is a "small" bootloader for the CH32V3x chips from WCH.
It takes about 13 kB and is being built on top of tinyUSB.

The downloaded application must be linked to be at 0x4000 (+16 kB)

There are several ways to enter DFU:

- There is a signature in ram upon reboot (0xDEADBEEFCC00FFEEULL at the beginning of RAM)
- The pin PB2 is pulled down to ground
- The signature of the main firmware is invalid


Firmware signature
------------------

The layout is as follows :

- 0000 : Reset instruction (usuall j reset vector)
- 0004 : firmware size in bytes
- 0008 : xxhash32 hash of the firmware

If firmware size is 0x1234 and xxhash is 0x5678, the hash check is skipped.

Building
--------
Edit platformConfig to point for your toolchain.

The one i use is a recent version of clang/riscv32 with picolibc.

Using
------

```
dfu-util -d 1d50:6030  -s 0x0004000:leave -D binaryfile
