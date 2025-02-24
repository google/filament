# -*- coding: utf-8 -*-
"""
    webapp2
    =======

    Taking Google App Engine's webapp to the next level!

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
from __future__ import with_statement

from __future__ import absolute_import
import cgi
import inspect
import logging
import os
import re
import sys
import threading
import traceback
import six.moves.urllib.request, six.moves.urllib.parse, six.moves.urllib.error
from wsgiref import handlers

import webob
from webob import exc
import six
from six.moves import range

_webapp = _webapp_util = _local = None

try: # pragma: no cover
    # WebOb < 1.0 (App Engine Python 2.5).
    from webob.statusreasons import status_reasons
    from webob.headerdict import HeaderDict as BaseResponseHeaders
except ImportError: # pragma: no cover
    # WebOb >= 1.0.
    from webob.util import status_reasons
    from webob.headers import ResponseHeaders as BaseResponseHeaders

# google.appengine.ext.webapp imports webapp2 in the
# App Engine Python 2.7 runtime.
if os.environ.get('APPENGINE_RUNTIME') != 'python27': # pragma: no cover
    try:
        from google.appengine.ext import webapp as _webapp
    except ImportError: # pragma: no cover
        # Running webapp2 outside of GAE.
        pass
    # webapp is not python 3 compatible. Importing it will raise syntax error.
    except SyntaxError: # pragma: no cover
        pass

try: # pragma: no cover
    # Thread-local variables container.
    from webapp2_extras import local
    _local = local.Local()
except ImportError: # pragma: no cover
    logging.warning("webapp2_extras.local is not available "
                    "so webapp2 won't be thread-safe!")


__version_info__ = (2, 5, 1)
__version__ = '.'.join(str(n) for n in __version_info__)

#: Base HTTP exception, set here as public interface.
HTTPException = exc.HTTPException

#: Regex for route definitions.
_route_re = re.compile(r"""
    \<               # The exact character "<"
    ([a-zA-Z_]\w*)?  # The optional variable name
    (?:\:([^\>]*))?  # The optional :regex part
    \>               # The exact character ">"
    """, re.VERBOSE)
#: Regex extract charset from environ.
_charset_re = re.compile(r';\s*charset=([^;]*)', re.I)

#: To show exceptions in debug mode.
_debug_template = """<html>
  <head>
    <title>Internal Server Error</title>
    <style>
      body {
        padding: 20px;
        font-family: arial, sans-serif;
        font-size: 14px;
      }
      pre {
        background: #F2F2F2;
        padding: 10px;
      }
    </style>
  </head>
  <body>
    <h1>Internal Server Error</h1>
    <p>The server has either erred or is incapable of performing
    the requested operation.</p>
    <pre>%s</pre>
  </body>
