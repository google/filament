====================
Contributing to Boto
====================


Setting Up a Development Environment
====================================

While not strictly required, it is highly recommended to do development
in a virtualenv.  You can install virtualenv using pip::

    $ pip install virtualenv

Once the package is installed, you'll have a ``virtualenv`` command you can
use to create a virtual environment::

    $ virtualenv venv

You can then activate the virtualenv::

    $ . venv/bin/activate

.. note::

  You may also want to check out virtualenvwrapper_, which is a set of
  extensions to virtualenv that makes it easy to manage multiple virtual
  environments.

A requirements.txt is included with boto which contains all the additional
packages needed for boto development.  You can install these packages by
running::

    $ pip install -r requirements.txt


Running the Tests
=================

All of the tests for boto are under the ``tests/`` directory. The tests for
boto have been split into two main categories, unit and integration tests:

* **unit** - These are tests that do not talk to any AWS services.  Anyone
  should be able to run these tests without have any credentials
  configured.  These are the types of tests that could be run in something
  like a public CI server.  These tests tend to be fast.

* **integration** - These are tests that will talk to AWS services, and
  will typically require a boto config file with valid credentials.
  Due to the nature of these tests, they tend to take a while to run.
  Also keep in mind anyone who runs these tests will incur any usage
  fees associated with the various AWS services.

To run all the unit tests, cd to the ``tests/`` directory and run::

    $ python test.py unit

You should see output like this::

    $ python test.py unit
    ................................
    ----------------------------------------------------------------------
    Ran 32 tests in 0.075s

    OK

To run the integration tests, run::

    $ python test.py integration

Note that running the integration tests may take a while.

Various integration tests have been tagged with service names to allow
you to easily run tests by service type.  For example, to run the ec2
integration tests you can run::

    $ python test.py -t ec2

You can specify the ``-t`` argument multiple times.  For example, to
run the s3 and ec2 tests you can run::

    $ python test.py -t ec2 -t s3

.. warning::

  In the examples above no top level directory was specified.  By default,
  nose will assume the current working directory, so the above command is
  equivalent to::

      $ python test.py -t ec2 -t s3 .

  Be sure that you are in the ``tests/`` directory when running the tests,
  or explicitly specify the top level directory.  For example, if you in the
  root directory of the boto repo, you could run the ec2 and s3 tests by
  running::

      $ python tests/test.py -t ec2 -t s3 tests/


You can use nose's collect plugin to see what tests are associated with each
service tag::

    $ python tests.py -t s3 -t ec2 --with-id --collect -v


Testing Details
---------------

The ``tests/test.py`` script is a lightweight wrapper around nose_. In
general, you should be able to run ``nosetests`` directly instead of
``tests/test.py``.  The ``tests/unit`` and ``tests/integration`` args
in the commands above were referring to directories.  The command line
arguments are forwarded to nose when you use ``tests/test.py``.  For example,
you can run::

    $ python tests/test.py -x -vv tests/unit/cloudformation

And the ``-x -vv tests/unit/cloudformation`` are forwarded to nose.  See
the nose_ docs for the supported command line options, or run
``nosetests --help``.

The only thing that ``tests/test.py`` does before invoking nose is to
inject an argument that specifies that any testcase tagged with "notdefault"
should not be run.  A testcase may be tagged with "notdefault" if the test
author does not want everyone to run the tests.  In general, there shouldn't be
many of these tests, but some reasons a test may be tagged "notdefault"
include:

* An integration test that requires specific credentials.
* An interactive test (the S3 MFA tests require you to type in the S/N and
  code).

Tagging is done using nose's tagging_ plugin.  To summarize, you can tag a
specific testcase by setting an attribute on the object.  Nose provides
an ``attr`` decorator for convenience::

    from nose.plugins.attrib import attr

    @attr('notdefault')
    def test_s3_mfs():
        pass

You can then run these tests be specifying::

    nosetests -a 'notdefault'

Or you can exclude any tests tagged with 'notdefault' by running::

    nosetests -a '!notdefault'

Conceptually, ``tests/test.py`` is injecting the "-a !notdefault" arg
into nosetests.


Testing Supported Python Versions
=================================

Boto supports python 2.6 and 2.7. An easy way to verify functionality
across multiple python versions is to use tox_. A tox.ini file is included
with boto.  You can run tox with no args and it will automatically test
all supported python versions::

    $ tox
    GLOB sdist-make: boto/setup.py
    py26 sdist-reinst: boto/.tox/dist/boto-2.4.1.zip
    py26 runtests: commands[0]
    ................................
    ----------------------------------------------------------------------
    Ran 32 tests in 0.089s

    OK
    py27 sdist-reinst: boto/.tox/dist/boto-2.4.1.zip
    py27 runtests: commands[0]
    ................................
    ----------------------------------------------------------------------
    Ran 32 tests in 0.087s

    OK
    ____ summary ____
      py26: commands succeeded
      py27: commands succeeded
      congratulations :)


Writing Documentation
=====================

The boto docs use sphinx_ to generate documentation.  All of the docs are
located in the ``docs/`` directory.  To generate the html documentation, cd
into the docs directory and run ``make html``::

    $ cd docs
    $ make html

The generated documentation will be in the ``docs/build/html`` directory.
The source for the documentation is located in ``docs/source`` directory,
and uses `restructured text`_ for the markup language.


.. _nose: http://readthedocs.org/docs/nose/en/latest/
.. _tagging: http://nose.readthedocs.org/en/latest/plugins/attrib.html
.. _tox: http://tox.testrun.org/latest/
.. _virtualenvwrapper: http://www.doughellmann.com/projects/virtualenvwrapper/
.. _sphinx: http://sphinx.pocoo.org/
.. _restructured text: http://sphinx.pocoo.org/rest.html


Merging A Branch (Core Devs)
============================

* All features/bugfixes should go through a review.

  * This includes new features added by core devs themselves. The usual
    branch/pull-request/merge flow that happens for community contributions
    should also apply to core.

* Ensure there is proper test coverage. If there's a change in behavior, there
  should be a test demonstrating the failure before the change & passing with
  the change.

  * This helps ensure we don't regress in the future as well.

* Merging of pull requests is typically done with
  ``git merge --no-ff <remote/branch_name>``.

  * GitHub's big green button is probably OK for very small PRs (like doc
    fixes), but you can't run tests on GH, so most things should get pulled
    down locally.
