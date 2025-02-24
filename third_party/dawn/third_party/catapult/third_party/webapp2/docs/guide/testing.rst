.. _guide.testing:

Unit testing
============
Thanks to `WebOb`_, webapp2 is very testable. Testing a handler is a matter
of building a custom ``Request`` object and calling ``get_response()`` on it
passing the WSGI application.

Let's see an example. First define a simple 'Hello world' handler to be
tested::

    import webapp2

    class HelloHandler(webapp2.RequestHandler):
        def get(self):
            self.response.write('Hello, world!')

    app = webapp2.WSGIapplication([('/', HelloHandler)])

    def main():
        app.run()

    if __name__ == '__main__':
        main()

To test if this handler returns the correct ``'Hello, world!'`` response, we
build a request object using ``Request.blank()`` and call ``get_response()``
on it::

    import unittest
    import webapp2

    # from the app main.py
    import main

    class TestHandlers(unittest.TestCase):
       def test_hello(self):
           # Build a request object passing the URI path to be tested.
           # You can also pass headers, query arguments etc.
           request = webapp2.Request.blank('/')
           # Get a response for that request.
           response = request.get_response(main.app)

           # Let's check if the response is correct.
           self.assertEqual(response.status_int, 200)
           self.assertEqual(response.body, 'Hello, world!')

To test different HTTP methods, just change the request object::

    request = webapp2.Request.blank('/')
    request.method = 'POST'
    response = request.get_response(main.app)

    # Our handler doesn't implement post(), so this response will have a
    # status code 405.
    self.assertEqual(response.status_int, 405)


Request.blank()
---------------
``Request.blank(path, environ=None, base_url=None, headers=None, POST=None, **kwargs)``
is a class method that creates a new request object for testing purposes. It
receives the following parameters:

path
  A URI path, urlencoded. The path will become path_info, with any query
  string split off and used.
environ
  An environ dictionary.
base_url
  If defined, wsgi.url_scheme, HTTP_HOST and SCRIPT_NAME will be filled in
  from this value.
headers
  A list of ``(header_name, value)`` tuples for the request headers.
POST
  A dictionary of POST data to be encoded, or a urlencoded string. This is a
  shortcut to set POST data in the environ. When set, the HTTP method is set
  to 'POST' and the CONTENT_TYPE is set to 'application/x-www-form-urlencoded'.
kwargs
  Extra keyword arguments to be passed to ``Request.__init__()``.

All necessary keys will be added to the environ, but the values you pass in
will take precedence.


app.get_response()
------------------
We can also get a response directly from the WSGI application, calling
``app.get_response()``. This is a convenience to test the app. It receives
the same parameters as ``Request.blank()`` to build a request and call the
application, returning the resulting response::

    class HelloHandler(webapp2.RequestHandler):
        def get(self):
            self.response.write('Hello, world!')

    app = webapp2.WSGIapplication([('/', HelloHandler)])

    # Test the app, passing parameters to build a request.
    response = app.get_response('/')
    assert response.status_int == 200
    assert response.body == 'Hello, world!'

Testing handlers could not be easier. Check the `WebOb`_ documentation for more
information about the request and response objects.


Testing App Engine services
---------------------------
If you're using App Engine and need to test an application that uses Datastore,
Memcache or other App Engine services, read
`Local Unit Testing for Python <http://code.google.com/appengine/docs/python/tools/localunittesting.html>`_
in the official documentation. The App Engine SDK provides the module
``google.appengine.ext.testbed`` that can be used to setup all the necessary
service stubs for testing.


.. _WebOb: http://docs.webob.org/
