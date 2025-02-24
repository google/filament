.. _guide.request:

Request data
============
The request handler instance can access the request data using its ``request``
property. This is initialized to a populated `WebOb`_ ``Request`` object by
the application.

The request object provides a ``get()`` method that returns values for
arguments parsed from the query and from POST data. The method takes the
argument name as its first parameter. For example::

    class MyHandler(webapp2.RequestHandler):
        def post(self):
            name = self.request.get('name')

By default, ``get()`` returns the empty string (``''``) if the requested
argument is not in the request. If the parameter ``default_value`` is
specified, ``get()`` returns the value of that parameter instead of the empty
string if the argument is not present.

If the argument appears more than once in a request, by default ``get()``
returns the first occurrence. To get all occurrences of an argument that might
appear more than once as a list (possibly empty), give ``get()`` the argument
``allow_multiple=True``::

    # <input name="name" type="text" />
    name = self.request.get("name")

    # <input name="subscribe" type="checkbox" value="yes" />
    subscribe_to_newsletter = self.request.get("subscribe", default_value="no")

    # <select name="favorite_foods" multiple="true">...</select>
    favorite_foods = self.request.get("favorite_foods", allow_multiple=True)

    # for food in favorite_foods:
    # ...

For requests with body content that is not a set of CGI parameters, such as
the body of an HTTP PUT request, the request object provides the attributes
``body`` and ``body_file``: ``body`` is the body content as a byte string and
``body_file`` provides a file-like interface to the same data::

    uploaded_file = self.request.body


GET data
--------
Query string variables are available in ``request.GET``.

``.GET`` is a `MultiDict`_: it is like a dictionary but the same key can have
multiple values. When you call ``.get(key)`` for a key with multiple values,
the last value is returned. To get all values for a key, use ``.getall(key)``.
Examples::

    request = Request.blank('/test?check=a&check=b&name=Bob')

    # The whole MultiDict:
    # GET([('check', 'a'), ('check', 'b'), ('name', 'Bob')])
    get_values = request.GET

    # The last value for a key: 'b'
    check_value = request.GET['check']

    # All values for a key: ['a', 'b']
    check_values = request.GET.getall('check')

    # An iterable with alll items in the MultiDict:
    # [('check', 'a'), ('check', 'b'), ('name', 'Bob')]
    request.GET.items()

The name ``GET`` is a bit misleading, but has historical reasons:
``request.GET`` is not only available when the HTTP method is GET. It is
available for any request with query strings in the URI, for any HTTP method:
GET, POST, PUT etc.


POST data
---------
Variables url encoded in the body of a request (generally a POST form submitted
using the ``application/x-www-form-urlencoded`` media type) are available in
``request.POST``.

It is also a `MultiDict`_ and can be accessed in the same way as ``.GET``.
Examples::

    request = Request.blank('/')
    request.method = 'POST'
    request.body = 'check=a&check=b&name=Bob'

    # The whole MultiDict:
    # POST([('check', 'a'), ('check', 'b'), ('name', 'Bob')])
    post_values = request.POST

    # The last value for a key: 'b'
    check_value = request.POST['check']

    # All values for a key: ['a', 'b']
    check_values = request.POST.getall('check')

    # An iterable with alll items in the MultiDict:
    # [('check', 'a'), ('check', 'b'), ('name', 'Bob')]
    request.POST.items()

Like ``GET``, the name ``POST`` is a somewjat misleading, but has historical
reasons: they are also available when the HTTP method is PUT, and not only
POST.


GET + POST data
---------------
``request.params`` combines the variables from ``GET`` and ``POST``. It can be
used when you don't care where the variable comes from.


Files
-----
Uploaded files are available as ``cgi.FieldStorage`` (see the :py:mod:`cgi`
module) instances directly in ``request.POST``.


.. _guide.request.cookies:

Cookies
-------
Cookies can be accessed in ``request.cookies``. It is a simple dictionary::

    request = Request.blank('/')
    request.headers['Cookie'] = 'test=value'

    # A value: 'value'
    cookie_value = request.cookies.get('test')

.. seealso::
   :ref:`How to set cookies using the response object <guide.response.setting-cookies>`


Common Request attributes
-------------------------
body
  A file-like object that gives the body of the request.
content_type
  Content-type of the request body.
method
  The HTTP method, e.g., 'GET' or 'POST'.
url
  Full URI, e.g., ``'http://localhost/blog/article?id=1'``.
scheme
  URI scheme, e.g., 'http' or 'https'.
host
  URI host, e.g., ``'localhost:80'``.
host_url
  URI host including scheme, e.g., ``'http://localhost'``.
path_url
  URI host including scheme and path, e.g., ``'http://localhost/blog/article'``.
path
  URI path, e.g., ``'/blog/article'``.
path_qs
  URI path including the query string, e.g., ``'/blog/article?id=1'``.
query_string
  Query string, e.g., ``id=1``.
headers
  A dictionary like object with request headers. Keys are case-insensitive.
GET
  A dictionary-like object with variables from the query string, as unicode.
POST
  A dictionary-like object with variables from a POST form, as unicode.
params
  A dictionary-like object combining the variables GET and POST.
cookies
  A dictionary-like object with cookie values.


Extra attributes
----------------
The parameters from the matched :class:`webapp2.Route` are set as attributes
of the request object. They are ``request.route_args``, for positional
arguments, and ``request.route_kwargs``, for keyword arguments. The matched
route object is available as ``request.route``.

A reference to the active WSGI application is also set as an attribute of the
request. You can access it in ``request.app``.


Getting the current request
---------------------------
The active ``Request`` instance can be accessed during a request using the
function :func:`webapp2.get_request`.


.. _guide.request.registry:

Registry
--------
A simple dictionary is available in the request object to register instances
that are shared during a request: it is the :attr:`webapp2.Request.registry`
attribute.

A registry dictionary is also available in the
:ref:`WSGI application object <guide.app.registry>`, to store objects shared
across requests.


Learn more about WebOb
----------------------
WebOb is an open source third-party library. See the `WebOb`_ documentation
for a detailed API reference and examples.


.. _WebOb: http://docs.webob.org/
.. _MultiDict: http://pythonpaste.org/webob/class-webob.multidict.MultiDict.html
