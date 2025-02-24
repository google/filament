# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
from flask import Flask
import itertools
import json
from unittest import mock
import random
import six
import string
import sys
import unittest
import webtest
import zlib

from google.appengine.ext import ndb

from dashboard import add_histograms
from dashboard import add_histograms_queue
from dashboard import uploads_info
from dashboard.api import api_auth
from dashboard.api import api_request_handler
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models import upload_completion_token
from dashboard.sheriff_config_client import SheriffConfigClient
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import breakdown
from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

# pylint: disable=too-many-lines


def SetGooglerOAuth(mock_oauth):
  mock_oauth.get_current_user.return_value = testing_common.INTERNAL_USER
  mock_oauth.get_client_id.return_value = api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0]


def _CreateHistogram(name='hist',
                     master=None,
                     bot=None,
                     benchmark=None,
                     device=None,
                     owner=None,
                     stories=None,
                     story_tags=None,
                     benchmark_description=None,
                     commit_position=None,
                     summary_options=None,
                     samples=None,
                     max_samples=None,
                     is_ref=False,
                     is_summary=None,
                     point_id=None,
                     build_url=None,
                     revision_timestamp=None):
  hists = [histogram_module.Histogram(name, 'count')]
  if max_samples:
    hists[0].max_num_sample_values = max_samples
  if summary_options:
    hists[0].CustomizeSummaryOptions(summary_options)
  if samples:
    for s in samples:
      hists[0].AddSample(s)

  histograms = histogram_set.HistogramSet(hists)
  if master:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.MASTERS.name, generic_set.GenericSet([master]))
  if bot:
    histograms.AddSharedDiagnosticToAllHistograms(reserved_infos.BOTS.name,
                                                  generic_set.GenericSet([bot]))
  if commit_position:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
        generic_set.GenericSet([commit_position]))
  if benchmark:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARKS.name, generic_set.GenericSet([benchmark]))
  if benchmark_description:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARK_DESCRIPTIONS.name,
        generic_set.GenericSet([benchmark_description]))
  if owner:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.OWNERS.name, generic_set.GenericSet([owner]))
  if device:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.DEVICE_IDS.name, generic_set.GenericSet([device]))
  if stories:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(stories))
  if story_tags:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(story_tags))
  if is_ref:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.IS_REFERENCE_BUILD.name, generic_set.GenericSet([True]))
  if is_summary is not None:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.SUMMARY_KEYS.name, generic_set.GenericSet(is_summary))
  if point_id is not None:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.POINT_ID.name, generic_set.GenericSet([point_id]))
  if build_url is not None:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BUILD_URLS.name,
        generic_set.GenericSet([['build', build_url]]))
  if revision_timestamp is not None:
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.REVISION_TIMESTAMPS.name,
        date_range.DateRange(revision_timestamp))
  return histograms


class BufferedFakeFile:

  def __init__(self, data=b''):
    self.data = data
    self.position = 0

  def read(self, size=None):  # pylint: disable=invalid-name
    if self.position == len(self.data):
      return b''
    if size is None or size < 0:
      result = self.data[self.position:]
      self.position = len(self.data)
      return six.ensure_binary(result)
    if size > len(self.data) + self.position:
      result = self.data[self.position:]
      self.position = len(self.data)
      return six.ensure_binary(result)

    current_position = self.position
    self.position += size
    result = self.data[current_position:self.position]
    return six.ensure_binary(result)

  def write(self, data):  # pylint: disable=invalid-name
    self.data += data
    return len(data)

  def close(self):  # pylint: disable=invalid-name
    pass

  def __exit__(self, *args):
    self.close()
    return False

  def __enter__(self):
    return self


flask_app = Flask(__name__)


@flask_app.route('/add_histograms', methods=['POST'])
def AddHistogramsPost():
  return add_histograms.AddHistogramsPost()


@flask_app.route('/add_histograms/process', methods=['POST'])
def AddHistogramsProcessPost():
  return add_histograms.AddHistogramsProcessPost()


@flask_app.route('/add_histograms_queue', methods=['GET', 'POST'])
def AddHistogramsQueuePost():
  return add_histograms_queue.AddHistogramsQueuePost()


@flask_app.route('/uploads/<token_id>')
def UploadsInfoGet(token_id):
  return uploads_info.UploadsInfoGet(token_id)


class AddHistogramsBaseTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    testing_common.SetIsInternalUser('foo@bar.com', True)
    self.SetCurrentUser('foo@bar.com', is_admin=True)
    oauth_patcher = mock.patch.object(api_auth, 'oauth')
    self.addCleanup(oauth_patcher.stop)
    SetGooglerOAuth(oauth_patcher.start())

    identity_patcher = mock.patch.object(utils, 'app_identity')
    self.addCleanup(identity_patcher.stop)
    self.mock_utils = identity_patcher.start()
    self.mock_utils.get_application_id.return_value = 'chromeperf'

    patcher = mock.patch.object(add_histograms, 'cloudstorage')
    self.mock_cloudstorage = patcher.start()
    self.addCleanup(patcher.stop)

    patcher = mock.patch('logging.error')
    self.mock_error = patcher.start()
    self.addCleanup(patcher.stop)

  def PostAddHistogram(self, data, status=200):
    mock_obj = mock.MagicMock(wraps=BufferedFakeFile())
    self.mock_cloudstorage.open.return_value = mock_obj

    r = self.testapp.post('/add_histograms', data, status=status)
    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    return r

  def PostAddHistogramProcess(self, data):
    data = six.ensure_binary(data)
    mock_read = mock.MagicMock(wraps=BufferedFakeFile(zlib.compress(data)))
    self.mock_cloudstorage.open.return_value = mock_read

    # TODO(simonhatch): Should we surface the error somewhere that can be
    # retrieved by the uploader?

    r = self.testapp.post('/add_histograms/process',
                          json.dumps({'gcs_file_path': ''}))
    self.assertTrue(self.mock_error.called)
    return r


#TODO(fancl): mocking Match to return some actuall result
@mock.patch.object(SheriffConfigClient, '__init__',
                   mock.MagicMock(return_value=None))
@mock.patch.object(SheriffConfigClient, 'Match',
                   mock.MagicMock(return_value=([], None)))
@mock.patch('dashboard.services.skia_bridge_service.SkiaServiceClient',
            mock.MagicMock())
