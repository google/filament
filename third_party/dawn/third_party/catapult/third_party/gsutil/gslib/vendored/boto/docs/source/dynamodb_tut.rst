.. dynamodb_tut:

============================================
An Introduction to boto's DynamoDB interface
============================================

This tutorial focuses on the boto interface to AWS' DynamoDB_. This tutorial
assumes that you have boto already downloaded and installed.

.. _DynamoDB: http://aws.amazon.com/dynamodb/

.. warning::

    This tutorial covers the **ORIGINAL** release of DynamoDB.
    It has since been supplanted by a second major version & an
    updated API to talk to the new version. The documentation for the
    new version of DynamoDB (& boto's support for it) is at
    :doc:`DynamoDB v2 <dynamodb2_tut>`.


Creating a Connection
---------------------

The first step in accessing DynamoDB is to create a connection to the service.
To do so, the most straight forward way is the following::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region(
            'us-west-2',
            aws_access_key_id='<YOUR_AWS_KEY_ID>',
            aws_secret_access_key='<YOUR_AWS_SECRET_KEY>')
    >>> conn
    <boto.dynamodb.layer2.Layer2 object at 0x3fb3090>

Bear in mind that if you have your credentials in boto config in your home
directory, the two keyword arguments in the call above are not needed. More
details on configuration can be found in :doc:`boto_config_tut`.

The :py:func:`boto.dynamodb.connect_to_region` function returns a
:py:class:`boto.dynamodb.layer2.Layer2` instance, which is a high-level API
for working with DynamoDB. Layer2 is a set of abstractions that sit atop
the lower level :py:class:`boto.dynamodb.layer1.Layer1` API, which closely
mirrors the Amazon DynamoDB API. For the purpose of this tutorial, we'll
just be covering Layer2.


Listing Tables
--------------

Now that we have a DynamoDB connection object, we can then query for a list of
existing tables in that region::

    >>> conn.list_tables()
    ['test-table', 'another-table']


Creating Tables
---------------

DynamoDB tables are created with the
:py:meth:`Layer2.create_table <boto.dynamodb.layer2.Layer2.create_table>`
method. While DynamoDB's items (a rough equivalent to a relational DB's row)
don't have a fixed schema, you do need to create a schema for the table's
hash key element, and the optional range key element. This is explained in
greater detail in DynamoDB's `Data Model`_ documentation.

We'll start by defining a schema that has a hash key and a range key that
are both strings::

    >>> message_table_schema = conn.create_schema(
            hash_key_name='forum_name',
            hash_key_proto_value=str,
            range_key_name='subject',
            range_key_proto_value=str
        )

The next few things to determine are table name and read/write throughput. We'll
defer explaining throughput to the DynamoDB's `Provisioned Throughput`_ docs.

We're now ready to create the table::

    >>> table = conn.create_table(
            name='messages',
            schema=message_table_schema,
            read_units=10,
            write_units=10
        )
    >>> table
    Table(messages)

This returns a :py:class:`boto.dynamodb.table.Table` instance, which provides
simple ways to create (put), update, and delete items.


Getting a Table
---------------

To retrieve an existing table, use
:py:meth:`Layer2.get_table <boto.dynamodb.layer2.Layer2.get_table>`::

    >>> conn.list_tables()
    ['test-table', 'another-table', 'messages']
    >>> table = conn.get_table('messages')
    >>> table
    Table(messages)

:py:meth:`Layer2.get_table <boto.dynamodb.layer2.Layer2.get_table>`, like
:py:meth:`Layer2.create_table <boto.dynamodb.layer2.Layer2.create_table>`,
returns a :py:class:`boto.dynamodb.table.Table` instance.

Keep in mind that :py:meth:`Layer2.get_table <boto.dynamodb.layer2.Layer2.get_table>`
will make an API call to retrieve various attributes of the table including the
creation time, the read and write capacity, and the table schema.  If you
already know the schema, you can save an API call and create a
:py:class:`boto.dynamodb.table.Table` object without making any calls to
Amazon DynamoDB::

    >>> table = conn.table_from_schema(
        name='messages',
        schema=message_table_schema)

If you do this, the following fields will have ``None`` values:

  * create_time
  * status
  * read_units
  * write_units

In addition, the ``item_count`` and ``size_bytes`` will be 0.
If you create a table object directly from a schema object and
decide later that you need to retrieve any of these additional
attributes, you can use the
:py:meth:`Table.refresh <boto.dynamodb.table.Table.refresh>` method::

    >>> from boto.dynamodb.schema import Schema
    >>> table = conn.table_from_schema(
            name='messages',
            schema=Schema.create(hash_key=('forum_name', 'S'),
                                 range_key=('subject', 'S')))
    >>> print table.write_units
    None
    >>> # Now we decide we need to know the write_units:
    >>> table.refresh()
    >>> print table.write_units
    10


The recommended best practice is to retrieve a table object once and
use that object for the duration of your application. So, for example,
instead of this::

    class Application(object):
        def __init__(self, layer2):
            self._layer2 = layer2

        def retrieve_item(self, table_name, key):
            return self._layer2.get_table(table_name).get_item(key)

You can do something like this instead::

    class Application(object):
        def __init__(self, layer2):
            self._layer2 = layer2
            self._tables_by_name = {}

        def retrieve_item(self, table_name, key):
            table = self._tables_by_name.get(table_name)
            if table is None:
                table = self._layer2.get_table(table_name)
                self._tables_by_name[table_name] = table
            return table.get_item(key)


