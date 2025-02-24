.. swf_tut:
 :Authors: Slawek "oozie" Ligus <root@ooz.ie>, Brad Morris <bradley.s.morris@gmail.com>

===============================
Amazon Simple Workflow Tutorial
===============================

This tutorial focuses on boto's interface to AWS SimpleWorkflow service.

.. _SimpleWorkflow: http://aws.amazon.com/swf/

What is a workflow?
-------------------

A workflow is a sequence of multiple activities aimed at accomplishing a well-defined objective. For instance, booking an airline ticket as a workflow may encompass multiple activities, such as selection of itinerary, submission of personal details, payment validation and booking confirmation. 

Except for the start and completion of a workflow, each step has a well-defined predecessor and successor. With that
  - on successful completion of an activity the workflow can progress with its execution,
  - when one of workflow's activities fails it can be retried,
  - and when it keeps failing repeatedly the workflow may regress to the previous step to gather alternative inputs or it may simply fail at that stage.

Why use workflows?
------------------

Modelling an application on a workflow provides a useful abstraction layer for writing highly-reliable programs for distributed systems, as individual responsibilities can be delegated to a set of redundant, independent and non-critical processing units.

How does Amazon SWF help you accomplish this?
---------------------------------------------

Amazon SimpleWorkflow service defines an interface for workflow orchestration and provides state persistence for workflow executions.

Amazon SWF applications involve communication between the following entities:
  - The Amazon Simple Workflow Service - providing centralized orchestration and workflow state persistence,
  - Workflow Executors - some entity starting workflow executions, typically through an action taken by a user or from a cronjob.
  - Deciders - a program codifying the business logic, i.e. a set of instructions and decisions. Deciders take decisions based on initial set of conditions and outcomes from activities.
  - Activity Workers - their objective is very straightforward: to take inputs, execute the tasks and return a result to the Service.

The Workflow Executor contacts SWF Service and requests instantiation of a workflow. A new workflow is created and its state is stored in the service. 
The next time a decider contacts SWF service to ask for a decision task, it will be informed about a new workflow execution is taking place and it will be asked to advise SWF service on what the next steps should be. The decider then instructs the service to dispatch specific tasks to activity workers. At the next activity worker poll, the task is dispatched, then executed and the results reported back to the SWF, which then passes them onto the deciders. This exchange keeps happening repeatedly until the decider is satisfied and instructs the service to complete the execution.

Prerequisites
-------------

You need a valid access and secret key. The examples below assume that you have exported them to your environment, as follows:

.. code-block:: bash

    bash$ export AWS_ACCESS_KEY_ID=<your access key>
    bash$ export AWS_SECRET_ACCESS_KEY=<your secret key>

Before workflows and activities can be used, they have to be registered with SWF service:

.. code-block:: python

    # register.py
    import boto.swf.layer2 as swf
    from boto.swf.exceptions import SWFTypeAlreadyExistsError, SWFDomainAlreadyExistsError
    DOMAIN = 'boto_tutorial'
    VERSION = '1.0'
    
    registerables = []
    registerables.append(swf.Domain(name=DOMAIN))
    for workflow_type in ('HelloWorkflow', 'SerialWorkflow', 'ParallelWorkflow', 'SubWorkflow'):
        registerables.append(swf.WorkflowType(domain=DOMAIN, name=workflow_type, version=VERSION, task_list='default'))
    
    for activity_type in ('HelloWorld', 'ActivityA', 'ActivityB', 'ActivityC'):
        registerables.append(swf.ActivityType(domain=DOMAIN, name=activity_type, version=VERSION, task_list='default'))
    
    for swf_entity in registerables:
        try:
            swf_entity.register()
            print swf_entity.name, 'registered successfully'
        except (SWFDomainAlreadyExistsError, SWFTypeAlreadyExistsError):
            print swf_entity.__class__.__name__, swf_entity.name, 'already exists'
            
    
Execution of the above should produce no errors.

