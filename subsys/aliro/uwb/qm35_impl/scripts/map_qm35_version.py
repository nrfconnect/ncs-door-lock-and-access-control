#!/usr/bin/env python3
"""
Script to map UWB QM35's version string to MCUboot compatible format by parsing
a firmware's binary file.

Usage: python3 map_qm35_version.py <binary file>

Output: String in a format of <major>.<minor>.<revision>+<build>

The build identifier is taken from the numeric suffix after '_' in the QM35 version
string (e.g. "0.6.0rc1_13847203194" → build id 13847203194), truncated to uint32
to fit the MCUboot build_num field.  This makes each distinct QM35 binary produce a
unique build_num even when binaries share the same rc label, enabling correct
version comparisons in ShouldUpdate().
"""

import re
import sys

MAX_HEADER_SIZE = 1024
UINT32_MAX = 0xFFFFFFFF


def main() -> None:
    """Parse a QM35 firmware binary and print its MCUboot-compatible version string.

    Reads up to MAX_HEADER_SIZE bytes from the given binary, locates the embedded
    QMV= version tag, and converts it to the ``major.minor.revision+build_num``
    format expected by imgtool.

    Raises:
        SystemExit: On missing argument, file error, or missing version string.
    """
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

    match = re.search(rb"QMV=([^\x00\r\n]+)", data)
    if not match:
        print("QMV version string not found", file=sys.stderr)
        sys.exit(1)

    ver_str = match.group(1).decode("ascii", errors="ignore")

    # Expected format: "X.Y.Z[rc|-rcN]_BUILDID" where BUILDID is a large integer.
    # Use BUILDID (truncated to uint32) as MCUboot build_num so that distinct
    # binaries sharing the same rc label still produce different version numbers.
    m = re.match(r"(\d+)\.(\d+)\.(\d+)(?:-?rc\d+)?_(\d+)", ver_str)
    if m:
        major, minor, revision, build_id = m.groups()
        build_num = int(build_id) & UINT32_MAX
        print(f"{major}.{minor}.{revision}+{build_num}")
        return

    # Fallback for unexpected formats: strip the build suffix and map "rc" → "+".
    # WARNING: this produces a build_num derived from the rc label (e.g. rc1 → 1),
    # which will NOT match what ParseVersionString extracts from the device version
    # string at runtime.  Update the regex above to handle this format properly.
    print(f"WARNING: unrecognized QM35 version format '{ver_str}', "
          "falling back to rc-label as build_num", file=sys.stderr)
    version = re.sub(r"_.*$", "", ver_str).replace("rc", "+")
    print(version)


if __name__ == "__main__":
    main()