class AddHistogramsEndToEndTest(AddHistogramsBaseTest):

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_Succeeds(self, mock_process_test, mock_graph_revisions):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=123,
        benchmark_description='Benchmark description.',
        samples=[1, 2, 3])
    data = json.dumps(hs.AsDicts())

    post_histogram_res = self.PostAddHistogram({'data': data})
    upload_token = json.loads(post_histogram_res.body)
    self.assertIsNotNone(upload_token)
    self.assertTrue('token' in upload_token)
    self.assertTrue('file' in upload_token)

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(4, len(diagnostics))
    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    tests = graph_data.TestMetadata.query().fetch()

    self.assertEqual('Benchmark description.', tests[0].description)

    # Verify that an anomaly processing was called.
    self.assertEqual(mock_process_test.call_count, 1)
    rows = graph_data.Row.query().fetch()
    # We want to verify that the method was called with all rows that have
    # been added, but the ordering will be different because we produce
    # the rows by iterating over a dict.
    mock_graph_revisions.assert_called_once_with(mock.ANY)
    self.assertEqual(len(mock_graph_revisions.mock_calls[0][1][0]), len(rows))

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_ZlibSucceeds(self, mock_process_test, mock_graph_revisions):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=123,
        benchmark_description='Benchmark description.',
        samples=[1, 2, 3])
    data = zlib.compress(six.ensure_binary(json.dumps(hs.AsDicts())))

    self.PostAddHistogram(data)
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(4, len(diagnostics))
    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    tests = graph_data.TestMetadata.query().fetch()

    self.assertEqual('Benchmark description.', tests[0].description)

    # Verify that an anomaly processing was called.
    self.assertEqual(mock_process_test.call_count, 1)

    rows = graph_data.Row.query().fetch()
    # We want to verify that the method was called with all rows that have
    # been added, but the ordering will be different because we produce
    # the rows by iterating over a dict.
    mock_graph_revisions.assert_called_once_with(mock.ANY)
    self.assertEqual(len(mock_graph_revisions.mock_calls[0][1][0]), len(rows))

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_BuildUrls_Added(self):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=123,
        benchmark_description='Benchmark description.',
        samples=[1, 2, 3],
        build_url='http://foo')
    data = zlib.compress(six.ensure_binary(json.dumps(hs.AsDicts())))

    self.PostAddHistogram(data)
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    rows = graph_data.Row.query().fetch()
    self.assertEqual(rows[0].a_build_uri, '[build](http://foo)')

  def testPost_NotZlib_Fails(self):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=123,
        benchmark_description='Benchmark description.',
        samples=[1, 2, 3])
    data = json.dumps(hs.AsDicts())

    self.PostAddHistogram(data, status=400)

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_BenchmarkTotalDuration_Filtered(self):
    hs = _CreateHistogram(
        name='benchmark_total_duration',
        master='master',
        bot='bot',
        benchmark='v8.browsing',
        commit_position=123,
        samples=[1],
        is_ref=True)
    data = json.dumps(hs.AsDicts())

    self.testapp.post('/add_histograms', {'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    tests = graph_data.TestMetadata.query().fetch()
    for t in tests:
      parts = t.key.id().split('/')
      if len(parts) <= 3:
        continue
      self.assertEqual('benchmark_total_duration', parts[3])

  def testPost_PurgesBinData(self):
    hs = _CreateHistogram(master='m', bot='b', benchmark='s', commit_position=1)
    b = breakdown.Breakdown()
    dm = histogram_module.DiagnosticMap()
    dm['breakdown'] = b
    hs.GetFirstHistogram().AddSample(0, dm)
    data = json.dumps(hs.AsDicts())

    self.PostAddHistogram({'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    histograms = histogram.Histogram.query().fetch()
    hist = histogram_module.Histogram.FromDict(histograms[0].data)
    for b in hist.bins:
      for dm in b.diagnostic_maps:
        self.assertEqual(0, len(dm))

  def testPost_IllegalMasterName_Fails(self):
    hs = _CreateHistogram(
        master='m/m', bot='b', benchmark='s', commit_position=1, samples=[1])
    data = json.dumps(hs.AsDicts())

    response = self.PostAddHistogramProcess(data)
    self.assertIn(b'Illegal slash', response.body)

  def testPost_IllegalBotName_Fails(self):
    hs = _CreateHistogram(
        master='m', bot='b/b', benchmark='s', commit_position=1, samples=[1])
    data = json.dumps(hs.AsDicts())

    response = self.PostAddHistogramProcess(data)
    self.assertIn(b'Illegal slash', response.body)

  def testPost_IllegalSuiteName_Fails(self):
    hs = _CreateHistogram(
        master='m', bot='b', benchmark='s/s', commit_position=1, samples=[1])
    data = json.dumps(hs.AsDicts())

    response = self.PostAddHistogramProcess(data)
    self.assertIn(b'Illegal slash', response.body)

  def testPost_DuplicateHistogram_Fails(self):
    hs1 = _CreateHistogram(
        master='m', bot='b', benchmark='s', commit_position=1, samples=[1])
    hs = _CreateHistogram(
        master='m', bot='b', benchmark='s', commit_position=1, samples=[1])
    hs.ImportDicts(hs1.AsDicts())
    data = json.dumps(hs.AsDicts())

    response = self.PostAddHistogramProcess(data)
    self.assertIn(b'Duplicate histogram detected', response.body)

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  @mock.patch.object(add_histograms_queue, 'CreateRowEntities',
                     mock.MagicMock(return_value=None))
  def testPost_EmptyHistogram_NotAdded(self, mock_process_test,
                                       mock_graph_revisions):
    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        name='foo',
        commit_position=1,
        samples=[])
    data = json.dumps(hs.AsDicts())

    self.testapp.post('/add_histograms', {'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    rows = graph_data.Row.query().fetch()
    self.assertEqual(0, len(rows))

    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(0, len(histograms))

    self.assertFalse(mock_process_test.called)
    self.assertFalse(mock_graph_revisions.called)

  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_TestNameEndsWithUnderscoreRef_ProcessTestIsNotCalled(
      self, mock_process_test):
    """Tests that Tests ending with "_ref" aren't analyzed for Anomalies."""
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=424242,
        stories=['abcd'],
        samples=[1, 2, 3],
        is_ref=True)
    data = json.dumps(hs.AsDicts())
    self.PostAddHistogram({'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    mock_process_test.assert_called_once_with([])

  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_TestNameEndsWithSlashRef_ProcessTestIsNotCalled(
      self, mock_process_test):
    """Tests that leaf tests named ref aren't added to the task queue."""
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=424242,
        stories=['ref'],
        samples=[1, 2, 3])
    data = json.dumps(hs.AsDicts())
    self.PostAddHistogram({'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    mock_process_test.assert_called_once_with([])

  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_TestNameEndsContainsButDoesntEndWithRef_ProcessTestIsCalled(
      self, mock_process_test):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=424242,
        stories=['_ref_abcd'],
        samples=[1, 2, 3])
    data = json.dumps(hs.AsDicts())
    self.PostAddHistogram({'data': data})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    self.assertTrue(mock_process_test.called)

  # (crbug/1403845): Routing is broken after ExecuteTaskQueueTasks is called.
  @unittest.skipIf(six.PY3, '''
    http requests after ExecuteTaskQueueTasks are not routed correctly for py3.
    ''')
  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_DeduplicateByName(self):
    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        stories=['s1', 's2'],
        commit_position=1111,
        device='device1',
        owner='owner1',
        samples=[42])
    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    # There should be the 4 suite level and 2 histogram level diagnostics
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(6, len(diagnostics))

    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        stories=['s1', 's2'],
        commit_position=1112,
        device='device1',
        owner='owner1',
        samples=[42])
    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    # There should STILL be the 4 suite level and 2 histogram level diagnostics
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(6, len(diagnostics))

    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        stories=['s1', 's2'],
        commit_position=1113,
        device='device2',
        owner='owner1',
        samples=[42])
    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    # There should one additional device diagnostic since that changed
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(7, len(diagnostics))

    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        stories=['s1', 's2'],
        commit_position=1114,
        device='device2',
        owner='owner2')
    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    # Now there's an additional owner diagnostic
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(8, len(diagnostics))

    hs = _CreateHistogram(
        master='m',
        bot='b',
        benchmark='s',
        stories=['s1', 's2'],
        commit_position=1115,
        device='device2',
        owner='owner2',
        samples=[42])
    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    # No more new diagnostics
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(8, len(diagnostics))

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_NamesAreSet(self):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345,
        device='foo',
        samples=[42])

    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(4, len(diagnostics))

    # The first 3 are all suite level diagnostics, the last is a histogram
    # level diagnostic.
    names = [
        reserved_infos.MASTERS.name, reserved_infos.BOTS.name,
        reserved_infos.BENCHMARKS.name, reserved_infos.DEVICE_IDS.name
    ]
    for d in diagnostics:
      self.assertIn(d.name, names)
      names.remove(d.name)

  def _TestDiagnosticsInternalOnly(self):
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345)

    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_DiagnosticsInternalOnly_False(self):
    graph_data.Bot(
        key=ndb.Key('Master', 'master', 'Bot', 'bot'),
        internal_only=False).put()
    self._TestDiagnosticsInternalOnly()

    diagnostics = histogram.SparseDiagnostic.query().fetch()
    for d in diagnostics:
      self.assertFalse(d.internal_only)

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync', mock.MagicMock())
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock())
  def testPost_DiagnosticsInternalOnly_True(self):
    self._TestDiagnosticsInternalOnly()

    diagnostics = histogram.SparseDiagnostic.query().fetch()
    for d in diagnostics:
      self.assertTrue(d.internal_only)

  def testPost_SetsCorrectTestPathForSummary(self):
    histograms = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345,
        device='device_foo',
        stories=['story'],
        story_tags=['group:media', 'case:browse'],
        is_summary=['name'],
        samples=[42])

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    tests = graph_data.TestMetadata.query().fetch()

    expected = [
        'master/bot/benchmark',
        'master/bot/benchmark/hist',
        'master/bot/benchmark/hist_avg',
        'master/bot/benchmark/hist_count',
        'master/bot/benchmark/hist_max',
        'master/bot/benchmark/hist_min',
        'master/bot/benchmark/hist_std',
        'master/bot/benchmark/hist_sum',
    ]

    for test in tests:
      self.assertIn(test.key.id(), expected)
      expected.remove(test.key.id())
    self.assertEqual(0, len(expected))

  def testPost_SetsCorrectTestPathForTIRLabelSummary(self):
    histograms = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345,
        device='device_foo',
        stories=['story'],
        story_tags=['group:media', 'case:browse'],
        is_summary=['name', 'storyTags'],
        samples=[42])

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    tests = graph_data.TestMetadata.query().fetch()

    expected = [
        'master/bot/benchmark',
        'master/bot/benchmark/hist',
        'master/bot/benchmark/hist/browse_media',
        'master/bot/benchmark/hist_avg',
        'master/bot/benchmark/hist_avg/browse_media',
        'master/bot/benchmark/hist_count',
        'master/bot/benchmark/hist_count/browse_media',
        'master/bot/benchmark/hist_max',
        'master/bot/benchmark/hist_max/browse_media',
        'master/bot/benchmark/hist_min',
        'master/bot/benchmark/hist_min/browse_media',
        'master/bot/benchmark/hist_std',
        'master/bot/benchmark/hist_std/browse_media',
        'master/bot/benchmark/hist_sum',
        'master/bot/benchmark/hist_sum/browse_media',
    ]

    for test in tests:
      self.assertIn(test.key.id(), expected)
      expected.remove(test.key.id())
    self.assertEqual(0, len(expected))

  def testPost_SetsCorrectTestPathForNonSummary(self):
    histograms = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345,
        device='device_foo',
        stories=['story'],
        is_summary=None,
        samples=[42])

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    tests = [t.key.id() for t in graph_data.TestMetadata.query().fetch()]
    self.assertEqual(15, len(tests))  # suite + hist + stats + story per stat
    self.assertIn('master/bot/benchmark/hist/story', tests)

  def testPost_SetsCorrectTestPathForSummaryAbsent(self):
    histograms = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=12345,
        device='device_foo',
        stories=['story'],
        samples=[42])

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    tests = [t.key.id() for t in graph_data.TestMetadata.query().fetch()]
    self.assertEqual(15, len(tests))
    self.assertIn('master/bot/benchmark/hist/story', tests)

  def _AddAtCommit(self, commit_position, device, owner):
    opts = {
        'avg': True,
        'std': False,
        'count': False,
        'max': False,
        'min': False,
        'sum': False
    }
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=commit_position,
        summary_options=opts,
        device=device,
        owner=owner,
        samples=[1])

    self.PostAddHistogram({'data': json.dumps(hs.AsDicts())})
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

  def _CheckOutOfOrderExpectations(self, expected):
    diags = histogram.SparseDiagnostic.query().fetch()

    for d in diags:
      if d.name not in expected:
        continue
      self.assertIn((d.start_revision, d.end_revision, d.data['values']),
                    expected[d.name])
      expected[d.name].remove(
          (d.start_revision, d.end_revision, d.data['values']))

    for k in expected.keys():
      self.assertFalse(expected[k])

  # (crbug/1403845): Routing is broken after ExecuteTaskQueueTasks is called.
  @unittest.skipIf(six.PY3, '''
    http requests after ExecuteTaskQueueTasks are not routed correctly for py3.
    ''')
  def testPost_OutOfOrder_SuiteLevel(self):
    self._AddAtCommit(1, 'd1', 'o1')
    self._AddAtCommit(10, 'd1', 'o1')
    self._AddAtCommit(20, 'd1', 'o1')
    self._AddAtCommit(30, 'd1', 'o1')
    self._AddAtCommit(15, 'd1', 'o2')

    expected = {
        'deviceIds': [(1, sys.maxsize, [u'd1'])],
        'owners': [(1, 14, [u'o1']), (15, 19, [u'o2']),
                   (20, sys.maxsize, [u'o1'])]
    }
    self._CheckOutOfOrderExpectations(expected)

  # (crbug/1403845): Routing is broken after ExecuteTaskQueueTasks is called.
  @unittest.skipIf(six.PY3, '''
    http requests after ExecuteTaskQueueTasks are not routed correctly for py3.
    ''')
  def testPost_OutOfOrder_HistogramLevel(self):
    self._AddAtCommit(1, 'd1', 'o1')
    self._AddAtCommit(10, 'd1', 'o1')
    self._AddAtCommit(20, 'd1', 'o1')
    self._AddAtCommit(30, 'd1', 'o1')
    self._AddAtCommit(15, 'd2', 'o1')

    expected = {
        'deviceIds': [(1, 14, [u'd1']), (15, 19, [u'd2']),
                      (20, sys.maxsize, [u'd1'])],
        'owners': [(1, sys.maxsize, [u'o1'])]
    }
    self._CheckOutOfOrderExpectations(expected)


