.. _rds_tut:

=======================================
An Introduction to boto's RDS interface
=======================================

This tutorial focuses on the boto interface to the Relational Database Service
from Amazon Web Services.  This tutorial assumes that you have boto already
downloaded and installed, and that you wish to setup a MySQL instance in RDS.

.. warning::

    This tutorial covers the **ORIGINAL** module for RDS.
    It has since been supplanted by a second major version & an
    updated API complete with all service operations. The documentation for the
    new version of boto's support for RDS is at
    :doc:`RDS v2 <ref/rds2>`.


Creating a Connection
---------------------
The first step in accessing RDS is to create a connection to the service.
The recommended method of doing this is as follows::

    >>> import boto.rds
    >>> conn = boto.rds.connect_to_region(
    ...     "us-west-2",
    ...     aws_access_key_id='<aws access key'>,
    ...     aws_secret_access_key='<aws secret key>')

At this point the variable conn will point to an RDSConnection object in the
US-WEST-2 region. Bear in mind that just as any other AWS service, RDS is
region-specific. In this example, the AWS access key and AWS secret key are
passed in to the method explicitly. Alternatively, you can set the environment
variables:

* ``AWS_ACCESS_KEY_ID`` - Your AWS Access Key ID
* ``AWS_SECRET_ACCESS_KEY`` - Your AWS Secret Access Key

and then simply call::

    >>> import boto.rds
    >>> conn = boto.rds.connect_to_region("us-west-2")

In either case, conn will point to an RDSConnection object which we will
use throughout the remainder of this tutorial.

Starting an RDS Instance
------------------------

Creating a DB instance is easy. You can do so as follows::

   >>> db = conn.create_dbinstance("db-master-1", 10, 'db.m1.small', 'root', 'hunter2')

This example would create a DB identified as ``db-master-1`` with 10GB of
storage. This instance would be running on ``db.m1.small`` type, with the login
name being ``root``, and the password ``hunter2``.

To check on the status of your RDS instance, you will have to query the RDS connection again::

    >>> instances = conn.get_all_dbinstances("db-master-1")
    >>> instances
    [DBInstance:db-master-1]
    >>> db = instances[0]
    >>> db.status
    u'available'
    >>> db.endpoint
    (u'db-master-1.aaaaaaaaaa.us-west-2.rds.amazonaws.com', 3306)

Creating a Security Group
-------------------------

Before you can actually connect to this RDS service, you must first
create a security group. You can add a CIDR range or an :py:class:`EC2 security
group <boto.ec2.securitygroup.SecurityGroup>`  to your :py:class:`DB security
group <boto.rds.dbsecuritygroup.DBSecurityGroup>` ::

    >>> sg = conn.create_dbsecurity_group('web_servers', 'Web front-ends')
    >>> sg.authorize(cidr_ip='10.3.2.45/32')
    True

You can then associate this security group with your RDS instance::

    >>> db.modify(security_groups=[sg])


Connecting to your New Database
-------------------------------

Once you have reached this step, you can connect to your RDS instance as you
would with any other MySQL instance::

    >>> db.endpoint
    (u'db-master-1.aaaaaaaaaa.us-west-2.rds.amazonaws.com', 3306)

    % mysql -h db-master-1.aaaaaaaaaa.us-west-2.rds.amazonaws.com -u root -phunter2
    mysql>


Making a backup
---------------

You can also create snapshots of your database very easily::

    >>> db.snapshot('db-master-1-2013-02-05')
    DBSnapshot:db-master-1-2013-02-05


Once this snapshot is complete, you can create a new database instance from
it::

    >>> db2 = conn.restore_dbinstance_from_dbsnapshot(
    ...    'db-master-1-2013-02-05',
    ...    'db-restored-1',
    ...    'db.m1.small',
    ...    'us-west-2')

