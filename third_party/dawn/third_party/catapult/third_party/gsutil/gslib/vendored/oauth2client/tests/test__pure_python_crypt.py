# Copyright 2016 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Unit tests for oauth2client._pure_python_crypt."""

import os
import unittest

import mock
from pyasn1_modules import pem
import rsa
import six

from oauth2client import _helpers
from oauth2client import _pure_python_crypt
from oauth2client import crypt


class TestRsaVerifier(unittest.TestCase):

    PUBLIC_KEY_FILENAME = os.path.join(os.path.dirname(__file__),
                                       'data', 'privatekey.pub')
    PUBLIC_CERT_FILENAME = os.path.join(os.path.dirname(__file__),
                                        'data', 'public_cert.pem')
    PRIVATE_KEY_FILENAME = os.path.join(os.path.dirname(__file__),
                                        'data', 'privatekey.pem')

    def _load_public_key_bytes(self):
        with open(self.PUBLIC_KEY_FILENAME, 'rb') as fh:
            return fh.read()

    def _load_public_cert_bytes(self):
        with open(self.PUBLIC_CERT_FILENAME, 'rb') as fh:
            return fh.read()

    def _load_private_key_bytes(self):
        with open(self.PRIVATE_KEY_FILENAME, 'rb') as fh:
            return fh.read()

    def test_verify_success(self):
        to_sign = b'foo'
        signer = crypt.RsaSigner.from_string(self._load_private_key_bytes())
        actual_signature = signer.sign(to_sign)

        verifier = crypt.RsaVerifier.from_string(
            self._load_public_key_bytes(), is_x509_cert=False)
        self.assertTrue(verifier.verify(to_sign, actual_signature))

    def test_verify_unicode_success(self):
        to_sign = u'foo'
        signer = crypt.RsaSigner.from_string(self._load_private_key_bytes())
        actual_signature = signer.sign(to_sign)

        verifier = crypt.RsaVerifier.from_string(
            self._load_public_key_bytes(), is_x509_cert=False)
        self.assertTrue(verifier.verify(to_sign, actual_signature))

    def test_verify_failure(self):
        verifier = crypt.RsaVerifier.from_string(
            self._load_public_key_bytes(), is_x509_cert=False)
        bad_signature1 = b''
        self.assertFalse(verifier.verify(b'foo', bad_signature1))
        bad_signature2 = b'a'
        self.assertFalse(verifier.verify(b'foo', bad_signature2))

    def test_from_string_pub_key(self):
        public_key = self._load_public_key_bytes()
        verifier = crypt.RsaVerifier.from_string(
            public_key, is_x509_cert=False)
        self.assertIsInstance(verifier, crypt.RsaVerifier)
        self.assertIsInstance(verifier._pubkey, rsa.key.PublicKey)

    def test_from_string_pub_key_unicode(self):
        public_key = _helpers._from_bytes(self._load_public_key_bytes())
        verifier = crypt.RsaVerifier.from_string(
            public_key, is_x509_cert=False)
        self.assertIsInstance(verifier, crypt.RsaVerifier)
        self.assertIsInstance(verifier._pubkey, rsa.key.PublicKey)

    def test_from_string_pub_cert(self):
        public_cert = self._load_public_cert_bytes()
        verifier = crypt.RsaVerifier.from_string(
            public_cert, is_x509_cert=True)
        self.assertIsInstance(verifier, crypt.RsaVerifier)
        self.assertIsInstance(verifier._pubkey, rsa.key.PublicKey)

    def test_from_string_pub_cert_unicode(self):
        public_cert = _helpers._from_bytes(self._load_public_cert_bytes())
        verifier = crypt.RsaVerifier.from_string(
            public_cert, is_x509_cert=True)
        self.assertIsInstance(verifier, crypt.RsaVerifier)
        self.assertIsInstance(verifier._pubkey, rsa.key.PublicKey)

    def test_from_string_pub_cert_failure(self):
        cert_bytes = self._load_public_cert_bytes()
        true_der = rsa.pem.load_pem(cert_bytes, 'CERTIFICATE')
        with mock.patch('rsa.pem.load_pem',
                        return_value=true_der + b'extra') as load_pem:
            with self.assertRaises(ValueError):
                crypt.RsaVerifier.from_string(cert_bytes, is_x509_cert=True)
            load_pem.assert_called_once_with(cert_bytes, 'CERTIFICATE')


