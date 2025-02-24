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

"""Unit test for django_util views"""

import copy
import json

import django
from django import http
import django.conf
from django.contrib.auth import models as django_models
import mock
from six.moves import reload_module

from oauth2client import client
import oauth2client.contrib.django_util
from oauth2client.contrib.django_util import views
from tests.contrib import django_util as tests_django_util
from tests.contrib.django_util import models as tests_models


class OAuth2AuthorizeTest(tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(OAuth2AuthorizeTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)
        reload_module(oauth2client.contrib.django_util)
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    def tearDown(self):
        django.conf.settings = copy.deepcopy(self.save_settings)

    def test_authorize_works(self):
        request = self.factory.get('oauth2/oauth2authorize')
        request.session = self.session
        request.user = self.user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)

    def test_authorize_anonymous_user(self):
        request = self.factory.get('oauth2/oauth2authorize')
        request.session = self.session
        request.user = django_models.AnonymousUser()
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)

    def test_authorize_works_explicit_return_url(self):
        request = self.factory.get('oauth2/oauth2authorize',
                                   data={'return_url': '/return_endpoint'})
        request.session = self.session
        request.user = self.user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)


class Oauth2AuthorizeStorageModelTest(
        tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(Oauth2AuthorizeStorageModelTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)

        STORAGE_MODEL = {
            'model': 'tests.contrib.django_util.models.CredentialsModel',
            'user_property': 'user_id',
            'credentials_property': 'credentials'
        }
        django.conf.settings.GOOGLE_OAUTH2_STORAGE_MODEL = STORAGE_MODEL

        # OAuth2 Settings gets configured based on Django settings
        # at import time, so in order for us to reload the settings
        # we need to reload the module
        reload_module(oauth2client.contrib.django_util)
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    def tearDown(self):
        django.conf.settings = copy.deepcopy(self.save_settings)

    def test_authorize_works(self):
        request = self.factory.get('oauth2/oauth2authorize')
        request.session = self.session
        request.user = self.user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        # redirects to Google oauth
        self.assertIn('accounts.google.com', response.url)

    def test_authorize_anonymous_user_redirects_login(self):
        request = self.factory.get('oauth2/oauth2authorize')
        request.session = self.session
        request.user = django_models.AnonymousUser()
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        # redirects to Django login
        self.assertIn(django.conf.settings.LOGIN_URL, response.url)

    def test_authorize_works_explicit_return_url(self):
        request = self.factory.get('oauth2/oauth2authorize',
                                   data={'return_url': '/return_endpoint'})
        request.session = self.session
        request.user = self.user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)

    def test_authorized_user_no_credentials_redirects(self):
        request = self.factory.get('oauth2/oauth2authorize',
                                   data={'return_url': '/return_endpoint'})
        request.session = self.session

        authorized_user = django_models.User.objects.create_user(
            username='bill2', email='bill@example.com', password='hunter2')

        tests_models.CredentialsModel.objects.create(
            user_id=authorized_user,
            credentials=None)

        request.user = authorized_user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)

    def test_already_authorized(self):
        request = self.factory.get('oauth2/oauth2authorize',
                                   data={'return_url': '/return_endpoint'})
        request.session = self.session

        authorized_user = django_models.User.objects.create_user(
            username='bill2', email='bill@example.com', password='hunter2')

        credentials = _Credentials()
        tests_models.CredentialsModel.objects.create(
            user_id=authorized_user,
            credentials=credentials)

        request.user = authorized_user
        response = views.oauth2_authorize(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        self.assertEqual(response.url, '/return_endpoint')


class _Credentials(object):
    # Can't use mock when testing Django models
    # https://code.djangoproject.com/ticket/25493
    def __init__(self):
        self.invalid = False
        self.scopes = set()

    def has_scopes(self, _):
        return True


class Oauth2CallbackTest(tests_django_util.TestWithDjangoEnvironment):

    def setUp(self):
        super(Oauth2CallbackTest, self).setUp()
        self.save_settings = copy.deepcopy(django.conf.settings)
        reload_module(oauth2client.contrib.django_util)

        self.CSRF_TOKEN = 'token'
        self.RETURN_URL = 'http://return-url.com'
        self.fake_state = {
            'csrf_token': self.CSRF_TOKEN,
            'return_url': self.RETURN_URL,
            'scopes': django.conf.settings.GOOGLE_OAUTH2_SCOPES
        }
        self.user = django_models.User.objects.create_user(
            username='bill', email='bill@example.com', password='hunter2')

    @mock.patch('oauth2client.contrib.django_util.views.jsonpickle')
    def test_callback_works(self, jsonpickle_mock):
        request = self.factory.get('oauth2/oauth2callback', data={
            'state': json.dumps(self.fake_state),
            'code': 123
        })

        self.session['google_oauth2_csrf_token'] = self.CSRF_TOKEN

        flow = client.OAuth2WebServerFlow(
            client_id='clientid',
            client_secret='clientsecret',
            scope=['email'],
            state=json.dumps(self.fake_state),
            redirect_uri=request.build_absolute_uri("oauth2/oauth2callback"))

        name = 'google_oauth2_flow_{0}'.format(self.CSRF_TOKEN)
        pickled_flow = object()
        self.session[name] = pickled_flow
        flow.step2_exchange = mock.Mock()
        jsonpickle_mock.decode.return_value = flow

        request.session = self.session
        request.user = self.user
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseRedirect)
        self.assertEqual(
            response.status_code, django.http.HttpResponseRedirect.status_code)
        self.assertEqual(response['Location'], self.RETURN_URL)
        jsonpickle_mock.decode.assert_called_once_with(pickled_flow)

    @mock.patch('oauth2client.contrib.django_util.views.jsonpickle')
    def test_callback_handles_bad_flow_exchange(self, jsonpickle_mock):
        request = self.factory.get('oauth2/oauth2callback', data={
            "state": json.dumps(self.fake_state),
            "code": 123
        })

        self.session['google_oauth2_csrf_token'] = self.CSRF_TOKEN

        flow = client.OAuth2WebServerFlow(
            client_id='clientid',
            client_secret='clientsecret',
            scope=['email'],
            state=json.dumps(self.fake_state),
            redirect_uri=request.build_absolute_uri('oauth2/oauth2callback'))

        session_key = 'google_oauth2_flow_{0}'.format(self.CSRF_TOKEN)
        pickled_flow = object()
        self.session[session_key] = pickled_flow

        def local_throws(code):
            raise client.FlowExchangeError('test')

        flow.step2_exchange = local_throws
        jsonpickle_mock.decode.return_value = flow

        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        jsonpickle_mock.decode.assert_called_once_with(pickled_flow)

    def test_error_returns_bad_request(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'error': 'There was an error in your authorization.',
        })
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertIn(b'Authorization failed', response.content)

    def test_error_escapes_html(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'error': '<script>bad</script>',
        })
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertNotIn(b'<script>', response.content)
        self.assertIn(b'&lt;script&gt;', response.content)

    def test_no_session(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'code': 123,
            'state': json.dumps(self.fake_state)
        })

        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertEqual(
            response.content, b'No existing session for this flow.')

    def test_missing_state_returns_bad_request(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'code': 123
        })
        self.session['google_oauth2_csrf_token'] = "token"
        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)

    def test_bad_state(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'code': 123,
            'state': json.dumps({'wrong': 'state'})
        })
        self.session['google_oauth2_csrf_token'] = 'token'
        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertEqual(response.content, b'Invalid state parameter.')

    def test_bad_csrf(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            "state": json.dumps(self.fake_state),
            "code": 123
        })
        self.session['google_oauth2_csrf_token'] = 'WRONG TOKEN'
        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertEqual(response.content, b'Invalid CSRF token.')

    def test_no_saved_flow(self):
        request = self.factory.get('oauth2/oauth2callback', data={
            'state': json.dumps(self.fake_state),
            'code': 123
        })
        self.session['google_oauth2_csrf_token'] = self.CSRF_TOKEN
        self.session['google_oauth2_flow_{0}'.format(self.CSRF_TOKEN)] = None
        request.session = self.session
        response = views.oauth2_callback(request)
        self.assertIsInstance(response, http.HttpResponseBadRequest)
        self.assertEqual(response.content, b'Missing Oauth2 flow.')
