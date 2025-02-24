# (c) 2005 Ian Bicking and contributors; written for Paste
# (http://pythonpaste.org)
# Licensed under the MIT license:
# http://www.opensource.org/licenses/mit-license.php
"""
Routines for testing WSGI applications.

Most interesting is TestApp
"""
from __future__ import unicode_literals

import os
import re
import json
import random
import fnmatch
import mimetypes

from base64 import b64encode

from six import StringIO
from six import BytesIO
from six import string_types
from six import binary_type
from six import text_type
from six.moves import http_cookiejar

from webtest.compat import urlparse
from webtest.compat import urlencode
from webtest.compat import to_bytes
from webtest.compat import escape_cookie_value
from webtest.response import TestResponse
from webtest import forms
from webtest import lint
from webtest import utils

import webob


__all__ = ['TestApp', 'TestRequest']


class AppError(Exception):

    def __init__(self, message, *args):
        if isinstance(message, binary_type):
            message = message.decode('utf8')
        str_args = ()
        for arg in args:
            if isinstance(arg, webob.Response):
                body = arg.body
                if isinstance(body, binary_type):
                    if arg.charset:
                        arg = body.decode(arg.charset)
                    else:
                        arg = repr(body)
            elif isinstance(arg, binary_type):
                try:
                    arg = arg.decode('utf8')
                except UnicodeDecodeError:
                    arg = repr(arg)
            str_args += (arg,)
        message = message % str_args
        Exception.__init__(self, message)


class CookiePolicy(http_cookiejar.DefaultCookiePolicy):
    """A subclass of DefaultCookiePolicy to allow cookie set for
    Domain=localhost."""

    def return_ok_domain(self, cookie, request):
        if cookie.domain == '.localhost':
            return True
        return http_cookiejar.DefaultCookiePolicy.return_ok_domain(
            self, cookie, request)

    def set_ok_domain(self, cookie, request):
        if cookie.domain == '.localhost':
            return True
        return http_cookiejar.DefaultCookiePolicy.set_ok_domain(
            self, cookie, request)


class TestRequest(webob.BaseRequest):
    """A subclass of webob.Request"""
    ResponseClass = TestResponse


