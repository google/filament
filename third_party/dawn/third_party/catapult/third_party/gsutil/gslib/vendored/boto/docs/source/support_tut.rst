.. _support_tut:

===========================================
An Introduction to boto's Support interface
===========================================

This tutorial focuses on the boto interface to Amazon Web Services Support,
allowing you to programmatically interact with cases created with Support.
This tutorial assumes that you have already downloaded and installed ``boto``.

Creating a Connection
---------------------

The first step in accessing Support is to create a connection
to the service.  There are two ways to do this in boto.  The first is:

>>> from boto.support.connection import SupportConnection
>>> conn = SupportConnection('<aws access key>', '<aws secret key>')

At this point the variable ``conn`` will point to a ``SupportConnection``
object. In this example, the AWS access key and AWS secret key are passed in to
the method explicitly. Alternatively, you can set the environment variables:

**AWS_ACCESS_KEY_ID**
    Your AWS Access Key ID

**AWS_SECRET_ACCESS_KEY**
    Your AWS Secret Access Key

and then call the constructor without any arguments, like this:

>>> conn = SupportConnection()

There is also a shortcut function in boto
that makes it easy to create Support connections:

>>> import boto.support
>>> conn = boto.support.connect_to_region('us-west-2')

In either case, ``conn`` points to a ``SupportConnection`` object which we will
use throughout the remainder of this tutorial.


Describing Existing Cases
-------------------------

If you have existing cases or want to fetch cases in the future, you'll
use the ``SupportConnection.describe_cases`` method. For example::

    >>> cases = conn.describe_cases()
    >>> len(cases['cases'])
    1
    >>> cases['cases'][0]['title']
    'A test case.'
    >>> cases['cases'][0]['caseId']
    'case-...'

You can also fetch a set of cases (or single case) by providing a
``case_id_list`` parameter::

    >>> cases = conn.describe_cases(case_id_list=['case-1'])
    >>> len(cases['cases'])
    1
    >>> cases['cases'][0]['title']
    'A test case.'
    >>> cases['cases'][0]['caseId']
    'case-...'


Describing Service Codes
------------------------

In order to create a new case, you'll need to fetch the service (& category)
codes available to you. Fetching them is a simple call to::

    >>> services = conn.describe_services()
    >>> services['services'][0]['code']
    'amazon-cloudsearch'

If you only care about certain services, you can pass a list of service codes::

    >>> service_details = conn.describe_services(service_code_list=[
    ...     'amazon-cloudsearch',
    ...     'amazon-dynamodb',
    ... ])


Describing Severity Levels
--------------------------

In order to create a new case, you'll also need to fetch the severity levels
available to you. Fetching them looks like::

    >>> severities = conn.describe_severity_levels()
    >>> severities['severityLevels'][0]['code']
    'low'


Creating a Case
---------------

Upon creating a connection to Support, you can now work with existing Support
cases, create new cases or resolve them. We'll start with creating a new case::

    >>> new_case = conn.create_case(
    ...     subject='This is a test case.',
    ...     service_code='',
    ...     category_code='',
    ...     communication_body="",
    ...     severity_code='low'
    ... )
    >>> new_case['caseId']
    'case-...'

For the ``service_code/category_code`` parameters, you'll need to do a
``SupportConnection.describe_services`` call, then select the appropriate
service code (& appropriate category code within that service) from the
response.

For the ``severity_code`` parameter, you'll need to do a
``SupportConnection.describe_severity_levels`` call, then select the appropriate
severity code from the response.


Adding to a Case
----------------

Since the purpose of a support case involves back-and-forth communication,
you can add additional communication to the case as well. Providing a response
might look like::

    >>> result = conn.add_communication_to_case(
    ...     communication_body="This is a followup. It's working now."
    ...     case_id='case-...'
    ... )


Fetching all Communications for a Case
--------------------------------------

Getting all communications for a given case looks like::

    >>> communications = conn.describe_communications('case-...')


Resolving a Case
----------------

Once a case is finished, you should mark it as resolved to close it out.
Resolving a case looks like::

    >>> closed = conn.resolve_case(case_id='case-...')
    >>> closed['result']
    True
