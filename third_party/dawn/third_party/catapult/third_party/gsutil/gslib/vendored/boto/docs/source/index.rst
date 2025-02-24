.. _index:

===============================================
boto: A Python interface to Amazon Web Services
===============================================

.. note::

  `Boto3 <https://github.com/boto/boto3>`__, the next version of Boto, is now
  stable and recommended for general use.  It can be used side-by-side with
  Boto in the same project, so it is easy to start using Boto3 in your existing
  projects as well as new projects. Going forward, API updates and all new
  feature work will be focused on Boto3.


An integrated interface to current and future infrastructural services
offered by `Amazon Web Services`_.

Currently, all features work with Python 2.6 and 2.7. Work is under way to
support Python 3.3+ in the same codebase. Modules are being ported one at
a time with the help of the open source community, so please check below
for compatibility with Python 3.3+.

To port a module to Python 3.3+, please view our
:doc:`Contributing Guidelines <contributing>` and the
:doc:`Porting Guide <porting_guide>`. If you would like, you can open an
issue to let others know about your work in progress. Tests **must** pass
on Python 2.6, 2.7, 3.3, and 3.4 for pull requests to be accepted.

.. _Amazon Web Services: http://aws.amazon.com/

Getting Started
---------------

If you've never used ``boto`` before, you should read the
:doc:`Getting Started with Boto <getting_started>` guide to get familiar
with ``boto`` & its usage.

Currently Supported Services
----------------------------

* **Compute**

  * :doc:`Elastic Compute Cloud (EC2) <ec2_tut>` -- (:doc:`API Reference <ref/ec2>`) (Python 3)
  * :doc:`Elastic MapReduce (EMR) <emr_tut>` -- (:doc:`API Reference <ref/emr>`) (Python 3)
  * :doc:`Auto Scaling <autoscale_tut>` -- (:doc:`API Reference <ref/autoscale>`) (Python 3)
  * Kinesis -- (:doc:`API Reference <ref/kinesis>`) (Python 3)
  * Lambda -- (:doc:`API Reference <ref/awslambda>`) (Python 3)
  * EC2 Container Service (ECS) -- (:doc:`API Reference <ref/ec2containerservice>`) (Python 3)

* **Content Delivery**

  * :doc:`CloudFront <cloudfront_tut>` -- (:doc:`API Reference <ref/cloudfront>`) (Python 3)

* **Database**

  * :doc:`DynamoDB2 <dynamodb2_tut>` -- (:doc:`API Reference <ref/dynamodb2>`) -- (:doc:`Migration Guide from v1 <migrations/dynamodb_v1_to_v2>`)
  * :doc:`DynamoDB <dynamodb_tut>` -- (:doc:`API Reference <ref/dynamodb>`) (Python 3)
  * Relational Data Services 2 (RDS) -- (:doc:`API Reference <ref/rds2>`) -- (:doc:`Migration Guide from v1 <migrations/rds_v1_to_v2>`)
  * :doc:`Relational Data Services (RDS) <rds_tut>` -- (:doc:`API Reference <ref/rds>`)
  * ElastiCache -- (:doc:`API Reference <ref/elasticache>`) (Python 3)
  * Redshift -- (:doc:`API Reference <ref/redshift>`) (Python 3)
  * :doc:`SimpleDB <simpledb_tut>` -- (:doc:`API Reference <ref/sdb>`) (Python 3)

* **Deployment and Management**

  * CloudFormation -- (:doc:`API Reference <ref/cloudformation>`) (Python 3)
  * Elastic Beanstalk -- (:doc:`API Reference <ref/beanstalk>`) (Python 3)
  * Data Pipeline -- (:doc:`API Reference <ref/datapipeline>`) (Python 3)
  * Opsworks -- (:doc:`API Reference <ref/opsworks>`) (Python 3)
  * CloudTrail -- (:doc:`API Reference <ref/cloudtrail>`) (Python 3)
  * CodeDeploy -- (:doc:`API Reference <ref/codedeploy>`) (Python 3)

