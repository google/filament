.. dynamodb_v1_to_v2:

=========================================
Migrating from DynamoDB v1 to DynamoDB v2
=========================================

For the v2 release of AWS' DynamoDB_, the high-level API for interacting via
``boto`` was rewritten. Since there were several new features added in v2,
people using the v1 API may wish to transition their code to the new API.
This guide covers the high-level APIs.

.. _DynamoDB: http://aws.amazon.com/dynamodb/


Creating New Tables
===================

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> message_table_schema = conn.create_schema(
    ...     hash_key_name='forum_name',
    ...     hash_key_proto_value=str,
    ...     range_key_name='subject',
    ...     range_key_proto_value=str
    ... )
    >>> table = conn.create_table(
    ...     name='messages',
    ...     schema=message_table_schema,
    ...     read_units=10,
    ...     write_units=10
    ... )

DynamoDB v2::

    >>> from boto.dynamodb2.fields import HashKey
    >>> from boto.dynamodb2.fields import RangeKey
    >>> from boto.dynamodb2.table import Table

    >>> table = Table.create('messages', schema=[
    ...     HashKey('forum_name'),
    ...     RangeKey('subject'),
    ... ], throughput={
    ...     'read': 10,
    ...     'write': 10,
    ... })


Using an Existing Table
=======================

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    # With API calls.
    >>> table = conn.get_table('messages')

    # Without API calls.
    >>> message_table_schema = conn.create_schema(
    ...     hash_key_name='forum_name',
    ...     hash_key_proto_value=str,
    ...     range_key_name='subject',
    ...     range_key_proto_value=str
    ... )
    >>> table = conn.table_from_schema(
    ...     name='messages',
    ...     schema=message_table_schema)


DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    # With API calls.
    >>> table = Table('messages')

    # Without API calls.
    >>> from boto.dynamodb2.fields import HashKey
    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages', schema=[
    ...     HashKey('forum_name'),
    ...     HashKey('subject'),
    ... ])


Updating Throughput
===================

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> conn.update_throughput(table, read_units=5, write_units=15)

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')
    >>> table.update(throughput={
    ...     'read': 5,
    ...     'write': 15,
    ... })


Deleting a Table
================

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> conn.delete_table(table)

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')
    >>> table.delete()


Creating an Item
================

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> item_data = {
    ...     'Body': 'http://url_to_lolcat.gif',
    ...     'SentBy': 'User A',
    ...     'ReceivedTime': '12/9/2011 11:36:03 PM',
    ... }
    >>> item = table.new_item(
    ...     # Our hash key is 'forum'
    ...     hash_key='LOLCat Forum',
    ...     # Our range key is 'subject'
    ...     range_key='Check this out!',
    ...     # This has the
    ...     attrs=item_data
    ... )

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')
    >>> item = table.put_item(data={
    ...     'forum_name': 'LOLCat Forum',
    ...     'subject': 'Check this out!',
    ...     'Body': 'http://url_to_lolcat.gif',
    ...     'SentBy': 'User A',
    ...     'ReceivedTime': '12/9/2011 11:36:03 PM',
    ... })


Getting an Existing Item
========================

DynamoDB v1::

    >>> table = conn.get_table('messages')
    >>> item = table.get_item(
    ...     hash_key='LOLCat Forum',
    ...     range_key='Check this out!'
    ... )

DynamoDB v2::

    >>> table = Table('messages')
    >>> item = table.get_item(
    ...     forum_name='LOLCat Forum',
    ...     subject='Check this out!'
    ... )


Updating an Item
================

DynamoDB v1::

    >>> item['a_new_key'] = 'testing'
    >>> del item['a_new_key']
    >>> item.put()

DynamoDB v2::

    >>> item['a_new_key'] = 'testing'
    >>> del item['a_new_key']

    # Conditional save, only if data hasn't changed.
    >>> item.save()

    # Forced full overwrite.
    >>> item.save(overwrite=True)

    # Partial update (only changed fields).
    >>> item.partial_save()


Deleting an Item
================

DynamoDB v1::

    >>> item.delete()

DynamoDB v2::

    >>> item.delete()


