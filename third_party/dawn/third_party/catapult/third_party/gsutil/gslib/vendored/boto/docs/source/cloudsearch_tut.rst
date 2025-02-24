.. cloudsearch_tut:

===============================================
An Introduction to boto's Cloudsearch interface
===============================================

This tutorial focuses on the boto interface to AWS' Cloudsearch_. This tutorial
assumes that you have boto already downloaded and installed.

.. _Cloudsearch: http://aws.amazon.com/cloudsearch/

Creating a Connection
---------------------
The first step in accessing CloudSearch is to create a connection to the service.

The recommended method of doing this is as follows::

    >>> import boto.cloudsearch
    >>> conn = boto.cloudsearch.connect_to_region("us-west-2",
    ...             aws_access_key_id='<aws access key>',
    ...             aws_secret_access_key='<aws secret key>')

At this point, the variable conn will point to a CloudSearch connection object
in the us-west-2 region. Available regions for cloudsearch can be found 
`here <http://docs.aws.amazon.com/general/latest/gr/rande.html#cloudsearch_region>`_.
In this example, the AWS access key and AWS secret key are
passed in to the method explicitly. Alternatively, you can set the environment
variables:

* `AWS_ACCESS_KEY_ID` - Your AWS Access Key ID
* `AWS_SECRET_ACCESS_KEY` - Your AWS Secret Access Key

and then simply call::

   >>> import boto.cloudsearch
   >>> conn = boto.cloudsearch.connect_to_region("us-west-2")

In either case, conn will point to the Connection object which we will use
throughout the remainder of this tutorial.

Creating a Domain
-----------------

Once you have a connection established with the CloudSearch service, you will
want to create a domain. A domain encapsulates the data that you wish to index,
as well as indexes and metadata relating to it::

    >>> from boto.cloudsearch.domain import Domain
    >>> domain = Domain(conn, conn.create_domain('demo'))

This domain can be used to control access policies, indexes, and the actual
document service, which you will use to index and search.

Setting access policies
-----------------------

Before you can connect to a document service, you need to set the correct
access properties.  For example, if you were connecting from 192.168.1.0, you
could give yourself access as follows::

    >>> our_ip = '192.168.1.0'

    >>> # Allow our IP address to access the document and search services
    >>> policy = domain.get_access_policies()
    >>> policy.allow_search_ip(our_ip)
    >>> policy.allow_doc_ip(our_ip)

You can use the :py:meth:`allow_search_ip
<boto.cloudsearch.optionstatus.ServicePoliciesStatus.allow_search_ip>` and
:py:meth:`allow_doc_ip <boto.cloudsearch.optionstatus.ServicePoliciesStatus.allow_doc_ip>`
methods to give different CIDR blocks access to searching and the document
service respectively.

Creating index fields
---------------------

Each domain can have up to twenty index fields which are indexed by the
CloudSearch service. For each index field, you will need to specify whether
it's a text or integer field, as well as optionally a default value::

    >>> # Create an 'text' index field called 'username'
    >>> uname_field = domain.create_index_field('username', 'text')

    >>> # Epoch time of when the user last did something
    >>> time_field = domain.create_index_field('last_activity',
    ...                                        'uint',
    ...                                        default=0)

It is also possible to mark an index field as a facet. Doing so allows a search
query to return categories into which results can be grouped, or to create
drill-down categories::

    >>> # But it would be neat to drill down into different countries
    >>> loc_field = domain.create_index_field('location', 'text', facet=True)

Finally, you can also mark a snippet of text as being able to be returned
directly in your search query by using the results option::

    >>> # Directly insert user snippets in our results
    >>> snippet_field = domain.create_index_field('snippet', 'text', result=True)

You can add up to 20 index fields in this manner::

    >>> follower_field = domain.create_index_field('follower_count',
    ...                                            'uint',
    ...                                            default=0)

Adding Documents to the Index
-----------------------------

Now, we can add some documents to our new search domain. First, you will need a
document service object through which queries are sent::

    >>> doc_service = domain.get_document_service()

