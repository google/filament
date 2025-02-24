# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint to add new histograms to the datastore."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging
import six
import sys
import uuid
from six.moves import zip_longest

from google.appengine.ext import ndb

from dashboard import add_point
from dashboard import add_point_queue
from dashboard import find_anomalies
from dashboard import graph_revisions
from dashboard import sheriff_config_client
from dashboard.common import datastore_hooks
from dashboard.common import histogram_helpers
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models import upload_completion_token
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import reserved_infos

from flask import request, make_response


# Note: annotation names should shorter than add_point._MAX_COLUMN_NAME_LENGTH.
DIAGNOSTIC_NAMES_TO_ANNOTATION_NAMES = {
    reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
        'r_commit_pos',
    reserved_infos.V8_COMMIT_POSITIONS.name:
        'r_v8_commit_pos',
    reserved_infos.CHROMIUM_REVISIONS.name:
        'r_chromium',
    reserved_infos.V8_REVISIONS.name:
        'r_v8_rev',
    # TODO(#3545): Add r_catapult_git to Dashboard revision_info map.
    reserved_infos.CATAPULT_REVISIONS.name:
        'r_catapult_git',
    reserved_infos.ANGLE_REVISIONS.name:
        'r_angle_git',
    reserved_infos.WEBRTC_REVISIONS.name:
        'r_webrtc_git',
    reserved_infos.WEBRTC_INTERNAL_SIRIUS_REVISIONS.name:
        'r_webrtc_sirius_cl',
    reserved_infos.WEBRTC_INTERNAL_VEGA_REVISIONS.name:
        'r_webrtc_vega_cl',
    reserved_infos.WEBRTC_INTERNAL_CANOPUS_REVISIONS.name:
        'r_webrtc_canopus_cl',
    reserved_infos.WEBRTC_INTERNAL_ARCTURUS_REVISIONS.name:
        'r_webrtc_arcturus_cl',
    reserved_infos.WEBRTC_INTERNAL_RIGEL_REVISIONS.name:
        'r_webrtc_rigel_cl',
    reserved_infos.FUCHSIA_GARNET_REVISIONS.name:
        'r_fuchsia_garnet_git',
    reserved_infos.FUCHSIA_PERIDOT_REVISIONS.name:
        'r_fuchsia_peridot_git',
    reserved_infos.FUCHSIA_TOPAZ_REVISIONS.name:
        'r_fuchsia_topaz_git',
    reserved_infos.FUCHSIA_ZIRCON_REVISIONS.name:
        'r_fuchsia_zircon_git',
    reserved_infos.REVISION_TIMESTAMPS.name:
        'r_revision_timestamp',
    reserved_infos.FUCHSIA_INTEGRATION_INTERNAL_REVISIONS.name:
        'r_fuchsia_integ_int_git',
    reserved_infos.FUCHSIA_INTEGRATION_PUBLIC_REVISIONS.name:
        'r_fuchsia_integ_pub_git',
    reserved_infos.FUCHSIA_SMART_INTEGRATION_REVISIONS.name:
        'r_fuchsia_smart_integ_git',
}


class BadRequestError(Exception):
  pass


def _CheckRequest(condition, msg):
  if not condition:
    raise BadRequestError(msg)


def AddHistogramsQueuePost():
  """Adds a single histogram or sparse shared diagnostic to the datastore.

  The |data| request parameter can be either a histogram or a sparse shared
  diagnostic; the set of diagnostics that are considered sparse (meaning that
  they don't normally change on every upload for a given benchmark from a
  given bot) is shown in histogram_helpers.SPARSE_DIAGNOSTIC_TYPES.

  See https://goo.gl/lHzea6 for detailed information on the JSON format for
  histograms and diagnostics.

  This request handler is intended to be used only by requests using the
  task queue; it shouldn't be directly from outside.

  Request parameters:
    data: JSON encoding of a histogram or shared diagnostic.
    revision: a revision, given as an int.
    test_path: the test path to which this diagnostic or histogram should be
        attached.
  """
  datastore_hooks.SetPrivilegedRequest()

  params = json.loads(request.get_data())

  _PrewarmGets(params)

  # Roughly, the processing of histograms and the processing of rows can be
  # done in parallel since there are no dependencies.

  histogram_futures = []
  token_state_futures = []

  try:
    for p in params:
      histogram_futures.append((p, _ProcessRowAndHistogram(p)))

  except Exception as e:  # pylint: disable=broad-except
    for param, futures_info in zip_longest(params, histogram_futures):
      if futures_info is not None:
        continue
      token_state_futures.append(
          upload_completion_token.Measurement.UpdateStateByPathAsync(
              param.get('test_path'), param.get('token'),
              upload_completion_token.State.FAILED, str(e)))
    ndb.Future.wait_all(token_state_futures)
    raise

  for info, futures in histogram_futures:
    operation_state = upload_completion_token.State.COMPLETED
    error_message = None
    for f in futures:
      exception = f.get_exception()
      if exception is not None:
        operation_state = upload_completion_token.State.FAILED
        error_message = str(exception)
    token_state_futures.append(
        upload_completion_token.Measurement.UpdateStateByPathAsync(
            info.get('test_path'), info.get('token'), operation_state,
            error_message))
  ndb.Future.wait_all(token_state_futures)
  return make_response('')