* **Administration & Security**

  * Identity and Access Management (IAM) -- (:doc:`API Reference <ref/iam>`) (Python 3)
  * Security Token Service (STS) -- (:doc:`API Reference <ref/sts>`) (Python 3)
  * Key Management Service (KMS) -- (:doc:`API Reference <ref/kms>`) (Python 3)
  * Config -- (:doc:`API Reference <ref/configservice>`) (Python 3)
  * CloudHSM -- (:doc:`API Reference <ref/cloudhsm>`) (Python 3)

* **Application Services**

  * Cloudsearch 2 -- (:doc:`API Reference <ref/cloudsearch2>`) (Python 3)
  * :doc:`Cloudsearch <cloudsearch_tut>` -- (:doc:`API Reference <ref/cloudsearch>`) (Python 3)
  * CloudSearch Domain --(:doc:`API Reference <ref/cloudsearchdomain>`) (Python 3)
  * Elastic Transcoder -- (:doc:`API Reference <ref/elastictranscoder>`) (Python 3)
  * :doc:`Simple Workflow Service (SWF) <swf_tut>` -- (:doc:`API Reference <ref/swf>`) (Python 3)
  * :doc:`Simple Queue Service (SQS) <sqs_tut>` -- (:doc:`API Reference <ref/sqs>`) (Python 3)
  * Simple Notification Service (SNS) -- (:doc:`API Reference <ref/sns>`) (Python 3)
  * :doc:`Simple Email Service (SES) <ses_tut>` -- (:doc:`API Reference <ref/ses>`) (Python 3)
  * Amazon Cognito Identity -- (:doc:`API Reference <ref/cognito-identity>`) (Python 3)
  * Amazon Cognito Sync -- (:doc:`API Reference <ref/cognito-sync>`) (Python 3)
  * Amazon Machine Learning -- (:doc:`API Reference <ref/machinelearning>`) (Python 3)

* **Monitoring**

  * :doc:`CloudWatch <cloudwatch_tut>` -- (:doc:`API Reference <ref/cloudwatch>`) (Python 3)
  * CloudWatch Logs -- (:doc:`API Reference <ref/logs>`) (Python 3)

* **Networking**

  * :doc:`Route 53 <route53_tut>` -- (:doc:`API Reference <ref/route53>`) (Python 3)
  * Route 53 Domains -- (:doc:`API Reference <ref/route53domains>`) (Python 3)
  * :doc:`Virtual Private Cloud (VPC) <vpc_tut>` -- (:doc:`API Reference <ref/vpc>`) (Python 3)
  * :doc:`Elastic Load Balancing (ELB) <elb_tut>` -- (:doc:`API Reference <ref/elb>`) (Python 3)
  * AWS Direct Connect (Python 3)

* **Payments & Billing**

  * Flexible Payments Service (FPS) -- (:doc:`API Reference <ref/fps>`)

* **Storage**

  * :doc:`Simple Storage Service (S3) <s3_tut>` -- (:doc:`API Reference <ref/s3>`) (Python 3)
  * Amazon Glacier -- (:doc:`API Reference <ref/glacier>`) (Python 3)
  * Google Cloud Storage -- (:doc:`API Reference <ref/gs>`)

* **Workforce**

  * Mechanical Turk -- (:doc:`API Reference <ref/mturk>`)

* **Other**

  * Marketplace Web Services -- (:doc:`API Reference <ref/mws>`) (Python 3)
  * :doc:`Support <support_tut>` -- (:doc:`API Reference <ref/support>`) (Python 3)

Additional Resources
--------------------

