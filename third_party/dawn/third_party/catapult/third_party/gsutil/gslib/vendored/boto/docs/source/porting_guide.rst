Porting Guide
=============
Boto supports Python versions 2.6, 2.7, 3.3, and 3.4. Python 3 support
is on a per-module basis. This guide will help you to get started porting
a Boto module to Python 3.

Please read the :doc:`Contributing Guide <contributing>` before getting
started.

Compat Module
-------------
Boto ships with a ``boto.compat`` module that helps to abstract the
differences between Python versions. A vendored version of the ``six``
module is exposed through ``boto.compat.six``, as well as a handful of
moved functions used throughout the codebase::

    # Import the six module
    from boto.compat import six

    # Other useful imports
    from boto.compat import BytesIO, StringIO
    from boto.compat import http_client
    from boto.compat import urlparse

Please check the ``boto.compat`` module before writing your own logic
around specialized behavior for different Python versions. Feel free
to add new functionality here, too.

Porting Steps
-------------
Please follow the following steps when porting a module:

* Install Python versions and ``pip install tox``
* Port your module to support Python 3. These help:

  * `Six documentation`_
  * `Porting to Python 3 An in-depth guide`_
  * `Porting to Python 3 Redux`_

* Whitelist your module's unit tests in ``tests/test.py``
* Make sure unit tests pass by running ``tox``
* Try running integration tests::

    tox tests/integration/yourmodule

    # You can also run against a specific Python version:
    tox -e py26 tests/integration/yourmodule

* Fix any failing tests. This is the fun part!
* If code you modified is not covered by tests, try to cover it with
  existing tests or write new tests. Here is how you can generate a
  coverage report in ``cover/index.html``::

    # Run a test with coverage
    tox -e py33 -- default --with-coverage --cover-html --cover-package boto

* Update ``README.rst`` and ``docs/source/index.rst`` to label your module
  as supporting Python 3
* Submit a pull request!

Note: We try our best to clean up resources after a test runs, but you should
double check that no resources are left after integration tests run. If they
are, then you will be charged for them!

.. _Six documentation: http://pythonhosted.org/six/
.. _Porting to Python 3 An in-depth guide: http://python3porting.com/
.. _Porting to Python 3 Redux: http://lucumr.pocoo.org/2013/5/21/porting-to-python-3-redux/
