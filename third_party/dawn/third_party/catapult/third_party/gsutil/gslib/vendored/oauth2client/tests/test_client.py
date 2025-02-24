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

"""Unit tests for oauth2client.client."""

import base64
import contextlib
import copy
import datetime
import json
import os
import socket
import sys
import tempfile
import unittest

import mock
import six
from six.moves import http_client
from six.moves import urllib

import oauth2client
from oauth2client import _helpers
from oauth2client import client
from oauth2client import clientsecrets
from oauth2client import service_account
from oauth2client import transport
from tests import http_mock


DATA_DIR = os.path.join(os.path.dirname(__file__), 'data')


def assertUrisEqual(testcase, expected, actual):
    """Test that URIs are the same, up to reordering of query parameters."""
    expected = urllib.parse.urlparse(expected)
    actual = urllib.parse.urlparse(actual)
    testcase.assertEqual(expected.scheme, actual.scheme)
    testcase.assertEqual(expected.netloc, actual.netloc)
    testcase.assertEqual(expected.path, actual.path)
    testcase.assertEqual(expected.params, actual.params)
    testcase.assertEqual(expected.fragment, actual.fragment)
    expected_query = urllib.parse.parse_qs(expected.query)
    actual_query = urllib.parse.parse_qs(actual.query)
    for name in expected_query.keys():
        testcase.assertEqual(expected_query[name], actual_query[name])
    for name in actual_query.keys():
        testcase.assertEqual(expected_query[name], actual_query[name])


def datafile(filename):
    return os.path.join(DATA_DIR, filename)


def load_and_cache(existing_file, fakename, cache_mock):
    client_type, client_info = clientsecrets._loadfile(datafile(existing_file))
    cache_mock.cache[fakename] = {client_type: client_info}


class CredentialsTests(unittest.TestCase):

    def test_to_from_json(self):
        credentials = client.Credentials()
        json = credentials.to_json()
        client.Credentials.new_from_json(json)

    def test_authorize_abstract(self):
        credentials = client.Credentials()
        http = object()
        with self.assertRaises(NotImplementedError):
            credentials.authorize(http)

    def test_refresh_abstract(self):
        credentials = client.Credentials()
        http = object()
        with self.assertRaises(NotImplementedError):
            credentials.refresh(http)

    def test_revoke_abstract(self):
        credentials = client.Credentials()
        http = object()
        with self.assertRaises(NotImplementedError):
            credentials.revoke(http)

    def test_apply_abstract(self):
        credentials = client.Credentials()
        headers = {}
        with self.assertRaises(NotImplementedError):
            credentials.apply(headers)

    def test__to_json_basic(self):
        credentials = client.Credentials()
        json_payload = credentials._to_json([])
        # str(bytes) in Python2 and str(unicode) in Python3
        self.assertIsInstance(json_payload, str)
        payload = json.loads(json_payload)
        expected_payload = {
            '_class': client.Credentials.__name__,
            '_module': client.Credentials.__module__,
            'token_expiry': None,
        }
        self.assertEqual(payload, expected_payload)

    def test__to_json_with_strip(self):
        credentials = client.Credentials()
        credentials.foo = 'bar'
        credentials.baz = 'quux'
        to_strip = ['foo']
        json_payload = credentials._to_json(to_strip)
        # str(bytes) in Python2 and str(unicode) in Python3
        self.assertIsInstance(json_payload, str)
        payload = json.loads(json_payload)
        expected_payload = {
            '_class': client.Credentials.__name__,
            '_module': client.Credentials.__module__,
            'token_expiry': None,
            'baz': credentials.baz,
        }
        self.assertEqual(payload, expected_payload)

    def test__to_json_to_serialize(self):
        credentials = client.Credentials()
        to_serialize = {
            'foo': b'bar',
            'baz': u'quux',
            'st': set(['a', 'b']),
        }
        orig_vals = to_serialize.copy()
        json_payload = credentials._to_json([], to_serialize=to_serialize)
        # str(bytes) in Python2 and str(unicode) in Python3
        self.assertIsInstance(json_payload, str)
        payload = json.loads(json_payload)
        expected_payload = {
            '_class': client.Credentials.__name__,
            '_module': client.Credentials.__module__,
            'token_expiry': None,
        }
        expected_payload.update(to_serialize)
        # Special-case the set.
        expected_payload['st'] = list(expected_payload['st'])
        # Special-case the bytes.
        expected_payload['foo'] = u'bar'
        self.assertEqual(payload, expected_payload)
        # Make sure the method call didn't modify our dictionary.
        self.assertEqual(to_serialize, orig_vals)

    @mock.patch.object(client.Credentials, '_to_json',
                       return_value=object())
    def test_to_json(self, to_json):
        credentials = client.Credentials()
        self.assertEqual(credentials.to_json(), to_json.return_value)
        to_json.assert_called_once_with(
            client.Credentials.NON_SERIALIZED_MEMBERS)

    def test_new_from_json_no_data(self):
        creds_data = {}
        json_data = json.dumps(creds_data)
        with self.assertRaises(KeyError):
            client.Credentials.new_from_json(json_data)

    def test_new_from_json_basic_data(self):
        creds_data = {
            '_module': 'oauth2client.client',
            '_class': 'Credentials',
        }
        json_data = json.dumps(creds_data)
        credentials = client.Credentials.new_from_json(json_data)
        self.assertIsInstance(credentials, client.Credentials)

    def test_new_from_json_old_name(self):
        creds_data = {
            '_module': 'oauth2client.googleapiclient.client',
            '_class': 'Credentials',
        }
        json_data = json.dumps(creds_data)
        credentials = client.Credentials.new_from_json(json_data)
        self.assertIsInstance(credentials, client.Credentials)

    def test_new_from_json_bad_module(self):
        creds_data = {
            '_module': 'oauth2client.foobar',
            '_class': 'Credentials',
        }
        json_data = json.dumps(creds_data)
        with self.assertRaises(ImportError):
            client.Credentials.new_from_json(json_data)

    def test_new_from_json_bad_class(self):
        creds_data = {
            '_module': 'oauth2client.client',
            '_class': 'NopeNotCredentials',
        }
        json_data = json.dumps(creds_data)
        with self.assertRaises(AttributeError):
            client.Credentials.new_from_json(json_data)

    def test_from_json(self):
        unused_data = {}
        credentials = client.Credentials.from_json(unused_data)
        self.assertIsInstance(credentials, client.Credentials)
        self.assertEqual(credentials.__dict__, {})


class TestStorage(unittest.TestCase):

    def test_locked_get_abstract(self):
        storage = client.Storage()
        with self.assertRaises(NotImplementedError):
            storage.locked_get()

    def test_locked_put_abstract(self):
        storage = client.Storage()
        credentials = object()
        with self.assertRaises(NotImplementedError):
            storage.locked_put(credentials)

    def test_locked_delete_abstract(self):
        storage = client.Storage()
        with self.assertRaises(NotImplementedError):
            storage.locked_delete()


@contextlib.contextmanager
def mock_module_import(module):
    """Place a dummy objects in sys.modules to mock an import test."""
    parts = module.split('.')
    entries = ['.'.join(parts[:i + 1]) for i in range(len(parts))]
    for entry in entries:
        sys.modules[entry] = object()

    try:
        yield

    finally:
        for entry in entries:
            del sys.modules[entry]


