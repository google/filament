.. _ec2_tut:

=======================================
An Introduction to boto's EC2 interface
=======================================

This tutorial focuses on the boto interface to the Elastic Compute Cloud
from Amazon Web Services.  This tutorial assumes that you have already
downloaded and installed boto.

Creating a Connection
---------------------

The first step in accessing EC2 is to create a connection to the service.
The recommended way of doing this in boto is::

    >>> import boto.ec2
    >>> conn = boto.ec2.connect_to_region("us-west-2",
    ...    aws_access_key_id='<aws access key>',
    ...    aws_secret_access_key='<aws secret key>')

At this point the variable ``conn`` will point to an EC2Connection object.  In
this example, the AWS access key and AWS secret key are passed in to the method
explicitly.  Alternatively, you can set the boto config environment variables
and then simply specify which region you want as follows::

    >>> conn = boto.ec2.connect_to_region("us-west-2")

In either case, conn will point to an EC2Connection object which we will
use throughout the remainder of this tutorial.

Launching Instances
-------------------

Possibly, the most important and common task you'll use EC2 for is to launch,
stop and terminate instances. In its most primitive form, you can launch an
instance as follows::

    >>> conn.run_instances('<ami-image-id>')

This will launch an instance in the specified region with the default parameters.
You will not be able to SSH into this machine, as it doesn't have a security
group set. See :doc:`security_groups` for details on creating one.

Now, let's say that you already have a key pair, want a specific type of
instance, and you have your :doc:`security group <security_groups>` all setup.
In this case we can use the keyword arguments to accomplish that::

    >>> conn.run_instances(
            '<ami-image-id>',
            key_name='myKey',
            instance_type='c1.xlarge',
            security_groups=['your-security-group-here'])

The main caveat with the above call is that it is possible to request an
instance type that is not compatible with the provided AMI (for example, the
instance was created for a 64-bit instance and you choose a m1.small instance_type).
For more details on the plethora of possible keyword parameters, be sure to
check out boto's :doc:`EC2 API reference <ref/ec2>`.

Stopping Instances
------------------
Once you have your instances up and running, you might wish to shut them down
if they're not in use. Please note that this will only de-allocate virtual
hardware resources (as well as instance store drives), but won't destroy your
EBS volumes -- this means you'll pay nominal provisioned EBS storage fees
even if your instance is stopped. To do this, you can do so as follows::

    >>> conn.stop_instances(instance_ids=['instance-id-1','instance-id-2', ...])

This will request a 'graceful' stop of each of the specified instances. If you
wish to request the equivalent of unplugging your instance(s), simply add
``force=True`` keyword argument to the call above. Please note that stop
instance is not allowed with Spot instances.

Terminating Instances
---------------------
Once you are completely done with your instance and wish to surrender both
virtual hardware, root EBS volume and all other underlying components
you can request instance termination. To do so you can use the call bellow::

    >>> conn.terminate_instances(instance_ids=['instance-id-1','instance-id-2', ...])

Please use with care since once you request termination for an instance there
is no turning back.

Checking What Instances Are Running
-----------------------------------
You can also get information on your currently running instances::

    >>> reservations = conn.get_all_reservations()
    >>> reservations
    [Reservation:r-00000000]

A reservation corresponds to a command to start instances. You can see what
instances are associated with a reservation::

    >>> instances = reservations[0].instances
    >>> instances
    [Instance:i-00000000]

An instance object allows you get more meta-data available about the instance::

    >>> inst = instances[0]
    >>> inst.instance_type
    u'c1.xlarge'
    >>> inst.placement
    u'us-west-2'

In this case, we can see that our instance is a c1.xlarge instance in the
`us-west-2` availability zone.

Checking Health Status Of Instances
-----------------------------------
You can also get the health status of your instances, including any scheduled events::

    >>> statuses = conn.get_all_instance_status()
    >>> statuses
    [InstanceStatus:i-00000000]

