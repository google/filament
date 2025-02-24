.. _index:

.. webapp2 documentation master file, created by
   sphinx-quickstart on Sat Jul 31 10:41:37 2010.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to webapp2!
===================
`webapp2`_ is a lightweight Python web framework compatible with Google App
Engine's `webapp`_.

webapp2 is a `simple`_. it follows the simplicity of webapp, but improves
it in some ways: it adds better URI routing and exception handling, a full
featured response object and a more flexible dispatching mechanism.

webapp2 also offers the package :ref:`webapp2_extras <index.api-reference-webapp2-extras>`
with several optional utilities: sessions, localization, internationalization,
domain and subdomain routing, secure cookies and others.

webapp2 can also be used outside of Google App Engine, independently of the
App Engine SDK.

For a complete description of how webapp2 improves webapp, see :ref:`features`.

.. note::
   webapp2 is part of the Python 2.7 runtime since App Engine SDK 1.6.0.
   To include it in your app see
   `Configuring Libraries <http://code.google.com/appengine/docs/python/python27/using27.html#Configuring_Libraries>`_.


Quick links
-----------
- `Downloads <http://code.google.com/p/webapp-improved/downloads/list>`_
- `Google Code Repository <http://code.google.com/p/webapp-improved/>`_
- `Discussion Group <http://groups.google.com/group/webapp2>`_
- `@webapp2 <https://twitter.com/#!/webapp2>`_

.. `Samples for Google App Engine <http://code.google.com/p/google-app-engine-samples/>`_:
   several mini-apps for webapp that serve as examples (they should work with
   webapp2 too).


Tutorials
---------
.. toctree::
   :maxdepth: 1

   tutorials/quickstart.rst
   tutorials/quickstart.nogae.rst
   tutorials/gettingstarted/index.rst
   tutorials/i18n.rst


Guide
-----
.. toctree::
   :maxdepth: 3

   guide/app.rst
   guide/routing.rst
   guide/handlers.rst
   guide/request.rst
   guide/response.rst
   guide/exceptions.rst
   guide/testing.rst
   guide/extras.rst


API Reference - webapp2
-----------------------
.. toctree::
   :maxdepth: 2

   api/webapp2.rst


.. _index.api-reference-webapp2-extras:

API Reference - webapp2_extras
------------------------------
.. toctree::
   :maxdepth: 1

   api/webapp2_extras/auth.rst
   api/webapp2_extras/i18n.rst
   api/webapp2_extras/jinja2.rst
   api/webapp2_extras/json.rst
   api/webapp2_extras/local.rst
   api/webapp2_extras/mako.rst
   api/webapp2_extras/routes.rst
   api/webapp2_extras/securecookie.rst
   api/webapp2_extras/security.rst
   api/webapp2_extras/sessions.rst


API Reference - webapp2_extras.appengine
----------------------------------------
Modules that use App Engine libraries and services are restricted to
``webapp2_extras.appengine``.

.. toctree::
   :maxdepth: 1

   api/webapp2_extras/appengine/sessions_memcache.rst
   api/webapp2_extras/appengine/sessions_ndb.rst
   api/webapp2_extras/appengine/auth/models.rst
   api/webapp2_extras/appengine/users.rst


Miscelaneous
------------
.. toctree::
   :maxdepth: 1

   features.rst


Indices and tables
------------------
* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`


.. Old docs linking to new ones or pages we don't want to link.

.. toctree::
   :hidden:

   api/index.rst
   api/extras.config.rst
   api/extras.i18n.rst
   api/extras.jinja2.rst
   api/extras.json.rst
   api/extras.local.rst
   api/extras.local_app.rst
   api/extras.mako.rst
   api/extras.routes.rst
   api/extras.securecookie.rst
   api/extras.security.rst
   api/extras.sessions.rst
   api/extras.sessions_memcache.rst
   api/extras.sessions_ndb.rst
   api/extras.users.rst
   guide/index.rst
   tutorials/index.rst
   todo.rst


Requirements
------------
webapp2 is compatible with Python 2.5 and superior. No Python 3 yet.

`WebOb`_ is the only library required for the core functionality.

Modules from webapp2_extras may require additional libraries, as indicated in
their docs.


Credits
-------
webapp2 is a superset of `webapp`_, created by the App Engine team.

Because webapp2 is intended to be compatible with webapp, the official webapp
documentation is valid for webapp2 too. Parts of this documentation were ported
from the `App Engine documentation`_, written by the App Engine team and
licensed under the Creative Commons Attribution 3.0 License.

webapp2 has code ported from `Werkzeug`_ and `Tipfy`_.

webapp2_extras has code ported from Werkzeug, Tipfy and `Tornado Web Server`_.

The `Sphinx`_ theme mimics the App Engine documentation.

This library was not created and is not maintained by Google.


Contribute
----------
webapp2 is considered stable, feature complete and well tested, but if you
think something is missing or is not working well, please describe it in our
issue tracker:

    http://code.google.com/p/webapp-improved/issues/list

Let us know if you found a bug or if something can be improved. New tutorials
and webapp2_extras modules are also welcome, and tests or documentation are
never too much.

Thanks!


License
-------
webapp2 is licensed under the `Apache License 2.0`_.


.. _webapp: http://code.google.com/appengine/docs/python/tools/webapp/
.. _webapp2: http://code.google.com/p/webapp-improved/
.. _simple: http://code.google.com/p/webapp-improved/source/browse/webapp2.py
.. _WebOb: http://docs.webob.org/
.. _Werkzeug: http://werkzeug.pocoo.org/
.. _Tipfy: http://www.tipfy.org/
.. _Tornado Web Server: http://www.tornadoweb.org/
.. _Sphinx: http://sphinx.pocoo.org/
.. _App Engine documentation: http://code.google.com/appengine/docs/
.. _Apache License 2.0: http://www.apache.org/licenses/LICENSE-2.0