.. code-block:: bash

    bash$ python -i register.py
    Domain boto_tutorial already exists
    WorkflowType HelloWorkflow already exists
    SerialWorkflow registered successfully
    ParallelWorkflow registered successfully
    ActivityType HelloWorld already exists
    ActivityA registered successfully
    ActivityB registered successfully
    ActivityC registered successfully
    >>> 

HelloWorld
----------

This example is an implementation of a minimal Hello World workflow. Its execution should unfold as follows:

#. A workflow execution is started.
#. The SWF service schedules the initial decision task.
#. A decider polls for decision tasks and receives one.
#. The decider requests scheduling of an activity task.
#. The SWF service schedules the greeting activity task.
#. An activity worker polls for activity task and receives one.
#. The worker completes the greeting activity.
#. The SWF service schedules a decision task to inform about work outcome.
#. The decider polls and receives a new decision task.
#. The decider schedules workflow completion.
#. The workflow execution finishes.

Workflow logic is encoded in the decider:

.. code-block:: python

    # hello_decider.py
    import boto.swf.layer2 as swf
    
    DOMAIN = 'boto_tutorial'
    ACTIVITY = 'HelloWorld'
    VERSION = '1.0'
    TASKLIST = 'default'
    
    class HelloDecider(swf.Decider):
    
        domain = DOMAIN
        task_list = TASKLIST
        version = VERSION
    
        def run(self):
            history = self.poll()
            if 'events' in history:
                # Find workflow events not related to decision scheduling.
                workflow_events = [e for e in history['events']
                    if not e['eventType'].startswith('Decision')]
                last_event = workflow_events[-1]
    
                decisions = swf.Layer1Decisions()
                if last_event['eventType'] == 'WorkflowExecutionStarted':
                    decisions.schedule_activity_task('saying_hi', ACTIVITY, VERSION, task_list=TASKLIST)
                elif last_event['eventType'] == 'ActivityTaskCompleted':
                    decisions.complete_workflow_execution()
                self.complete(decisions=decisions)
                return True   
    
The activity worker is responsible for printing the greeting message when the activity task is dispatched to it by the service:

.. code-block:: python

    import boto.swf.layer2 as swf
    
    DOMAIN = 'boto_tutorial'
    VERSION = '1.0'
    TASKLIST = 'default'
    
    class HelloWorker(swf.ActivityWorker):
    
        domain = DOMAIN
        version = VERSION
        task_list = TASKLIST
    
        def run(self):
            activity_task = self.poll()
            if 'activityId' in activity_task:
                print 'Hello, World!'
                self.complete()
                return True

With actors implemented we can spin up a workflow execution:

.. code-block:: bash

    $ python
    >>> import boto.swf.layer2 as swf
    >>> execution = swf.WorkflowType(name='HelloWorkflow', domain='boto_tutorial', version='1.0', task_list='default').start()
    >>> 
    
From separate terminals run an instance of a worker and a decider to carry out a workflow execution (the worker and decider may run from two independent machines).

.. code-block:: bash

   $ python -i hello_decider.py
   >>> while HelloDecider().run(): pass
   ... 

.. code-block:: bash

   $ python -i hello_worker.py
   >>> while HelloWorker().run(): pass
   ... 
   Hello, World!

Great. Now, to see what just happened, go back to the original terminal from which the execution was started, and read its history.

