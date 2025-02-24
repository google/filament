# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Pinpoint FIFO Scheduler Handler

This HTTP handler is responsible for polling the state of the various FIFO
queues currently defined in the service, and running queued jobs as they are
ready. The scheduler enforces that there's only one currently runing job for any
configuration, and does not attempt to do any admission control nor load
shedding. Those features will be implemented in a dedicated scheduler service.
"""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from flask import make_response

from dashboard.pinpoint.models import job as job_model
from dashboard.pinpoint.models import scheduler

DEFAULT_BUDGET = 1.0


def FifoSchedulerHandler():
  _ProcessFIFOQueues()
  return make_response('', 200)


def _ProcessFIFOQueues():
  configurations = scheduler.AllConfigurations()
  logging.info('Found %d FIFO Queues', len(configurations))
  for configuration in configurations:
    logging.info('Processing queue \'%s\'', configuration)
    process_queue = True

    # The way we're doing the capacity-aware scheduling is by setting a
    # budget that we can consume on every scheduler run. Each type of
    # comparison_mode for a job will have an associated budget for the queue
    # (or we'll set a default). We will consume this budget every time by
    # accounting the jobs that are running against it, and stop when we have
    # the budget exhausted in the loop.
    # TODO(dberris): See if we can use retroactive cost assignment instead of
    # relying on the cost at job creation time.
    _, budget = scheduler.GetSchedulerOptions(configuration)
    logging.info('Budget: %s', budget)
    while process_queue:
      jobs = scheduler.PickJobs(configuration, budget)
      for job_id, queue_status in jobs:
        if not job_id:
          logging.info('Empty queue for configuration = %s', configuration)
          process_queue = False
        else:
          process_queue = _ProcessJob(job_id, queue_status, configuration)


def _ProcessJob(job_id, queue_status, configuration):
  job = job_model.JobFromId(job_id)
  if not job:
    logging.error('Failed to load job with id: %s', job_id)
    scheduler.Remove(configuration, job_id)
    return False

  logging.info('Job "%s" status: "%s" queue: "%s"', job_id, job.status,
               configuration)

  if queue_status == 'Running':
    if job.status in {'Failed', 'Completed'}:
      scheduler.Complete(job)
      return True  # Continue processing this queue.

  if queue_status == 'Queued':
    job.Start()
    # TODO(dberris): Remove this when the execution engine is the default.
    if job.use_execution_engine:
      logging.info(
          'Skipping Job that uses the experimental execution engine: %s (%s)',
          job.job_id, job.url)
      scheduler.Complete(job)

  return False
