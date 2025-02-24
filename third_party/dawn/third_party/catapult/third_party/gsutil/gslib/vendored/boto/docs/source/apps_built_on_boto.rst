.. _apps_built_on_boto:

==========================
Applications Built On Boto
==========================

Many people have taken Boto and layered on additional functionality, then shared
them with the community. This is a (partial) list of applications that use Boto.

If you have an application or utility you've open-sourced that uses Boto &
you'd like it listed here, please submit a `pull request`_ adding it!

.. _`pull request`: https://github.com/boto/boto/pulls

**botornado**
    https://pypi.python.org/pypi/botornado
    An asynchronous AWS client on Tornado. This is a dirty work to move boto
    onto Tornado ioloop. Currently works with SQS and S3.

**boto_rsync**
    https://pypi.python.org/pypi/boto_rsync
    boto-rsync is a rough adaptation of boto's s3put script which has been
    reengineered to more closely mimic rsync. Its goal is to provide a familiar
    rsync-like wrapper for boto's S3 and Google Storage interfaces.

**boto_utils**
    https://pypi.python.org/pypi/boto_utils
    Command-line tools for interacting with Amazon Web Services, based on Boto.
    Includes utils for S3, SES & Cloudwatch.

**django-storages**
    https://pypi.python.org/pypi/django-storages
    A collection of storage backends for Django. Features the ``S3BotoStorage``
    backend for storing media on S3.

**mr.awsome**
    https://pypi.python.org/pypi/mr.awsome
    mr.awsome is a commandline-tool (aws) to manage and control Amazon
    Webservice's EC2 instances. Once configured with your AWS key, you can
    create, delete, monitor and ssh into instances, as well as perform scripted
    tasks on them (via fabfiles). Examples are adding additional,
    pre-configured webservers to a cluster (including updating the load
    balancer), performing automated software deployments and creating backups -
    each with just one call from the commandline.

**iamer**
    https://pypi.python.org/pypi/iamer
    IAMer dump and load your AWS IAM configuration into text files. Once
    dumped, you can version the resulting json and ini files to keep track of
    changes, and even ask your team mates to do Pull Requests when they want
    access to something.
