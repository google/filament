# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Processes tests and creates new Anomaly entities.

This module contains the ProcessTest function, which searches the recent
points in a test for potential regressions or improvements, and creates
new Anomaly entities.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.ext import deferred
from google.appengine.ext import ndb

from dashboard import email_sheriff
from dashboard import find_change_points
from dashboard.common import cloud_metric
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models import subscription
from dashboard.services import perf_issue_service_client
from dashboard.sheriff_config_client import SheriffConfigClient
from tracing.value.diagnostics import reserved_infos

# Number of points to fetch and pass to FindChangePoints. A different number
# may be used if a test has a "max_window_size" anomaly config parameter.
DEFAULT_NUM_POINTS = 50


@ndb.synctasklet
def ProcessTests(test_keys):
  """Processes a list of tests to find new anoamlies.

  Args:
    test_keys: A list of TestMetadata ndb.Key's.
  """
  yield ProcessTestsAsync(test_keys)


@ndb.tasklet
def ProcessTestsAsync(test_keys):
  # Using a parallel yield here let's the tasklets for each _ProcessTest run
  # in parallel.
  yield [_ProcessTest(k) for k in test_keys]


@ndb.tasklet
def _ProcessTest(test_key):
  """Processes a test to find new anomalies.

  Args:
    test_key: The ndb.Key for a TestMetadata.
  """
  # We're dropping clank support, which goes through the old recipe_bisect
  # system. For now, we're simply disabling alert generation and stopping
  # bisects from getting kicked off. We'll follow up with a more thorough
  # removal of all old bisect related code.
  # crbug.com/937230
  if test_key.id().startswith('ClankInternal'):
    raise ndb.Return(None)

  test = yield test_key.get_async()

  max_num_rows = DEFAULT_NUM_POINTS
  rows_by_stat = yield GetRowsToAnalyzeAsync(test, max_num_rows)

  ref_rows_by_stat = {}
  ref_test = yield _CorrespondingRefTest(test_key)
  if ref_test:
    ref_rows_by_stat = yield GetRowsToAnalyzeAsync(ref_test, max_num_rows)

  for s, rows in rows_by_stat.items():
    if rows:
      logging.info('Processing test: %s', test_key.id())
      yield _ProcessTestStat(test, s, rows, ref_rows_by_stat.get(s))


def _EmailSheriff(sheriff, test_key, anomaly_key):
  test_entity = test_key.get()
  anomaly_entity = anomaly_key.get()

  email_sheriff.EmailSheriff(sheriff, test_entity, anomaly_entity)


@ndb.tasklet
def _ProcessTestStat(test, stat, rows, ref_rows):
  # If there were no rows fetched, then there's nothing to analyze.
  if not rows:
    logging.error('No rows fetched for %s', test.test_path)
    raise ndb.Return(None)


  # TODO(crbug/1158326): Use the data from the git-hosted anomaly configuration
  # instead of the provided config.
  # Get all the sheriff from sheriff-config match the path
  client = SheriffConfigClient()
  subscriptions, err_msg = client.Match(test.test_path)

  # Breaks the process when Match failed to ensure find_anomaly do the best
  # effort to find the subscriber. Leave retrying to upstream.
  if err_msg is not None:
    raise RuntimeError(err_msg)

  # If we don't find any subscriptions, then we shouldn't waste resources on
  # trying to find anomalies that we aren't going to alert on anyway.
  if not subscriptions:
    logging.error('No subscription for %s', test.test_path)
    raise ndb.Return(None)

  configs = {
      s.name: [c.to_dict() for c in s.anomaly_configs
              ] for s in subscriptions or [] if s.anomaly_configs
  }
  if configs:
    logging.debug('matched anomaly configs: %s', configs)

  # Get anomalies and check if they happen in ref build also.
  test_key = test.key
  anomaly_inputs = []
  for matching_sub in subscriptions:
    anomaly_configs = matching_sub.anomaly_configs or [
        subscription.AnomalyConfig()
    ]
    for config in [c.to_dict() for c in anomaly_configs]:
      change_points = FindChangePointsForTest(rows, config)
      if ref_rows:
        ref_change_points = FindChangePointsForTest(ref_rows, config)
        change_points = _FilterAnomaliesFoundInRef(change_points,
                                                   ref_change_points, test_key)
      anomaly_inputs.extend(
          (c, test, stat, rows, config, matching_sub) for c in change_points)

  anomalies = yield [_MakeAnomalyEntity(*inputs) for inputs in anomaly_inputs]

  anomalies = [a for a in anomalies if a is not None]

  # If no new anomalies were found, then we're done.
  if not anomalies:
    raise ndb.Return(None)

  logging.info('Created %d anomalies: %s', len(anomalies), [
      '(start=%s, end=%s, project=%s)' %
      (a.start_revision, a.end_revision, a.master_name) for a in anomalies
  ])
  logging.info(' Test: %s', test_key.id())
  logging.info(' Stat: %s', stat)
  logging.info(' Inputs: %s', [input[0] for input in anomaly_inputs])

  for a in anomalies:
    a.subscriptions = [a.matching_subscription]
    a.subscription_names = [a.matching_subscription.name]
    a.internal_only = (
        any(s.visibility != subscription.VISIBILITY.PUBLIC
            for s in subscriptions) or test.internal_only)
    alert_groups = alert_group.AlertGroup.GetGroupsForAnomaly(a, subscriptions)
    try:
      # parity results from perf_issue_service
      groups_by_request = perf_issue_service_client.GetAlertGroupsForAnomaly(a)
      group_keys = [ndb.Key('AlertGroup', g) for g in groups_by_request]
      if sorted(group_keys) != sorted(alert_groups):
        logging.warning('Imparity found for GetAlertGroupsForAnomaly. %s, %s',
                        group_keys, alert_groups)
        cloud_metric.PublishPerfIssueServiceGroupingImpariry(
            'GetAlertGroupsForAnomaly')
      a.groups = group_keys
      logging.debug('[GroupingDebug] Anomaly %s is associated with groups %s.',
                    a.key, a.groups)
    except Exception as e:  # pylint: disable=broad-except
      logging.warning('Parity logic failed in GetAlertGroupsForAnomaly. %s',
                      str(e))

  yield ndb.put_multi_async(anomalies)

  # TODO(simonhatch): email_sheriff.EmailSheriff() isn't a tasklet yet, so this
  # code will run serially.
  # Email sheriff about any new regressions, but deduplicate them by the
  # matched subscription.
  for anomaly_entity in anomalies:
    if anomaly_entity.bug_id is None and not anomaly_entity.is_improvement:
      deferred.defer(_EmailSheriff, [anomaly_entity.matching_subscription],
                     test.key, anomaly_entity.key)


