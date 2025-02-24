# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
from unittest import mock

from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint import test


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class StatsTest(test.TestCase):

  def testPost_ValidRequest(self):
    # Create job.
    job = job_module.Job.New((), ())

    data = json.loads(self.testapp.get('/api/stats').body)

    expected = [{
        'status': 'Queued',
        'comparison_mode': job.comparison_mode or 'try',
        'created': job.created.isoformat(),
        'started': job.started_time.isoformat() if job.started_time else None,
        'difference_count': None,
        'updated': job.updated.isoformat(),
    }]
    self.assertEqual(data, expected)
