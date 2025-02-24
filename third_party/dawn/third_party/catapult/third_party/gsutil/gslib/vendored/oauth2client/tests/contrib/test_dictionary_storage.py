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

"""Unit tests for oauth2client.contrib.dictionary_storage"""

import unittest

import oauth2client
from oauth2client import client
from oauth2client.contrib import dictionary_storage


def _generate_credentials(scopes=None):
    return client.OAuth2Credentials(
        'access_tokenz',
        'client_idz',
        'client_secretz',
        'refresh_tokenz',
        '3600',
        oauth2client.GOOGLE_TOKEN_URI,
        'Test',
        id_token={
            'sub': '123',
            'email': 'user@example.com'
        },
        scopes=scopes)


class DictionaryStorageTests(unittest.TestCase):

    def test_constructor_defaults(self):
        dictionary = {}
        key = 'test-key'
        storage = dictionary_storage.DictionaryStorage(dictionary, key)

        self.assertEqual(dictionary, storage._dictionary)
        self.assertEqual(key, storage._key)
        self.assertIsNone(storage._lock)

    def test_constructor_explicit(self):
        dictionary = {}
        key = 'test-key'
        storage = dictionary_storage.DictionaryStorage(dictionary, key)

        lock = object()
        storage = dictionary_storage.DictionaryStorage(
            dictionary, key, lock=lock)
        self.assertEqual(storage._lock, lock)

    def test_get(self):
        credentials = _generate_credentials()
        dictionary = {}
        key = 'credentials'
        storage = dictionary_storage.DictionaryStorage(dictionary, key)

        self.assertIsNone(storage.get())

        dictionary[key] = credentials.to_json()
        returned = storage.get()

        self.assertIsNotNone(returned)
        self.assertEqual(returned.access_token, credentials.access_token)
        self.assertEqual(returned.id_token, credentials.id_token)
        self.assertEqual(returned.refresh_token, credentials.refresh_token)
        self.assertEqual(returned.client_id, credentials.client_id)

    def test_put(self):
        credentials = _generate_credentials()
        dictionary = {}
        key = 'credentials'
        storage = dictionary_storage.DictionaryStorage(dictionary, key)

        storage.put(credentials)
        returned = storage.get()

        self.assertIn(key, dictionary)
        self.assertIsNotNone(returned)
        self.assertEqual(returned.access_token, credentials.access_token)
        self.assertEqual(returned.id_token, credentials.id_token)
        self.assertEqual(returned.refresh_token, credentials.refresh_token)
        self.assertEqual(returned.client_id, credentials.client_id)

    def test_delete(self):
        credentials = _generate_credentials()
        dictionary = {}
        key = 'credentials'
        storage = dictionary_storage.DictionaryStorage(dictionary, key)

        storage.put(credentials)

        self.assertIn(key, dictionary)

        storage.delete()

        self.assertNotIn(key, dictionary)
        self.assertIsNone(storage.get())
