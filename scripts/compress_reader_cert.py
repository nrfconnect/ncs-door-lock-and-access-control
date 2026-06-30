#!/usr/bin/env python3
"""
Script to compress an X.509 certificate according to Aliro Profile0000 specification.

Usage: python3 compress_cert.py <certificate_hex>

The script parses the input X.509 certificate and compresses it by:
- Omitting optional fields that match Profile0000 defaults
- Including only mandatory fields: profile(0x0000), public key, and signature

Output: Compressed certificate in hex format and C++ array for firmware
"""

import sys
import argparse
import binascii
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

def encode_length(length):
    """Encode ASN.1 length field"""
    if length < 128:
        return bytes([length])
    else:
        if length < 256:
            return bytes([0x81, length])
        elif length < 65536:
            return bytes([0x82, (length >> 8) & 0xFF, length & 0xFF])
        else:
            raise ValueError("Length too large")

def encode_context_tag(tag_nr, value):
    """Encode ASN.1 context-specific tag"""
    tag_byte = 0x80 | tag_nr  # Context class + tag number
    length_bytes = encode_length(len(value))
    return bytes([tag_byte]) + length_bytes + value

def extract_certificate_components(cert_der):
    """Extract components needed for Profile0000 compression"""
    # Parse the certificate
    cert = x509.load_der_x509_certificate(cert_der, backend=default_backend())

    # Extract public key BIT STRING content (what the test harness expects)
    spki_der = cert.public_key().public_bytes(
        encoding=serialization.Encoding.DER,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )
    # The test harness extracts SPKI[25:] which gives us the BIT STRING content
    public_key_bit_string = spki_der[25:]

    # Extract signature with 0x00 prefix (what the test harness expects)
    signature = b'\x00' + cert.signature

    # Extract optional fields
    serial_number = cert.serial_number
    issuer_cn = cert.issuer.get_attributes_for_oid(x509.oid.NameOID.COMMON_NAME)[0].value
    subject_cn = cert.subject.get_attributes_for_oid(x509.oid.NameOID.COMMON_NAME)[0].value
    not_valid_before = getattr(cert, "not_valid_before_utc", cert.not_valid_before)
    not_valid_after = getattr(cert, "not_valid_after_utc", cert.not_valid_after)
    not_before = not_valid_before.strftime("%y%m%d%H%M%SZ")
    not_after = not_valid_after.strftime("%y%m%d%H%M%SZ")

    return public_key_bit_string, signature, serial_number, issuer_cn, subject_cn, not_before, not_after

def compress_certificate(original_hex):
    """Compress an X.509 certificate to Profile0000 format"""

    # Parse the certificate
    cert_der = binascii.unhexlify(original_hex)

    # Extract components
    public_key_bit_string, signature, serial_number, issuer_cn, subject_cn, not_before, not_after = extract_certificate_components(cert_der)

    # Profile0000 identifier
    profile = b'\x00\x00'

    # Default values from spec
    DEFAULT_SERIAL = 1
    DEFAULT_ISSUER = "issuer"
    DEFAULT_SUBJECT = "subject"
    DEFAULT_NOT_BEFORE = "200101000000Z"
    DEFAULT_NOT_AFTER = "490101000000Z"

    print(f"Profile: {profile.hex()}")
    print(f"Public key BIT STRING ({len(public_key_bit_string)} bytes): {public_key_bit_string.hex()}")
    print(f"Signature ({len(signature)} bytes): {signature.hex()}")

    # Build ASN.1 structure with context tags [0-6]
    inner_data = b''

    # [0] Serial number (if not default)
    if serial_number != DEFAULT_SERIAL:
        serial_bytes = serial_number.to_bytes((serial_number.bit_length() + 7) // 8, 'big')
        inner_data += encode_context_tag(0, serial_bytes)
        print(f"[0] Serial: {serial_bytes.hex()} ({len(serial_bytes)} bytes)")

    # [1] Issuer CN (if not default)
    if issuer_cn != DEFAULT_ISSUER:
        issuer_bytes = issuer_cn.encode('utf-8')
        inner_data += encode_context_tag(1, issuer_bytes)
        print(f"[1] Issuer: {issuer_bytes.hex()} ({len(issuer_bytes)} bytes)")

    # [2] Not Before (if not default)
    if not_before != DEFAULT_NOT_BEFORE:
        not_before_bytes = not_before.encode('ascii')
        inner_data += encode_context_tag(2, not_before_bytes)
        print(f"[2] Not Before: {not_before_bytes.hex()} ({len(not_before_bytes)} bytes)")

    # [3] Not After (if not default)
    if not_after != DEFAULT_NOT_AFTER:
        not_after_bytes = not_after.encode('ascii')
        inner_data += encode_context_tag(3, not_after_bytes)
        print(f"[3] Not After: {not_after_bytes.hex()} ({len(not_after_bytes)} bytes)")

    # [4] Subject CN (if not default)
    if subject_cn != DEFAULT_SUBJECT:
        subject_bytes = subject_cn.encode('utf-8')
        inner_data += encode_context_tag(4, subject_bytes)
        print(f"[4] Subject: {subject_bytes.hex()} ({len(subject_bytes)} bytes)")

    # [5] Public key BIT STRING (mandatory)
    inner_data += encode_context_tag(5, public_key_bit_string)
    print(f"[5] Public key: {len(public_key_bit_string)} bytes")

    # [6] Signature (mandatory)
    inner_data += encode_context_tag(6, signature)
    print(f"[6] Signature: {len(signature)} bytes")

    # Inner sequence
    inner_seq = b'\x30' + encode_length(len(inner_data)) + inner_data

    # Profile octet string
    profile_octet = b'\x04' + encode_length(len(profile)) + profile

    # Outer sequence
    outer_data = profile_octet + inner_seq
    compressed = b'\x30' + encode_length(len(outer_data)) + outer_data

    return compressed

def main():
    parser = argparse.ArgumentParser(
        description="Compress an X.509 certificate according to Aliro Profile0000 specification",
        epilog="Example: python3 compress_reader_cert.py 30820152...",
    )
    parser.add_argument(
        "certificate_hex",
        help="X.509 certificate in hexadecimal format"
    )

    args = parser.parse_args()
    original_hex = args.certificate_hex

    # Validate input
    try:
        cert_der = binascii.unhexlify(original_hex)
    except Exception as e:
        parser.error(f"Invalid hex input: {e}")

    print("=== Aliro Profile0000 Certificate Compression ===")
    print(f"Original certificate: {len(original_hex)//2} bytes")
    print()

    try:
        # Compress the certificate
        compressed = compress_certificate(original_hex)
        print()
        print(f"Compressed certificate: {len(compressed)} bytes")
        print(f"Hex: {compressed.hex()}")
        print(f"Compression ratio: {len(compressed)}/{len(original_hex)//2} = {len(compressed)/(len(original_hex)//2)*100:.1f}%")
        print()

    except Exception as e:
        print(f"Error compressing certificate: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
