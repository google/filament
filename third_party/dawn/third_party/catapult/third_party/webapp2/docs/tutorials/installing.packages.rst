.. _tutorials.installing.packages:

Installing packages
===================
To use webapp2 outside of App Engine, you need a package manager to install
dependencies in your system -- mostly WebOb, and maybe libraries required
by the various webapp2_extras modules, if you will use them.

For App Engine, some webapp2_extras modules may require that you install
external packages to access specific command line tools (i18n, for example,
uses pybabel to extract and compile translation catalogs).

In this tutorial we'll show how to install a package manager and installer.


Install a distutils library
---------------------------
If you don't have a distutils library (`distribute <http://pypi.python.org/pypi/distribute>`_
or `setuptools <http://pypi.python.org/pypi/setuptools>`_) installed on
you system yet, you need to install one. Distribute is recommended, but
setuptools will serve as well.

Distribute is "the standard method for working with Python module
distributions". It will manage our package dependencies and upgrades.
If you already have one of them, jump to next step. If not, the installation
is straighforward:

**1.** Download the installer and save it anywhere. It is a single file:

    http://python-distribute.org/distribute_setup.py

**2.** Execute it from the command line (this will require sudo if you are
using Linux or a Mac):

.. code-block:: text

   $ python distribute_setup.py

If you don't see any error messages, yay, it installed successfully. Let's
move forward. For Windows, check the distribute or setuptools documentation.


Install a package installer
---------------------------
We need a package installer (``pip`` or ``easy_install``) to install and
update Python packages. Any will work, but if you don't have one yet, ``pip``
is recommended. Here's how to install it:

**1.** Download ``pip`` from PyPi:

    http://pypi.python.org/pypi/pip

**2.** Unpack it and access the unpacked directory using the command line.
Then run ``setup.py install`` on that directory (this will require sudo if you
are using Linux or a Mac):

.. code-block:: text

   $ python setup.py install

That's it. If you don't see any error messages, the ``pip`` command should
now be available in your system.
