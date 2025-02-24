# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from flask import make_response

from dashboard.pinpoint.models import scheduler


def QueueStatsHandlerGet(configuration):
  if not configuration:
    return make_response(
        json.dumps({'error': 'Missing configuration in request.'}), 400)

  try:
    queue_stats = scheduler.QueueStats(configuration)
  except scheduler.QueueNotFound:
    return make_response('The queue does not exist: %s' % configuration, 404)
  return make_response(json.dumps(queue_stats))