class GoogleCredentialsTests(unittest.TestCase):

    def setUp(self):
        self.os_name = os.name
        client.SETTINGS.env_name = None

    def tearDown(self):
        self.reset_env('SERVER_SOFTWARE')
        self.reset_env(client.GOOGLE_APPLICATION_CREDENTIALS)
        self.reset_env('APPDATA')
        os.name = self.os_name

    def reset_env(self, env):
        """Set the environment variable 'env' to 'value'."""
        os.environ.pop(env, None)

    def validate_service_account_credentials(self, credentials):
        self.assertIsInstance(
            credentials, service_account.ServiceAccountCredentials)
        self.assertEqual('123', credentials.client_id)
        self.assertEqual('dummy@google.com',
                         credentials._service_account_email)
        self.assertEqual('ABCDEF', credentials._private_key_id)
        self.assertEqual('', credentials._scopes)

    def validate_google_credentials(self, credentials):
        self.assertIsInstance(credentials, client.GoogleCredentials)
        self.assertEqual(None, credentials.access_token)
        self.assertEqual('123', credentials.client_id)
        self.assertEqual('secret', credentials.client_secret)
        self.assertEqual('alabalaportocala', credentials.refresh_token)
        self.assertEqual(None, credentials.token_expiry)
        self.assertEqual(oauth2client.GOOGLE_TOKEN_URI, credentials.token_uri)
        self.assertEqual('Python client library', credentials.user_agent)

    def get_a_google_credentials_object(self):
        return client.GoogleCredentials(None, None, None, None,
                                        None, None, None, None)

    def test_create_scoped_required(self):
        self.assertFalse(
            self.get_a_google_credentials_object().create_scoped_required())

    def test_create_scoped(self):
        credentials = self.get_a_google_credentials_object()
        self.assertEqual(credentials, credentials.create_scoped(None))
        self.assertEqual(credentials,
                         credentials.create_scoped(['dummy_scope']))

    @mock.patch.object(client.GoogleCredentials,
                       '_implicit_credentials_from_files',
                       return_value=None)
    @mock.patch.object(client.GoogleCredentials,
                       '_implicit_credentials_from_gce')
    @mock.patch.object(client, '_in_gae_environment',
                       return_value=True)
    @mock.patch.object(client, '_get_application_default_credential_GAE',
                       return_value=object())
    def test_get_application_default_in_gae(self, gae_adc, in_gae,
                                            from_gce, from_files):
        credentials = client.GoogleCredentials.get_application_default()
        self.assertEqual(credentials, gae_adc.return_value)
        from_files.assert_called_once_with()
        in_gae.assert_called_once_with()
        from_gce.assert_not_called()

    @mock.patch.object(client.GoogleCredentials,
                       '_implicit_credentials_from_gae',
                       return_value=None)
    @mock.patch.object(client.GoogleCredentials,
                       '_implicit_credentials_from_files',
                       return_value=None)
    @mock.patch.object(client, '_in_gce_environment',
                       return_value=True)
    @mock.patch.object(client, '_get_application_default_credential_GCE',
                       return_value=object())
    def test_get_application_default_in_gce(self, gce_adc, in_gce,
                                            from_files, from_gae):
        credentials = client.GoogleCredentials.get_application_default()
        self.assertEqual(credentials, gce_adc.return_value)
        in_gce.assert_called_once_with()
        from_gae.assert_called_once_with()
        from_files.assert_called_once_with()

    def test_environment_check_gae_production(self):
        with mock_module_import('google.appengine'):
            self._environment_check_gce_helper(
                server_software='Google App Engine/XYZ')

    def test_environment_check_gae_local(self):
        with mock_module_import('google.appengine'):
            self._environment_check_gce_helper(
                server_software='Development/XYZ')

    def test_environment_check_fastpath(self):
        with mock_module_import('google.appengine'):
            self._environment_check_gce_helper(
                server_software='Development/XYZ')

    def test_environment_caching(self):
        os.environ['SERVER_SOFTWARE'] = 'Development/XYZ'
        with mock_module_import('google.appengine'):
            self.assertTrue(client._in_gae_environment())
            os.environ['SERVER_SOFTWARE'] = ''
            # Even though we no longer pass the environment check, it
            # is cached.
            self.assertTrue(client._in_gae_environment())

    def _environment_check_gce_helper(self, status_ok=True,
                                      server_software=''):
        if status_ok:
            headers = {'status': http_client.OK}
            headers.update(client._GCE_HEADERS)
        else:
            headers = {'status': http_client.NOT_FOUND}

        http = http_mock.HttpMock(headers=headers)
        with mock.patch('oauth2client.client.os') as os_module:
            os_module.environ = {client._SERVER_SOFTWARE: server_software}
            with mock.patch('oauth2client.transport.get_http_object',
                            return_value=http) as new_http:
                if server_software == '':
                    self.assertFalse(client._in_gae_environment())
                else:
                    self.assertTrue(client._in_gae_environment())

                if status_ok and server_software == '':
                    self.assertTrue(client._in_gce_environment())
                else:
                    self.assertFalse(client._in_gce_environment())

                # Verify mocks.
                if server_software == '':
                    new_http.assert_called_once_with(
                        timeout=client.GCE_METADATA_TIMEOUT)
                    self.assertEqual(http.requests, 1)
                    self.assertEqual(http.uri, client._GCE_METADATA_URI)
                    self.assertEqual(http.method, 'GET')
                    self.assertIsNone(http.body)
                    self.assertEqual(http.headers, client._GCE_HEADERS)
                else:
                    new_http.assert_not_called()
                    self.assertEqual(http.requests, 0)

    def test_environment_check_gce_production(self):
        self._environment_check_gce_helper(status_ok=True)

    def test_environment_check_gce_prod_with_working_gae_imports(self):
        with mock_module_import('google.appengine'):
            self._environment_check_gce_helper(status_ok=True)

    @mock.patch('oauth2client.client.os.environ',
                new={client._SERVER_SOFTWARE: ''})
    @mock.patch('oauth2client.transport.get_http_object',
                return_value=object())
    @mock.patch('oauth2client.transport.request',
                side_effect=socket.timeout())
    def test_environment_check_gce_timeout(self, mock_request, new_http):
        self.assertFalse(client._in_gae_environment())
        self.assertFalse(client._in_gce_environment())

        # Verify mocks.
        new_http.assert_called_once_with(timeout=client.GCE_METADATA_TIMEOUT)
        mock_request.assert_called_once_with(
            new_http.return_value, client._GCE_METADATA_URI,
            headers=client._GCE_HEADERS)

    def test_environ_check_gae_module_unknown(self):
        with mock_module_import('google.appengine'):
            self._environment_check_gce_helper(status_ok=False)

    def test_environment_check_unknown(self):
        self._environment_check_gce_helper(status_ok=False)

    def test_get_environment_variable_file(self):
        environment_variable_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        os.environ[client.GOOGLE_APPLICATION_CREDENTIALS] = (
            environment_variable_file)
        self.assertEqual(environment_variable_file,
                         client._get_environment_variable_file())

    def test_get_environment_variable_file_error(self):
        nonexistent_file = datafile('nonexistent')
        os.environ[client.GOOGLE_APPLICATION_CREDENTIALS] = nonexistent_file
        expected_err_msg = (
            'File {0} \(pointed by {1} environment variable\) does not '
            'exist!'.format(
                nonexistent_file, client.GOOGLE_APPLICATION_CREDENTIALS))
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            client._get_environment_variable_file()

    @mock.patch.dict(os.environ, {}, clear=True)
    def test_get_environment_variable_file_without_env_var(self):
        self.assertIsNone(client._get_environment_variable_file())

    @mock.patch('os.name', new='nt')
    @mock.patch.dict(os.environ, {'APPDATA': DATA_DIR}, clear=True)
    def test_get_well_known_file_on_windows(self):
        well_known_file = datafile(
            os.path.join(client._CLOUDSDK_CONFIG_DIRECTORY,
                         client._WELL_KNOWN_CREDENTIALS_FILE))
        self.assertEqual(well_known_file, client._get_well_known_file())

    @mock.patch('os.name', new='nt')
    @mock.patch.dict(os.environ, {'SystemDrive': 'G:'}, clear=True)
    def test_get_well_known_file_on_windows_without_appdata(self):
        well_known_file = os.path.join('G:', '\\',
                                       client._CLOUDSDK_CONFIG_DIRECTORY,
                                       client._WELL_KNOWN_CREDENTIALS_FILE)
        self.assertEqual(well_known_file, client._get_well_known_file())

    @mock.patch.dict(os.environ,
                     {client._CLOUDSDK_CONFIG_ENV_VAR: 'CUSTOM_DIR'},
                     clear=True)
    def test_get_well_known_file_with_custom_config_dir(self):
        CUSTOM_DIR = os.environ[client._CLOUDSDK_CONFIG_ENV_VAR]
        EXPECTED_FILE = os.path.join(CUSTOM_DIR,
                                     client._WELL_KNOWN_CREDENTIALS_FILE)
        well_known_file = client._get_well_known_file()
        self.assertEqual(well_known_file, EXPECTED_FILE)

    def test_get_adc_from_file_service_account(self):
        credentials_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        credentials = client._get_application_default_credential_from_file(
            credentials_file)
        self.validate_service_account_credentials(credentials)

    def test_save_to_well_known_file_service_account(self):
        credential_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        credentials = client._get_application_default_credential_from_file(
            credential_file)
        temp_credential_file = datafile(
            os.path.join('gcloud',
                         'temp_well_known_file_service_account.json'))
        client.save_to_well_known_file(credentials, temp_credential_file)
        with open(temp_credential_file) as f:
            d = json.load(f)
        self.assertEqual('service_account', d['type'])
        self.assertEqual('123', d['client_id'])
        self.assertEqual('dummy@google.com', d['client_email'])
        self.assertEqual('ABCDEF', d['private_key_id'])
        os.remove(temp_credential_file)

    @mock.patch('os.path.isdir', return_value=False)
    def test_save_well_known_file_with_non_existent_config_dir(self,
                                                               isdir_mock):
        credential_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        credentials = client._get_application_default_credential_from_file(
            credential_file)
        with self.assertRaises(OSError):
            client.save_to_well_known_file(credentials)
        config_dir = os.path.join(os.path.expanduser('~'), '.config', 'gcloud')
        isdir_mock.assert_called_once_with(config_dir)

    def test_get_adc_from_file_authorized_user(self):
        credentials_file = datafile(os.path.join(
            'gcloud',
            'application_default_credentials_authorized_user.json'))
        credentials = client._get_application_default_credential_from_file(
            credentials_file)
        self.validate_google_credentials(credentials)

    def test_save_to_well_known_file_authorized_user(self):
        credentials_file = datafile(os.path.join(
            'gcloud',
            'application_default_credentials_authorized_user.json'))
        credentials = client._get_application_default_credential_from_file(
            credentials_file)
        temp_credential_file = datafile(
            os.path.join('gcloud',
                         'temp_well_known_file_authorized_user.json'))
        client.save_to_well_known_file(credentials, temp_credential_file)
        with open(temp_credential_file) as f:
            d = json.load(f)
        self.assertEqual('authorized_user', d['type'])
        self.assertEqual('123', d['client_id'])
        self.assertEqual('secret', d['client_secret'])
        self.assertEqual('alabalaportocala', d['refresh_token'])
        os.remove(temp_credential_file)

    def test_get_application_default_credential_from_malformed_file_1(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_1.json'))
        expected_err_msg = (
            "'type' field should be defined \(and have one of the '{0}' or "
            "'{1}' values\)".format(client.AUTHORIZED_USER,
                                    client.SERVICE_ACCOUNT))

        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            client._get_application_default_credential_from_file(
                credentials_file)

    def test_get_application_default_credential_from_malformed_file_2(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_2.json'))
        expected_err_msg = (
            'The following field\(s\) must be defined: private_key_id')
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            client._get_application_default_credential_from_file(
                credentials_file)

    def test_get_application_default_credential_from_malformed_file_3(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_3.json'))
        with self.assertRaises(ValueError):
            client._get_application_default_credential_from_file(
                credentials_file)

    def test_raise_exception_for_missing_fields(self):
        missing_fields = ['first', 'second', 'third']
        expected_err_msg = ('The following field\(s\) must be defined: ' +
                            ', '.join(missing_fields))
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            client._raise_exception_for_missing_fields(missing_fields)

    def test_raise_exception_for_reading_json(self):
        credential_file = 'any_file'
        extra_help = ' be good'
        error = client.ApplicationDefaultCredentialsError('stuff happens')
        expected_err_msg = ('An error was encountered while reading '
                            'json file: ' + credential_file +
                            extra_help + ': ' + str(error))
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            client._raise_exception_for_reading_json(
                credential_file, extra_help, error)

    @mock.patch('oauth2client.client._in_gce_environment')
    @mock.patch('oauth2client.client._in_gae_environment', return_value=False)
    @mock.patch('oauth2client.client._get_environment_variable_file')
    @mock.patch('oauth2client.client._get_well_known_file')
    def test_get_adc_from_env_var_service_account(self, *stubs):
        # Set up stubs.
        get_well_known, get_env_file, in_gae, in_gce = stubs
        get_env_file.return_value = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))

        credentials = client.GoogleCredentials.get_application_default()
        self.validate_service_account_credentials(credentials)

        get_env_file.assert_called_once_with()
        get_well_known.assert_not_called()
        in_gae.assert_not_called()
        in_gce.assert_not_called()

    def test_env_name(self):
        self.assertEqual(None, client.SETTINGS.env_name)
        self.test_get_adc_from_env_var_service_account()
        self.assertEqual(client.DEFAULT_ENV_NAME, client.SETTINGS.env_name)

    @mock.patch('oauth2client.client._in_gce_environment')
    @mock.patch('oauth2client.client._in_gae_environment', return_value=False)
    @mock.patch('oauth2client.client._get_environment_variable_file')
    @mock.patch('oauth2client.client._get_well_known_file')
    def test_get_adc_from_env_var_authorized_user(self, *stubs):
        # Set up stubs.
        get_well_known, get_env_file, in_gae, in_gce = stubs
        get_env_file.return_value = datafile(os.path.join(
            'gcloud',
            'application_default_credentials_authorized_user.json'))

        credentials = client.GoogleCredentials.get_application_default()
        self.validate_google_credentials(credentials)

        get_env_file.assert_called_once_with()
        get_well_known.assert_not_called()
        in_gae.assert_not_called()
        in_gce.assert_not_called()

    @mock.patch('oauth2client.client._in_gce_environment')
    @mock.patch('oauth2client.client._in_gae_environment', return_value=False)
    @mock.patch('oauth2client.client._get_environment_variable_file')
    @mock.patch('oauth2client.client._get_well_known_file')
    def test_get_adc_from_env_var_malformed_file(self, *stubs):
        # Set up stubs.
        get_well_known, get_env_file, in_gae, in_gce = stubs
        get_env_file.return_value = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_3.json'))

        expected_err = client.ApplicationDefaultCredentialsError
        with self.assertRaises(expected_err) as exc_manager:
            client.GoogleCredentials.get_application_default()

        self.assertTrue(str(exc_manager.exception).startswith(
            'An error was encountered while reading json file: ' +
            get_env_file.return_value + ' (pointed to by ' +
            client.GOOGLE_APPLICATION_CREDENTIALS + ' environment variable):'))

        get_env_file.assert_called_once_with()
        get_well_known.assert_not_called()
        in_gae.assert_not_called()
        in_gce.assert_not_called()

    @mock.patch('oauth2client.client._in_gce_environment', return_value=False)
    @mock.patch('oauth2client.client._in_gae_environment', return_value=False)
    @mock.patch('oauth2client.client._get_environment_variable_file',
                return_value=None)
    @mock.patch('oauth2client.client._get_well_known_file',
                return_value='BOGUS_FILE')
    def test_get_adc_env_not_set_up(self, *stubs):
        # Unpack stubs.
        get_well_known, get_env_file, in_gae, in_gce = stubs
        # Make sure the well-known file actually doesn't exist.
        self.assertFalse(os.path.exists(get_well_known.return_value))

        expected_err = client.ApplicationDefaultCredentialsError
        with self.assertRaises(expected_err) as exc_manager:
            client.GoogleCredentials.get_application_default()

        self.assertEqual(client.ADC_HELP_MSG, str(exc_manager.exception))
        get_env_file.assert_called_once_with()
        get_well_known.assert_called_once_with()
        in_gae.assert_called_once_with()
        in_gce.assert_called_once_with()

    @mock.patch('oauth2client.client._in_gce_environment', return_value=False)
    @mock.patch('oauth2client.client._in_gae_environment', return_value=False)
    @mock.patch('oauth2client.client._get_environment_variable_file',
                return_value=None)
    @mock.patch('oauth2client.client._get_well_known_file')
    def test_get_adc_env_from_well_known(self, *stubs):
        # Unpack stubs.
        get_well_known, get_env_file, in_gae, in_gce = stubs
        # Make sure the well-known file is an actual file.
        get_well_known.return_value = __file__
        # Make sure the well-known file actually doesn't exist.
        self.assertTrue(os.path.exists(get_well_known.return_value))

        method_name = (
            'oauth2client.client.'
            '_get_application_default_credential_from_file')
        result_creds = object()
        with mock.patch(method_name,
                        return_value=result_creds) as get_from_file:
            result = client.GoogleCredentials.get_application_default()
            self.assertEqual(result, result_creds)
            get_from_file.assert_called_once_with(__file__)

        get_env_file.assert_called_once_with()
        get_well_known.assert_called_once_with()
        in_gae.assert_not_called()
        in_gce.assert_not_called()

    def test_from_stream_service_account(self):
        credentials_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        credentials = self.get_a_google_credentials_object().from_stream(
            credentials_file)
        self.validate_service_account_credentials(credentials)

    def test_from_stream_authorized_user(self):
        credentials_file = datafile(os.path.join(
            'gcloud',
            'application_default_credentials_authorized_user.json'))
        credentials = self.get_a_google_credentials_object().from_stream(
            credentials_file)
        self.validate_google_credentials(credentials)

    def test_from_stream_missing_file(self):
        credentials_filename = None
        expected_err_msg = (r'The parameter passed to the from_stream\(\) '
                            r'method should point to a file.')
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            self.get_a_google_credentials_object().from_stream(
                credentials_filename)

    def test_from_stream_malformed_file_1(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_1.json'))
        expected_err_msg = (
            'An error was encountered while reading json file: ' +
            credentials_file +
            ' \(provided as parameter to the from_stream\(\) method\): ' +
            "'type' field should be defined \(and have one of the '" +
            client.AUTHORIZED_USER + "' or '" + client.SERVICE_ACCOUNT +
            "' values\)")
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            self.get_a_google_credentials_object().from_stream(
                credentials_file)

    def test_from_stream_malformed_file_2(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_2.json'))
        expected_err_msg = (
            'An error was encountered while reading json file: ' +
            credentials_file +
            ' \(provided as parameter to the from_stream\(\) method\): '
            'The following field\(s\) must be defined: '
            'private_key_id')
        with self.assertRaisesRegexp(client.ApplicationDefaultCredentialsError,
                                     expected_err_msg):
            self.get_a_google_credentials_object().from_stream(
                credentials_file)

    def test_from_stream_malformed_file_3(self):
        credentials_file = datafile(
            os.path.join('gcloud',
                         'application_default_credentials_malformed_3.json'))
        with self.assertRaises(client.ApplicationDefaultCredentialsError):
            self.get_a_google_credentials_object().from_stream(
                credentials_file)

    def test_to_from_json_authorized_user(self):
        filename = 'application_default_credentials_authorized_user.json'
        credentials_file = datafile(os.path.join('gcloud', filename))
        creds = client.GoogleCredentials.from_stream(credentials_file)
        json = creds.to_json()
        creds2 = client.GoogleCredentials.from_json(json)

        self.assertEqual(creds.__dict__, creds2.__dict__)

    def test_to_from_json_service_account(self):
        credentials_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        creds1 = client.GoogleCredentials.from_stream(credentials_file)
        # Convert to and then back from json.
        creds2 = client.GoogleCredentials.from_json(creds1.to_json())

        creds1_vals = creds1.__dict__
        creds1_vals.pop('_signer')
        creds2_vals = creds2.__dict__
        creds2_vals.pop('_signer')
        self.assertEqual(creds1_vals, creds2_vals)

    def test_to_from_json_service_account_scoped(self):
        credentials_file = datafile(
            os.path.join('gcloud', client._WELL_KNOWN_CREDENTIALS_FILE))
        creds1 = client.GoogleCredentials.from_stream(credentials_file)
        creds1 = creds1.create_scoped(['dummy_scope'])
        # Convert to and then back from json.
        creds2 = client.GoogleCredentials.from_json(creds1.to_json())

        creds1_vals = creds1.__dict__
        creds1_vals.pop('_signer')
        creds2_vals = creds2.__dict__
        creds2_vals.pop('_signer')
        self.assertEqual(creds1_vals, creds2_vals)

    def test_parse_expiry(self):
        dt = datetime.datetime(2016, 1, 1)
        parsed_expiry = client._parse_expiry(dt)
        self.assertEqual('2016-01-01T00:00:00Z', parsed_expiry)

    def test_bad_expiry(self):
        dt = object()
        parsed_expiry = client._parse_expiry(dt)
        self.assertEqual(None, parsed_expiry)


class DummyDeleteStorage(client.Storage):
    delete_called = False

    def locked_delete(self):
        self.delete_called = True


def _token_revoke_test_helper(testcase, revoke_raise, valid_bool_value,
                              token_attr, http_mock):
    current_store = getattr(testcase.credentials, 'store', None)

    dummy_store = DummyDeleteStorage()
    testcase.credentials.set_store(dummy_store)

    actual_do_revoke = testcase.credentials._do_revoke
    testcase.token_from_revoke = None

    def do_revoke_stub(http, token):
        testcase.token_from_revoke = token
        return actual_do_revoke(http, token)
    testcase.credentials._do_revoke = do_revoke_stub

    if revoke_raise:
        testcase.assertRaises(client.TokenRevokeError,
                              testcase.credentials.revoke, http_mock)
    else:
        testcase.credentials.revoke(http_mock)

    testcase.assertEqual(getattr(testcase.credentials, token_attr),
                         testcase.token_from_revoke)
    testcase.assertEqual(valid_bool_value, testcase.credentials.invalid)
    testcase.assertEqual(valid_bool_value, dummy_store.delete_called)

    testcase.credentials.set_store(current_store)


class BasicCredentialsTests(unittest.TestCase):

    def setUp(self):
        access_token = 'foo'
        client_id = 'some_client_id'
        client_secret = 'cOuDdkfjxxnv+'
        refresh_token = '1/0/a.df219fjls0'
        token_expiry = datetime.datetime.utcnow()
        user_agent = 'refresh_checker/1.0'
        self.credentials = client.OAuth2Credentials(
            access_token, client_id, client_secret,
            refresh_token, token_expiry, oauth2client.GOOGLE_TOKEN_URI,
            user_agent, revoke_uri=oauth2client.GOOGLE_REVOKE_URI,
            scopes='foo', token_info_uri=oauth2client.GOOGLE_TOKEN_INFO_URI)

        # Provoke a failure if @_helpers.positional is not respected.
        self.old_positional_enforcement = (
            _helpers.positional_parameters_enforcement)
        _helpers.positional_parameters_enforcement = (
            _helpers.POSITIONAL_EXCEPTION)

    def tearDown(self):
        _helpers.positional_parameters_enforcement = (
            self.old_positional_enforcement)

    def test_token_refresh_success(self):
        for status_code in client.REFRESH_STATUS_CODES:
            token_response = {'access_token': '1/3w', 'expires_in': 3600}
            json_resp = json.dumps(token_response).encode('utf-8')
            http = http_mock.HttpMockSequence([
                ({'status': status_code}, b''),
                ({'status': http_client.OK}, json_resp),
                ({'status': http_client.OK}, 'echo_request_headers'),
            ])
            http = self.credentials.authorize(http)
            resp, content = transport.request(http, 'http://example.com')
            self.assertEqual(b'Bearer 1/3w', content[b'Authorization'])
            self.assertFalse(self.credentials.access_token_expired)
            self.assertEqual(token_response, self.credentials.token_response)

    def test_recursive_authorize(self):
        # Tests that OAuth2Credentials doesn't introduce new method
        # constraints. Formerly, OAuth2Credentials.authorize monkeypatched the
        # request method of the passed in HTTP object with a wrapper annotated
        # with @_helpers.positional(1). Since the original method has no such
        # annotation, that meant that the wrapper was violating the contract of
        # the original method by adding a new requirement to it. And in fact
        # the wrapper itself doesn't even respect that requirement. So before
        # the removal of the annotation, this test would fail.
        token_response = {'access_token': '1/3w', 'expires_in': 3600}
        encoded_response = json.dumps(token_response).encode('utf-8')
        http = http_mock.HttpMock(data=encoded_response)
        http = self.credentials.authorize(http)
        http = self.credentials.authorize(http)
        transport.request(http, 'http://example.com')

    def test_token_refresh_failure(self):
        for status_code in client.REFRESH_STATUS_CODES:
            http = http_mock.HttpMockSequence([
                ({'status': status_code}, b''),
                ({'status': http_client.BAD_REQUEST},
                 b'{"error":"access_denied"}'),
            ])
            http = self.credentials.authorize(http)
            with self.assertRaises(
                    client.HttpAccessTokenRefreshError) as exc_manager:
                transport.request(http, 'http://example.com')
            self.assertEqual(http_client.BAD_REQUEST,
                             exc_manager.exception.status)
            self.assertTrue(self.credentials.access_token_expired)
            self.assertEqual(None, self.credentials.token_response)

    def test_token_revoke_success(self):
        http = http_mock.HttpMock(headers={'status': http_client.OK})
        _token_revoke_test_helper(
            self, revoke_raise=False, valid_bool_value=True,
            token_attr='refresh_token', http_mock=http)

    def test_token_revoke_failure(self):
        http = http_mock.HttpMock(headers={'status': http_client.BAD_REQUEST})
        _token_revoke_test_helper(
            self, revoke_raise=True, valid_bool_value=False,
            token_attr='refresh_token', http_mock=http)

    def test_token_revoke_fallback(self):
        original_credentials = self.credentials.to_json()
        self.credentials.refresh_token = None

        http = http_mock.HttpMock(headers={'status': http_client.OK})
        _token_revoke_test_helper(
            self, revoke_raise=False, valid_bool_value=True,
            token_attr='access_token', http_mock=http)
        self.credentials = self.credentials.from_json(original_credentials)

    def test_token_revoke_405(self):
        original_credentials = self.credentials.to_json()
        self.credentials.refresh_token = None

        http = http_mock.HttpMockSequence([
            ({'status': http_client.METHOD_NOT_ALLOWED}, b''),
            ({'status': http_client.OK}, b''),
        ])
        _token_revoke_test_helper(
            self, revoke_raise=False, valid_bool_value=True,
            token_attr='access_token', http_mock=http)
        self.credentials = self.credentials.from_json(original_credentials)

    def test_non_401_error_response(self):
        http = http_mock.HttpMock(headers={'status': http_client.BAD_REQUEST})
        http = self.credentials.authorize(http)
        resp, content = transport.request(http, 'http://example.com')
        self.assertEqual(http_client.BAD_REQUEST, resp.status)
        self.assertEqual(None, self.credentials.token_response)

    def test_to_from_json(self):
        json = self.credentials.to_json()
        instance = client.OAuth2Credentials.from_json(json)
        self.assertEqual(client.OAuth2Credentials, type(instance))
        instance.token_expiry = None
        self.credentials.token_expiry = None

        self.assertEqual(instance.__dict__, self.credentials.__dict__)

    def test_from_json_token_expiry(self):
        data = json.loads(self.credentials.to_json())
        data['token_expiry'] = None
        instance = client.OAuth2Credentials.from_json(json.dumps(data))
        self.assertIsInstance(instance, client.OAuth2Credentials)

    def test_from_json_bad_token_expiry(self):
        data = json.loads(self.credentials.to_json())
        data['token_expiry'] = 'foobar'
        instance = client.OAuth2Credentials.from_json(json.dumps(data))
        self.assertIsInstance(instance, client.OAuth2Credentials)

    def test_unicode_header_checks(self):
        access_token = u'foo'
        client_id = u'some_client_id'
        client_secret = u'cOuDdkfjxxnv+'
        refresh_token = u'1/0/a.df219fjls0'
        token_expiry = str(datetime.datetime.utcnow())
        token_uri = str(oauth2client.GOOGLE_TOKEN_URI)
        revoke_uri = str(oauth2client.GOOGLE_REVOKE_URI)
        user_agent = u'refresh_checker/1.0'
        credentials = client.OAuth2Credentials(
            access_token, client_id, client_secret, refresh_token,
            token_expiry, token_uri, user_agent, revoke_uri=revoke_uri)

        # First, test that we correctly encode basic objects, making sure
        # to include a bytes object. Note that oauth2client will normalize
        # everything to bytes, no matter what python version we're in.
        http = credentials.authorize(http_mock.HttpMock())
        headers = {u'foo': 3, b'bar': True, 'baz': b'abc'}
        cleaned_headers = {b'foo': b'3', b'bar': b'True', b'baz': b'abc'}
        transport.request(
            http, u'http://example.com', method=u'GET', headers=headers)
        for k, v in cleaned_headers.items():
            self.assertTrue(k in http.headers)
            self.assertEqual(v, http.headers[k])

        # Next, test that we do fail on unicode.
        unicode_str = six.unichr(40960) + 'abcd'
        with self.assertRaises(client.NonAsciiHeaderError):
            transport.request(
                http, u'http://example.com', method=u'GET',
                headers={u'foo': unicode_str})

    def test_no_unicode_in_request_params(self):
        access_token = u'foo'
        client_id = u'some_client_id'
        client_secret = u'cOuDdkfjxxnv+'
        refresh_token = u'1/0/a.df219fjls0'
        token_expiry = str(datetime.datetime.utcnow())
        token_uri = str(oauth2client.GOOGLE_TOKEN_URI)
        revoke_uri = str(oauth2client.GOOGLE_REVOKE_URI)
        user_agent = u'refresh_checker/1.0'
        credentials = client.OAuth2Credentials(
            access_token, client_id, client_secret, refresh_token,
            token_expiry, token_uri, user_agent, revoke_uri=revoke_uri)

        http = http_mock.HttpMock()
        http = credentials.authorize(http)
        transport.request(
            http, u'http://example.com', method=u'GET',
            headers={u'foo': u'bar'})
        for k, v in six.iteritems(http.headers):
            self.assertIsInstance(k, six.binary_type)
            self.assertIsInstance(v, six.binary_type)

        # Test again with unicode strings that can't simply be converted
        # to ASCII.
        with self.assertRaises(client.NonAsciiHeaderError):
            transport.request(
                http, u'http://example.com', method=u'GET',
                headers={u'foo': u'\N{COMET}'})

        self.credentials.token_response = 'foobar'
        instance = client.OAuth2Credentials.from_json(
            self.credentials.to_json())
        self.assertEqual('foobar', instance.token_response)

    def test__expires_in_no_expiry(self):
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, None, None)
        self.assertIsNone(credentials.token_expiry)
        self.assertIsNone(credentials._expires_in())

    @mock.patch('oauth2client.client._UTCNOW')
    def test__expires_in_expired(self, utcnow):
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, None, None)
        credentials.token_expiry = datetime.datetime.utcnow()
        now = credentials.token_expiry + datetime.timedelta(seconds=1)
        self.assertLess(credentials.token_expiry, now)
        utcnow.return_value = now
        self.assertEqual(credentials._expires_in(), 0)
        utcnow.assert_called_once_with()

    @mock.patch('oauth2client.client._UTCNOW')
    def test__expires_in_not_expired(self, utcnow):
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, None, None)
        credentials.token_expiry = datetime.datetime.utcnow()
        seconds = 1234
        now = credentials.token_expiry - datetime.timedelta(seconds=seconds)
        self.assertLess(now, credentials.token_expiry)
        utcnow.return_value = now
        self.assertEqual(credentials._expires_in(), seconds)
        utcnow.assert_called_once_with()

    @mock.patch('oauth2client.client._UTCNOW')
    def test_get_access_token(self, utcnow):
        # Configure the patch.
        seconds = 11
        NOW = datetime.datetime(1992, 12, 31, second=seconds)
        utcnow.return_value = NOW

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
            ({'status': '200'}, json.dumps(token_response_first).encode(
                'utf-8')),
            ({'status': '200'}, json.dumps(token_response_second).encode(
                'utf-8')),
        ])

        # Use the current credentials but unset the expiry and
        # the access token.
        credentials = copy.deepcopy(self.credentials)
        credentials.access_token = None
        credentials.token_expiry = None

        # Get Access Token, First attempt.
        self.assertEqual(credentials.access_token, None)
        self.assertFalse(credentials.access_token_expired)
        self.assertEqual(credentials.token_expiry, None)
        token = credentials.get_access_token(http=http)
        self.assertEqual(credentials.token_expiry, EXPIRY_TIME)
        self.assertEqual(token1, token.access_token)
        self.assertEqual(lifetime, token.expires_in)
        self.assertEqual(token_response_first, credentials.token_response)
        # Two utcnow calls are expected:
        # - get_access_token() -> _do_refresh_request (setting expires in)
        # - get_access_token() -> _expires_in()
        expected_utcnow_calls = [mock.call()] * 2
        self.assertEqual(expected_utcnow_calls, utcnow.mock_calls)

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

    @mock.patch.object(client.OAuth2Credentials, 'refresh')
    @mock.patch.object(client.OAuth2Credentials, '_expires_in',
                       return_value=1835)
    def test_get_access_token_without_http(self, expires_in, refresh_mock):
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, None, None)
        # Make sure access_token_expired returns True
        credentials.invalid = True
        # Specify a token so we can use it in the response.
        credentials.access_token = 'ya29-s3kr3t'

        with mock.patch('oauth2client.transport.get_http_object',
                        return_value=object()) as new_http:
            token_info = credentials.get_access_token()
            expires_in.assert_called_once_with()
            refresh_mock.assert_called_once_with(new_http.return_value)
            new_http.assert_called_once_with()

        self.assertIsInstance(token_info, client.AccessTokenInfo)
        self.assertEqual(token_info.access_token,
                         credentials.access_token)
        self.assertEqual(token_info.expires_in,
                         expires_in.return_value)

    @mock.patch.object(client.OAuth2Credentials, 'refresh')
    @mock.patch.object(client.OAuth2Credentials, '_expires_in',
                       return_value=1835)
    def test_get_access_token_with_http(self, expires_in, refresh_mock):
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, None, None)
        # Make sure access_token_expired returns True
        credentials.invalid = True
        # Specify a token so we can use it in the response.
        credentials.access_token = 'ya29-s3kr3t'

        http_obj = object()
        token_info = credentials.get_access_token(http_obj)
        self.assertIsInstance(token_info, client.AccessTokenInfo)
        self.assertEqual(token_info.access_token,
                         credentials.access_token)
        self.assertEqual(token_info.expires_in,
                         expires_in.return_value)

        expires_in.assert_called_once_with()
        refresh_mock.assert_called_once_with(http_obj)

    @mock.patch.object(client.OAuth2Credentials,
                       '_generate_refresh_request_headers',
                       return_value=object())
    @mock.patch.object(client.OAuth2Credentials,
                       '_generate_refresh_request_body',
                       return_value=object())
    @mock.patch('oauth2client.client.logger')
    def _do_refresh_request_test_helper(self, response, content,
                                        error_msg, logger, gen_body,
                                        gen_headers, store=None):
        token_uri = 'http://token_uri'
        credentials = client.OAuth2Credentials(None, None, None, None,
                                               None, token_uri, None)
        credentials.store = store
        http = http_mock.HttpMock(headers=response, data=content)

        with self.assertRaises(
                client.HttpAccessTokenRefreshError) as exc_manager:
            credentials._do_refresh_request(http)

        self.assertEqual(exc_manager.exception.args, (error_msg,))
        self.assertEqual(exc_manager.exception.status, response.status)

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, token_uri)
        self.assertEqual(http.method, 'POST')
        self.assertEqual(http.body, gen_body.return_value)
        self.assertEqual(http.headers, gen_headers.return_value)

        call1 = mock.call('Refreshing access_token')
        failure_template = 'Failed to retrieve access token: %s'
        call2 = mock.call(failure_template, content)
        self.assertEqual(logger.info.mock_calls, [call1, call2])
        if store is not None:
            store.locked_put.assert_called_once_with(credentials)

    def test__do_refresh_request_non_json_failure(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_REQUEST})
        content = u'Bad request'
        error_msg = 'Invalid response {0}.'.format(int(response.status))
        self._do_refresh_request_test_helper(response, content, error_msg)

    def test__do_refresh_request_basic_failure(self):
        response = http_mock.ResponseMock(
            {'status': http_client.INTERNAL_SERVER_ERROR})
        content = u'{}'
        error_msg = 'Invalid response {0}.'.format(int(response.status))
        self._do_refresh_request_test_helper(response, content, error_msg)

    def test__do_refresh_request_failure_w_json_error(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_GATEWAY})
        error_msg = 'Hi I am an error not a bearer'
        content = json.dumps({'error': error_msg})
        self._do_refresh_request_test_helper(response, content, error_msg)

    def test__do_refresh_request_failure_w_json_error_and_store(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_GATEWAY})
        error_msg = 'Where are we going wearer?'
        content = json.dumps({'error': error_msg})
        store = mock.Mock()
        self._do_refresh_request_test_helper(response, content, error_msg,
                                             store=store)

    def test__do_refresh_request_failure_w_json_error_and_desc(self):
        response = http_mock.ResponseMock(
            {'status': http_client.SERVICE_UNAVAILABLE})
        base_error = 'Ruckus'
        error_desc = 'Can you describe the ruckus'
        content = json.dumps({
            'error': base_error,
            'error_description': error_desc,
        })
        error_msg = '{0}: {1}'.format(base_error, error_desc)
        self._do_refresh_request_test_helper(response, content, error_msg)

    @mock.patch('oauth2client.client.logger')
    def _do_revoke_test_helper(self, response, content,
                               error_msg, logger, store=None):
        credentials = client.OAuth2Credentials(
            None, None, None, None, None, None, None,
            revoke_uri=oauth2client.GOOGLE_REVOKE_URI)
        credentials.store = store

        http = http_mock.HttpMock(headers=response, data=content)
        token = u's3kr3tz'

        if response.status == http_client.OK:
            self.assertFalse(credentials.invalid)
            self.assertIsNone(credentials._do_revoke(http, token))
            self.assertTrue(credentials.invalid)
            if store is not None:
                store.delete.assert_called_once_with()
        else:
            self.assertFalse(credentials.invalid)
            with self.assertRaises(client.TokenRevokeError) as exc_manager:
                credentials._do_revoke(http, token)
            # Make sure invalid was not flipped on.
            self.assertFalse(credentials.invalid)
            self.assertEqual(exc_manager.exception.args, (error_msg,))
            if store is not None:
                store.delete.assert_not_called()

        revoke_uri = oauth2client.GOOGLE_REVOKE_URI + '?token=' + token

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        self.assertEqual(http.uri, revoke_uri)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertIsNone(http.headers)

        logger.info.assert_called_once_with('Revoking token')

    def test__do_revoke_success(self):
        response = http_mock.ResponseMock()
        self._do_revoke_test_helper(response, b'', None)

    def test__do_revoke_success_with_store(self):
        response = http_mock.ResponseMock()
        store = mock.Mock()
        self._do_revoke_test_helper(response, b'', None, store=store)

    def test__do_revoke_non_json_failure(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_REQUEST})
        content = u'Bad request'
        error_msg = 'Invalid response {0}.'.format(response.status)
        self._do_revoke_test_helper(response, content, error_msg)

    def test__do_revoke_basic_failure(self):
        response = http_mock.ResponseMock(
            {'status': http_client.INTERNAL_SERVER_ERROR})
        content = u'{}'
        error_msg = 'Invalid response {0}.'.format(response.status)
        self._do_revoke_test_helper(response, content, error_msg)

    def test__do_revoke_failure_w_json_error(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_GATEWAY})
        error_msg = 'Hi I am an error not a bearer'
        content = json.dumps({'error': error_msg})
        self._do_revoke_test_helper(response, content, error_msg)

    def test__do_revoke_failure_w_json_error_and_store(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_GATEWAY})
        error_msg = 'Where are we going wearer?'
        content = json.dumps({'error': error_msg})
        store = mock.Mock()
        self._do_revoke_test_helper(response, content, error_msg,
                                    store=store)

    @mock.patch('oauth2client.client.logger')
    def _do_retrieve_scopes_test_helper(self, response, content,
                                        error_msg, logger, scopes=None):
        credentials = client.OAuth2Credentials(
            None, None, None, None, None, None, None,
            token_info_uri=oauth2client.GOOGLE_TOKEN_INFO_URI)
        http = http_mock.HttpMock(headers=response, data=content)
        token = u's3kr3tz'

        if response.status == http_client.OK:
            self.assertEqual(credentials.scopes, set())
            self.assertIsNone(
                credentials._do_retrieve_scopes(http, token))
            self.assertEqual(credentials.scopes, scopes)
        else:
            self.assertEqual(credentials.scopes, set())
            with self.assertRaises(client.Error) as exc_manager:
                credentials._do_retrieve_scopes(http, token)
            # Make sure scopes were not changed.
            self.assertEqual(credentials.scopes, set())
            self.assertEqual(exc_manager.exception.args, (error_msg,))

        token_uri = _helpers.update_query_params(
            oauth2client.GOOGLE_TOKEN_INFO_URI,
            {'fields': 'scope', 'access_token': token})

        # Verify mocks.
        self.assertEqual(http.requests, 1)
        assertUrisEqual(self, token_uri, http.uri)
        self.assertEqual(http.method, 'GET')
        self.assertIsNone(http.body)
        self.assertIsNone(http.headers)
        logger.info.assert_called_once_with('Refreshing scopes')

    def test__do_retrieve_scopes_success_bad_json(self):
        response = http_mock.ResponseMock()
        invalid_json = b'{'
        with self.assertRaises(ValueError):
            self._do_retrieve_scopes_test_helper(response, invalid_json, None)

    def test__do_retrieve_scopes_success(self):
        response = http_mock.ResponseMock()
        content = b'{"scope": "foo bar"}'
        self._do_retrieve_scopes_test_helper(response, content, None,
                                             scopes=set(['foo', 'bar']))

    def test__do_retrieve_scopes_non_json_failure(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_REQUEST})
        content = u'Bad request'
        error_msg = 'Invalid response {0}.'.format(response.status)
        self._do_retrieve_scopes_test_helper(response, content, error_msg)

    def test__do_retrieve_scopes_basic_failure(self):
        response = http_mock.ResponseMock(
            {'status': http_client.INTERNAL_SERVER_ERROR})
        content = u'{}'
        error_msg = 'Invalid response {0}.'.format(response.status)
        self._do_retrieve_scopes_test_helper(response, content, error_msg)

    def test__do_retrieve_scopes_failure_w_json_error(self):
        response = http_mock.ResponseMock({'status': http_client.BAD_GATEWAY})
        error_msg = 'Error desc I sit at a desk'
        content = json.dumps({'error_description': error_msg})
        self._do_retrieve_scopes_test_helper(response, content, error_msg)

    def test_has_scopes(self):
        self.assertTrue(self.credentials.has_scopes('foo'))
        self.assertTrue(self.credentials.has_scopes(['foo']))
        self.assertFalse(self.credentials.has_scopes('bar'))
        self.assertFalse(self.credentials.has_scopes(['bar']))

        self.credentials.scopes = set(['foo', 'bar'])
        self.assertTrue(self.credentials.has_scopes('foo'))
        self.assertTrue(self.credentials.has_scopes('bar'))
        self.assertFalse(self.credentials.has_scopes('baz'))
        self.assertTrue(self.credentials.has_scopes(['foo', 'bar']))
        self.assertFalse(self.credentials.has_scopes(['foo', 'baz']))

        self.credentials.scopes = set([])
        self.assertFalse(self.credentials.has_scopes('foo'))

    def test_retrieve_scopes(self):
        info_response_first = {'scope': 'foo bar'}
        info_response_second = {'error_description': 'abcdef'}
        http = http_mock.HttpMockSequence([
            ({'status': '200'}, json.dumps(info_response_first).encode(
                'utf-8')),
            ({'status': '400'}, json.dumps(info_response_second).encode(
                'utf-8')),
            ({'status': '500'}, b''),
        ])

        self.credentials.retrieve_scopes(http)
        self.assertEqual(set(['foo', 'bar']), self.credentials.scopes)

        with self.assertRaises(client.Error):
            self.credentials.retrieve_scopes(http)

        with self.assertRaises(client.Error):
            self.credentials.retrieve_scopes(http)

    def test_refresh_updates_id_token(self):
        for status_code in client.REFRESH_STATUS_CODES:
            body = {'foo': 'bar'}
            body_json = json.dumps(body).encode('ascii')
            payload = base64.urlsafe_b64encode(body_json).strip(b'=')
            jwt = b'stuff.' + payload + b'.signature'

            token_response = (b'{'
                              b'  "access_token":"1/3w",'
                              b'  "expires_in":3600,'
                              b'  "id_token": "' + jwt + b'"'
                              b'}')
            http = http_mock.HttpMockSequence([
                ({'status': status_code}, b''),
                ({'status': '200'}, token_response),
                ({'status': '200'}, 'echo_request_headers'),
            ])
            http = self.credentials.authorize(http)
            resp, content = transport.request(http, 'http://example.com')
            self.assertEqual(self.credentials.id_token, body)
            self.assertEqual(self.credentials.id_token_jwt, jwt.decode())