class TestApp(object):
    """
    Wraps a WSGI application in a more convenient interface for
    testing. It uses extended version of :class:`webob.BaseRequest`
    and :class:`webob.Response`.

    :param app:
        May be an WSGI application or Paste Deploy app,
        like ``'config:filename.ini#test'``.

        .. versionadded:: 2.0

        It can also be an actual full URL to an http server and webtest
        will proxy requests with `WSGIProxy2
        <https://pypi.python.org/pypi/WSGIProxy2/>`_.
    :type app:
        WSGI application
    :param extra_environ:
        A dictionary of values that should go
        into the environment for each request. These can provide a
        communication channel with the application.
    :type extra_environ:
        dict
    :param relative_to:
        A directory used for file
        uploads are calculated relative to this.  Also ``config:``
        URIs that aren't absolute.
    :type relative_to:
        string
    :param cookiejar:
        :class:`cookielib.CookieJar` alike API that keeps cookies
        across requets.
    :type cookiejar:
        CookieJar instance

    .. attribute:: cookies

        A convenient shortcut for a dict of all cookies in
        ``cookiejar``.

    :param parser_features:
        Passed to BeautifulSoup when parsing responses.
    :type parser_features:
        string or list
    :param json_encoder:
        Passed to json.dumps when encoding json
    :type json_encoder:
        A subclass of json.JSONEncoder
    :param lint:
        If True (default) then check that the application is WSGI compliant
    :type lint:
        A boolean
    """

    RequestClass = TestRequest

    def __init__(self, app, extra_environ=None, relative_to=None,
                 use_unicode=True, cookiejar=None, parser_features=None,
                 json_encoder=None, lint=True):

        if 'WEBTEST_TARGET_URL' in os.environ:
            app = os.environ['WEBTEST_TARGET_URL']
        if isinstance(app, string_types):
            if app.startswith('http'):
                try:
                    from wsgiproxy import HostProxy
                except ImportError:  # pragma: no cover
                    raise ImportError((
                        'Using webtest with a real url requires WSGIProxy2. '
                        'Please install it with: '
                        'pip install WSGIProxy2'))
                if '#' not in app:
                    app += '#httplib'
                url, client = app.split('#', 1)
                app = HostProxy(url, client=client)
            else:
                from paste.deploy import loadapp
                # @@: Should pick up relative_to from calling module's
                # __file__
                app = loadapp(app, relative_to=relative_to)
        self.app = app
        self.lint = lint
        self.relative_to = relative_to
        if extra_environ is None:
            extra_environ = {}
        self.extra_environ = extra_environ
        self.use_unicode = use_unicode
        if cookiejar is None:
            cookiejar = http_cookiejar.CookieJar(policy=CookiePolicy())
        self.cookiejar = cookiejar
        if parser_features is None:
            parser_features = 'html.parser'
        self.RequestClass.ResponseClass.parser_features = parser_features
        if json_encoder is None:
            json_encoder = json.JSONEncoder
        self.JSONEncoder = json_encoder

    def get_authorization(self):
        """Allow to set the HTTP_AUTHORIZATION environ key. Value should looks
        like ``('Basic', ('user', 'password'))``

        If value is None the the HTTP_AUTHORIZATION is removed
        """
        return self.authorization_value

    def set_authorization(self, value):
        self.authorization_value = value
        if value is not None:
            invalid_value = (
                "You should use a value like ('Basic', ('user', 'password'))"
            )
            if isinstance(value, (list, tuple)) and len(value) == 2:
                authtype, val = value
                if authtype == 'Basic' and val and \
                   isinstance(val, (list, tuple)):
                    val = ':'.join(list(val))
                    val = b64encode(to_bytes(val)).strip()
                    val = val.decode('latin1')
                else:
                    raise ValueError(invalid_value)
                value = str('%s %s' % (authtype, val))
            else:
                raise ValueError(invalid_value)
            self.extra_environ.update({
                'HTTP_AUTHORIZATION': value,
            })
        else:
            if 'HTTP_AUTHORIZATION' in self.extra_environ:
                del self.extra_environ['HTTP_AUTHORIZATION']

    authorization = property(get_authorization, set_authorization)

    @property
    def cookies(self):
        return dict([(cookie.name, cookie.value) for cookie in self.cookiejar])

    def set_cookie(self, name, value):
        """
        Sets a cookie to be passed through with requests.

        """
        value = escape_cookie_value(value)
        cookie = http_cookiejar.Cookie(
            version=0,
            name=name,
            value=value,
            port=None,
            port_specified=False,
            domain='.localhost',
            domain_specified=True,
            domain_initial_dot=False,
            path='/',
            path_specified=True,
            secure=False,
            expires=None,
            discard=False,
            comment=None,
            comment_url=None,
            rest=None
        )
        self.cookiejar.set_cookie(cookie)

    def reset(self):
        """
        Resets the state of the application; currently just clears
        saved cookies.
        """
        self.cookiejar.clear()

    def set_parser_features(self, parser_features):
        """
        Changes the parser used by BeautifulSoup. See its documentation to
        know the supported parsers.
        """
        self.RequestClass.ResponseClass.parser_features = parser_features

    def get(self, url, params=None, headers=None, extra_environ=None,
            status=None, expect_errors=False, xhr=False):
        """
        Do a GET request given the url path.

        :param params:
            A query string, or a dictionary that will be encoded
            into a query string.  You may also include a URL query
            string on the ``url``.
        :param headers:
            Extra headers to send.
        :type headers:
            dictionary
        :param extra_environ:
            Environmental variables that should be added to the request.
        :type extra_environ:
            dictionary
        :param status:
            The HTTP status code you expect in response (if not 200 or 3xx).
            You can also use a wildcard, like ``'3*'`` or ``'*'``.
        :type status:
            integer or string
        :param expect_errors:
            If this is False, then if anything is written to
            environ ``wsgi.errors`` it will be an error.
            If it is True, then non-200/3xx responses are also okay.
        :type expect_errors:
            boolean
        :param xhr:
            If this is true, then marks response as ajax. The same as
            headers={'X-REQUESTED-WITH': 'XMLHttpRequest', }
        :type xhr:
            boolean

        :returns: :class:`webtest.TestResponse` instance.

        """
        environ = self._make_environ(extra_environ)
        url = str(url)
        url = self._remove_fragment(url)
        if params:
            if not isinstance(params, string_types):
                params = urlencode(params, doseq=True)
            if str('?') in url:
                url += str('&')
            else:
                url += str('?')
            url += params
        if str('?') in url:
            url, environ['QUERY_STRING'] = url.split(str('?'), 1)
        else:
            environ['QUERY_STRING'] = str('')
        req = self.RequestClass.blank(url, environ)
        if xhr:
            headers = self._add_xhr_header(headers)
        if headers:
            req.headers.update(headers)
        return self.do_request(req, status=status,
                               expect_errors=expect_errors)

    def post(self, url, params='', headers=None, extra_environ=None,
             status=None, upload_files=None, expect_errors=False,
             content_type=None, xhr=False):
        """
        Do a POST request. Similar to :meth:`~webtest.TestApp.get`.

        :param params:
            Are put in the body of the request. If params is a
            iterator it will be urlencoded, if it is string it will not
            be encoded, but placed in the body directly.

            Can be a collections.OrderedDict with
            :class:`webtest.forms.Upload` fields included::


            app.post('/myurl', collections.OrderedDict([
                ('textfield1', 'value1'),
                ('uploadfield', webapp.Upload('filename.txt', 'contents'),
                ('textfield2', 'value2')])))

        :param upload_files:
            It should be a list of ``(fieldname, filename, file_content)``.
            You can also use just ``(fieldname, filename)`` and the file
            contents will be read from disk.
        :type upload_files:
            list
        :param content_type:
            HTTP content type, for example `application/json`.
        :type content_type:
            string

        :param xhr:
            If this is true, then marks response as ajax. The same as
            headers={'X-REQUESTED-WITH': 'XMLHttpRequest', }
        :type xhr:
            boolean

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('POST', url, params=params, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=upload_files,
                                 expect_errors=expect_errors,
                                 content_type=content_type)

    def put(self, url, params='', headers=None, extra_environ=None,
            status=None, upload_files=None, expect_errors=False,
            content_type=None, xhr=False):
        """
        Do a PUT request. Similar to :meth:`~webtest.TestApp.post`.

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('PUT', url, params=params, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=upload_files,
                                 expect_errors=expect_errors,
                                 content_type=content_type,
                                 )

    def patch(self, url, params='', headers=None, extra_environ=None,
              status=None, upload_files=None, expect_errors=False,
              content_type=None, xhr=False):
        """
        Do a PATCH request. Similar to :meth:`~webtest.TestApp.post`.

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('PATCH', url, params=params, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=upload_files,
                                 expect_errors=expect_errors,
                                 content_type=content_type)

    def delete(self, url, params='', headers=None,
               extra_environ=None, status=None, expect_errors=False,
               content_type=None, xhr=False):
        """
        Do a DELETE request. Similar to :meth:`~webtest.TestApp.get`.

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('DELETE', url, params=params, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=None,
                                 expect_errors=expect_errors,
                                 content_type=content_type)

    def options(self, url, headers=None, extra_environ=None,
                status=None, expect_errors=False, xhr=False):
        """
        Do a OPTIONS request. Similar to :meth:`~webtest.TestApp.get`.

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('OPTIONS', url, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=None,
                                 expect_errors=expect_errors)

    def head(self, url, headers=None, extra_environ=None,
             status=None, expect_errors=False, xhr=False):
        """
        Do a HEAD request. Similar to :meth:`~webtest.TestApp.get`.

        :returns: :class:`webtest.TestResponse` instance.

        """
        if xhr:
            headers = self._add_xhr_header(headers)
        return self._gen_request('HEAD', url, headers=headers,
                                 extra_environ=extra_environ, status=status,
                                 upload_files=None,
                                 expect_errors=expect_errors)

    post_json = utils.json_method('POST')
    put_json = utils.json_method('PUT')
    patch_json = utils.json_method('PATCH')
    delete_json = utils.json_method('DELETE')

    def encode_multipart(self, params, files):
        """
        Encodes a set of parameters (typically a name/value list) and
        a set of files (a list of (name, filename, file_body, mimetype)) into a
        typical POST body, returning the (content_type, body).

        """
        boundary = to_bytes(str(random.random()))[2:]
        boundary = b'----------a_BoUnDaRy' + boundary + b'$'
        lines = []

        def _append_file(file_info):
            key, filename, value, fcontent = self._get_file_info(file_info)
            if isinstance(key, text_type):
                try:
                    key = key.encode('ascii')
                except:  # pragma: no cover
                    raise  # file name must be ascii
            if isinstance(filename, text_type):
                try:
                    filename = filename.encode('utf8')
                except:  # pragma: no cover
                    raise  # file name must be ascii or utf8
            if not fcontent:
                fcontent = mimetypes.guess_type(filename.decode('utf8'))[0]
            fcontent = to_bytes(fcontent)
            fcontent = fcontent or b'application/octet-stream'
            lines.extend([
                b'--' + boundary,
                b'Content-Disposition: form-data; ' +
                b'name="' + key + b'"; filename="' + filename + b'"',
                b'Content-Type: ' + fcontent, b'', value])

        for key, value in params:
            if isinstance(key, text_type):
                try:
                    key = key.encode('ascii')
                except:  # pragma: no cover
                    raise  # field name are always ascii
            if isinstance(value, forms.File):
                if value.value:
                    _append_file([key] + list(value.value))
            elif isinstance(value, forms.Upload):
                file_info = [key, value.filename]
                if value.content is not None:
                    file_info.append(value.content)
                    if value.content_type is not None:
                        file_info.append(value.content_type)
                _append_file(file_info)
            else:
                if isinstance(value, text_type):
                    value = value.encode('utf8')
                lines.extend([
                    b'--' + boundary,
                    b'Content-Disposition: form-data; name="' + key + b'"',
                    b'', value])

        for file_info in files:
            _append_file(file_info)

        lines.extend([b'--' + boundary + b'--', b''])
        body = b'\r\n'.join(lines)
        boundary = boundary.decode('ascii')
        content_type = 'multipart/form-data; boundary=%s' % boundary
        return content_type, body

    def request(self, url_or_req, status=None, expect_errors=False,
                **req_params):
        """
        Creates and executes a request. You may either pass in an
        instantiated :class:`TestRequest` object, or you may pass in a
        URL and keyword arguments to be passed to
        :meth:`TestRequest.blank`.

        You can use this to run a request without the intermediary
        functioning of :meth:`TestApp.get` etc.  For instance, to
        test a WebDAV method::

            resp = app.request('/new-col', method='MKCOL')

        Note that the request won't have a body unless you specify it,
        like::

            resp = app.request('/test.txt', method='PUT', body='test')

        You can use :class:`webtest.TestRequest`::

            req = webtest.TestRequest.blank('/url/', method='GET')
            resp = app.do_request(req)

        """
        if isinstance(url_or_req, text_type):
            url_or_req = str(url_or_req)
        for (k, v) in req_params.items():
            if isinstance(v, text_type):
                req_params[k] = str(v)
        if isinstance(url_or_req, string_types):
            req = self.RequestClass.blank(url_or_req, **req_params)
        else:
            req = url_or_req.copy()
            for name, value in req_params.items():
                setattr(req, name, value)
        req.environ['paste.throw_errors'] = True
        for name, value in self.extra_environ.items():
            req.environ.setdefault(name, value)
        return self.do_request(req,
                               status=status,
                               expect_errors=expect_errors,
                               )

    def do_request(self, req, status=None, expect_errors=None):
        """
        Executes the given webob Request (``req``), with the expected
        ``status``.  Generally :meth:`~webtest.TestApp.get` and
        :meth:`~webtest.TestApp.post` are used instead.

        To use this::

            req = webtest.TestRequest.blank('url', ...args...)
            resp = app.do_request(req)

        .. note::

            You can pass any keyword arguments to
            ``TestRequest.blank()``, which will be set on the request.
            These can be arguments like ``content_type``, ``accept``, etc.

        """

        errors = StringIO()
        req.environ['wsgi.errors'] = errors
        script_name = req.environ.get('SCRIPT_NAME', '')
        if script_name and req.path_info.startswith(script_name):
            req.path_info = req.path_info[len(script_name):]

        # set framework hooks
        req.environ['paste.testing'] = True
        req.environ['paste.testing_variables'] = {}

        # set request cookies
        self.cookiejar.add_cookie_header(utils._RequestCookieAdapter(req))

        # verify wsgi compatibility
        app = lint.middleware(self.app) if self.lint else self.app

        ## FIXME: should it be an option to not catch exc_info?
        res = req.get_response(app, catch_exc_info=True)

        # be sure to decode the content
        res.decode_content()

        # set a few handy attributes
        res._use_unicode = self.use_unicode
        res.request = req
        res.app = app
        res.test_app = self

        # We do this to make sure the app_iter is exausted:
        try:
            res.body
        except TypeError:  # pragma: no cover
            pass
        res.errors = errors.getvalue()

        for name, value in req.environ['paste.testing_variables'].items():
            if hasattr(res, name):
                raise ValueError(
                    "paste.testing_variables contains the variable %r, but "
                    "the response object already has an attribute by that "
                    "name" % name)
            setattr(res, name, value)
        if not expect_errors:
            self._check_status(status, res)
            self._check_errors(res)

        # merge cookies back in
        self.cookiejar.extract_cookies(utils._ResponseCookieAdapter(res),
                                       utils._RequestCookieAdapter(req))

        return res

    def _check_status(self, status, res):
        if status == '*':
            return
        res_status = res.status
        if (isinstance(status, string_types) and '*' in status):
            if re.match(fnmatch.translate(status), res_status, re.I):
                return
        if isinstance(status, string_types):
            if status == res_status:
                return
        if isinstance(status, (list, tuple)):
            if res.status_int not in status:
                raise AppError(
                    "Bad response: %s (not one of %s for %s)\n%s",
                    res_status, ', '.join(map(str, status)),
                    res.request.url, res)
            return
        if status is None:
            if res.status_int >= 200 and res.status_int < 400:
                return
            raise AppError(
                "Bad response: %s (not 200 OK or 3xx redirect for %s)\n%s",
                res_status, res.request.url,
                res)
        if status != res.status_int:
            raise AppError(
                "Bad response: %s (not %s)", res_status, status)

    def _check_errors(self, res):
        errors = res.errors
        if errors:
            raise AppError(
                "Application had errors logged:\n%s", errors)

    def _make_environ(self, extra_environ=None):
        environ = self.extra_environ.copy()
        environ['paste.throw_errors'] = True
        if extra_environ:
            environ.update(extra_environ)
        return environ

    def _remove_fragment(self, url):
        scheme, netloc, path, query, fragment = urlparse.urlsplit(url)
        return urlparse.urlunsplit((scheme, netloc, path, query, ""))

    def _gen_request(self, method, url, params=utils.NoDefault,
                     headers=None, extra_environ=None, status=None,
                     upload_files=None, expect_errors=False,
                     content_type=None):
        """
        Do a generic request.
        """

        environ = self._make_environ(extra_environ)

        inline_uploads = []

        # this supports OrderedDict
        if isinstance(params, dict) or hasattr(params, 'items'):
            params = list(params.items())

        if isinstance(params, (list, tuple)):
            inline_uploads = [v for (k, v) in params
                              if isinstance(v, (forms.File, forms.Upload))]

        if len(inline_uploads) > 0:
            content_type, params = self.encode_multipart(
                params, upload_files or ())
            environ['CONTENT_TYPE'] = content_type
        else:
            params = utils.encode_params(params, content_type)
            if upload_files or \
                (content_type and
                 to_bytes(content_type).startswith(b'multipart')):
                params = urlparse.parse_qsl(params, keep_blank_values=True)
                content_type, params = self.encode_multipart(
                    params, upload_files or ())
                environ['CONTENT_TYPE'] = content_type
            elif params:
                environ.setdefault('CONTENT_TYPE',
                                   str('application/x-www-form-urlencoded'))

        if content_type is not None:
            environ['CONTENT_TYPE'] = content_type
        environ['REQUEST_METHOD'] = str(method)
        url = str(url)
        url = self._remove_fragment(url)
        req = self.RequestClass.blank(url, environ)
        if isinstance(params, text_type):
            params = params.encode(req.charset or 'utf8')
        req.environ['wsgi.input'] = BytesIO(params)
        req.content_length = len(params)
        if headers:
            req.headers.update(headers)
        return self.do_request(req, status=status,
                               expect_errors=expect_errors)

    def _get_file_info(self, file_info):
        if len(file_info) == 2:
            # It only has a filename
            filename = file_info[1]
            if self.relative_to:
                filename = os.path.join(self.relative_to, filename)
            f = open(filename, 'rb')
            content = f.read()
            f.close()
            return (file_info[0], filename, content, None)
        elif 3 <= len(file_info) <= 4:
            content = file_info[2]
            if not isinstance(content, binary_type):
                raise ValueError('File content must be %s not %s'
                                 % (binary_type, type(content)))
            if len(file_info) == 3:
                return tuple(file_info) + (None,)
            else:
                return file_info
        else:
            raise ValueError(
                "upload_files need to be a list of tuples of (fieldname, "
                "filename, filecontent, mimetype) or (fieldname, "
                "filename, filecontent) or (fieldname, filename); "
                "you gave: %r"
                % repr(file_info)[:100])

    @staticmethod
    def _add_xhr_header(headers):
        headers = headers or {}
        # if remove str we will be have an error in lint.middleware
        headers.update({'X-REQUESTED-WITH': str('XMLHttpRequest')})
        return headers
