.. _features:

webapp2 features
================
Here's an overview of the main improvements of webapp2 compared to webapp.

.. contents:: Table of Contents
   :depth: 3
   :backlinks: none


Compatible with webapp
----------------------
webapp2 is designed to work with existing webapp apps without any changes.
See how this looks familiar::

    import webapp2 as webapp
    from google.appengine.ext.webapp.util import run_wsgi_app

    class HelloWorldHandler(webapp.RequestHandler):
        def get(self):
            self.response.out.write('Hello, World!')

    app = webapp.WSGIApplication([
        ('/', HelloWorldHandler),
    ], debug=True)

    def main():
        run_wsgi_app(app)

    if __name__ == '__main__':
        main()

Everybody starting with App Engine must know a bit of webapp. And you you can
use webapp2 exactly like webapp, following the official tutorials, and learn
the new features later, as you go. This makes webapp2 insanely easy to learn.

Also, the SDK libraries that use webapp can be used with webapp2 as they are
or with minimal adaptations.


Compatible with latest WebOb
----------------------------
The ``WebOb`` version included in the App Engine SDK was released in 2008.
Since then many bugs were fixed and the source code became cleaner and better
documented. webapp2 is compatible with the ``WebOb`` version included in the
SDK, but for those that prefer the latest version can be used as well.
This avoids the bugs
`#170 <http://code.google.com/p/googleappengine/issues/detail?id=170>`_,
`#719 <http://code.google.com/p/googleappengine/issues/detail?id=719>`_ and
`#2788 <http://code.google.com/p/googleappengine/issues/detail?id=2788>`_,
at least.


Full-featured response object
-----------------------------
webapp2 uses a full-featured response object from ``WebOb``. If offers several
conveniences to set headers, like easy cookies and other goodies::

    class MyHandler(webapp2.RequestHandler):
        def get(self):
            self.response.set_cookie('key', 'value', max_age=360, path='/')


Status code exceptions
----------------------
``abort()`` (or ``self.abort()`` inside handlers) raises a proper
``HTTPException`` (from ``WebOb``) and stops processing::

    # Raise a 'Not Found' exception and let the 404 error handler do its job.
    abort(404)
    # Raise a 'Forbidden' exception and let the 403 error handler do its job.
    self.abort(403)


Improved exception handling
---------------------------
HTTP exceptions can also be handled by the WSGI application::

    # ...
    import logging

    def handle_404(request, response, exception):
        logging.exception(exception)
        response.write('Oops! I could swear this page was here!')
        response.set_status(404)

    app = webapp2.WSGIApplication([
        ('/', MyHandler),
    ])
    app.error_handlers[404] = handle_404


Lazy handlers
-------------
Lazy handlers can be defined as a string to be imported only when needed::

    app = webapp2.WSGIApplication([
        ('/', 'my.module.MyHandler'),
    ])


Keyword arguments from URI
--------------------------
``RequestHandler`` methods can also receive keyword arguments, which are easier
to maintain than positional ones. Simply use the ``Route`` class to define
URIs (and you can also create custom route classes, examples
`here <http://code.google.com/p/webapp-improved/source/browse/webapp2_extras/routes.py>`_)::

    class BlogArchiveHandler(webapp2.RequestHandler):
        def get(self, year=None, month=None):
            self.response.write('Hello, keyword arguments world!')

    app = webapp2.WSGIApplication([
        webapp2.Route('/<year:\d{4}>/<month:\d{2}>', handler=BlogArchiveHandler, name='blog-archive'),
    ])


Positional arguments from URI
-----------------------------
Positional arguments are also supported, as URI routing is fully compatible
with webapp::

    class BlogArchiveHandler(webapp2.RequestHandler):
        def get(self, year, month):
            self.response.write('Hello, webapp routing world!')

    app = webapp2.WSGIApplication([
        ('/(\d{4})/(\d{2})', BlogArchiveHandler),
    ])


Returned responses
------------------
Several Python frameworks adopt the pattern on returning a response object,
instead of writing to an existing response object like webapp. For those that
prefer, webapp2 supports this: simply return a response object from a handler
and it will be used instead of the one created by the application::

    class BlogArchiveHandler(webapp2.RequestHandler):
        def get(self):
            return webapp2.Response('Hello, returned response world!')

    app = webapp2.WSGIApplication([
        webapp2.Route('/', handler=HomeHandler, name='home'),
    ])