class TestRsaSigner(unittest.TestCase):

    PKCS1_KEY_FILENAME = os.path.join(os.path.dirname(__file__),
                                      'data', 'privatekey.pem')
    PKCS8_KEY_FILENAME = os.path.join(os.path.dirname(__file__),
                                      'data', 'pem_from_pkcs12.pem')
    PKCS12_KEY_FILENAME = os.path.join(os.path.dirname(__file__),
                                       'data', 'privatekey.p12')

    def _load_pkcs1_key_bytes(self):
        with open(self.PKCS1_KEY_FILENAME, 'rb') as fh:
            return fh.read()

    def _load_pkcs8_key_bytes(self):
        with open(self.PKCS8_KEY_FILENAME, 'rb') as fh:
            return fh.read()

    def _load_pkcs12_key_bytes(self):
        with open(self.PKCS12_KEY_FILENAME, 'rb') as fh:
            return fh.read()

    def test_from_string_pkcs1(self):
        key_bytes = self._load_pkcs1_key_bytes()
        signer = crypt.RsaSigner.from_string(key_bytes)
        self.assertIsInstance(signer, crypt.RsaSigner)
        self.assertIsInstance(signer._key, rsa.key.PrivateKey)

    def test_from_string_pkcs1_unicode(self):
        key_bytes = _helpers._from_bytes(self._load_pkcs1_key_bytes())
        signer = crypt.RsaSigner.from_string(key_bytes)
        self.assertIsInstance(signer, crypt.RsaSigner)
        self.assertIsInstance(signer._key, rsa.key.PrivateKey)

    def test_from_string_pkcs8(self):
        key_bytes = self._load_pkcs8_key_bytes()
        signer = crypt.RsaSigner.from_string(key_bytes)
        self.assertIsInstance(signer, crypt.RsaSigner)
        self.assertIsInstance(signer._key, rsa.key.PrivateKey)

    def test_from_string_pkcs8_extra_bytes(self):
        key_bytes = self._load_pkcs8_key_bytes()
        _, pem_bytes = pem.readPemBlocksFromFile(
            six.StringIO(_helpers._from_bytes(key_bytes)),
            _pure_python_crypt._PKCS8_MARKER)

        with mock.patch('pyasn1.codec.der.decoder.decode') as mock_decode:
            key_info, remaining = None, 'extra'
            mock_decode.return_value = (key_info, remaining)
            with self.assertRaises(ValueError):
                crypt.RsaSigner.from_string(key_bytes)
            # Verify mock was called.
            mock_decode.assert_called_once_with(
                pem_bytes, asn1Spec=_pure_python_crypt._PKCS8_SPEC)

    def test_from_string_pkcs8_unicode(self):
        key_bytes = _helpers._from_bytes(self._load_pkcs8_key_bytes())
        signer = crypt.RsaSigner.from_string(key_bytes)
        self.assertIsInstance(signer, crypt.RsaSigner)
        self.assertIsInstance(signer._key, rsa.key.PrivateKey)

    def test_from_string_pkcs12(self):
        key_bytes = self._load_pkcs12_key_bytes()
        with self.assertRaises(ValueError):
            crypt.RsaSigner.from_string(key_bytes)

    def test_from_string_bogus_key(self):
        key_bytes = 'bogus-key'
        with self.assertRaises(ValueError):
            crypt.RsaSigner.from_string(key_bytes)