@ndb.tasklet
def _FindLatestAlert(test, stat):
  query = anomaly.Anomaly.query()
  query = query.filter(anomaly.Anomaly.test == test.key)
  query = query.filter(anomaly.Anomaly.statistic == stat)
  query = query.order(-anomaly.Anomaly.end_revision)
  results = yield query.get_async()
  if not results:
    raise ndb.Return(None)
  raise ndb.Return(results)


@ndb.tasklet
def _FindMonitoredStatsForTest(test):
  del test
  # TODO: This will get filled out after refactor.
  raise ndb.Return(['avg'])


@ndb.synctasklet
def GetRowsToAnalyze(test, max_num_rows):
  """Gets the Row entities that we want to analyze.

  Args:
    test: The TestMetadata entity to get data for.
    max_num_rows: The maximum number of points to get.

  Returns:
    A list of the latest Rows after the last alerted revision, ordered by
    revision. These rows are fetched with t a projection query so they only
    have the revision, value, and timestamp properties.
  """
  result = yield GetRowsToAnalyzeAsync(test, max_num_rows)
  raise ndb.Return(result)


@ndb.tasklet
def GetRowsToAnalyzeAsync(test, max_num_rows):
  # If this is a histogram based test, there may be multiple statistics we want
  # to analyze
  alerted_stats = yield _FindMonitoredStatsForTest(test)

  latest_alert_by_stat = dict(
      (s, _FindLatestAlert(test, s)) for s in alerted_stats)

  results = {}
  for s in alerted_stats:
    results[s] = _FetchRowsByStat(test.key, s, latest_alert_by_stat[s],
                                  max_num_rows)

  for s in results:
    results[s] = yield results[s]

  raise ndb.Return(results)


@ndb.tasklet
def _FetchRowsByStat(test_key, stat, last_alert_future, max_num_rows):
  # If stats are specified, we only want to alert on those, otherwise alert on
  # everything.
  if stat == 'avg':
    query = graph_data.Row.query(
        projection=['revision', 'timestamp', 'value', 'swarming_bot_id'])
  else:
    query = graph_data.Row.query()

  query = query.filter(
      graph_data.Row.parent_test == utils.OldStyleTestKey(test_key))

  # The query is ordered in descending order by revision because we want
  # to get the newest points.
  if last_alert_future:
    last_alert = yield last_alert_future
    if last_alert:
      query = query.filter(graph_data.Row.revision > last_alert.end_revision)
  query = query.order(-graph_data.Row.revision)  # pylint: disable=invalid-unary-operand-type

  # However, we want to analyze them in ascending order.
  rows = yield query.fetch_async(limit=max_num_rows)

  vals = []
  for r in list(reversed(rows)):
    if stat == 'avg':
      vals.append((r.revision, r, r.value))
    elif stat == 'std':
      vals.append((r.revision, r, r.error))
    else:
      vals.append((r.revision, r, getattr(r, 'd_%s' % stat)))

  raise ndb.Return(vals)


