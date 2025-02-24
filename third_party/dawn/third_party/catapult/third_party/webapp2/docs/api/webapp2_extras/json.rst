.. _api.webapp2_extras.json:

JSON
====
.. module:: webapp2_extras.json

This is a wrapper for the :py:mod:`json` module: it will use simplejson if
available, or the ``json`` module from Python >= 2.6 if available, and as a
last resource the ``django.utils.simplejson`` module on App Engine.

It will also escape forward slashes and, by default, output the serialized
JSON in a compact format, eliminating white spaces.

Some convenience functions are also available to encode and decode to and from
base64 and to quote or unquote the values.

.. autofunction:: encode
.. autofunction:: decode
.. autofunction:: b64encode
.. autofunction:: b64decode
.. autofunction:: quote
.. autofunction:: unquote


.. _Tornado: http://www.tornadoweb.org/
