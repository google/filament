.. _api.webapp2_extras.config:

Configuration
=============
This module is deprecated. :class:`webapp2.WSGIApplication` now has a simple
configuration dictionary used by default, stored in
:class:`webapp2.WSGIApplication.config`. See also :class:`webapp2.Config`.

.. module:: webapp2_extras.config

.. autodata:: DEFAULT_VALUE

.. autodata:: REQUIRED_VALUE

.. autoclass:: Config
   :members: __init__, __getitem__, __setitem__, get, setdefault, update,
             get_config
