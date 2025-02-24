# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides the web interface for displaying an overview of jobs."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import json

from flask import make_response

from dashboard.common import cloud_metric
from dashboard.pinpoint.models import job

_MAX_JOBS_TO_FETCH = 10000

# TODO: Generalize the Jobs handler to allow the user to choose what fields to
# include and how many Jobs to fetch.


@cloud_metric.APIMetric("pinpoint", "/api/stats")
def StatsHandler():
  return make_response(json.dumps(_GetJobs()))


def _GetJobs():
  created_limit = datetime.datetime.now() - datetime.timedelta(days=28)
  query = job.Job.query(job.Job.created >= created_limit)
  jobs = query.fetch(limit=_MAX_JOBS_TO_FETCH)
  return [{
      'status': j.status,
      'comparison_mode': j.comparison_mode or 'try',
      'created': j.created.isoformat(),
      'started': j.started_time.isoformat() if j.started_time else None,
      'difference_count': j.difference_count,
      'updated': j.updated.isoformat(),
  } for j in jobs]