def _GetStoryFromDiagnosticsDict(diagnostics):
  if not diagnostics:
    return None

  story_name = diagnostics.get(reserved_infos.STORIES.name)
  if not story_name:
    return None

  story_name = diagnostic.Diagnostic.FromDict(story_name)
  if story_name and len(story_name) == 1:
    return story_name.GetOnlyElement()
  return None


def _PrewarmGets(params):
  keys = set()

  for p in params:
    test_path = p['test_path']
    path_parts = test_path.split('/')

    keys.add(ndb.Key('Master', path_parts[0]))
    keys.add(ndb.Key('Bot', path_parts[1]))

    test_parts = path_parts[2:]
    test_key = '%s/%s' % (path_parts[0], path_parts[1])
    for test_part in test_parts:
      test_key += '/%s' % test_part
      keys.add(ndb.Key('TestMetadata', test_key))

  ndb.get_multi_async(list(keys))


def _ProcessRowAndHistogram(params):
  revision = int(params['revision'])
  test_path = params['test_path']
  benchmark_description = params['benchmark_description']
  data_dict = params['data']

  # Disable this log since it's killing the quota of Cloud Logging API -
  # write requests per minute
  # logging.info('Processing: %s', test_path)

  hist = histogram_module.Histogram.FromDict(data_dict)

  if hist.num_values == 0:
    return []

  test_path_parts = test_path.split('/')
  master = test_path_parts[0]
  bot = test_path_parts[1]
  benchmark_name = test_path_parts[2]
  histogram_name = test_path_parts[3]
  if len(test_path_parts) > 4:
    rest = '/'.join(test_path_parts[4:])
  else:
    rest = None
  full_test_name = '/'.join(test_path_parts[2:])
  internal_only = graph_data.Bot.GetInternalOnlySync(master, bot)
  extra_args = GetUnitArgs(hist.unit)

  unescaped_story_name = _GetStoryFromDiagnosticsDict(params.get('diagnostics'))

  parent_test = add_point_queue.GetOrCreateAncestors(
      master,
      bot,
      full_test_name,
      internal_only=internal_only,
      unescaped_story_name=unescaped_story_name,
      benchmark_description=benchmark_description,
      **extra_args)
  test_key = parent_test.key

  statistics_scalars = hist.statistics_scalars
  legacy_parent_tests = {}

  # TODO(#4213): Stop doing this.
  if histogram_helpers.IsLegacyBenchmark(benchmark_name):
    statistics_scalars = {}

  for stat_name, scalar in statistics_scalars.items():
    if histogram_helpers.ShouldFilterStatistic(histogram_name, benchmark_name,
                                               stat_name):
      continue
    extra_args = GetUnitArgs(scalar.unit)
    suffixed_name = '%s/%s_%s' % (benchmark_name, histogram_name, stat_name)
    if rest is not None:
      suffixed_name += '/' + rest
    legacy_parent_tests[stat_name] = add_point_queue.GetOrCreateAncestors(
        master,
        bot,
        suffixed_name,
        internal_only=internal_only,
        unescaped_story_name=unescaped_story_name,
        **extra_args)

  return [
      _AddRowsFromData(params, revision, parent_test, legacy_parent_tests),
      _AddHistogramFromData(params, revision, test_key, internal_only)
  ]


