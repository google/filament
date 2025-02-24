# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for getting commit information from Pinpoint."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import collections

from dashboard.common import datastore_hooks
from dashboard.common import utils
from dashboard.services import request

_PINPOINT_URL = 'https://pinpoint-dot-chromeperf.appspot.com'
_PINPOINT_SKIA_URL = 'https://perf.luci.app'

def NewJob(params):
  """Submits a new job request to Pinpoint."""
  return _Request(_PINPOINT_URL + '/api/new', params)

def NewJobInSkia(params):
  """Submits a new job request to Pinpoint in Skia."""
  return _Request(_PINPOINT_SKIA_URL + '/pinpoint/v1/culprit-finder', params)

def _Request(endpoint, params):
  """Sends a request to an endpoint and returns JSON data."""
  assert datastore_hooks.IsUnalteredQueryPermitted()
  try:
    return request.RequestJson(
        endpoint,
        method='POST',
        use_cache=False,
        use_auth=True,
        **params)
  except request.RequestError as e:
    try:
      return json.loads(e.content)
    except ValueError:
      # for errors.SwarmingNoBots()
      return {"error": str(e)}


class CommitRange(collections.namedtuple('CommitRange', ['start', 'end'])):
  __slots__ = ()


def UpdateSkiaCulpritFinderRequest(pinpoint_params, alert, bug_id, agg_method):
  """Update a Pinpoint culprit finder request for the Skia backend.

  Culprit finder is also known as sandwich verification.
  Pinpoint is being migrated from catapult repo to skia repo.
  The Skia backend accepts slightly different inputs. The Skia backend
  accepts the improvement direction.

  Args:
    pinpoint_params: the bisection request used in catapult
    alert: the candidate regression
    bug_id: the bugID
    agg_method: the method used to aggregate the benchmark runs
        i.e. avg, mean, std, count, median. Also known as statistic.

  Returns:
    Pinpoint request to start a new skia culprit finder job
  """
  mag = alert.median_after_anomaly - alert.median_before_anomaly
  pinpoint_params['comparison_magnitude'] = mag
  pinpoint_params['bug_id'] = bug_id
  pinpoint_params['aggregation_method'] = agg_method

  return pinpoint_params

def MakeBisectionRequest(test,
                         commit_range,
                         issue,
                         comparison_mode,
                         target,
                         comparison_magnitude=None,
                         user=None,
                         name=None,
                         story_filter=None,
                         priority=0,
                         pin=None,
                         tags=None):
  """Generate a valid pinpoint bisection request.

  Args:
    test: TestMetadata entiry
    commit_range: CommitRange including git hash start and end
    issue: Related issue id
    comparison_mode: performance or functional
    target: Isolation target
    comparison_magnitude: Magnitude used in bisection
    user: User email triggered the request
    name: Pinpoint job name
    story_filter: Test story
    priority: Job priority
    pin: Pin a base version
    tags: Extra tags

  Returns:
    Pinpoint request to start a new bisection job
  """

  story = story_filter or test.unescaped_story_name

  grouping_label = ''
  chart = ''
  trace = ''
  if comparison_mode == 'performance' and test.suite_name not in ['v8']:
    grouping_label, chart, trace = utils.ParseTelemetryMetricParts(
        test.test_path)
    if trace and test.unescaped_story_name:
      trace = test.unescaped_story_name
  chart, statistic = utils.ParseStatisticNameFromChart(chart)

  pinpoint_params = {
      'configuration': test.bot_name,
      'benchmark': test.suite_name,
      'chart': chart,
      'start_git_hash': commit_range.start,
      'end_git_hash': commit_range.end,
      'comparison_mode': comparison_mode,
      'target': target,
      'priority': priority,
      'tags': json.dumps(tags or {}),
      'initial_attempt_count': 20,
  }

  pinpoint_params.update({
      k: v for k, v in [
          ('user', user),
          ('name', name),
          ('bug_id', issue and issue.issue_id),
          ('project', issue and issue.project_id),
          ('comparison_magnitude', comparison_magnitude),
          ('pin', pin),
          ('statistic', statistic),
          ('story', story),
          ('grouping_label', grouping_label),
          ('trace', trace),
      ] if v
  })

  return pinpoint_params