For this example, we will use a pre-populated list of sample content for our
import. You would normally pull such data from your database or another
document store::

    >>> users = [
        {
            'id': 1,
            'username': 'dan',
            'last_activity': 1334252740,
            'follower_count': 20,
            'location': 'USA',
            'snippet': 'Dan likes watching sunsets and rock climbing',
        },
        {
            'id': 2,
            'username': 'dankosaur',
            'last_activity': 1334252904,
            'follower_count': 1,
            'location': 'UK',
            'snippet': 'Likes to dress up as a dinosaur.',
        },
        {
            'id': 3,
            'username': 'danielle',
            'last_activity': 1334252969,
            'follower_count': 100,
            'location': 'DE',
            'snippet': 'Just moved to Germany!'
        },
        {
            'id': 4,
            'username': 'daniella',
            'last_activity': 1334253279,
            'follower_count': 7,
            'location': 'USA',
            'snippet': 'Just like Dan, I like to watch a good sunset, but heights scare me.',
        }
    ]

When adding documents to our document service, we will batch them together. You
can schedule a document to be added by using the :py:meth:`add
<boto.cloudsearch.document.DocumentServiceConnection.add>` method. Whenever you are adding a
document, you must provide a unique ID, a version ID, and the actual document
to be indexed. In this case, we are using the user ID as our unique ID. The
version ID is used to determine which is the latest version of an object to be
indexed. If you wish to update a document, you must use a higher version ID. In
this case, we are using the time of the user's last activity as a version
number::

    >>> for user in users:
    >>>     doc_service.add(user['id'], user['last_activity'], user)

When you are ready to send the batched request to the document service, you can
do with the :py:meth:`commit
<boto.cloudsearch.document.DocumentServiceConnection.commit>` method. Note that
cloudsearch will charge per 1000 batch uploads. Each batch upload must be under
5MB::

    >>> result = doc_service.commit()

The result is an instance of :py:class:`CommitResponse
<boto.cloudsearch.document.CommitResponse>` which will make the plain
dictionary response a nice object (ie result.adds, result.deletes) and raise an
exception for us if all of our documents weren't actually committed.

If you wish to use the same document service connection after a commit,
you must use :py:meth:`clear_sdf
<boto.cloudsearch.document.DocumentServiceConnection.clear_sdf>` to clear its
internal cache.

Searching Documents
-------------------

Now, let's try performing a search. First, we will need a
SearchServiceConnection::

    >>> search_service = domain.get_search_service()

A standard search will return documents which contain the exact words being
searched for::

    >>> results = search_service.search(q="dan")
    >>> results.hits
    2
    >>> map(lambda x: x['id'], results)
    [u'1', u'4']

The standard search does not look at word order::

    >>> results = search_service.search(q="dinosaur dress")
    >>> results.hits
    1
    >>> map(lambda x: x['id'], results)
    [u'2']

It's also possible to do more complex queries using the bq argument (Boolean
Query). When you are using bq, your search terms must be enclosed in single
quotes::

    >>> results = search_service.search(bq="'dan'")
    >>> results.hits
    2
    >>> map(lambda x: x['id'], results)
    [u'1', u'4']

When you are using boolean queries, it's also possible to use wildcards to
extend your search to all words which start with your search terms::

    >>> results = search_service.search(bq="'dan*'")
    >>> results.hits
    4
    >>> map(lambda x: x['id'], results)
    [u'1', u'2', u'3', u'4']

The boolean query also allows you to create more complex queries. You can OR
term together using "|", AND terms together using "+" or a space, and you can
remove words from the query using the "-" operator::

    >>> results = search_service.search(bq="'watched|moved'")
    >>> results.hits
    2
    >>> map(lambda x: x['id'], results)
    [u'3', u'4']

By default, the search will return 10 terms but it is possible to adjust this
by using the size argument as follows::

    >>> results = search_service.search(bq="'dan*'", size=2)
    >>> results.hits
    4
    >>> map(lambda x: x['id'], results)
    [u'1', u'2']

It is also possible to offset the start of the search by using the start
argument as follows::

    >>> results = search_service.search(bq="'dan*'", start=2)
    >>> results.hits
    4
    >>> map(lambda x: x['id'], results)
    [u'3', u'4']


Ordering search results and rank expressions
--------------------------------------------

If your search query is going to return many results, it is good to be able to
sort them. You can order your search results by using the rank argument. You are
able to sort on any fields which have the results option turned on::

    >>> results = search_service.search(bq=query, rank=['-follower_count'])

You can also create your own rank expressions to sort your results according to
other criteria, such as showing most recently active user, or combining the
recency score with the text_relevance::

    >>> domain.create_rank_expression('recently_active', 'last_activity')

    >>> domain.create_rank_expression('activish',
    ...   'text_relevance + ((follower_count/(time() - last_activity))*1000)')

    >>> results = search_service.search(bq=query, rank=['-recently_active'])