def _FilterAnomaliesFoundInRef(change_points, ref_change_points, test):
  change_points_filtered = []
  test_path = utils.TestPath(test)
  for c in change_points:
    # Log information about what anomaly got filtered and what did not.
    if not _IsAnomalyInRef(c, ref_change_points):
      logging.info('Nothing was filtered out for test %s, and revision %s',
                   test_path, c.x_value)
      change_points_filtered.append(c)
    else:
      logging.info('Filtering out anomaly for test %s, and revision %s',
                   test_path, c.x_value)
  return change_points_filtered


@ndb.tasklet
def _CorrespondingRefTest(test_key):
  """Returns the TestMetadata for the corresponding ref build trace, or None."""
  test_path = utils.TestPath(test_key)
  possible_ref_test_paths = [test_path + '_ref', test_path + '/ref']
  for path in possible_ref_test_paths:
    ref_test = yield utils.TestKey(path).get_async()
    if ref_test:
      raise ndb.Return(ref_test)
  raise ndb.Return(None)


def _IsAnomalyInRef(change_point, ref_change_points):
  """Checks if anomalies are detected in both ref and non ref build.

  Args:
    change_point: A find_change_points.ChangePoint object to check.
    ref_change_points: List of find_change_points.ChangePoint objects
        found for a ref build series.

  Returns:
    True if there is a match found among the ref build series change points.
  """
  for ref_change_point in ref_change_points:
    if change_point.x_value == ref_change_point.x_value:
      return True
  return False


def _GetImmediatelyPreviousRevisionNumber(later_revision, rows):
  """Gets the revision number of the Row immediately before the given one.

  Args:
    later_revision: A revision number.
    rows: List of Row entities in ascending order by revision.

  Returns:
    The revision number just before the given one.
  """
  for (revision, _, _) in reversed(rows):
    if revision < later_revision:
      return revision
  raise AssertionError('No matching revision found in |rows|.')


def _GetRefBuildKeyForTest(test):
  """TestMetadata key of the reference build for the given test, if one exists.

  Args:
    test: the TestMetadata entity to get the ref build for.

  Returns:
    A TestMetadata key if found, or None if not.
  """
  potential_path = '%s/ref' % test.test_path
  potential_test = utils.TestKey(potential_path).get()
  if potential_test:
    return potential_test.key
  potential_path = '%s_ref' % test.test_path
  potential_test = utils.TestKey(potential_path).get()
  if potential_test:
    return potential_test.key
  return None


def _GetDisplayRange(old_end, rows):
  """Get the revision range using a_display_rev, if applicable.

  Args:
    old_end: the x_value from the change_point
    rows: List of Row entities in asscending order by revision.

  Returns:
    A end_rev, start_rev tuple with the correct revision.
  """
  start_rev = end_rev = 0
  for (revision, row, _) in reversed(rows):
    if revision == old_end and hasattr(row, 'r_commit_pos'):
      end_rev = row.r_commit_pos
    elif revision < old_end and hasattr(row, 'r_commit_pos'):
      start_rev = row.r_commit_pos + 1
      break
  if not end_rev or not start_rev:
    end_rev = start_rev = None
  return start_rev, end_rev


def _GetBotIdForRevisionNumber(row_tuples, revision_number):
  for _, row, _ in row_tuples:
    if row.revision == revision_number:
      if hasattr(row, 'swarming_bot_id') and row.swarming_bot_id:
        return row.swarming_bot_id
  return None


