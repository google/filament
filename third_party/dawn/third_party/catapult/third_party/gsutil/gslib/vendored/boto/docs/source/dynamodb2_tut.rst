.. _dynamodb2_tut:

===============================================
An Introduction to boto's DynamoDB v2 interface
===============================================

This tutorial focuses on the boto interface to AWS' DynamoDB_ v2. This tutorial
assumes that you have boto already downloaded and installed.

.. _DynamoDB: http://aws.amazon.com/dynamodb/

.. warning::

    This tutorial covers the **SECOND** major release of DynamoDB (including
    local secondary index support). The documentation for the original
    version of DynamoDB (& boto's support for it) is at
    :doc:`DynamoDB v1 <dynamodb_tut>`.

The v2 DynamoDB API has both a high-level & low-level component. The low-level
API (contained primarily within ``boto.dynamodb2.layer1``) provides an
interface that rough matches exactly what is provided by the API. It supports
all options available to the service.

The high-level API attempts to make interacting with the service more natural
from Python. It supports most of the featureset.


The High-Level API
==================

Most of the interaction centers around a single object, the ``Table``. Tables
act as a way to effectively namespace your records. If you're familiar with
database tables from an RDBMS, tables will feel somewhat familiar.


Creating a New Table
--------------------

To create a new table, you need to call ``Table.create`` & specify (at a
minimum) both the table's name as well as the key schema for the table::

    >>> from boto.dynamodb2.fields import HashKey
    >>> from boto.dynamodb2.table import Table
    >>> users = Table.create('users', schema=[HashKey('username')]);

Since both the key schema and local secondary indexes can not be
modified after the table is created, you'll need to plan ahead of time how you
think the table will be used. Both the keys & indexes are also used for
querying, so you'll want to represent the data you'll need when querying
there as well.

For the schema, you can either have a single ``HashKey`` or a combined
``HashKey+RangeKey``. The ``HashKey`` by itself should be thought of as a
unique identifier (for instance, like a username or UUID). It is typically
looked up as an exact value.
A ``HashKey+RangeKey`` combination is slightly different, in that the
``HashKey`` acts like a namespace/prefix & the ``RangeKey`` acts as a value
that can be referred to by a sorted range of values.

For the local secondary indexes, you can choose from an ``AllIndex``, a
``KeysOnlyIndex`` or a ``IncludeIndex`` field. Each builds an index of values
that can be queried on. The ``AllIndex`` duplicates all values onto the index
(to prevent additional reads to fetch the data). The ``KeysOnlyIndex``
duplicates only the keys from the schema onto the index. The ``IncludeIndex``
lets you specify a list of fieldnames to duplicate over.

A full example::

    >>> import boto.dynamodb2
    >>> from boto.dynamodb2.fields import HashKey, RangeKey, KeysOnlyIndex, GlobalAllIndex
    >>> from boto.dynamodb2.table import Table
    >>> from boto.dynamodb2.types import NUMBER

    # Uses your ``aws_access_key_id`` & ``aws_secret_access_key`` from either a
    # config file or environment variable & the default region.
    >>> users = Table.create('users', schema=[
    ...     HashKey('username'), # defaults to STRING data_type
    ...     RangeKey('last_name'),
    ... ], throughput={
    ...     'read': 5,
    ...     'write': 15,
    ... }, global_indexes=[
    ...     GlobalAllIndex('EverythingIndex', parts=[
    ...         HashKey('account_type'),
    ...     ],
    ...     throughput={
    ...         'read': 1,
    ...         'write': 1,
    ...     })
    ... ],
    ... # If you need to specify custom parameters, such as credentials or region,
    ... # use the following:
    ... # connection=boto.dynamodb2.connect_to_region('us-east-1')
    ... )


Using an Existing Table
-----------------------

Once a table has been created, using it is relatively simple. You can either
specify just the ``table_name`` (allowing the object to lazily do an additional
call to get details about itself if needed) or provide the ``schema/indexes``
again (same as what was used with ``Table.create``) to avoid extra overhead.

Lazy example::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

Efficient example::

    >>> from boto.dynamodb2.fields import HashKey, RangeKey, GlobalAllIndex
    >>> from boto.dynamodb2.table import Table
    >>> from boto.dynamodb2.types import NUMBER
    >>> users = Table('users', schema=[
    ...     HashKey('username'),
    ...     RangeKey('last_name'),
    ... ], global_indexes=[
    ...     GlobalAllIndex('EverythingIndex', parts=[
    ...         HashKey('account_type'),
    ...     ])
    ... ])


Creating a New Item
-------------------

Once you have a ``Table`` instance, you can add new items to the table. There
are two ways to do this.

The first is to use the ``Table.put_item`` method. Simply hand it a dictionary
of data & it will create the item on the server side. This dictionary should
be relatively flat (as you can nest in other dictionaries) & **must** contain
the keys used in the ``schema``.

Example::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    # Create the new user.
    >>> users.put_item(data={
    ...     'username': 'johndoe',
    ...     'first_name': 'John',
    ...     'last_name': 'Doe',
    ...     'account_type': 'standard_user',
    ... })
    True

The alternative is to manually construct an ``Item`` instance & tell it to
``save`` itself. This is useful if the object will be around for awhile & you
don't want to re-fetch it.

Example::

    >>> from boto.dynamodb2.items import Item
    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    # WARNING - This doens't save it yet!
    >>> janedoe = Item(users, data={
    ...     'username': 'janedoe',
    ...     'first_name': 'Jane',
    ...     'last_name': 'Doe',
    ...     'account_type': 'standard_user',
    ... })

    # The data now gets persisted to the server.
    >>> janedoe.save()
    True


Getting an Item & Accessing Data
--------------------------------

With data now in DynamoDB, if you know the key of the item, you can fetch it
back out. Specify the key value(s) as kwargs to ``Table.get_item``.

Example::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    >>> johndoe = users.get_item(username='johndoe', last_name='Doe')

Once you have an ``Item`` instance, it presents a dictionary-like interface to
the data.::

    # Read a field out.
    >>> johndoe['first_name']
    'John'

    # Change a field (DOESN'T SAVE YET!).
    >>> johndoe['first_name'] = 'Johann'

    # Delete data from it (DOESN'T SAVE YET!).
    >>> del johndoe['account_type']


Updating an Item
----------------

Just creating new items or changing only the in-memory version of the ``Item``
isn't particularly effective. To persist the changes to DynamoDB, you have
three choices.

The first is sending all the data with the expectation nothing has changed
since you read the data. DynamoDB will verify the data is in the original state
and, if so, will send all of the item's data. If that expectation fails, the
call will fail::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    >>> johndoe = users.get_item(username='johndoe', last_name='Doe')
    >>> johndoe['first_name'] = 'Johann'
    >>> johndoe['whatever'] = "man, that's just like your opinion"
    >>> del johndoe['account_type']

    # Affects all fields, even the ones not changed locally.
    >>> johndoe.save()
    True

The second is a full overwrite. If you can be confident your version of the
data is the most correct, you can force an overwrite of the data.::

    >>> johndoe = users.get_item(username='johndoe', last_name='Doe')
    >>> johndoe['first_name'] = 'Johann'
    >>> johndoe['whatever'] = "Man, that's just like your opinion"

    # Specify ``overwrite=True`` to fully replace the data.
    >>> johndoe.save(overwrite=True)
    True

The last is a partial update. If you've only modified certain fields, you
can send a partial update that only writes those fields, allowing other
(potentially changed) fields to go untouched.::

    >>> johndoe = users.get_item(username='johndoe', last_name='Doe')
    >>> johndoe['first_name'] = 'Johann'
    >>> johndoe['whatever'] = "man, that's just like your opinion"
    >>> del johndoe['account_type']

    # Partial update, only sending/affecting the
    # ``first_name/whatever/account_type`` fields.
    >>> johndoe.partial_save()
    True


Deleting an Item
----------------

You can also delete items from the table. You have two choices, depending on
what data you have present.

If you already have an ``Item`` instance, the easiest approach is just to call
``Item.delete``.::

    >>> johndoe.delete()
    True

If you don't have an ``Item`` instance & you don't want to incur the
``Table.get_item`` call to get it, you can call ``Table.delete_item`` method.::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    >>> users.delete_item(username='johndoe', last_name='Doe')
    True


Batch Writing
-------------

If you're loading a lot of data at a time, making use of batch writing can
both speed up the process & reduce the number of write requests made to the
service.

Batch writing involves wrapping the calls you want batched in a context manager.
The context manager imitates the ``Table.put_item`` & ``Table.delete_item``
APIs. Getting & using the context manager looks like::

    >>> import time
    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users')

    >>> with users.batch_write() as batch:
    ...     batch.put_item(data={
    ...         'username': 'anotherdoe',
    ...         'first_name': 'Another',
    ...         'last_name': 'Doe',
    ...         'date_joined': int(time.time()),
    ...     })
    ...     batch.put_item(data={
    ...         'username': 'joebloggs',
    ...         'first_name': 'Joe',
    ...         'last_name': 'Bloggs',
    ...         'date_joined': int(time.time()),
    ...     })
    ...     batch.delete_item(username='janedoe', last_name='Doe')

However, there are some limitations on what you can do within the context
manager.

* It can't read data at all or do batch any other operations.
* You can't put & delete the same data within a batch request.

.. note::

    Additionally, the context manager can only batch 25 items at a time for a
    request (this is a DynamoDB limitation). It is handled for you so you can
    keep writing additional items, but you should be aware that 100 ``put_item``
    calls is 4 batch requests, not 1.


Querying
--------

.. warning::

    The ``Table`` object has both a ``query`` & a ``query_2`` method. If you
    are writing new code, **DO NOT** use ``Table.query``. It presents results
    in an incorrect order than expected & is strictly present for
    backward-compatibility.

Manually fetching out each item by itself isn't tenable for large datasets.
To cope with fetching many records, you can either perform a standard query,
query via a local secondary index or scan the entire table.

A standard query typically gets run against a hash+range key combination.
Filter parameters are passed as kwargs & use a ``__`` to separate the fieldname
from the operator being used to filter the value.

In terms of querying, our original schema is less than optimal. For the
following examples, we'll be using the following table setup::

    >>> from boto.dynamodb2.fields import HashKey, RangeKey, GlobalAllIndex
    >>> from boto.dynamodb2.table import Table
    >>> from boto.dynamodb2.types import NUMBER
    >>> import time
    >>> users = Table.create('users2', schema=[
    ...     HashKey('account_type'),
    ...     RangeKey('last_name'),
    ... ], throughput={
    ...     'read': 5,
    ...     'write': 15,
    ... }, global_indexes=[
    ...     GlobalAllIndex('DateJoinedIndex', parts=[
    ...         HashKey('account_type'),
    ...         RangeKey('date_joined', data_type=NUMBER),
    ...     ],
    ...     throughput={
    ...         'read': 1,
    ...         'write': 1,
    ...     }),
    ... ])

And the following data::

    >>> with users.batch_write() as batch:
    ...     batch.put_item(data={
    ...         'account_type': 'standard_user',
    ...         'first_name': 'John',
    ...         'last_name': 'Doe',
    ...         'is_owner': True,
    ...         'email': True,
    ...         'date_joined': int(time.time()) - (60*60*2),
    ...     })
    ...     batch.put_item(data={
    ...         'account_type': 'standard_user',
    ...         'first_name': 'Jane',
    ...         'last_name': 'Doering',
    ...         'date_joined': int(time.time()) - 2,
    ...     })
    ...     batch.put_item(data={
    ...         'account_type': 'standard_user',
    ...         'first_name': 'Bob',
    ...         'last_name': 'Doerr',
    ...         'date_joined': int(time.time()) - (60*60*3),
    ...     })
    ...     batch.put_item(data={
    ...         'account_type': 'super_user',
    ...         'first_name': 'Alice',
    ...         'last_name': 'Liddel',
    ...         'is_owner': True,
    ...         'email': True,
    ...         'date_joined': int(time.time()) - 1,
    ...     })

When executing the query, you get an iterable back that contains your results.
These results may be spread over multiple requests as DynamoDB paginates them.
This is done transparently, but you should be aware it may take more than one
request.

To run a query for last names starting with the letter "D"::

    >>> names_with_d = users.query_2(
    ...     account_type__eq='standard_user',
    ...     last_name__beginswith='D'
    ... )

    >>> for user in names_with_d:
    ...     print user['first_name']
    'John'
    'Jane'
    'Bob'

You can also reverse results (``reverse=True``) as well as limiting them
(``limit=2``)::

    >>> rev_with_d = users.query_2(
    ...     account_type__eq='standard_user',
    ...     last_name__beginswith='D',
    ...     reverse=True,
    ...     limit=2
    ... )

    >>> for user in rev_with_d:
    ...     print user['first_name']
    'Bob'
    'Jane'

You can also run queries against the local secondary indexes. Simply provide
the index name (``index='DateJoinedIndex'``) & filter parameters against its
fields::

    # Users within the last hour.
    >>> recent = users.query_2(
    ...     account_type__eq='standard_user',
    ...     date_joined__gte=time.time() - (60 * 60),
    ...     index='DateJoinedIndex'
    ... )

    >>> for user in recent:
    ...     print user['first_name']
    'Jane'

By default, DynamoDB can return a large amount of data per-request (up to 1Mb
of data). To prevent these requests from drowning other smaller gets, you can
specify a smaller page size via the ``max_page_size`` argument to
``Table.query_2`` & ``Table.scan``. Doing so looks like::

    # Small pages yield faster responses & less potential of drowning other
    # requests.
    >>> all_users = users.query_2(
    ...     account_type__eq='standard_user',
    ...     date_joined__gte=0,
    ...     index='DateJoinedIndex',
    ...     max_page_size=10
    ... )

    # Usage is the same, but now many smaller requests are done.
    >>> for user in all_users:
    ...     print user['first_name']
    'Bob'
    'John'
    'Jane'

Finally, if you need to query on data that's not in either a key or in an
index, you can run a ``Table.scan`` across the whole table, which accepts a
similar but expanded set of filters. If you're familiar with the Map/Reduce
concept, this is akin to what DynamoDB does.

.. warning::

    Scans are eventually consistent & run over the entire table, so
    relatively speaking, they're more expensive than plain queries or queries
    against an LSI.

An example scan of all records in the table looks like::

    >>> all_users = users.scan()

Filtering a scan looks like::

    >>> owners_with_emails = users.scan(
    ...     is_owner__eq=True,
    ...     email__null=False,
    ... )

    >>> for user in owners_with_emails:
    ...     print user['first_name']
    'John'
    'Alice'


The ``ResultSet``
~~~~~~~~~~~~~~~~~

Both ``Table.query_2`` & ``Table.scan`` return an object called ``ResultSet``.
It's a lazily-evaluated object that uses the `Iterator protocol`_. It delays
your queries until you request the next item in the result set.

Typical use is simply a standard ``for`` to iterate over the results::

    >>> result_set = users.scan()
    >>> for user in result_set:
    ...     print user['first_name']
    'John'
    'Jane'
    'Bob'
    'Alice'

However, this throws away results as it fetches more data. As a result, you
can't index it like a ``list``::

    >>> len(result_set)
    TypeError: object of type 'ResultSet' has no len()

Because it does this, if you need to loop over your results more than once (or
do things like negative indexing, length checks, etc.), you should wrap it in
a call to ``list()``. Ex.::

    >>> result_set = users.scan()
    >>> all_users = list(result_set)
    # Slice it for every other user.
    >>> for user in all_users[::2]:
    ...     print user['first_name']
    'John'
    'Bob'

.. warning::

    Wrapping calls like the above in ``list(...)`` **WILL** cause it to evaluate
    the **ENTIRE** potentially large data set.

    Appropriate use of the ``limit=...`` kwarg to ``Table.query_2`` &
    ``Table.scan`` calls are **VERY** important should you chose to do this.

    Alternatively, you can build your own list, using ``for`` on the
    ``ResultSet`` to lazily build the list (& potentially stop early).

.. _`Iterator protocol`: http://docs.python.org/2/library/stdtypes.html#iterator-types


Parallel Scan
-------------

DynamoDB also includes a feature called "Parallel Scan", which allows you
to make use of **extra** read capacity to divide up your result set & scan
an entire table faster.

This does require extra code on the user's part & you should ensure that
you need the speed boost, have enough data to justify it and have the extra
capacity to read it without impacting other queries/scans.

To run it, you should pick the ``total_segments`` to use, which is an integer
representing the number of temporary partitions you'd divide your table into.
You then need to spin up a thread/process for each one, giving each
thread/process a ``segment``, which is a zero-based integer of the segment
you'd like to scan.

An example of using parallel scan to send out email to all users might look
something like::

    #!/usr/bin/env python
    import threading

    import boto.ses
    import boto.dynamodb2
    from boto.dynamodb2.table import Table


    AWS_ACCESS_KEY_ID = '<YOUR_AWS_KEY_ID>'
    AWS_SECRET_ACCESS_KEY = '<YOUR_AWS_SECRET_KEY>'
    APPROVED_EMAIL = 'some@address.com'


    def send_email(email):
        # Using Amazon's Simple Email Service, send an email to a given
        # email address. You must already have an email you've verified with
        # AWS before this will work.
        conn = boto.ses.connect_to_region(
            'us-east-1',
            aws_access_key_id=AWS_ACCESS_KEY_ID,
            aws_secret_access_key=AWS_SECRET_ACCESS_KEY
        )
        conn.send_email(
            APPROVED_EMAIL,
            "[OurSite] New feature alert!",
            "We've got some exciting news! We added a new feature to...",
            [email]
        )


    def process_segment(segment=0, total_segments=10):
        # This method/function is executed in each thread, each getting its
        # own segment to process through.
        conn = boto.dynamodb2.connect_to_region(
            'us-east-1',
            aws_access_key_id=AWS_ACCESS_KEY_ID,
            aws_secret_access_key=AWS_SECRET_ACCESS_KEY
        )
        table = Table('users', connection=conn)

        # We pass in the segment & total_segments to scan here.
        for user in table.scan(segment=segment, total_segments=total_segments):
            send_email(user['email'])


    def send_all_emails():
        pool = []
        # We're choosing to divide the table in 3, then...
        pool_size = 3

        # ...spinning up a thread for each segment.
        for i in range(pool_size):
            worker = threading.Thread(
                target=process_segment,
                kwargs={
                    'segment': i,
                    'total_segments': pool_size,
                }
            )
            pool.append(worker)
            # We start them to let them start scanning & consuming their
            # assigned segment.
            worker.start()

        # Finally, we wait for each to finish.
        for thread in pool:
            thread.join()


    if __name__ == '__main__':
        send_all_emails()


Batch Reading
-------------

Similar to batch writing, batch reading can also help reduce the number of
API requests necessary to access a large number of items. The
``Table.batch_get`` method takes a list (or any sliceable collection) of keys
& fetches all of them, presented as an iterator interface.

This is done lazily, so if you never iterate over the results, no requests are
executed. Additionally, if you only iterate over part of the set, the minimum
number of calls are made to fetch those results (typically max 100 per
response).

Example::

    >>> from boto.dynamodb2.table import Table
    >>> users = Table('users2')

    # No request yet.
    >>> many_users = users.batch_get(keys=[
    ...     {'account_type': 'standard_user', 'last_name': 'Doe'},
    ...     {'account_type': 'standard_user', 'last_name': 'Doering'},
    ...     {'account_type': 'super_user', 'last_name': 'Liddel'},
    ... ])

    # Now the request is performed, requesting all five in one request.
    >>> for user in many_users:
    ...     print user['first_name']
    'Alice'
    'John'
    'Jane'


Deleting a Table
----------------

Deleting a table is a simple exercise. When you no longer need a table, simply
run::

    >>> users.delete()


DynamoDB Local
--------------

`Amazon DynamoDB Local`_ is a utility which can be used to mock DynamoDB
during development. Connecting to a running DynamoDB Local server is easy::

    #!/usr/bin/env python
    from boto.dynamodb2.layer1 import DynamoDBConnection


    # Connect to DynamoDB Local
    conn = DynamoDBConnection(
        host='localhost',
        port=8000,
        aws_access_key_id='anything',
        aws_secret_access_key='anything',
        is_secure=False)

    # List all local tables
    tables = conn.list_tables()


.. _`Amazon DynamoDB Local`: http://docs.aws.amazon.com/amazondynamodb/latest/developerguide/Tools.html


Next Steps
----------

You can find additional information about other calls & parameter options
in the :doc:`API docs <ref/dynamodb2>`.
