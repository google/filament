.. _autoscale_tut:

=============================================
An Introduction to boto's Autoscale interface
=============================================

This tutorial focuses on the boto interface to the Autoscale service. This
assumes you are familiar with boto's EC2 interface and concepts.

Autoscale Concepts
------------------

The AWS Autoscale service is comprised of three core concepts:

 #. *Autoscale Group (AG):* An AG can be viewed as a collection of criteria for
    maintaining or scaling a set of EC2 instances over one or more availability
    zones. An AG is limited to a single region.
 #. *Launch Configuration (LC):* An LC is the set of information needed by the
    AG to launch new instances - this can encompass image ids, startup data,
    security groups and keys. Only one LC is attached to an AG.
 #. *Triggers*: A trigger is essentially a set of rules for determining when to
    scale an AG up or down. These rules can encompass a set of metrics such as
    average CPU usage across instances, or incoming requests, a threshold for
    when an action will take place, as well as parameters to control how long
    to wait after a threshold is crossed.

Creating a Connection
---------------------
The first step in accessing autoscaling is to create a connection to the service.
There are two ways to do this in boto.  The first is:

>>> from boto.ec2.autoscale import AutoScaleConnection
>>> conn = AutoScaleConnection('<aws access key>', '<aws secret key>')


A Note About Regions and Endpoints
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Like EC2 the Autoscale service has a different endpoint for each region. By
default the US endpoint is used. To choose a specific region, instantiate the
AutoScaleConnection object with that region's endpoint.

>>> import boto.ec2.autoscale
>>> autoscale = boto.ec2.autoscale.connect_to_region('eu-west-1')

Alternatively, edit your boto.cfg with the default Autoscale endpoint to use::

    [Boto]
    autoscale_endpoint = autoscaling.eu-west-1.amazonaws.com

Getting Existing AutoScale Groups
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To retrieve existing autoscale groups:

>>> conn.get_all_groups()

You will get back a list of AutoScale group objects, one for each AG you have.

Creating Autoscaling Groups
---------------------------
An Autoscaling group has a number of parameters associated with it.

 #. *Name*: The name of the AG.
 #. *Availability Zones*: The list of availability zones it is defined over.
 #. *Minimum Size*: Minimum number of instances running at one time.
 #. *Maximum Size*: Maximum number of instances running at one time.
 #. *Launch Configuration (LC)*: A set of instructions on how to launch an instance.
 #. *Load Balancer*: An optional ELB load balancer to use. See the ELB tutorial
    for information on how to create a load balancer.

For the purposes of this tutorial, let's assume we want to create one autoscale
group over the us-east-1a and us-east-1b availability zones. We want to have
two instances in each availability zone, thus a minimum size of 4. For now we
won't worry about scaling up or down - we'll introduce that later when we talk
about triggers. Thus we'll set a maximum size of 4 as well. We'll also associate
the AG with a load balancer which we assume we've already created, called 'my_lb'.

Our LC tells us how to start an instance. This will at least include the image
id to use, security_group, and key information. We assume the image id, key
name and security groups have already been defined elsewhere - see the EC2
tutorial for information on how to create these.

>>> from boto.ec2.autoscale import LaunchConfiguration
>>> from boto.ec2.autoscale import AutoScalingGroup
>>> lc = LaunchConfiguration(name='my-launch_config', image_id='my-ami',
                             key_name='my_key_name',
                             security_groups=['my_security_groups'])
>>> conn.create_launch_configuration(lc)

We now have created a launch configuration called 'my-launch-config'. We are now
ready to associate it with our new autoscale group.

>>> ag = AutoScalingGroup(group_name='my_group', load_balancers=['my-lb'],
                          availability_zones=['us-east-1a', 'us-east-1b'],
                          launch_config=lc, min_size=4, max_size=8,
                          connection=conn)
>>> conn.create_auto_scaling_group(ag)

We now have a new autoscaling group defined! At this point instances should be
starting to launch. To view activity on an autoscale group:

>>> ag.get_activities()
 [Activity:Launching a new EC2 instance status:Successful progress:100,
  ...]

or alternatively:

>>> conn.get_all_activities(ag)

This autoscale group is fairly useful in that it will maintain the minimum size without
breaching the maximum size defined. That means if one instance crashes, the autoscale
group will use the launch configuration to start a new one in an attempt to maintain
its minimum defined size. It knows instance health using the health check defined on
its associated load balancer.

