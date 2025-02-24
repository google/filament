.. _ref-boto_config:

===========
Boto Config
===========

Introduction
------------

There is a growing list of configuration options for the boto library. Many of
these options can be passed into the constructors for top-level objects such as
connections. Some options, such as credentials, can also be read from
environment variables (e.g. ``AWS_ACCESS_KEY_ID``, ``AWS_SECRET_ACCESS_KEY``,
``AWS_SECURITY_TOKEN`` and ``AWS_PROFILE``). It is also possible to manage
these options in a central place through the use of boto config files.

Details
-------

A boto config file is a text file formatted like an .ini configuration file that specifies
values for options that control the behavior of the boto library. In Unix/Linux systems,
on startup, the boto library looks for configuration files in the following locations
and in the following order:

* /etc/boto.cfg - for site-wide settings that all users on this machine will use
* (if profile is given) ~/.aws/credentials - for credentials shared between SDKs
* (if profile is given) ~/.boto - for user-specific settings
* ~/.aws/credentials - for credentials shared between SDKs
* ~/.boto - for user-specific settings

**Comments** 
You can comment out a line by putting a '#' at the beginning of the line, just like in Python code.


In Windows, create a text file that has any name (e.g. boto.config). It's
recommended that you put this file in your user folder. Then set 
a user environment variable named BOTO_CONFIG to the full path of that file.

The options in the config file are merged into a single, in-memory configuration 
that is available as :py:mod:`boto.config`. The :py:class:`boto.pyami.config.Config`
class is a subclass of the standard Python
:py:class:`ConfigParser.SafeConfigParser` object and inherits all of the
methods of that object. In addition, the boto
:py:class:`Config <boto.pyami.config.Config>` class defines additional
methods that are described on the PyamiConfigMethods page.

An example boto config file might look like::

    [Credentials]
    aws_access_key_id = <your_access_key_here>
    aws_secret_access_key = <your_secret_key_here>


Sections
--------

The following sections and options are currently recognized within the
boto config file.

Credentials
^^^^^^^^^^^

The Credentials section is used to specify the AWS credentials used for all
boto requests. The order of precedence for authentication credentials is:

* Credentials passed into the Connection class constructor.
* Credentials specified by environment variables
* Credentials specified as named profiles in the shared credential file.
* Credentials specified by default in the shared credential file.
* Credentials specified as named profiles in the config file.
* Credentials specified by default in the config file.

This section defines the following options: ``aws_access_key_id`` and
``aws_secret_access_key``. The former being your AWS key id and the latter
being the secret key.

For example::

    [profile name_goes_here]
    aws_access_key_id = <access key for this profile>
    aws_secret_access_key = <secret key for this profile>

    [Credentials]
    aws_access_key_id = <your default access key>
    aws_secret_access_key = <your default secret key>

Please notice that quote characters are not used to either side of the '='
operator even when both your AWS access key ID and secret key are strings.

If you have multiple AWS keypairs that you use for different purposes,
use the ``profile`` style shown above. You can set an arbitrary number
of profiles within your configuration files and then reference them by name
when you instantiate your connection. If you specify a profile that does not
exist in the configuration, the keys used under the ``[Credentials]`` heading
will be applied by default.

The shared credentials file in ``~/.aws/credentials`` uses a slightly
different format. For example::

    [default]
    aws_access_key_id = <your default access key>
    aws_secret_access_key = <your default secret key>

    [name_goes_here]
    aws_access_key_id = <access key for this profile>
    aws_secret_access_key = <secret key for this profile>

    [another_profile]
    aws_access_key_id = <access key for this profile>
    aws_secret_access_key = <secret key for this profile>
    aws_security_token = <optional security token for this profile>

For greater security, the secret key can be stored in a keyring and
retrieved via the keyring package.  To use a keyring, use ``keyring``,
rather than ``aws_secret_access_key``::

    [Credentials]
    aws_access_key_id = <your access key>
    keyring = <keyring name>

To use a keyring, you must have the Python `keyring
<http://pypi.python.org/pypi/keyring>`_ package installed and in the
Python path. To learn about setting up keyrings, see the `keyring
documentation
<http://pypi.python.org/pypi/keyring#installing-and-using-python-keyring-lib>`_

Credentials can also be supplied for a Eucalyptus service::

    [Credentials]
    euca_access_key_id = <your access key>
    euca_secret_access_key = <your secret key>

