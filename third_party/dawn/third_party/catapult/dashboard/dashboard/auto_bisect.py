# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for a cron job to automatically run bisects."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import functools
import json
import logging
import six

from dashboard import can_bisect
from dashboard import pinpoint_request
from dashboard.common import namespaced_stored_object
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.services import pinpoint_service


class NotBisectableError(Exception):
  """An error indicating that a bisect couldn't be automatically started."""


def StartNewBisectForBug(bug_id, project_id):
  """Tries to trigger a bisect job for the alerts associated with a bug.

  Args:
    bug_id: A bug ID number.
    project_id: A Monorail project ID.

  Returns:
    If successful, a dict containing "issue_id" and "issue_url" for the
    bisect job. Otherwise, a dict containing "error", with some description
    of the reason why a job wasn't started.
  """
  try:
    return _StartBisectForBug(bug_id, project_id)
  except NotBisectableError as e:
    logging.info('New bisect errored out with message: %s', str(e))
    return {'error': str(e)}


def _StartBisectForBug(bug_id, project_id):
  anomalies, _, _ = anomaly.Anomaly.QueryAsync(
      bug_id=bug_id, project_id=project_id, limit=500).get_result()
  if not anomalies:
    raise NotBisectableError('No Anomaly alerts found for this bug.')

  test_anomaly = _ChooseTest(anomalies)
  test = None
  if test_anomaly:
    test = test_anomaly.GetTestMetadataKey().get()
  if not test or not can_bisect.IsValidTestForBisect(test.test_path):
    raise NotBisectableError('Could not select a test.')

  bot_configurations = namespaced_stored_object.Get('bot_configurations')

  if test.bot_name not in list(bot_configurations.keys()):
    raise NotBisectableError('Bot: %s has no corresponding Pinpoint bot.' %
                             test.bot_name)
  return _StartPinpointBisect(bug_id, project_id, test_anomaly, test)


def _StartPinpointBisect(bug_id, project_id, test_anomaly, test):
  # Convert params to Pinpoint compatible
  anomaly_keys = utils.ConvertBytesBeforeJsonDumps([test_anomaly.key.urlsafe()])
  params = {
      'test_path': test.test_path,
      'start_commit': test_anomaly.start_revision - 1,
      'end_commit': test_anomaly.end_revision,
      'bug_id': bug_id,
      'project_id': project_id,
      'bisect_mode': 'performance',
      'story_filter': test.unescaped_story_name,
      'alerts': json.dumps(anomaly_keys)
  }

  try:
    results = pinpoint_service.NewJob(
        pinpoint_request.PinpointParamsFromBisectParams(params))
  except pinpoint_request.InvalidParamsError as e:
    six.raise_from(NotBisectableError(str(e)), e)

  # For compatibility with existing bisect, switch these to issueId/url
  if 'jobId' in results:
    results['issue_id'] = results['jobId']
    test_anomaly.pinpoint_bisects.append(str(results['jobId']))
    test_anomaly.put()
    del results['jobId']

  if 'jobUrl' in results:
    results['issue_url'] = results['jobUrl']
    del results['jobUrl']

  return results


def _ChooseTest(anomalies):
  """Chooses a test to use for a bisect job.

  The particular TestMetadata chosen determines the command and metric name that
  is chosen. The test to choose could depend on which of the anomalies has the
  largest regression size.

  Ideally, the choice of bisect bot to use should be based on bisect bot queue
  length, and the choice of metric should be based on regression size and noise
  level.

  However, we don't choose bisect bot and metric independently, since some
  regressions only happen for some tests on some platforms; we should generally
  only bisect with a given bisect bot on a given metric if we know that the
  regression showed up on that platform for that metric.

  Args:
    anomalies: A non-empty list of Anomaly entities.

  Returns:
    An Anomaly entity, or None if no valid entity could be chosen.

  Raises:
    NotBisectableError: The only matching tests are on domains that have been
        excluded for automatic bisects on alert triage.
  """
  if not anomalies:
    return None
  anomalies.sort(key=functools.cmp_to_key(_CompareAnomalyBisectability))
  found_excluded_domain = False
  for anomaly_entity in anomalies:
    if can_bisect.IsValidTestForBisect(
        utils.TestPath(anomaly_entity.GetTestMetadataKey())):
      if can_bisect.DomainIsExcludedFromTriageBisects(
          anomaly_entity.master_name):
        found_excluded_domain = True
        continue
      return anomaly_entity
  if found_excluded_domain:
    raise NotBisectableError(
        'Did not kick off bisect because only available domains are '
        'excluded from automatic bisects on triage.')
  return None


def _CompareAnomalyBisectability(a1, a2):
  """Compares two Anomalies to decide which Anomaly's TestMetadata is better to
     use.

  Note: Potentially, this could be made more sophisticated by using
  more signals:
   - Bisect bot queue length
   - Platform
   - Test run time
   - Stddev of test

  Args:
    a1: The first Anomaly entity.
    a2: The second Anomaly entity.

  Returns:
    Negative integer if a1 is better than a2, positive integer if a2 is better
    than a1, or zero if they're equally good.
  """
  if a1.percent_changed > a2.percent_changed:
    return -1
  if a1.percent_changed < a2.percent_changed:
    return 1
  return 0


def GetRevisionForBisect(revision, test_key):
  """Gets a start or end revision value which can be used when bisecting.

  Note: This logic is parallel to that in elements/chart-container.html
  in the method getRevisionForBisect.

  Args:
    revision: The ID of a Row, not necessarily an actual revision number.
    test_key: The ndb.Key for a TestMetadata.

  Returns:
    An int or string value which can be used when bisecting.
  """
  row_parent_key = utils.GetTestContainerKey(test_key)
  row = graph_data.Row.get_by_id(revision, parent=row_parent_key)
  if row and hasattr(row, 'a_default_rev') and hasattr(row, row.a_default_rev):
    return getattr(row, row.a_default_rev)
  return revision
