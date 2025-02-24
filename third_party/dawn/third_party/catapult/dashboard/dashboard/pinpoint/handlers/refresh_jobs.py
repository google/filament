# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import logging

from flask import make_response

from google.appengine.api import app_identity

from dashboard.common import cloud_metric
from dashboard.common import layered_cache
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import job as job_module

_JOB_CACHE_KEY = 'pinpoint_refresh_jobs_%s'
_JOB_MAX_RETRIES = 1
_JOB_FROZEN_THRESHOLD = datetime.timedelta(hours=1)


def RefreshJobsHandler():
  _FindAndRestartJobs()
  return make_response('', 200)


def _FindFrozenJobs():
  jobs = job_module.Job.query(job_module.Job.running == True).fetch()
  now = datetime.datetime.utcnow()

  def _IsFrozen(j):
    time_elapsed = now - j.updated
    return time_elapsed >= _JOB_FROZEN_THRESHOLD

  results = [j for j in jobs if _IsFrozen(j)]
  logging.debug('frozen jobs = %r', [j.job_id for j in results])

  for j in results:
    cloud_metric.PublishFrozenJobMetric(app_identity.get_application_id(),
                                        j.job_id, j.comparison_mode,
                                        "frozen-job-found")

  return results


def _FindAndRestartJobs():
  jobs = _FindFrozenJobs()

  for j in jobs:
    _ProcessFrozenJob(j)


def _ProcessFrozenJob(job):
  key = _JOB_CACHE_KEY % job.job_id
  info = layered_cache.Get(key) or {'retries': 0}

  retries = info.setdefault('retries', 0)
  if retries >= _JOB_MAX_RETRIES:
    info['retries'] += 1
    layered_cache.Set(key, info, days_to_keep=30)
    job.Fail(errors.JobRetryFailed())
    logging.error('Retry #%d: failed retrying job %s', retries, job.job_id)
    cloud_metric.PublishFrozenJobMetric(app_identity.get_application_id(),
                                        job.job_id, job.comparison_mode,
                                        "frozen-job-failed")
    return

  logging.debug('Retry #%d: retrying job %s', retries, job.job_id)
  cloud_metric.PublishFrozenJobMetric(app_identity.get_application_id(),
                                      job.job_id, job.comparison_mode,
                                      "frozen-job-processing")

  info['retries'] += 1
  layered_cache.Set(key, info, days_to_keep=30)

  logging.info('Restarting job %s', job.job_id)
  job._Schedule()
  job.put()