@ndb.tasklet
def _AddRowsFromData(params, revision, parent_test, legacy_parent_tests):
  data_dict = params['data']
  test_key = parent_test.key

  stat_names_to_test_keys = {k: v.key for k, v in legacy_parent_tests.items()}
  row, stat_name_row_dict = _CreateRowEntitiesInternal(data_dict, test_key,
                                                       stat_names_to_test_keys,
                                                       revision)
  if not row:
    raise ndb.Return()

  rows = [row]
  if stat_name_row_dict:
    rows.extend(stat_name_row_dict.values())
  yield ndb.put_multi_async(rows) + [r.UpdateParentAsync() for r in rows]

  logging.info('Added %s rows to Datastore', str(len(rows)))

  def IsMonitored(client, test):
    reason = []
    has_subscribers = utils.IsMonitored(client, test.test_path)
    if not has_subscribers:
      reason.append('subscriptions')
    if not test.has_rows:
      reason.append('has_rows')
    if reason:
      # Disable this log since it's killing the quota of Cloud Logging API -
      # write requests per minute
      # logging.info('Skip test: %s reason=%s', test.key, ','.join(reason))
      return False
    logging.info('Process test: %s', test.key)
    return True

  client = sheriff_config_client.GetSheriffConfigClient()
  tests_keys = []
  if IsMonitored(client, parent_test):
    tests_keys.append(parent_test.key)

  for legacy_parent_test in legacy_parent_tests.values():
    if IsMonitored(client, legacy_parent_test):
      tests_keys.append(legacy_parent_test.key)

  tests_keys = [k for k in tests_keys if not add_point_queue.IsRefBuild(k)]

  # Updating of the cached graph revisions should happen after put because
  # it requires the new row to have a timestamp, which happens upon put.
  futures = [
      graph_revisions.AddRowsToCacheAsync(rows),
      find_anomalies.ProcessTestsAsync(tests_keys)
  ]
  yield futures


@ndb.tasklet
def _AddHistogramFromData(params, revision, test_key, internal_only):
  data_dict = params['data']
  diagnostics = params.get('diagnostics')
  new_guids_to_existing_diagnostics = yield ProcessDiagnostics(
      diagnostics, revision, test_key, internal_only)

  hs = histogram_set.HistogramSet()
  hs.ImportDicts([data_dict])
  for new_guid, existing_diagnostic in (iter(
      new_guids_to_existing_diagnostics.items())):
    hs.ReplaceSharedDiagnostic(
        new_guid, diagnostic_ref.DiagnosticRef(existing_diagnostic['guid']))
  data = hs.GetFirstHistogram().AsDict()

  entity = histogram.Histogram(
      id=str(uuid.uuid4()),
      data=data,
      test=test_key,
      revision=revision,
      internal_only=internal_only)
  yield entity.put_async()

  measurement = upload_completion_token.Measurement.GetByPath(
      params.get('test_path'), params.get('token'))
  if measurement is not None:
    measurement.histogram = entity.key
    measurement.put()


@ndb.tasklet
def ProcessDiagnostics(diagnostic_data, revision, test_key, internal_only):
  if not diagnostic_data:
    raise ndb.Return({})

  diagnostic_entities = []
  for name, diagnostic_datum in diagnostic_data.items():
    guid = diagnostic_datum['guid']
    diagnostic_entities.append(
        histogram.SparseDiagnostic(
            id=guid,
            name=name,
            data=diagnostic_datum,
            test=test_key,
            start_revision=revision,
            end_revision=sys.maxsize,
            internal_only=internal_only))

  suite_key = utils.TestKey('/'.join(test_key.id().split('/')[:3]))
  last_added = yield histogram.HistogramRevisionRecord.GetLatest(suite_key)
  assert last_added

  new_guids_to_existing_diagnostics = yield (
      histogram.SparseDiagnostic.FindOrInsertDiagnostics(
          diagnostic_entities, test_key, revision, last_added.revision))

  raise ndb.Return(new_guids_to_existing_diagnostics)


def GetUnitArgs(unit):
  unit_args = {'units': unit}
  histogram_improvement_direction = unit.split('_')[-1]
  if histogram_improvement_direction == 'biggerIsBetter':
    unit_args['improvement_direction'] = anomaly.UP
  elif histogram_improvement_direction == 'smallerIsBetter':
    unit_args['improvement_direction'] = anomaly.DOWN
  else:
    unit_args['improvement_direction'] = anomaly.UNKNOWN
  return unit_args

def CreateRowEntities(histogram_dict, test_metadata_key,
                      stat_names_to_test_keys, revision):
  row, stat_name_row_dict = _CreateRowEntitiesInternal(histogram_dict,
                                                       test_metadata_key,
                                                       stat_names_to_test_keys,
                                                       revision)

  if not row:
    return None

  rows = [row]
  if stat_name_row_dict:
    rows.extend(stat_name_row_dict.values())

  return rows


def _CreateRowEntitiesInternal(histogram_dict, test_metadata_key,
                               stat_names_to_test_keys, revision):
  h = histogram_module.Histogram.FromDict(histogram_dict)
  # TODO(#3564): Move this check into _PopulateNumericalFields once we
  # know that it's okay to put rows that don't have a value/error.
  if h.num_values == 0:
    return None, None

  row_dict = _MakeRowDict(revision, test_metadata_key.id(), h)
  parent_test_key = utils.GetTestContainerKey(test_metadata_key)
  row = graph_data.Row(
      id=revision,
      parent=parent_test_key,
      **add_point.GetAndValidateRowProperties(row_dict))

  stat_name_row_dict = {}
  for stat_name, suffixed_key in stat_names_to_test_keys.items():
    suffixed_parent_test_key = utils.GetTestContainerKey(suffixed_key)
    row_dict = _MakeRowDict(revision, suffixed_key.id(), h, stat_name=stat_name)
    new_row = graph_data.Row(
        id=revision,
        parent=suffixed_parent_test_key,
        **add_point.GetAndValidateRowProperties(row_dict))
    stat_name_row_dict[stat_name] = new_row

  return row, stat_name_row_dict


