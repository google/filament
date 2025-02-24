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

"""oauth2client tests.

Unit tests for service account credentials implemented using RSA.
"""

import datetime
import json
import os
import tempfile
import unittest

import mock
import rsa
import six
from six.moves import http_client

from oauth2client import client
from oauth2client import crypt
from oauth2client import service_account
from oauth2client import transport
from tests import http_mock


def data_filename(filename):
    return os.path.join(os.path.dirname(__file__), 'data', filename)


def datafile(filename):
    with open(data_filename(filename), 'rb') as file_obj:
        return file_obj.read()


class ServiceAccountCredentialsTests(unittest.TestCase):

    def setUp(self):
        self.orig_signer = crypt.Signer
        self.orig_verifier = crypt.Verifier
        self.client_id = '123'
        self.service_account_email = 'dummy@google.com'
        self.private_key_id = 'ABCDEF'
        self.private_key = datafile('pem_from_pkcs12.pem')
        self.scopes = ['dummy_scope']
        self.signer = crypt.Signer.from_string(self.private_key)
        self.credentials = service_account.ServiceAccountCredentials(
            self.service_account_email,
            self.signer,
            private_key_id=self.private_key_id,
            client_id=self.client_id,
        )

    def tearDown(self):
        crypt.Signer = self.orig_signer
        crypt.Verifier = self.orig_verifier

    @mock.patch('oauth2client.crypt.Signer.from_string', return_value=object())
    @mock.patch('oauth2client.crypt.make_signed_jwt', return_value=object())
    @mock.patch('time.time')
    def test__generate_assertion(self, time, mock_signed_jwt, _):
        now = 123456
        time.return_value = now
        payload1 = {
            'type': client.SERVICE_ACCOUNT,
            'client_id': 'id123',
            'client_email': 'foo@bar.com',
            'private_key_id': 'pkid456',
            'private_key': 's3kr3tz',
        }
        creds = self._from_json_keyfile_name_helper(payload1,
                                                    scopes=['foo', 'bar'],
                                                    token_uri='baz',
                                                    revoke_uri='qux')
        creds._generate_assertion()

        payload2 = {
            'aud': 'https://oauth2.googleapis.com/token',
            'scope': 'foo bar',
            'iat': now,
            'exp': now + creds.MAX_TOKEN_LIFETIME_SECS,
            'iss': 'foo@bar.com',
        }
        mock_signed_jwt.assert_called_once_with(creds._signer,
                                                payload2,
                                                key_id='pkid456')

    def test__to_json_override(self):
        signer = object()
        creds = service_account.ServiceAccountCredentials(
            'name@email.com', signer)
        self.assertEqual(creds._signer, signer)
        # Serialize over-ridden data (unrelated to ``creds``).
        to_serialize = {'unrelated': 'data'}
        serialized_str = creds._to_json([], to_serialize.copy())
        serialized_data = json.loads(serialized_str)
        expected_serialized = {
            '_class': 'ServiceAccountCredentials',
            '_module': 'oauth2client.service_account',
            'token_expiry': None,
        }
        expected_serialized.update(to_serialize)
        self.assertEqual(serialized_data, expected_serialized)

    def test_sign_blob(self):
        private_key_id, signature = self.credentials.sign_blob('Google')
        self.assertEqual(self.private_key_id, private_key_id)

        pub_key = rsa.PublicKey.load_pkcs1_openssl_pem(
            datafile('publickey_openssl.pem'))

        self.assertTrue(rsa.pkcs1.verify(b'Google', signature, pub_key))

        with self.assertRaises(rsa.pkcs1.VerificationError):
            rsa.pkcs1.verify(b'Orest', signature, pub_key)
        with self.assertRaises(rsa.pkcs1.VerificationError):
            rsa.pkcs1.verify(b'Google', b'bad signature', pub_key)

    def test_service_account_email(self):
        self.assertEqual(self.service_account_email,
                         self.credentials.service_account_email)

    @staticmethod
    def _from_json_keyfile_name_helper(payload, scopes=None,
                                       token_uri=None, revoke_uri=None):
        filehandle, filename = tempfile.mkstemp()
        os.close(filehandle)
        try:
            with open(filename, 'w') as file_obj:
                json.dump(payload, file_obj)
            return (
                service_account.ServiceAccountCredentials
                .from_json_keyfile_name(
                    filename, scopes=scopes, token_uri=token_uri,
                    revoke_uri=revoke_uri))
        finally:
            os.remove(filename)

    @mock.patch('oauth2client.crypt.Signer.from_string',
                return_value=object())
    def test_from_json_keyfile_name_factory(self, signer_factory):
        client_id = 'id123'
        client_email = 'foo@bar.com'
        private_key_id = 'pkid456'
        private_key = 's3kr3tz'
        payload = {
            'type': client.SERVICE_ACCOUNT,
            'client_id': client_id,
            'client_email': client_email,
            'private_key_id': private_key_id,
            'private_key': private_key,
        }
        scopes = ['foo', 'bar']
        token_uri = 'baz'
        revoke_uri = 'qux'
        base_creds = self._from_json_keyfile_name_helper(
            payload, scopes=scopes, token_uri=token_uri, revoke_uri=revoke_uri)
        self.assertEqual(base_creds._signer, signer_factory.return_value)
        signer_factory.assert_called_once_with(private_key)

        payload['token_uri'] = token_uri
        payload['revoke_uri'] = revoke_uri
        creds_with_uris_from_file = self._from_json_keyfile_name_helper(
            payload, scopes=scopes)
        for creds in (base_creds, creds_with_uris_from_file):
            self.assertIsInstance(
                creds, service_account.ServiceAccountCredentials)
            self.assertEqual(creds.client_id, client_id)
            self.assertEqual(creds._service_account_email, client_email)
            self.assertEqual(creds._private_key_id, private_key_id)
            self.assertEqual(creds._private_key_pkcs8_pem, private_key)
            self.assertEqual(creds._scopes, ' '.join(scopes))
            self.assertEqual(creds.token_uri, token_uri)
            self.assertEqual(creds.revoke_uri, revoke_uri)

    def test_from_json_keyfile_name_factory_bad_type(self):
        type_ = 'bad-type'
        self.assertNotEqual(type_, client.SERVICE_ACCOUNT)
        payload = {'type': type_}
        with self.assertRaises(ValueError):
            self._from_json_keyfile_name_helper(payload)

    def test_from_json_keyfile_name_factory_missing_field(self):
        payload = {
            'type': client.SERVICE_ACCOUNT,
            'client_id': 'my-client',
        }
        with self.assertRaises(KeyError):
            self._from_json_keyfile_name_helper(payload)

    def _from_p12_keyfile_helper(self, private_key_password=None, scopes='',
                                 token_uri=None, revoke_uri=None):
        service_account_email = 'name@email.com'
        filename = data_filename('privatekey.p12')
        with open(filename, 'rb') as file_obj:
            key_contents = file_obj.read()
        creds_from_filename = (
            service_account.ServiceAccountCredentials.from_p12_keyfile(
                service_account_email, filename,
                private_key_password=private_key_password,
                scopes=scopes, token_uri=token_uri, revoke_uri=revoke_uri))
        creds_from_file_contents = (
            service_account.ServiceAccountCredentials.from_p12_keyfile_buffer(
                service_account_email, six.BytesIO(key_contents),
                private_key_password=private_key_password,
                scopes=scopes, token_uri=token_uri, revoke_uri=revoke_uri))
        for creds in (creds_from_filename, creds_from_file_contents):
            self.assertIsInstance(
                creds, service_account.ServiceAccountCredentials)
            self.assertIsNone(creds.client_id)
            self.assertEqual(creds._service_account_email,
                             service_account_email)
            self.assertIsNone(creds._private_key_id)
            self.assertIsNone(creds._private_key_pkcs8_pem)
            self.assertEqual(creds._private_key_pkcs12, key_contents)
            if private_key_password is not None:
                self.assertEqual(creds._private_key_password,
                                 private_key_password)
            self.assertEqual(creds._scopes, ' '.join(scopes))
            self.assertEqual(creds.token_uri, token_uri)
            self.assertEqual(creds.revoke_uri, revoke_uri)

    def _p12_not_implemented_helper(self):
        service_account_email = 'name@email.com'
        filename = data_filename('privatekey.p12')
        with self.assertRaises(NotImplementedError):
            service_account.ServiceAccountCredentials.from_p12_keyfile(
                service_account_email, filename)

    @mock.patch('oauth2client.crypt.Signer', new=crypt.PyCryptoSigner)
    def test_from_p12_keyfile_with_pycrypto(self):
        self._p12_not_implemented_helper()

    @mock.patch('oauth2client.crypt.Signer', new=crypt.RsaSigner)
    def test_from_p12_keyfile_with_rsa(self):
        self._p12_not_implemented_helper()

    def test_from_p12_keyfile_defaults(self):
        self._from_p12_keyfile_helper()

    def test_from_p12_keyfile_explicit(self):
        password = 'notasecret'
        self._from_p12_keyfile_helper(private_key_password=password,
                                      scopes=['foo', 'bar'],
                                      token_uri='baz', revoke_uri='qux')

    def test_create_scoped_required_without_scopes(self):
        self.assertTrue(self.credentials.create_scoped_required())

    def test_create_scoped_required_with_scopes(self):
        signer = object()
        self.credentials = service_account.ServiceAccountCredentials(
            self.service_account_email,
            signer,
            scopes=self.scopes,
            private_key_id=self.private_key_id,
            client_id=self.client_id,
        )
        self.assertFalse(self.credentials.create_scoped_required())

    def test_create_scoped(self):
        new_credentials = self.credentials.create_scoped(self.scopes)
        self.assertNotEqual(self.credentials, new_credentials)
        self.assertIsInstance(new_credentials,
                              service_account.ServiceAccountCredentials)
        self.assertEqual('dummy_scope', new_credentials._scopes)

    def test_create_delegated(self):
        signer = object()
        sub = 'foo@email.com'
        creds = service_account.ServiceAccountCredentials(
            'name@email.com', signer)
        self.assertNotIn('sub', creds._kwargs)
        delegated_creds = creds.create_delegated(sub)
        self.assertEqual(delegated_creds._kwargs['sub'], sub)
        # Make sure the original is unchanged.
        self.assertNotIn('sub', creds._kwargs)

    def test_create_delegated_existing_sub(self):
        signer = object()
        sub1 = 'existing@email.com'
        sub2 = 'new@email.com'
        creds = service_account.ServiceAccountCredentials(
            'name@email.com', signer, sub=sub1)
        self.assertEqual(creds._kwargs['sub'], sub1)
        delegated_creds = creds.create_delegated(sub2)
        self.assertEqual(delegated_creds._kwargs['sub'], sub2)
        # Make sure the original is unchanged.
        self.assertEqual(creds._kwargs['sub'], sub1)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_access_token(self, utcnow):
        # Configure the patch.
        seconds = 11
        NOW = datetime.datetime(1992, 12, 31, second=seconds)
        utcnow.return_value = NOW

        # Create a custom credentials with a mock signer.
        signer = mock.Mock()
        signed_value = b'signed-content'
        signer.sign = mock.Mock(name='sign',
                                return_value=signed_value)
        credentials = service_account.ServiceAccountCredentials(
            self.service_account_email,
            signer,
            private_key_id=self.private_key_id,
            client_id=self.client_id,
        )

        # Begin testing.
        lifetime = 2  # number of seconds in which the token expires
        EXPIRY_TIME = datetime.datetime(1992, 12, 31,
                                        second=seconds + lifetime)

        token1 = u'first_token'
        token_response_first = {
            'access_token': token1,
            'expires_in': lifetime,
        }
        token2 = u'second_token'
        token_response_second = {
            'access_token': token2,
            'expires_in': lifetime,
        }
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK},
             json.dumps(token_response_first).encode('utf-8')),
            ({'status': http_client.OK},
             json.dumps(token_response_second).encode('utf-8')),
        ])

        # Get Access Token, First attempt.
        self.assertIsNone(credentials.access_token)
        self.assertFalse(credentials.access_token_expired)
        self.assertIsNone(credentials.token_expiry)
        token = credentials.get_access_token(http=http)
        self.assertEqual(credentials.token_expiry, EXPIRY_TIME)
        self.assertEqual(token1, token.access_token)
        self.assertEqual(lifetime, token.expires_in)
        self.assertEqual(token_response_first,
                         credentials.token_response)
        # Two utcnow calls are expected:
        # - get_access_token() -> _do_refresh_request (setting expires in)
        # - get_access_token() -> _expires_in()
        expected_utcnow_calls = [mock.call()] * 2
        self.assertEqual(expected_utcnow_calls, utcnow.mock_calls)
        # One call to sign() expected: Actual refresh was needed.
        self.assertEqual(len(signer.sign.mock_calls), 1)

        # Get Access Token, Second Attempt (not expired)
        self.assertEqual(credentials.access_token, token1)
        self.assertFalse(credentials.access_token_expired)
        token = credentials.get_access_token(http=http)
        # Make sure no refresh occurred since the token was not expired.
        self.assertEqual(token1, token.access_token)
        self.assertEqual(lifetime, token.expires_in)
        self.assertEqual(token_response_first, credentials.token_response)
        # Three more utcnow calls are expected:
        # - access_token_expired
        # - get_access_token() -> access_token_expired
        # - get_access_token -> _expires_in
        expected_utcnow_calls = [mock.call()] * (2 + 3)
        self.assertEqual(expected_utcnow_calls, utcnow.mock_calls)
        # No call to sign() expected: the token was not expired.
        self.assertEqual(len(signer.sign.mock_calls), 1 + 0)

        # Get Access Token, Third Attempt (force expiration)
        self.assertEqual(credentials.access_token, token1)
        credentials.token_expiry = NOW  # Manually force expiry.
        self.assertTrue(credentials.access_token_expired)
        token = credentials.get_access_token(http=http)
        # Make sure refresh occurred since the token was not expired.
        self.assertEqual(token2, token.access_token)
        self.assertEqual(lifetime, token.expires_in)
        self.assertFalse(credentials.access_token_expired)
        self.assertEqual(token_response_second,
                         credentials.token_response)
        # Five more utcnow calls are expected:
        # - access_token_expired
        # - get_access_token -> access_token_expired
        # - get_access_token -> _do_refresh_request
        # - get_access_token -> _expires_in
        # - access_token_expired
        expected_utcnow_calls = [mock.call()] * (2 + 3 + 5)
        self.assertEqual(expected_utcnow_calls, utcnow.mock_calls)
        # One more call to sign() expected: Actual refresh was needed.
        self.assertEqual(len(signer.sign.mock_calls), 1 + 0 + 1)

        self.assertEqual(credentials.access_token, token2)