class AccessTokenCredentialsTests(unittest.TestCase):

    def setUp(self):
        access_token = 'foo'
        user_agent = 'refresh_checker/1.0'
        self.credentials = client.AccessTokenCredentials(
            access_token, user_agent,
            revoke_uri=oauth2client.GOOGLE_REVOKE_URI)

    def test_token_refresh_success(self):
        for status_code in client.REFRESH_STATUS_CODES:
            http = http_mock.HttpMock(
                headers={'status': status_code}, data=b'')
            http = self.credentials.authorize(http)
            with self.assertRaises(client.AccessTokenCredentialsError):
                resp, content = transport.request(http, 'http://example.com')

    def test_token_revoke_success(self):
        http = http_mock.HttpMock(headers={'status': http_client.OK})
        _token_revoke_test_helper(
            self, revoke_raise=False, valid_bool_value=True,
            token_attr='access_token', http_mock=http)

    def test_token_revoke_failure(self):
        http = http_mock.HttpMock(headers={'status': http_client.BAD_REQUEST})
        _token_revoke_test_helper(
            self, revoke_raise=True, valid_bool_value=False,
            token_attr='access_token', http_mock=http)

    def test_non_401_error_response(self):
        http = http_mock.HttpMock(headers={'status': http_client.BAD_REQUEST})
        http = self.credentials.authorize(http)
        resp, content = transport.request(http, 'http://example.com')
        self.assertEqual(http_client.BAD_REQUEST, resp.status)

    def test_auth_header_sent(self):
        http = http_mock.HttpMockSequence([
            ({'status': '200'}, 'echo_request_headers'),
        ])
        http = self.credentials.authorize(http)
        resp, content = transport.request(http, 'http://example.com')
        self.assertEqual(b'Bearer foo', content[b'Authorization'])