.. code-block:: bash

    >>> execution.history()
    [{'eventId': 1,
      'eventTimestamp': 1381095173.2539999,
      'eventType': 'WorkflowExecutionStarted',
      'workflowExecutionStartedEventAttributes': {'childPolicy': 'TERMINATE',
                                                  'executionStartToCloseTimeout': '3600',
                                                  'parentInitiatedEventId': 0,
                                                  'taskList': {'name': 'default'},
                                                  'taskStartToCloseTimeout': '300',
                                                  'workflowType': {'name': 'HelloWorkflow',
                                                                   'version': '1.0'}}},
     {'decisionTaskScheduledEventAttributes': {'startToCloseTimeout': '300',
                                               'taskList': {'name': 'default'}},
      'eventId': 2,
      'eventTimestamp': 1381095173.2539999,
      'eventType': 'DecisionTaskScheduled'},
     {'decisionTaskStartedEventAttributes': {'scheduledEventId': 2},
      'eventId': 3,
      'eventTimestamp': 1381095177.5439999,
      'eventType': 'DecisionTaskStarted'},
     {'decisionTaskCompletedEventAttributes': {'scheduledEventId': 2,
                                               'startedEventId': 3},
      'eventId': 4,
      'eventTimestamp': 1381095177.855,
      'eventType': 'DecisionTaskCompleted'},
     {'activityTaskScheduledEventAttributes': {'activityId': 'saying_hi',
                                               'activityType': {'name': 'HelloWorld',
                                                                'version': '1.0'},
                                               'decisionTaskCompletedEventId': 4,
                                               'heartbeatTimeout': '600',
                                               'scheduleToCloseTimeout': '3900',
                                               'scheduleToStartTimeout': '300',
                                               'startToCloseTimeout': '3600',
                                               'taskList': {'name': 'default'}},
      'eventId': 5,
      'eventTimestamp': 1381095177.855,
      'eventType': 'ActivityTaskScheduled'},
     {'activityTaskStartedEventAttributes': {'scheduledEventId': 5},
      'eventId': 6,
      'eventTimestamp': 1381095179.427,
      'eventType': 'ActivityTaskStarted'},
     {'activityTaskCompletedEventAttributes': {'scheduledEventId': 5,
                                               'startedEventId': 6},
      'eventId': 7,
      'eventTimestamp': 1381095179.6989999,
      'eventType': 'ActivityTaskCompleted'},
     {'decisionTaskScheduledEventAttributes': {'startToCloseTimeout': '300',
                                               'taskList': {'name': 'default'}},
      'eventId': 8,
      'eventTimestamp': 1381095179.6989999,
      'eventType': 'DecisionTaskScheduled'},
     {'decisionTaskStartedEventAttributes': {'scheduledEventId': 8},
      'eventId': 9,
      'eventTimestamp': 1381095179.7420001,
      'eventType': 'DecisionTaskStarted'},
     {'decisionTaskCompletedEventAttributes': {'scheduledEventId': 8,
                                               'startedEventId': 9},
      'eventId': 10,
      'eventTimestamp': 1381095180.026,
      'eventType': 'DecisionTaskCompleted'},
     {'eventId': 11,
      'eventTimestamp': 1381095180.026,
      'eventType': 'WorkflowExecutionCompleted',
      'workflowExecutionCompletedEventAttributes': {'decisionTaskCompletedEventId': 10}}]
    

Serial Activity Execution
-------------------------

The following example implements a basic workflow with activities executed one after another.

The business logic, i.e. the serial execution of activities, is encoded in the decider:

.. code-block:: python

    # serial_decider.py
    import time
    import boto.swf.layer2 as swf
    
    class SerialDecider(swf.Decider):
    
        domain = 'boto_tutorial'
        task_list = 'default_tasks'
        version = '1.0'
    
        def run(self):
            history = self.poll()
            if 'events' in history:
                # Get a list of non-decision events to see what event came in last.
                workflow_events = [e for e in history['events']
                                   if not e['eventType'].startswith('Decision')]
                decisions = swf.Layer1Decisions()
                # Record latest non-decision event.
                last_event = workflow_events[-1]
                last_event_type = last_event['eventType']
                if last_event_type == 'WorkflowExecutionStarted':
                    # Schedule the first activity.
                    decisions.schedule_activity_task('%s-%i' % ('ActivityA', time.time()),
                       'ActivityA', self.version, task_list='a_tasks')
                elif last_event_type == 'ActivityTaskCompleted':
                    # Take decision based on the name of activity that has just completed.
                    # 1) Get activity's event id.
                    last_event_attrs = last_event['activityTaskCompletedEventAttributes']
                    completed_activity_id = last_event_attrs['scheduledEventId'] - 1
                    # 2) Extract its name.
                    activity_data = history['events'][completed_activity_id]
                    activity_attrs = activity_data['activityTaskScheduledEventAttributes']
                    activity_name = activity_attrs['activityType']['name']
                    # 3) Optionally, get the result from the activity.
                    result = last_event['activityTaskCompletedEventAttributes'].get('result')
    
                    # Take the decision.
                    if activity_name == 'ActivityA':
                        decisions.schedule_activity_task('%s-%i' % ('ActivityB', time.time()),
                            'ActivityB', self.version, task_list='b_tasks', input=result)
                    if activity_name == 'ActivityB':
                        decisions.schedule_activity_task('%s-%i' % ('ActivityC', time.time()),
                            'ActivityC', self.version, task_list='c_tasks', input=result)
                    elif activity_name == 'ActivityC':
                        # Final activity completed. We're done.
                        decisions.complete_workflow_execution()
    
                self.complete(decisions=decisions)
                return True

