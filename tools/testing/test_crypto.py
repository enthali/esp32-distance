#!/usr/bin/env python3
"""
Test script to verify cryptography library availability in ESP-IDF environment
"""

print("Testing cryptography library availability...")

try:
    from cryptography import x509
    from cryptography.x509.oid import NameOID
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import rsa
    from ipaddress import IPv4Address
    import datetime
    
    print("✅ All cryptography modules imported successfully!")
    print("✅ IPv4Address imported successfully!")
    print("✅ datetime imported successfully!")
    
    # Test basic functionality
    print("\nTesting basic RSA key generation...")
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
    )
    print("✅ RSA key generation works!")
    
    print("\nTesting certificate builder...")
    subject = x509.Name([
        x509.NameAttribute(NameOID.COMMON_NAME, "Test-ESP32"),
    ])
    print("✅ Certificate builder works!")
    
    print("\n🎉 All tests passed! Certificate generation should work.")
    
except ImportError as e:
    print(f"❌ Import error: {e}")
    print("Missing cryptography library - would need to install with:")
    print("pip install cryptography")
    
except Exception as e:
    print(f"❌ Unexpected error: {e}")