Finally, this section is also be used to provide credentials for the Internet Archive API::

    [Credentials]
    ia_access_key_id = <your access key>
    ia_secret_access_key = <your secret key>

Boto
^^^^

The Boto section is used to specify options that control the operation of
boto itself. This section defines the following options:

:debug: Controls the level of debug messages that will be printed by the boto library.
    The following values are defined::

        0 - no debug messages are printed
        1 - basic debug messages from boto are printed
        2 - all boto debugging messages plus request/response messages from httplib

:proxy: The name of the proxy host to use for connecting to AWS.
:proxy_port: The port number to use to connect to the proxy host.
:proxy_user: The user name to use when authenticating with proxy host.
:proxy_pass: The password to use when authenticating with proxy host.
:num_retries: The number of times to retry failed requests to an AWS server.
  If boto receives an error from AWS, it will attempt to recover and retry the
  request. The default number of retries is 5 but you can change the default
  with this option.

For example::

    [Boto]
    debug = 0
    num_retries = 10

    proxy = myproxy.com
    proxy_port = 8080
    proxy_user = foo
    proxy_pass = bar


:connection_stale_duration: Amount of time to wait in seconds before a
  connection will stop getting reused. AWS will disconnect connections which
  have been idle for 180 seconds.
:is_secure: Is the connection over SSL. This setting will override passed in
  values.
:https_validate_certificates: Validate HTTPS certificates. This is on by default
:ca_certificates_file: Location of CA certificates or the keyword "system".
  Using the system keyword lets boto get out of the way and makes the
  SSL certificate validation the responsibility the underlying SSL
  implementation provided by the system.
:http_socket_timeout: Timeout used to overwrite the system default socket
  timeout for httplib .
:send_crlf_after_proxy_auth_headers: Change line ending behaviour with proxies.
  For more details see this `discussion <https://groups.google.com/forum/?fromgroups=#!topic/boto-dev/teenFvOq2Cc>`_
:endpoints_path: Allows customizing the regions/endpoints available in Boto.
  Provide an absolute path to a custom JSON file, which gets merged into the
  defaults. (This can also be specified with the ``BOTO_ENDPOINTS``
  environment variable instead.)
:use_endpoint_heuristics: Allows using endpoint heuristics to guess
  endpoints for regions that aren't built in. This can also be specified with
  the ``BOTO_USE_ENDPOINT_HEURISTICS`` environment variable.

These settings will default to::

    [Boto]
    connection_stale_duration = 180
    is_secure = True
    https_validate_certificates = True
    ca_certificates_file = cacerts.txt
    http_socket_timeout = 60
    send_crlf_after_proxy_auth_headers = False
    endpoints_path = /path/to/my/boto/endpoints.json
    use_endpoint_heuristics = False

You can control the timeouts and number of retries used when retrieving
information from the Metadata Service (this is used for retrieving credentials
for IAM roles on EC2 instances):

:metadata_service_timeout: Number of seconds until requests to the metadata
  service will timeout (float).
:metadata_service_num_attempts: Number of times to attempt to retrieve
  information from the metadata service before giving up (int).

These settings will default to::

    [Boto]
    metadata_service_timeout = 1.0
    metadata_service_num_attempts = 1


This section is also used for specifying endpoints for non-AWS services such as
Eucalyptus and Walrus.

:eucalyptus_host: Select a default endpoint host for eucalyptus
:walrus_host: Select a default host for Walrus

For example::

    [Boto]
    eucalyptus_host = somehost.example.com
    walrus_host = somehost.example.com


Finally, the Boto section is used to set defaults versions for many AWS services

AutoScale settings:

options:
:autoscale_version: Set the API version
:autoscale_endpoint: Endpoint to use
:autoscale_region_name: Default region to use

For example::

    [Boto]
    autoscale_version = 2011-01-01
    autoscale_endpoint = autoscaling.us-west-2.amazonaws.com
    autoscale_region_name = us-west-2


Cloudformation settings can also be defined:

:cfn_version: Cloud formation API version
:cfn_region_name: Default region name
:cfn_region_endpoint: Default endpoint

For example::

    [Boto]
    cfn_version = 2010-05-15
    cfn_region_name = us-west-2
    cfn_region_endpoint = cloudformation.us-west-2.amazonaws.com

