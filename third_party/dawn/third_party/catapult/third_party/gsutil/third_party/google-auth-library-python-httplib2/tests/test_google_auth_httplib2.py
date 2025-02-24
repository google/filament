# Copyright 2016 Google Inc.
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

import http.client
import io
from unittest import mock

import httplib2

import google_auth_httplib2
from tests import compliance


class MockHttp(object):
    def __init__(self, responses, headers=None):
        self.responses = responses
        self.requests = []
        self.headers = headers or {}
        self.add_certificate = mock.Mock(return_value=None)

    def request(
        self,
        url,
        method="GET",
        body=None,
        headers=None,
        redirections=httplib2.DEFAULT_MAX_REDIRECTS,
        connection_type=None,
    ):
        self.requests.append(
            (method, url, body, headers, redirections, connection_type)
        )
        return self.responses.pop(0)


class MockResponse(object):
    def __init__(self, status=http.client.OK, data=b""):
        self.status = status
        self.data = data

    def __iter__(self):
        yield self
        yield self.data


class TestRequestResponse(compliance.RequestResponseTests):
    def make_request(self):
        http = httplib2.Http()
        return google_auth_httplib2.Request(http)

    def test_timeout(self):
        url = "http://example.com"
        http = MockHttp(responses=[MockResponse()])
        request = google_auth_httplib2.Request(http)
        request(url=url, method="GET", timeout=5)

        assert http.requests[0] == (
            "GET",
            url,
            None,
            None,
            httplib2.DEFAULT_MAX_REDIRECTS,
            None,
        )


def test__make_default_http():
    http = google_auth_httplib2._make_default_http()
    assert isinstance(http, httplib2.Http)


class MockCredentials(object):
    def __init__(self, token="token"):
        self.token = token

    def apply(self, headers):
        headers["authorization"] = self.token

    def before_request(self, request, method, url, headers):
        self.apply(headers)

    def refresh(self, request):
        self.token += "1"


class TestAuthorizedHttp(object):
    TEST_URL = "http://example.com"

    def test_authed_http_defaults(self):
        authed_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)

        assert authed_http.credentials == mock.sentinel.credentials
        assert isinstance(authed_http.http, httplib2.Http)

    def test_close(self):
        with mock.patch("httplib2.Http.close", autospec=True) as close:
            authed_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)
            authed_http.close()

            close.assert_called_once()

    def test_connections(self):
        authed_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)

        assert authed_http.connections == authed_http.http.connections

        authed_http.connections = mock.sentinel.connections
        assert authed_http.http.connections == mock.sentinel.connections

    def test_follow_redirects(self):
        auth_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)

        assert auth_http.follow_redirects == auth_http.http.follow_redirects

        mock_follow_redirects = mock.sentinel.follow_redirects
        auth_http.follow_redirects = mock_follow_redirects
        assert auth_http.http.follow_redirects == mock_follow_redirects

    def test_timeout(self):
        authed_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)

        assert authed_http.timeout == authed_http.http.timeout

        authed_http.timeout = mock.sentinel.timeout
        assert authed_http.http.timeout == mock.sentinel.timeout

    def test_redirect_codes(self):
        authed_http = google_auth_httplib2.AuthorizedHttp(mock.sentinel.credentials)

        assert authed_http.redirect_codes == authed_http.http.redirect_codes

        authed_http.redirect_codes = mock.sentinel.redirect_codes
        assert authed_http.http.redirect_codes == mock.sentinel.redirect_codes

    def test_add_certificate(self):
        authed_http = google_auth_httplib2.AuthorizedHttp(
            mock.sentinel.credentials, http=MockHttp(MockResponse())
        )

        authed_http.add_certificate("key", "cert", "domain", password="password")
        authed_http.http.add_certificate.assert_called_once_with(
            "key", "cert", "domain", password="password"
        )

    def test_request_no_refresh(self):
        mock_credentials = mock.Mock(wraps=MockCredentials())
        mock_response = MockResponse()
        mock_http = MockHttp([mock_response])

        authed_http = google_auth_httplib2.AuthorizedHttp(
            mock_credentials, http=mock_http
        )

        response, data = authed_http.request(self.TEST_URL)

        assert response == mock_response
        assert data == mock_response.data
        assert mock_credentials.before_request.called
        assert not mock_credentials.refresh.called
        assert mock_http.requests == [
            (
                "GET",
                self.TEST_URL,
                None,
                {"authorization": "token"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            )
        ]

    def test_request_refresh(self):
        mock_credentials = mock.Mock(wraps=MockCredentials())
        mock_final_response = MockResponse(status=http.client.OK)
        # First request will 401, second request will succeed.
        mock_http = MockHttp(
            [MockResponse(status=http.client.UNAUTHORIZED), mock_final_response]
        )

        authed_http = google_auth_httplib2.AuthorizedHttp(
            mock_credentials, http=mock_http
        )

        response, data = authed_http.request(self.TEST_URL)

        assert response == mock_final_response
        assert data == mock_final_response.data
        assert mock_credentials.before_request.call_count == 2
        assert mock_credentials.refresh.called
        assert mock_http.requests == [
            (
                "GET",
                self.TEST_URL,
                None,
                {"authorization": "token"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            ),
            (
                "GET",
                self.TEST_URL,
                None,
                {"authorization": "token1"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            ),
        ]

    def test_request_stream_body(self):
        mock_credentials = mock.Mock(wraps=MockCredentials())
        mock_response = MockResponse()
        # Refresh is needed to cover the resetting of the body position.
        mock_http = MockHttp(
            [MockResponse(status=http.client.UNAUTHORIZED), mock_response]
        )

        body = io.StringIO("body")
        body.seek(1)

        authed_http = google_auth_httplib2.AuthorizedHttp(
            mock_credentials, http=mock_http
        )

        response, data = authed_http.request(self.TEST_URL, method="POST", body=body)

        assert response == mock_response
        assert data == mock_response.data
        assert mock_http.requests == [
            (
                "POST",
                self.TEST_URL,
                body,
                {"authorization": "token"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            ),
            (
                "POST",
                self.TEST_URL,
                body,
                {"authorization": "token1"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            ),
        ]

    def test_request_positional_args(self):
        """Verifies that clients can pass args to request as positioanls."""
        mock_credentials = mock.Mock(wraps=MockCredentials())
        mock_response = MockResponse()
        mock_http = MockHttp([mock_response])

        authed_http = google_auth_httplib2.AuthorizedHttp(
            mock_credentials, http=mock_http
        )

        response, data = authed_http.request(
            self.TEST_URL, "GET", None, None, httplib2.DEFAULT_MAX_REDIRECTS, None
        )

        assert response == mock_response
        assert data == mock_response.data
        assert mock_credentials.before_request.called
        assert not mock_credentials.refresh.called
        assert mock_http.requests == [
            (
                "GET",
                self.TEST_URL,
                None,
                {"authorization": "token"},
                httplib2.DEFAULT_MAX_REDIRECTS,
                None,
            )
        ]