@mock.patch.object(SheriffConfigClient, '__init__',
                   mock.MagicMock(return_value=None))
@mock.patch.object(SheriffConfigClient, 'Match',
                   mock.MagicMock(return_value=([], None)))
class AddHistogramsTest(AddHistogramsBaseTest):

  def TaskParams(self):
    tasks = self.GetTaskQueueTasks(add_histograms.TASK_QUEUE_NAME)
    params = []
    for task in tasks:
      params.extend(json.loads(base64.b64decode(task['body'])))
    return params

  @mock.patch.object(add_histograms, '_QueueHistogramTasks')
  def testPostHistogram_TooManyHistograms_Splits(self, mock_queue):

    def _MakeHistogram(name):
      h = histogram_module.Histogram(name, 'count')
      for i in range(100):
        h.AddSample(i)
      return h

    hists = [_MakeHistogram('hist_%d' % i) for i in range(100)]
    histograms = histogram_set.HistogramSet(hists)
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.MASTERS.name, generic_set.GenericSet(['master']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BOTS.name, generic_set.GenericSet(['bot']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
        generic_set.GenericSet([12345]))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARKS.name, generic_set.GenericSet(['benchmark']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.DEVICE_IDS.name, generic_set.GenericSet(['devie_foo']))

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})

    self.assertTrue(len(mock_queue.call_args[0][0]) > 1)

  @mock.patch.object(add_histograms, '_QueueHistogramTasks')
  def testPostHistogram_OneToOneHistogramTasks(self, mock_queue):

    def _MakeHistogram(name):
      h = histogram_module.Histogram(name, 'count')
      for i in range(100):
        h.AddSample(i)
      return h

    hists = [_MakeHistogram('hist_%d' % i) for i in range(50)]
    histograms = histogram_set.HistogramSet(hists)
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.MASTERS.name, generic_set.GenericSet(['master']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BOTS.name, generic_set.GenericSet(['bot']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
        generic_set.GenericSet([12345]))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARKS.name, generic_set.GenericSet(['benchmark']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.DEVICE_IDS.name, generic_set.GenericSet(['devie_foo']))

    self.PostAddHistogram({'data': json.dumps(histograms.AsDicts())})

    self.assertEqual(len(mock_queue.call_args[0][0]), 50)

  def testPostHistogramSetsTestPathAndRevision(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'values': ['story'],
        'guid': 'dc894bd9-0b73-4400-9d95-b21ee371031d',
        'type': 'GenericSet',
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.STORIES.name:
                'dc894bd9-0b73-4400-9d95-b21ee371031d',
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
            reserved_infos.STORIES.name:
                'dc894bd9-0b73-4400-9d95-b21ee371031d',
        },
        'name': 'foo2',
        'unit': 'count'
    }])
    self.PostAddHistogram({'data': data})
    params = self.TaskParams()

    self.assertEqual(2, len(params))
    self.assertEqual(424242, params[0]['revision'])
    self.assertEqual(424242, params[1]['revision'])
    paths = set(p['test_path'] for p in params)
    self.assertIn('master/bot/benchmark/foo/story', paths)
    self.assertIn('master/bot/benchmark/foo2/story', paths)

  def testPostHistogramPassesHistogramLevelSparseDiagnostics(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        'type': 'GenericSet',
    }, {
        'values': ['test'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        },
        'name': 'foo',
        'unit': 'count'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.ALERT_GROUPING.name:
                'f71bad03-ef98-42c0-833c-790d4e873871',
            reserved_infos.DEVICE_IDS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        },
        'name': 'foo2',
        'unit': 'count'
    }])
    self.PostAddHistogram({'data': data})
    for params in self.TaskParams():
      diagnostics = params['diagnostics']
      if len(diagnostics) < 1:
        continue
      self.assertEqual(['test'],
                       diagnostics[reserved_infos.DEVICE_IDS.name]['values'])
      self.assertNotEqual('0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
                          diagnostics[reserved_infos.DEVICE_IDS.name]['guid'])

  def testPostHistogramPassesAlertGroupingSparseDiagnostics(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'values': ['group'],
        'guid': 'f71bad03-ef98-42c0-833c-790d4e873871',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        },
        'name': 'foo',
        'unit': 'count'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.ALERT_GROUPING.name:
                'f71bad03-ef98-42c0-833c-790d4e873871',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '876d0fba-1d12-4c00-a7e9-5fed467e19e3',
        },
        'name': 'foo2',
        'unit': 'count'
    }])
    self.PostAddHistogram({'data': data})
    for params in self.TaskParams():
      diagnostics = params['diagnostics']
      if len(diagnostics) < 1:
        continue
      self.assertEqual(
          ['group'],
          diagnostics[reserved_infos.ALERT_GROUPING.name]['values'],
      )

  def testPostHistogram_AddsNewSparseDiagnostic(self):
    diag_dict = {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }
    diag = histogram.SparseDiagnostic(
        data=diag_dict,
        start_revision=1,
        end_revision=sys.maxsize,
        test=utils.TestKey('master/bot/benchmark'))
    diag.put()
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }])
    self.PostAddHistogram({'data': data})

    diagnostics = histogram.SparseDiagnostic.query().fetch()
    params = self.TaskParams()[0]
    hist = params['data']

    self.assertEqual(4, len(diagnostics))
    self.assertEqual('e9c2891d-2b04-413f-8cf4-099827e67626',
                     hist['diagnostics'][reserved_infos.MASTERS.name])

  def testPostHistogram_DeduplicatesSameSparseDiagnostic(self):
    diag_dict = {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }
    diag = histogram.SparseDiagnostic(
        id='e9c2891d-2b04-413f-8cf4-099827e67626',
        data=diag_dict,
        start_revision=1,
        end_revision=sys.maxsize,
        test=utils.TestKey('master/bot/benchmark'))
    diag.put()
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }])
    self.PostAddHistogram({'data': data})

    diagnostics = histogram.SparseDiagnostic.query().fetch()
    hist = self.TaskParams()[0]['data']

    self.assertEqual(3, len(diagnostics))
    self.assertEqual('e9c2891d-2b04-413f-8cf4-099827e67626',
                     hist['diagnostics'][reserved_infos.MASTERS.name])

  def testPostHistogramFailsWithoutHistograms(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }])

    self.PostAddHistogramProcess(data)

  def testPostHistogramFailsWithoutBuildbotInfo(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet'
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }])

    self.PostAddHistogramProcess(data)

  def testPostHistogramFailsWithoutChromiumCommit(self):
    data = json.dumps([{
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }])

    self.PostAddHistogramProcess(data)

  def testPostHistogramFailsWithoutBenchmark(self):
    data = json.dumps([{
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160'
        },
        'name': 'foo',
        'unit': 'count'
    }])

    self.PostAddHistogramProcess(data)

  def testPostHistogram_AddsSparseDiagnosticByName(self):
    data = json.dumps([{
        'type': 'GenericSet',
        'guid': 'cabb59fe-4bcf-4512-881c-d038c7a80635',
        'values': ['alice@chromium.org']
    }, {
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet',
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.OWNERS.name:
                'cabb59fe-4bcf-4512-881c-d038c7a80635',
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'foo',
        'unit': 'count'
    }])

    self.PostAddHistogram({'data': data})

    diagnostics = histogram.SparseDiagnostic.query().fetch()

    params = self.TaskParams()[0]
    hist = params['data']
    owners_info = hist['diagnostics'][reserved_infos.OWNERS.name]
    self.assertEqual(4, len(diagnostics))

    names = [
        reserved_infos.BENCHMARKS.name, reserved_infos.BOTS.name,
        reserved_infos.OWNERS.name, reserved_infos.MASTERS.name
    ]
    diagnostics_by_name = {}
    for d in diagnostics:
      self.assertIn(d.name, names)
      names.remove(d.name)
      diagnostics_by_name[d.name] = d
    self.assertEqual(
        ['benchmark'],
        diagnostics_by_name[reserved_infos.BENCHMARKS.name].data['values'])
    self.assertEqual(
        ['bot'], diagnostics_by_name[reserved_infos.BOTS.name].data['values'])
    self.assertEqual(
        ['alice@chromium.org'],
        diagnostics_by_name[reserved_infos.OWNERS.name].data['values'])
    self.assertEqual(
        ['master'],
        diagnostics_by_name[reserved_infos.MASTERS.name].data['values'])
    self.assertEqual('cabb59fe-4bcf-4512-881c-d038c7a80635', owners_info)

  def testPostHistogram_AddsSparseDiagnosticByName_OnlyOnce(self):
    data = json.dumps([{
        'type': 'GenericSet',
        'guid': 'cabb59fe-4bcf-4512-881c-d038c7a80635',
        'values': ['alice@chromium.org']
    }, {
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet',
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet'
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
            reserved_infos.OWNERS.name:
                'cabb59fe-4bcf-4512-881c-d038c7a80635'
        },
        'name': 'foo',
        'unit': 'count'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.OWNERS.name:
                'cabb59fe-4bcf-4512-881c-d038c7a80635',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        },
        'name': 'bar',
        'unit': 'count'
    }])

    self.PostAddHistogram({'data': data})

    diagnostics = histogram.SparseDiagnostic.query().fetch()

    self.assertEqual(4, len(diagnostics))
    self.assertEqual(reserved_infos.BOTS.name, diagnostics[1].name)
    self.assertNotEqual(reserved_infos.BOTS.name, diagnostics[0].name)

  def testPostHistogram_AddsSparseDiagnosticByName_ErrorsIfDiverging(self):
    data = json.dumps([{
        'type': 'GenericSet',
        'guid': 'cabb59fe-4bcf-4512-881c-d038c7a80635',
        'values': ['alice@chromium.org']
    }, {
        'type': 'GenericSet',
        'guid': '7c5bd92f-4146-411b-9192-248ffc1be92c',
        'values': ['bob@chromium.org']
    }, {
        'values': ['benchmark'],
        'guid': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
        'type': 'GenericSet'
    }, {
        'values': [424242],
        'guid': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'type': 'GenericSet'
    }, {
        'values': ['master'],
        'guid': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'type': 'GenericSet'
    }, {
        'values': ['bot'],
        'guid': '53fb5448-9f8d-407a-8891-e7233fe1740f',
        'type': 'GenericSet'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
            reserved_infos.OWNERS.name:
                'cabb59fe-4bcf-4512-881c-d038c7a80635'
        },
        'name': 'foo',
        'unit': 'count'
    }, {
        'binBoundaries': [1, [1, 1000, 20]],
        'diagnostics': {
            reserved_infos.MASTERS.name:
                'e9c2891d-2b04-413f-8cf4-099827e67626',
            reserved_infos.BOTS.name:
                '53fb5448-9f8d-407a-8891-e7233fe1740f',
            reserved_infos.CHROMIUM_COMMIT_POSITIONS.name:
                '25f0a111-9bb4-4cea-b0c1-af2609623160',
            reserved_infos.BENCHMARKS.name:
                '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae',
            reserved_infos.OWNERS.name:
                '7c5bd92f-4146-411b-9192-248ffc1be92c'
        },
        'name': 'foo',
        'unit': 'count'
    }])

    self.PostAddHistogramProcess(data)

  def testFindHistogramLevelSparseDiagnostics(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        'foo', generic_set.GenericSet(['bar']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.DEVICE_IDS.name, generic_set.GenericSet([]))
    diagnostics = add_histograms.FindHistogramLevelSparseDiagnostics(hist)

    self.assertEqual(1, len(diagnostics))
    self.assertIsInstance(diagnostics[reserved_infos.DEVICE_IDS.name],
                          generic_set.GenericSet)

  def testFindSuiteLevelSparseDiagnostics(self):

    def _CreateSingleHistogram(hist, master):
      h = histogram_module.Histogram(hist, 'count')
      h.diagnostics[reserved_infos.MASTERS.name] = (
          generic_set.GenericSet([master]))
      return h

    histograms = histogram_set.HistogramSet([
        _CreateSingleHistogram('hist1', 'master1'),
        _CreateSingleHistogram('hist2', 'master2')
    ])

    with self.assertRaises(ValueError):
      add_histograms.FindSuiteLevelSparseDiagnostics(histograms,
                                                     utils.TestKey('M/B/Foo'),
                                                     12345, False)

  def testComputeRevision(self):
    hs = _CreateHistogram(
        name='hist', commit_position=424242, revision_timestamp=123456)
    self.assertEqual(424242, add_histograms.ComputeRevision(hs))

  def testComputeRevision_NotInteger_Raises(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    chromium_commit = generic_set.GenericSet(['123'])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name, chromium_commit)
    with self.assertRaises(api_request_handler.BadRequestError):
      add_histograms.ComputeRevision(histograms)

  def testComputeRevision_RaisesOnError(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    chromium_commit = generic_set.GenericSet([424242, 0])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name, chromium_commit)
    with self.assertRaises(api_request_handler.BadRequestError):
      add_histograms.ComputeRevision(histograms)

  def testComputeRevision_UsesPointIdIfPresent(self):
    hs = _CreateHistogram(
        name='foo',
        commit_position=123456,
        point_id=234567,
        revision_timestamp=345678)
    rev = add_histograms.ComputeRevision(hs)
    self.assertEqual(234567, rev)

  def testComputeRevision_PointIdNotInteger_Raises(self):
    hs = _CreateHistogram(name='foo', commit_position=123456, point_id='abc')
    with self.assertRaises(api_request_handler.BadRequestError):
      add_histograms.ComputeRevision(hs)

  def testComputeRevision_UsesRevisionTimestampIfNecessary(self):
    hs = _CreateHistogram(name='foo', revision_timestamp=123456)
    rev = add_histograms.ComputeRevision(hs)
    self.assertEqual(123456, rev)

  def testComputeRevision_RevisionTimestampNotInteger_Raises(self):
    hs = _CreateHistogram(name='foo', revision_timestamp='abc')
    with self.assertRaises(api_request_handler.BadRequestError):
      add_histograms.ComputeRevision(hs)

  def testComputeRevision_NoRevision_Raises(self):
    hs = _CreateHistogram(name='foo')
    with self.assertRaises(api_request_handler.BadRequestError):
      add_histograms.ComputeRevision(hs)

  def testSparseDiagnosticsAreNotInlined(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARKS.name, generic_set.GenericSet(['benchmark']))
    add_histograms.InlineDenseSharedDiagnostics(histograms)
    self.assertTrue(hist.diagnostics[reserved_infos.BENCHMARKS.name].has_guid)

  @mock.patch('logging.info')
  def testLogDebugInfo_Succeeds(self, mock_log):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.LOG_URLS.name, generic_set.GenericSet(['http://foo']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BUILD_URLS.name, generic_set.GenericSet(['http://bar']))
    add_histograms._LogDebugInfo(histograms)
    self.assertEqual('Buildbot URL: %s' % "['http://foo']",
                     mock_log.call_args_list[0][0][0])
    self.assertEqual('Build URL: %s' % "['http://bar']",
                     mock_log.call_args_list[1][0][0])

  @mock.patch('logging.info')
  def testLogDebugInfo_NoHistograms(self, mock_log):
    histograms = histogram_set.HistogramSet()
    add_histograms._LogDebugInfo(histograms)
    mock_log.assert_called_once_with('No histograms in data.')

  @mock.patch('logging.info')
  def testLogDebugInfo_NoLogUrls(self, mock_log):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    add_histograms._LogDebugInfo(histograms)
    self.assertEqual('No LOG_URLS in data.', mock_log.call_args_list[0][0][0])
    self.assertEqual('No BUILD_URLS in data.', mock_log.call_args_list[1][0][0])


@mock.patch.object(SheriffConfigClient, '__init__',
                   mock.MagicMock(return_value=None))
@mock.patch.object(SheriffConfigClient, 'Match',
                   mock.MagicMock(return_value=([], None)))
@mock.patch('dashboard.services.skia_bridge_service.SkiaServiceClient',
            mock.MagicMock())
class AddHistogramsUploadCompleteonTokenTest(AddHistogramsBaseTest):

  def setUp(self):
    super().setUp()
    self._TrunOnUploadCompletionTokenExperiment()
    hs = _CreateHistogram(
        master='master',
        bot='bot',
        benchmark='benchmark',
        commit_position=123,
        benchmark_description='Benchmark description.',
        samples=[1, 2, 3])
    self.histogram_data = json.dumps(hs.AsDicts())

  def _TrunOnUploadCompletionTokenExperiment(self):
    """Sets the domain that users who can access internal data belong to."""
    self.PatchObject(utils, 'ShouldTurnOnUploadCompletionTokenExperiment',
                     mock.Mock(return_value=True))

  def PostAddHistogram(self, data, status=200):
    mock_obj = mock.MagicMock(wraps=BufferedFakeFile())
    self.mock_cloudstorage.open.return_value = mock_obj
    return self.testapp.post('/add_histograms', data, status=status).json

  def GetUploads(self, token_id, status=200):
    return json.loads(
        self.testapp.get(
            '/uploads/%s?additional_info=measurements,dimensions' % token_id,
            status=status).body)

  def testPost_Succeeds(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})
    self.assertTrue(token_info.get('token') is not None)
    self.assertTrue(token_info.get('file') is not None)

    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.key.id(), token_info['token'])
    self.assertEqual(token.temporary_staging_file_path, token_info['file'])
    self.assertEqual(token.state, upload_completion_token.State.PENDING)
    self.assertEqual(token.error_message, None)
    self.assertEqual(token.internal_only, utils.IsInternalUser())

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    measurements = token.GetMeasurements()
    self.assertEqual(len(measurements), 1)
    self.assertEqual(measurements[0].state,
                     upload_completion_token.State.PROCESSING)
    self.assertEqual(measurements[0].error_message, None)
    self.assertEqual(measurements[0].monitored, False)
    self.assertEqual(measurements[0].histogram, None)

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)

    measurements = token.GetMeasurements()
    self.assertEqual(measurements[0].state,
                     upload_completion_token.State.COMPLETED)
    self.assertEqual(measurements[0].error_message, None)
    self.assertNotEqual(measurements[0].histogram, None)

    added_histogram = measurements[0].histogram.get()
    self.assertNotEqual(added_histogram, None)
    self.assertEqual(added_histogram.test.id(), 'master/bot/benchmark/hist')
    self.assertEqual(added_histogram.revision, 123)

  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock(side_effect=Exception('Test error')))
  def testPost_MeasurementFails(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})

    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.PENDING)

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    # Error happened on measurement level.
    self.assertEqual(token.error_message, None)

  @mock.patch.object(add_histograms, 'ProcessHistogramSet',
                     mock.MagicMock(side_effect=Exception('Test error')))
  def testPost_AddHistogramProcessFails(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertEqual(token.error_message, 'Test error')

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_TokenExpiredBeforeProcess(self, mock_process_test,
                                         mock_graph_revisions):
    token_info = self.PostAddHistogram({'data': self.histogram_data})

    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.PENDING)

    token.key.delete()
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token, None)

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    diagnostics = histogram.SparseDiagnostic.query().fetch()

    # Perform same checks as for AddHistogramsEndToEndTest.testPost_Succeeds.

    self.assertEqual(4, len(diagnostics))
    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    tests = graph_data.TestMetadata.query().fetch()

    self.assertEqual('Benchmark description.', tests[0].description)

    self.assertEqual(mock_process_test.call_count, 1)
    rows = graph_data.Row.query().fetch()

    mock_graph_revisions.assert_called_once_with(mock.ANY)
    self.assertEqual(len(mock_graph_revisions.mock_calls[0][1][0]), len(rows))

  @mock.patch.object(add_histograms_queue.graph_revisions,
                     'AddRowsToCacheAsync')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync')
  def testPost_TokenExpiredAfterProcess(self, mock_process_test,
                                        mock_graph_revisions):
    token_info = self.PostAddHistogram({'data': self.histogram_data})

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    token.key.delete()
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token, None)

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    diagnostics = histogram.SparseDiagnostic.query().fetch()

    # Perform same checks as for AddHistogramsEndToEndTest.testPost_Succeeds.

    self.assertEqual(4, len(diagnostics))
    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    tests = graph_data.TestMetadata.query().fetch()

    self.assertEqual('Benchmark description.', tests[0].description)

    self.assertEqual(mock_process_test.call_count, 1)
    rows = graph_data.Row.query().fetch()

    mock_graph_revisions.assert_called_once_with(mock.ANY)
    self.assertEqual(len(mock_graph_revisions.mock_calls[0][1][0]), len(rows))

  @mock.patch.object(utils, 'IsMonitored', mock.MagicMock(return_value=True))
  def testPost_MonitoredMeasurementSucceeds(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})
    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')

    token = upload_completion_token.Token.get_by_id(token_info['token'])
    measurements = token.GetMeasurements()
    self.assertEqual(len(measurements), 1)
    self.assertEqual(measurements[0].monitored, True)

  # (crbug/1298177) The setup for Flask is not ready yet. We will force the test
  # to run in the old setup for now.
  @unittest.skipIf(six.PY3, 'DevAppserver not ready yet for python 3.')
  @mock.patch.object(utils, 'IsDevAppserver', mock.MagicMock(return_value=True))
  def testPost_DevAppserverSucceeds(self):
    token_info = self.PostAddHistogram(self.histogram_data)
    self.assertTrue('token' in token_info)
    self.assertTrue('file' in token_info)

    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token_info['file'], None)
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)
    self.assertEqual(token.internal_only, utils.IsInternalUser())

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)
    token = upload_completion_token.Token.get_by_id(token_info['token'])
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)

  @mock.patch('logging.info')
  def testPostLogs_Succeeds(self, mock_log):
    token_info = self.PostAddHistogram({'data': self.histogram_data})
    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    measurement_id = 'master/bot/benchmark/hist'
    log_calls = [
        mock.call(
            'Upload completion token created. Token id: %s',
            token_info['token']),
        mock.call(
            'Upload completion token updated. Token id: %s, state: %s',
            token_info['token'], 'PROCESSING'),
        mock.call(
            'Upload completion token measurement created. Token id: %s, '
            'measurement test path: %r', token_info['token'], measurement_id),
        mock.call(
            'Upload completion token updated. Token id: %s, state: %s',
            token_info['token'], 'COMPLETED'),
        mock.call(
            'Upload completion token measurement updated. Token id: %s, '
            'measurement test path: %s, state: %s', token_info['token'],
            measurement_id, 'COMPLETED'),
    ]
    mock_log.assert_has_calls(log_calls, any_order=True)

  @mock.patch('logging.info')
  @mock.patch.object(add_histograms, 'ProcessHistogramSet',
                     mock.MagicMock(side_effect=Exception()))
  def testPostLogs_AddHistogramProcessFails(self, mock_log):
    token_info = self.PostAddHistogram({'data': self.histogram_data})
    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')

    log_calls = [
        mock.call('Upload completion token created. Token id: %s',
                  token_info['token']),
        mock.call('Upload completion token updated. Token id: %s, state: %s',
                  token_info['token'], 'FAILED'),
    ]
    mock_log.assert_has_calls(log_calls, any_order=True)

  # (crbug/1403845): Routing is broken after ExecuteTaskQueueTasks is called.
  @unittest.skipIf(six.PY3, '''
    http requests after ExecuteTaskQueueTasks are not routed correctly for py3.
    ''')
  def testFullCycle_Success(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})

    uploads_response = self.GetUploads(token_info['token'])
    self.assertEqual(uploads_response['token'], token_info['token'])
    self.assertEqual(uploads_response['state'], 'PENDING')
    self.assertTrue(uploads_response.get('error_message') is None)
    self.assertTrue(uploads_response.get('file') is not None)
    self.assertTrue(uploads_response.get('created') is not None)
    self.assertTrue(uploads_response.get('lastUpdated') is not None)
    self.assertTrue(uploads_response.get('measurements') is None)

    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')

    uploads_response = self.GetUploads(token_info['token'])
    self.assertEqual(uploads_response['state'], 'PROCESSING')
    self.assertEqual(len(uploads_response.get('measurements')), 1)

    measurement = uploads_response['measurements'][0]
    self.assertEqual(measurement['name'], 'master/bot/benchmark/hist')
    self.assertEqual(measurement['state'], 'PROCESSING')
    self.assertEqual(measurement['monitored'], False)
    self.assertTrue(measurement.get('dimensions') is None)
    self.assertTrue(measurement.get('lastUpdated') is not None)

    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    uploads_response = self.GetUploads(token_info['token'])
    measurement = uploads_response['measurements'][0]
    self.assertEqual(uploads_response['state'], 'COMPLETED')
    self.assertEqual(measurement['state'], 'COMPLETED')
    self.assertEqual(len(measurement['dimensions']), 5)

  # (crbug/1403845): Routing is broken after ExecuteTaskQueueTasks is called.
  @unittest.skipIf(six.PY3, '''
    http requests after ExecuteTaskQueueTasks are not routed correctly for py3.
    ''')
  @mock.patch.object(add_histograms_queue.find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock(side_effect=Exception('Test error')))
  def testFullCycle_MeasurementFails(self):
    token_info = self.PostAddHistogram({'data': self.histogram_data})
    self.ExecuteTaskQueueTasks('/add_histograms/process', 'default')
    self.ExecuteTaskQueueTasks('/add_histograms_queue',
                               add_histograms.TASK_QUEUE_NAME)

    uploads_response = self.GetUploads(token_info['token'])
    self.assertEqual(uploads_response['state'], 'FAILED')
    self.assertEqual(len(uploads_response.get('measurements')), 1)
    self.assertEqual(uploads_response['measurements'][0]['state'], 'FAILED')
    self.assertEqual(uploads_response['measurements'][0]['error_message'],
                     'Test error')


