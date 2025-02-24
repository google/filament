.. _cloudfront_tut:

==========
CloudFront
==========

This new boto module provides an interface to Amazon's Content Service,
CloudFront.

.. warning::

    This module is not well tested.  Paging of distributions is not yet
    supported. CNAME support is completely untested.  Use with caution.
    Feedback and bug reports are greatly appreciated.

Creating a CloudFront connection
--------------------------------
If you've placed your credentials in your ``$HOME/.boto`` config file then you
can simply create a CloudFront connection using::

    >>> import boto
    >>> c = boto.connect_cloudfront()

If you do not have this file you will need to specify your AWS access key and
secret access key::

    >>> import boto
    >>> c = boto.connect_cloudfront('your-aws-access-key-id', 'your-aws-secret-access-key')

Working with CloudFront Distributions
-------------------------------------
Create a new :class:`boto.cloudfront.distribution.Distribution`::

    >>> origin = boto.cloudfront.origin.S3Origin('mybucket.s3.amazonaws.com')
    >>> distro = c.create_distribution(origin=origin, enabled=False, comment='My new Distribution')
    >>> d.domain_name
    u'd2oxf3980lnb8l.cloudfront.net'
    >>> d.id
    u'ECH69MOIW7613'
    >>> d.status
    u'InProgress'
    >>> d.config.comment
    u'My new distribution'
    >>> d.config.origin
    <S3Origin: mybucket.s3.amazonaws.com>
    >>> d.config.caller_reference
    u'31b8d9cf-a623-4a28-b062-a91856fac6d0'
    >>> d.config.enabled
    False

Note that a new caller reference is created automatically, using
uuid.uuid4(). The :class:`boto.cloudfront.distribution.Distribution`,
:class:`boto.cloudfront.distribution.DistributionConfig` and
:class:`boto.cloudfront.distribution.DistributionSummary` objects are defined
in the :mod:`boto.cloudfront.distribution` module.

To get a listing of all current distributions::

    >>> rs = c.get_all_distributions()
    >>> rs
    [<boto.cloudfront.distribution.DistributionSummary instance at 0xe8d4e0>,
     <boto.cloudfront.distribution.DistributionSummary instance at 0xe8d788>]

This returns a list of :class:`boto.cloudfront.distribution.DistributionSummary`
objects. Note that paging is not yet supported! To get a
:class:`boto.cloudfront.distribution.DistributionObject` from a
:class:`boto.cloudfront.distribution.DistributionSummary` object::

    >>> ds = rs[1]
    >>> distro = ds.get_distribution()
    >>> distro.domain_name
    u'd2oxf3980lnb8l.cloudfront.net'

To change a property of a distribution object::

    >>> distro.comment
    u'My new distribution'
    >>> distro.update(comment='This is a much better comment')
    >>> distro.comment
    'This is a much better comment'

You can also enable/disable a distribution using the following
convenience methods::

    >>> distro.enable()  # just calls distro.update(enabled=True)

or::

    >>> distro.disable()  # just calls distro.update(enabled=False)

The only attributes that can be updated for a Distribution are
comment, enabled and cnames.

To delete a :class:`boto.cloudfront.distribution.Distribution`::

    >>> distro.delete()

Invalidating CloudFront Distribution Paths
------------------------------------------
Invalidate a list of paths in a CloudFront distribution::

    >>> paths = ['/path/to/file1.html', '/path/to/file2.html', ...]
    >>> inval_req = c.create_invalidation_request(u'ECH69MOIW7613', paths)
    >>> print inval_req
    <InvalidationBatch: IFCT7K03VUETK>
    >>> print inval_req.id
    u'IFCT7K03VUETK'
    >>> print inval_req.paths
    [u'/path/to/file1.html', u'/path/to/file2.html', ..]

.. warning::

    Each CloudFront invalidation request can only specify up to 1000 paths. If
    you need to invalidate more than 1000 paths you will need to split up the
    paths into groups of 1000 or less and create multiple invalidation requests.

