.. _api.webapp2:

webapp2
=======
.. module:: webapp2

- WSGI app

  - :class:`WSGIApplication`
  - :class:`RequestContext`

- URI routing

  - :class:`Router`
  - :class:`BaseRoute`
  - :class:`SimpleRoute`
  - :class:`Route`

- Configuration

  - :class:`Config`

- Request and Response

  - :class:`Request`
  - :class:`Response`

- Request handlers

  - :class:`RequestHandler`
  - :class:`RedirectHandler`

- Utilities

  - :class:`cached_property`
  - :func:`get_app`
  - :func:`get_request`
  - :func:`redirect`
  - :func:`redirect_to`
  - :func:`uri_for`
  - :func:`abort`
  - :func:`import_string`
  - :func:`urlunsplit`


WSGI app
--------
.. seealso::
   :ref:`guide.app`

.. autoclass:: WSGIApplication
   :members: request_class, response_class, request_context_class,
             router_class, config_class,
             debug, router, config, registry, error_handlers, app, request,
             active_instance, allowed_methods,
             __init__, __call__, set_globals, clear_globals,
             handle_exception, run, get_response

.. autoclass:: RequestContext
   :members: __init__, __enter__, __exit__


URI routing
-----------
.. seealso::
   :ref:`guide.app.router` and :ref:`guide.routing`

.. autoclass:: Router
   :members: route_class, __init__, add,
             match, build,
             dispatch, adapt,
             default_matcher, default_builder,
             default_dispatcher, default_adapter,
             set_matcher, set_builder,
             set_dispatcher, set_adapter

.. autoclass:: BaseRoute
   :members: template, name, handler, handler_method, handler_adapter,
             build_only, match, build, get_routes, get_match_routes,
             get_build_routes

.. autoclass:: SimpleRoute
   :members: __init__, match

.. autoclass:: Route
   :members: __init__, match, build


Configuration
-------------
.. seealso::
   :ref:`guide.app.config`

.. autoclass:: Config
   :members: __init__, load_config


Request and Response
--------------------
.. seealso::
   :ref:`guide.request` and :ref:`guide.response`

.. autoclass:: Request
   :members: app, response, route, route_args, route_kwargs, registry,
             __init__, get, get_all, arguments, get_range


.. autoclass:: Response
   :members: __init__, status, status_message, has_error, clear, wsgi_write,
             http_status_message


Request handlers
----------------
.. seealso::
   :ref:`guide.handlers`

.. autoclass:: RequestHandler
   :members: app, request, response, __init__, initialize, dispatch, error,
             abort, redirect, redirect_to, uri_for, handle_exception


.. autoclass:: RedirectHandler
   :members: get


Utilities
---------
These are some other utilities also available for general use.

.. autoclass:: cached_property

.. autofunction:: get_app

.. autofunction:: get_request

.. autofunction:: redirect

.. autofunction:: redirect_to

.. autofunction:: uri_for

.. autofunction:: abort

.. autofunction:: import_string


.. _Another Do-It-Yourself Framework: http://docs.webob.org/en/latest/do-it-yourself.html
.. _Flask: http://flask.pocoo.org/
.. _Tornado: http://www.tornadoweb.org/
.. _WebOb: http://docs.webob.org/
.. _Werkzeug: http://werkzeug.pocoo.org/