def RandomChars(length):
  for _ in itertools.islice(itertools.count(0), length):
    yield '%s' % (random.choice(string.ascii_letters))


class DecompressFileWrapperTest(testing_common.TestCase):

  def testBasic(self):
    filesize = 1024 * 256
    random.seed(1)
    payload = ''.join(list(RandomChars(filesize)))
    random.seed(None)
    self.assertEqual(len(payload), filesize)
    input_file = BufferedFakeFile(zlib.compress(six.ensure_binary(payload)))
    retrieved_payload = str()
    with add_histograms.DecompressFileWrapper(input_file, 2048) as decompressor:
      while True:
        chunk = six.ensure_str(decompressor.read(1024))
        if len(chunk) == 0:
          break
        retrieved_payload += chunk
    self.assertEqual(payload, retrieved_payload)

  def testDecompressionFail(self):
    filesize = 1024 * 256
    random.seed(1)
    payload = ''.join(list(RandomChars(filesize)))
    random.seed(None)
    self.assertEqual(len(payload), filesize)

    # We create a BufferedFakeFile which does not contain zlib-compressed data.
    input_file = BufferedFakeFile(payload)
    retrieved_payload = str()
    with self.assertRaises(zlib.error):
      with add_histograms.DecompressFileWrapper(input_file, 2048) as d:
        while True:
          chunk = six.ensure_str(d.read(1024))
          if len(chunk) == 0:
            break
          retrieved_payload += chunk

  def testJSON(self):
    # Create a JSON payload that's compressed and loaded appropriately.
    def _MakeHistogram(name):
      h = histogram_module.Histogram(name, 'count')
      for i in range(100):
        h.AddSample(i)
      return h

    hists = [
        _MakeHistogram('hist_%d' % i)
        for i in itertools.islice(itertools.count(0), 1000)
    ]
    histograms = histogram_set.HistogramSet(hists)
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.MASTERS.name, generic_set.GenericSet(['master']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BOTS.name, generic_set.GenericSet(['bot']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
        generic_set.GenericSet([12345]))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.BENCHMARKS.name, generic_set.GenericSet(['benchmark']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.DEVICE_IDS.name, generic_set.GenericSet(['device_foo']))

    input_file_compressed = BufferedFakeFile(
        zlib.compress(six.ensure_binary(json.dumps(histograms.AsDicts()))))
    input_file_raw = BufferedFakeFile(json.dumps(histograms.AsDicts()))

    loaded_compressed_histograms = histogram_set.HistogramSet()

    with add_histograms.DecompressFileWrapper(input_file_compressed,
                                              256) as decompressor:
      loaded_compressed_histograms.ImportDicts(
          add_histograms._LoadHistogramList(decompressor))

    loaded_compressed_histograms.DeduplicateDiagnostics()

    loaded_raw_histograms = histogram_set.HistogramSet()
    loaded_raw_histograms.ImportDicts(
        add_histograms._LoadHistogramList(input_file_raw))
    loaded_raw_histograms.DeduplicateDiagnostics()

    raw_dicts = loaded_raw_histograms.AsDicts()
    compressed_dicts = loaded_compressed_histograms.AsDicts()
    self.assertCountEqual(raw_dicts, compressed_dicts)

  def testJSONFail(self):
    with BufferedFakeFile('Not JSON') as input_file:
      with self.assertRaises(ValueError):
        _ = add_histograms._LoadHistogramList(input_file)

  def testIncrementalJSONSupport(self):
    with BufferedFakeFile('[{"key": "value incomplete"') as input_file:
      with self.assertRaises(ValueError):
        _ = add_histograms._LoadHistogramList(input_file)

    with BufferedFakeFile('[{"key": "complete list"}]') as input_file:
      dicts = add_histograms._LoadHistogramList(input_file)
      self.assertSequenceEqual([{u'key': u'complete list'}], dicts)