Describing Tables
-----------------

To get a complete description of a table, use
:py:meth:`Layer2.describe_table <boto.dynamodb.layer2.Layer2.describe_table>`::

    >>> conn.list_tables()
    ['test-table', 'another-table', 'messages']
    >>> conn.describe_table('messages')
    {
        'Table': {
            'CreationDateTime': 1327117581.624,
            'ItemCount': 0,
            'KeySchema': {
                'HashKeyElement': {
                    'AttributeName': 'forum_name',
                    'AttributeType': 'S'
                },
                'RangeKeyElement': {
                    'AttributeName': 'subject',
                    'AttributeType': 'S'
                }
            },
            'ProvisionedThroughput': {
                'ReadCapacityUnits': 10,
                'WriteCapacityUnits': 10
            },
            'TableName': 'messages',
            'TableSizeBytes': 0,
            'TableStatus': 'ACTIVE'
        }
    }


Adding Items
------------

Continuing on with our previously created ``messages`` table, adding an::

    >>> table = conn.get_table('messages')
    >>> item_data = {
            'Body': 'http://url_to_lolcat.gif',
            'SentBy': 'User A',
            'ReceivedTime': '12/9/2011 11:36:03 PM',
        }
    >>> item = table.new_item(
            # Our hash key is 'forum'
            hash_key='LOLCat Forum',
            # Our range key is 'subject'
            range_key='Check this out!',
            # This has the
            attrs=item_data
        )

The
:py:meth:`Table.new_item <boto.dynamodb.table.Table.new_item>` method creates
a new :py:class:`boto.dynamodb.item.Item` instance with your specified
hash key, range key, and attributes already set.
:py:class:`Item <boto.dynamodb.item.Item>` is a :py:class:`dict` sub-class,
meaning you can edit your data as such::

    item['a_new_key'] = 'testing'
    del item['a_new_key']

After you are happy with the contents of the item, use
:py:meth:`Item.put <boto.dynamodb.item.Item.put>` to commit it to DynamoDB::

    >>> item.put()


Retrieving Items
----------------

Now, let's check if it got added correctly. Since DynamoDB works under an
'eventual consistency' mode, we need to specify that we wish a consistent read,
as follows::

    >>> table = conn.get_table('messages')
    >>> item = table.get_item(
            # Your hash key was 'forum_name'
            hash_key='LOLCat Forum',
            # Your range key was 'subject'
            range_key='Check this out!'
        )
    >>> item
    {
        # Note that this was your hash key attribute (forum_name)
        'forum_name': 'LOLCat Forum',
        # This is your range key attribute (subject)
        'subject': 'Check this out!'
        'Body': 'http://url_to_lolcat.gif',
        'ReceivedTime': '12/9/2011 11:36:03 PM',
        'SentBy': 'User A',
    }


Updating Items
--------------

To update an item's attributes, simply retrieve it, modify the value, then
:py:meth:`Item.put <boto.dynamodb.item.Item.put>` it again::

    >>> table = conn.get_table('messages')
    >>> item = table.get_item(
            hash_key='LOLCat Forum',
            range_key='Check this out!'
        )
    >>> item['SentBy'] = 'User B'
    >>> item.put()

Working with Decimals
---------------------

To avoid the loss of precision, you can stipulate that the
``decimal.Decimal`` type be used for numeric values::

    >>> import decimal
    >>> conn.use_decimals()
    >>> table = conn.get_table('messages')
    >>> item = table.new_item(
            hash_key='LOLCat Forum',
            range_key='Check this out!'
        )
    >>> item['decimal_type'] = decimal.Decimal('1.12345678912345')
    >>> item.put()
    >>> print table.get_item('LOLCat Forum', 'Check this out!')
    {u'forum_name': 'LOLCat Forum', u'decimal_type': Decimal('1.12345678912345'),
     u'subject': 'Check this out!'}

You can enable the usage of ``decimal.Decimal`` by using either the ``use_decimals``
method, or by passing in the
:py:class:`Dynamizer <boto.dynamodb.types.Dynamizer>` class for
the ``dynamizer`` param::

    >>> from boto.dynamodb.types import Dynamizer
    >>> conn = boto.dynamodb.connect_to_region(dynamizer=Dynamizer)

This mechanism can also be used if you want to customize the encoding/decoding
process of DynamoDB types.


Deleting Items
--------------

To delete items, use the
:py:meth:`Item.delete <boto.dynamodb.item.Item.delete>` method::

    >>> table = conn.get_table('messages')
    >>> item = table.get_item(
            hash_key='LOLCat Forum',
            range_key='Check this out!'
        )
    >>> item.delete()


Deleting Tables
---------------

.. WARNING::
  Deleting a table will also **permanently** delete all of its contents without prompt. Use carefully.

There are two easy ways to delete a table. Through your top-level
:py:class:`Layer2 <boto.dynamodb.layer2.Layer2>` object::

    >>> conn.delete_table(table)

Or by getting the table, then using
:py:meth:`Table.delete <boto.dynamodb.table.Table.delete>`::

    >>> table = conn.get_table('messages')
    >>> table.delete()


.. _Data Model: http://docs.amazonwebservices.com/amazondynamodb/latest/developerguide/DataModel.html
.. _Provisioned Throughput: http://docs.amazonwebservices.com/amazondynamodb/latest/developerguide/ProvisionedThroughputIntro.html