Querying
========

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> from boto.dynamodb.condition import BEGINS_WITH
    >>> items = table.query('Amazon DynamoDB',
    ...                     range_key_condition=BEGINS_WITH('DynamoDB'),
    ...                     request_limit=1, max_results=1)
    >>> for item in items:
    >>>     print item['Body']

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')
    >>> items = table.query_2(
    ...     forum_name__eq='Amazon DynamoDB',
    ...     subject__beginswith='DynamoDB',
    ...     limit=1
    ... )
    >>> for item in items:
    >>>     print item['Body']


Scans
=====

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')

    # All items.
    >>> items = table.scan()

    # With a filter.
    >>> items = table.scan(scan_filter={'Replies': GT(0)})

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')

    # All items.
    >>> items = table.scan()

    # With a filter.
    >>> items = table.scan(replies__gt=0)


Batch Gets
==========

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> from boto.dynamodb.batch import BatchList
    >>> the_batch = BatchList(conn)
    >>> the_batch.add_batch(table, keys=[
    ...     ('LOLCat Forum', 'Check this out!'),
    ...     ('LOLCat Forum', 'I can haz docs?'),
    ...     ('LOLCat Forum', 'Maru'),
    ... ])
    >>> results = conn.batch_get_item(the_batch)

    # (Largely) Raw dictionaries back from DynamoDB.
    >>> for item_dict in response['Responses'][table.name]['Items']:
    ...     print item_dict['Body']

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')
    >>> results = table.batch_get(keys=[
    ...     {'forum_name': 'LOLCat Forum', 'subject': 'Check this out!'},
    ...     {'forum_name': 'LOLCat Forum', 'subject': 'I can haz docs?'},
    ...     {'forum_name': 'LOLCat Forum', 'subject': 'Maru'},
    ... ])

    # Lazy requests across pages, if paginated.
    >>> for res in results:
    ...     # You get back actual ``Item`` instances.
    ...     print item['Body']


Batch Writes
============

DynamoDB v1::

    >>> import boto.dynamodb
    >>> conn = boto.dynamodb.connect_to_region()
    >>> table = conn.get_table('messages')
    >>> from boto.dynamodb.batch import BatchWriteList
    >>> from boto.dynamodb.item import Item

    # You must manually manage this so that your total ``puts/deletes`` don't
    # exceed 25.
    >>> the_batch = BatchList(conn)
    >>> the_batch.add_batch(table, puts=[
    ...     Item(table, 'Corgi Fanciers', 'Sploots!', {
    ...         'Body': 'Post your favorite corgi-on-the-floor shots!',
    ...         'SentBy': 'User B',
    ...         'ReceivedTime': '2013/05/02 10:56:45 AM',
    ...     }),
    ...     Item(table, 'Corgi Fanciers', 'Maximum FRAPS', {
    ...         'Body': 'http://internetvideosite/watch?v=1247869',
    ...         'SentBy': 'User C',
    ...         'ReceivedTime': '2013/05/01 09:15:25 PM',
    ...     }),
    ... ], deletes=[
    ...     ('LOLCat Forum', 'Off-topic post'),
    ...     ('LOLCat Forum', 'They be stealin mah bukket!'),
    ... ])
    >>> conn.batch_write_item(the_writes)

DynamoDB v2::

    >>> from boto.dynamodb2.table import Table
    >>> table = Table('messages')

    # Uses a context manager, which also automatically handles batch sizes.
    >>> with table.batch_write() as batch:
    ...     batch.delete_item(
    ...         forum_name='LOLCat Forum',
    ...         subject='Off-topic post'
    ...     )
    ...     batch.put_item(data={
    ...         'forum_name': 'Corgi Fanciers',
    ...         'subject': 'Sploots!',
    ...         'Body': 'Post your favorite corgi-on-the-floor shots!',
    ...         'SentBy': 'User B',
    ...         'ReceivedTime': '2013/05/02 10:56:45 AM',
    ...     })
    ...     batch.put_item(data={
    ...         'forum_name': 'Corgi Fanciers',
    ...         'subject': 'Sploots!',
    ...         'Body': 'Post your favorite corgi-on-the-floor shots!',
    ...         'SentBy': 'User B',
    ...         'ReceivedTime': '2013/05/02 10:56:45 AM',
    ...     })
    ...     batch.delete_item(
    ...         forum_name='LOLCat Forum',
    ...         subject='They be stealin mah bukket!'
    ...     )
