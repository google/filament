.. _s3_tut:

======================================
An Introduction to boto's S3 interface
======================================

This tutorial focuses on the boto interface to the Simple Storage Service
from Amazon Web Services.  This tutorial assumes that you have already
downloaded and installed boto.

Creating a Connection
---------------------
The first step in accessing S3 is to create a connection to the service.
There are two ways to do this in boto.  The first is:

>>> from boto.s3.connection import S3Connection
>>> conn = S3Connection('<aws access key>', '<aws secret key>')

At this point the variable conn will point to an S3Connection object.  In
this example, the AWS access key and AWS secret key are passed in to the
method explicitly.  Alternatively, you can set the environment variables:

* `AWS_ACCESS_KEY_ID` - Your AWS Access Key ID
* `AWS_SECRET_ACCESS_KEY` - Your AWS Secret Access Key

and then call the constructor without any arguments, like this:

>>> conn = S3Connection()

There is also a shortcut function in the boto package, called connect_s3
that may provide a slightly easier means of creating a connection::

    >>> import boto
    >>> conn = boto.connect_s3()

In either case, conn will point to an S3Connection object which we will
use throughout the remainder of this tutorial.

Creating a Bucket
-----------------

Once you have a connection established with S3, you will probably want to
create a bucket.  A bucket is a container used to store key/value pairs
in S3.  A bucket can hold an unlimited amount of data so you could potentially
have just one bucket in S3 for all of your information.  Or, you could create
separate buckets for different types of data.  You can figure all of that out
later, first let's just create a bucket.  That can be accomplished like this::

    >>> bucket = conn.create_bucket('mybucket')
    Traceback (most recent call last):
      File "<stdin>", line 1, in ?
      File "boto/connection.py", line 285, in create_bucket
        raise S3CreateError(response.status, response.reason)
    boto.exception.S3CreateError: S3Error[409]: Conflict

Whoa.  What happened there?  Well, the thing you have to know about
buckets is that they are kind of like domain names.  It's one flat name
space that everyone who uses S3 shares.  So, someone has already create
a bucket called "mybucket" in S3 and that means no one else can grab that
bucket name.  So, you have to come up with a name that hasn't been taken yet.
For example, something that uses a unique string as a prefix.  Your
AWS_ACCESS_KEY (NOT YOUR SECRET KEY!) could work but I'll leave it to
your imagination to come up with something.  I'll just assume that you
found an acceptable name.

The create_bucket method will create the requested bucket if it does not
exist or will return the existing bucket if it does exist.

Creating a Bucket In Another Location
-------------------------------------

The example above assumes that you want to create a bucket in the
standard US region.  However, it is possible to create buckets in
other locations.  To do so, first import the Location object from the
boto.s3.connection module, like this::

    >>> from boto.s3.connection import Location
    >>> print '\n'.join(i for i in dir(Location) if i[0].isupper())
    APNortheast
    APSoutheast
    APSoutheast2
    DEFAULT
    EU
    EUCentral1
    SAEast
    USWest
    USWest2

As you can see, the Location object defines a number of possible locations.  By
default, the location is the empty string which is interpreted as the US
Classic Region, the original S3 region.  However, by specifying another
location at the time the bucket is created, you can instruct S3 to create the
bucket in that location.  For example::

    >>> conn.create_bucket('mybucket', location=Location.EU)

will create the bucket in the EU region (assuming the name is available).

Storing Data
------------

Once you have a bucket, presumably you will want to store some data
in it.  S3 doesn't care what kind of information you store in your objects
or what format you use to store it.  All you need is a key that is unique
within your bucket.

The Key object is used in boto to keep track of data stored in S3.  To store
new data in S3, start by creating a new Key object::

    >>> from boto.s3.key import Key
    >>> k = Key(bucket)
    >>> k.key = 'foobar'
    >>> k.set_contents_from_string('This is a test of S3')