The workers only need to know which task lists to poll.

.. code-block:: python

    # serial_worker.py
    import time
    import boto.swf.layer2 as swf
    
    class MyBaseWorker(swf.ActivityWorker):
    
        domain = 'boto_tutorial'
        version = '1.0'
        task_list = None
    
        def run(self):
            activity_task = self.poll()
            if 'activityId' in activity_task:
                # Get input.
                # Get the method for the requested activity.
                try:
                    print 'working on activity from tasklist %s at %i' % (self.task_list, time.time())
                    self.activity(activity_task.get('input'))
                except Exception as error:
                    self.fail(reason=str(error))
                    raise error
    
                return True
    
        def activity(self, activity_input):
            raise NotImplementedError
    
    class WorkerA(MyBaseWorker):
        task_list = 'a_tasks'
        def activity(self, activity_input):
            self.complete(result="Now don't be givin him sambuca!")
    
    class WorkerB(MyBaseWorker):
        task_list = 'b_tasks'
        def activity(self, activity_input):
            self.complete()
    
    class WorkerC(MyBaseWorker):
        task_list = 'c_tasks'
        def activity(self, activity_input):
            self.complete()


Spin up a workflow execution and run the decider:

.. code-block:: bash

    $ python
    >>> import boto.swf.layer2 as swf
    >>> execution = swf.WorkflowType(name='SerialWorkflow', domain='boto_tutorial', version='1.0', task_list='default_tasks').start()
    >>> 
    
.. code-block:: bash

   $ python -i serial_decider.py
   >>> while SerialDecider().run(): pass
   ... 


Run the workers. The activities will be executed in order:

.. code-block:: bash

   $ python -i serial_worker.py
   >>> while WorkerA().run(): pass
   ... 
   working on activity from tasklist a_tasks at 1382046291

.. code-block:: bash

   $ python -i serial_worker.py
   >>> while WorkerB().run(): pass
   ... 
   working on activity from tasklist b_tasks at 1382046541

.. code-block:: bash

   $ python -i serial_worker.py
   >>> while WorkerC().run(): pass
   ... 
   working on activity from tasklist c_tasks at 1382046560


Looks good. Now, do the following to inspect the state and history of the execution:

.. code-block:: python

    >>> execution.describe()
    {'executionConfiguration': {'childPolicy': 'TERMINATE',
      'executionStartToCloseTimeout': '3600',
      'taskList': {'name': 'default_tasks'},
      'taskStartToCloseTimeout': '300'},
     'executionInfo': {'cancelRequested': False,
      'closeStatus': 'COMPLETED',
      'closeTimestamp': 1382046560.901,
      'execution': {'runId': '12fQ1zSaLmI5+lLXB8ux+8U+hLOnnXNZCY9Zy+ZvXgzhE=',
       'workflowId': 'SerialWorkflow-1.0-1382046514'},
      'executionStatus': 'CLOSED',
      'startTimestamp': 1382046514.994,
      'workflowType': {'name': 'SerialWorkflow', 'version': '1.0'}},
     'latestActivityTaskTimestamp': 1382046560.632,
     'openCounts': {'openActivityTasks': 0,
      'openChildWorkflowExecutions': 0,
      'openDecisionTasks': 0,
      'openTimers': 0}}
    >>> execution.history()
    ...