* :doc:`Applications Built On Boto <apps_built_on_boto>`
* :doc:`Command Line Utilities <commandline>`
* :doc:`Boto Config Tutorial <boto_config_tut>`
* :doc:`Contributing to Boto <contributing>`
* :doc:`Evaluating Application performance with Boto logging <request_hook_tut>`
* `Boto Source Repository`_
* `Boto Issue Tracker`_
* `Boto Twitter`_
* `Follow Mitch on Twitter`_
* Join our `IRC channel`_ (#boto on FreeNode).

.. _Boto Issue Tracker: https://github.com/boto/boto/issues
.. _Boto Source Repository: https://github.com/boto/boto
.. _Boto Twitter: http://twitter.com/pythonboto
.. _IRC channel: http://webchat.freenode.net/?channels=boto
.. _Follow Mitch on Twitter: http://twitter.com/garnaat


Release Notes
-------------

.. toctree::
   :titlesonly:

   releasenotes/v2.49.0
   releasenotes/v2.48.0
   releasenotes/v2.47.0
   releasenotes/v2.46.1
   releasenotes/v2.46.0
   releasenotes/v2.45.0
   releasenotes/v2.44.0
   releasenotes/v2.43.0
   releasenotes/v2.42.0
   releasenotes/v2.41.0
   releasenotes/v2.40.0
   releasenotes/v2.39.0
   releasenotes/v2.38.0
   releasenotes/v2.37.0
   releasenotes/v2.36.0
   releasenotes/v2.35.2
   releasenotes/v2.35.1
   releasenotes/v2.35.0
   releasenotes/v2.34.0
   releasenotes/v2.33.0
   releasenotes/v2.32.1
   releasenotes/v2.32.0
   releasenotes/v2.31.1
   releasenotes/v2.31.0
   releasenotes/v2.30.0
   releasenotes/v2.29.1
   releasenotes/v2.29.0
   releasenotes/v2.28.0
   releasenotes/v2.27.0
   releasenotes/v2.26.1
   releasenotes/v2.26.0
   releasenotes/v2.25.0
   releasenotes/v2.24.0
   releasenotes/v2.23.0
   releasenotes/v2.22.1
   releasenotes/v2.22.0
   releasenotes/v2.21.2
   releasenotes/v2.21.1
   releasenotes/v2.21.0
   releasenotes/v2.20.1
   releasenotes/v2.20.0
   releasenotes/v2.19.0
   releasenotes/v2.18.0
   releasenotes/v2.17.0
   releasenotes/v2.16.0
   releasenotes/v2.15.0
   releasenotes/v2.14.0
   releasenotes/v2.13.3
   releasenotes/v2.13.2
   releasenotes/v2.13.0
   releasenotes/v2.12.0
   releasenotes/v2.11.0
   releasenotes/v2.10.0
   releasenotes/v2.9.9
   releasenotes/v2.9.8
   releasenotes/v2.9.7
   releasenotes/v2.9.6
   releasenotes/v2.9.5
   releasenotes/v2.9.4
   releasenotes/v2.9.3
   releasenotes/v2.9.2
   releasenotes/v2.9.1
   releasenotes/v2.9.0
   releasenotes/v2.8.0
   releasenotes/v2.7.0
   releasenotes/v2.6.0
   releasenotes/v2.5.2
   releasenotes/v2.5.1
   releasenotes/v2.5.0
   releasenotes/v2.4.0
   releasenotes/v2.3.0
   releasenotes/v2.2.2
   releasenotes/v2.2.1
   releasenotes/v2.2.0
   releasenotes/v2.1.1
   releasenotes/v2.1.0
   releasenotes/v2.0.0
   releasenotes/v2.0b1


.. toctree::
   :hidden:
   :glob:

   getting_started
   ec2_tut
   security_groups
   emr_tut
   autoscale_tut
   cloudfront_tut
   simpledb_tut
   dynamodb_tut
   rds_tut
   sqs_tut
   ses_tut
   swf_tut
   cloudsearch_tut
   cloudwatch_tut
   vpc_tut
   elb_tut
   s3_tut
   route53_tut
   boto_config_tut
   documentation
   contributing
   commandline
   support_tut
   dynamodb2_tut
   migrations/dynamodb_v1_to_v2
   migrations/rds_v1_to_v2
   apps_built_on_boto
   ref/*
   releasenotes/*


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
