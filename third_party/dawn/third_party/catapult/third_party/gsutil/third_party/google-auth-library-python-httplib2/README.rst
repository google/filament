``httplib2`` Transport for Google Auth
======================================

|pypi|

This library provides an `httplib2`_ transport for `google-auth`_.

.. note:: ``httplib`` has lots of problems such as lack of threadsafety
    and insecure usage of TLS. Using it is highly discouraged. This
    library is intended to help existing users of ``oauth2client`` migrate to
    ``google-auth``.

.. |pypi| image:: https://img.shields.io/pypi/v/google-auth-httplib2.svg
   :target: https://pypi.python.org/pypi/google-auth-httplib2

.. _httplib2: https://github.com/httplib2/httplib2
.. _google-auth: https://github.com/GoogleCloudPlatform/google-auth-library-python/

Installing
----------

You can install using `pip`_::

    $ pip install google-auth-httplib2

.. _pip: https://pip.pypa.io/en/stable/

License
-------

Apache 2.0 - See `the LICENSE`_ for more information.

.. _the LICENSE: https://github.com/GoogleCloudPlatform/google-auth-library-python/blob/main/LICENSE