The net effect of these statements is to create a new object in S3 with a
key of "foobar" and a value of "This is a test of S3".  To validate that
this worked, quit out of the interpreter and start it up again.  Then::

    >>> import boto
    >>> c = boto.connect_s3()
    >>> b = c.get_bucket('mybucket') # substitute your bucket name here
    >>> from boto.s3.key import Key
    >>> k = Key(b)
    >>> k.key = 'foobar'
    >>> k.get_contents_as_string()
    'This is a test of S3'

So, we can definitely store and retrieve strings.  A more interesting
example may be to store the contents of a local file in S3 and then retrieve
the contents to another local file.

::

    >>> k = Key(b)
    >>> k.key = 'myfile'
    >>> k.set_contents_from_filename('foo.jpg')
    >>> k.get_contents_to_filename('bar.jpg')

There are a couple of things to note about this.  When you send data to
S3 from a file or filename, boto will attempt to determine the correct
mime type for that file and send it as a Content-Type header.  The boto
package uses the standard mimetypes package in Python to do the mime type
guessing.  The other thing to note is that boto does stream the content
to and from S3 so you should be able to send and receive large files without
any problem.

When fetching a key that already exists, you have two options. If you're
uncertain whether a key exists (or if you need the metadata set on it, you can
call ``Bucket.get_key(key_name_here)``. However, if you're sure a key already
exists within a bucket, you can skip the check for a key on the server.

::

    >>> import boto
    >>> c = boto.connect_s3()
    >>> b = c.get_bucket('mybucket') # substitute your bucket name here

    # Will hit the API to check if it exists.
    >>> possible_key = b.get_key('mykey') # substitute your key name here

    # Won't hit the API.
    >>> key_we_know_is_there = b.get_key('mykey', validate=False)


Storing Large Data
------------------

At times the data you may want to store will be hundreds of megabytes or
more in size. S3 allows you to split such files into smaller components.
You upload each component in turn and then S3 combines them into the final
object. While this is fairly straightforward, it requires a few extra steps
to be taken. The example below makes use of the FileChunkIO module, so
``pip install FileChunkIO`` if it isn't already installed.

::

    >>> import math, os
    >>> import boto
    >>> from filechunkio import FileChunkIO

    # Connect to S3
    >>> c = boto.connect_s3()
    >>> b = c.get_bucket('mybucket')

    # Get file info
    >>> source_path = 'path/to/your/file.ext'
    >>> source_size = os.stat(source_path).st_size

    # Create a multipart upload request
    >>> mp = b.initiate_multipart_upload(os.path.basename(source_path))

    # Use a chunk size of 50 MiB (feel free to change this)
    >>> chunk_size = 52428800
    >>> chunk_count = int(math.ceil(source_size / float(chunk_size)))

    # Send the file parts, using FileChunkIO to create a file-like object
    # that points to a certain byte range within the original file. We
    # set bytes to never exceed the original file size.
    >>> for i in range(chunk_count):
    >>>     offset = chunk_size * i
    >>>     bytes = min(chunk_size, source_size - offset)
    >>>     with FileChunkIO(source_path, 'r', offset=offset,
                             bytes=bytes) as fp:
    >>>         mp.upload_part_from_file(fp, part_num=i + 1)

    # Finish the upload
    >>> mp.complete_upload()

It is also possible to upload the parts in parallel using threads. The
``s3put`` script that ships with Boto provides an example of doing so
using a thread pool.

Note that if you forget to call either ``mp.complete_upload()`` or
``mp.cancel_upload()`` you will be left with an incomplete upload and
charged for the storage consumed by the uploaded parts. A call to
``bucket.get_all_multipart_uploads()`` can help to show lost multipart
upload parts.


Accessing A Bucket
------------------

Once a bucket exists, you can access it by getting the bucket. For example::

    >>> mybucket = conn.get_bucket('mybucket') # Substitute in your bucket name
    >>> mybucket.list()
    ...listing of keys in the bucket...

By default, this method tries to validate the bucket's existence. You can
override this behavior by passing ``validate=False``.::

    >>> nonexistent = conn.get_bucket('i-dont-exist-at-all', validate=False)

.. versionchanged:: 2.25.0
.. warning::

    If ``validate=False`` is passed, no request is made to the service (no
    charge/communication delay). This is only safe to do if you are **sure**
    the bucket exists.

    If the default ``validate=True`` is passed, a request is made to the
    service to ensure the bucket exists. Prior to Boto v2.25.0, this fetched
    a list of keys (but with a max limit set to ``0``, always returning an empty
    list) in the bucket (& included better error messages), at an
    increased expense. As of Boto v2.25.0, this now performs a HEAD request
    (less expensive but worse error messages).

    If you were relying on parsing the error message before, you should call
    something like::

        bucket = conn.get_bucket('<bucket_name>', validate=False)
        bucket.get_all_keys(maxkeys=0)

If the bucket does not exist, a ``S3ResponseError`` will commonly be thrown. If
you'd rather not deal with any exceptions, you can use the ``lookup`` method.::

    >>> nonexistent = conn.lookup('i-dont-exist-at-all')
    >>> if nonexistent is None:
    ...     print "No such bucket!"
    ...
    No such bucket!


Deleting A Bucket
-----------------

Removing a bucket can be done using the ``delete_bucket`` method. For example::

    >>> conn.delete_bucket('mybucket') # Substitute in your bucket name

The bucket must be empty of keys or this call will fail & an exception will be
raised. You can remove a non-empty bucket by doing something like::

    >>> full_bucket = conn.get_bucket('bucket-to-delete')
    # It's full of keys. Delete them all.
    >>> for key in full_bucket.list():
    ...     key.delete()
    ...
    # The bucket is empty now. Delete it.
    >>> conn.delete_bucket('bucket-to-delete')

.. warning::

    This method can cause data loss! Be very careful when using it.

    Additionally, be aware that using the above method for removing all keys
    and deleting the bucket involves a request for each key. As such, it's not
    particularly fast & is very chatty.

Listing All Available Buckets
-----------------------------
In addition to accessing specific buckets via the create_bucket method
you can also get a list of all available buckets that you have created.

::

    >>> rs = conn.get_all_buckets()

This returns a ResultSet object (see the SQS Tutorial for more info on
ResultSet objects).  The ResultSet can be used as a sequence or list type
object to retrieve Bucket objects.

::

    >>> len(rs)
    11
    >>> for b in rs:
    ... print b.name
    ...
    <listing of available buckets>
    >>> b = rs[0]

Setting / Getting the Access Control List for Buckets and Keys
--------------------------------------------------------------
The S3 service provides the ability to control access to buckets and keys
within s3 via the Access Control List (ACL) associated with each object in
S3.  There are two ways to set the ACL for an object:

1. Create a custom ACL that grants specific rights to specific users.  At the
   moment, the users that are specified within grants have to be registered
   users of Amazon Web Services so this isn't as useful or as general as it
   could be.

2. Use a "canned" access control policy.  There are four canned policies
   defined:

   a. private: Owner gets FULL_CONTROL.  No one else has any access rights.
   b. public-read: Owners gets FULL_CONTROL and the anonymous principal is granted READ access.
   c. public-read-write: Owner gets FULL_CONTROL and the anonymous principal is granted READ and WRITE access.
   d. authenticated-read: Owner gets FULL_CONTROL and any principal authenticated as a registered Amazon S3 user is granted READ access.

To set a canned ACL for a bucket, use the set_acl method of the Bucket object.
The argument passed to this method must be one of the four permissable
canned policies named in the list CannedACLStrings contained in acl.py.
For example, to make a bucket readable by anyone:

>>> b.set_acl('public-read')

You can also set the ACL for Key objects, either by passing an additional
argument to the above method:

>>> b.set_acl('public-read', 'foobar')

where 'foobar' is the key of some object within the bucket b or you can
call the set_acl method of the Key object:

>>> k.set_acl('public-read')

You can also retrieve the current ACL for a Bucket or Key object using the
get_acl object.  This method parses the AccessControlPolicy response sent
by S3 and creates a set of Python objects that represent the ACL.

::

    >>> acp = b.get_acl()
    >>> acp
    <boto.acl.Policy instance at 0x2e6940>
    >>> acp.acl
    <boto.acl.ACL instance at 0x2e69e0>
    >>> acp.acl.grants
    [<boto.acl.Grant instance at 0x2e6a08>]
    >>> for grant in acp.acl.grants:
    ...   print grant.permission, grant.display_name, grant.email_address, grant.id
    ...
    FULL_CONTROL <boto.user.User instance at 0x2e6a30>

The Python objects representing the ACL can be found in the acl.py module
of boto.

Both the Bucket object and the Key object also provide shortcut
methods to simplify the process of granting individuals specific
access.  For example, if you want to grant an individual user READ
access to a particular object in S3 you could do the following::

    >>> key = b.lookup('mykeytoshare')
    >>> key.add_email_grant('READ', 'foo@bar.com')

The email address provided should be the one associated with the users
AWS account.  There is a similar method called add_user_grant that accepts the
canonical id of the user rather than the email address.

Setting/Getting Metadata Values on Key Objects
----------------------------------------------
S3 allows arbitrary user metadata to be assigned to objects within a bucket.
To take advantage of this S3 feature, you should use the set_metadata and
get_metadata methods of the Key object to set and retrieve metadata associated
with an S3 object.  For example::

    >>> k = Key(b)
    >>> k.key = 'has_metadata'
    >>> k.set_metadata('meta1', 'This is the first metadata value')
    >>> k.set_metadata('meta2', 'This is the second metadata value')
    >>> k.set_contents_from_filename('foo.txt')

This code associates two metadata key/value pairs with the Key k.  To retrieve
those values later::

    >>> k = b.get_key('has_metadata')
    >>> k.get_metadata('meta1')
    'This is the first metadata value'
    >>> k.get_metadata('meta2')
    'This is the second metadata value'
    >>>

Setting/Getting/Deleting CORS Configuration on a Bucket
-------------------------------------------------------

Cross-origin resource sharing (CORS) defines a way for client web
applications that are loaded in one domain to interact with resources
in a different domain. With CORS support in Amazon S3, you can build
rich client-side web applications with Amazon S3 and selectively allow
cross-origin access to your Amazon S3 resources.

To create a CORS configuration and associate it with a bucket::

    >>> from boto.s3.cors import CORSConfiguration
    >>> cors_cfg = CORSConfiguration()
    >>> cors_cfg.add_rule(['PUT', 'POST', 'DELETE'], 'https://www.example.com', allowed_header='*', max_age_seconds=3000, expose_header='x-amz-server-side-encryption')
    >>> cors_cfg.add_rule('GET', '*')

The above code creates a CORS configuration object with two rules.

* The first rule allows cross-origin PUT, POST, and DELETE requests from
  the https://www.example.com/ origin.  The rule also allows all headers
  in preflight OPTIONS request through the Access-Control-Request-Headers
  header.  In response to any preflight OPTIONS request, Amazon S3 will
  return any requested headers.
* The second rule allows cross-origin GET requests from all origins.

To associate this configuration with a bucket::

    >>> import boto
    >>> c = boto.connect_s3()
    >>> bucket = c.lookup('mybucket')
    >>> bucket.set_cors(cors_cfg)

To retrieve the CORS configuration associated with a bucket::

    >>> cors_cfg = bucket.get_cors()

And, finally, to delete all CORS configurations from a bucket::

    >>> bucket.delete_cors()

Transitioning Objects
--------------------------------

S3 buckets support transitioning objects to various storage classes. This is
done using lifecycle policies. You can currently transitions objects to 
Infrequent Access, Glacier, or just plain Expire. All of these options are 
capable of being applied after a number of days or after a given date.
Lifecycle configurations are assigned to buckets and require these parameters:

* The object prefix that identifies the objects you are targeting. (or none)
* The action you want S3 to perform on the identified objects.
* The date or number of days when you want S3 to perform these actions.

For example, given a bucket ``s3-lifecycle-boto-demo``, we can first retrieve the
bucket::

    >>> import boto
    >>> c = boto.connect_s3()
    >>> bucket = c.get_bucket('s3-lifecycle-boto-demo')

Then we can create a lifecycle object.  In our example, we want all objects
under ``logs/*`` to transition to Standard IA 30 days after the object is created,
glacier 90 days after creation, and be deleted 120 days after creation.

::

    >>> from boto.s3.lifecycle import Lifecycle, Transitions, Rule
    >>> transitions = Transitions()
    >>> transitions.add_transition(days=30, storage_class='STANDARD_IA')
    >>> transitions.add_transition(days=90, storage_class='GLACIER')
    >>> expiration = Expiration(days=120)
    >>> rule = Rule(id='ruleid', prefix='logs/', status='Enabled', expiration=expiration, transition=transitions)
    >>> lifecycle = Lifecycle()
    >>> lifecycle.append(rule)

.. note::

  For API docs for the lifecycle objects, see :py:mod:`boto.s3.lifecycle`

We can now configure the bucket with this lifecycle policy::

    >>> bucket.configure_lifecycle(lifecycle)
    True

You can also retrieve the current lifecycle policy for the bucket::

    >>> current = bucket.get_lifecycle_config()
    >>> print current[0].transition
    >>> print current[0].expiration
    [<Transition: in: 90 days, GLACIER>, <Transition: in: 30 days, STANDARD_IA>]
    <Expiration: in: 120 days>

Note: We have deprecated directly accessing transition properties from the lifecycle
object. You must index into the transition array first.

When an object transitions, the storage class will be
updated.  This can be seen when you **list** the objects in a bucket::

    >>> for key in bucket.list():
    ...   print key, key.storage_class
    ...
    <Key: s3-lifecycle-boto-demo,logs/testlog1.log> STANDARD_IA
    <Key: s3-lifecycle-boto-demo,logs/testlog2.log> GLACIER

You can also use the prefix argument to the ``bucket.list`` method::

    >>> print list(b.list(prefix='logs/testlog1.log'))[0].storage_class
    >>> print list(b.list(prefix='logs/testlog2.log'))[0].storage_class
    u'STANDARD_IA'
    u'GLACIER'


Restoring Objects from Glacier
------------------------------

Once an object has been transitioned to Glacier, you can restore the object
back to S3.  To do so, you can use the :py:meth:`boto.s3.key.Key.restore`
method of the key object.
The ``restore`` method takes an integer that specifies the number of days
to keep the object in S3.

::

    >>> import boto
    >>> c = boto.connect_s3()
    >>> bucket = c.get_bucket('s3-glacier-boto-demo')
    >>> key = bucket.get_key('logs/testlog1.log')
    >>> key.restore(days=5)

It takes about 4 hours for a restore operation to make a copy of the archive
available for you to access.  While the object is being restored, the
``ongoing_restore`` attribute will be set to ``True``::


    >>> key = bucket.get_key('logs/testlog1.log')
    >>> print key.ongoing_restore
    True

When the restore is finished, this value will be ``False`` and the expiry
date of the object will be non ``None``::

    >>> key = bucket.get_key('logs/testlog1.log')
    >>> print key.ongoing_restore
    False
    >>> print key.expiry_date
    "Fri, 21 Dec 2012 00:00:00 GMT"


.. note:: If there is no restore operation either in progress or completed,
  the ``ongoing_restore`` attribute will be ``None``.

Once the object is restored you can then download the contents::

    >>> key.get_contents_to_filename('testlog1.log')