An instance status object allows you to get information about impaired
functionality or scheduled / system maintenance events::

    >>> status = statuses[0]
    >>> status.events
    [Event:instance-reboot]
    >>> event = status.events[0]
    >>> event.description
    u'Maintenance software update.'
    >>> event.not_before
    u'2011-12-11T04:00:00.000Z'
    >>> event.not_after
    u'2011-12-11T10:00:00.000Z'
    >>> status.instance_status
    Status:ok
    >>> status.system_status
    Status:ok
    >>> status.system_status.details
    {u'reachability': u'passed'}

This will by default include the health status only for running instances.
If you wish to request the health status for all instances, simply add
``include_all_instances=True`` keyword argument to the call above.

=================================
Using Elastic Block Storage (EBS)
=================================


EBS Basics
----------

EBS can be used by EC2 instances for permanent storage. Note that EBS volumes
must be in the same availability zone as the EC2 instance you wish to attach it
to.

To actually create a volume you will need to specify a few details. The
following example will create a 50GB EBS in one of the `us-west-2` availability
zones::

   >>> vol = conn.create_volume(50, "us-west-2")
   >>> vol
   Volume:vol-00000000

You can check that the volume is now ready and available::

   >>> curr_vol = conn.get_all_volumes([vol.id])[0]
   >>> curr_vol.status
   u'available'
   >>> curr_vol.zone
   u'us-west-2'

We can now attach this volume to the EC2 instance we created earlier, making it
available as a new device::

   >>> conn.attach_volume (vol.id, inst.id, "/dev/sdx")
   u'attaching'

You will now have a new volume attached to your instance. Note that with some
Linux kernels, `/dev/sdx` may get translated to `/dev/xvdx`. This device can
now be used as a normal block device within Linux.

Working With Snapshots
----------------------

Snapshots allow you to make point-in-time snapshots of an EBS volume for future
recovery. Snapshots allow you to create incremental backups, and can also be
used to instantiate multiple new volumes. Snapshots can also be used to move
EBS volumes across availability zones or making backups to S3.

Creating a snapshot is easy::

   >>> snapshot = conn.create_snapshot(vol.id, 'My snapshot')
   >>> snapshot
   Snapshot:snap-00000000

Once you have a snapshot, you can create a new volume from it. Volumes are
created lazily from snapshots, which means you can start using such a volume
straight away::

   >>> new_vol = snapshot.create_volume('us-west-2')
   >>> conn.attach_volume (new_vol.id, inst.id, "/dev/sdy")
   u'attaching'

If you no longer need a snapshot, you can also easily delete it::

   >>> conn.delete_snapshot(snapshot.id)
   True


Working With Launch Configurations
----------------------------------

Launch Configurations allow you to create a re-usable set of properties for an
instance.  These are used with AutoScaling groups to produce consistent repeatable
instances sets.

Creating a Launch Configuration is easy:

   >>> conn = boto.connect_autoscale()
   >>> config = LaunchConfiguration(name='foo', image_id='ami-abcd1234', key_name='foo.pem')
   >>> conn.create_launch_configuration(config)

Once you have a launch configuration, you can list you current configurations:

   >>> conn = boto.connect_autoscale()
   >>> config = conn.get_all_launch_configurations(names=['foo'])

If you no longer need a launch configuration, you can delete it:

   >>> conn = boto.connect_autoscale()
   >>> conn.delete_launch_configuration('foo')

.. versionchanged:: 2.27.0
.. Note::

    If ``use_block_device_types=True`` is passed to the connection it will deserialize
    Launch Configurations with Block Device Mappings into a re-usable format with
    BlockDeviceType objects, similar to how AMIs are deserialized currently.  Legacy
    behavior is to put them into a format that is incompatible with creating new Launch
    Configurations. This switch is in place to preserve backwards compatability, but
    its usage is the preferred format going forward.

    If you would like to use the new format, you should use something like:

      >>> conn = boto.connect_autoscale(use_block_device_types=True)
      >>> config = conn.get_all_launch_configurations(names=['foo'])