class TestAssertionCredentials(unittest.TestCase):
    assertion_text = 'This is the assertion'
    assertion_type = 'http://www.google.com/assertionType'

    class AssertionCredentialsTestImpl(client.AssertionCredentials):

        def _generate_assertion(self):
            return TestAssertionCredentials.assertion_text

    def setUp(self):
        user_agent = 'fun/2.0'
        self.credentials = self.AssertionCredentialsTestImpl(
            self.assertion_type, user_agent=user_agent)

    def test__generate_assertion_abstract(self):
        credentials = client.AssertionCredentials(None)
        with self.assertRaises(NotImplementedError):
            credentials._generate_assertion()

    def test_assertion_body(self):
        body = urllib.parse.parse_qs(
            self.credentials._generate_refresh_request_body())
        self.assertEqual(self.assertion_text, body['assertion'][0])
        self.assertEqual('urn:ietf:params:oauth:grant-type:jwt-bearer',
                         body['grant_type'][0])

    def test_assertion_refresh(self):
        http = http_mock.HttpMockSequence([
            ({'status': '200'}, b'{"access_token":"1/3w"}'),
            ({'status': '200'}, 'echo_request_headers'),
        ])
        http = self.credentials.authorize(http)
        resp, content = transport.request(http, 'http://example.com')
        self.assertEqual(b'Bearer 1/3w', content[b'Authorization'])

    def test_token_revoke_success(self):
        http = http_mock.HttpMock(headers={'status': http_client.OK})
        _token_revoke_test_helper(
            self, revoke_raise=False, valid_bool_value=True,
            token_attr='access_token', http_mock=http)

    def test_token_revoke_failure(self):
        http = http_mock.HttpMock(headers={'status': http_client.BAD_REQUEST})
        _token_revoke_test_helper(
            self, revoke_raise=True, valid_bool_value=False,
            token_attr='access_token', http_mock=http)

    def test_sign_blob_abstract(self):
        credentials = client.AssertionCredentials(None)
        with self.assertRaises(NotImplementedError):
            credentials.sign_blob(b'blob')


