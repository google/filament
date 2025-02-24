# -*- coding: utf-8 -*-
import re
import six
from json import dumps

from webtest.compat import urlencode


class NoDefault(object):
    """Sentinel to uniquely represent no default value."""

    def __repr__(self):
        return '<NoDefault>'

NoDefault = NoDefault()


def json_method(method):
    """Do a %(method)s request.  Very like the
    :class:`~webtest.TestApp.%(lmethod)s` method.

    ``params`` are dumped to json and put in the body of the request.
    Content-Type is set to ``application/json``.

    Returns a :class:`webtest.TestResponse` object.
    """

    def wrapper(self, url, params=NoDefault, **kw):
        content_type = 'application/json'
        if params is not NoDefault:
            params = dumps(params, cls=self.JSONEncoder)
        kw.update(
            params=params,
            content_type=content_type,
            upload_files=None,
        )
        return self._gen_request(method, url, **kw)

    subst = dict(lmethod=method.lower(), method=method)
    wrapper.__doc__ = json_method.__doc__ % subst
    wrapper.__name__ = str('%(lmethod)s_json' % subst)

    return wrapper


def stringify(value):
    if isinstance(value, six.text_type):
        return value
    elif isinstance(value, six.binary_type):
        return value.decode('utf8')
    else:
        return str(value)


entity_pattern = re.compile(r"&(\w+|#\d+|#[xX][a-fA-F0-9]+);")


def encode_params(params, content_type):
    if params is NoDefault:
        return ''
    if isinstance(params, dict) or hasattr(params, 'items'):
        params = list(params.items())
    if isinstance(params, (list, tuple)):
        if content_type:
            content_type = content_type.lower()
            if 'charset=' in content_type:
                charset = content_type.split('charset=')[1]
                charset = charset.strip('; ').lower()
                encoded_params = []
                for k, v in params:
                    if isinstance(v, six.text_type):
                        v = v.encode(charset)
                    encoded_params.append((k, v))
                params = encoded_params
        params = urlencode(params, doseq=True)
    return params


def make_pattern(pat):
    """Find element pattern can be a regex or a callable."""
    if pat is None:
        return None
    if isinstance(pat, six.binary_type):
        pat = pat.decode('utf8')
    if isinstance(pat, six.text_type):
        pat = re.compile(pat)
    if hasattr(pat, 'search'):
        return pat.search
    if hasattr(pat, '__call__'):
        return pat
    raise ValueError(
        "Cannot make callable pattern object out of %r" % pat)


class _RequestCookieAdapter(object):
    """
    cookielib.CookieJar support for webob.Request
    """
    def __init__(self, request):
        self._request = request
        self.origin_req_host = request.host

    def is_unverifiable(self):
        return True  # sure? Why not?

    @property
    def unverifiable(self):  # NOQA
        # This is undocumented method that Python 3 cookielib uses
        return True

    def get_full_url(self):
        return self._request.url

    def get_host(self):
        return self.origin_req_host
    get_origin_req_host = get_host

    def add_unredirected_header(self, key, header):
        self._request.headers[key] = header

    def has_header(self, key):
        return key in self._request.headers

    def get_type(self):
        return self._request.scheme

    @property
    def type(self):  # NOQA
        # This is undocumented method that Python 3 cookielib uses
        return self.get_type()

    def header_items(self):  # pragma: no cover
        # This is unused on most python versions
        return self._request.headers.items()


class _ResponseCookieAdapter(object):
    """
    cookielib.CookieJar support for webob.Response
    """
    def __init__(self, response):
        self._response = response

    def info(self):
        return self

    def getheaders(self, header):
        return self._response.headers.getall(header)

    def get_all(self, headers, default):  # NOQA
        # This is undocumented method that Python 3 cookielib uses
        return self._response.headers.getall(headers)
