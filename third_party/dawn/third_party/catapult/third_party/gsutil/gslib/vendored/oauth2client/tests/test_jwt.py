# Copyright 2014 Google Inc. All rights reserved.
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

"""Unit tests for JWT related methods in oauth2client."""

import os
import tempfile
import time
import unittest

import mock
from six.moves import http_client

from oauth2client import _helpers
from oauth2client import client
from oauth2client import crypt
from oauth2client import file as file_module
from oauth2client import service_account
from oauth2client import transport
from tests import http_mock


_FORMATS_TO_CONSTRUCTOR_ARGS = {
    'p12': 'private_key_pkcs12',
    'pem': 'private_key_pkcs8_pem',
}


def data_filename(filename):
    return os.path.join(os.path.dirname(__file__), 'data', filename)


def datafile(filename):
    with open(data_filename(filename), 'rb') as file_obj:
        return file_obj.read()


class CryptTests(unittest.TestCase):

    def setUp(self):
        self.format_ = 'p12'
        self.signer = crypt.OpenSSLSigner
        self.verifier = crypt.OpenSSLVerifier

    def test_sign_and_verify(self):
        self._check_sign_and_verify('privatekey.' + self.format_)

    def test_sign_and_verify_from_converted_pkcs12(self):
        # Tests that following instructions to convert from PKCS12 to
        # PEM works.
        if self.format_ == 'pem':
            self._check_sign_and_verify('pem_from_pkcs12.pem')

    def _check_sign_and_verify(self, private_key_file):
        private_key = datafile(private_key_file)
        public_key = datafile('public_cert.pem')

        # We pass in a non-bytes password to make sure all branches
        # are traversed in tests.
        signer = self.signer.from_string(private_key,
                                         password=u'notasecret')
        signature = signer.sign('foo')

        verifier = self.verifier.from_string(public_key, True)
        self.assertTrue(verifier.verify(b'foo', signature))

        self.assertFalse(verifier.verify(b'bar', signature))
        self.assertFalse(verifier.verify(b'foo', b'bad signagure'))
        self.assertFalse(verifier.verify(b'foo', u'bad signagure'))

    def _check_jwt_failure(self, jwt, expected_error):
        public_key = datafile('public_cert.pem')
        certs = {'foo': public_key}
        audience = ('https://www.googleapis.com/auth/id?client_id='
                    'external_public_key@testing.gserviceaccount.com')

        with self.assertRaises(crypt.AppIdentityError) as exc_manager:
            crypt.verify_signed_jwt_with_certs(jwt, certs, audience)

        self.assertTrue(expected_error in str(exc_manager.exception))

    def _create_signed_jwt(self):
        private_key = datafile('privatekey.' + self.format_)
        signer = self.signer.from_string(private_key)
        audience = 'some_audience_address@testing.gserviceaccount.com'
        now = int(time.time())

        return crypt.make_signed_jwt(signer, {
            'aud': audience,
            'iat': now,
            'exp': now + 300,
            'user': 'billy bob',
            'metadata': {'meta': 'data'},
        })

    def test_verify_id_token(self):
        jwt = self._create_signed_jwt()
        public_key = datafile('public_cert.pem')
        certs = {'foo': public_key}
        audience = 'some_audience_address@testing.gserviceaccount.com'
        contents = crypt.verify_signed_jwt_with_certs(jwt, certs, audience)
        self.assertEqual('billy bob', contents['user'])
        self.assertEqual('data', contents['metadata']['meta'])

    def _verify_http_mock(self, http):
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, client.ID_TOKEN_VERIFICATION_CERTS)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertIsNone(http.headers)

    def test_verify_id_token_with_certs_uri(self):
        jwt = self._create_signed_jwt()

        http = http_mock.HttpMock(data=datafile('certs.json'))
        contents = client.verify_id_token(
            jwt, 'some_audience_address@testing.gserviceaccount.com',
            http=http)
        self.assertEqual('billy bob', contents['user'])
        self.assertEqual('data', contents['metadata']['meta'])

        # Verify mocks.
        self._verify_http_mock(http)

    def test_verify_id_token_with_certs_uri_default_http(self):
        jwt = self._create_signed_jwt()

        http = http_mock.HttpMock(data=datafile('certs.json'))

        with mock.patch('oauth2client.transport._CACHED_HTTP', new=http):
            contents = client.verify_id_token(
                jwt, 'some_audience_address@testing.gserviceaccount.com')

        self.assertEqual('billy bob', contents['user'])
        self.assertEqual('data', contents['metadata']['meta'])

        # Verify mocks.
        self._verify_http_mock(http)

    def test_verify_id_token_with_certs_uri_fails(self):
        jwt = self._create_signed_jwt()
        test_email = 'some_audience_address@testing.gserviceaccount.com'

        http = http_mock.HttpMock(
            headers={'status': http_client.NOT_FOUND},
            data=datafile('certs.json'))

        with self.assertRaises(client.VerifyJwtTokenError):
            client.verify_id_token(jwt, test_email, http=http)

        # Verify mocks.
        self._verify_http_mock(http)

    def test_verify_id_token_bad_tokens(self):
        private_key = datafile('privatekey.' + self.format_)

        # Wrong number of segments
        self._check_jwt_failure('foo', 'Wrong number of segments')

        # Not json
        self._check_jwt_failure('foo.bar.baz', 'Can\'t parse token')

        # Bad signature
        jwt = b'.'.join([b'foo',
                         _helpers._urlsafe_b64encode('{"a":"b"}'),
                         b'baz'])
        self._check_jwt_failure(jwt, 'Invalid token signature')

        # No expiration
        signer = self.signer.from_string(private_key)
        audience = ('https:#www.googleapis.com/auth/id?client_id='
                    'external_public_key@testing.gserviceaccount.com')
        jwt = crypt.make_signed_jwt(signer, {
            'aud': audience,
            'iat': time.time(),
        })
        self._check_jwt_failure(jwt, 'No exp field in token')

        # No issued at
        jwt = crypt.make_signed_jwt(signer, {
            'aud': 'audience',
            'exp': time.time() + 400,
        })
        self._check_jwt_failure(jwt, 'No iat field in token')

        # Too early
        jwt = crypt.make_signed_jwt(signer, {
            'aud': 'audience',
            'iat': time.time() + 301,
            'exp': time.time() + 400,
        })
        self._check_jwt_failure(jwt, 'Token used too early')

        # Too late
        jwt = crypt.make_signed_jwt(signer, {
            'aud': 'audience',
            'iat': time.time() - 500,
            'exp': time.time() - 301,
        })
        self._check_jwt_failure(jwt, 'Token used too late')

        # Wrong target
        jwt = crypt.make_signed_jwt(signer, {
            'aud': 'somebody else',
            'iat': time.time(),
            'exp': time.time() + 300,
        })
        self._check_jwt_failure(jwt, 'Wrong recipient')

    def test_from_string_non_509_cert(self):
        # Use a private key instead of a certificate to test the other branch
        # of from_string().
        public_key = datafile('privatekey.pem')
        verifier = self.verifier.from_string(public_key, is_x509_cert=False)
        self.assertIsInstance(verifier, self.verifier)


