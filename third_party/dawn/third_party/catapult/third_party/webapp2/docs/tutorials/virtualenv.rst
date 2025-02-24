.. _tutorials.virtualenv:

Installing virtualenv
=====================
`virtualenv <http://pypi.python.org/pypi/virtualenv>`_, sets a
"virtual environment" that allows you to run different projects with separate
libraries side by side. This is a good idea both for development and
production, as it'll assure that each project uses their own library versions
and don't affect each other.

.. note::
   For App Engine development, virtualenv is not necessary. The SDK provides
   a "sandboxed environment" that serves almost the same purposes.

If you don't have a package installer in your system yet (like ``pip`` or
``easy_install``), install one. See :ref:`tutorials.installing.packages`.

Then follow these steps to install virtualenv:

**1.** To install it on a Linux or Mac systems, type in the command line:

.. code-block:: text

   $ sudo pip install virtualenv

Or, using easy_install:

.. code-block:: text

   $ sudo easy_install virtualenv

**2.** Then create a directory for your app, access it and setup a virtual
environment using the following command:

.. code-block:: text

   $ virtualenv env

**3.** Activate the environment. On Linux of Mac, use:

.. code-block:: text

   $ . env/bin/activate

Or on a Windows system:

.. code-block:: text

   $ env\scripts\activate
