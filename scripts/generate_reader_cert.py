#!/usr/bin/env python3
"""
Generate a Reader certificate for Aliro testing.

This script generates an X.509 certificate with configurable parameters according to
Aliro specification chapter 13.2 (Reader certificate requirements).

All certificate fields can be specified via command-line arguments.
Use compress_cert.py separately to compress the generated certificate.
"""

import sys
import argparse
from datetime import datetime, timezone
from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import default_backend
from cryptography.x509.oid import NameOID


def load_private_key(private_key_hex: str) -> ec.EllipticCurvePrivateKey:
    """Load EC private key from hex string"""
    private_key_bytes = bytes.fromhex(private_key_hex)
    return ec.derive_private_key(
        int.from_bytes(private_key_bytes, 'big'),
        ec.SECP256R1(),
        default_backend()
    )


def load_public_key(public_key_hex: str) -> ec.EllipticCurvePublicKey:
    """Load EC public key from hex string (uncompressed format 04||X||Y)"""
    public_key_bytes = bytes.fromhex(public_key_hex)
    return ec.EllipticCurvePublicKey.from_encoded_point(
        ec.SECP256R1(),
        public_key_bytes
    )


def generate_certificate(
    subject_public_key_hex: str,
    issuer_private_key_hex: str,
    serial_number: int = 1,
    issuer_cn: str = "issuer",
    subject_cn: str = "subject",
    not_before: str = "200101000000Z",
    not_after: str = "490101000000Z",
    key_usage_digital_signature: bool = True,
    basic_constraints_ca: bool = False
) -> bytes:
    """
    Generate an X.509 certificate according to Aliro spec 13.2.

    Args:
        subject_public_key_hex: Subject's public key (hex string, 65 bytes uncompressed)
        issuer_private_key_hex: Issuer's private key for signing (hex string, 32 bytes)
        serial_number: Certificate serial number (default: 1)
        issuer_cn: Issuer Common Name (default: "issuer")
        subject_cn: Subject Common Name (default: "subject")
        not_before: Validity start date in YYMMDDhhmmssZ format (default: "200101000000Z")
        not_after: Validity end date in YYMMDDhhmmssZ format (default: "490101000000Z")
        key_usage_digital_signature: Enable digital signature key usage (default: True)
        basic_constraints_ca: Set CA flag in Basic Constraints (default: False)

    Returns:
        DER-encoded certificate bytes
    """
    # Load keys
    subject_public_key = load_public_key(subject_public_key_hex)
    issuer_private_key = load_private_key(issuer_private_key_hex)
    issuer_public_key = issuer_private_key.public_key()

    # Parse dates
    not_before_dt = datetime.strptime(not_before, "%y%m%d%H%M%SZ").replace(tzinfo=timezone.utc)
    not_after_dt = datetime.strptime(not_after, "%y%m%d%H%M%SZ").replace(tzinfo=timezone.utc)

    # Build certificate
    builder = x509.CertificateBuilder()

    # Subject and Issuer
    builder = builder.subject_name(x509.Name([
        x509.NameAttribute(NameOID.COMMON_NAME, subject_cn),
    ]))
    builder = builder.issuer_name(x509.Name([
        x509.NameAttribute(NameOID.COMMON_NAME, issuer_cn),
    ]))

    # Serial number
    builder = builder.serial_number(serial_number)

    # Validity period
    builder = builder.not_valid_before(not_before_dt)
    builder = builder.not_valid_after(not_after_dt)

    # Subject public key
    builder = builder.public_key(subject_public_key)

    # Extensions
    # Authority Key Identifier (from issuer's public key)
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(issuer_public_key),
        critical=False,
    )

    # Basic Constraints
    builder = builder.add_extension(
        x509.BasicConstraints(ca=basic_constraints_ca, path_length=None),
        critical=True,
    )

    # Key Usage
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=key_usage_digital_signature,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=False,
            crl_sign=False,
            encipher_only=False,
            decipher_only=False,
        ),
        critical=True,
    )

    # Sign the certificate
    certificate = builder.sign(issuer_private_key, hashes.SHA256(), default_backend())

    # Return DER-encoded certificate
    return certificate.public_bytes(serialization.Encoding.DER)


