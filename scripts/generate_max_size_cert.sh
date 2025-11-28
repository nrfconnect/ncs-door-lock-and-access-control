#!/bin/bash

# Generate Aliro certificate with maximum field sizes for APDU chaining testing
# Based on Demo 4 from Aliro spec

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check for required arguments
if [[ "$#" -ne 2 ]]; then
    echo "Usage: $0 <reader_public_key> <issuer_private_key>" >&2
    echo "" >&2
    echo "Arguments:" >&2
    echo "  reader_public_key   - Reader's public key (65 bytes hex, uncompressed format 04||X||Y)" >&2
    echo "  issuer_private_key  - Issuer's private key for signing (32 bytes hex)" >&2
    echo "" >&2
    echo "Example:" >&2
    echo "  $0 04f27d92b78c0ba00c105dc56b9f5a669d67d44fea5139b85dc5e368c851167c9f40b8d9add7ce7bf846331f1f8fd06e1e15cfd540190f482ec294f52690349f18 379e5e0cda81f7a9e573fb165ab7eddde40ea032dfbb458af0a14dece531d12c" >&2
    echo "" >&2
    echo "To generate new keys, use:" >&2
    echo "  python3 $SCRIPT_DIR/generate_keypair.py" >&2
    exit 1
fi

READER_PUB="$1"
ISSUER_PRIV="$2"

# Validate key formats
if [[ ${#READER_PUB} -ne 130 ]]; then
    echo "Error: Reader public key must be 65 bytes (130 hex characters), got ${#READER_PUB}" >&2
    exit 1
fi

if [[ ${#ISSUER_PRIV} -ne 64 ]]; then
    echo "Error: Issuer private key must be 32 bytes (64 hex characters), got ${#ISSUER_PRIV}" >&2
    exit 1
fi

echo "=== Using provided keys ===" >&2
echo "Reader Public Key: $READER_PUB" >&2
echo "Issuer Private Key: $ISSUER_PRIV" >&2
echo "" >&2

# Maximum size fields (from spec Demo 4)
SERIAL="5555555555555555555555555555555555555555"  # 20 bytes (max)
ISSUER_CN="custom issuer name............"           # 32 bytes
SUBJECT_CN="custom subject name..........."          # 32 bytes
NOT_BEFORE="200102000000Z"                           # Custom date
NOT_AFTER="250505000000Z"                            # Custom date

echo "=== Generating certificate with maximum field sizes ===" >&2
echo "Serial Number: $SERIAL (20 bytes)" >&2
echo "Issuer CN: '$ISSUER_CN' (32 bytes)" >&2
echo "Subject CN: '$SUBJECT_CN' (32 bytes)" >&2
echo "Not Before: $NOT_BEFORE" >&2
echo "Not After: $NOT_AFTER" >&2
echo "" >&2

# Convert hex serial to decimal
SERIAL_DEC=$(python3 -c "print(int('$SERIAL', 16))")

# Generate X.509 certificate
CERT_HEX=$(python3 $SCRIPT_DIR/generate_reader_cert.py \
  --subject-pubkey "$READER_PUB" \
  --issuer-privkey "$ISSUER_PRIV" \
  --serial "$SERIAL_DEC" \
  --issuer-cn "$ISSUER_CN" \
  --subject-cn "$SUBJECT_CN" \
  --not-before "$NOT_BEFORE" \
  --not-after "$NOT_AFTER" \
  --output hex)

CERT_SIZE=$((${#CERT_HEX}/2))
echo "X.509 Certificate: $CERT_SIZE bytes" >&2
echo "$CERT_HEX" >&2
echo "" >&2

# Compress certificate
echo "=== Compressing certificate ===" >&2
COMPRESSED_OUTPUT=$(python3 $SCRIPT_DIR/compress_reader_cert.py "$CERT_HEX")
COMPRESSED_HEX=$(echo "$COMPRESSED_OUTPUT" | grep "^Hex:" | cut -d: -f2 | tr -d ' ')
COMPRESSED_SIZE=$((${#COMPRESSED_HEX}/2))

echo "$COMPRESSED_OUTPUT" >&2
echo "" >&2

# Check if chaining is required
if [[ $COMPRESSED_SIZE -gt 245 ]]; then
    CHUNKS=$(( ($COMPRESSED_SIZE + 244) / 245 ))
    echo "✓ Certificate requires APDU chaining: $COMPRESSED_SIZE bytes > 245 bytes" >&2
    echo "  Will be sent in $CHUNKS chunks" >&2
else
    echo "⚠ Certificate does NOT require chaining: $COMPRESSED_SIZE bytes ≤ 245 bytes" >&2
fi
echo "" >&2

# Output provisioning commands
echo "=== Provisioning Commands ===" >&2
if [[ -n "$ISSUER_PUB" ]]; then
    echo "dl provisioning issuer_pk set $ISSUER_PUB" >&2
fi
echo "dl provisioning reader_cert set $COMPRESSED_HEX" >&2
echo "" >&2

# Output hex for easy copy-paste
echo "$COMPRESSED_HEX"