class PEMCryptTestsPyCrypto(CryptTests):

    def setUp(self):
        self.format_ = 'pem'
        self.signer = crypt.PyCryptoSigner
        self.verifier = crypt.PyCryptoVerifier


class PEMCryptTestsOpenSSL(CryptTests):

    def setUp(self):
        self.format_ = 'pem'
        self.signer = crypt.OpenSSLSigner
        self.verifier = crypt.OpenSSLVerifier


class SignedJwtAssertionCredentialsTests(unittest.TestCase):

    def setUp(self):
        self.orig_signer = crypt.Signer
        self.format_ = 'p12'
        crypt.Signer = crypt.OpenSSLSigner

    def tearDown(self):
        crypt.Signer = self.orig_signer

    def _make_credentials(self):
        private_key = datafile('privatekey.' + self.format_)
        signer = crypt.Signer.from_string(private_key)
        credentials = service_account.ServiceAccountCredentials(
            'some_account@example.com', signer,
            scopes='read+write',
            sub='joe@example.org')
        if self.format_ == 'pem':
            credentials._private_key_pkcs8_pem = private_key
        elif self.format_ == 'p12':
            credentials._private_key_pkcs12 = private_key
            credentials._private_key_password = (
                service_account._PASSWORD_DEFAULT)
        else:  # pragma: NO COVER
            raise ValueError('Unexpected format.')
        return credentials

    def test_credentials_good(self):
        credentials = self._make_credentials()
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK},
             b'{"access_token":"1/3w","expires_in":3600}'),
            ({'status': http_client.OK}, 'echo_request_headers'),
        ])
        http = credentials.authorize(http)
        resp, content = transport.request(http, 'http://example.org')
        self.assertEqual(b'Bearer 1/3w', content[b'Authorization'])

    def test_credentials_to_from_json(self):
        credentials = self._make_credentials()
        json = credentials.to_json()
        restored = client.Credentials.new_from_json(json)
        self.assertEqual(credentials._private_key_pkcs12,
                         restored._private_key_pkcs12)
        self.assertEqual(credentials._private_key_password,
                         restored._private_key_password)
        self.assertEqual(credentials._kwargs, restored._kwargs)

    def _credentials_refresh(self, credentials):
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK},
             b'{"access_token":"1/3w","expires_in":3600}'),
            ({'status': http_client.UNAUTHORIZED}, b''),
            ({'status': http_client.OK},
             b'{"access_token":"3/3w","expires_in":3600}'),
            ({'status': http_client.OK}, 'echo_request_headers'),
        ])
        http = credentials.authorize(http)
        _, content = transport.request(http, 'http://example.org')
        return content

    def test_credentials_refresh_without_storage(self):
        credentials = self._make_credentials()
        content = self._credentials_refresh(credentials)
        self.assertEqual(b'Bearer 3/3w', content[b'Authorization'])

    def test_credentials_refresh_with_storage(self):
        credentials = self._make_credentials()

        filehandle, filename = tempfile.mkstemp()
        os.close(filehandle)
        store = file_module.Storage(filename)
        store.put(credentials)
        credentials.set_store(store)

        content = self._credentials_refresh(credentials)

        self.assertEqual(b'Bearer 3/3w', content[b'Authorization'])
        os.unlink(filename)


class PEMSignedJwtAssertionCredentialsOpenSSLTests(
        SignedJwtAssertionCredentialsTests):

    def setUp(self):
        self.orig_signer = crypt.Signer
        self.format_ = 'pem'
        crypt.Signer = crypt.OpenSSLSigner

    def tearDown(self):
        crypt.Signer = self.orig_signer


class PEMSignedJwtAssertionCredentialsPyCryptoTests(
        SignedJwtAssertionCredentialsTests):

    def setUp(self):
        self.orig_signer = crypt.Signer
        self.format_ = 'pem'
        crypt.Signer = crypt.PyCryptoSigner

    def tearDown(self):
        crypt.Signer = self.orig_signer


class TestHasOpenSSLFlag(unittest.TestCase):

    def test_true(self):
        self.assertEqual(True, client.HAS_OPENSSL)
        self.assertEqual(True, client.HAS_CRYPTO)
