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

"""Tests for oauth2client.contrib.keyring_storage."""

import datetime
import threading
import unittest

import keyring
import mock

import oauth2client
from oauth2client import client
from oauth2client.contrib import keyring_storage


class KeyringStorageTests(unittest.TestCase):

    def test_constructor(self):
        service_name = 'my_unit_test'
        user_name = 'me'
        store = keyring_storage.Storage(service_name, user_name)
        self.assertEqual(store._service_name, service_name)
        self.assertEqual(store._user_name, user_name)
        lock_type = type(threading.Lock())
        self.assertIsInstance(store._lock, lock_type)

    def test_acquire_lock(self):
        store = keyring_storage.Storage('my_unit_test', 'me')
        store._lock = lock = _FakeLock()
        self.assertEqual(lock._acquire_count, 0)
        store.acquire_lock()
        self.assertEqual(lock._acquire_count, 1)

    def test_release_lock(self):
        store = keyring_storage.Storage('my_unit_test', 'me')
        store._lock = lock = _FakeLock()
        self.assertEqual(lock._release_count, 0)
        store.release_lock()
        self.assertEqual(lock._release_count, 1)

    def test_locked_get(self):
        service_name = 'my_unit_test'
        user_name = 'me'
        mock_content = (object(), 'mock_content')
        mock_return_creds = mock.Mock()
        mock_return_creds.set_store = set_store = mock.Mock(
            name='set_store')
        with mock.patch.object(keyring, 'get_password',
                               return_value=mock_content,
                               autospec=True) as get_password:
            class_name = 'oauth2client.client.Credentials'
            with mock.patch(class_name) as MockCreds:
                MockCreds.new_from_json = new_from_json = mock.Mock(
                    name='new_from_json', return_value=mock_return_creds)
                store = keyring_storage.Storage(service_name, user_name)
                credentials = store.locked_get()
                new_from_json.assert_called_once_with(mock_content)
                get_password.assert_called_once_with(service_name, user_name)
                self.assertEqual(credentials, mock_return_creds)
                set_store.assert_called_once_with(store)

    def test_locked_put(self):
        service_name = 'my_unit_test'
        user_name = 'me'
        store = keyring_storage.Storage(service_name, user_name)
        with mock.patch.object(keyring, 'set_password',
                               return_value=None,
                               autospec=True) as set_password:
            credentials = mock.Mock()
            to_json_ret = object()
            credentials.to_json = to_json = mock.Mock(
                name='to_json', return_value=to_json_ret)
            store.locked_put(credentials)
            to_json.assert_called_once_with()
            set_password.assert_called_once_with(service_name, user_name,
                                                 to_json_ret)

    def test_locked_delete(self):
        service_name = 'my_unit_test'
        user_name = 'me'
        store = keyring_storage.Storage(service_name, user_name)
        with mock.patch.object(keyring, 'set_password',
                               return_value=None,
                               autospec=True) as set_password:
            store.locked_delete()
            set_password.assert_called_once_with(service_name, user_name, '')

    def test_get_with_no_credentials_stored(self):
        with mock.patch.object(keyring, 'get_password',
                               return_value=None,
                               autospec=True) as get_password:
            store = keyring_storage.Storage('my_unit_test', 'me')
            credentials = store.get()
            self.assertEquals(None, credentials)
            get_password.assert_called_once_with('my_unit_test', 'me')

    def test_get_with_malformed_json_credentials_stored(self):
        with mock.patch.object(keyring, 'get_password',
                               return_value='{',
                               autospec=True) as get_password:
            store = keyring_storage.Storage('my_unit_test', 'me')
            credentials = store.get()
            self.assertEquals(None, credentials)
            get_password.assert_called_once_with('my_unit_test', 'me')

    def test_get_and_set_with_json_credentials_stored(self):
        access_token = 'foo'
        client_id = 'some_client_id'
        client_secret = 'cOuDdkfjxxnv+'
        refresh_token = '1/0/a.df219fjls0'
        token_expiry = datetime.datetime.utcnow()
        user_agent = 'refresh_checker/1.0'

        credentials = client.OAuth2Credentials(
            access_token, client_id, client_secret,
            refresh_token, token_expiry, oauth2client.GOOGLE_TOKEN_URI,
            user_agent)

        # Setting autospec on a mock with an iterable side_effect is
        # currently broken (http://bugs.python.org/issue17826), so instead
        # we patch twice.
        with mock.patch.object(keyring, 'get_password',
                               return_value=None,
                               autospec=True) as get_password:
            with mock.patch.object(keyring, 'set_password',
                                   return_value=None,
                                   autospec=True) as set_password:
                store = keyring_storage.Storage('my_unit_test', 'me')
                self.assertEquals(None, store.get())

                store.put(credentials)

                set_password.assert_called_once_with(
                    'my_unit_test', 'me', credentials.to_json())
                get_password.assert_called_once_with('my_unit_test', 'me')

        with mock.patch.object(keyring, 'get_password',
                               return_value=credentials.to_json(),
                               autospec=True) as get_password:
            restored = store.get()
            self.assertEqual('foo', restored.access_token)
            self.assertEqual('some_client_id', restored.client_id)
            get_password.assert_called_once_with('my_unit_test', 'me')


class _FakeLock(object):

    _acquire_count = 0
    _release_count = 0

    def acquire(self):
        self._acquire_count += 1

    def release(self):
        self._release_count += 1
