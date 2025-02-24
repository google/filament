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

"""Django model tests.

Unit tests for models and fields defined by the django_util helper.
"""

import base64
import pickle
import unittest

import jsonpickle

from oauth2client import _helpers
from oauth2client import client
from oauth2client.contrib.django_util import models
from tests.contrib.django_util import models as tests_models


class TestCredentialsField(unittest.TestCase):

    def setUp(self):
        self.fake_model = tests_models.CredentialsModel()
        self.fake_model_field = self.fake_model._meta.get_field('credentials')
        self.field = models.CredentialsField(null=True)
        self.credentials = client.Credentials()
        self.pickle_str = _helpers._from_bytes(
            base64.b64encode(pickle.dumps(self.credentials)))
        self.jsonpickle_str = _helpers._from_bytes(
            base64.b64encode(jsonpickle.encode(self.credentials).encode()))

    def test_field_is_text(self):
        self.assertEqual(self.field.get_internal_type(), 'BinaryField')

    def test_field_unpickled(self):
        self.assertIsInstance(
            self.field.to_python(self.pickle_str), client.Credentials)

    def test_field_jsonunpickled(self):
        self.assertIsInstance(
            self.field.to_python(self.jsonpickle_str), client.Credentials)

    def test_field_already_unpickled(self):
        self.assertIsInstance(
            self.field.to_python(self.credentials), client.Credentials)

    def test_none_field_unpickled(self):
        self.assertIsNone(self.field.to_python(None))

    def test_from_db_value(self):
        value = self.field.from_db_value(
            self.pickle_str, None, None, None)
        self.assertIsInstance(value, client.Credentials)

    def test_field_unpickled_none(self):
        self.assertEqual(self.field.to_python(None), None)

    def test_field_pickled(self):
        prep_value = self.field.get_db_prep_value(self.credentials,
                                                  connection=None)
        self.assertEqual(prep_value, self.jsonpickle_str)

    def test_field_value_to_string(self):
        self.fake_model.credentials = self.credentials
        value_str = self.fake_model_field.value_to_string(self.fake_model)
        self.assertEqual(value_str, self.jsonpickle_str)

    def test_field_value_to_string_none(self):
        self.fake_model.credentials = None
        value_str = self.fake_model_field.value_to_string(self.fake_model)
        self.assertIsNone(value_str)

    def test_credentials_without_null(self):
        credentials = models.CredentialsField()
        self.assertTrue(credentials.null)


class CredentialWithSetStore(models.CredentialsField):
    def __init__(self):
        self.model = CredentialWithSetStore

    def set_store(self, storage):
        pass  # pragma: NO COVER


class FakeCredentialsModelMock(object):

    credentials = CredentialWithSetStore()


class FakeCredentialsModelMockNoSet(object):

    credentials = models.CredentialsField()
