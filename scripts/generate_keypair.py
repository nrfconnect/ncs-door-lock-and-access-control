#!/usr/bin/env python3
"""
Generate EC key pair for Aliro certificate signing.

This script generates a secp256r1 (P-256) EC key pair suitable for:
- Credential Issuer keys (for signing Reader certificates)
- Reader keys (for ECDH key agreement during transactions)

The keys are output as hex strings.
"""

import sys
import argparse
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend


def generate_keypair() -> tuple[ec.EllipticCurvePrivateKey, ec.EllipticCurvePublicKey]:
    """Generate a new secp256r1 EC key pair"""
    private_key = ec.generate_private_key(ec.SECP256R1(), default_backend())
    public_key = private_key.public_key()
    return private_key, public_key


def main():
    parser = argparse.ArgumentParser(
        description='Generate EC key pair for Aliro certificate signing',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate random key pair (minimal output)
  python3 generate_keypair.py

  # Generate with detailed information
  python3 generate_keypair.py --verbose

  # Generate key pair from seed (deterministic)
  python3 generate_keypair.py --seed fdf71a3714e078c2c2fa907ae9acf624aa98add7edf7500e61cf8af4cc5a70a9

  # Use with certificate generation
  ISSUER_KEYS=$(python3 generate_keypair.py)
  ISSUER_PRIV=$(echo "$ISSUER_KEYS" | grep "Private" | cut -d: -f2 | tr -d ' ')
  ISSUER_PUB=$(echo "$ISSUER_KEYS" | grep "Public" | cut -d: -f2 | tr -d ' ')

  READER_PUB=<reader_public_key>

  # Generate certificate
  python3 generate_reader_cert.py --subject-pubkey $READER_PUB --issuer-privkey $ISSUER_PRIV
        """
    )

    parser.add_argument('--seed', type=str,
                        help='Optional: hex string seed for deterministic key generation (32 bytes)')
    parser.add_argument('--verify', action='store_true', default=False,
                        help='Verify the generated key pair')
    parser.add_argument('--verbose', action='store_true', default=False,
                        help='Verbose output with detailed information and usage examples')

    args = parser.parse_args()

    # Generate or derive key pair
    if args.seed:
        try:
            seed_bytes = bytes.fromhex(args.seed)
            if len(seed_bytes) != 32:
                print(f"Error: Seed must be 32 bytes (64 hex characters), got {len(seed_bytes)} bytes",
                      file=sys.stderr)
                sys.exit(1)

            # Derive private key from seed
            private_key = ec.derive_private_key(
                int.from_bytes(seed_bytes, 'big'),
                ec.SECP256R1(),
                default_backend()
            )
            public_key = private_key.public_key()
            if args.verbose:
                print("=== EC Key Pair (from seed) ===", file=sys.stderr)
        except ValueError as e:
            print(f"Error: Invalid seed hex: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        private_key, public_key = generate_keypair()
        if args.verbose:
            print("=== EC Key Pair (randomly generated) ===", file=sys.stderr)

    # Extract key bytes
    private_key_int = private_key.private_numbers().private_value
    private_key_bytes = private_key_int.to_bytes(32, 'big')

    public_key_bytes = public_key.public_bytes(
        encoding=serialization.Encoding.X962,
        format=serialization.PublicFormat.UncompressedPoint
    )

    if args.verbose:
        print(file=sys.stderr)

    # Output keys
    if args.verbose:
        print(f"Private Key ({len(private_key_bytes)} bytes):")
        print(private_key_bytes.hex())
        print()
        print(f"Public Key ({len(public_key_bytes)} bytes):")
        print(public_key_bytes.hex())
        print()
    else:
        print(f"Private Key: {private_key_bytes.hex()}")
        print(f"Public Key: {public_key_bytes.hex()}")

    # Verify key pair if requested
    if args.verify:
        if args.verbose:
            print("=== Verification ===", file=sys.stderr)
        try:
            # Sign a test message
            from cryptography.hazmat.primitives import hashes
            test_message = b"Aliro test message"
            signature = private_key.sign(test_message, ec.ECDSA(hashes.SHA256()))

            # Verify with public key
            public_key.verify(signature, test_message, ec.ECDSA(hashes.SHA256()))
            if args.verbose:
                print("✓ Key pair verification: PASSED", file=sys.stderr)
                print("  (Private key can sign, public key can verify)", file=sys.stderr)
            else:
                print("Verification: PASSED", file=sys.stderr)
        except Exception as e:
            print(f"✗ Key pair verification: FAILED - {e}", file=sys.stderr)
            sys.exit(1)

        if args.verbose:
            print(file=sys.stderr)

if __name__ == "__main__":
    main()