class ExtractIdTokenTest(unittest.TestCase):
    """Tests client._extract_id_token()."""

    def test_extract_success(self):
        body = {'foo': 'bar'}
        body_json = json.dumps(body).encode('ascii')
        payload = base64.urlsafe_b64encode(body_json).strip(b'=')
        jwt = b'stuff.' + payload + b'.signature'

        extracted = client._extract_id_token(jwt)
        self.assertEqual(extracted, body)

    def test_extract_failure(self):
        body = {'foo': 'bar'}
        body_json = json.dumps(body).encode('ascii')
        payload = base64.urlsafe_b64encode(body_json).strip(b'=')
        jwt = b'stuff.' + payload
        with self.assertRaises(client.VerifyJwtTokenError):
            client._extract_id_token(jwt)


class OAuth2WebServerFlowTest(unittest.TestCase):

    def setUp(self):
        self.flow = client.OAuth2WebServerFlow(
            client_id='client_id+1',
            client_secret='secret+1',
            scope='foo',
            redirect_uri=client.OOB_CALLBACK_URN,
            user_agent='unittest-sample/1.0',
            revoke_uri='dummy_revoke_uri',
        )
        self.bad_verifier = b'__NOT_THE_VERIFIER_YOURE_LOOKING_FOR__'
        self.good_verifier = b'__TEST_VERIFIER__'
        self.good_challenger = b'__TEST_CHALLENGE__'

    def test_construct_authorize_url(self):
        authorize_url = self.flow.step1_get_authorize_url(state='state+1')

        parsed = urllib.parse.urlparse(authorize_url)
        q = urllib.parse.parse_qs(parsed[4])
        self.assertEqual('client_id+1', q['client_id'][0])
        self.assertEqual('code', q['response_type'][0])
        self.assertEqual('foo', q['scope'][0])
        self.assertEqual(client.OOB_CALLBACK_URN, q['redirect_uri'][0])
        self.assertEqual('offline', q['access_type'][0])
        self.assertEqual('state+1', q['state'][0])

    def test_override_flow_via_kwargs(self):
        """Passing kwargs to override defaults."""
        flow = client.OAuth2WebServerFlow(
            client_id='client_id+1',
            client_secret='secret+1',
            scope='foo',
            redirect_uri=client.OOB_CALLBACK_URN,
            user_agent='unittest-sample/1.0',
            access_type='online',
            response_type='token'
        )
        authorize_url = flow.step1_get_authorize_url()

        parsed = urllib.parse.urlparse(authorize_url)
        q = urllib.parse.parse_qs(parsed[4])
        self.assertEqual('client_id+1', q['client_id'][0])
        self.assertEqual('token', q['response_type'][0])
        self.assertEqual('foo', q['scope'][0])
        self.assertEqual(client.OOB_CALLBACK_URN, q['redirect_uri'][0])
        self.assertEqual('online', q['access_type'][0])

    def test__oauth2_web_server_flow_params(self):
        params = client._oauth2_web_server_flow_params({})
        self.assertEqual(params['access_type'], 'offline')
        self.assertEqual(params['response_type'], 'code')

        params = client._oauth2_web_server_flow_params({
            'approval_prompt': 'force'})
        self.assertEqual(params['prompt'], 'consent')
        self.assertNotIn('approval_prompt', params)

        params = client._oauth2_web_server_flow_params({
            'approval_prompt': 'other'})
        self.assertEqual(params['approval_prompt'], 'other')

    @mock.patch('oauth2client.client.logger')
    def test_step1_get_authorize_url_redirect_override(self, logger):
        flow = client.OAuth2WebServerFlow('client_id+1', scope='foo',
                                          redirect_uri=client.OOB_CALLBACK_URN)
        alt_redirect = 'foo:bar'
        self.assertEqual(flow.redirect_uri, client.OOB_CALLBACK_URN)
        result = flow.step1_get_authorize_url(redirect_uri=alt_redirect)
        # Make sure the redirect value was updated.
        self.assertEqual(flow.redirect_uri, alt_redirect)
        query_params = {
            'client_id': flow.client_id,
            'redirect_uri': alt_redirect,
            'scope': flow.scope,
            'access_type': 'offline',
            'response_type': 'code',
        }
        expected = _helpers.update_query_params(flow.auth_uri, query_params)
        assertUrisEqual(self, expected, result)
        # Check stubs.
        self.assertEqual(logger.warning.call_count, 1)

    @mock.patch('oauth2client.client._pkce.code_challenge')
    @mock.patch('oauth2client.client._pkce.code_verifier')
    def test_step1_get_authorize_url_pkce(self, fake_verifier, fake_challenge):
        fake_verifier.return_value = self.good_verifier
        fake_challenge.return_value = self.good_challenger
        flow = client.OAuth2WebServerFlow(
            'client_id+1',
            scope='foo',
            redirect_uri='http://example.com',
            pkce=True)
        auth_url = urllib.parse.urlparse(flow.step1_get_authorize_url())
        self.assertEqual(flow.code_verifier, self.good_verifier)
        results = dict(urllib.parse.parse_qsl(auth_url.query))
        self.assertEqual(
            results['code_challenge'], self.good_challenger.decode())
        self.assertEqual(results['code_challenge_method'], 'S256')
        fake_verifier.assert_called()
        fake_challenge.assert_called_with(self.good_verifier)

    @mock.patch('oauth2client.client._pkce.code_challenge')
    @mock.patch('oauth2client.client._pkce.code_verifier')
    def test_step1_get_authorize_url_pkce_invalid_verifier(
            self, fake_verifier, fake_challenge):
        fake_verifier.return_value = self.good_verifier
        fake_challenge.return_value = self.good_challenger
        flow = client.OAuth2WebServerFlow(
            'client_id+1',
            scope='foo',
            redirect_uri='http://example.com',
            pkce=True,
            code_verifier=self.bad_verifier)
        auth_url = urllib.parse.urlparse(flow.step1_get_authorize_url())
        self.assertEqual(flow.code_verifier, self.bad_verifier)
        results = dict(urllib.parse.parse_qsl(auth_url.query))
        self.assertEqual(
            results['code_challenge'], self.good_challenger.decode())
        self.assertEqual(results['code_challenge_method'], 'S256')
        fake_verifier.assert_not_called()
        fake_challenge.assert_called_with(self.bad_verifier)

    def test_step1_get_authorize_url_without_redirect(self):
        flow = client.OAuth2WebServerFlow('client_id+1', scope='foo',
                                          redirect_uri=None)
        with self.assertRaises(ValueError):
            flow.step1_get_authorize_url(redirect_uri=None)

    def test_step1_get_authorize_url_without_login_hint(self):
        login_hint = 'There are wascally wabbits nearby'
        flow = client.OAuth2WebServerFlow('client_id+1', scope='foo',
                                          redirect_uri=client.OOB_CALLBACK_URN,
                                          login_hint=login_hint)
        result = flow.step1_get_authorize_url()
        query_params = {
            'client_id': flow.client_id,
            'login_hint': login_hint,
            'redirect_uri': client.OOB_CALLBACK_URN,
            'scope': flow.scope,
            'access_type': 'offline',
            'response_type': 'code',
        }
        expected = _helpers.update_query_params(flow.auth_uri, query_params)
        assertUrisEqual(self, expected, result)

    def test_step1_get_device_and_user_codes_wo_device_uri(self):
        flow = client.OAuth2WebServerFlow('CID', scope='foo', device_uri=None)
        with self.assertRaises(ValueError):
            flow.step1_get_device_and_user_codes()

    def _step1_get_device_and_user_codes_helper(
            self, extra_headers=None, user_agent=None, default_http=False,
            content=None):
        flow = client.OAuth2WebServerFlow('CID', scope='foo',
                                          user_agent=user_agent)
        device_code = 'bfc06756-062e-430f-9f0f-460ca44724e5'
        user_code = '5faf2780-fc83-11e5-9bc2-00c2c63e5792'
        ver_url = 'http://foo.bar'
        if content is None:
            content = json.dumps({
                'device_code': device_code,
                'user_code': user_code,
                'verification_url': ver_url,
            })
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, content),
        ])
        if default_http:
            with mock.patch('oauth2client.transport.get_http_object',
                            return_value=http) as new_http:
                result = flow.step1_get_device_and_user_codes()
                # Check the mock was called.
                new_http.assert_called_once_with()
        else:
            result = flow.step1_get_device_and_user_codes(http=http)

        expected = client.DeviceFlowInfo(
            device_code, user_code, None, ver_url, None)
        self.assertEqual(result, expected)
        self.assertEqual(len(http.requests), 1)
        info = http.requests[0]
        self.assertEqual(info['uri'], oauth2client.GOOGLE_DEVICE_URI)
        expected_body = {
            'client_id': [flow.client_id],
            'scope': [flow.scope],
        }
        self.assertEqual(urllib.parse.parse_qs(info['body']), expected_body)
        headers = {'content-type': 'application/x-www-form-urlencoded'}
        if extra_headers is not None:
            headers.update(extra_headers)
        self.assertEqual(info['headers'], headers)

    def test_step1_get_device_and_user_codes(self):
        self._step1_get_device_and_user_codes_helper()

    def test_step1_get_device_and_user_codes_w_user_agent(self):
        user_agent = 'spiderman'
        extra_headers = {'user-agent': user_agent}
        self._step1_get_device_and_user_codes_helper(
            user_agent=user_agent, extra_headers=extra_headers)

    def test_step1_get_device_and_user_codes_w_default_http(self):
        self._step1_get_device_and_user_codes_helper(default_http=True)

    def test_step1_get_device_and_user_codes_bad_payload(self):
        non_json_content = b'{'
        with self.assertRaises(client.OAuth2DeviceCodeError):
            self._step1_get_device_and_user_codes_helper(
                content=non_json_content)

    def _step1_get_device_and_user_codes_fail_helper(self, status,
                                                     content, error_msg):
        flow = client.OAuth2WebServerFlow('CID', scope='foo')
        http = http_mock.HttpMock(headers={'status': status}, data=content)
        with self.assertRaises(client.OAuth2DeviceCodeError) as exc_manager:
            flow.step1_get_device_and_user_codes(http=http)

        self.assertEqual(exc_manager.exception.args, (error_msg,))

    def test_step1_get_device_and_user_codes_non_json_failure(self):
        status = int(http_client.BAD_REQUEST)
        content = 'Nope not JSON.'
        error_msg = 'Invalid response {0}.'.format(status)
        self._step1_get_device_and_user_codes_fail_helper(status, content,
                                                          error_msg)

    def test_step1_get_device_and_user_codes_basic_failure(self):
        status = int(http_client.INTERNAL_SERVER_ERROR)
        content = b'{}'
        error_msg = 'Invalid response {0}.'.format(status)
        self._step1_get_device_and_user_codes_fail_helper(status, content,
                                                          error_msg)

    def test_step1_get_device_and_user_codes_failure_w_json_error(self):
        status = int(http_client.BAD_GATEWAY)
        base_error = 'ZOMG user codes failure.'
        content = json.dumps({'error': base_error})
        error_msg = 'Invalid response {0}. Error: {1}'.format(status,
                                                              base_error)
        self._step1_get_device_and_user_codes_fail_helper(status, content,
                                                          error_msg)

    def test_step2_exchange_no_input(self):
        flow = client.OAuth2WebServerFlow('client_id+1', scope='foo')
        with self.assertRaises(ValueError):
            flow.step2_exchange()

    def test_step2_exchange_code_and_device_flow(self):
        flow = client.OAuth2WebServerFlow('client_id+1', scope='foo')
        with self.assertRaises(ValueError):
            flow.step2_exchange(code='code', device_flow_info='dfi')

    def test_scope_is_required(self):
        with self.assertRaises(TypeError):
            client.OAuth2WebServerFlow('client_id+1')

    def test_exchange_failure(self):
        http = http_mock.HttpMock(
            headers={'status': http_client.BAD_REQUEST},
            data=b'{"error":"invalid_request"}',
        )

        with self.assertRaises(client.FlowExchangeError):
            self.flow.step2_exchange(code='some random code', http=http)

    def test_urlencoded_exchange_failure(self):
        http = http_mock.HttpMock(
            headers={'status': http_client.BAD_REQUEST},
            data=b'error=invalid_request',
        )

        with self.assertRaisesRegexp(client.FlowExchangeError,
                                     'invalid_request'):
            self.flow.step2_exchange(code='some random code', http=http)

    def test_exchange_failure_with_json_error(self):
        # Some providers have 'error' attribute as a JSON object
        # in place of regular string.
        # This test makes sure no strange object-to-string coversion
        # exceptions are being raised instead of FlowExchangeError.
        payload = (b'{'
                   b'  "error": {'
                   b'    "message": "Error validating verification code.",'
                   b'    "type": "OAuthException"'
                   b'  }'
                   b'}')
        http = http_mock.HttpMock(data=payload)

        with self.assertRaises(client.FlowExchangeError):
            self.flow.step2_exchange(code='some random code', http=http)

    def _exchange_success_test_helper(self, code=None, device_flow_info=None):
        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "expires_in":3600,'
                   b'  "refresh_token":"8xLOxBtZp8"'
                   b'}')
        http = http_mock.HttpMock(data=payload)
        credentials = self.flow.step2_exchange(
            code=code, device_flow_info=device_flow_info, http=http)
        self.assertEqual('SlAV32hkKG', credentials.access_token)
        self.assertNotEqual(None, credentials.token_expiry)
        self.assertEqual('8xLOxBtZp8', credentials.refresh_token)
        self.assertEqual('dummy_revoke_uri', credentials.revoke_uri)
        self.assertEqual(set(['foo']), credentials.scopes)

    def test_exchange_success(self):
        self._exchange_success_test_helper(code='some random code')

    def test_exchange_success_with_device_flow_info(self):
        device_flow_info = client.DeviceFlowInfo(
            'some random code', None, None, None, None)
        self._exchange_success_test_helper(device_flow_info=device_flow_info)

    def test_exchange_success_binary_code(self):
        binary_code = b'some random code'
        access_token = 'SlAV32hkKG'
        expires_in = '3600'
        refresh_token = '8xLOxBtZp8'
        revoke_uri = 'dummy_revoke_uri'

        payload = ('{'
                   '  "access_token":"' + access_token + '",'
                   '  "expires_in":' + expires_in + ','
                   '  "refresh_token":"' + refresh_token + '"'
                   '}')
        http = http_mock.HttpMock(data=_helpers._to_bytes(payload))
        credentials = self.flow.step2_exchange(code=binary_code, http=http)
        self.assertEqual(access_token, credentials.access_token)
        self.assertIsNotNone(credentials.token_expiry)
        self.assertEqual(refresh_token, credentials.refresh_token)
        self.assertEqual(revoke_uri, credentials.revoke_uri)
        self.assertEqual(set(['foo']), credentials.scopes)

    def test_exchange_dictlike(self):
        class FakeDict(object):
            def __init__(self, d):
                self.d = d

            def __getitem__(self, name):
                return self.d[name]

            def __contains__(self, name):
                return name in self.d

        code = 'some random code'
        not_a_dict = FakeDict({'code': code})
        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "expires_in":3600,'
                   b'  "refresh_token":"8xLOxBtZp8"'
                   b'}')
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, payload),
        ])

        credentials = self.flow.step2_exchange(code=not_a_dict, http=http)
        self.assertEqual('SlAV32hkKG', credentials.access_token)
        self.assertNotEqual(None, credentials.token_expiry)
        self.assertEqual('8xLOxBtZp8', credentials.refresh_token)
        self.assertEqual('dummy_revoke_uri', credentials.revoke_uri)
        self.assertEqual(set(['foo']), credentials.scopes)
        self.assertEqual(len(http.requests), 1)
        request_code = urllib.parse.parse_qs(
            http.requests[0]['body'])['code'][0]
        self.assertEqual(code, request_code)

    def test_exchange_with_pkce(self):
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b'access_token=SlAV32hkKG'),
        ])
        flow = client.OAuth2WebServerFlow(
            'client_id+1',
            scope='foo',
            redirect_uri='http://example.com',
            pkce=True,
            code_verifier=self.good_verifier)
        flow.step2_exchange(code='some random code', http=http)

        self.assertEqual(len(http.requests), 1)
        test_request = http.requests[0]
        self.assertIn(
            'code_verifier={0}'.format(self.good_verifier.decode()),
            test_request['body'])

    def test_exchange_using_authorization_header(self):
        auth_header = 'Basic Y2xpZW50X2lkKzE6c2Vjexc_managerV0KzE=',
        flow = client.OAuth2WebServerFlow(
            client_id='client_id+1',
            authorization_header=auth_header,
            scope='foo',
            redirect_uri=client.OOB_CALLBACK_URN,
            user_agent='unittest-sample/1.0',
            revoke_uri='dummy_revoke_uri',
        )
        http = http_mock.HttpMockSequence([
            ({'status': http_client.OK}, b'access_token=SlAV32hkKG'),
        ])

        credentials = flow.step2_exchange(code='some random code', http=http)
        self.assertEqual('SlAV32hkKG', credentials.access_token)

        self.assertEqual(len(http.requests), 1)
        test_request = http.requests[0]
        # Did we pass the Authorization header?
        self.assertEqual(test_request['headers']['Authorization'], auth_header)
        # Did we omit client_secret from POST body?
        self.assertTrue('client_secret' not in test_request['body'])

    def test_urlencoded_exchange_success(self):
        http = http_mock.HttpMock(
            data=b'access_token=SlAV32hkKG&expires_in=3600')

        credentials = self.flow.step2_exchange(code='some random code',
                                               http=http)
        self.assertEqual('SlAV32hkKG', credentials.access_token)
        self.assertNotEqual(None, credentials.token_expiry)

    def test_urlencoded_expires_param(self):
        # Note the 'expires=3600' where you'd normally
        # have if named 'expires_in'
        http = http_mock.HttpMock(data=b'access_token=SlAV32hkKG&expires=3600')
        credentials = self.flow.step2_exchange(code='some random code',
                                               http=http)
        self.assertNotEqual(None, credentials.token_expiry)

    def test_exchange_no_expires_in(self):
        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "refresh_token":"8xLOxBtZp8"'
                   b'}')
        http = http_mock.HttpMock(data=payload)

        credentials = self.flow.step2_exchange(code='some random code',
                                               http=http)
        self.assertEqual(None, credentials.token_expiry)

    def test_urlencoded_exchange_no_expires_in(self):
        # This might be redundant but just to make sure
        # urlencoded access_token gets parsed correctly
        http = http_mock.HttpMock(data=b'access_token=SlAV32hkKG')

        credentials = self.flow.step2_exchange(code='some random code',
                                               http=http)
        self.assertEqual(None, credentials.token_expiry)

    def test_exchange_fails_if_no_code(self):
        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "refresh_token":"8xLOxBtZp8"'
                   b'}')
        http = http_mock.HttpMock(data=payload)

        code = {'error': 'thou shall not pass'}
        with self.assertRaisesRegexp(
                client.FlowExchangeError, 'shall not pass'):
            self.flow.step2_exchange(code=code, http=http)

    def test_exchange_id_token_fail(self):
        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "refresh_token":"8xLOxBtZp8",'
                   b'  "id_token": "stuff.payload"'
                   b'}')
        http = http_mock.HttpMock(data=payload)

        with self.assertRaises(client.VerifyJwtTokenError):
            self.flow.step2_exchange(code='some random code', http=http)

    def test_exchange_id_token(self):
        body = {'foo': 'bar'}
        body_json = json.dumps(body).encode('ascii')
        payload = base64.urlsafe_b64encode(body_json).strip(b'=')
        jwt = (base64.urlsafe_b64encode(b'stuff') + b'.' + payload + b'.' +
               base64.urlsafe_b64encode(b'signature'))

        payload = (b'{'
                   b'  "access_token":"SlAV32hkKG",'
                   b'  "refresh_token":"8xLOxBtZp8",'
                   b'  "id_token": "' + jwt + b'"'
                   b'}')
        http = http_mock.HttpMock(data=payload)
        credentials = self.flow.step2_exchange(code='some random code',
                                               http=http)
        self.assertEqual(credentials.id_token, body)
        self.assertEqual(credentials.id_token_jwt, jwt.decode())


