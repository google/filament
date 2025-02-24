# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import common.request as request

_DASHBOARD_URL = 'https://chromeperf.appspot.com'


def VerifiedAlertGroup(params):
  return _Request(_DASHBOARD_URL + '/verified_alert_group', params=params)


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
