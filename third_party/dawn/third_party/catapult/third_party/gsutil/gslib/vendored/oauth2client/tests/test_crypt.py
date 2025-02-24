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

import base64
import os
import unittest

import mock

from oauth2client import _helpers
from oauth2client import client
from oauth2client import crypt
from oauth2client import service_account


def data_filename(filename):
    return os.path.join(os.path.dirname(__file__), 'data', filename)


def datafile(filename):
    with open(data_filename(filename), 'rb') as file_obj:
        return file_obj.read()


class Test__bad_pkcs12_key_as_pem(unittest.TestCase):

    def test_fails(self):
        with self.assertRaises(NotImplementedError):
            crypt._bad_pkcs12_key_as_pem()


class Test_pkcs12_key_as_pem(unittest.TestCase):

    def _make_svc_account_creds(self, private_key_file='privatekey.p12'):
        filename = data_filename(private_key_file)
        credentials = (
            service_account.ServiceAccountCredentials.from_p12_keyfile(
                'some_account@example.com', filename,
                scopes='read+write'))
        credentials._kwargs['sub'] = 'joe@example.org'
        return credentials

    def _succeeds_helper(self, password=None):
        self.assertEqual(True, client.HAS_OPENSSL)

        credentials = self._make_svc_account_creds()
        if password is None:
            password = credentials._private_key_password
        pem_contents = crypt.pkcs12_key_as_pem(
            credentials._private_key_pkcs12, password)
        pkcs12_key_as_pem = datafile('pem_from_pkcs12.pem')
        pkcs12_key_as_pem = _helpers._parse_pem_key(pkcs12_key_as_pem)
        alternate_pem = datafile('pem_from_pkcs12_alternate.pem')
        self.assertTrue(pem_contents in [pkcs12_key_as_pem, alternate_pem])

    def test_succeeds(self):
        self._succeeds_helper()

    def test_succeeds_with_unicode_password(self):
        password = u'notasecret'
        self._succeeds_helper(password)


class Test__verify_signature(unittest.TestCase):

    def test_success_single_cert(self):
        cert_value = 'cert-value'
        certs = [cert_value]
        message = object()
        signature = object()

        verifier = mock.Mock()
        verifier.verify = mock.Mock(name='verify', return_value=True)
        with mock.patch('oauth2client.crypt.Verifier') as Verifier:
            Verifier.from_string = mock.Mock(name='from_string',
                                             return_value=verifier)
            result = crypt._verify_signature(message, signature, certs)
            self.assertEqual(result, None)

            # Make sure our mocks were called as expected.
            Verifier.from_string.assert_called_once_with(cert_value,
                                                         is_x509_cert=True)
            verifier.verify.assert_called_once_with(message, signature)

    def test_success_multiple_certs(self):
        cert_value1 = 'cert-value1'
        cert_value2 = 'cert-value2'
        cert_value3 = 'cert-value3'
        certs = [cert_value1, cert_value2, cert_value3]
        message = object()
        signature = object()

        verifier = mock.Mock()
        # Use side_effect to force all 3 cert values to be used by failing
        # to verify on the first two.
        verifier.verify = mock.Mock(name='verify',
                                    side_effect=[False, False, True])
        with mock.patch('oauth2client.crypt.Verifier') as Verifier:
            Verifier.from_string = mock.Mock(name='from_string',
                                             return_value=verifier)
            result = crypt._verify_signature(message, signature, certs)
            self.assertEqual(result, None)

            # Make sure our mocks were called three times.
            expected_from_string_calls = [
                mock.call(cert_value1, is_x509_cert=True),
                mock.call(cert_value2, is_x509_cert=True),
                mock.call(cert_value3, is_x509_cert=True),
            ]
            self.assertEqual(Verifier.from_string.mock_calls,
                             expected_from_string_calls)
            expected_verify_calls = [mock.call(message, signature)] * 3
            self.assertEqual(verifier.verify.mock_calls,
                             expected_verify_calls)

    def test_failure(self):
        cert_value = 'cert-value'
        certs = [cert_value]
        message = object()
        signature = object()

        verifier = mock.Mock()
        verifier.verify = mock.Mock(name='verify', return_value=False)
        with mock.patch('oauth2client.crypt.Verifier') as Verifier:
            Verifier.from_string = mock.Mock(name='from_string',
                                             return_value=verifier)
            with self.assertRaises(crypt.AppIdentityError):
                crypt._verify_signature(message, signature, certs)

            # Make sure our mocks were called as expected.
            Verifier.from_string.assert_called_once_with(cert_value,
                                                         is_x509_cert=True)
            verifier.verify.assert_called_once_with(message, signature)


class Test__check_audience(unittest.TestCase):

    def test_null_audience(self):
        result = crypt._check_audience(None, None)
        self.assertEqual(result, None)

    def test_success(self):
        audience = 'audience'
        payload_dict = {'aud': audience}
        result = crypt._check_audience(payload_dict, audience)
        # No exception and no result.
        self.assertEqual(result, None)

    def test_missing_aud(self):
        audience = 'audience'
        payload_dict = {}
        with self.assertRaises(crypt.AppIdentityError):
            crypt._check_audience(payload_dict, audience)

    def test_wrong_aud(self):
        audience1 = 'audience1'
        audience2 = 'audience2'
        self.assertNotEqual(audience1, audience2)
        payload_dict = {'aud': audience1}
        with self.assertRaises(crypt.AppIdentityError):
            crypt._check_audience(payload_dict, audience2)


