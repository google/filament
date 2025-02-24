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

"""HTTP helpers mock functionality."""


from six.moves import http_client


class ResponseMock(dict):
    """Mock HTTP response"""

    def __init__(self, vals=None):
        if vals is None:
            vals = {}
        self.update(vals)
        self.status = int(self.get('status', http_client.OK))


class HttpMock(object):
    """Mock of HTTP object."""

    def __init__(self, headers=None, data=None):
        """HttpMock constructor.

        Args:
            headers: dict, header to return with response
        """
        if headers is None:
            headers = {'status': http_client.OK}
        self.data = data
        self.response_headers = headers
        self.headers = None
        self.uri = None
        self.method = None
        self.body = None
        self.headers = None
        self.requests = 0

    def request(self, uri,
                method='GET',
                body=None,
                headers=None,
                redirections=1,
                connection_type=None):
        self.uri = uri
        self.method = method
        self.body = body
        self.headers = headers
        self.redirections = redirections
        self.requests += 1
        return ResponseMock(self.response_headers), self.data


class HttpMockSequence(object):
    """Mock of HTTP object with multiple return values.

    Mocks a sequence of calls to request returning different responses for each
    call. Create an instance initialized with the desired response headers
    and content and then use as if an HttpMock instance::

        http = HttpMockSequence([
            ({'status': '401'}, b''),
            ({'status': '200'}, b'{"access_token":"1/3w","expires_in":3600}'),
            ({'status': '200'}, 'echo_request_headers'),
        ])
        resp, content = http.request('http://examples.com')

    There are special values you can pass in for content to trigger
    behavours that are helpful in testing.

    * 'echo_request_headers' means return the request headers in the response
       body
    * 'echo_request_body' means return the request body in the response body
    """

    def __init__(self, iterable):
        """HttpMockSequence constructor.

        Args:
            iterable: iterable, a sequence of pairs of (headers, body)
        """
        self._iterable = iterable
        self.requests = []

    def request(self, uri,
                method='GET',
                body=None,
                headers=None,
                redirections=1,
                connection_type=None):
        resp, content = self._iterable.pop(0)
        self.requests.append({
            'method': method,
            'uri': uri,
            'body': body,
            'headers': headers,
        })
        # Read any underlying stream before sending the request.
        body_stream_content = (body.read()
                               if getattr(body, 'read', None) else None)
        if content == 'echo_request_headers':
            content = headers
        elif content == 'echo_request_body':
            content = (body
                       if body_stream_content is None else body_stream_content)
        return ResponseMock(resp), content


class CacheMock(object):

    def __init__(self):
        self.cache = {}

    def get(self, key, namespace=''):
        # ignoring namespace for easier testing
        return self.cache.get(key, None)
