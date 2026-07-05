#!/usr/bin/env python3
"""Convert a binary file to a C header with a uint8_t array."""
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: bin2h.py <binary_file>", file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1], 'rb') as f:
        data = f.read()

    print("#pragma once")
    print("#include <stdint.h>")
    print()
    print("static const uint8_t stage2_payload[] = {")
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        hex_bytes = ", ".join(f"0x{b:02x}" for b in chunk)
        print(f"    {hex_bytes},")
    print("};")
    print(f"static const unsigned int stage2_payload_size = {len(data)}u;")

if __name__ == "__main__":
    main()