def _MakeRowDict(revision, test_path, tracing_histogram, stat_name=None):
  d = {}
  test_parts = test_path.split('/')
  d['master'] = test_parts[0]
  d['bot'] = test_parts[1]
  d['test'] = '/'.join(test_parts[2:])
  d['revision'] = revision
  d['supplemental_columns'] = {}

  # TODO(#3628): Remove this annotation when the frontend displays the full
  # histogram and all its diagnostics including the full set of trace urls.
  trace_url_set = tracing_histogram.diagnostics.get(
      reserved_infos.TRACE_URLS.name)
  # We don't show trace URLs for summary values in the legacy pipeline
  is_summary = reserved_infos.SUMMARY_KEYS.name in tracing_histogram.diagnostics
  if trace_url_set and not is_summary:
    d['supplemental_columns']['a_tracing_uri'] = list(trace_url_set)[-1]

  try:
    bot_id_name = tracing_histogram.diagnostics.get(
        reserved_infos.BOT_ID.name)
    if bot_id_name:
      bot_id_names = list(bot_id_name)
      d['supplemental_columns']['a_bot_id'] = bot_id_names
      if len(bot_id_names) == 1:
        d['swarming_bot_id'] = bot_id_names[0]

  except Exception as e: # pylint: disable=broad-except
    logging.warning('bot_id failed. Error: %s', e)
  try:
    os_detail_vers = tracing_histogram.diagnostics.get(
        reserved_infos.OS_DETAILED_VERSIONS.name)
    if os_detail_vers:
      d['supplemental_columns']['a_os_detail_vers'] = list(os_detail_vers)
  except Exception as e: # pylint: disable=broad-except
    logging.warning('os_detail_vers failed. Error: %s', e)

  for diag_name, annotation in DIAGNOSTIC_NAMES_TO_ANNOTATION_NAMES.items():
    revision_info = tracing_histogram.diagnostics.get(diag_name)
    if not revision_info:
      continue
    if diag_name == reserved_infos.REVISION_TIMESTAMPS.name:
      value = [revision_info.min_timestamp]
    else:
      value = list(revision_info)
    _CheckRequest(
        len(value) == 1,
        'RevisionInfo fields must contain at most one revision')

    d['supplemental_columns'][annotation] = value[0]

  _AddStdioUris(tracing_histogram, d)

  if stat_name is not None:
    d['value'] = tracing_histogram.statistics_scalars[stat_name].value
    d['error'] = 0.0
    if stat_name == 'avg':
      d['error'] = tracing_histogram.standard_deviation
  else:
    _PopulateNumericalFields(d, tracing_histogram)
  return d


def _AddStdioUris(tracing_histogram, row_dict):
  build_urls_diagnostic = tracing_histogram.diagnostics.get(
      reserved_infos.BUILD_URLS.name)
  if build_urls_diagnostic:
    build_tuple = build_urls_diagnostic.GetOnlyElement()
    if isinstance(build_tuple, list):
      link = '[%s](%s)' % tuple(build_tuple)
      row_dict['supplemental_columns']['a_build_uri'] = link

  log_urls = tracing_histogram.diagnostics.get(reserved_infos.LOG_URLS.name)
  if not log_urls:
    return

  if len(log_urls) == 1:
    _AddStdioUri('a_stdio_uri', log_urls.GetOnlyElement(), row_dict)


def _AddStdioUri(name, link_list, row_dict):
  # TODO(#4397): Change this to an assert after infra-side fixes roll
  if isinstance(link_list, list):
    row_dict['supplemental_columns'][name] = '[%s](%s)' % tuple(link_list)
  # Support busted format until infra changes roll
  elif isinstance(link_list, six.string_types):
    row_dict['supplemental_columns'][name] = link_list


def _PopulateNumericalFields(row_dict, tracing_histogram):
  statistics_scalars = tracing_histogram.statistics_scalars
  for name, scalar in statistics_scalars.items():
    # We'll skip avg/std since these are already stored as value/error in rows.
    if name in ('avg', 'std'):
      continue

    row_dict['supplemental_columns']['d_%s' % name] = scalar.value

  row_dict['value'] = tracing_histogram.average
  row_dict['error'] = tracing_histogram.standard_deviation
