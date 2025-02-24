.. _guide.response:

Building a Response
===================
The request handler instance builds the response using its response property.
This is initialized to an empty `WebOb`_ ``Response`` object by the
application.

The response object's acts as a file-like object that can be used for
writing the body of the response::

    class MyHandler(webapp2.RequestHandler):
        def get(self):
            self.response.write("<html><body><p>Hi there!</p></body></html>")

The response buffers all output in memory, then sends the final output when
the handler exits. webapp2 does not support streaming data to the client.

The ``clear()`` method erases the contents of the output buffer, leaving it
empty.

If the data written to the output stream is a Unicode value, or if the
response includes a ``Content-Type`` header that ends with ``; charset=utf-8``,
webapp2 encodes the output as UTF-8. By default, the ``Content-Type`` header
is ``text/html; charset=utf-8``, including the encoding behavior. If the
``Content-Type`` is changed to have a different charset, webapp2 assumes the
output is a byte string to be sent verbatim.

.. warning:
   The ``status`` attribute from a response is the status code plus message,
   e.g., '200 OK'. This is different from webapp, which has the status code
   (an integer) stored in ``status``. In webapp2, the status code is stored
   in the ``status_int`` attribute, as in WebOb.


.. _guide.response.setting-cookies:

Setting cookies
---------------
Cookies are set in the response object. The methods to handle cookies are:

set_cookie(key, value='', max_age=None, path='/', domain=None, secure=None, httponly=False, comment=None, expires=None, overwrite=False)
  Sets a cookie.

delete_cookie(key, path='/', domain=None)
  Deletes a cookie previously set in the client.

unset_cookie(key)
  Unsets a cookie previously set in the response object. Note that this
  doesn't delete the cookie from clients, only from the response.

For example::

    # Saves a cookie in the client.
    response.set_cookie('some_key', 'value', max_age=360, path='/',
                        domain='example.org', secure=True)

    # Deletes a cookie previously set in the client.
    response.delete_cookie('bad_cookie')

    # Cancels a cookie previously set in the response.
    response.unset_cookie('some_key')

Only the ``key`` parameter is required. The parameters are:

key
  Cookie name.
value
  Cookie value.
expires
  An expiration date. Must be a :py:mod:`datetime`.datetime object. Use this
  instead of max_age since the former is not supported by Internet Explorer.
max_age
  Cookie max age in seconds.
path
  URI path in which the cookie is valid.
domain
  URI domain in which the cookie is valid.
secure
  If True, the cookie is only available via HTTPS.
httponly
  Disallow JavaScript to access the cookie.
comment
  Defines a cookie comment.
overwrite
  If true, overwrites previously set (and not yet sent to the client) cookies
  with the same name.

.. seealso::
   :ref:`How to read cookies from the request object <guide.request.cookies>`

Common Response attributes
--------------------------
status
  Status code plus message, e.g., '404 Not Found'. The status can be set
  passing an ``int``, e.g., ``request.status = 404``, or including the message,
  e.g., ``request.status = '404 Not Found'``.
status_int
  Status code as an ``int``, e.g., 404.
status_message
  Status message, e.g., 'Not Found'.
body
  The contents of the response, as a string.
unicode_body
  The contents of the response, as a unicode.
headers
  A dictionary-like object with headers. Keys are case-insensitive. It supports
  multiple values for a key, but you must use
  ``response.headers.add(key, value)`` to add keys. To get all values, use
  ``response.headers.getall(key)``.
headerlist
  List of headers, as a list of tuples ``(header_name, value)``.
charset
  Character encoding.
content_type
  'Content-Type' value from the headers, e.g., ``'text/html'``.
content_type_params
  Dictionary of extra Content-type parameters, e.g., ``{'charset': 'utf8'}``.
location
  'Location' header variable, used for redirects.
etag
  'ETag' header variable. You can automatically generate an etag based on the
  response body calling ``response.md5_etag()``.


Learn more about WebOb
----------------------
WebOb is an open source third-party library. See the `WebOb`_ documentation
for a detailed API reference and examples.


.. _WebOb: http://docs.webob.org/
