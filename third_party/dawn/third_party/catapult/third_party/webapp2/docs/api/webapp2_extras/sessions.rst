.. _api.webapp2_extras.sessions:

Sessions
========
.. module:: webapp2_extras.sessions

This module provides a lightweight but flexible session support for webapp2.

It has three built-in backends: secure cookies, memcache and datastore.
New backends can be added extending :class:`CustomBackendSessionFactory`.

The session store can provide multiple sessions using different keys,
even using different backends in the same request, through the method
:meth:`SessionStore.get_session`. By default it returns a session using the
default key from configuration.

.. autodata:: default_config

.. autoclass:: SessionStore
   :members: __init__, get_backend, get_session, save_sessions

.. autoclass:: SessionDict
   :members: get_flashes, add_flash

.. autoclass:: SecureCookieSessionFactory

.. autoclass:: CustomBackendSessionFactory

.. autofunction:: get_store
.. autofunction:: set_store