def main():
    parser = argparse.ArgumentParser(
        description='Generate Aliro Reader certificate according to spec 13.2',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate with default test harness values
  python3 generate_reader_cert.py \\
    --subject-pubkey 04f27d92b78c0ba00c105dc56b9f5a669d67d44fea5139b85dc5e368c851167c9f40b8d9add7ce7bf846331f1f8fd06e1e15cfd540190f482ec294f52690349f18 \\
    --issuer-privkey 004111DBF98EC0FAE70D37218E1BE8B752D5016F2912A5A226F48EBB2D515058

  # Generate with custom fields
  python3 generate_reader_cert.py \\
    --subject-pubkey <65-byte-hex> \\
    --issuer-privkey <32-byte-hex> \\
    --serial 12345 \\
    --issuer-cn "MyIssuer" \\
    --subject-cn "MyReader" \\
    --not-before 250101000000Z \\
    --not-after 300101000000Z

  # Compress the generated certificate separately
  python3 generate_reader_cert.py ... > cert.hex
  python3 compress_cert.py $(cat cert.hex)
        """
    )

    # Required arguments
    parser.add_argument('--subject-pubkey', required=True,
                        help='Subject public key (65-byte hex, uncompressed format 04||X||Y)')
    parser.add_argument('--issuer-privkey', required=True,
                        help='Issuer private key for signing (32-byte hex)')

    # Optional certificate fields
    parser.add_argument('--serial', type=int, default=1,
                        help='Certificate serial number (default: 1)')
    parser.add_argument('--issuer-cn', default='issuer',
                        help='Issuer Common Name (default: "issuer")')
    parser.add_argument('--subject-cn', default='subject',
                        help='Subject Common Name (default: "subject")')
    parser.add_argument('--not-before', default='200101000000Z',
                        help='Validity start date YYMMDDhhmmssZ (default: "200101000000Z")')
    parser.add_argument('--not-after', default='490101000000Z',
                        help='Validity end date YYMMDDhhmmssZ (default: "490101000000Z")')

    # Optional extensions
    parser.add_argument('--key-usage-digital-signature', action='store_true', default=True,
                        help='Enable digital signature in Key Usage (default: True)')
    parser.add_argument('--no-key-usage-digital-signature', action='store_false',
                        dest='key_usage_digital_signature',
                        help='Disable digital signature in Key Usage')
    parser.add_argument('--basic-constraints-ca', action='store_true', default=False,
                        help='Set CA=TRUE in Basic Constraints (default: False)')

    # Output options
    parser.add_argument('--verify', action='store_true', default=True,
                        help='Verify certificate signature (default: True)')
    parser.add_argument('--no-verify', action='store_false', dest='verify',
                        help='Skip signature verification')
    parser.add_argument('--output', choices=['hex', 'details', 'both'], default='both',
                        help='Output format: hex only, details only, or both (default: both)')

    args = parser.parse_args()

    # Validate inputs
    try:
        subject_pubkey_bytes = bytes.fromhex(args.subject_pubkey)
        if len(subject_pubkey_bytes) != 65 or subject_pubkey_bytes[0] != 0x04:
            print(f"Error: Subject public key must be 65 bytes in uncompressed format (04||X||Y)", file=sys.stderr)
            sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid subject public key hex: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        issuer_privkey_bytes = bytes.fromhex(args.issuer_privkey)
        if len(issuer_privkey_bytes) != 32:
            print(f"Error: Issuer private key must be 32 bytes", file=sys.stderr)
            sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid issuer private key hex: {e}", file=sys.stderr)
        sys.exit(1)

    if args.output in ['details', 'both']:
        print("=== Generating Reader Certificate ===", file=sys.stderr)
        print(f"Subject Public Key: {args.subject_pubkey}", file=sys.stderr)
        print(f"Issuer Private Key: {args.issuer_privkey}", file=sys.stderr)
        print(f"Serial Number: {args.serial}", file=sys.stderr)
        print(f"Issuer CN: {args.issuer_cn}", file=sys.stderr)
        print(f"Subject CN: {args.subject_cn}", file=sys.stderr)
        print(f"Not Before: {args.not_before}", file=sys.stderr)
        print(f"Not After: {args.not_after}", file=sys.stderr)
        print(f"Key Usage (Digital Signature): {args.key_usage_digital_signature}", file=sys.stderr)
        print(f"Basic Constraints (CA): {args.basic_constraints_ca}", file=sys.stderr)
        print(file=sys.stderr)

    # Generate certificate
    try:
        cert_der = generate_certificate(
            subject_public_key_hex=args.subject_pubkey,
            issuer_private_key_hex=args.issuer_privkey,
            serial_number=args.serial,
            issuer_cn=args.issuer_cn,
            subject_cn=args.subject_cn,
            not_before=args.not_before,
            not_after=args.not_after,
            key_usage_digital_signature=args.key_usage_digital_signature,
            basic_constraints_ca=args.basic_constraints_ca
        )
    except Exception as e:
        print(f"Error generating certificate: {e}", file=sys.stderr)
        sys.exit(1)

    # Verify certificate if requested
    if args.verify:
        try:
            cert = x509.load_der_x509_certificate(cert_der, default_backend())
            cert_public_key = cert.public_key()
            cert_public_key_bytes = cert_public_key.public_bytes(
                encoding=serialization.Encoding.X962,
                format=serialization.PublicFormat.UncompressedPoint
            )

            if args.output in ['details', 'both']:
                print("Certificate Details:", file=sys.stderr)
                print(f"  Serial Number: {cert.serial_number}", file=sys.stderr)
                print(f"  Issuer: {cert.issuer.rfc4514_string()}", file=sys.stderr)
                print(f"  Subject: {cert.subject.rfc4514_string()}", file=sys.stderr)
                print(f"  Not Before: {cert.not_valid_before_utc}", file=sys.stderr)
                print(f"  Not After: {cert.not_valid_after_utc}", file=sys.stderr)
                print(f"  Public Key: {cert_public_key_bytes.hex()}", file=sys.stderr)
                print(f"  Signature: {cert.signature.hex()}", file=sys.stderr)
                print(file=sys.stderr)

            # Verify signature
            issuer_private_key = load_private_key(args.issuer_privkey)
            issuer_public_key = issuer_private_key.public_key()

            issuer_public_key.verify(
                cert.signature,
                cert.tbs_certificate_bytes,
                ec.ECDSA(hashes.SHA256())
            )

            if args.output in ['details', 'both']:
                print("✓ Signature verification: PASSED", file=sys.stderr)
                print(file=sys.stderr)
        except Exception as e:
            print(f"✗ Signature verification: FAILED - {e}", file=sys.stderr)
            print(file=sys.stderr)

    # Output certificate hex
    if args.output in ['hex', 'both']:
        if args.output == 'both':
            print("=== X.509 Certificate (DER) ===", file=sys.stderr)
            print(f"Size: {len(cert_der)} bytes", file=sys.stderr)
            print(file=sys.stderr)
        print(cert_der.hex())


if __name__ == "__main__":
    main()