@ndb.tasklet
def _MakeAnomalyEntity(change_point, test, stat, rows, config, matching_sub):
  """Creates an Anomaly entity.

  Args:
    change_point: A find_change_points.ChangePoint object.
    test: The TestMetadata entity that the anomalies were found on.
    stat: The TestMetadata stat that the anomaly was found on.
    rows: List of (revision, graph_data.Row, value) tuples that the anomalies were found on.
    config: A dict representing the anomaly detection configuration
        parameters used to produce this anomaly.
    matching_sub: A subscription to which this anomaly is associated.

  Returns:
    An Anomaly entity, which is not yet put in the datastore.
  """
  end_rev = change_point.extended_end
  start_rev = _GetImmediatelyPreviousRevisionNumber(
      change_point.extended_start, rows) + 1
  print(change_point.extended_start, change_point.extended_end)
  display_start = display_end = None
  if test.master_name == 'ClankInternal':
    display_start, display_end = _GetDisplayRange(change_point.x_value, rows)
  median_before = change_point.median_before
  median_after = change_point.median_after
  bot_id_before = _GetBotIdForRevisionNumber(rows, change_point.extended_start)
  bot_id_after = _GetBotIdForRevisionNumber(rows, change_point.extended_end)

  bot_id_before_start_rev = _GetBotIdForRevisionNumber(rows, start_rev - 1)
  logging.debug('bot_id_before: %s, new_bot_id_before: %s, bot_id_after: %s',
                bot_id_before, bot_id_before_start_rev, bot_id_after)
  logging.debug('start_rev: %s, extended_start: %s, end_rev: %s',
                start_rev, change_point.extended_start, end_rev)

  suite_key = test.key.id().split('/')[:3]
  suite_key = '/'.join(suite_key)
  suite_key = utils.TestKey(suite_key)

  # Apply the bot_id_before_start_rev
  bot_id_before = bot_id_before_start_rev
  if bot_id_after is not None and bot_id_before is not None and bot_id_after != bot_id_before:
    logging.info(
        'ignoring anomaly for %s, range %s-%s, reason: swarming bot id changed (%s vs %s)',
        utils.TestPath(suite_key), change_point.extended_start,
        change_point.extended_end, bot_id_before, bot_id_after)
    raise ndb.Return(None)

  queried_diagnostics = yield (
      histogram.SparseDiagnostic.GetMostRecentDataByNamesAsync(
          suite_key, {
              reserved_infos.BUG_COMPONENTS.name,
              reserved_infos.OWNERS.name,
              reserved_infos.INFO_BLURB.name,
              reserved_infos.ALERT_GROUPING.name,
          }))

  bug_components = queried_diagnostics.get(reserved_infos.BUG_COMPONENTS.name,
                                           {}).get('values')
  if bug_components:
    bug_components = bug_components[0]
    # TODO(902796): Remove this typecheck.
    if isinstance(bug_components, list):
      bug_components = bug_components[0]

  ownership_information = {
      'emails':
          queried_diagnostics.get(reserved_infos.OWNERS.name, {}).get('values'),
      'component':
          bug_components,

      # Info blurbs should be a single string, and we'll only take the firs
      #  element of the list of values.
      'info_blurb':
          queried_diagnostics.get(reserved_infos.INFO_BLURB.name,
                                  {}).get('values', [None])[0],
  }

  alert_grouping = queried_diagnostics.get(reserved_infos.ALERT_GROUPING.name,
                                           {}).get('values', [])

  # Compute additional anomaly metadata.
  def MinMax(iterable):
    min_ = max_ = None
    for val in iterable:
      if min_ is None:
        min_ = max_ = val
      else:
        min_ = min(min_, val)
        max_ = max(max_, val)
    return min_, max_

  earliest_input_timestamp, latest_input_timestamp = MinMax(
      r.timestamp for unused_rev, r, unused_val in rows)

  new_anomaly = anomaly.Anomaly(
      start_revision=start_rev,
      end_revision=end_rev,
      median_before_anomaly=median_before,
      median_after_anomaly=median_after,
      segment_size_before=change_point.size_before,
      segment_size_after=change_point.size_after,
      window_end_revision=change_point.window_end,
      std_dev_before_anomaly=change_point.std_dev_before,
      t_statistic=change_point.t_statistic,
      degrees_of_freedom=change_point.degrees_of_freedom,
      p_value=change_point.p_value,
      is_improvement=_IsImprovement(test, median_before, median_after),
      ref_test=_GetRefBuildKeyForTest(test),
      test=test.key,
      statistic=stat,
      internal_only=test.internal_only,
      units=test.units,
      display_start=display_start,
      display_end=display_end,
      ownership=ownership_information,
      alert_grouping=alert_grouping,
      earliest_input_timestamp=earliest_input_timestamp,
      latest_input_timestamp=latest_input_timestamp,
      anomaly_config=config,
      matching_subscription=matching_sub,
  )
  raise ndb.Return(new_anomaly)


def FindChangePointsForTest(rows, config_dict):
  """Gets the anomaly data from the anomaly detection module.

  Args:
    rows: The Row entities to find anomalies for, sorted backwards by revision.
    config_dict: Anomaly threshold parameters as a dictionary.

  Returns:
    A list of find_change_points.ChangePoint objects.
  """
  data_series = [(revision, value) for (revision, _, value) in rows]
  return find_change_points.FindChangePoints(data_series, **config_dict)


def _IsImprovement(test, median_before, median_after):
  """Returns whether the alert is an improvement for the given test.

  Args:
    test: TestMetadata to get the improvement direction for.
    median_before: The median of the segment immediately before the anomaly.
    median_after: The median of the segment immediately after the anomaly.

  Returns:
    True if it is improvement anomaly, otherwise False.
  """
  if (median_before < median_after
      and test.improvement_direction == anomaly.UP):
    return True
  if (median_before >= median_after
      and test.improvement_direction == anomaly.DOWN):
    return True
  return False