class Test__verify_time_range(unittest.TestCase):

    def _exception_helper(self, payload_dict):
        exception_caught = None
        try:
            crypt._verify_time_range(payload_dict)
        except crypt.AppIdentityError as exc:
            exception_caught = exc

        return exception_caught

    def test_without_issued_at(self):
        payload_dict = {}
        exception_caught = self._exception_helper(payload_dict)
        self.assertNotEqual(exception_caught, None)
        self.assertTrue(str(exception_caught).startswith(
            'No iat field in token'))

    def test_without_expiration(self):
        payload_dict = {'iat': 'iat'}
        exception_caught = self._exception_helper(payload_dict)
        self.assertNotEqual(exception_caught, None)
        self.assertTrue(str(exception_caught).startswith(
            'No exp field in token'))

    def test_with_bad_token_lifetime(self):
        current_time = 123456
        payload_dict = {
            'iat': 'iat',
            'exp': current_time + crypt.MAX_TOKEN_LIFETIME_SECS + 1,
        }
        with mock.patch('oauth2client.crypt.time') as time:
            time.time = mock.Mock(name='time',
                                  return_value=current_time)

            exception_caught = self._exception_helper(payload_dict)
            self.assertNotEqual(exception_caught, None)
            self.assertTrue(str(exception_caught).startswith(
                'exp field too far in future'))

    def test_with_issued_at_in_future(self):
        current_time = 123456
        payload_dict = {
            'iat': current_time + crypt.CLOCK_SKEW_SECS + 1,
            'exp': current_time + crypt.MAX_TOKEN_LIFETIME_SECS - 1,
        }
        with mock.patch('oauth2client.crypt.time') as time:
            time.time = mock.Mock(name='time',
                                  return_value=current_time)

            exception_caught = self._exception_helper(payload_dict)
            self.assertNotEqual(exception_caught, None)
            self.assertTrue(str(exception_caught).startswith(
                'Token used too early'))

    def test_with_expiration_in_the_past(self):
        current_time = 123456
        payload_dict = {
            'iat': current_time,
            'exp': current_time - crypt.CLOCK_SKEW_SECS - 1,
        }
        with mock.patch('oauth2client.crypt.time') as time:
            time.time = mock.Mock(name='time',
                                  return_value=current_time)

            exception_caught = self._exception_helper(payload_dict)
            self.assertNotEqual(exception_caught, None)
            self.assertTrue(str(exception_caught).startswith(
                'Token used too late'))

    def test_success(self):
        current_time = 123456
        payload_dict = {
            'iat': current_time,
            'exp': current_time + crypt.MAX_TOKEN_LIFETIME_SECS - 1,
        }
        with mock.patch('oauth2client.crypt.time') as time:
            time.time = mock.Mock(name='time',
                                  return_value=current_time)

            exception_caught = self._exception_helper(payload_dict)
            self.assertEqual(exception_caught, None)


class Test_verify_signed_jwt_with_certs(unittest.TestCase):

    def test_jwt_no_segments(self):
        exception_caught = None
        try:
            crypt.verify_signed_jwt_with_certs(b'', None)
        except crypt.AppIdentityError as exc:
            exception_caught = exc

        self.assertNotEqual(exception_caught, None)
        self.assertTrue(str(exception_caught).startswith(
            'Wrong number of segments in token'))

    def test_jwt_payload_bad_json(self):
        header = signature = b''
        payload = base64.b64encode(b'{BADJSON')
        jwt = b'.'.join([header, payload, signature])

        exception_caught = None
        try:
            crypt.verify_signed_jwt_with_certs(jwt, None)
        except crypt.AppIdentityError as exc:
            exception_caught = exc

        self.assertNotEqual(exception_caught, None)
        self.assertTrue(str(exception_caught).startswith(
            'Can\'t parse token'))

    @mock.patch('oauth2client.crypt._check_audience')
    @mock.patch('oauth2client.crypt._verify_time_range')
    @mock.patch('oauth2client.crypt._verify_signature')
    def test_success(self, verify_sig, verify_time, check_aud):
        certs = mock.Mock()
        cert_values = object()
        certs.values = mock.Mock(name='values',
                                 return_value=cert_values)
        audience = object()

        header = b'header'
        signature_bytes = b'signature'
        signature = base64.b64encode(signature_bytes)
        payload_dict = {'a': 'b'}
        payload = base64.b64encode(b'{"a": "b"}')
        jwt = b'.'.join([header, payload, signature])

        result = crypt.verify_signed_jwt_with_certs(
            jwt, certs, audience=audience)
        self.assertEqual(result, payload_dict)

        message_to_sign = header + b'.' + payload
        verify_sig.assert_called_once_with(
            message_to_sign, signature_bytes, cert_values)
        verify_time.assert_called_once_with(payload_dict)
        check_aud.assert_called_once_with(payload_dict, audience)
        certs.values.assert_called_once_with()
