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

"""Tests string operations."""

import struct
import sys
import unittest

import rsa
from rsa import pkcs1
from rsa._compat import byte, is_bytes


class BinaryTest(unittest.TestCase):
    def setUp(self):
        (self.pub, self.priv) = rsa.newkeys(256)

    def test_enc_dec(self):
        message = struct.pack('>IIII', 0, 0, 0, 1)
        print("\tMessage:   %r" % message)

        encrypted = pkcs1.encrypt(message, self.pub)
        print("\tEncrypted: %r" % encrypted)

        decrypted = pkcs1.decrypt(encrypted, self.priv)
        print("\tDecrypted: %r" % decrypted)

        self.assertEqual(message, decrypted)

    def test_decoding_failure(self):
        message = struct.pack('>IIII', 0, 0, 0, 1)
        encrypted = pkcs1.encrypt(message, self.pub)

        # Alter the encrypted stream
        a = encrypted[5]
        if is_bytes(a):
            a = ord(a)
        altered_a = (a + 1) % 256
        encrypted = encrypted[:5] + byte(altered_a) + encrypted[6:]

        self.assertRaises(pkcs1.DecryptionError, pkcs1.decrypt, encrypted,
                          self.priv)

    def test_randomness(self):
        """Encrypting the same message twice should result in different
        cryptos.
        """

        message = struct.pack('>IIII', 0, 0, 0, 1)
        encrypted1 = pkcs1.encrypt(message, self.pub)
        encrypted2 = pkcs1.encrypt(message, self.pub)

        self.assertNotEqual(encrypted1, encrypted2)


class ExtraZeroesTest(unittest.TestCase):
    def setUp(self):
        # Key, cyphertext, and plaintext taken from https://github.com/sybrenstuvel/python-rsa/issues/146
        self.private_key = rsa.PrivateKey.load_pkcs1(
            "-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEAs1EKK81M5kTFtZSuUFnhKy8FS2WNXaWVmi/fGHG4CLw98+Yo\n0nkuUarVwSS0O9pFPcpc3kvPKOe9Tv+6DLS3Qru21aATy2PRqjqJ4CYn71OYtSwM\n/ZfSCKvrjXybzgu+sBmobdtYm+sppbdL+GEHXGd8gdQw8DDCZSR6+dPJFAzLZTCd\nB+Ctwe/RXPF+ewVdfaOGjkZIzDoYDw7n+OHnsYCYozkbTOcWHpjVevipR+IBpGPi\n1rvKgFnlcG6d/tj0hWRl/6cS7RqhjoiNEtxqoJzpXs/Kg8xbCxXbCchkf11STA8u\ndiCjQWuWI8rcDwl69XMmHJjIQAqhKvOOQ8rYTQIDAQABAoIBABpQLQ7qbHtp4h1Y\nORAfcFRW7Q74UvtH/iEHH1TF8zyM6wZsYtcn4y0mxYE3Mp+J0xlTJbeVJkwZXYVH\nL3UH29CWHSlR+TWiazTwrCTRVJDhEoqbcTiRW8fb+o/jljVxMcVDrpyYUHNo2c6w\njBxhmKPtp66hhaDpds1Cwi0A8APZ8Z2W6kya/L/hRBzMgCz7Bon1nYBMak5PQEwV\nF0dF7Wy4vIjvCzO6DSqA415DvJDzUAUucgFudbANNXo4HJwNRnBpymYIh8mHdmNJ\n/MQ0YLSqUWvOB57dh7oWQwe3UsJ37ZUorTugvxh3NJ7Tt5ZqbCQBEECb9ND63gxo\n/a3YR/0CgYEA7BJc834xCi/0YmO5suBinWOQAF7IiRPU+3G9TdhWEkSYquupg9e6\nK9lC5k0iP+t6I69NYF7+6mvXDTmv6Z01o6oV50oXaHeAk74O3UqNCbLe9tybZ/+F\ndkYlwuGSNttMQBzjCiVy0+y0+Wm3rRnFIsAtd0RlZ24aN3bFTWJINIsCgYEAwnQq\nvNmJe9SwtnH5c/yCqPhKv1cF/4jdQZSGI6/p3KYNxlQzkHZ/6uvrU5V27ov6YbX8\nvKlKfO91oJFQxUD6lpTdgAStI3GMiJBJIZNpyZ9EWNSvwUj28H34cySpbZz3s4Xd\nhiJBShgy+fKURvBQwtWmQHZJ3EGrcOI7PcwiyYcCgYEAlql5jSUCY0ALtidzQogW\nJ+B87N+RGHsBuJ/0cxQYinwg+ySAAVbSyF1WZujfbO/5+YBN362A/1dn3lbswCnH\nK/bHF9+fZNqvwprPnceQj5oK1n4g6JSZNsy6GNAhosT+uwQ0misgR8SQE4W25dDG\nkdEYsz+BgCsyrCcu8J5C+tUCgYAFVPQbC4f2ikVyKzvgz0qx4WUDTBqRACq48p6e\n+eLatv7nskVbr7QgN+nS9+Uz80ihR0Ev1yCAvnwmM/XYAskcOea87OPmdeWZlQM8\nVXNwINrZ6LMNBLgorfuTBK1UoRo1pPUHCYdqxbEYI2unak18mikd2WB7Fp3h0YI4\nVpGZnwKBgBxkAYnZv+jGI4MyEKdsQgxvROXXYOJZkWzsKuKxVkVpYP2V4nR2YMOJ\nViJQ8FUEnPq35cMDlUk4SnoqrrHIJNOvcJSCqM+bWHAioAsfByLbUPM8sm3CDdIk\nXVJl32HuKYPJOMIWfc7hIfxLRHnCN+coz2M6tgqMDs0E/OfjuqVZ\n-----END RSA PRIVATE KEY-----",
            format='PEM')
        cyphertext = "4501b4d669e01b9ef2dc800aa1b06d49196f5a09fe8fbcd037323c60eaf027bfb98432be4e4a26c567ffec718bcbea977dd26812fa071c33808b4d5ebb742d9879806094b6fbeea63d25ea3141733b60e31c6912106e1b758a7fe0014f075193faa8b4622bfd5d3013f0a32190a95de61a3604711bc62945f95a6522bd4dfed0a994ef185b28c281f7b5e4c8ed41176d12d9fc1b837e6a0111d0132d08a6d6f0580de0c9eed8ed105531799482d1e466c68c23b0c222af7fc12ac279bc4ff57e7b4586d209371b38c4c1035edd418dc5f960441cb21ea2bedbfea86de0d7861e81021b650a1de51002c315f1e7c12debe4dcebf790caaa54a2f26b149cf9e77d"
        plaintext = "54657374"

        if sys.version_info < (3, 0):
            self.cyphertext = cyphertext.decode("hex")
            self.plaintext = plaintext.decode('hex')
        else:
            self.cyphertext = bytes.fromhex(cyphertext)
            self.plaintext = bytes.fromhex(plaintext)

    def test_unmodified(self):
        message = rsa.decrypt(self.cyphertext, self.private_key)
        self.assertEqual(message, self.plaintext)

    def test_prepend_zeroes(self):
        cyphertext = b'\00\00' + self.cyphertext
        with self.assertRaises(rsa.DecryptionError):
            rsa.decrypt(cyphertext, self.private_key)

    def test_append_zeroes(self):
        cyphertext = self.cyphertext + b'\00\00'
        with self.assertRaises(rsa.DecryptionError):
            rsa.decrypt(cyphertext, self.private_key)