class FlowFromCachedClientsecrets(unittest.TestCase):

    def test_flow_from_clientsecrets_cached(self):
        cache_mock = http_mock.CacheMock()
        load_and_cache('client_secrets.json', 'some_secrets', cache_mock)

        flow = client.flow_from_clientsecrets(
            'some_secrets', '', redirect_uri='oob', cache=cache_mock)
        self.assertEqual('foo_client_secret', flow.client_secret)

    @mock.patch('oauth2client.clientsecrets.loadfile')
    def _flow_from_clientsecrets_success_helper(self, loadfile_mock,
                                                device_uri=None,
                                                revoke_uri=None):
        client_type = clientsecrets.TYPE_WEB
        client_info = {
            'auth_uri': 'auth_uri',
            'token_uri': 'token_uri',
            'client_id': 'client_id',
            'client_secret': 'client_secret',
        }
        if revoke_uri is not None:
            client_info['revoke_uri'] = revoke_uri
        loadfile_mock.return_value = client_type, client_info
        filename = object()
        scope = ['baz']
        cache = object()

        if device_uri is not None:
            result = client.flow_from_clientsecrets(
                filename, scope, cache=cache, device_uri=device_uri)
            self.assertEqual(result.device_uri, device_uri)
        else:
            result = client.flow_from_clientsecrets(
                filename, scope, cache=cache)

        self.assertIsInstance(result, client.OAuth2WebServerFlow)
        loadfile_mock.assert_called_once_with(filename, cache=cache)

    def test_flow_from_clientsecrets_success(self):
        self._flow_from_clientsecrets_success_helper()

    def test_flow_from_clientsecrets_success_w_device_uri(self):
        device_uri = 'http://device.uri'
        self._flow_from_clientsecrets_success_helper(device_uri=device_uri)

    def test_flow_from_clientsecrets_success_w_revoke_uri(self):
        revoke_uri = 'http://revoke.uri'
        self._flow_from_clientsecrets_success_helper(revoke_uri=revoke_uri)

    @mock.patch('oauth2client.clientsecrets.loadfile',
                side_effect=clientsecrets.InvalidClientSecretsError)
    def test_flow_from_clientsecrets_invalid(self, loadfile_mock):
        filename = object()
        cache = object()
        with self.assertRaises(clientsecrets.InvalidClientSecretsError):
            client.flow_from_clientsecrets(
                filename, None, cache=cache, message=None)
        loadfile_mock.assert_called_once_with(filename, cache=cache)

    @mock.patch('oauth2client.clientsecrets.loadfile',
                side_effect=clientsecrets.InvalidClientSecretsError)
    @mock.patch('sys.exit')
    def test_flow_from_clientsecrets_invalid_w_msg(self, sys_exit,
                                                   loadfile_mock):
        filename = object()
        cache = object()
        message = 'hi mom'

        client.flow_from_clientsecrets(
            filename, None, cache=cache, message=message)
        sys_exit.assert_called_once_with(message)
        loadfile_mock.assert_called_once_with(filename, cache=cache)

    @mock.patch('oauth2client.clientsecrets.loadfile',
                side_effect=clientsecrets.InvalidClientSecretsError('foobar'))
    @mock.patch('sys.exit')
    def test_flow_from_clientsecrets_invalid_w_msg_and_text(self, sys_exit,
                                                            loadfile_mock):
        filename = object()
        cache = object()
        message = 'hi mom'
        expected = ('The client secrets were invalid: '
                    '\n{0}\n{1}'.format('foobar', 'hi mom'))

        client.flow_from_clientsecrets(
            filename, None, cache=cache, message=message)
        sys_exit.assert_called_once_with(expected)
        loadfile_mock.assert_called_once_with(filename, cache=cache)

    @mock.patch('oauth2client.clientsecrets.loadfile')
    def test_flow_from_clientsecrets_unknown_flow(self, loadfile_mock):
        client_type = 'UNKNOWN'
        loadfile_mock.return_value = client_type, None
        filename = object()
        cache = object()

        err_msg = ('This OAuth 2.0 flow is unsupported: '
                   '{0!r}'.format(client_type))
        with self.assertRaisesRegexp(client.UnknownClientSecretsFlowError,
                                     err_msg):
            client.flow_from_clientsecrets(filename, None, cache=cache)

        loadfile_mock.assert_called_once_with(filename, cache=cache)


