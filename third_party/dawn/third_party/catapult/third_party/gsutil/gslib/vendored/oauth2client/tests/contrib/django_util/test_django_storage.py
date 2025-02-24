# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tests for the DjangoORM storage class."""

# Mock a Django environment
import datetime
import unittest

from django.db import models
import mock

from oauth2client import GOOGLE_TOKEN_URI
from oauth2client.client import OAuth2Credentials
from oauth2client.contrib.django_util.models import CredentialsField
from oauth2client.contrib.django_util.storage import (
    DjangoORMStorage as Storage)


class TestStorage(unittest.TestCase):
    def setUp(self):
        access_token = 'foo'
        client_id = 'some_client_id'
        client_secret = 'cOuDdkfjxxnv+'
        refresh_token = '1/0/a.df219fjls0'
        token_expiry = datetime.datetime.utcnow()
        user_agent = 'refresh_checker/1.0'

        self.credentials = OAuth2Credentials(
            access_token, client_id, client_secret,
            refresh_token, token_expiry, GOOGLE_TOKEN_URI,
            user_agent)

        self.key_name = 'id'
        self.key_value = '1'
        self.property_name = 'credentials'

    def test_constructor(self):
        storage = Storage(FakeCredentialsModel, self.key_name,
                          self.key_value, self.property_name)

        self.assertEqual(storage.model_class, FakeCredentialsModel)
        self.assertEqual(storage.key_name, self.key_name)
        self.assertEqual(storage.key_value, self.key_value)
        self.assertEqual(storage.property_name, self.property_name)

    @mock.patch('django.db.models')
    def test_locked_get(self, djangoModel):
        fake_model_with_credentials = FakeCredentialsModelMock()
        entities = [
            fake_model_with_credentials
        ]
        filter_mock = mock.Mock(return_value=entities)
        object_mock = mock.Mock()
        object_mock.filter = filter_mock
        FakeCredentialsModelMock.objects = object_mock

        storage = Storage(FakeCredentialsModelMock, self.key_name,
                          self.key_value, self.property_name)
        credential = storage.locked_get()
        self.assertEqual(
            credential, fake_model_with_credentials.credentials)

    @mock.patch('django.db.models')
    def test_locked_get_no_entities(self, djangoModel):
        entities = []
        filter_mock = mock.Mock(return_value=entities)
        object_mock = mock.Mock()
        object_mock.filter = filter_mock
        FakeCredentialsModelMock.objects = object_mock

        storage = Storage(FakeCredentialsModelMock, self.key_name,
                          self.key_value, self.property_name)
        credential = storage.locked_get()
        self.assertIsNone(credential)

    @mock.patch('django.db.models')
    def test_locked_get_no_set_store(self, djangoModel):
        fake_model_with_credentials = FakeCredentialsModelMockNoSet()
        entities = [
            fake_model_with_credentials
        ]
        filter_mock = mock.Mock(return_value=entities)
        object_mock = mock.Mock()
        object_mock.filter = filter_mock
        FakeCredentialsModelMockNoSet.objects = object_mock

        storage = Storage(FakeCredentialsModelMockNoSet, self.key_name,
                          self.key_value, self.property_name)
        credential = storage.locked_get()
        self.assertEqual(
            credential, fake_model_with_credentials.credentials)

    @mock.patch('django.db.models')
    def test_locked_put(self, djangoModel):
        entity_mock = mock.Mock(credentials=None)
        objects = mock.Mock(
            get_or_create=mock.Mock(return_value=(entity_mock, None)))
        FakeCredentialsModelMock.objects = objects
        storage = Storage(FakeCredentialsModelMock, self.key_name,
                          self.key_value, self.property_name)
        storage.locked_put(self.credentials)

    @mock.patch('django.db.models')
    def test_locked_delete(self, djangoModel):
        class FakeEntities(object):
            def __init__(self):
                self.deleted = False

            def delete(self):
                self.deleted = True

        fake_entities = FakeEntities()
        entities = fake_entities

        filter_mock = mock.Mock(return_value=entities)
        object_mock = mock.Mock()
        object_mock.filter = filter_mock
        FakeCredentialsModelMock.objects = object_mock
        storage = Storage(FakeCredentialsModelMock, self.key_name,
                          self.key_value, self.property_name)
        storage.locked_delete()
        self.assertTrue(fake_entities.deleted)


class CredentialWithSetStore(CredentialsField):
    def __init__(self):
        self.model = CredentialWithSetStore

    def set_store(self, storage):
        pass


class FakeCredentialsModel(models.Model):
    credentials = CredentialsField()


class FakeCredentialsModelMock(object):
    def __init__(self, *args, **kwargs):
        self.model = FakeCredentialsModelMock
        self.saved = False
        self.deleted = False

    credentials = CredentialWithSetStore()


class FakeCredentialsModelMockNoSet(object):
    def __init__(self, set_store=False, *args, **kwargs):
        self.model = FakeCredentialsModelMock
        self.saved = False
        self.deleted = False

    credentials = CredentialsField()
