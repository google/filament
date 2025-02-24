.. cloudwatch_tut:

==========
CloudWatch
==========

First, make sure you have something to monitor.  You can either create a
LoadBalancer or enable monitoring on an existing EC2 instance.  To enable
monitoring, you can either call the monitor_instance method on the
EC2Connection object or call the monitor method on the Instance object.

It takes a while for the monitoring data to start accumulating but once
it does, you can do this::

    >>> import boto.ec2.cloudwatch
    >>> c = boto.ec2.cloudwatch.connect_to_region('us-west-2')
    >>> metrics = c.list_metrics()
    >>> metrics
    [Metric:DiskReadBytes,
     Metric:CPUUtilization,
     Metric:DiskWriteOps,
     Metric:DiskWriteOps,
     Metric:DiskReadOps,
     Metric:DiskReadBytes,
     Metric:DiskReadOps, 
     Metric:CPUUtilization, 
     Metric:DiskWriteOps, 
     Metric:NetworkIn, 
     Metric:NetworkOut, 
     Metric:NetworkIn, 
     Metric:DiskReadBytes, 
     Metric:DiskWriteBytes, 
     Metric:DiskWriteBytes, 
     Metric:NetworkIn, 
     Metric:NetworkIn, 
     Metric:NetworkOut, 
     Metric:NetworkOut, 
     Metric:DiskReadOps, 
     Metric:CPUUtilization, 
     Metric:DiskReadOps, 
     Metric:CPUUtilization, 
     Metric:DiskWriteBytes, 
     Metric:DiskWriteBytes, 
     Metric:DiskReadBytes, 
     Metric:NetworkOut, 
     Metric:DiskWriteOps]


The list_metrics call will return a list of all of the available metrics
that you can query against.  Each entry in the list is a Metric object.
As you can see from the list above, some of the metrics are repeated. The repeated metrics are across different dimensions (per-instance, per-image type, per instance type) which can identified by looking at the dimensions property.

Because for this example, I'm only monitoring a single instance, the set
of metrics available to me are fairly limited.  If I was monitoring many
instances, using many different instance types and AMI's and also several
load balancers, the list of available metrics would grow considerably.

Once you have the list of available metrics, you can actually
query the CloudWatch system for that metric.  
Let's choose the CPU utilization metric for one of the ImageID.::
    >>> m_image = metrics[7]
    >>> m_image
    Metric:CPUUtilization
    >>> m_image.dimensions
    {u'ImageId': [u'ami-6ac2a85a']}

Let's choose another CPU utilization metric for our instance.::

    >>> m = metrics[20]
    >>> m
    Metric:CPUUtilization
    >>> m.dimensions
    {u'InstanceId': [u'i-4ca81747']}

The Metric object has a query method that lets us actually perform
the query against the collected data in CloudWatch.  To call that,
we need a start time and end time to control the time span of data
that we are interested in.  For this example, let's say we want the
data for the previous hour::

    >>> import datetime
    >>> end = datetime.datetime.utcnow()
    >>> start = end - datetime.timedelta(hours=1)

We also need to supply the Statistic that we want reported and
the Units to use for the results.  The Statistic can be one of these
values::

    ['Minimum', 'Maximum', 'Sum', 'Average', 'SampleCount']

And Units must be one of the following::

    ['Seconds', 'Microseconds', 'Milliseconds', 'Bytes', 'Kilobytes', 'Megabytes', 'Gigabytes', 'Terabytes', 'Bits', 'Kilobits', 'Megabits', 'Gigabits', 'Terabits', 'Percent', 'Count', 'Bytes/Second', 'Kilobytes/Second', 'Megabytes/Second', 'Gigabytes/Second', 'Terabytes/Second', 'Bits/Second', 'Kilobits/Second', 'Megabits/Second', 'Gigabits/Second', 'Terabits/Second', 'Count/Second', None]

The query method also takes an optional parameter, period.  This
parameter controls the granularity (in seconds) of the data returned.
The smallest period is 60 seconds and the value must be a multiple
of 60 seconds.  So, let's ask for the average as a percent::

    >>> datapoints = m.query(start, end, 'Average', 'Percent')
    >>> len(datapoints)
    60

Our period was 60 seconds and our duration was one hour so
we should get 60 data points back and we can see that we did.
Each element in the datapoints list is a DataPoint object
which is a simple subclass of a Python dict object.  Each
Datapoint object contains all of the information available
about that particular data point.::

    >>> d = datapoints[0]
    >>> d
    {u'Timestamp': datetime.datetime(2014, 6, 23, 22, 25),
     u'Average': 20.0, 
     u'Unit': u'Percent'}

My server obviously isn't very busy right now!