Parallel Activity Execution
---------------------------

When activities are independent from one another, their execution may be scheduled in parallel.

The decider schedules all activities at once and marks progress until all activities are completed, at which point the workflow is completed.

.. code-block:: python

    # parallel_decider.py

    import boto.swf.layer2 as swf
    import time

    SCHED_COUNT = 5

    class ParallelDecider(swf.Decider):

        domain = 'boto_tutorial'
        task_list = 'default'
        def run(self):
            decision_task = self.poll()
            if 'events' in decision_task:
                decisions = swf.Layer1Decisions()
                # Decision* events are irrelevant here and can be ignored.
                workflow_events = [e for e in decision_task['events'] 
                                   if not e['eventType'].startswith('Decision')]
                # Record latest non-decision event.
                last_event = workflow_events[-1]
                last_event_type = last_event['eventType']
                if last_event_type == 'WorkflowExecutionStarted':
                    # At start, kickoff SCHED_COUNT activities in parallel.
                    for i in range(SCHED_COUNT):
                        decisions.schedule_activity_task('activity%i' % i, 'ActivityA', '1.0',
                                                         task_list=self.task_list)
                elif last_event_type == 'ActivityTaskCompleted':
                    # Monitor progress. When all activities complete, complete workflow.
                    completed_count = sum([1 for a in decision_task['events']
                                           if a['eventType'] == 'ActivityTaskCompleted'])
                    print '%i/%i' % (completed_count, SCHED_COUNT)
                    if completed_count == SCHED_COUNT:
                        decisions.complete_workflow_execution()
                self.complete(decisions=decisions)
                return True

Again, the only bit of information a worker needs is which task list to poll.

.. code-block:: python

    # parallel_worker.py
    import time
    import boto.swf.layer2 as swf

    class ParallelWorker(swf.ActivityWorker):

        domain = 'boto_tutorial'
        task_list = 'default'

        def run(self):
            """Report current time."""
            activity_task = self.poll()
            if 'activityId' in activity_task:
                print 'working on', activity_task['activityId']
                self.complete(result=str(time.time()))
                return True

Spin up a workflow execution and run the decider:

.. code-block:: bash

    $ python -i parallel_decider.py 
    >>> execution = swf.WorkflowType(name='ParallelWorkflow', domain='boto_tutorial', version='1.0', task_list='default').start()
    >>> while ParallelDecider().run(): pass
    ... 
    1/5
    2/5
    4/5
    5/5

Run two or more workers to see how the service partitions work execution in parallel.

.. code-block:: bash

    $ python -i parallel_worker.py 
    >>> while ParallelWorker().run(): pass
    ... 
    working on activity1
    working on activity3
    working on activity4

.. code-block:: bash

    $ python -i parallel_worker.py 
    >>> while ParallelWorker().run(): pass
    ... 
    working on activity2
    working on activity0

As seen above, the work was partitioned between the two running workers.

Sub-Workflows
-------------

Sometimes it's desired or necessary to break the process up into multiple workflows.

Since the decider is stateless, it's up to you to determine which workflow is being used and which action
you would like to take.