This will return a :class:`boto.cloudfront.invalidation.InvalidationBatch`
object representing the invalidation request. You can also fetch a single
invalidation request for a given distribution using
``invalidation_request_status``::

    >>> inval_req = c.invalidation_request_status(u'ECH69MOIW7613', u'IFCT7K03VUETK')
    >>> print inval_req
    <InvalidationBatch: IFCT7K03VUETK>

The first parameter is the CloudFront distribution id the request belongs to
and the second parameter is the invalidation request id.

It's also possible to get *all* invalidations for a given CloudFront
distribution::

    >>> invals = c.get_invalidation_requests(u'ECH69MOIW7613')
    >>> print invals
    <boto.cloudfront.invalidation.InvalidationListResultSet instance at 0x15d28d0>

This will return an instance of
:class:`boto.cloudfront.invalidation.InvalidationListResultSet` which is an
iterable object that contains a list of
:class:`boto.cloudfront.invalidation.InvalidationSummary` objects that describe
each invalidation request and its status::

    >>> for inval in invals:
    >>>     print 'Object: %s, ID: %s, Status: %s' % (inval, inval.id, inval.status)
    Object: <InvalidationSummary: ICXT2K02SUETK>, ID: ICXT2K02SUETK, Status: Completed
    Object: <InvalidationSummary: ITV9SV0PDNY1Y>, ID: ITV9SV0PDNY1Y, Status: Completed
    Object: <InvalidationSummary: I1X3F6N0PLGJN5>, ID: I1X3F6N0PLGJN5, Status: Completed
    Object: <InvalidationSummary: I1F3G9N0ZLGKN2>, ID: I1F3G9N0ZLGKN2, Status: Completed
    ...

Simply iterating over the
:class:`boto.cloudfront.invalidation.InvalidationListResultSet` object will
automatically paginate the results on-the-fly as needed by repeatedly
requesting more results from CloudFront until there are none left.

If you wish to paginate the results manually you can do so by specifying the
``max_items`` option when calling ``get_invalidation_requests``::

    >>> invals = c.get_invalidation_requests(u'ECH69MOIW7613', max_items=2)
    >>> print len(list(invals))
    2
    >>> for inval in invals:
    >>>     print 'Object: %s, ID: %s, Status: %s' % (inval, inval.id, inval.status)
    Object: <InvalidationSummary: ICXT2K02SUETK>, ID: ICXT2K02SUETK, Status: Completed
    Object: <InvalidationSummary: ITV9SV0PDNY1Y>, ID: ITV9SV0PDNY1Y, Status: Completed

In this case, iterating over the
:class:`boto.cloudfront.invalidation.InvalidationListResultSet` object will
*only* make a single request to CloudFront and *only* ``max_items``
invalidation requests are returned by the iterator. To get the next "page" of
results pass the ``next_marker`` attribute of the previous
:class:`boto.cloudfront.invalidation.InvalidationListResultSet` object as the
``marker`` option to the next call to ``get_invalidation_requests``::

    >>> invals = c.get_invalidation_requests(u'ECH69MOIW7613', max_items=10, marker=invals.next_marker)
    >>> print len(list(invals))
    2
    >>> for inval in invals:
    >>>     print 'Object: %s, ID: %s, Status: %s' % (inval, inval.id, inval.status)
    Object: <InvalidationSummary: I1X3F6N0PLGJN5>, ID: I1X3F6N0PLGJN5, Status: Completed
    Object: <InvalidationSummary: I1F3G9N0ZLGKN2>, ID: I1F3G9N0ZLGKN2, Status: Completed

You can get the :class:`boto.cloudfront.invalidation.InvalidationBatch` object
representing the invalidation request pointed to by a
:class:`boto.cloudfront.invalidation.InvalidationSummary` object using::

    >>> inval_req = inval.get_invalidation_request()
    >>> print inval_req
    <InvalidationBatch: IFCT7K03VUETK>

Similarly you can get the parent
:class:`boto.cloudfront.distribution.Distribution` object for the invalidation
request from a :class:`boto.cloudfront.invalidation.InvalidationSummary` object
using::

    >>> dist = inval.get_distribution()
    >>> print dist
    <boto.cloudfront.distribution.Distribution instance at 0x304a7e8>
