# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import common.request as request

_PINPOINT_URL = 'https://pinpoint-dot-chromeperf.appspot.com'


def NewJob(params):
  """Submits a new job request to Pinpoint."""
  return _Request(_PINPOINT_URL + '/api/new', params=params)


def GetJob(job_id):
  return _Request(_PINPOINT_URL + '/api/job/%s' % job_id, method='GET')


def _Request(endpoint, params=None, method='POST'):
  """Sends a request to an endpoint and returns JSON data."""
  if not params:
    params = {}
  try:
    return request.RequestJson(endpoint, method=method, **params)
  except request.RequestError as e:
    try:
      return json.loads(e.content)
    except ValueError:
      # for errors.SwarmingNoBots()
      return {"error": str(e)}