.. code-block:: python

    import boto.swf.layer2 as swf

    class SubWorkflowDecider(swf.Decider):

        domain = 'boto_tutorial'
        task_list = 'default'
        version = '1.0'

        def run(self):
            history = self.poll()
            events = []
            if 'events' in history:
                events = history['events']
                # Collect the entire history if there are enough events to become paginated
                while 'nextPageToken' in history:
                    history = self.poll(next_page_token=history['nextPageToken'])
                    if 'events' in history:
                        events = events + history['events']

                workflow_type = history['workflowType']['name']

                # Get all of the relevent events that have happened since the last decision task was started
                workflow_events = [e for e in events
                        if e['eventId'] > history['previousStartedEventId'] and
                        not e['eventType'].startswith('Decision')]

                decisions = swf.Layer1Decisions()

                for event in workflow_events:
                    last_event_type = event['eventType']
                    if last_event_type == 'WorkflowExecutionStarted':
                        if workflow_type == 'SerialWorkflow':
                            decisions.start_child_workflow_execution('SubWorkflow', self.version,
                                "subworkflow_1", task_list=self.task_list, input="sub_1")
                        elif workflow_type == 'SubWorkflow':
                            for i in range(2):
                                decisions.schedule_activity_task("activity_%d" % i, 'ActivityA', self.version, task_list='a_tasks')
                        else:
                            decisions.fail_workflow_execution(reason="Unknown workflow %s" % workflow_type)
                            break

                    elif last_event_type == 'ChildWorkflowExecutionCompleted':
                        decisions.schedule_activity_task("activity_2", 'ActivityB', self.version, task_list='b_tasks')

                    elif last_event_type == 'ActivityTaskCompleted':
                        attrs = event['activityTaskCompletedEventAttributes']
                        activity = events[attrs['scheduledEventId'] - 1]
                        activity_name = activity['activityTaskScheduledEventAttributes']['activityType']['name']

                        if activity_name == 'ActivityA':
                            completed_count = sum([1 for a in events if a['eventType'] == 'ActivityTaskCompleted'])
                            if completed_count == 2:
                                # Complete the child workflow
                                decisions.complete_workflow_execution()
                        elif activity_name == 'ActivityB':
                            # Complete the parent workflow
                            decisions.complete_workflow_execution()

                self.complete(decisions=decisions)
            return True

Misc
----

Some of these things are not obvious by reading the API documents, so hopefully they help you
avoid some time-consuming pitfalls.

Pagination
==========

When the decider polls for new tasks, the maximum number of events it will return at a time is 100
(configurable to a smaller number, but not larger). When running a workflow, this number gets quickly
exceeded. If it does, the decision task will contain a key ``nextPageToken`` which can be submit to the
``poll()`` call to get the next page of events.

.. code-block:: python

    decision_task = self.poll()

    events = []
    if 'events' in decision_task:
      events = decision_task['events']
      while 'nextPageToken' in decision_task:
          decision_task = self.poll(next_page_token=decision_task['nextPageToken'])
          if 'events' in decision_task:
              events += decision_task['events']

Depending on your workflow logic, you might not need to aggregate all of the events.

Decision Tasks
==============

When first running deciders and activities, it may seem that the decider gets called for every event that
an activity triggers; however, this is not the case. More than one event can happen between decision tasks.
The decision task will contain a key ``previousStartedEventId`` that lets you know the ``eventId`` of the
last DecisionTaskStarted event that was processed. Your script will need to handle all of the events
that have happened since then, not just the last activity.

.. code-block:: python

    workflow_events = [e for e in events if e['eventId'] > decision_task['previousStartedEventId']]

You may also wish to still filter out tasks that start with 'Decision' or filter it in some other way
that fulfills your needs. You will now have to iterate over the workflow_events list and respond to
each event, as it may contain multiple events.

Filtering Events
================

When running many tasks in parallel, a common task is searching through the history to see how many events
of a particular activity type started, completed, and/or failed. Some basic list comprehension makes
this trivial.

.. code-block:: python

    def filter_completed_events(self, events, type):
        completed = [e for e in events if e['eventType'] == 'ActivityTaskCompleted']
        orig = [events[e['activityTaskCompletedEventAttributes']['scheduledEventId']-1] for e in completed]
        return [e for e in orig if e['activityTaskScheduledEventAttributes']['activityType']['name'] == type]

.. _Amazon SWF API Reference: http://docs.aws.amazon.com/amazonswf/latest/apireference/Welcome.html
.. _StackOverflow questions: http://stackoverflow.com/questions/tagged/amazon-swf
.. _Miscellaneous Blog Articles: http://log.ooz.ie/search/label/SimpleWorkflow
