# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from flask import make_response

from dashboard.common import cloud_metric
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import errors


@cloud_metric.APIMetric("pinpoint", "/api/run")
def RunHandler(job_id):
  job = job_module.JobFromId(job_id)
  try:
    job.Run()
    return make_response('', 200)
  except errors.BuildCancelled as e:
    logging.warning(
        'Failed to run a job which has been already cancelled. Jod ID: %s',
        job_id)
    return make_response(str(e), 400)
