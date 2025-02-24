# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Pinpoint Job Scheduler Module

This module implements a simple FIFO scheduler which in the future will be a
full-featured multi-dimensional priority queue based scheduler that leverages
more features of Swarming for managing the capacity of the Pinpoint swarming
pool.

"""

# TODO(dberris): Isolate the service that will make all the scheduling decisions
# and make this API a wrapper to the scheduler.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
import functools
import random
import logging

from google.appengine.api import app_identity
from google.appengine.ext import ndb

from dashboard.common import bot_configurations
from dashboard.common import cloud_metric
from dashboard.pinpoint import models

SECS_PER_HOUR = datetime.timedelta(hours=1).total_seconds()
DEFAULT_BUDGET = 1.0
DEFAULT_COST = 1.0


# TODO(dberris): These models are temporary, when we move to using the service
# we'll use the google-cloud-datastore API directly.
class QueueElement(ndb.Model):
  """Models an element in a queues."""
  _default_indexed = False
  timestamp = ndb.DateTimeProperty(required=True, auto_now_add=True)
  queue_length = ndb.IntegerProperty(required=True)
  job_id = ndb.StringProperty(required=True)
  status = ndb.StringProperty(
      required=True, default='Queued', choices=['Running', 'Done', 'Cancelled'])

  # Priority is in "nice" order, where 0 is highest priority and anything higher
  # is lower priority.
  priority = ndb.IntegerProperty(required=True, default=0)

  # Cost is imbued at schedule time, and is advisory if the queue supports
  # cost-based scheduling.
  cost = ndb.FloatProperty(default=1.0)


class SampleElementTiming(ndb.Model):
  """Represents a measurement of queue time delay."""
  _default_indexed = False
  job_id = ndb.StringProperty(required=True)
  enqueue_timestamp = ndb.DateTimeProperty(required=True)
  picked_timestamp = ndb.DateTimeProperty(required=True, auto_now_add=True)
  queue_length = ndb.IntegerProperty(required=True)


class Queues(ndb.Model):
  """A root element for all queues."""


class ConfigurationQueue(ndb.Model):
  """Models a per-pool (configuration) FIFO queue."""
  _default_indexed = False
  _default_memcache = True
  jobs = ndb.StructuredProperty(QueueElement, repeated=True)
  configuration = ndb.StringProperty(required=True, indexed=True)
  samples = ndb.StructuredProperty(SampleElementTiming, repeated=True)

  @classmethod
  def GetOrCreateQueue(cls, configuration):
    parent = Queues.get_by_id('root')
    if not parent:
      parent = Queues(id='root')
      parent.put()

    queue = ConfigurationQueue.get_by_id(
        configuration, parent=ndb.Key('Queues', 'root'))
    if not queue:
      return ConfigurationQueue(
          jobs=[],
          configuration=configuration,
          id=configuration,
          parent=ndb.Key('Queues', 'root'))
    return queue

  @classmethod
  def AllQueues(cls):
    return cls.query(
        projection=[cls.configuration], ancestor=ndb.Key('Queues', 'root'))

  def put(self):
    # We clean up the queue of any 'Done' and 'Cancelled' elements before we
    # persist the data.
    self.jobs = [j for j in self.jobs if j.status not in {'Done', 'Cancelled'}]

    # We also only persist samples that are < 7 days old, and capping the number
    # of samples. This prevents us from growing the entries too large dominated
    # by the samples.
    now = datetime.datetime.utcnow()
    self.samples = [
        s for s in self.samples
        if s.enqueue_timestamp - now < datetime.timedelta(days=7)
    ]
    if len(self.samples) > 50:
      self.samples = random.sample(self.samples, 50)
    super().put()


class Error(Exception):
  pass


class QueueNotFound(Error):
  pass


@ndb.transactional
def Schedule(job, cost=1.0):
  """Schedules a job for later execution.

  This function deduces the appropriate queue to which a fully-formed
  `dashboard.pinpoint.models.job.Job` must be enqueued, and persists a reference
  to the job ID to the queue for later execution.

  Arguments:
  - job: a fully-formed `dashboard.models.job.Job` instance.
  - cost: an advisory weight for scheduling in a cost-based scheduler.

  Raises:
  - ndb.TransactionFailedError when we fail to persist the queue
    transactionally.

  Returns None.
  """
  # Take a job and find an appropriate pool to enqueue it through.

  # 1. Use the configuration as the name of the pool.
  # TODO(dberris): Figure out whether a missing configuration is even valid.
  configuration = job.arguments.get('configuration', '(none)')
  priority = job.priority

  # 2. Load the (potentially empty) FIFO queue.
  queue = ConfigurationQueue.GetOrCreateQueue(configuration)

  # TODO(dberris): Check whether we have too many elements in the queue,
  # and reject the attempt?

  # 3. Enqueue job according to insertion time.
  queue.jobs.append(
      QueueElement(
          job_id=job.job_id,
          queue_length=len(queue.jobs),
          priority=priority,
          cost=cost))
  queue.put()
  cloud_metric.PublishPinpointJobStatusMetric(
      app_identity.get_application_id(), job.job_id,
      job.comparison_mode, "queued", job.user, job.origin,
      models.job.GetJobTypeByName(job.name), job.configuration,
      job.benchmark_arguments.benchmark, job.benchmark_arguments.story)


@ndb.transactional
def PickJobs(configuration, budget=1.0):
  """Picks a job for execution for a given configuration.

  This returns the next eligible job to run which is one that's either already
  running, or one that's Queued.

  Returns a list of tuples (job_id, 'Running'|'Queued') if we have eligible
  jobs to run, or a list with a single element (None, None). We'll use the
  provided budget and the costs of each queued element to determine which
  jobs to pick for scheduling decisions.

  Arguments:
  - configuration: a configuration name, also used as a queue identifier.
  - budget: we will consume this budget, i.e. only return a value if the cost
    for a queued item is less than or equal to the budget provided.

  Raises:
  - ndb.TransactionFailedError when we fail to persist the queue
    transactionally.
  """
  # Load the FIFO queue for the configuration.
  queue = ConfigurationQueue.GetOrCreateQueue(configuration)

  if not queue.jobs:
    return [(None, None)]

  # Find all the 'Running' instances and consume the budget to return all the
  # currently running jobs.
  results = []
  for job in queue.jobs:
    if budget <= 0:
      # We have no more budget for running new jobs.
      return results
    if job.status == 'Running':
      results.append((job.job_id, job.status))
      budget -= job.cost

  # Sort the jobs in priority and submission time. Note that we can starve lower
  # priority (those whose priority is higher than 0) jobs by design, since we'll
  # assume those are batch jobs.
  queue.jobs.sort(key=lambda j: (j.priority or 0, j.timestamp))
  for job in queue.jobs:
    # Short-circuit out if the budget is not exhausted.
    if budget <= 0.0:
      break

    # Pick the first job that's queued, and mark it 'Running'.
    if job.status == 'Queued':
      results.append((job.job_id, job.status))
      budget -= job.cost
      job.status = 'Running'

      # Add this to the samples.
      queue.samples.append(
          SampleElementTiming(
              job_id=job.job_id,
              enqueue_timestamp=job.timestamp,
              queue_length=job.queue_length))

  # Persist the changes transactionally.
  queue.put()

  # Then return the results.
  return results


@ndb.transactional
def QueueStats(configuration):
  """Computes and returns statistics for a queue.

  Returns a dictionary with the following keys:
  - queued_jobs: A point-in-time count of the number of queued jobs for the
    configuration.
  - cancelled_jobs: A point-in-time count of cancelled jobs.
  - running_jobs: A point-in-time count of jobs that are "running".
  - queue_time_samples: A list of floats, representing the number of hours the
    most recent jobs from the past 7 days have been in the queue.
  """
  queue = ConfigurationQueue.get_by_id(
      configuration, parent=ndb.Key('Queues', 'root'))
  if not queue:
    logging.warning('Failed to find queue for configuration: %s', configuration)
    raise QueueNotFound()

  def StatCombiner(status_map, job):
    key = '{}_jobs'.format(job.status.lower())
    status_map.setdefault(key, 0)
    status_map[key] += 1
    return status_map

  def _FormatSample(s):
    t = s.picked_timestamp - s.enqueue_timestamp
    return (t.total_seconds() / SECS_PER_HOUR, s.queue_length)

  result = functools.reduce(StatCombiner, queue.jobs, {})
  result.update({
      'queue_time_samples': [_FormatSample(s) for s in queue.samples],
      'job_id_with_status': [{
          'job_id': j.job_id,
          'status': j.status
      } for j in queue.jobs],
  })
  return result

@ndb.transactional
def IsStopped(job):
  """Checks if a job has stopped or not. Jobs should be stopped if
  their status in the job queue is not Running or Queued."""

  # Take a job and determine the FIFO Queue it's associated to.
  configuration = job.arguments.get('configuration', '(none)')

  # Iterate through the queue and see if job is either running or queued
  queue = ConfigurationQueue.GetOrCreateQueue(configuration)
  for queued_job in queue.jobs:
    if queued_job.job_id == job.job_id:
      if queued_job.status in {'Running', 'Queued'}:
        return False
  return True

@ndb.transactional
def Cancel(job):
  """Marks a job for cancellation in the appropriate queue.

  This updates a job's status in the queue as cancelled, making it ineligible
  for running. This operation is not reversible.

  Arguments:
  - job: a fully-formed dashboard.pinpoint.models.job.Job instance.

  Raises:
  - ndb.TransactionFailedError on failure to transactionally update the queue.

  Returns a boolean indicating whether the job was found and cancelled.
  """
  # Take a job and determine the FIFO Queue it's associated to.
  configuration = job.arguments.get('configuration', '(none)')

  # Find the job, and mark it cancelled.
  # TODO(dberris): Figure out whether a missing configuration is even valid.
  queue = ConfigurationQueue.GetOrCreateQueue(configuration)

  found = False
  for queued_job in queue.jobs:
    if queued_job.job_id == job.job_id:
      if queued_job.status in {'Running', 'Queued'}:
        queued_job.status = 'Cancelled'
        found = True
      break
  queue.put()
  return found


@ndb.transactional
def Complete(job):
  """Marks a job completed in the appropriate queue.

  This updates a job's status in the queue as completed, making it ineligible
  for running. This operation is not reversible.

  Arguments:
  - job: a fully-formed dashboard.pinpoint.models.job.Job instance.

  Raises:
  - ndb.TransactionFilaedError on failure to transactionally update the queue.

  Returns None.
  """
  # TODO(dberris): Figure out whether a missing configuration is even valid.
  configuration = job.arguments.get('configuration', '(none)')

  queue = ConfigurationQueue.GetOrCreateQueue(configuration)

  # We can only complete 'Running' jobs.
  for queued_job in queue.jobs:
    if queued_job.job_id == job.job_id:
      if queued_job.status == 'Running':
        queued_job.status = 'Done'
      break
  queue.put()


@ndb.transactional
def Remove(configuration, job_id):
  """Forcibly removes a job from the queue, by ID.

  This updates the queue to remove the job identifier. Note that this does not
  update a job's status. This is mostly a convenience method to forcibly remove
  jobs from a queue as a remedial action.

  Arguments:
  - configuration: a string identifying the configuration, used as a queue
    identifier as well.
  - job_id: a string identifying a job instance.

  Raises:
  - ndb.TransactionFilaedError on failure to transactionally update the queue.

  Returns None
  """
  queue = ConfigurationQueue.GetOrCreateQueue(configuration)
  queue.jobs = [j for j in queue.jobs if j.job_id != job_id]
  queue.put()


@ndb.transactional
def AllConfigurations():
  return [q.configuration for q in ConfigurationQueue.AllQueues().fetch()]


class SchedulerOptions(
    collections.namedtuple('SchedulerOptions', ('costs', 'budget'))):
  __slots__ = ()


def GetSchedulerOptions(configuration):
  # Here we're getting the configuration settings on the following, for this
  # particular configuration:
  #
  #   - What cost do we attribute to tryjobs/bisections?
  #   - What is the budget for each scheduling iteration?
  #
  # These options will all be part of a sub-object in the 'scheduler' key in
  # the bot configuration. What we're expecting is a structure like so:
  #
  #   {
  #     "scheduler": {
  #       "cost": {
  #       },
  #       "budget": <floating point>
  #     }
  #   }
  try:
    bot_config = bot_configurations.Get(configuration)
  except ValueError:
    bot_config = {}

  scheduler_options = bot_config.get('scheduler', {})
  return SchedulerOptions(
      costs=scheduler_options.get(
          'costs', collections.defaultdict(lambda: DEFAULT_COST)),
      budget=scheduler_options.get('budget', DEFAULT_BUDGET))


def Cost(job):
  """Computes the cost for a job, for scheduling decisions.

  Returns a floating point number to indicate cost in a cost-based scheduler.
  """
  return GetSchedulerOptions(job.configuration).costs[job.comparison_mode]
