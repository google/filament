.. _elb_tut:

==========================================================
An Introduction to boto's Elastic Load Balancing interface
==========================================================

This tutorial focuses on the boto interface for `Elastic Load Balancing`_
from Amazon Web Services. This tutorial assumes that you have already
downloaded and installed boto, and are familiar with the boto ec2 interface.

.. _Elastic Load Balancing: http://aws.amazon.com/elasticloadbalancing/

Elastic Load Balancing Concepts
-------------------------------
`Elastic Load Balancing`_ (ELB) is intimately connected with Amazon's `Elastic
Compute Cloud`_ (EC2) service. Using the ELB service allows you to create a load
balancer - a DNS endpoint and set of ports that distributes incoming requests
to a set of EC2 instances. The advantages of using a load balancer is that it
allows you to truly scale up or down a set of backend instances without
disrupting service. Before the ELB service, you had to do this manually by
launching an EC2 instance and installing load balancer software on it (nginx,
haproxy, perlbal, etc.) to distribute traffic to other EC2 instances.

Recall that the EC2 service is split into Regions, which are further
divided into Availability Zones (AZ).
For example, the US-East region is divided into us-east-1a, us-east-1b,
us-east-1c, us-east-1d, and us-east-1e. You can think of AZs as data centers -
each runs off a different set of ISP backbones and power providers.
ELB load balancers can span multiple AZs but cannot span multiple regions.
That means that if you'd like to create a set of instances spanning both the
US and Europe Regions you'd have to create two load balancers and have some
sort of other means of distributing requests between the two load balancers.
An example of this could be using GeoIP techniques to choose the correct load
balancer, or perhaps DNS round robin. Keep in mind also that traffic is
distributed equally over all AZs the ELB balancer spans. This means you should
have an equal number of instances in each AZ if you want to equally distribute
load amongst all your instances.

.. _Elastic Compute Cloud: http://aws.amazon.com/ec2/

Creating a Connection
---------------------

The first step in accessing ELB is to create a connection to the service.


Like EC2, the ELB service has a different endpoint for each region. By default
the US East endpoint is used. To choose a specific region, use the
``connect_to_region`` function::

    >>> import boto.ec2.elb
    >>> elb = boto.ec2.elb.connect_to_region('us-west-2')

Here's yet another way to discover what regions are available and then
connect to one::

    >>> import boto.ec2.elb
    >>> regions = boto.ec2.elb.regions()
    >>> regions
    [RegionInfo:us-east-1,
     RegionInfo:ap-northeast-1,
     RegionInfo:us-west-1,
     RegionInfo:us-west-2,
     RegionInfo:ap-southeast-1,
     RegionInfo:eu-west-1]
    >>> elb = regions[-1].connect()

Alternatively, edit your boto.cfg with the default ELB endpoint to use::

    [Boto]
    elb_region_name = eu-west-1
    elb_region_endpoint = elasticloadbalancing.eu-west-1.amazonaws.com

Getting Existing Load Balancers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To retrieve any existing load balancers:

>>> conn.get_all_load_balancers()
[LoadBalancer:load-balancer-prod, LoadBalancer:load-balancer-staging]

You can also filter by name

>>> conn.get_all_load_balancers(load_balancer_names=['load-balancer-prod'])
[LoadBalancer:load-balancer-prod]

:py:meth:`get_all_load_balancers <boto.ec2.elb.ELBConnection.get_all_load_balancers>`
returns a :py:class:`boto.resultset.ResultSet` that contains instances
of :class:`boto.ec2.elb.loadbalancer.LoadBalancer`, each of which abstracts
access to a load balancer. :py:class:`ResultSet <boto.resultset.ResultSet>`
works very much like a list.

>>> balancers = conn.get_all_load_balancers()
>>> balancers[0]
LoadBalancer:load-balancer-prod

Creating a Load Balancer
------------------------
To create a load balancer you need the following:
 #. The specific **ports and protocols** you want to load balancer over, and what port
    you want to connect to all instances.
 #. A **health check** - the ELB concept of a *heart beat* or *ping*. ELB will use this health
    check to see whether your instances are up or down. If they go down, the load balancer
    will no longer send requests to them.
 #. A **list of Availability Zones** you'd like to create your load balancer over.

Ports and Protocols
^^^^^^^^^^^^^^^^^^^
An incoming connection to your load balancer will come on one or more ports -
for example 80 (HTTP) and 443 (HTTPS). Each can be using a protocol -
currently, the supported protocols are TCP and HTTP.  We also need to tell the
load balancer which port to route connects *to* on each instance.  For example,
to create a load balancer for a website that accepts connections on 80 and 443,
and that routes connections to port 8080 and 8443 on each instance, you would
specify that the load balancer ports and protocols are:

 * 80, 8080, HTTP
 * 443, 8443, TCP

This says that the load balancer will listen on two ports - 80 and 443.
Connections on 80 will use an HTTP load balancer to forward connections to port
8080 on instances. Likewise, the load balancer will listen on 443 to forward
connections to 8443 on each instance using the TCP balancer. We need to
use TCP for the HTTPS port because it is encrypted at the application
layer. Of course, we could specify the load balancer use TCP for port 80,
however specifying HTTP allows you to let ELB handle some work for you -
for example HTTP header parsing.

.. _elb-configuring-a-health-check:

Configuring a Health Check
^^^^^^^^^^^^^^^^^^^^^^^^^^
A health check allows ELB to determine which instances are alive and able to
respond to requests. A health check is essentially a tuple consisting of:

 * *Target*: What to check on an instance. For a TCP check this is comprised of::

        TCP:PORT_TO_CHECK

   Which attempts to open a connection on PORT_TO_CHECK. If the connection opens
   successfully, that specific instance is deemed healthy, otherwise it is marked
   temporarily as unhealthy. For HTTP, the situation is slightly different::

        HTTP:PORT_TO_CHECK/RESOURCE

   This means that the health check will connect to the resource /RESOURCE on
   PORT_TO_CHECK. If an HTTP 200 status is returned the instance is deemed healthy.
 * *Interval*: How often the check is made. This is given in seconds and defaults
   to 30. The valid range of intervals goes from 5 seconds to 600 seconds.
 * *Timeout*: The number of seconds the load balancer will wait for a check to
   return a result.
 * *Unhealthy threshold*: The number of consecutive failed checks to deem the
   instance as being dead. The default is 5, and the range of valid values lies
   from 2 to 10.

The following example creates a health check called *instance_health* that
simply checks instances every 20 seconds on port 80 over HTTP at the
resource /health for 200 successes.

>>> from boto.ec2.elb import HealthCheck
>>> hc = HealthCheck(
        interval=20,
        healthy_threshold=3,
        unhealthy_threshold=5,
        target='HTTP:8080/health'
    )

Putting It All Together
^^^^^^^^^^^^^^^^^^^^^^^

Finally, let's create a load balancer in the US region that listens on ports
80 and 443 and distributes requests to instances on 8080 and 8443 over HTTP
and TCP. We want the load balancer to span the availability zones
*us-east-1a* and *us-east-1b*:

>>> zones = ['us-east-1a', 'us-east-1b']
>>> ports = [(80, 8080, 'http'), (443, 8443, 'tcp')]
>>> lb = conn.create_load_balancer('my-lb', zones, ports)
>>> # This is from the previous section.
>>> lb.configure_health_check(hc)

The load balancer has been created. To see where you can actually connect to
it, do:

>>> print lb.dns_name
my_elb-123456789.us-east-1.elb.amazonaws.com

You can then CNAME map a better name, i.e. www.MYWEBSITE.com to the
above address.

Adding Instances To a Load Balancer
-----------------------------------

Now that the load balancer has been created, there are two ways to add
instances to it:

 #. Manually, adding each instance in turn.
 #. Mapping an autoscale group to the load balancer. Please see the
    :doc:`Autoscale tutorial <autoscale_tut>` for information on how to do this.

Manually Adding and Removing Instances
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Assuming you have a list of instance ids, you can add them to the load balancer

>>> instance_ids = ['i-4f8cf126', 'i-0bb7ca62']
>>> lb.register_instances(instance_ids)

Keep in mind that these instances should be in Security Groups that match the
internal ports of the load balancer you just created (for this example, they
should allow incoming connections on 8080 and 8443).

To remove instances:

>>> lb.deregister_instances(instance_ids)

Modifying Availability Zones for a Load Balancer
------------------------------------------------

If you wanted to disable one or more zones from an existing load balancer:

>>> lb.disable_zones(['us-east-1a'])

You can then terminate each instance in the disabled zone and then deregister then from your load
balancer.

To enable zones:

>>> lb.enable_zones(['us-east-1c'])

Deleting a Load Balancer
------------------------

>>> lb.delete()


