.. _ref-boto_commandline:

==================
Command Line Tools
==================

Introduction
============

Boto ships with a number of command line utilities, which are installed
when the package is installed. This guide outlines which ones are available
& what they do.

.. note::

    If you're not already depending on these utilities, you may wish to check
    out the AWS-CLI (http://aws.amazon.com/cli/ - `User Guide`_ &
    `Reference Guide`_). It provides much wider & complete access to the
    AWS services.

    .. _`User Guide`: http://docs.aws.amazon.com/cli/latest/userguide/cli-chap-welcome.html
    .. _`Reference Guide`: http://docs.aws.amazon.com/cli/latest/reference/

The included utilities available are:

``asadmin``
    Works with Autoscaling

``bundle_image``
    Creates a bundled AMI in S3 based on a EC2 instance

``cfadmin``
    Works with CloudFront & invalidations

``cq``
    Works with SQS queues

``cwutil``
    Works with CloudWatch

``dynamodb_dump``
``dynamodb_load``
    Handle dumping/loading data from DynamoDB tables

``elbadmin``
    Manages Elastic Load Balancer instances

``fetch_file``
    Downloads an S3 key to disk

``glacier``
    Lists vaults, jobs & uploads files to Glacier

``instance_events``
    Lists all events for EC2 reservations

``kill_instance``
    Kills a list of EC2 instances

``launch_instance``
    Launches an EC2 instance

``list_instances``
    Lists all of your EC2 instances

``lss3``
    Lists what keys you have within a bucket in S3

``mturk``
    Provides a number of facilities for interacting with Mechanical Turk

``pyami_sendmail``
    Sends an email from the Pyami instance

``route53``
    Interacts with the Route53 service

``s3put``
    Uploads a directory or a specific file(s) to S3

``sdbadmin``
    Allows for working with SimpleDB domains

``taskadmin``
    A tool for working with the tasks in SimpleDB
