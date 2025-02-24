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

"""Tests for the django_util decorators."""

import copy

from django import http
import django.conf
from django.contrib.auth import models as django_models
import mock
from six.moves import http_client
from six.moves import reload_module
from six.moves.urllib import parse

import oauth2client.contrib.django_util
from oauth2client.contrib.django_util import decorators
from tests.contrib import django_util as tests_django_util


class OAuth2EnabledDecoratorTest(tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(OAuth2EnabledDecoratorTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)

        # OAuth2 Settings gets configured based on Django settings
        # at import time, so in order for us to reload the settings
        # we need to reload the module
        reload_module(oauth2client.contrib.django_util)
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    def tearDown(self):
        super(OAuth2EnabledDecoratorTest, self).tearDown()
        django.conf.settings = copy.deepcopy(self.save_settings)

    def test_no_credentials_without_credentials(self):
        request = self.factory.get('/test')
        request.session = self.session

        @decorators.oauth_enabled
        def test_view(request):
            return http.HttpResponse("test")  # pragma: NO COVER

        response = test_view(request)
        self.assertEqual(response.status_code, http_client.OK)
        self.assertIsNotNone(request.oauth)
        self.assertFalse(request.oauth.has_credentials())
        self.assertIsNone(request.oauth.http)

    @mock.patch('oauth2client.client.OAuth2Credentials')
    def test_has_credentials_in_storage(self, OAuth2Credentials):
        request = self.factory.get('/test')
        request.session = mock.Mock()

        credentials_mock = mock.Mock(
            scopes=set(django.conf.settings.GOOGLE_OAUTH2_SCOPES))
        credentials_mock.has_scopes.return_value = True
        credentials_mock.invalid = False
        credentials_mock.scopes = set([])
        OAuth2Credentials.from_json.return_value = credentials_mock

        @decorators.oauth_enabled
        def test_view(request):
            return http.HttpResponse('test')

        response = test_view(request)
        self.assertEqual(response.status_code, http_client.OK)
        self.assertEqual(response.content, b'test')
        self.assertTrue(request.oauth.has_credentials())
        self.assertIsNotNone(request.oauth.http)
        self.assertSetEqual(
            request.oauth.scopes,
            set(django.conf.settings.GOOGLE_OAUTH2_SCOPES))

    @mock.patch('oauth2client.contrib.dictionary_storage.DictionaryStorage')
    def test_specified_scopes(self, dictionary_storage_mock):
        request = self.factory.get('/test')
        request.session = mock.Mock()

        credentials_mock = mock.Mock(
            scopes=set(django.conf.settings.GOOGLE_OAUTH2_SCOPES))
        credentials_mock.has_scopes = mock.Mock(return_value=True)
        credentials_mock.is_valid = True
        dictionary_storage_mock.get.return_value = credentials_mock

        @decorators.oauth_enabled(scopes=['additional-scope'])
        def test_view(request):
            return http.HttpResponse('hello world')  # pragma: NO COVER

        response = test_view(request)
        self.assertEqual(response.status_code, http_client.OK)
        self.assertIsNotNone(request.oauth)
        self.assertFalse(request.oauth.has_credentials())


class OAuth2RequiredDecoratorTest(tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(OAuth2RequiredDecoratorTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)

        reload_module(oauth2client.contrib.django_util)
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    def tearDown(self):
        super(OAuth2RequiredDecoratorTest, self).tearDown()
        django.conf.settings = copy.deepcopy(self.save_settings)

    def test_redirects_without_credentials(self):
        request = self.factory.get('/test')
        request.session = self.session

        @decorators.oauth_required
        def test_view(request):
            return http.HttpResponse('test')  # pragma: NO COVER

        response = test_view(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        self.assertEqual(parse.urlparse(response['Location']).path,
                         '/oauth2/oauth2authorize/')
        self.assertIn(
            'return_url=%2Ftest', parse.urlparse(response['Location']).query)

        self.assertEqual(response.status_code,
                         http.HttpResponseRedirect.status_code)

    @mock.patch('oauth2client.contrib.django_util.UserOAuth2', autospec=True)
    def test_has_credentials_in_storage(self, UserOAuth2):
        request = self.factory.get('/test')
        request.session = mock.Mock()

        @decorators.oauth_required
        def test_view(request):
            return http.HttpResponse("test")

        my_user_oauth = mock.Mock()

        UserOAuth2.return_value = my_user_oauth
        my_user_oauth.has_credentials.return_value = True

        response = test_view(request)
        self.assertEqual(response.status_code, http_client.OK)
        self.assertEqual(response.content, b"test")

    @mock.patch('oauth2client.client.OAuth2Credentials')
    def test_has_credentials_in_storage_no_scopes(
            self, OAuth2Credentials):
        request = self.factory.get('/test')

        request.session = mock.Mock()
        credentials_mock = mock.Mock(
            scopes=set(django.conf.settings.GOOGLE_OAUTH2_SCOPES))
        credentials_mock.has_scopes.return_value = False

        OAuth2Credentials.from_json.return_value = credentials_mock

        @decorators.oauth_required
        def test_view(request):
            return http.HttpResponse("test")  # pragma: NO COVER

        response = test_view(request)
        self.assertEqual(
            response.status_code, django.http.HttpResponseRedirect.status_code)

    @mock.patch('oauth2client.client.OAuth2Credentials')
    def test_specified_scopes(self, OAuth2Credentials):
        request = self.factory.get('/test')
        request.session = mock.Mock()

        credentials_mock = mock.Mock(
            scopes=set(django.conf.settings.GOOGLE_OAUTH2_SCOPES))
        credentials_mock.has_scopes = mock.Mock(return_value=False)
        OAuth2Credentials.from_json.return_value = credentials_mock

        @decorators.oauth_required(scopes=['additional-scope'])
        def test_view(request):
            return http.HttpResponse("hello world")  # pragma: NO COVER

        response = test_view(request)
        self.assertEqual(
            response.status_code, django.http.HttpResponseRedirect.status_code)


class OAuth2RequiredDecoratorStorageModelTest(
        tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(OAuth2RequiredDecoratorStorageModelTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)

        STORAGE_MODEL = {
            'model': 'tests.contrib.django_util.models.CredentialsModel',
            'user_property': 'user_id',
            'credentials_property': 'credentials'
        }
        django.conf.settings.GOOGLE_OAUTH2_STORAGE_MODEL = STORAGE_MODEL

        reload_module(oauth2client.contrib.django_util)
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    def tearDown(self):
        super(OAuth2RequiredDecoratorStorageModelTest, self).tearDown()
        django.conf.settings = copy.deepcopy(self.save_settings)

    def test_redirects_anonymous_to_login(self):
        request = self.factory.get('/test')
        request.session = self.session
        request.user = django_models.AnonymousUser()

        @decorators.oauth_required
        def test_view(request):
            return http.HttpResponse("test")  # pragma: NO COVER

        response = test_view(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        self.assertEqual(parse.urlparse(response['Location']).path,
                         django.conf.settings.LOGIN_URL)

    def test_redirects_user_to_oauth_authorize(self):
        request = self.factory.get('/test')
        request.session = self.session
        request.user = django_models.User.objects.create_user(
            username='bill3', email='bill@example.com', password='hunter2')

        @decorators.oauth_required
        def test_view(request):
            return http.HttpResponse("test")  # pragma: NO COVER

        response = test_view(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        self.assertEqual(parse.urlparse(response['Location']).path,
                         '/oauth2/oauth2authorize/')
