.. simpledb_tut:

============================================
An Introduction to boto's SimpleDB interface
============================================

This tutorial focuses on the boto interface to AWS' SimpleDB_. This tutorial
assumes that you have boto already downloaded and installed.

.. _SimpleDB: http://aws.amazon.com/simpledb/

.. note::

    If you're starting a new application, you might want to consider using
    :doc:`DynamoDB2 <dynamodb2_tut>` instead, as it has a more comprehensive
    feature set & has guaranteed performance throughput levels.

Creating a Connection
---------------------
The first step in accessing SimpleDB is to create a connection to the service.
To do so, the most straight forward way is the following::

    >>> import boto.sdb
    >>> conn = boto.sdb.connect_to_region(
    ...     'us-west-2',
    ...     aws_access_key_id='<YOUR_AWS_KEY_ID>',
    ...     aws_secret_access_key='<YOUR_AWS_SECRET_KEY>')
    >>> conn
    SDBConnection:sdb.amazonaws.com
    >>>

Bear in mind that if you have your credentials in boto config in your home
directory, the two keyword arguments in the call above are not needed. Also
important to note is that just as any other AWS service, SimpleDB is
region-specific and as such you might want to specify which region to connect
to, by default, it'll connect to the US-EAST-1 region.

Creating Domains
----------------
Arguably, once you have your connection established, you'll want to create one or more dmains.
Creating new domains is a fairly straight forward operation. To do so, you can proceed as follows::

    >>> conn.create_domain('test-domain')
    Domain:test-domain
    >>>
    >>> conn.create_domain('test-domain-2')
    Domain:test-domain
    >>>

Please note that SimpleDB, unlike its newest sibling DynamoDB, is truly and completely schema-less.
Thus, there's no need specify domain keys or ranges.

Listing All Domains
-------------------
Unlike DynamoDB or other database systems, SimpleDB uses the concept of 'domains' instead of tables.
So, to list all your domains for your account in a region, you can simply do as follows::

    >>> domains = conn.get_all_domains()
    >>> domains
    [Domain:test-domain, Domain:test-domain-2]
    >>>

The get_all_domains() method returns a :py:class:`boto.resultset.ResultSet` containing
all :py:class:`boto.sdb.domain.Domain` objects associated with
this connection's Access Key ID for that region.

Retrieving a Domain (by name)
-----------------------------
If you wish to retrieve a specific domain whose name is known, you can do so as follows::

    >>> dom = conn.get_domain('test-domain')
    >>> dom
    Domain:test-domain
    >>>

The get_domain call has an optional validate parameter, which defaults to True. This will make sure to raise
an exception if the domain you are looking for doesn't exist. If you set it to false, it will return a
:py:class:`Domain <boto.sdb.domain.Domain>` object blindly regardless of its existence.

Getting Domain Metadata
-----------------------
There are times when you might want to know your domains' machine usage, aprox. item count and other such data.
To this end, boto offers a simple and convenient way to do so as shown below::

    >>> domain_meta = conn.domain_metadata(dom)
    >>> domain_meta
    <boto.sdb.domain.DomainMetaData instance at 0x23cd440>
    >>> dir(domain_meta)
    ['BoxUsage', 'DomainMetadataResponse', 'DomainMetadataResult', 'RequestId', 'ResponseMetadata',
    '__doc__', '__init__', '__module__', 'attr_name_count', 'attr_names_size', 'attr_value_count', 'attr_values_size',
    'domain', 'endElement', 'item_count', 'item_names_size', 'startElement', 'timestamp']
    >>> domain_meta.item_count
    0
    >>>

Please bear in mind that while in the example above we used a previously retrieved domain object as the parameter, you
can retrieve the domain metadata via its name (string).

Adding Items (and attributes)
-----------------------------
Once you have your domain setup, presumably, you'll want to start adding items to it.
In its most straight forward form, you need to provide a name for the item -- think of it
as a record id -- and a collection of the attributes you want to store in the item (often a Dictionary-like object).
So, adding an item to a domain looks as follows::

    >>> item_name = 'ABC_123'
    >>> item_attrs = {'Artist': 'The Jackson 5', 'Genera':'Pop'}
    >>> dom.put_attributes(item_name, item_attrs)
    True
    >>>

Now let's check if it worked::

    >>> domain_meta = conn.domain_metadata(dom)
    >>> domain_meta.item_count
    1
    >>>


Batch Adding Items (and attributes)
-----------------------------------
You can also add a number of items at the same time in a similar fashion. All you have to provide to the batch_put_attributes() method
is a Dictionary-like object with your items and their respective attributes, as follows::

    >>> items = {'item1':{'attr1':'val1'},'item2':{'attr2':'val2'}}
    >>> dom.batch_put_attributes(items)
    True
    >>>

Now, let's check the item count once again::

    >>> domain_meta = conn.domain_metadata(dom)
    >>> domain_meta.item_count
    3
    >>>

A few words of warning: both batch_put_attributes() and put_item(), by default, will overwrite the values of the attributes if both
the item and attribute already exist. If the item exists, but not the attributes, it will append the new attributes to the
attribute list of that item. If you do not wish these methods to behave in that manner, simply supply them with a 'replace=False'
parameter.


Retrieving Items
----------------
To retrieve an item along with its attributes is a fairly straight forward operation and can be accomplished as follows::

    >>> dom.get_item('item1')
    {u'attr1': u'val1'}
    >>>

Since SimpleDB works in an "eventual consistency" manner, we can also request a forced consistent read (though this will
invariably adversely affect read performance). The way to accomplish that is as shown below::

    >>> dom.get_item('item1', consistent_read=True)
    {u'attr1': u'val1'}
    >>>

Retrieving One or More Items
----------------------------
Another way to retrieve items is through boto's select() method. This method, at the bare minimum, requires a standard SQL select query string
and you would do something along the lines of::

    >>> query = 'select * from `test-domain` where attr1="val1"'
    >>> rs = dom.select(query)
    >>> for j in rs:
    ...   print 'o hai'
    ...
    o hai
    >>>

This method returns a ResultSet collection you can iterate over.

Updating Item Attributes
------------------------
The easiest way to modify an item's attributes is by manipulating the item's attributes and then saving those changes. For example::

    >>> item = dom.get_item('item1')
    >>> item['attr1'] = 'val_changed'
    >>> item.save()


Deleting Items (and its attributes)
-----------------------------------
Deleting an item is a very simple operation. All you are required to provide is either the name of the item or an item object to the
delete_item() method, boto will take care of the rest::

    >>>dom.delete_item(item)
    >>>True



Deleting Domains
----------------
To delete a domain and all items under it (i.e. be very careful), you can do it as follows::

    >>> conn.delete_domain('test-domain')
    True
    >>>
