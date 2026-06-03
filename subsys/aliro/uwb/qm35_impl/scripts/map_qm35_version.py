#!/usr/bin/env python3
"""
Script to map UWB QM35's version string to MCUboot compatible format by parsing
a firmware's binary file.

Usage: python3 map_qm35_version.py <binary file>

Output: String in a format of <major>.<minor>.<revision>+<build>
"""

import re
import sys

MAX_HEADER_SIZE = 1024

def main():
    if len(sys.argv) != 2:
        print(f"Usage: python3 {sys.argv[0]} <binary_file>", file=sys.stderr)
        print("  binary_file - QM35 FW binary file to be parsed", file=sys.stderr)
        sys.exit(1)

    binary_path = sys.argv[1]

    try:
        with open(binary_path, "rb") as f:
            data = f.read(MAX_HEADER_SIZE)
    except OSError as e:
        print(f"Error opening file: {e}", file=sys.stderr)
        sys.exit(1)

    # Search for ASCII string starting with QMV=
    match = re.search(rb"QMV=([^\x00\r\n]+)", data)
    if not match:
        print("QMV version string not found", file=sys.stderr)
        sys.exit(1)

    version = re.sub(r"_.*$", "", match.group(1).decode("ascii", errors="ignore")).replace("rc", "+")

    print(version)


if __name__ == "__main__":
    main()
