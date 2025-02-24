#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  Copyright 2011 Sybren A. Stüvel <sybren@stuvel.eu>
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import unittest

from rsa._compat import is_bytes
from rsa.pem import _markers
import rsa.key

# 512-bit key. Too small for practical purposes, but good enough for testing with.
public_key_pem = '''
-----BEGIN PUBLIC KEY-----
MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKH0aYP9ZFuctlPnXhEyHjgc8ltKKx9M
0c+h4sKMXwjhjbQAZdtWIw8RRghpUJnKj+6bN2XzZDazyULxgPhtax0CAwEAAQ==
-----END PUBLIC KEY-----
'''

private_key_pem = '''
-----BEGIN RSA PRIVATE KEY-----
MIIBOwIBAAJBAKH0aYP9ZFuctlPnXhEyHjgc8ltKKx9M0c+h4sKMXwjhjbQAZdtW
Iw8RRghpUJnKj+6bN2XzZDazyULxgPhtax0CAwEAAQJADwR36EpNzQTqDzusCFIq
ZS+h9X8aIovgBK3RNhMIGO2ThpsnhiDTcqIvgQ56knbl6B2W4iOl54tJ6CNtf6l6
zQIhANTaNLFGsJfOvZHcI0WL1r89+1A4JVxR+lpslJJwAvgDAiEAwsjqqZ2wY2F0
F8p1J98BEbtjU2mEZIVCMn6vQuhWdl8CIDRL4IJl4eGKlB0QP0JJF1wpeGO/R76l
DaPF5cMM7k3NAiEAss28m/ck9BWBfFVdNjx/vsdFZkx2O9AX9EJWoBSnSgECIQCa
+sVQMUVJFGsdE/31C7wCIbE3IpB7ziABZ7mN+V3Dhg==
-----END RSA PRIVATE KEY-----
'''

# Private key components
prime1 = 96275860229939261876671084930484419185939191875438854026071315955024109172739
prime2 = 88103681619592083641803383393198542599284510949756076218404908654323473741407


class TestMarkers(unittest.TestCase):
    def test_values(self):
        self.assertEqual(_markers('RSA PRIVATE KEY'),
                         (b'-----BEGIN RSA PRIVATE KEY-----',
                          b'-----END RSA PRIVATE KEY-----'))


class TestBytesAndStrings(unittest.TestCase):
    """Test that we can use PEM in both Unicode strings and bytes."""

    def test_unicode_public(self):
        key = rsa.key.PublicKey.load_pkcs1_openssl_pem(public_key_pem)
        self.assertEqual(prime1 * prime2, key.n)

    def test_bytes_public(self):
        key = rsa.key.PublicKey.load_pkcs1_openssl_pem(public_key_pem.encode('ascii'))
        self.assertEqual(prime1 * prime2, key.n)

    def test_unicode_private(self):
        key = rsa.key.PrivateKey.load_pkcs1(private_key_pem)
        self.assertEqual(prime1 * prime2, key.n)

    def test_bytes_private(self):
        key = rsa.key.PrivateKey.load_pkcs1(private_key_pem.encode('ascii'))
        self.assertEqual(prime1, key.p)
        self.assertEqual(prime2, key.q)


class TestByteOutput(unittest.TestCase):
    """Tests that PEM and DER are returned as bytes."""

    def test_bytes_public(self):
        key = rsa.key.PublicKey.load_pkcs1_openssl_pem(public_key_pem)
        self.assertTrue(is_bytes(key.save_pkcs1(format='DER')))
        self.assertTrue(is_bytes(key.save_pkcs1(format='PEM')))

    def test_bytes_private(self):
        key = rsa.key.PrivateKey.load_pkcs1(private_key_pem)
        self.assertTrue(is_bytes(key.save_pkcs1(format='DER')))
        self.assertTrue(is_bytes(key.save_pkcs1(format='PEM')))


class TestByteInput(unittest.TestCase):
    """Tests that PEM and DER can be loaded from bytes."""

    def test_bytes_public(self):
        key = rsa.key.PublicKey.load_pkcs1_openssl_pem(public_key_pem.encode('ascii'))
        self.assertTrue(is_bytes(key.save_pkcs1(format='DER')))
        self.assertTrue(is_bytes(key.save_pkcs1(format='PEM')))

    def test_bytes_private(self):
        key = rsa.key.PrivateKey.load_pkcs1(private_key_pem.encode('ascii'))
        self.assertTrue(is_bytes(key.save_pkcs1(format='DER')))
        self.assertTrue(is_bytes(key.save_pkcs1(format='PEM')))
