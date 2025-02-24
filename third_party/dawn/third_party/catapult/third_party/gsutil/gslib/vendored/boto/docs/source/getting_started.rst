.. _getting-started:

=========================
Getting Started with Boto
=========================

This tutorial will walk you through installing and configuring ``boto``, as
well how to use it to make API calls.

This tutorial assumes you are familiar with Python & that you have registered
for an `Amazon Web Services`_ account. You'll need retrieve your
``Access Key ID`` and ``Secret Access Key`` from the web-based console.

.. _`Amazon Web Services`: https://aws.amazon.com/


Installing Boto
---------------

You can use ``pip`` to install the latest released version of ``boto``::

    pip install boto

If you want to install ``boto`` from source::

    git clone git://github.com/boto/boto.git
    cd boto
    python setup.py install

.. note::

    For most services, this is enough to get going. However, to support
    everything Boto ships with, you should additionally run
    ``pip install -r requirements.txt``.

    This installs all additional, non-stdlib modules, enabling use of things
    like ``boto.cloudsearch``, ``boto.manage`` & ``boto.mashups``, as well as
    covering everything needed for the test suite.


Using Virtual Environments
--------------------------

Another common way to install ``boto`` is to use a ``virtualenv``, which
provides isolated environments. First, install the ``virtualenv`` Python
package::

    pip install virtualenv

Next, create a virtual environment by using the ``virtualenv`` command and
specifying where you want the virtualenv to be created (you can specify
any directory you like, though this example allows for compatibility with
``virtualenvwrapper``)::

    mkdir ~/.virtualenvs
    virtualenv ~/.virtualenvs/boto

You can now activate the virtual environment::

    source ~/.virtualenvs/boto/bin/activate

Now, any usage of ``python`` or ``pip`` (within the current shell) will default
to the new, isolated version within your virtualenv.

You can now install ``boto`` into this virtual environment::

    pip install boto

When you are done using ``boto``, you can deactivate your virtual environment::

    deactivate

If you are creating a lot of virtual environments, `virtualenvwrapper`_
is an excellent tool that lets you easily manage your virtual environments.

.. _`virtualenvwrapper`: http://virtualenvwrapper.readthedocs.org/en/latest/


Configuring Boto Credentials
----------------------------

You have a few options for configuring ``boto`` (see :doc:`boto_config_tut`).
For this tutorial, we'll be using a configuration file. First, create a
``~/.boto`` file with these contents::

    [Credentials]
    aws_access_key_id = YOURACCESSKEY
    aws_secret_access_key = YOURSECRETKEY

``boto`` supports a number of configuration values. For more information,
see :doc:`boto_config_tut`. The above file, however, is all we need for now.
You're now ready to use ``boto``.


Making Connections
------------------

``boto`` provides a number of convenience functions to simplify connecting to a
service. For example, to work with S3, you can run::

    >>> import boto
    >>> s3 = boto.connect_s3()

If you want to connect to a different region, you can import the service module
and use the ``connect_to_region`` functions. For example, to create an EC2
client in 'us-west-2' region, you'd run the following::

    >>> import boto.ec2
    >>> ec2 = boto.ec2.connect_to_region('us-west-2')


Troubleshooting Connections
---------------------------

When calling the various ``connect_*`` functions, you might run into an error
like this::

    >>> import boto
    >>> s3 = boto.connect_s3()
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "boto/__init__.py", line 121, in connect_s3
        return S3Connection(aws_access_key_id, aws_secret_access_key, **kwargs)
      File "boto/s3/connection.py", line 171, in __init__
        validate_certs=validate_certs)
      File "boto/connection.py", line 548, in __init__
        host, config, self.provider, self._required_auth_capability())
      File "boto/auth.py", line 668, in get_auth_handler
        'Check your credentials' % (len(names), str(names)))
    boto.exception.NoAuthHandlerFound: No handler was ready to authenticate. 1 handlers were checked. ['HmacAuthV1Handler'] Check your credentials

This is because ``boto`` cannot find credentials to use. Verify that you have
created a ``~/.boto`` file as shown above. You can also turn on debug logging
to verify where your credentials are coming from::

    >>> import boto
    >>> boto.set_stream_logger('boto')
    >>> s3 = boto.connect_s3()
    2012-12-10 17:15:03,799 boto [DEBUG]:Using access key found in config file.
    2012-12-10 17:15:03,799 boto [DEBUG]:Using secret key found in config file.


Interacting with AWS Services
-----------------------------

Once you have a client for the specific service you want, there are methods on
that object that will invoke API operations for that service. The following
code demonstrates how to create a bucket and put an object in that bucket::

    >>> import boto
    >>> import time
    >>> s3 = boto.connect_s3()

    # Create a new bucket. Buckets must have a globally unique name (not just
    # unique to your account).
    >>> bucket = s3.create_bucket('boto-demo-%s' % int(time.time()))

    # Create a new key/value pair.
    >>> key = bucket.new_key('mykey')
    >>> key.set_contents_from_string("Hello World!")

    # Sleep to ensure the data is eventually there.
    >>> time.sleep(2)

    # Retrieve the contents of ``mykey``.
    >>> print key.get_contents_as_string()
    'Hello World!'

    # Delete the key.
    >>> key.delete()
    # Delete the bucket.
    >>> bucket.delete()

Each service supports a different set of commands. You'll want to refer to the
other guides & API references in this documentation, as well as referring to
the `official AWS API`_ documentation.

.. _`official AWS API`: https://aws.amazon.com/documentation/

Next Steps
----------

For many of the services that ``boto`` supports, there are tutorials as
well as detailed API documentation. If you are interested in a specific
service, the tutorial for the service is a good starting point. For instance,
if you'd like more information on S3, check out the :ref:`S3 Tutorial <s3_tut>`
and the :doc:`S3 API reference <ref/s3>`.