</html>"""

# Set same default messages from webapp plus missing ones.
_webapp_status_reasons = {
    203: 'Non-Authoritative Information',
    302: 'Moved Temporarily',
    306: 'Unused',
    408: 'Request Time-out',
    414: 'Request-URI Too Large',
    504: 'Gateway Time-out',
    505: 'HTTP Version not supported',
}
status_reasons.update(_webapp_status_reasons)
for code, message in _webapp_status_reasons.items():
    cls = exc.status_map.get(code)
    if cls:
        cls.title = message


class Request(webob.Request):
    """Abstraction for an HTTP request.

    Most extra methods and attributes are ported from webapp. Check the
    `WebOb documentation <WebOb>`_ for the ones not listed here.
    """

    #: A reference to the active :class:`WSGIApplication` instance.
    app = None
    #: A reference to the active :class:`Response` instance.
    response = None
    #: A reference to the matched :class:`Route`.
    route = None
    #: The matched route positional arguments.
    route_args = None
    #: The matched route keyword arguments.
    route_kwargs = None
    #: A dictionary to register objects used during the request lifetime.
    registry = None
    # Attributes from webapp.
    request_body_tempfile_limit = 0
    uri = property(lambda self: self.url)
    query = property(lambda self: self.query_string)

    def __init__(self, environ, *args, **kwargs):
        """Constructs a Request object from a WSGI environment.

        :param environ:
            A WSGI-compliant environment dictionary.
        """
        if kwargs.get('charset') is None and not hasattr(webob, '__version__'):
            # webob 0.9 didn't have a __version__ attribute and also defaulted
            # to None rather than UTF-8 if no charset was provided. Providing a
            # default charset is required for backwards compatibility.
            match = _charset_re.search(environ.get('CONTENT_TYPE', ''))
            if match:
                charset = match.group(1).lower().strip().strip('"').strip()
            else:
                charset = 'utf-8'
            kwargs['charset'] = charset

        super(Request, self).__init__(environ, *args, **kwargs)
        self.registry = {}

    def get(self, argument_name, default_value='', allow_multiple=False):
        """Returns the query or POST argument with the given name.

        We parse the query string and POST payload lazily, so this will be a
        slower operation on the first call.

        :param argument_name:
            The name of the query or POST argument.
        :param default_value:
            The value to return if the given argument is not present.
        :param allow_multiple:
            Return a list of values with the given name (deprecated).
        :returns:
            If allow_multiple is False (which it is by default), we return
            the first value with the given name given in the request. If it
            is True, we always return a list.
        """
        param_value = self.get_all(argument_name)
        if allow_multiple:
            logging.warning('allow_multiple is a deprecated param. '
                'Please use the Request.get_all() method instead.')

        if len(param_value) > 0:
            if allow_multiple:
                return param_value

            return param_value[0]
        else:
            if allow_multiple and not default_value:
                return []

            return default_value

    def get_all(self, argument_name, default_value=None):
        """Returns a list of query or POST arguments with the given name.

        We parse the query string and POST payload lazily, so this will be a
        slower operation on the first call.

        :param argument_name:
            The name of the query or POST argument.
        :param default_value:
            The value to return if the given argument is not present,
            None may not be used as a default, if it is then an empty
            list will be returned instead.
        :returns:
            A (possibly empty) list of values.
        """
        if self.charset:
            argument_name = argument_name.encode(self.charset)

        if default_value is None:
            default_value = []

        param_value = self.params.getall(argument_name)

        if param_value is None or len(param_value) == 0:
            return default_value

        for i in range(len(param_value)):
            if isinstance(param_value[i], cgi.FieldStorage):
                param_value[i] = param_value[i].value

        return param_value

    def arguments(self):
        """Returns a list of the arguments provided in the query and/or POST.

        The return value is a list of strings.
        """
        return list(set(self.params.keys()))

    def get_range(self, name, min_value=None, max_value=None, default=0):
        """Parses the given int argument, limiting it to the given range.

        :param name:
            The name of the argument.
        :param min_value:
            The minimum int value of the argument (if any).
        :param max_value:
            The maximum int value of the argument (if any).
        :param default:
            The default value of the argument if it is not given.
        :returns:
            An int within the given range for the argument.
        """
        value = self.get(name, default)
        if value is None:
            return value

        try:
            value = int(value)
        except ValueError:
            value = default
            if value is not None:
                if max_value is not None:
                    value = min(value, max_value)

                if min_value is not None:
                    value = max(value, min_value)

        return value

    @classmethod
    def blank(cls, path, environ=None, base_url=None,
              headers=None, **kwargs): # pragma: no cover
        """Adds parameters compatible with WebOb >= 1.0: POST and **kwargs."""
        try:
            return super(Request, cls).blank(path, environ=environ,
                                             base_url=base_url,
                                             headers=headers, **kwargs)
        except TypeError:
            if not kwargs:
                raise

        data = kwargs.pop('POST', None)
        if data is not None:
            from cStringIO import StringIO
            environ = environ or {}
            environ['REQUEST_METHOD'] = 'POST'
            if hasattr(data, 'items'):
                data = list(data.items())
            if not isinstance(data, str):
                data = six.moves.urllib.parse.urlencode(data)
            environ['wsgi.input'] = StringIO(data)
            environ['webob.is_body_seekable'] = True
            environ['CONTENT_LENGTH'] = str(len(data))
            environ['CONTENT_TYPE'] = 'application/x-www-form-urlencoded'

        base = super(Request, cls).blank(path, environ=environ,
                                         base_url=base_url, headers=headers)
        if kwargs:
            obj = cls(base.environ, **kwargs)
            obj.headers.update(base.headers)
            return obj
        else:
            return base


class ResponseHeaders(BaseResponseHeaders):
    """Implements methods from ``wsgiref.headers.Headers``, used by webapp."""

    get_all = BaseResponseHeaders.getall

    def add_header(self, _name, _value, **_params):
        """Extended header setting.

        _name is the header field to add.  keyword arguments can be used to set
        additional parameters for the header field, with underscores converted
        to dashes.  Normally the parameter will be added as key="value" unless
        value is None, in which case only the key will be added.

        Example::

            h.add_header('content-disposition', 'attachment',
                         filename='bud.gif')

        Note that unlike the corresponding 'email.message' method, this does
        *not* handle '(charset, language, value)' tuples: all values must be
        strings or None.
        """
        parts = []
        if _value is not None:
            parts.append(_value)

        for k, v in _params.items():
            k = k.replace('_', '-')
            if v is not None and len(v) > 0:
                v = v.replace('\\', '\\\\').replace('"', r'\"')
                parts.append('%s="%s"' % (k, v))
            else:
                parts.append(k)

        self.add(_name, '; '.join(parts))

    def __str__(self):
        """Returns the formatted headers ready for HTTP transmission."""
        return '\r\n'.join(['%s: %s' % v for v in self.items()] + ['', ''])


class Response(webob.Response):
    """Abstraction for an HTTP response.

    Most extra methods and attributes are ported from webapp. Check the
    `WebOb documentation <WebOb>`_ for the ones not listed here.

    Differences from webapp.Response:

    - ``out`` is not a ``StringIO.StringIO`` instance. Instead it is the
      response itself, as it has the method ``write()``.
    - As in WebOb, ``status`` is the code plus message, e.g., '200 OK', while
      in webapp it is the integer code. The status code as an integer is
      available in ``status_int``, and the status message is available in
      ``status_message``.
    - ``response.headers`` raises an exception when a key that doesn't exist
      is accessed or deleted, differently from ``wsgiref.headers.Headers``.
    """

    #: Default charset as in webapp.
    default_charset = 'utf-8'

    def __init__(self, *args, **kwargs):
        """Constructs a response with the default settings."""
        super(Response, self).__init__(*args, **kwargs)
        self.headers['Cache-Control'] = 'no-cache'

    @property
    def out(self):
        """A reference to the Response instance itself, for compatibility with
        webapp only: webapp uses `Response.out.write()`, so we point `out` to
        `self` and it will use `Response.write()`.
        """
        return self

    def write(self, text):
        """Appends a text to the response body."""
        # webapp uses StringIO as Response.out, so we need to convert anything
        # that is not str or unicode to string to keep same behavior.
        if not isinstance(text, six.string_types):
            text = six.text_type(text)

        if isinstance(text, six.text_type) and not self.charset:
            self.charset = self.default_charset

        super(Response, self).write(text)

    def _set_status(self, value):
        """The status string, including code and message."""
        message = None
        # Accept long because urlfetch in App Engine returns codes as longs.
        if isinstance(value, six.integer_types):
            code = int(value)
        else:
            if isinstance(value, six.text_type):
                # Status messages have to be ASCII safe, so this is OK.
                value = str(value)

            if not isinstance(value, str):
                raise TypeError(
                    'You must set status to a string or integer (not %s)' %
                    type(value))

            parts = value.split(' ', 1)
            code = int(parts[0])
            if len(parts) == 2:
                message = parts[1]

        message = message or Response.http_status_message(code)
        self._status = '%d %s' % (code, message)

    def _get_status(self):
        return self._status

    status = property(_get_status, _set_status, doc=_set_status.__doc__)

    def set_status(self, code, message=None):
        """Sets the HTTP status code of this response.

        :param code:
            The HTTP status string to use
        :param message:
            A status string. If none is given, uses the default from the
            HTTP/1.1 specification.
        """
        if message:
            self.status = '%d %s' % (code, message)
        else:
            self.status = code

    def _get_status_message(self):
        """The response status message, as a string."""
        return self.status.split(' ', 1)[1]

    def _set_status_message(self, message):
        self.status = '%d %s' % (self.status_int, message)

    status_message = property(_get_status_message, _set_status_message,
                              doc=_get_status_message.__doc__)

    def _get_headers(self):
        """The headers as a dictionary-like object."""
        if self._headers is None:
            self._headers = ResponseHeaders.view_list(self.headerlist)

        return self._headers

    def _set_headers(self, value):
        if hasattr(value, 'items'):
            value = list(value.items())
        elif not isinstance(value, list):
            raise TypeError('Response headers must be a list or dictionary.')

        self.headerlist = value
        self._headers = None

    headers = property(_get_headers, _set_headers, doc=_get_headers.__doc__)

    def has_error(self):
        """Indicates whether the response was an error response."""
        return self.status_int >= 400

    def clear(self):
        """Clears all data written to the output stream so that it is empty."""
        self.body = b''

    def wsgi_write(self, start_response):
        """Writes this response using using the given WSGI function.

        This is only here for compatibility with ``webapp.WSGIApplication``.

        :param start_response:
            The WSGI-compatible start_response function.
        """
        if (self.headers.get('Cache-Control') == 'no-cache' and
            not self.headers.get('Expires')):
            self.headers['Expires'] = 'Fri, 01 Jan 1990 00:00:00 GMT'
            self.headers['Content-Length'] = str(len(self.body))

        write = start_response(self.status, self.headerlist)
        write(self.body)

    @staticmethod
    def http_status_message(code):
        """Returns the default HTTP status message for the given code.

        :param code:
            The HTTP code for which we want a message.
        """
        message = status_reasons.get(code)
        if not message:
            raise KeyError('Invalid HTTP status code: %d' % code)

        return message


class RequestHandler(object):
    """Base HTTP request handler.

    Implements most of ``webapp.RequestHandler`` interface.
    """

    #: A :class:`Request` instance.
    request = None
    #: A :class:`Response` instance.
    response = None
    #: A :class:`WSGIApplication` instance.
    app = None

    def __init__(self, request=None, response=None):
        """Initializes this request handler with the given WSGI application,
        Request and Response.

        When instantiated by ``webapp.WSGIApplication``, request and response
        are not set on instantiation. Instead, initialize() is called right
        after the handler is created to set them.

        Also in webapp dispatching is done by the WSGI app, while webapp2
        does it here to allow more flexibility in extended classes: handlers
        can wrap :meth:`dispatch` to check for conditions before executing the
        requested method and/or post-process the response.

        .. note::
           Parameters are optional only to support webapp's constructor which
           doesn't take any arguments. Consider them as required.

        :param request:
            A :class:`Request` instance.
        :param response:
            A :class:`Response` instance.
        """
        self.initialize(request, response)

    def initialize(self, request, response):
        """Initializes this request handler with the given WSGI application,
        Request and Response.

        :param request:
            A :class:`Request` instance.
        :param response:
            A :class:`Response` instance.
        """
        self.request = request
        self.response = response
        self.app = WSGIApplication.active_instance

    def dispatch(self):
        """Dispatches the request.

        This will first check if there's a handler_method defined in the
        matched route, and if not it'll use the method correspondent to the
        request method (``get()``, ``post()`` etc).
        """
        request = self.request
        method_name = request.route.handler_method
        if not method_name:
            method_name = _normalize_handler_method(request.method)

        method = getattr(self, method_name, None)
        if method is None:
            # 405 Method Not Allowed.
            # The response MUST include an Allow header containing a
            # list of valid methods for the requested resource.
            # http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.6
            valid = ', '.join(_get_handler_methods(self))
            self.abort(405, headers=[('Allow', valid)])

        # The handler only receives *args if no named variables are set.
        args, kwargs = request.route_args, request.route_kwargs
        if kwargs:
            args = ()

        try:
            return method(*args, **kwargs)
        except Exception as e:
            return self.handle_exception(e, self.app.debug)

    def error(self, code):
        """Clears the response and sets the given HTTP status code.

        This doesn't stop code execution; for this, use :meth:`abort`.

        :param code:
            HTTP status error code (e.g., 501).
        """
        self.response.status = code
        self.response.clear()

    def abort(self, code, *args, **kwargs):
        """Raises an :class:`HTTPException`.

        This stops code execution, leaving the HTTP exception to be handled
        by an exception handler.

        :param code:
            HTTP status code (e.g., 404).
        :param args:
            Positional arguments to be passed to the exception class.
        :param kwargs:
            Keyword arguments to be passed to the exception class.
        """
        abort(code, *args, **kwargs)

    def redirect(self, uri, permanent=False, abort=False, code=None,
                 body=None):
        """Issues an HTTP redirect to the given relative URI.

        The arguments are described in :func:`redirect`.
        """
        return redirect(uri, permanent=permanent, abort=abort, code=code,
                        body=body, request=self.request,
                        response=self.response)

    def redirect_to(self, _name, _permanent=False, _abort=False, _code=None,
                    _body=None, *args, **kwargs):
        """Convenience method mixing :meth:`redirect` and :meth:`uri_for`.

        The arguments are described in :func:`redirect` and :func:`uri_for`.
        """
        uri = self.uri_for(_name, *args, **kwargs)
        return self.redirect(uri, permanent=_permanent, abort=_abort,
                             code=_code, body=_body)

    def uri_for(self, _name, *args, **kwargs):
        """Returns a URI for a named :class:`Route`.

        .. seealso:: :meth:`Router.build`.
        """
        return self.app.router.build(self.request, _name, args, kwargs)
    # Alias.
    url_for = uri_for

    def handle_exception(self, exception, debug):
        """Called if this handler throws an exception during execution.

        The default behavior is to re-raise the exception to be handled by
        :meth:`WSGIApplication.handle_exception`.

        :param exception:
            The exception that was thrown.
        :param debug_mode:
            True if the web application is running in debug mode.
        """
        raise


class RedirectHandler(RequestHandler):
    """Redirects to the given URI for all GET requests.

    This is intended to be used when defining URI routes. You must provide at
    least the keyword argument *url* in the route default values. Example::

        def get_redirect_url(handler, *args, **kwargs):
            return handler.uri_for('new-route-name')

        app = WSGIApplication([
            Route('/old-url', RedirectHandler, defaults={'_uri': '/new-url'}),
            Route('/other-old-url', RedirectHandler, defaults={
                  '_uri': get_redirect_url}),
        ])

    Based on idea from `Tornado`_.
    """

    def get(self, *args, **kwargs):
        """Performs a redirect.

        Two keyword arguments can be passed through the URI route:

        - **_uri**: A URI string or a callable that returns a URI. The callable
          is called passing ``(handler, *args, **kwargs)`` as arguments.
        - **_code**: The redirect status code. Default is 301 (permanent
          redirect).
        """
        uri = kwargs.pop('_uri', '/')
        permanent = kwargs.pop('_permanent', True)
        code = kwargs.pop('_code', None)

        func = getattr(uri, '__call__', None)
        if func:
            uri = func(self, *args, **kwargs)

        self.redirect(uri, permanent=permanent, code=code)


class cached_property(object):
    """A decorator that converts a function into a lazy property.

    The function wrapped is called the first time to retrieve the result
    and then that calculated result is used the next time you access
    the value::

        class Foo(object):

            @cached_property
            def foo(self):
                # calculate something important here
                return 42

    The class has to have a `__dict__` in order for this property to
    work.

    .. note:: Implementation detail: this property is implemented as non-data
       descriptor.  non-data descriptors are only invoked if there is
       no entry with the same name in the instance's __dict__.
       this allows us to completely get rid of the access function call
       overhead.  If one choses to invoke __get__ by hand the property
       will still work as expected because the lookup logic is replicated
       in __get__ for manual invocation.

    This class was ported from `Werkzeug`_ and `Flask`_.
    """

    _default_value = object()

    def __init__(self, func, name=None, doc=None):
        self.__name__ = name or func.__name__
        self.__module__ = func.__module__
        self.__doc__ = doc or func.__doc__
        self.func = func
        self.lock = threading.RLock()

    def __get__(self, obj, type=None):
        if obj is None:
            return self

        with self.lock:
            value = obj.__dict__.get(self.__name__, self._default_value)
            if value is self._default_value:
                value = self.func(obj)
                obj.__dict__[self.__name__] = value

            return value


class BaseRoute(object):
    """Interface for URI routes."""

    #: The regex template.
    template = None
    #: Route name, used to build URIs.
    name = None
    #: True if this route is only used for URI generation and never matches.
    build_only = False
    #: The handler or string in dotted notation to be lazily imported.
    handler = None
    #: The custom handler method, if handler is a class.
    handler_method = None
    #: The handler, imported and ready for dispatching.
    handler_adapter = None

    def __init__(self, template, handler=None, name=None, build_only=False):
        """Initializes this route.

        :param template:
            A regex to be matched.
        :param handler:
            A callable or string in dotted notation to be lazily imported,
            e.g., ``'my.module.MyHandler'`` or ``'my.module.my_function'``.
        :param name:
            The name of this route, used to build URIs based on it.
        :param build_only:
            If True, this route never matches and is used only to build URIs.
        """
        if build_only and name is None:
            raise ValueError(
                "Route %r is build_only but doesn't have a name." % self)

        self.template = template
        self.handler = handler
        self.name = name
        self.build_only = build_only

    def match(self, request):
        """Matches all routes against a request object.

        The first one that matches is returned.

        :param request:
            A :class:`Request` instance.
        :returns:
            A tuple ``(route, args, kwargs)`` if a route matched, or None.
        """
        raise NotImplementedError()

    def build(self, request, args, kwargs):
        """Returns a URI for this route.

        :param request:
            The current :class:`Request` object.
        :param args:
            Tuple of positional arguments to build the URI.
        :param kwargs:
            Dictionary of keyword arguments to build the URI.
        :returns:
            An absolute or relative URI.
        """
        raise NotImplementedError()

    def get_routes(self):
        """Generator to get all routes from a route.

        :yields:
            This route or all nested routes that it contains.
        """
        yield self

    def get_match_routes(self):
        """Generator to get all routes that can be matched from a route.

        Match routes must implement :meth:`match`.

        :yields:
            This route or all nested routes that can be matched.
        """
        if not self.build_only:
            yield self

    def get_build_routes(self):
        """Generator to get all routes that can be built from a route.

        Build routes must implement :meth:`build`.

        :yields:
            A tuple ``(name, route)`` for all nested routes that can be built.
        """
        if self.name is not None:
            yield self.name, self


class SimpleRoute(BaseRoute):
    """A route that is compatible with webapp's routing mechanism.

    URI building is not implemented as webapp has rudimentar support for it,
    and this is the most unknown webapp feature anyway.
    """

    @cached_property
    def regex(self):
        """Lazy regex compiler."""
        if not self.template.startswith('^'):
            self.template = '^' + self.template

        if not self.template.endswith('$'):
            self.template += '$'

        return re.compile(self.template)

    def match(self, request):
        """Matches this route against the current request.

        .. seealso:: :meth:`BaseRoute.match`.
        """
        match = self.regex.match(six.moves.urllib.parse.unquote(request.path))
        if match:
            return self, match.groups(), {}

    def __repr__(self):
        return '<SimpleRoute(%r, %r)>' % (self.template, self.handler)


class Route(BaseRoute):
    """A route definition that maps a URI path to a handler.

    The initial concept was based on `Another Do-It-Yourself Framework`_, by
    Ian Bicking.
    """

    #: Default parameters values.
    defaults = None
    #: Sequence of allowed HTTP methods. If not set, all methods are allowed.
    methods = None
    #: Sequence of allowed URI schemes. If not set, all schemes are allowed.
    schemes = None
    # Lazy properties extracted from the route template.
    regex = None
    reverse_template = None
    variables = None
    args_count = 0
    kwargs_count = 0

    def __init__(self, template, handler=None, name=None, defaults=None,
                 build_only=False, handler_method=None, methods=None,
                 schemes=None):
        """Initializes this route.

        :param template:
            A route template to match against the request path. A template
            can have variables enclosed by ``<>`` that define a name, a
            regular expression or both. Examples:

              =================  ==================================
              Format             Example
              =================  ==================================
              ``<name>``         ``'/blog/<year>/<month>'``
              ``<:regex>``       ``'/blog/<:\d{4}>/<:\d{2}>'``
              ``<name:regex>``   ``'/blog/<year:\d{4}>/<month:\d{2}>'``
              =================  ==================================

            The same template can mix parts with name, regular expression or
            both.

            If the name is set, the value of the matched regular expression
            is passed as keyword argument to the handler. Otherwise it is
            passed as positional argument.

            If only the name is set, it will match anything except a slash.
            So these routes are equivalent::

                Route('/<user_id>/settings', handler=SettingsHandler,
                      name='user-settings')
                Route('/<user_id:[^/]+>/settings', handler=SettingsHandler,
                      name='user-settings')

            .. note::
               The handler only receives ``*args`` if no named variables are
               set. Otherwise, the handler only receives ``**kwargs``. This
               allows you to set regular expressions that are not captured:
               just mix named and unnamed variables and the handler will
               only receive the named ones.

        :param handler:
            A callable or string in dotted notation to be lazily imported,
            e.g., ``'my.module.MyHandler'`` or ``'my.module.my_function'``.
            It is possible to define a method if the callable is a class,
            separating it by a colon: ``'my.module.MyHandler:my_method'``.
            This is a shortcut and has the same effect as defining the
            `handler_method` parameter.
        :param name:
            The name of this route, used to build URIs based on it.
        :param defaults:
            Default or extra keywords to be returned by this route. Values
            also present in the route variables are used to build the URI
            when they are missing.
        :param build_only:
            If True, this route never matches and is used only to build URIs.
        :param handler_method:
            The name of a custom handler method to be called, in case `handler`
            is a class. If not defined, the default behavior is to call the
            handler method correspondent to the HTTP request method in lower
            case (e.g., `get()`, `post()` etc).
        :param methods:
            A sequence of HTTP methods. If set, the route will only match if
            the request method is allowed.
        :param schemes:
            A sequence of URI schemes, e.g., ``['http']`` or ``['https']``.
            If set, the route will only match requests with these schemes.
        """
        super(Route, self).__init__(template, handler=handler, name=name,
                                    build_only=build_only)
        self.defaults = defaults or {}
        self.methods = methods
        self.schemes = schemes
        if isinstance(handler, six.string_types) and ':' in handler:
            if handler_method:
                raise ValueError(
                    "If handler_method is defined in a Route, handler "
                    "can't have a colon (got %r)." % handler)
            else:
                self.handler, self.handler_method = handler.rsplit(':', 1)
        else:
            self.handler_method = handler_method

    @cached_property
    def regex(self):
        """Lazy route template parser."""
        regex, self.reverse_template, self.args_count, self.kwargs_count, \
            self.variables = _parse_route_template(self.template,
                                                   default_sufix='[^/]+')
        return regex

    def match(self, request):
        """Matches this route against the current request.

        :raises:
            ``exc.HTTPMethodNotAllowed`` if the route defines :attr:`methods`
            and the request method isn't allowed.

        .. seealso:: :meth:`BaseRoute.match`.
        """
        match = self.regex.match(six.moves.urllib.parse.unquote(request.path))
        if not match or self.schemes and request.scheme not in self.schemes:
            return None

        if self.methods and request.method not in self.methods:
            # This will be caught by the router, so routes with different
            # methods can be tried.
            raise exc.HTTPMethodNotAllowed()

        args, kwargs = _get_route_variables(match, self.defaults.copy())
        return self, args, kwargs

    def build(self, request, args, kwargs):
        """Returns a URI for this route.

        .. seealso:: :meth:`Router.build`.
        """
        scheme = kwargs.pop('_scheme', None)
        netloc = kwargs.pop('_netloc', None)
        anchor = kwargs.pop('_fragment', None)
        full = kwargs.pop('_full', False) and not scheme and not netloc

        if full or scheme or netloc:
            netloc = netloc or request.host
            scheme = scheme or request.scheme

        path, query = self._build(args, kwargs)
        return _urlunsplit(scheme, netloc, path, query, anchor)

    def _build(self, args, kwargs):
        """Returns the URI path for this route.

        :returns:
            A tuple ``(path, kwargs)`` with the built URI path and extra
            keywords to be used as URI query arguments.
        """
        # Access self.regex just to set the lazy properties.
        regex = self.regex
        variables = self.variables
        if self.args_count:
            for index, value in enumerate(args):
                key = '__%d__' % index
                if key in variables:
                    kwargs[key] = value

        values = {}
        for name, regex in variables.items():
            value = kwargs.pop(name, self.defaults.get(name))
            if value is None:
                raise KeyError('Missing argument "%s" to build URI.' % \
                    name.strip('_'))

            if not isinstance(value, six.string_types):
                value = str(value)

            if not regex.match(value):
                raise ValueError('URI buiding error: Value "%s" is not '
                    'supported for argument "%s".' % (value, name.strip('_')))

            values[name] = value

        return (self.reverse_template % values, kwargs)

    def __repr__(self):
        return '<Route(%r, %r, name=%r, defaults=%r, build_only=%r)>' % \
            (self.template, self.handler, self.name, self.defaults,
            self.build_only)


class BaseHandlerAdapter(object):
    """A basic adapter to dispatch a handler.

    This is used when the handler is a simple function: it just calls the
    handler and returns the resulted response.
    """

    #: The handler to be dispatched.
    handler = None

    def __init__(self, handler):
        self.handler = handler

    def __call__(self, request, response):
        # The handler only receives *args if no named variables are set.
        args, kwargs = request.route_args, request.route_kwargs
        if kwargs:
            args = ()

        return self.handler(request, *args, **kwargs)


class WebappHandlerAdapter(BaseHandlerAdapter):
    """An adapter to dispatch a ``webapp.RequestHandler``.

    Like in webapp, the handler is constructed, then ``initialize()`` is
    called, then the method corresponding to the HTTP request method is called.
    """

    def __call__(self, request, response):
        handler = self.handler()
        handler.initialize(request, response)
        method_name = _normalize_handler_method(request.method)
        method = getattr(handler, method_name, None)
        if not method:
            abort(501)

        # The handler only receives *args if no named variables are set.
        args, kwargs = request.route_args, request.route_kwargs
        if kwargs:
            args = ()

        try:
            method(*args, **kwargs)
        except Exception as e:
            handler.handle_exception(e, request.app.debug)


class Webapp2HandlerAdapter(BaseHandlerAdapter):
    """An adapter to dispatch a ``webapp2.RequestHandler``.

    The handler is constructed then ``dispatch()`` is called.
    """

    def __call__(self, request, response):
        handler = self.handler(request, response)
        return handler.dispatch()


class Router(object):
    """A URI router used to match, dispatch and build URIs."""

    #: Class used when the route is set as a tuple.
    route_class = SimpleRoute
    #: All routes that can be matched.
    match_routes = None
    #: All routes that can be built.
    build_routes = None
    #: Handler classes imported lazily.
    handlers = None

    def __init__(self, routes=None):
        """Initializes the router.

        :param routes:
            A sequence of :class:`Route` instances or, for simple routes,
            tuples ``(regex, handler)``.
        """
        self.match_routes = []
        self.build_routes = {}
        self.handlers = {}
        if routes:
            for route in routes:
                self.add(route)

    def add(self, route):
        """Adds a route to this router.

        :param route:
            A :class:`Route` instance or, for simple routes, a tuple
            ``(regex, handler)``.
        """
        if isinstance(route, tuple):
            # Exceptional case: simple routes defined as a tuple.
            route = self.route_class(*route)

        for r in route.get_match_routes():
            self.match_routes.append(r)

        for name, r in route.get_build_routes():
            self.build_routes[name] = r

    def set_matcher(self, func):
        """Sets the function called to match URIs.

        :param func:
            A function that receives ``(router, request)`` and returns
            a tuple ``(route, args, kwargs)`` if any route matches, or
            raise ``exc.HTTPNotFound`` if no route matched or
            ``exc.HTTPMethodNotAllowed`` if a route matched but the HTTP
            method was not allowed.
        """
        # Functions are descriptors, so bind it to this instance with __get__.
        self.match = func.__get__(self, self.__class__)

    def set_builder(self, func):
        """Sets the function called to build URIs.

        :param func:
            A function that receives ``(router, request, name, args, kwargs)``
            and returns a URI.
        """
        self.build = func.__get__(self, self.__class__)

    def set_dispatcher(self, func):
        """Sets the function called to dispatch the handler.

        :param func:
            A function that receives ``(router, request, response)``
            and returns the value returned by the dispatched handler.
        """
        self.dispatch = func.__get__(self, self.__class__)

    def set_adapter(self, func):
        """Sets the function that adapts loaded handlers for dispatching.

        :param func:
            A function that receives ``(router, handler)`` and returns a
            handler callable.
        """
        self.adapt = func.__get__(self, self.__class__)

    def default_matcher(self, request):
        """Matches all routes against a request object.

        The first one that matches is returned.

        :param request:
            A :class:`Request` instance.
        :returns:
            A tuple ``(route, args, kwargs)`` if a route matched, or None.
        :raises:
            ``exc.HTTPNotFound`` if no route matched or
            ``exc.HTTPMethodNotAllowed`` if a route matched but the HTTP
            method was not allowed.
        """
        method_not_allowed = False
        for route in self.match_routes:
            try:
                match = route.match(request)
                if match:
                    return match
            except exc.HTTPMethodNotAllowed:
                method_not_allowed = True

        if method_not_allowed:
            raise exc.HTTPMethodNotAllowed()

        raise exc.HTTPNotFound()

    def default_builder(self, request, name, args, kwargs):
        """Returns a URI for a named :class:`Route`.

        :param request:
            The current :class:`Request` object.
        :param name:
            The route name.
        :param args:
            Tuple of positional arguments to build the URI. All positional
            variables defined in the route must be passed and must conform
            to the format set in the route. Extra arguments are ignored.
        :param kwargs:
            Dictionary of keyword arguments to build the URI. All variables
            not set in the route default values must be passed and must
            conform to the format set in the route. Extra keywords are
            appended as a query string.

            A few keywords have special meaning:

            - **_full**: If True, builds an absolute URI.
            - **_scheme**: URI scheme, e.g., `http` or `https`. If defined,
              an absolute URI is always returned.
            - **_netloc**: Network location, e.g., `www.google.com`. If
              defined, an absolute URI is always returned.
            - **_fragment**: If set, appends a fragment (or "anchor") to the
              generated URI.
        :returns:
            An absolute or relative URI.
        """
        route = self.build_routes.get(name)
        if route is None:
            raise KeyError('Route named %r is not defined.' % name)

        return route.build(request, args, kwargs)

    def default_dispatcher(self, request, response):
        """Dispatches a handler.

        :param request:
            A :class:`Request` instance.
        :param response:
            A :class:`Response` instance.
        :raises:
            ``exc.HTTPNotFound`` if no route matched or
            ``exc.HTTPMethodNotAllowed`` if a route matched but the HTTP
            method was not allowed.
        :returns:
            The returned value from the handler.
        """
        route, args, kwargs = rv = self.match(request)
        request.route, request.route_args, request.route_kwargs = rv

        if route.handler_adapter is None:
            handler = route.handler
            if isinstance(handler, six.string_types):
                if handler not in self.handlers:
                    self.handlers[handler] = handler = import_string(handler)
                else:
                    handler = self.handlers[handler]

            route.handler_adapter = self.adapt(handler)

        return route.handler_adapter(request, response)

    def default_adapter(self, handler):
        """Adapts a handler for dispatching.

        Because handlers use or implement different dispatching mechanisms,
        they can be wrapped to use a unified API for dispatching.
        This way webapp2 can support, for example, a :class:`RequestHandler`
        class and function views or, for compatibility purposes, a
        ``webapp.RequestHandler`` class. The adapters follow the same router
        dispatching API but dispatch each handler type differently.

        :param handler:
            A handler callable.
        :returns:
            A wrapped handler callable.
        """
        if inspect.isclass(handler):
            if _webapp and issubclass(handler, _webapp.RequestHandler):
                # Compatible with webapp.RequestHandler.
                adapter = WebappHandlerAdapter
            else:
                # Default, compatible with webapp2.RequestHandler.
                adapter = Webapp2HandlerAdapter
        else:
            # A "view" function.
            adapter = BaseHandlerAdapter

        return adapter(handler)

    def __repr__(self):
        routes = self.match_routes + [v for k, v in \
            self.build_routes.items() if v not in self.match_routes]

        return '<Router(%r)>' % routes

    # Default matcher, builder, dispatcher and adapter.
    match = default_matcher
    build = default_builder
    dispatch = default_dispatcher
    adapt = default_adapter


class Config(dict):
    """A simple configuration dictionary for the :class:`WSGIApplication`."""

    #: Loaded configurations.
    loaded = None

    def __init__(self, defaults=None):
        dict.__init__(self, defaults or ())
        self.loaded = []

    def load_config(self, key, default_values=None, user_values=None,
                    required_keys=None):
        """Returns a configuration for a given key.

        This can be used by objects that define a default configuration. It
        will update the app configuration with the default values the first
        time it is requested, and mark the key as loaded.

        :param key:
            A configuration key.
        :param default_values:
            Default values defined by a module or class.
        :param user_values:
            User values, used when an object can be initialized with
            configuration. This overrides the app configuration.
        :param required_keys:
            Keys that can not be None.
        :raises:
            Exception, when a required key is not set or is None.
        """
        if key in self.loaded:
            config = self[key]
        else:
            config = dict(default_values or ())
            if key in self:
                config.update(self[key])

            self[key] = config
            self.loaded.append(key)
            if required_keys and not user_values:
                self._validate_required(key, config, required_keys)

        if user_values:
            config = config.copy()
            config.update(user_values)
            if required_keys:
                self._validate_required(key, config, required_keys)

        return config

    def _validate_required(self, key, config, required_keys):
        missing = [k for k in required_keys if config.get(k) is None]
        if missing:
            raise Exception(
                'Missing configuration keys for %r: %r.' % (key, missing))


class RequestContext(object):
    """Context for a single request.

    The context is responsible for setting and cleaning global variables for
    a request.
    """

    #: A :class:`WSGIApplication` instance.
    app = None
    #: WSGI environment dictionary.
    environ = None

    def __init__(self, app, environ):
        """Initializes the request context.

        :param app:
            An :class:`WSGIApplication` instance.
        :param environ:
            A WSGI environment dictionary.
        """
        self.app = app
        self.environ = environ

    def __enter__(self):
        """Enters the request context.

        :returns:
            A tuple ``(request, response)``.
        """
        # Build request and response.
        request = self.app.request_class(self.environ)
        response = self.app.response_class()
        # Make active app and response available through the request object.
        request.app = self.app
        request.response = response
        # Register global variables.
        self.app.set_globals(app=self.app, request=request)
        return request, response

    def __exit__(self, exc_type, exc_value, traceback):
        """Exits the request context.

        This release the context locals except if an exception is caught
        in debug mode. In this case they are kept to be inspected.
        """
        if exc_type is None or not self.app.debug:
            # Unregister global variables.
            self.app.clear_globals()


class WSGIApplication(object):
    """A WSGI-compliant application."""

    #: Allowed request methods.
    allowed_methods = frozenset(('GET', 'POST', 'HEAD', 'OPTIONS', 'PUT',
                                 'DELETE', 'TRACE'))
    #: Class used for the request object.
    request_class = Request
    #: Class used for the response object.
    response_class = Response
    #: Class used for the router object.
    router_class = Router
    #: Class used for the request context object.
    request_context_class = RequestContext
    #: Class used for the configuration object.
    config_class = Config
    #: A general purpose flag to indicate development mode: if True, uncaught
    #: exceptions are raised instead of using ``HTTPInternalServerError``.
    debug = False
    #: A :class:`Router` instance with all URIs registered for the application.
    router = None
    #: A :class:`Config` instance with the application configuration.
    config = None
    #: A dictionary to register objects used during the app lifetime.
    registry = None
    #: A dictionary mapping HTTP error codes to callables to handle those
    #: HTTP exceptions. See :meth:`handle_exception`.
    error_handlers = None
    #: Active :class:`WSGIApplication` instance. See :meth:`set_globals`.
    app = None
    #: Active :class:`Request` instance. See :meth:`set_globals`.
    request = None
    #: Same as :attr:`app`, for webapp compatibility. See :meth:`set_globals`.
    active_instance = None

    def __init__(self, routes=None, debug=False, config=None):
        """Initializes the WSGI application.

        :param routes:
            A sequence of :class:`Route` instances or, for simple routes,
            tuples ``(regex, handler)``.
        :param debug:
            True to enable debug mode, False otherwise.
        :param config:
            A configuration dictionary for the application.
        """
        self.debug = debug
        self.registry = {}
        self.error_handlers = {}
        self.set_globals(app=self)
        self.config = self.config_class(config)
        self.router = self.router_class(routes)

    def set_globals(self, app=None, request=None):
        """Registers the global variables for app and request.

        If :mod:`webapp2_extras.local` is available the app and request
        class attributes are assigned to a proxy object that returns them
        using thread-local, making the application thread-safe. This can also
        be used in environments that don't support threading.

        If :mod:`webapp2_extras.local` is not available app and request will
        be assigned directly as class attributes. This should only be used in
        non-threaded environments (e.g., App Engine Python 2.5).

        :param app:
            A :class:`WSGIApplication` instance.
        :param request:
            A :class:`Request` instance.
        """
        if _local is not None: # pragma: no cover
            _local.app = app
            _local.request = request
        else: # pragma: no cover
            WSGIApplication.app = WSGIApplication.active_instance = app
            WSGIApplication.request = request

    def clear_globals(self):
        """Clears global variables. See :meth:`set_globals`."""
        if _local is not None: # pragma: no cover
            _local.__release_local__()
        else: # pragma: no cover
            WSGIApplication.app = WSGIApplication.active_instance = None
            WSGIApplication.request = None

    def __call__(self, environ, start_response):
        """Called by WSGI when a request comes in.

        :param environ:
            A WSGI environment.
        :param start_response:
            A callable accepting a status code, a list of headers and an
            optional exception context to start the response.
        :returns:
            An iterable with the response to return to the client.
        """
        with self.request_context_class(self, environ) as (request, response):
            try:
                if request.method not in self.allowed_methods:
                    # 501 Not Implemented.
                    raise exc.HTTPNotImplemented()

                rv = self.router.dispatch(request, response)
                if rv is not None:
                    response = rv
            except Exception as e:
                try:
                    # Try to handle it with a custom error handler.
                    rv = self.handle_exception(request, response, e)
                    if rv is not None:
                        response = rv
                except HTTPException as e:
                    # Use the HTTP exception as response.
                    response = e
                except Exception as e:
                    # Error wasn't handled so we have nothing else to do.
                    response = self._internal_error(e)

            try:
                return response(environ, start_response)
            except Exception as e:
                return self._internal_error(e)(environ, start_response)

    def _internal_error(self, exception):
        """Last resource error for :meth:`__call__`."""
        logging.exception(exception)
        if self.debug:
            lines = ''.join(traceback.format_exception(*sys.exc_info()))
            html = _debug_template % (cgi.escape(lines, quote=True))
            return Response(body=html, status=500)

        return exc.HTTPInternalServerError()

    def handle_exception(self, request, response, e):
        """Handles a uncaught exception occurred in :meth:`__call__`.

        Uncaught exceptions can be handled by error handlers registered in
        :attr:`error_handlers`. This is a dictionary that maps HTTP status
        codes to callables that will handle the corresponding error code.
        If the exception is not an ``HTTPException``, the status code 500
        is used.

        The error handlers receive (request, response, exception) and can be
        a callable or a string in dotted notation to be lazily imported.

        If no error handler is found, the exception is re-raised.

        Based on idea from `Flask`_.

        :param request:
            A :class:`Request` instance.
        :param response:
            A :class:`Response` instance.
        :param e:
            The uncaught exception.
        :returns:
            The returned value from the error handler.
        """
        if isinstance(e, HTTPException):
            code = e.code
        else:
            code = 500

        handler = self.error_handlers.get(code)
        if handler:
            if isinstance(handler, six.string_types):
                self.error_handlers[code] = handler = import_string(handler)

            return handler(request, response, e)
        else:
            # Re-raise it to be caught by the WSGI app.
            raise

    def run(self, bare=False):
        """Runs this WSGI-compliant application in a CGI environment.

        This uses functions provided by ``google.appengine.ext.webapp.util``,
        if available: ``run_bare_wsgi_app`` and ``run_wsgi_app``.

        Otherwise, it uses ``wsgiref.handlers.CGIHandler().run()``.

        :param bare:
            If True, doesn't add registered WSGI middleware: use
            ``run_bare_wsgi_app`` instead of ``run_wsgi_app``.
        """
        if _webapp_util:
            if bare:
                _webapp_util.run_bare_wsgi_app(self)
            else:
                _webapp_util.run_wsgi_app(self)
        else: # pragma: no cover
            handlers.CGIHandler().run(self)

    def get_response(self, *args, **kwargs):
        """Creates a request and returns a response for this app.

        This is a convenience for unit testing purposes. It receives
        parameters to build a request and calls the application, returning
        the resulting response::

            class HelloHandler(webapp2.RequestHandler):
                def get(self):
                    self.response.write('Hello, world!')

            app = webapp2.WSGIapplication([('/', HelloHandler)])

            # Test the app, passing parameters to build a request.
            response = app.get_response('/')
            assert response.status_int == 200
            assert response.body == 'Hello, world!'

        :param args:
            Positional arguments to be passed to ``Request.blank()``.
        :param kwargs:
            Keyword arguments to be passed to ``Request.blank()``.
        :returns:
            A :class:`Response` object.
        """
        return self.request_class.blank(*args, **kwargs).get_response(self)


_import_string_error = """\
import_string() failed for %r. Possible reasons are:

- missing __init__.py in a package;
- package or module path not included in sys.path;
- duplicated package or module name taking precedence in sys.path;
- missing module, class, function or variable;

Original exception:

%s: %s

Debugged import:

%s"""


class ImportStringError(Exception):
    """Provides information about a failed :func:`import_string` attempt."""

    #: String in dotted notation that failed to be imported.
    import_name = None
    #: Wrapped exception.
    exception = None

    def __init__(self, import_name, exception):
        self.import_name = import_name
        self.exception = exception
        msg = _import_string_error
        name = ''
        tracked = []
        for part in import_name.split('.'):
            name += (name and '.') + part
            imported = import_string(name, silent=True)
            if imported:
                tracked.append((name, imported.__file__))
            else:
                track = ['- %r found in %r.' % rv for rv in tracked]
                track.append('- %r not found.' % name)
                msg = msg % (import_name, exception.__class__.__name__,
                             str(exception), '\n'.join(track))
                break

        Exception.__init__(self, msg)


_get_app_error = 'WSGIApplication global variable is not set.'
_get_request_error = 'Request global variable is not set.'


def get_app():
    """Returns the active app instance.

    :returns:
        A :class:`WSGIApplication` instance.
    """
    if _local:
        assert getattr(_local, 'app', None) is not None, _get_app_error
    else:
        assert WSGIApplication.app is not None, _get_app_error

    return WSGIApplication.app


def get_request():
    """Returns the active request instance.

    :returns:
        A :class:`Request` instance.
    """
    if _local:
        assert getattr(_local, 'request', None) is not None, _get_request_error
    else:
        assert WSGIApplication.request is not None, _get_request_error

    return WSGIApplication.request


def uri_for(_name, _request=None, *args, **kwargs):
    """A standalone uri_for version that can be passed to templates.

    .. seealso:: :meth:`Router.build`.
    """
    request = _request or get_request()
    return request.app.router.build(request, _name, args, kwargs)


def redirect(uri, permanent=False, abort=False, code=None, body=None,
             request=None, response=None):
    """Issues an HTTP redirect to the given relative URI.

    This won't stop code execution unless **abort** is True. A common
    practice is to return when calling this method::

        return redirect('/some-path')

    :param uri:
        A relative or absolute URI (e.g., ``'../flowers.html'``).
    :param permanent:
        If True, uses a 301 redirect instead of a 302 redirect.
    :param abort:
        If True, raises an exception to perform the redirect.
    :param code:
        The redirect status code. Supported codes are 301, 302, 303, 305,
        and 307.  300 is not supported because it's not a real redirect
        and 304 because it's the answer for a request with defined
        ``If-Modified-Since`` headers.
    :param body:
        Response body, if any.
    :param request:
        Optional request object. If not set, uses :func:`get_request`.
    :param response:
        Optional response object. If not set, a new response is created.
    :returns:
        A :class:`Response` instance.
    """
    if uri.startswith(('.', '/')):
        request = request or get_request()
        uri = str(six.moves.urllib.parse.urljoin(request.url, uri))

    if code is None:
        if permanent:
            code = 301
        else:
            code = 302

    assert code in (301, 302, 303, 305, 307), \
        'Invalid redirect status code.'

    if abort:
        _abort(code, headers=[('Location', uri)])

    if response is None:
        request = request or get_request()
        response = request.app.response_class()
    else:
        response.clear()

    response.headers['Location'] = uri
    response.status = code
    if body is not None:
        response.write(body)

    return response


def redirect_to(_name, _permanent=False, _abort=False, _code=None,
                _body=None, _request=None, _response=None, *args, **kwargs):
    """Convenience function mixing :func:`redirect` and :func:`uri_for`.

    Issues an HTTP redirect to a named URI built using :func:`uri_for`.

    :param _name:
        The route name to redirect to.
    :param args:
        Positional arguments to build the URI.
    :param kwargs:
        Keyword arguments to build the URI.
    :returns:
        A :class:`Response` instance.

    The other arguments are described in :func:`redirect`.
    """
    uri = uri_for(_name, _request=_request, *args, **kwargs)
    return redirect(uri, permanent=_permanent, abort=_abort, code=_code,
                    body=_body, request=_request, response=_response)


def abort(code, *args, **kwargs):
    """Raises an ``HTTPException``.

    :param code:
        An integer that represents a valid HTTP status code.
    :param args:
        Positional arguments to instantiate the exception.
    :param kwargs:
        Keyword arguments to instantiate the exception.
    """
    cls = exc.status_map.get(code)
    if not cls:
        raise KeyError('No exception is defined for code %r.' % code)

    raise cls(*args, **kwargs)


def import_string(import_name, silent=False):
    """Imports an object based on a string in dotted notation.

    Simplified version of the function with same name from `Werkzeug`_.

    :param import_name:
        String in dotted notation of the object to be imported.
    :param silent:
        If True, import or attribute errors are ignored and None is returned
        instead of raising an exception.
    :returns:
        The imported object.
    """
    import_name = _to_utf8(import_name)
    try:
        if '.' in import_name:
            module, obj = import_name.rsplit('.', 1)
            return getattr(__import__(module, None, None, [obj]), obj)
        else:
            return __import__(import_name)
    except (ImportError, AttributeError) as e:
        if not silent:
            six.reraise(ImportStringError(import_name, e), None, sys.exc_info()[2])


def _urlunsplit(scheme=None, netloc=None, path=None, query=None,
                fragment=None):
    """Like ``urlparse.urlunsplit``, but will escape values and urlencode and
    sort query arguments.

    :param scheme:
        URI scheme, e.g., `http` or `https`.
    :param netloc:
        Network location, e.g., `localhost:8080` or `www.google.com`.
    :param path:
        URI path.
    :param query:
        URI query as an escaped string, or a dictionary or list of key-values
        tuples to build a query.
    :param fragment:
        Fragment identifier, also known as "anchor".
    :returns:
        An assembled absolute or relative URI.
    """
    if not scheme or not netloc:
        scheme = None
        netloc = None

    if path:
        path = six.moves.urllib.parse.quote(_to_utf8(path))

    if query and not isinstance(query, six.string_types):
        if isinstance(query, dict):
            query = query.items()

        # Sort args: commonly needed to build signatures for services.
        query = six.moves.urllib.parse.urlencode(sorted(query))

    if fragment:
        fragment = six.moves.urllib.parse.quote(_to_utf8(fragment))

    return six.moves.urllib.parse.urlunsplit((scheme, netloc, path, query, fragment))


def _get_handler_methods(handler):
    """Returns a list of HTTP methods supported by a handler.

    :param handler:
        A :class:`RequestHandler` instance.
    :returns:
        A list of HTTP methods supported by the handler.
    """
    methods = []
    for method in get_app().allowed_methods:
        if getattr(handler, _normalize_handler_method(method), None):
            methods.append(method)

    return methods


def _normalize_handler_method(method):
    """Transforms an HTTP method into a valid Python identifier."""
    return method.lower().replace('-', '_')


def _to_utf8(value):
    """Encodes a unicode value to UTF-8 if not yet encoded."""
    if isinstance(value, str):
        return value

    return value.encode('utf-8')


def _parse_route_template(template, default_sufix=''):
    """Lazy route template parser."""
    variables = {}
    reverse_template = pattern = ''
    args_count = last = 0
    for match in _route_re.finditer(template):
        part = template[last:match.start()]
        name = match.group(1)
        expr = match.group(2) or default_sufix
        last = match.end()

        if not name:
            name = '__%d__' % args_count
            args_count += 1

        pattern += '%s(?P<%s>%s)' % (re.escape(part), name, expr)
        reverse_template += '%s%%(%s)s' % (part, name)
        variables[name] = re.compile('^%s$' % expr)

    part = template[last:]
    kwargs_count = len(variables) - args_count
    reverse_template += part
    regex = re.compile('^%s%s$' % (pattern, re.escape(part)))
    return regex, reverse_template, args_count, kwargs_count, variables


def _get_route_variables(match, default_kwargs=None):
    """Returns (args, kwargs) for a route match."""
    kwargs = default_kwargs or {}
    kwargs.update(match.groupdict())
    if kwargs:
        args = tuple(
            value[1] for value in sorted(
                (int(key[2:-2]), kwargs[key])
                for key in kwargs.keys()
                if key.startswith('__') and key.endswith('__')))
    else:
        args = ()

    return args, kwargs


def _set_thread_safe_app():
    """Assigns WSGIApplication globals to a proxy pointing to thread-local."""
    if _local is not None: # pragma: no cover
        WSGIApplication.app = WSGIApplication.active_instance = _local('app')
        WSGIApplication.request = _local('request')


Request.ResponseClass = Response
Response.RequestClass = Request
# Alias.
_abort = abort
# Thread-safety support.
_set_thread_safe_app()

# Defer importing google.appengine.ext.webapp.util until every public symbol
# has been defined since google.appengine.ext.webapp in App Engine Python 2.7
# runtime imports this module to provide its public interface.
try:
    from google.appengine.ext.webapp import util as _webapp_util
except ImportError: # pragma: no cover
    pass
# webapp is not python 3 compatible. Importing it will raise syntax error.
except SyntaxError: # pragma: no cover
    pass
