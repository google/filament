.. _emr_tut:

=====================================================
An Introduction to boto's Elastic Mapreduce interface
=====================================================

This tutorial focuses on the boto interface to Elastic Mapreduce from
Amazon Web Services.  This tutorial assumes that you have already
downloaded and installed boto.

Creating a Connection
---------------------
The first step in accessing Elastic Mapreduce is to create a connection
to the service.  There are two ways to do this in boto.  The first is:

>>> from boto.emr.connection import EmrConnection
>>> conn = EmrConnection('<aws access key>', '<aws secret key>')

At this point the variable conn will point to an EmrConnection object.
In this example, the AWS access key and AWS secret key are passed in to
the method explicitly.  Alternatively, you can set the environment variables:

AWS_ACCESS_KEY_ID - Your AWS Access Key ID \
AWS_SECRET_ACCESS_KEY - Your AWS Secret Access Key

and then call the constructor without any arguments, like this:

>>> conn = EmrConnection()

There is also a shortcut function in boto
that makes it easy to create EMR connections:

>>> import boto.emr
>>> conn = boto.emr.connect_to_region('us-west-2')

In either case, conn points to an EmrConnection object which we will use
throughout the remainder of this tutorial.

Creating Streaming JobFlow Steps
--------------------------------
Upon creating a connection to Elastic Mapreduce you will next
want to create one or more jobflow steps.  There are two types of steps, streaming
and custom jar, both of which have a class in the boto Elastic Mapreduce implementation.

Creating a streaming step that runs the AWS wordcount example, itself written in Python, can be accomplished by:

>>> from boto.emr.step import StreamingStep
>>> step = StreamingStep(name='My wordcount example',
...                      mapper='s3n://elasticmapreduce/samples/wordcount/wordSplitter.py',
...                      reducer='aggregate',
...                      input='s3n://elasticmapreduce/samples/wordcount/input',
...                      output='s3n://<my output bucket>/output/wordcount_output')

where <my output bucket> is a bucket you have created in S3.

Note that this statement does not run the step, that is accomplished later when we create a jobflow.  

Additional arguments of note to the streaming jobflow step are cache_files, cache_archive and step_args.  The options cache_files and cache_archive enable you to use the Hadoops distributed cache to share files amongst the instances that run the step.  The argument step_args allows one to pass additional arguments to Hadoop streaming, for example modifications to the Hadoop job configuration.

Creating Custom Jar Job Flow Steps
----------------------------------

The second type of jobflow step executes tasks written with a custom jar.  Creating a custom jar step for the AWS CloudBurst example can be accomplished by:

>>> from boto.emr.step import JarStep
>>> step = JarStep(name='Coudburst example',
...                jar='s3n://elasticmapreduce/samples/cloudburst/cloudburst.jar',
...                step_args=['s3n://elasticmapreduce/samples/cloudburst/input/s_suis.br',
...                           's3n://elasticmapreduce/samples/cloudburst/input/100k.br',
...                           's3n://<my output bucket>/output/cloudfront_output',
...                            36, 3, 0, 1, 240, 48, 24, 24, 128, 16])

Note that this statement does not actually run the step, that is accomplished later when we create a jobflow.  Also note that this JarStep does not include a main_class argument since the jar MANIFEST.MF has a Main-Class entry.

Creating JobFlows
-----------------
Once you have created one or more jobflow steps, you will next want to create and run a jobflow.  Creating a jobflow that executes either of the steps we created above can be accomplished by:

>>> import boto.emr
>>> conn = boto.emr.connect_to_region('us-west-2')
>>> jobid = conn.run_jobflow(name='My jobflow', 
...                          log_uri='s3://<my log uri>/jobflow_logs', 
...                          steps=[step])

The method will not block for the completion of the jobflow, but will immediately return.  The status of the jobflow can be determined by:

>>> status = conn.describe_jobflow(jobid)
>>> status.state
u'STARTING'

One can then use this state to block for a jobflow to complete.  Valid jobflow states currently defined in the AWS API are COMPLETED, FAILED, TERMINATED, RUNNING, SHUTTING_DOWN, STARTING and WAITING.

In some cases you may not have built all of the steps prior to running the jobflow.  In these cases additional steps can be added to a jobflow by running:

>>> conn.add_jobflow_steps(jobid, [second_step])

If you wish to add additional steps to a running jobflow you may want to set the keep_alive parameter to True in run_jobflow so that the jobflow does not automatically terminate when the first step completes.

The run_jobflow method has a number of important parameters that are worth investigating.  They include parameters to change the number and type of EC2 instances on which the jobflow is executed, set a SSH key for manual debugging and enable AWS console debugging.

Terminating JobFlows
--------------------
By default when all the steps of a jobflow have finished or failed the jobflow terminates.  However, if you set the keep_alive parameter to True or just want to halt the execution of a jobflow early you can terminate a jobflow by:

>>> import boto.emr
>>> conn = boto.emr.connect_to_region('us-west-2')
>>> conn.terminate_jobflow('<jobflow id>') 