Custom handler methods
----------------------
webapp2 routing and dispatching system can do a lot more than webapp.
For example, handlers can also use custom methods::

    class MyHandler(webapp2.RequestHandler):
        def my_custom_method(self):
            self.response.write('Hello, custom method world!')

        def my_other_method(self):
            self.response.write('Hello, another custom method world!')

    app = webapp2.WSGIApplication([
        webapp2.Route('/', handler=MyHandler, name='custom-1', handler_method='my_custom_method'),
        webapp2.Route('/other', handler=MyHandler, name='custom-2', handler_method='my_other_method'),
    ])


View functions
--------------
In webapp2 handlers don't need necessarily to be classes. For those that
prefer, functions can be used as well::

    def my_sweet_function(request, *args, **kwargs):
        return webapp2.Response('Hello, function world!')

    app = webapp2.WSGIApplication([
        webapp2.Route('/', handler=my_sweet_function, name='home'),
    ])


More flexible dispatching mechanism
-----------------------------------
The ``WSGIApplication`` in webapp is hard to modify. It dispatches the
handler giving little chance to define how it is done, or to pre-process
requests before a handler method is actually called. In webapp2 the handlers
dispatch themselves, making it easy to implement before and after dispatch
hooks.

webapp2 is thought to be lightweight but flexible. It basically provides an
easy to customize URI routing and dispatching mechanisms: you can even extend
how URIs are matched or built or how handlers are adapted or dispatched
without subclassing.

For an example of webapp2's flexibility,
see :ref:`guide.handlers.a.micro.framework.based.on.webapp2`.


Domain and subdomain routing
----------------------------
webapp2 supports :ref:`domain and subdomain routing <guide.routing.domain-and-subdomain-routing>`
to restrict URI matches based on the server name::

    routes.DomainRoute('www.mydomain.com', [
        webapp2.Route('/', handler=HomeHandler, name='home'),
    ])


Match HTTP methods or URI schemes
---------------------------------
webapp2 routing system allows routes to be restricted to the
:ref:`HTTP method <guide.routing.restricting-http-methods>` or a specific
:ref:`URI scheme <guide.routing.restricting-uri-schemes>`. You can set routes
that will only match requests using 'https', for example.


URI builder
-----------
URIs defined in the aplication can be built. This is more maintanable than
hardcoding them in the code or templates. Simply use the ``uri_for()``
function::

    uri = uri_for('blog-archive', year='2010', month='07')

And a handler helper for redirects builds the URI to redirect to.
redirect_to = redirect + uri_for::

    self.redirect_to('blog-archive', year='2010', month='07')


Redirection for legacy URIs
---------------------------
Old URIs can be conveniently redirected using a simple route::

    def get_redirect_uri(handler, *args, **kwargs):
        return handler.uri_for('view', item=kwargs.get('item'))

    app = webapp2.WSGIApplication([
        webapp2.Route('/view/<item>', ViewHandler, 'view'),
        webapp2.Route('/old-page', RedirectHandler, defaults={'uri': '/view/i-came-from-a-redirect'}),
        webapp2.Route('/old-view/<item>', RedirectHandler, defaults={'uri': get_redirect_uri}),
    ])


Simple, well-tested and documented
----------------------------------
webapp2 is `simple <http://code.google.com/p/webapp-improved/source/browse/webapp2.py>`_,
extensively documented and has almost 100% test coverage. The source code is
explicit, magic-free and made to be extended. We like less.


Independent of the App Engine SDK
---------------------------------
webapp2 doesn't depend on the Google App Engine SDK and
:ref:`can be used outside of App Engine <tutorials.quickstart.nogae>`.
If the SDK is not found, it has fallbacks to be used in any server as a
general purpose web framework.


Future proof
------------
Because it works on threaded environments, webapp2 is ready for when
App Engine introduces threading support in the Python 2.7 runtime.


Same performance
----------------
Best of all is that with all these features, there is no loss of performance:
cold start times are the same as webapp. Here are some logs of a 'Hello World'
cold start:

.. code-block:: text

   100ms 77cpu_ms
   143ms 58cpu_ms
   155ms 77cpu_ms
   197ms 96cpu_ms
   106ms 77cpu_ms


Extras
------
The `webapp2_extras <http://code.google.com/p/webapp-improved/source/browse/#hg%2Fwebapp2_extras>`_
package provides common utilities that integrate well with webapp2:

- Localization and internationalization support
- Sessions using secure cookies, memcache or datastore
- Extra route classes -- to match subdomains and other conveniences
- Support for third party libraries: Jinja2 and Mako
- Support for threaded environments, so that you can use webapp2 outside of
  App Engine or in the upcoming App Engine Python 2.7 runtime