class SignatureTest(unittest.TestCase):
    def setUp(self):
        (self.pub, self.priv) = rsa.newkeys(512)

    def test_sign_verify(self):
        """Test happy flow of sign and verify"""

        message = b'je moeder'
        signature = pkcs1.sign(message, self.priv, 'SHA-256')

        self.assertEqual('SHA-256', pkcs1.verify(message, signature, self.pub))

    def test_find_signature_hash(self):
        """Test happy flow of sign and find_signature_hash"""

        message = b'je moeder'
        signature = pkcs1.sign(message, self.priv, 'SHA-256')

        self.assertEqual('SHA-256', pkcs1.find_signature_hash(signature, self.pub))

    def test_alter_message(self):
        """Altering the message should let the verification fail."""

        signature = pkcs1.sign(b'je moeder', self.priv, 'SHA-256')
        self.assertRaises(pkcs1.VerificationError, pkcs1.verify,
                          b'mijn moeder', signature, self.pub)

    def test_sign_different_key(self):
        """Signing with another key should let the verification fail."""

        (otherpub, _) = rsa.newkeys(512)

        message = b'je moeder'
        signature = pkcs1.sign(message, self.priv, 'SHA-256')
        self.assertRaises(pkcs1.VerificationError, pkcs1.verify,
                          message, signature, otherpub)

    def test_multiple_signings(self):
        """Signing the same message twice should return the same signatures."""

        message = struct.pack('>IIII', 0, 0, 0, 1)
        signature1 = pkcs1.sign(message, self.priv, 'SHA-1')
        signature2 = pkcs1.sign(message, self.priv, 'SHA-1')

        self.assertEqual(signature1, signature2)

    def test_split_hash_sign(self):
        """Hashing and then signing should match with directly signing the message. """

        message = b'je moeder'
        msg_hash = pkcs1.compute_hash(message, 'SHA-256')
        signature1 = pkcs1.sign_hash(msg_hash, self.priv, 'SHA-256')

        # Calculate the signature using the unified method
        signature2 = pkcs1.sign(message, self.priv, 'SHA-256')

        self.assertEqual(signature1, signature2)

    def test_hash_sign_verify(self):
        """Test happy flow of hash, sign, and verify"""

        message = b'je moeder'
        msg_hash = pkcs1.compute_hash(message, 'SHA-224')
        signature = pkcs1.sign_hash(msg_hash, self.priv, 'SHA-224')

        self.assertTrue(pkcs1.verify(message, signature, self.pub))

    def test_prepend_zeroes(self):
        """Prepending the signature with zeroes should be detected."""

        message = b'je moeder'
        signature = pkcs1.sign(message, self.priv, 'SHA-256')
        signature = b'\00\00' + signature
        with self.assertRaises(rsa.VerificationError):
            pkcs1.verify(message, signature, self.pub)

    def test_apppend_zeroes(self):
        """Apppending the signature with zeroes should be detected."""

        message = b'je moeder'
        signature = pkcs1.sign(message, self.priv, 'SHA-256')
        signature = signature + b'\00\00'
        with self.assertRaises(rsa.VerificationError):
            pkcs1.verify(message, signature, self.pub)