TOKEN_LIFE = service_account._JWTAccessCredentials._MAX_TOKEN_LIFETIME_SECS
T1 = 42
T1_DATE = datetime.datetime(1970, 1, 1, second=T1)
T1_EXPIRY = T1 + TOKEN_LIFE
T1_EXPIRY_DATE = T1_DATE + datetime.timedelta(seconds=TOKEN_LIFE)

T2 = T1 + 100
T2_DATE = T1_DATE + datetime.timedelta(seconds=100)
T2_EXPIRY = T2 + TOKEN_LIFE
T2_EXPIRY_DATE = T2_DATE + datetime.timedelta(seconds=TOKEN_LIFE)

T3 = T1 + TOKEN_LIFE + 1
T3_DATE = T1_DATE + datetime.timedelta(seconds=TOKEN_LIFE + 1)
T3_EXPIRY = T3 + TOKEN_LIFE
T3_EXPIRY_DATE = T3_DATE + datetime.timedelta(seconds=TOKEN_LIFE)


class JWTAccessCredentialsTests(unittest.TestCase):

    def setUp(self):
        self.client_id = '123'
        self.service_account_email = 'dummy@google.com'
        self.private_key_id = 'ABCDEF'
        self.private_key = datafile('pem_from_pkcs12.pem')
        self.signer = crypt.Signer.from_string(self.private_key)
        self.url = 'https://test.url.com'
        self.jwt = service_account._JWTAccessCredentials(
            self.service_account_email, self.signer,
            private_key_id=self.private_key_id, client_id=self.client_id,
            additional_claims={'aud': self.url})

    @mock.patch('oauth2client.client._UTCNOW')
    @mock.patch('time.time')
    def test_get_access_token_no_claims(self, time, utcnow):
        utcnow.return_value = T1_DATE
        time.return_value = T1

        token_info = self.jwt.get_access_token()
        certs = {'key': datafile('public_cert.pem')}
        payload = crypt.verify_signed_jwt_with_certs(
            token_info.access_token, certs, audience=self.url)
        self.assertEqual(len(payload), 5)
        self.assertEqual(payload['iss'], self.service_account_email)
        self.assertEqual(payload['sub'], self.service_account_email)
        self.assertEqual(payload['iat'], T1)
        self.assertEqual(payload['exp'], T1_EXPIRY)
        self.assertEqual(payload['aud'], self.url)
        self.assertEqual(token_info.expires_in, T1_EXPIRY - T1)

        # Verify that we vend the same token after 100 seconds
        utcnow.return_value = T2_DATE
        token_info = self.jwt.get_access_token()
        payload = crypt.verify_signed_jwt_with_certs(
            token_info.access_token,
            {'key': datafile('public_cert.pem')}, audience=self.url)
        self.assertEqual(payload['iat'], T1)
        self.assertEqual(payload['exp'], T1_EXPIRY)
        self.assertEqual(token_info.expires_in, T1_EXPIRY - T2)

        # Verify that we vend a new token after _MAX_TOKEN_LIFETIME_SECS
        utcnow.return_value = T3_DATE
        time.return_value = T3
        token_info = self.jwt.get_access_token()
        payload = crypt.verify_signed_jwt_with_certs(
            token_info.access_token,
            {'key': datafile('public_cert.pem')}, audience=self.url)
        expires_in = token_info.expires_in
        self.assertEqual(payload['iat'], T3)
        self.assertEqual(payload['exp'], T3_EXPIRY)
        self.assertEqual(expires_in, T3_EXPIRY - T3)

    @mock.patch('oauth2client.client._UTCNOW')
    @mock.patch('time.time')
    def test_get_access_token_additional_claims(self, time, utcnow):
        utcnow.return_value = T1_DATE
        time.return_value = T1

        audience = 'https://test2.url.com'
        subject = 'dummy2@google.com'
        claims = {'aud': audience, 'sub': subject}
        token_info = self.jwt.get_access_token(additional_claims=claims)
        certs = {'key': datafile('public_cert.pem')}
        payload = crypt.verify_signed_jwt_with_certs(
            token_info.access_token, certs, audience=audience)
        expires_in = token_info.expires_in
        self.assertEqual(len(payload), 5)
        self.assertEqual(payload['iss'], self.service_account_email)
        self.assertEqual(payload['sub'], subject)
        self.assertEqual(payload['iat'], T1)
        self.assertEqual(payload['exp'], T1_EXPIRY)
        self.assertEqual(payload['aud'], audience)
        self.assertEqual(expires_in, T1_EXPIRY - T1)

    def test_revoke(self):
        self.jwt.revoke(None)

    def test_create_scoped_required(self):
        self.assertTrue(self.jwt.create_scoped_required())

    def test_create_scoped(self):
        self.jwt._private_key_pkcs12 = ''
        self.jwt._private_key_password = ''

        new_credentials = self.jwt.create_scoped('dummy_scope')
        self.assertNotEqual(self.jwt, new_credentials)
        self.assertIsInstance(
            new_credentials, service_account.ServiceAccountCredentials)
        self.assertEqual('dummy_scope', new_credentials._scopes)

    @mock.patch('oauth2client.client._UTCNOW')
    @mock.patch('time.time')
    def test_authorize_success(self, time, utcnow):
        utcnow.return_value = T1_DATE
        time.return_value = T1

        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b''),
            ({'status': http_client.OK}, b''),
        ])

        self.jwt.authorize(http)
        transport.request(http, self.url)

        # Ensure we use the cached token
        utcnow.return_value = T2_DATE
        transport.request(http, self.url)

        # Verify mocks.
        certs = {'key': datafile('public_cert.pem')}
        self.assertEqual(len(http.requests), 2)
        for info in http.requests:
            self.assertEqual(info['method'], 'GET')
            self.assertEqual(info['uri'], self.url)
            self.assertIsNone(info['body'])
            self.assertEqual(len(info['headers']), 1)
            bearer, token = info['headers'][b'Authorization'].split()
            self.assertEqual(bearer, b'Bearer')
            payload = crypt.verify_signed_jwt_with_certs(
                token, certs, audience=self.url)
            self.assertEqual(len(payload), 5)
            self.assertEqual(payload['iss'], self.service_account_email)
            self.assertEqual(payload['sub'], self.service_account_email)
            self.assertEqual(payload['iat'], T1)
            self.assertEqual(payload['exp'], T1_EXPIRY)
            self.assertEqual(payload['aud'], self.url)

    @mock.patch('oauth2client.client._UTCNOW')
    @mock.patch('time.time')
    def test_authorize_no_aud(self, time, utcnow):
        utcnow.return_value = T1_DATE
        time.return_value = T1

        jwt = service_account._JWTAccessCredentials(
            self.service_account_email, self.signer,
            private_key_id=self.private_key_id, client_id=self.client_id)

        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b''),
        ])

        jwt.authorize(http)
        transport.request(http, self.url)

        # Ensure we do not cache the token
        self.assertIsNone(jwt.access_token)

        # Verify mocks.
        self.assertEqual(len(http.requests), 1)
        info = http.requests[0]
        self.assertEqual(info['method'], 'GET')
        self.assertEqual(info['uri'], self.url)
        self.assertIsNone(info['body'])
        self.assertEqual(len(info['headers']), 1)
        bearer, token = info['headers'][b'Authorization'].split()
        self.assertEqual(bearer, b'Bearer')
        certs = {'key': datafile('public_cert.pem')}
        payload = crypt.verify_signed_jwt_with_certs(
            token, certs, audience=self.url)
        self.assertEqual(len(payload), 5)
        self.assertEqual(payload['iss'], self.service_account_email)
        self.assertEqual(payload['sub'], self.service_account_email)
        self.assertEqual(payload['iat'], T1)
        self.assertEqual(payload['exp'], T1_EXPIRY)
        self.assertEqual(payload['aud'], self.url)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_authorize_stale_token(self, utcnow):
        utcnow.return_value = T1_DATE
        # Create an initial token
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b''),
            ({'status': http_client.OK}, b''),
        ])
        self.jwt.authorize(http)
        transport.request(http, self.url)
        token_1 = self.jwt.access_token

        # Expire the token
        utcnow.return_value = T3_DATE
        transport.request(http, self.url)
        token_2 = self.jwt.access_token
        self.assertEquals(self.jwt.token_expiry, T3_EXPIRY_DATE)
        self.assertNotEqual(token_1, token_2)

        # Verify mocks.
        certs = {'key': datafile('public_cert.pem')}
        self.assertEqual(len(http.requests), 2)
        issued_at_vals = (T1, T3)
        exp_vals = (T1_EXPIRY, T3_EXPIRY)
        for info, issued_at, exp_val in zip(http.requests, issued_at_vals,
                                            exp_vals):
            self.assertEqual(info['uri'], self.url)
            self.assertEqual(info['method'], 'GET')
            self.assertIsNone(info['body'])
            self.assertEqual(len(info['headers']), 1)
            bearer, token = info['headers'][b'Authorization'].split()
            self.assertEqual(bearer, b'Bearer')
            # To parse the token, skip the time check, since this
            # test intentionally has stale tokens.
            with mock.patch('oauth2client.crypt._verify_time_range',
                            return_value=True):
                payload = crypt.verify_signed_jwt_with_certs(
                    token, certs, audience=self.url)
            self.assertEqual(len(payload), 5)
            self.assertEqual(payload['iss'], self.service_account_email)
            self.assertEqual(payload['sub'], self.service_account_email)
            self.assertEqual(payload['iat'], issued_at)
            self.assertEqual(payload['exp'], exp_val)
            self.assertEqual(payload['aud'], self.url)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_authorize_401(self, utcnow):
        utcnow.return_value = T1_DATE

        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b''),
            ({'status': http_client.UNAUTHORIZED}, b''),
            ({'status': http_client.OK}, b''),
        ])
        self.jwt.authorize(http)
        transport.request(http, self.url)
        token_1 = self.jwt.access_token

        utcnow.return_value = T2_DATE
        response, _ = transport.request(http, self.url)
        self.assertEquals(response.status, http_client.OK)
        token_2 = self.jwt.access_token
        # Check the 401 forced a new token
        self.assertNotEqual(token_1, token_2)

        # Verify mocks.
        certs = {'key': datafile('public_cert.pem')}
        self.assertEqual(len(http.requests), 3)
        issued_at_vals = (T1, T1, T2)
        exp_vals = (T1_EXPIRY, T1_EXPIRY, T2_EXPIRY)
        for info, issued_at, exp_val in zip(http.requests, issued_at_vals,
                                            exp_vals):
            self.assertEqual(info['uri'], self.url)
            self.assertEqual(info['method'], 'GET')
            self.assertIsNone(info['body'])
            self.assertEqual(len(info['headers']), 1)
            bearer, token = info['headers'][b'Authorization'].split()
            self.assertEqual(bearer, b'Bearer')
            # To parse the token, skip the time check, since this
            # test intentionally has stale tokens.
            with mock.patch('oauth2client.crypt._verify_time_range',
                            return_value=True):
                payload = crypt.verify_signed_jwt_with_certs(
                    token, certs, audience=self.url)
            self.assertEqual(len(payload), 5)
            self.assertEqual(payload['iss'], self.service_account_email)
            self.assertEqual(payload['sub'], self.service_account_email)
            self.assertEqual(payload['iat'], issued_at)
            self.assertEqual(payload['exp'], exp_val)
            self.assertEqual(payload['aud'], self.url)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_refresh(self, utcnow):
        utcnow.return_value = T1_DATE
        token_1 = self.jwt.access_token

        utcnow.return_value = T2_DATE
        self.jwt.refresh(None)
        token_2 = self.jwt.access_token
        self.assertEquals(self.jwt.token_expiry, T2_EXPIRY_DATE)
        self.assertNotEqual(token_1, token_2)