class CredentialsFromCodeTests(unittest.TestCase):

    def setUp(self):
        self.client_id = 'client_id_abc'
        self.client_secret = 'secret_use_code'
        self.scope = 'foo'
        self.code = '12345abcde'
        self.redirect_uri = 'postmessage'

    def test_exchange_code_for_token(self):
        token = 'asdfghjkl'
        payload = json.dumps({'access_token': token, 'expires_in': 3600})
        http = http_mock.HttpMock(data=payload.encode('utf-8'))
        credentials = client.credentials_from_code(
            self.client_id, self.client_secret, self.scope,
            self.code, http=http, redirect_uri=self.redirect_uri)
        self.assertEqual(credentials.access_token, token)
        self.assertNotEqual(None, credentials.token_expiry)
        self.assertEqual(set(['foo']), credentials.scopes)

    def test_exchange_code_for_token_fail(self):
        http = http_mock.HttpMock(
            headers={'status': http_client.BAD_REQUEST},
            data=b'{"error":"invalid_request"}',
        )

        with self.assertRaises(client.FlowExchangeError):
            client.credentials_from_code(
                self.client_id, self.client_secret, self.scope,
                self.code, http=http, redirect_uri=self.redirect_uri)

    def test_exchange_code_and_file_for_token(self):
        payload = (b'{'
                   b'  "access_token":"asdfghjkl",'
                   b'  "expires_in":3600'
                   b'}')
        http = http_mock.HttpMock(data=payload)
        credentials = client.credentials_from_clientsecrets_and_code(
            datafile('client_secrets.json'), self.scope,
            self.code, http=http)
        self.assertEqual(credentials.access_token, 'asdfghjkl')
        self.assertNotEqual(None, credentials.token_expiry)
        self.assertEqual(set(['foo']), credentials.scopes)

    def test_exchange_code_and_cached_file_for_token(self):
        http = http_mock.HttpMock(data=b'{ "access_token":"asdfghjkl"}')
        cache_mock = http_mock.CacheMock()
        load_and_cache('client_secrets.json', 'some_secrets', cache_mock)

        credentials = client.credentials_from_clientsecrets_and_code(
            'some_secrets', self.scope,
            self.code, http=http, cache=cache_mock)
        self.assertEqual(credentials.access_token, 'asdfghjkl')
        self.assertEqual(set(['foo']), credentials.scopes)

    def test_exchange_code_and_file_for_token_fail(self):
        http = http_mock.HttpMock(
            headers={'status': http_client.BAD_REQUEST},
            data=b'{"error":"invalid_request"}',
        )

        with self.assertRaises(client.FlowExchangeError):
            client.credentials_from_clientsecrets_and_code(
                datafile('client_secrets.json'), self.scope,
                self.code, http=http)


