# Copyright 2015 Google Inc. All rights reserved.
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

"""Unit tests for the Flask utilities"""

import datetime
import json
import logging
import unittest

import flask
import mock
import six.moves.http_client as httplib
import six.moves.urllib.parse as urlparse

import oauth2client
from oauth2client import client
from oauth2client import clientsecrets
from oauth2client.contrib import flask_util
from tests import http_mock


DEFAULT_RESP = """\
{
    "access_token": "foo_access_token",
    "expires_in": 3600,
    "extra": "value",
    "refresh_token": "foo_refresh_token"
}
"""


class FlaskOAuth2Tests(unittest.TestCase):

    def setUp(self):
        self.app = flask.Flask(__name__)
        self.app.testing = True
        self.app.config['SECRET_KEY'] = 'notasecert'
        self.app.logger.setLevel(logging.CRITICAL)
        self.oauth2 = flask_util.UserOAuth2(
            self.app,
            client_id='client_idz',
            client_secret='client_secretz')

    def _generate_credentials(self, scopes=None):
        return client.OAuth2Credentials(
            'access_tokenz',
            'client_idz',
            'client_secretz',
            'refresh_tokenz',
            datetime.datetime.utcnow() + datetime.timedelta(seconds=3600),
            oauth2client.GOOGLE_TOKEN_URI,
            'Test',
            id_token={
                'sub': '123',
                'email': 'user@example.com'
            },
            scopes=scopes)

    def test_explicit_configuration(self):
        oauth2 = flask_util.UserOAuth2(
            flask.Flask(__name__), client_id='id', client_secret='secret')

        self.assertEqual(oauth2.client_id, 'id')
        self.assertEqual(oauth2.client_secret, 'secret')

        return_val = (
            clientsecrets.TYPE_WEB,
            {'client_id': 'id', 'client_secret': 'secret'})

        with mock.patch('oauth2client.clientsecrets.loadfile',
                        return_value=return_val):

            oauth2 = flask_util.UserOAuth2(
                flask.Flask(__name__), client_secrets_file='file.json')

            self.assertEqual(oauth2.client_id, 'id')
            self.assertEqual(oauth2.client_secret, 'secret')

    def test_delayed_configuration(self):
        app = flask.Flask(__name__)
        oauth2 = flask_util.UserOAuth2()
        oauth2.init_app(app, client_id='id', client_secret='secret')
        self.assertEqual(oauth2.app, app)

    def test_explicit_storage(self):
        storage_mock = mock.Mock()
        oauth2 = flask_util.UserOAuth2(
            flask.Flask(__name__), storage=storage_mock, client_id='id',
            client_secret='secret')
        self.assertEqual(oauth2.storage, storage_mock)

    def test_explicit_scopes(self):
        oauth2 = flask_util.UserOAuth2(
            flask.Flask(__name__), scopes=['1', '2'], client_id='id',
            client_secret='secret')
        self.assertEqual(oauth2.scopes, ['1', '2'])

    def test_bad_client_secrets(self):
        return_val = (
            'other',
            {'client_id': 'id', 'client_secret': 'secret'})

        with mock.patch('oauth2client.clientsecrets.loadfile',
                        return_value=return_val):
            with self.assertRaises(ValueError):
                flask_util.UserOAuth2(flask.Flask(__name__),
                                      client_secrets_file='file.json')

    def test_app_configuration(self):
        app = flask.Flask(__name__)
        app.config['GOOGLE_OAUTH2_CLIENT_ID'] = 'id'
        app.config['GOOGLE_OAUTH2_CLIENT_SECRET'] = 'secret'

        oauth2 = flask_util.UserOAuth2(app)

        self.assertEqual(oauth2.client_id, 'id')
        self.assertEqual(oauth2.client_secret, 'secret')

        return_val = (
            clientsecrets.TYPE_WEB,
            {'client_id': 'id2', 'client_secret': 'secret2'})

        with mock.patch('oauth2client.clientsecrets.loadfile',
                        return_value=return_val):

            app = flask.Flask(__name__)
            app.config['GOOGLE_OAUTH2_CLIENT_SECRETS_FILE'] = 'file.json'
            oauth2 = flask_util.UserOAuth2(app)

            self.assertEqual(oauth2.client_id, 'id2')
            self.assertEqual(oauth2.client_secret, 'secret2')

    def test_no_configuration(self):
        with self.assertRaises(ValueError):
            flask_util.UserOAuth2(flask.Flask(__name__))

    def test_create_flow(self):
        with self.app.test_request_context():
            flow = self.oauth2._make_flow()
            state = json.loads(flow.params['state'])
            self.assertIn('google_oauth2_csrf_token', flask.session)
            self.assertEqual(
                flask.session['google_oauth2_csrf_token'], state['csrf_token'])
            self.assertEqual(flow.client_id, self.oauth2.client_id)
            self.assertEqual(flow.client_secret, self.oauth2.client_secret)
            self.assertIn('http', flow.redirect_uri)
            self.assertIn('oauth2callback', flow.redirect_uri)

            flow = self.oauth2._make_flow(return_url='/return_url')
            state = json.loads(flow.params['state'])
            self.assertEqual(state['return_url'], '/return_url')

            flow = self.oauth2._make_flow(extra_arg='test')
            self.assertEqual(flow.params['extra_arg'], 'test')

        # Test extra args specified in the constructor.
        app = flask.Flask(__name__)
        app.config['SECRET_KEY'] = 'notasecert'
        oauth2 = flask_util.UserOAuth2(
            app, client_id='client_id', client_secret='secret',
            extra_arg='test')

        with app.test_request_context():
            flow = oauth2._make_flow()
            self.assertEqual(flow.params['extra_arg'], 'test')

    def test_authorize_view(self):
        with self.app.test_client() as client:
            response = client.get('/oauth2authorize')
            location = response.headers['Location']
            q = urlparse.parse_qs(location.split('?', 1)[1])
            state = json.loads(q['state'][0])

            self.assertIn(oauth2client.GOOGLE_AUTH_URI, location)
            self.assertNotIn(self.oauth2.client_secret, location)
            self.assertIn(self.oauth2.client_id, q['client_id'])
            self.assertEqual(
                flask.session['google_oauth2_csrf_token'], state['csrf_token'])
            self.assertEqual(state['return_url'], '/')

        with self.app.test_client() as client:
            response = client.get('/oauth2authorize?return_url=/test')
            location = response.headers['Location']
            q = urlparse.parse_qs(location.split('?', 1)[1])
            state = json.loads(q['state'][0])
            self.assertEqual(state['return_url'], '/test')

        with self.app.test_client() as client:
            response = client.get('/oauth2authorize?extra_param=test')
            location = response.headers['Location']
            self.assertIn('extra_param=test', location)

    def _setup_callback_state(self, client, **kwargs):
        with self.app.test_request_context():
            # Flask doesn't create a request context with a session
            # transaction for some reason, so, set up the flow here,
            # then apply it to the session in the transaction.
            if not kwargs:
                self.oauth2._make_flow(return_url='/return_url')
            else:
                self.oauth2._make_flow(**kwargs)

            with client.session_transaction() as session:
                session.update(flask.session)
                csrf_token = session['google_oauth2_csrf_token']
                flow = flask_util._get_flow_for_token(csrf_token)
                state = flow.params['state']

        return state

    def test_callback_view(self):
        self.oauth2.storage = mock.Mock()
        with self.app.test_client() as client:
            with mock.patch(
                    'oauth2client.transport.get_http_object') as new_http:
                # Set-up mock.
                http = http_mock.HttpMock(data=DEFAULT_RESP)
                new_http.return_value = http
                # Run tests.
                state = self._setup_callback_state(client)

                response = client.get(
                    '/oauth2callback?state={0}&code=codez'.format(state))

                self.assertEqual(response.status_code, httplib.FOUND)
                self.assertIn('/return_url', response.headers['Location'])
                self.assertIn(self.oauth2.client_secret, http.body)
                self.assertIn('codez', http.body)
                self.assertTrue(self.oauth2.storage.put.called)

                # Check the mocks were called.
                new_http.assert_called_once_with()

    def test_authorize_callback(self):
        self.oauth2.authorize_callback = mock.Mock()
        self.test_callback_view()
        self.assertTrue(self.oauth2.authorize_callback.called)

    def test_callback_view_errors(self):
        # Error supplied to callback
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_csrf_token'] = 'tokenz'

            response = client.get('/oauth2callback?state={}&error=something')
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)
            self.assertIn('something', response.data.decode('utf-8'))

        # Error supplied to callback with html
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_csrf_token'] = 'tokenz'

            response = client.get(
                '/oauth2callback?state={}&error=<script>something<script>')
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)
            self.assertIn(
                '&lt;script&gt;something&lt;script&gt;',
                response.data.decode('utf-8'))

        # CSRF mismatch
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_csrf_token'] = 'goodstate'

            state = json.dumps({
                'csrf_token': 'badstate',
                'return_url': '/return_url'
            })

            response = client.get(
                '/oauth2callback?state={0}&code=codez'.format(state))
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)

        # KeyError, no CSRF state.
        with self.app.test_client() as client:
            response = client.get('/oauth2callback?state={}&code=codez')
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)

        # Code exchange error
        with self.app.test_client() as client:
            state = self._setup_callback_state(client)

            with mock.patch(
                    'oauth2client.transport.get_http_object') as new_http:
                # Set-up mock.
                new_http.return_value = http_mock.HttpMock(
                    headers={'status': httplib.INTERNAL_SERVER_ERROR},
                    data=DEFAULT_RESP)
                # Run tests.
                response = client.get(
                    '/oauth2callback?state={0}&code=codez'.format(state))
                self.assertEqual(response.status_code, httplib.BAD_REQUEST)

                # Check the mocks were called.
                new_http.assert_called_once_with()

        # Invalid state json
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_csrf_token'] = 'tokenz'

            state = '[{'
            response = client.get(
                '/oauth2callback?state={0}&code=codez'.format(state))
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)

        # Missing flow.
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_csrf_token'] = 'tokenz'

            state = json.dumps({
                'csrf_token': 'tokenz',
                'return_url': '/return_url'
            })

            response = client.get(
                '/oauth2callback?state={0}&code=codez'.format(state))
            self.assertEqual(response.status_code, httplib.BAD_REQUEST)

    def test_no_credentials(self):
        with self.app.test_request_context():
            self.assertFalse(self.oauth2.has_credentials())
            self.assertTrue(self.oauth2.credentials is None)
            self.assertTrue(self.oauth2.user_id is None)
            self.assertTrue(self.oauth2.email is None)
            with self.assertRaises(ValueError):
                self.oauth2.http()
            self.assertFalse(self.oauth2.storage.get())
            self.oauth2.storage.delete()

    def test_with_credentials(self):
        credentials = self._generate_credentials()
        with self.app.test_request_context():
            self.oauth2.storage.put(credentials)
            self.assertEqual(
                self.oauth2.credentials.access_token, credentials.access_token)
            self.assertEqual(
                self.oauth2.credentials.refresh_token,
                credentials.refresh_token)
            self.assertEqual(self.oauth2.user_id, '123')
            self.assertEqual(self.oauth2.email, 'user@example.com')
            self.assertTrue(self.oauth2.http())

    @mock.patch('oauth2client.client._UTCNOW')
    def test_with_expired_credentials(self, utcnow):
        utcnow.return_value = datetime.datetime(1990, 5, 29)

        credentials = self._generate_credentials()
        credentials.token_expiry = datetime.datetime(1990, 5, 28)

        # Has a refresh token, so this should be fine.
        with self.app.test_request_context():
            self.oauth2.storage.put(credentials)
            self.assertTrue(self.oauth2.has_credentials())

        # Without a refresh token this should return false.
        credentials.refresh_token = None
        with self.app.test_request_context():
            self.oauth2.storage.put(credentials)
            self.assertFalse(self.oauth2.has_credentials())

    def test_bad_id_token(self):
        credentials = self._generate_credentials()
        credentials.id_token = {}
        with self.app.test_request_context():
            self.oauth2.storage.put(credentials)
            self.assertTrue(self.oauth2.user_id is None)
            self.assertTrue(self.oauth2.email is None)

    def test_required(self):
        @self.app.route('/protected')
        @self.oauth2.required
        def index():
            return 'Hello'

        # No credentials, should redirect
        with self.app.test_client() as client:
            response = client.get('/protected')
            self.assertEqual(response.status_code, httplib.FOUND)
            self.assertIn('oauth2authorize', response.headers['Location'])
            self.assertIn('protected', response.headers['Location'])

        credentials = self._generate_credentials(scopes=self.oauth2.scopes)

        # With credentials, should allow
        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_credentials'] = credentials.to_json()

            response = client.get('/protected')
            self.assertEqual(response.status_code, httplib.OK)
            self.assertIn('Hello', response.data.decode('utf-8'))

        # Expired credentials with refresh token, should allow.
        credentials.token_expiry = datetime.datetime(1990, 5, 28)
        with mock.patch('oauth2client.client._UTCNOW') as utcnow:
            utcnow.return_value = datetime.datetime(1990, 5, 29)

            with self.app.test_client() as client:
                with client.session_transaction() as session:
                    session['google_oauth2_credentials'] = (
                        credentials.to_json())

                response = client.get('/protected')
                self.assertEqual(response.status_code, httplib.OK)
                self.assertIn('Hello', response.data.decode('utf-8'))

        # Expired credentials without a refresh token, should redirect.
        credentials.refresh_token = None
        with mock.patch('oauth2client.client._UTCNOW') as utcnow:
            utcnow.return_value = datetime.datetime(1990, 5, 29)

            with self.app.test_client() as client:
                with client.session_transaction() as session:
                    session['google_oauth2_credentials'] = (
                        credentials.to_json())

                response = client.get('/protected')
            self.assertEqual(response.status_code, httplib.FOUND)
            self.assertIn('oauth2authorize', response.headers['Location'])
            self.assertIn('protected', response.headers['Location'])

    def _create_incremental_auth_app(self):
        self.app = flask.Flask(__name__)
        self.app.testing = True
        self.app.config['SECRET_KEY'] = 'notasecert'
        self.oauth2 = flask_util.UserOAuth2(
            self.app,
            client_id='client_idz',
            client_secret='client_secretz',
            include_granted_scopes=True)

        @self.app.route('/one')
        @self.oauth2.required(scopes=['one'])
        def one():
            return 'Hello'

        @self.app.route('/two')
        @self.oauth2.required(scopes=['two', 'three'])
        def two():
            return 'Hello'

    def test_incremental_auth(self):
        self._create_incremental_auth_app()

        # No credentials, should redirect
        with self.app.test_client() as client:
            response = client.get('/one')
            self.assertIn('one', response.headers['Location'])
            self.assertEqual(response.status_code, httplib.FOUND)

        # Credentials for one. /one should allow, /two should redirect.
        credentials = self._generate_credentials(scopes=['email', 'one'])

        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_credentials'] = credentials.to_json()

            response = client.get('/one')
            self.assertEqual(response.status_code, httplib.OK)

            response = client.get('/two')
            self.assertIn('two', response.headers['Location'])
            self.assertEqual(response.status_code, httplib.FOUND)

            # Starting the authorization flow should include the
            # include_granted_scopes parameter as well as the scopes.
            response = client.get(response.headers['Location'][17:])
            q = urlparse.parse_qs(
                response.headers['Location'].split('?', 1)[1])
            self.assertIn('include_granted_scopes', q)
            self.assertEqual(
                set(q['scope'][0].split(' ')),
                set(['one', 'email', 'two', 'three']))

        # Actually call two() without a redirect.
        credentials2 = self._generate_credentials(
            scopes=['email', 'two', 'three'])

        with self.app.test_client() as client:
            with client.session_transaction() as session:
                session['google_oauth2_credentials'] = credentials2.to_json()

            response = client.get('/two')
            self.assertEqual(response.status_code, httplib.OK)

    def test_incremental_auth_exchange(self):
        self._create_incremental_auth_app()

        with mock.patch('oauth2client.transport.get_http_object') as new_http:
            # Set-up mock.
            new_http.return_value = http_mock.HttpMock(data=DEFAULT_RESP)
            # Run tests.
            with self.app.test_client() as client:
                state = self._setup_callback_state(
                    client,
                    return_url='/return_url',
                    # Incremental auth scopes.
                    scopes=['one', 'two'])

                response = client.get(
                    '/oauth2callback?state={0}&code=codez'.format(state))
                self.assertEqual(response.status_code, httplib.FOUND)

                credentials = self.oauth2.credentials
                self.assertTrue(
                    credentials.has_scopes(['email', 'one', 'two']))

            # Check the mocks were called.
            new_http.assert_called_once_with()

    def test_refresh(self):
        token_val = 'new_token'
        json_resp = '{"access_token": "%s"}' % (token_val,)
        http = http_mock.HttpMock(data=json_resp)
        with self.app.test_request_context():
            with mock.patch('flask.session'):
                self.oauth2.storage.put(self._generate_credentials())

                self.oauth2.credentials.refresh(http)

                self.assertEqual(
                    self.oauth2.storage.get().access_token, token_val)

    def test_delete(self):
        with self.app.test_request_context():

            self.oauth2.storage.put(self._generate_credentials())
            self.oauth2.storage.delete()

            self.assertNotIn('google_oauth2_credentials', flask.session)
