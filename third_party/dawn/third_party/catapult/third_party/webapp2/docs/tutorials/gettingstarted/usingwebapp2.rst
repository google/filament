.. _tutorials.gettingstarted.usingwebapp2:

Using the webapp2 Framework
===========================
The CGI standard is simple, but it would be cumbersome to write all of the
code that uses it by hand. Web application frameworks handle these details
for you, so you can focus your development efforts on your application's
features. Google App Engine supports any framework written in pure Python
that speaks CGI (and any
`WSGI <http://www.python.org/dev/peps/pep-0333/>`_-compliant framework using a
CGI adaptor). You can bundle a framework of your choosing with your application
code by copying its code into your application directory.

App Engine includes a simple web application framework of its own, called
``webapp``. The ``webapp`` framework is already installed in the App Engine
environment and in the SDK, and as ``webapp2`` is based on it, you only need
to bundle a single file with your application code to use it. We will use
``webapp2`` for the rest of this tutorial.

Follow these steps to bundle the ``webapp2`` framework with your application:

- Create a file ``webapp2.py`` inside your application directory. Paste the
  contents from `webapp2.py <http://code.google.com/p/webapp-improved/source/browse/webapp2.py>`_
  inside it.
- There's no second step. You can start using webapp2 right now.


Hello, webapp2!
---------------
A ``webapp2`` application has three parts:

- One or more ``RequestHandler`` classes that process requests and build
  responses.
- A ``WSGIApplication`` instance that routes incoming requests to handlers
  based on the URL.
- A main routine that runs the ``WSGIApplication`` using a CGI adaptor.

Let's rewrite our friendly greeting as a ``webapp2`` application. Edit
``helloworld/helloworld.py`` and replace its contents with the following::

    import webapp2

    class MainPage(webapp2.RequestHandler):
        def get(self):
            self.response.headers['Content-Type'] = 'text/plain'
            self.response.out.write('Hello, webapp2 World!')

    application = webapp2.WSGIApplication([
        ('/', MainPage)
    ], debug=True)

Also edit ``app.yaml`` and replace its contents with the following:

.. code-block:: yaml

   application: helloworld
   version: 1
   runtime: python27
   api_version: 1
   threadsafe: true

   handlers:
   - url: /.*
     script: helloworld.app

Reload `http://localhost:8080/ <http://localhost:8080/>`_ in your browser to
see the new version in action (if you stopped your web server, restart it by
running the command described in ":ref:`tutorials.gettingstarted.helloworld`").


What webapp2 Does
-----------------
This code defines one request handler, ``MainPage``, mapped to the root URL
(``/``). When ``webapp2`` receives an HTTP GET request to the URL ``/``, it
instantiates the ``MainPage`` class and calls the instance's ``get`` method.
Inside the method, information about the request is available using
``self.request``. Typically, the method sets properties on ``self.response``
to prepare the response, then exits. ``webapp2`` sends a response based on
the final state of the ``MainPage`` instance.

The application itself is represented by a ``webapp2.WSGIApplication``
instance. The parameter ``debug=true`` passed to its constructor tells
``webapp2`` to print stack traces to the browser output if a handler
encounters an error or raises an uncaught exception. You may wish to remove
this option from the final version of your application.

The code ``application.run()`` runs the application in App Engine's CGI
environment. It uses a function provided by App Engine that is similar to the
WSGI-to-CGI adaptor provided by the ``wsgiref`` module in the Python standard
library, but includes a few additional features. For example, it can
automatically detect whether the application is running in the development
server or on App Engine, and display errors in the browser if it is running
on the development server.

We'll use a few more features of ``webapp2`` later in this tutorial. For more
information about ``webapp2``, see the webapp2 reference.


Next...
-------
Frameworks make web application development easier, faster and less error
prone. webapp2 is just one of many such frameworks available for Python.
Now that we're using a framework, let's add some features.

Continue to :ref:`tutorials.gettingstarted.usingusers`.
