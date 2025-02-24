.. _request_hook_tut.rst:

======================================
An Introduction to boto's request hook
======================================

This tutorial shows you how to use the request hook for data gathering.

It is often important to measure things we do as developers to better
understand application performance and the interactions between components
of the system. Boto plays a key role in some of those interactions as any
client library would.

We'll go over how to use the request hook to do some simple request logging.

Creating a connection
---------------------

For this example, let's use the EC2 interface as an example. Any connection
will work (IAM, SQS, etc..)::

    >>> from boto import ec2
    >>> conn = ec2.connect_to_region('us-west-2')

You will be using this conn object for the remainder of the tutorial to send
commands to EC2.

Adding your own hook
--------------------

The hook interface is defined in boto.utils.RequestHook
The method signature looks like::

    def handle_request_data(self, request, response, error=False):

In boto.requestlog.py, there is an implementation of this interface which
is written to handle multiple threads sending data to a single log
writing thread. Exammining this file, you'll see a log file, queue and thread
are created, then as requests are made, the handle_request_data() method is
called. It extracts data from the request and respose object to create a log
message. That's inserted into the queue and handled by the _request_log_worker
thread.

One thing to note is that the boto request object has an additional value
"start_time", which is a datetime.now() as of the time right before the
request was issued. This can be used along with the current time (after the
request) to calculate the duration of the request.

To add this logger to your connection::

    >>> from boto.requestlog import RequestLogger
    >>> conn.set_request_hook(RequestLogger())

That's all you need to do! Now, if you make a request, like::

    >>> conn.get_all_volumes()

The log message produced might look something like this::

    '2014-02-26 21:38:27', '200', '0.791542', '592', 'DescribeVolumes'