Cloudsearch settings:

:cs_region_name: Default cloudsearch region
:cs_region_endpoint: Default cloudsearch endpoint

For example::

    [Boto]
    cs_region_name = us-west-2
    cs_region_endpoint = cloudsearch.us-west-2.amazonaws.com

Cloudwatch settings:

:cloudwatch_version: Cloudwatch API version
:cloudwatch_region_name: Default region name
:cloudwatch_region_endpoint: Default endpoint

For example::

    [Boto]
    cloudwatch_version = 2010-08-01
    cloudwatch_region_name = us-west-2
    cloudwatch_region_endpoint = monitoring.us-west-2.amazonaws.com

EC2 settings:

:ec2_version: EC2 API version
:ec2_region_name: Default region name
:ec2_region_endpoint: Default endpoint

For example::

    [Boto]
    ec2_version = 2012-12-01
    ec2_region_name = us-west-2
    ec2_region_endpoint = ec2.us-west-2.amazonaws.com

ELB settings:

:elb_version: ELB API version
:elb_region_name: Default region name
:elb_region_endpoint: Default endpoint

For example::

    [Boto]
    elb_version = 2012-06-01
    elb_region_name = us-west-2
    elb_region_endpoint = elasticloadbalancing.us-west-2.amazonaws.com

EMR settings:

:emr_version: EMR API version
:emr_region_name: Default region name
:emr_region_endpoint: Default endpoint

For example::

    [Boto]
    emr_version = 2009-03-31
    emr_region_name = us-west-2
    emr_region_endpoint = elasticmapreduce.us-west-2.amazonaws.com


Precedence
----------

Even if you have your boto config setup, you can also have credentials and
options stored in environmental variables or you can explicitly pass them to
method calls i.e.::

    >>> boto.ec2.connect_to_region(
    ...     'us-west-2',
    ...     aws_access_key_id='foo',
    ...     aws_secret_access_key='bar')

In these cases where these options can be found in more than one place boto
will first use the explicitly supplied arguments, if none found it will then
look for them amidst environment variables and if that fails it will use the
ones in boto config.

Notification
^^^^^^^^^^^^

If you are using notifications for boto.pyami, you can specify the email
details through the following variables.

:smtp_from: Used as the sender in notification emails.
:smtp_to: Destination to which emails should be sent
:smtp_host: Host to connect to when sending notification emails.
:smtp_port: Port to connect to when connecting to the :smtp_host:

Default values are::

    [notification]
    smtp_from = boto
    smtp_to = None
    smtp_host = localhost
    smtp_port = 25
    smtp_tls = True
    smtp_user = john
    smtp_pass = hunter2

SWF
^^^

The SWF section allows you to configure the default region to be used for the
Amazon Simple Workflow service.

:region: Set the default region

Example::

    [SWF]
    region = us-west-2

Pyami
^^^^^

The Pyami section is used to configure the working directory for PyAMI.

:working_dir: Working directory used by PyAMI

Example::

    [Pyami]
    working_dir = /home/foo/

DB
^^
The DB section is used to configure access to databases through the
:func:`boto.sdb.db.manager.get_manager` function.

:db_type: Type of the database. Current allowed values are `SimpleDB` and
    `XML`.
:db_user: AWS access key id.
:db_passwd: AWS secret access key.
:db_name: Database that will be connected to.
:db_table: Table name :note: This doesn't appear to be used.
:db_host: Host to connect to
:db_port: Port to connect to
:enable_ssl: Use SSL

More examples::

    [DB]
    db_type = SimpleDB
    db_user = <aws access key id>
    db_passwd = <aws secret access key>
    db_name = my_domain
    db_table = table
    db_host = sdb.amazonaws.com
    enable_ssl = True
    debug = True

    [DB_TestBasic]
    db_type = SimpleDB
    db_user = <another aws access key id>
    db_passwd = <another aws secret access key>
    db_name = basic_domain
    db_port = 1111

SDB
^^^

This section is used to configure SimpleDB

:region: Set the region to which SDB should connect

Example::

    [SDB]
    region = us-west-2

DynamoDB
^^^^^^^^

This section is used to configure DynamoDB

:region: Choose the default region
:validate_checksums: Check checksums returned by DynamoDB

Example::

    [DynamoDB]
    region = us-west-2
    validate_checksums = True