Scaling a Group Up or Down
^^^^^^^^^^^^^^^^^^^^^^^^^^
It can also be useful to scale a group up or down depending on certain criteria.
For example, if the average CPU utilization of the group goes above 70%, you may
want to scale up the number of instances to deal with demand. Likewise, you
might want to scale down if usage drops again.
These rules for **how** to scale are defined by *Scaling Policies*, and the rules for
**when** to scale are defined by CloudWatch *Metric Alarms*.

For example, let's configure scaling for the above group based on CPU utilization.
We'll say it should scale up if the average CPU usage goes above 70% and scale
down if it goes below 40%.

Firstly, define some Scaling Policies. These tell Auto Scaling how to scale
the group (but not when to do it, we'll specify that later).

We need one policy for scaling up and one for scaling down.

>>> from boto.ec2.autoscale import ScalingPolicy
>>> scale_up_policy = ScalingPolicy(
            name='scale_up', adjustment_type='ChangeInCapacity',
            as_name='my_group', scaling_adjustment=1, cooldown=180)
>>> scale_down_policy = ScalingPolicy(
            name='scale_down', adjustment_type='ChangeInCapacity',
            as_name='my_group', scaling_adjustment=-1, cooldown=180)

The policy objects are now defined locally.
Let's submit them to AWS.

>>> conn.create_scaling_policy(scale_up_policy)
>>> conn.create_scaling_policy(scale_down_policy)

Now that the polices have been digested by AWS, they have extra properties
that we aren't aware of locally. We need to refresh them by requesting them
back again.

>>> scale_up_policy = conn.get_all_policies(
            as_group='my_group', policy_names=['scale_up'])[0]
>>> scale_down_policy = conn.get_all_policies(
            as_group='my_group', policy_names=['scale_down'])[0]

Specifically, we'll need the Amazon Resource Name (ARN) of each policy, which
will now be a property of our ScalingPolicy objects.

Next we'll create CloudWatch alarms that will define when to run the
Auto Scaling Policies.

>>> import boto.ec2.cloudwatch
>>> cloudwatch = boto.ec2.cloudwatch.connect_to_region('us-west-2')

It makes sense to measure the average CPU usage across the whole Auto Scaling
Group, rather than individual instances. We express that as CloudWatch
*Dimensions*.

>>> alarm_dimensions = {"AutoScalingGroupName": 'my_group'}

Create an alarm for when to scale up, and one for when to scale down.

>>> from boto.ec2.cloudwatch import MetricAlarm
>>> scale_up_alarm = MetricAlarm(
            name='scale_up_on_cpu', namespace='AWS/EC2',
            metric='CPUUtilization', statistic='Average',
            comparison='>', threshold='70',
            period='60', evaluation_periods=2,
            alarm_actions=[scale_up_policy.policy_arn],
            dimensions=alarm_dimensions)
>>> cloudwatch.create_alarm(scale_up_alarm)

>>> scale_down_alarm = MetricAlarm(
            name='scale_down_on_cpu', namespace='AWS/EC2',
            metric='CPUUtilization', statistic='Average',
            comparison='<', threshold='40',
            period='60', evaluation_periods=2,
            alarm_actions=[scale_down_policy.policy_arn],
            dimensions=alarm_dimensions)
>>> cloudwatch.create_alarm(scale_down_alarm)

Auto Scaling will now create a new instance if the existing cluster averages
more than 70% CPU for two minutes. Similarly, it will terminate an instance
when CPU usage sits below 40%. Auto Scaling will not add or remove instances
beyond the limits of the Scaling Group's 'max_size' and 'min_size' properties.

To retrieve the instances in your autoscale group:

>>> import boto.ec2
>>> ec2 = boto.ec2.connect_to_region('us-west-2)
>>> group = conn.get_all_groups(names=['my_group'])[0]
>>> instance_ids = [i.instance_id for i in group.instances]
>>> instances = ec2.get_only_instances(instance_ids)

To delete your autoscale group, we first need to shutdown all the
instances:

>>> ag.shutdown_instances()

Once the instances have been shutdown, you can delete the autoscale
group:

>>> ag.delete()

You can also delete your launch configuration:

>>> lc.delete()