Viewing and Adjusting Stemming for a Domain
-------------------------------------------

A stemming dictionary maps related words to a common stem. A stem is
typically the root or base word from which variants are derived. For
example, run is the stem of running and ran. During indexing, Amazon
CloudSearch uses the stemming dictionary when it performs
text-processing on text fields. At search time, the stemming
dictionary is used to perform text-processing on the search
request. This enables matching on variants of a word. For example, if
you map the term running to the stem run and then search for running,
the request matches documents that contain run as well as running.

To get the current stemming dictionary defined for a domain, use the
:py:meth:`get_stemming <boto.cloudsearch.domain.Domain.get_stemming>` method::

    >>> stems = domain.get_stemming()
    >>> stems
    {u'stems': {}}
    >>>

This returns a dictionary object that can be manipulated directly to
add additional stems for your search domain by adding pairs of term:stem
to the stems dictionary::

    >>> stems['stems']['running'] = 'run'
    >>> stems['stems']['ran'] = 'run'
    >>> stems
    {u'stems': {u'ran': u'run', u'running': u'run'}}
    >>>

This has changed the value locally.  To update the information in
Amazon CloudSearch, you need to save the data::

    >>> stems.save()

You can also access certain CloudSearch-specific attributes related to
the stemming dictionary defined for your domain::

    >>> stems.status
    u'RequiresIndexDocuments'
    >>> stems.creation_date
    u'2012-05-01T12:12:32Z'
    >>> stems.update_date
    u'2012-05-01T12:12:32Z'
    >>> stems.update_version
    19
    >>>

The status indicates that, because you have changed the stems associated
with the domain, you will need to re-index the documents in the domain
before the new stems are used.

Viewing and Adjusting Stopwords for a Domain
--------------------------------------------

Stopwords are words that should typically be ignored both during
indexing and at search time because they are either insignificant or
so common that including them would result in a massive number of
matches.

To view the stopwords currently defined for your domain, use the
:py:meth:`get_stopwords <boto.cloudsearch.domain.Domain.get_stopwords>` method::

    >>> stopwords = domain.get_stopwords()
    >>> stopwords
    {u'stopwords': [u'a',
     u'an',
     u'and',
     u'are',
     u'as',
     u'at',
     u'be',
     u'but',
     u'by',
     u'for',
     u'in',
     u'is',
     u'it',
     u'of',
     u'on',
     u'or',
     u'the',
     u'to',
     u'was']}
    >>>

You can add additional stopwords by simply appending the values to the
list::

    >>> stopwords['stopwords'].append('foo')
    >>> stopwords['stopwords'].append('bar')
    >>> stopwords

Similarly, you could remove currently defined stopwords from the list.
To save the changes, use the :py:meth:`save
<boto.cloudsearch.optionstatus.OptionStatus.save>` method::

    >>> stopwords.save()

The stopwords object has similar attributes defined above for stemming
that provide additional information about the stopwords in your domain.


Viewing and Adjusting Synonyms for a Domain
--------------------------------------------

You can configure synonyms for terms that appear in the data you are
searching. That way, if a user searches for the synonym rather than
the indexed term, the results will include documents that contain the
indexed term.

If you want two terms to match the same documents, you must define
them as synonyms of each other. For example::

    cat, feline
    feline, cat

To view the synonyms currently defined for your domain, use the
:py:meth:`get_synonyms <boto.cloudsearch.domain.Domain.get_synonyms>` method::

    >>> synonyms = domain.get_synonyms()
    >>> synonyms
    {u'synonyms': {}}
    >>>

You can define new synonyms by adding new term:synonyms entries to the
synonyms dictionary object::

    >>> synonyms['synonyms']['cat'] = ['feline', 'kitten']
    >>> synonyms['synonyms']['dog'] = ['canine', 'puppy']

To save the changes, use the :py:meth:`save
<boto.cloudsearch.optionstatus.OptionStatus.save>` method::

    >>> synonyms.save()

The synonyms object has similar attributes defined above for stemming
that provide additional information about the stopwords in your domain.

Deleting Documents
------------------

It is also possible to delete documents::

    >>> import time
    >>> from datetime import datetime

    >>> doc_service = domain.get_document_service()

    >>> # Again we'll cheat and use the current epoch time as our version number

    >>> doc_service.delete(4, int(time.mktime(datetime.utcnow().timetuple())))
    >>> doc_service.commit()
