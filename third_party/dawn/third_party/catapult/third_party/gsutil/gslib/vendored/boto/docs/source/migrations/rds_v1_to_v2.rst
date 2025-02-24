.. rds_v1_to_v2:

===============================
Migrating from RDS v1 to RDS v2
===============================

The original ``boto.rds`` module has historically lagged quite far behind the
service (at time of writing, almost 50% of the API calls are
missing/out-of-date). To address this, the Boto core team has switched to
a generated client for RDS (``boto.rds2.layer1.RDSConnection``).

However, this generated variant is not backward-compatible with the older
``boto.rds.RDSConnection``. This document is to help you update your code
(as desired) to take advantage of the latest API calls.

For the duration of the document, **RDS2Connection** refers to
``boto.rds2.layer1.RDSConnection``, where **RDSConnection** refers to
``boto.rds.RDSConnection``.


Prominent Differences
=====================

* The new **RDS2Connection** maps very closely to the `official API operations`_,
  where the old **RDSConnection** had non-standard & inconsistent method names.
* **RDS2Connection** almost always returns a Python dictionary that maps
  closely to the API output. **RDSConnection** returned Python objects.
* **RDS2Connection** is much more verbose in terms of output. Tools like
  `jmespath`_ or `jsonq`_ can make handling these sometimes complex dictionaries more
  manageable.

.. _`official API operations`: http://docs.aws.amazon.com/AmazonRDS/latest/APIReference/Welcome.html
.. _`jmespath`: https://github.com/boto/jmespath
.. _`jsonq`: https://github.com/edmund-huber/jsonq


Method Renames
==============

Format is ``old_method_name`` -> ``new_method_name``:

* ``authorize_dbsecurity_group`` -> ``authorize_db_security_group_ingress``
* ``create_dbinstance`` -> ``create_db_instance``
* ``create_dbinstance_read_replica`` -> ``create_db_instance_read_replica``
* ``create_parameter_group`` -> ``create_db_parameter_group``
* ``get_all_dbsnapshots`` -> ``describe_db_snapshots``
* ``get_all_events`` -> ``describe_events``
* ``modify_dbinstance`` -> ``modify_db_instance``
* ``reboot_dbinstance`` -> ``reboot_db_instance``
* ``restore_dbinstance_from_dbsnapshot`` -> ``restore_db_instance_from_db_snapshot``
* ``restore_dbinstance_from_point_in_time`` -> ``restore_db_instance_to_point_in_time``
* ``revoke_dbsecurity_group`` -> ``revoke_db_security_group_ingress``


Parameter Changes
=================

Many parameter names have changed between **RDSConnection** &
**RDS2Connection**. For instance, the old name for the instance identifier was
``id``, where the new name is ``db_instance_identifier``. These changes are to
ensure things map more closely to the API.

In addition, in some cases, ordering & required-ness of parameters has changed
as well. For instance, in ``create_db_instance``, the
``engine`` parameter is now required (previously defaulted to ``MySQL5.1``) &
its position in the call has change to be before ``master_username``.

As such, when updating your API calls, you should check the
API Reference documentation to ensure you're passing the
correct parameters.


Return Values
=============

**RDSConnection** frequently returned higher-level Python objects. In contrast,
**RDS2Connection** returns Python dictionaries of the data. This will require
a bit more work to extract the necessary values. For example::

    # Old
    >>> instances = rds1_conn.get_all_dbinstances()
    >>> inst = instances[0]
    >>> inst.name
    'test-db'

    # New
    >>> instances = rds2_conn.describe_db_instances()
    >>> inst = instances['DescribeDBInstancesResponse']\
    ...                 ['DescribeDBInstancesResult']['DBInstances'][0]
    >>> inst['DBName']
    'test-db'