class Test__save_private_file(unittest.TestCase):

    def _save_helper(self, filename):
        contents = []
        contents_str = '[]'
        client._save_private_file(filename, contents)
        with open(filename, 'r') as f:
            stored_contents = f.read()
        self.assertEqual(stored_contents, contents_str)

        stat_mode = os.stat(filename).st_mode
        # Octal 777, only last 3 positions matter for permissions mask.
        stat_mode &= 0o777
        self.assertEqual(stat_mode, 0o600)

    def test_new(self):
        filename = tempfile.mktemp()
        self.assertFalse(os.path.exists(filename))
        self._save_helper(filename)

    def test_existing(self):
        filename = tempfile.mktemp()
        with open(filename, 'w') as f:
            f.write('a bunch of nonsense longer than []')
        self.assertTrue(os.path.exists(filename))
        self._save_helper(filename)


class Test__get_application_default_credential_GAE(unittest.TestCase):

    @mock.patch.dict('sys.modules', {
        'oauth2client.contrib.appengine': mock.Mock()})
    def test_it(self):
        gae_mod = sys.modules['oauth2client.contrib.appengine']
        gae_mod.AppAssertionCredentials = creds_kls = mock.Mock()
        creds_kls.return_value = object()
        credentials = client._get_application_default_credential_GAE()
        self.assertEqual(credentials, creds_kls.return_value)
        creds_kls.assert_called_once_with([])


class Test__get_application_default_credential_GCE(unittest.TestCase):

    @mock.patch.dict('sys.modules', {
        'oauth2client.contrib.gce': mock.Mock()})
    def test_it(self):
        gce_mod = sys.modules['oauth2client.contrib.gce']
        gce_mod.AppAssertionCredentials = creds_kls = mock.Mock()
        creds_kls.return_value = object()
        credentials = client._get_application_default_credential_GCE()
        self.assertEqual(credentials, creds_kls.return_value)
        creds_kls.assert_called_once_with()


class Test__require_crypto_or_die(unittest.TestCase):

    @mock.patch.object(client, 'HAS_CRYPTO', new=True)
    def test_with_crypto(self):
        self.assertIsNone(client._require_crypto_or_die())

    @mock.patch.object(client, 'HAS_CRYPTO', new=False)
    def test_without_crypto(self):
        with self.assertRaises(client.CryptoUnavailableError):
            client._require_crypto_or_die()


class TestDeviceFlowInfo(unittest.TestCase):

    DEVICE_CODE = 'e80ff179-fd65-416c-9dbf-56a23e5d23e4'
    USER_CODE = '4bbd8b82-fc73-11e5-adf3-00c2c63e5792'
    VER_URL = 'http://foo.bar'

    def test_FromResponse(self):
        response = {
            'device_code': self.DEVICE_CODE,
            'user_code': self.USER_CODE,
            'verification_url': self.VER_URL,
        }
        result = client.DeviceFlowInfo.FromResponse(response)
        expected_result = client.DeviceFlowInfo(
            self.DEVICE_CODE, self.USER_CODE, None, self.VER_URL, None)
        self.assertEqual(result, expected_result)

    def test_FromResponse_fallback_to_uri(self):
        response = {
            'device_code': self.DEVICE_CODE,
            'user_code': self.USER_CODE,
            'verification_uri': self.VER_URL,
        }
        result = client.DeviceFlowInfo.FromResponse(response)
        expected_result = client.DeviceFlowInfo(
            self.DEVICE_CODE, self.USER_CODE, None, self.VER_URL, None)
        self.assertEqual(result, expected_result)

    def test_FromResponse_missing_url(self):
        response = {
            'device_code': self.DEVICE_CODE,
            'user_code': self.USER_CODE,
        }
        with self.assertRaises(client.OAuth2DeviceCodeError):
            client.DeviceFlowInfo.FromResponse(response)

    @mock.patch('oauth2client.client._UTCNOW')
    def test_FromResponse_with_expires_in(self, utcnow):
        expires_in = 23
        response = {
            'device_code': self.DEVICE_CODE,
            'user_code': self.USER_CODE,
            'verification_url': self.VER_URL,
            'expires_in': expires_in,
        }
        now = datetime.datetime(1999, 1, 1, 12, 30, 27)
        expire = datetime.datetime(1999, 1, 1, 12, 30, 27 + expires_in)
        utcnow.return_value = now

        result = client.DeviceFlowInfo.FromResponse(response)
        expected_result = client.DeviceFlowInfo(
            self.DEVICE_CODE, self.USER_CODE, None, self.VER_URL, expire)
        self.assertEqual(result, expected_result)
