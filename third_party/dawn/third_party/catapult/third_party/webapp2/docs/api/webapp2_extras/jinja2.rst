.. _api.webapp2_extras.jinja2:

Jinja2
======
.. module:: webapp2_extras.jinja2

This module provides `Jinja2`_ template support for webapp2.

To use it, you must add the ``jinja2`` package to your application
directory (for App Engine) or install it in your virtual environment
(for other servers).

You can download ``jinja2`` from PyPi:

    http://pypi.python.org/pypi/Jinja2

Learn more about Jinja2:

    http://jinja.pocoo.org/

.. autodata:: default_config

.. autoclass:: Jinja2
   :members: __init__, render_template, get_template_attribute

.. autofunction:: get_jinja2
.. autofunction:: set_jinja2


.. _Jinja2: http://jinja.pocoo.org/
