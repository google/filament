# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import copy
from flask import Flask
import json
from unittest import mock
import sys
import uuid
import webtest

from google.appengine.ext import ndb

from dashboard import add_histograms_queue
from dashboard import find_anomalies
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models import upload_completion_token
from dashboard.sheriff_config_client import SheriffConfigClient
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

TEST_HISTOGRAM = {
    'allBins': {
        '1': [1],
        '3': [1],
        '4': [1]
    },
    'binBoundaries': [1, [1, 1000, 20]],
    'diagnostics': {
        reserved_infos.LOG_URLS.name: {
            'values': [['Buildbot stdio', 'http://log.url/']],
            'type': 'GenericSet',
        },
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name: {
            'values': [123],
            'type': 'GenericSet'
        },
        reserved_infos.V8_REVISIONS.name: {
            'values': ['4cd34ad3320db114ad3a2bd2acc02aba004d0cb4'],
            'type': 'GenericSet'
        },
        reserved_infos.OWNERS.name: '68e5b3bd-829c-4f4f-be3a-98a94279ccf0',
        reserved_infos.BENCHMARKS.name: 'ec2c0cdc-cd9f-4736-82b4-6ffc3d76e3eb',
        reserved_infos.TRACE_URLS.name: {
            'type': 'GenericSet',
            'values': ['http://google.com/'],
        },
    },
    'name': 'foo',
    'running': [3, 3, 0.5972531564093516, 2, 1, 6, 2],
    'sampleValues': [1, 2, 3],
    'unit': 'count_biggerIsBetter'
}

TEST_BENCHMARKS = {
    'guid': 'ec2c0cdc-cd9f-4736-82b4-6ffc3d76e3eb',
    'values': ['myBenchmark'],
    'type': 'GenericSet',
}

TEST_OWNERS = {
    'guid': '68e5b3bd-829c-4f4f-be3a-98a94279ccf0',
    'values': ['abc@chromium.org'],
    'type': 'GenericSet'
}

flask_app = Flask(__name__)


@flask_app.route('/add_histograms_queue', methods=['GET', 'POST'])
def AddHistogramsQueuePost():
  return add_histograms_queue.AddHistogramsQueuePost()


@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class AddHistogramsQueueTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    self.SetCurrentUser('foo@bar.com', is_admin=True)

  def testPostHistogram(self):
    graph_data.Bot(
        key=ndb.Key('Master', 'Chromium', 'Bot', 'win7'),
        internal_only=False).put()
    test_path = 'Chromium/win7/suite/metric'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    test_key = utils.TestKey(test_path)

    test = test_key.get()
    self.assertEqual(test.units, 'count_biggerIsBetter')
    self.assertEqual(test.improvement_direction, anomaly.UP)

    master = ndb.Key('Master', 'Chromium').get()
    self.assertIsNotNone(master)

    bot = ndb.Key('Master', 'Chromium', 'Bot', 'win7').get()
    self.assertIsNotNone(bot)

    tests = graph_data.TestMetadata.query().fetch()
    self.assertEqual(8, len(tests))

    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    h = histograms[0]
    h1 = histograms[0].data

    h2 = copy.deepcopy(TEST_HISTOGRAM)
    self.assertEqual(h2, h1)
    self.assertEqual(test_key, h.test)
    self.assertEqual(123, h.revision)
    self.assertFalse(h.internal_only)

  def testPostHistogram_Internal(self):
    test_path = 'Chromium/win7/suite/metric'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    test_key = utils.TestKey(test_path)

    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    h = histograms[0]
    h1 = histograms[0].data

    h2 = copy.deepcopy(TEST_HISTOGRAM)
    self.assertEqual(h2, h1)
    self.assertEqual(test_key, h.test)
    self.assertEqual(123, h.revision)
    self.assertTrue(h.internal_only)

    rows = graph_data.Row.query().fetch()
    self.assertEqual(7, len(rows))
    self.assertEqual(None, rows[0].swarming_bot_id)

  def testPostHistogram_Internal_swarmingBotId(self):
    test_path = 'Chromium/win7/suite/metric'
    h2 = copy.deepcopy(TEST_HISTOGRAM)
    h2['diagnostics'][reserved_infos.BOT_ID.name] = {
        'type': 'GenericSet',
        'values': ['swarming-bot-id-0'],
    }

    params = [{
        'data': h2,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(7, len(rows))
    self.assertEqual('swarming-bot-id-0', rows[0].swarming_bot_id)

  def testPostHistogram_Internal_invalidSwarmingBotId(self):
    test_path = 'Chromium/win7/suite/metric'
    h2 = copy.deepcopy(TEST_HISTOGRAM)
    h2['diagnostics'][reserved_infos.BOT_ID.name] = {
        'type':
            'GenericSet',
        'values': [
            'swarming-bot-id-0', 'swarming-bot-id-1', 'swarming-bot-id-2',
            'swarming-bot-id-3'
        ],
    }

    params = [{
        'data': h2,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    histograms = histogram.Histogram.query().fetch()
    self.assertEqual(1, len(histograms))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(7, len(rows))
    self.assertEqual(None, rows[0].swarming_bot_id)

  def testPostHistogram_WithFreshDiagnostics(self):
    graph_data.Bot(
        key=ndb.Key('Master', 'Chromium', 'Bot', 'win7'),
        internal_only=False).put()
    test_path = 'Chromium/win7/suite/metric'
    histogram.HistogramRevisionRecord.GetOrCreate(
        utils.TestKey('Chromium/win7/suite'), 1).put()
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123,
        'diagnostics': {
            'benchmarks': TEST_BENCHMARKS,
            'owners': TEST_OWNERS
        }
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))
    histogram_entity = histogram.Histogram.query().fetch()[0]
    hist = histogram_module.Histogram.FromDict(histogram_entity.data)
    self.assertEqual('ec2c0cdc-cd9f-4736-82b4-6ffc3d76e3eb',
                     hist.diagnostics[reserved_infos.BENCHMARKS.name].guid)
    self.assertEqual('68e5b3bd-829c-4f4f-be3a-98a94279ccf0',
                     hist.diagnostics['owners'].guid)
    telemetry_info_entity = ndb.Key('SparseDiagnostic',
                                    TEST_BENCHMARKS['guid']).get()
    ownership_entity = ndb.Key('SparseDiagnostic', TEST_OWNERS['guid']).get()
    self.assertFalse(telemetry_info_entity.internal_only)
    self.assertEqual('benchmarks', telemetry_info_entity.name)
    self.assertFalse(ownership_entity.internal_only)
    self.assertEqual('owners', ownership_entity.name)

  def testPostHistogram_WithSameDiagnostic(self):
    diag_dict = {
        'guid': '05341937-1272-4214-80ce-43b2d03807f9',
        'values': ['myBenchmark'],
        'type': 'GenericSet',
    }
    diag = histogram.SparseDiagnostic(
        data=diag_dict,
        start_revision=1,
        end_revision=sys.maxsize,
        test=utils.TestKey('Chromium/win7/suite/metric'))
    diag.put()
    histogram.HistogramRevisionRecord.GetOrCreate(
        utils.TestKey('Chromium/win7/suite'), 1).put()
    test_path = 'Chromium/win7/suite/metric'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123,
        'diagnostics': {
            'benchmarks': TEST_BENCHMARKS,
            'owners': TEST_OWNERS
        }
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))
    histogram_entity = histogram.Histogram.query().fetch()[0]
    hist = histogram_module.Histogram.FromDict(histogram_entity.data)
    self.assertEqual(TEST_BENCHMARKS['guid'],
                     hist.diagnostics[reserved_infos.BENCHMARKS.name].guid)
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(len(diagnostics), 3)

  def testPostHistogram_WithDifferentDiagnostic(self):
    diag_dict = {
        'guid': 'c397a1a0-e289-45b2-abe7-29e638e09168',
        'values': ['def@chromium.org'],
        'type': 'GenericSet'
    }
    diag = histogram.SparseDiagnostic(
        data=diag_dict,
        name='owners',
        start_revision=1,
        end_revision=sys.maxsize,
        test=utils.TestKey('Chromium/win7/suite/metric'))
    diag.put()
    histogram.HistogramRevisionRecord.GetOrCreate(
        utils.TestKey('Chromium/win7/suite'), 1).put()
    test_path = 'Chromium/win7/suite/metric'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123,
        'diagnostics': {
            'benchmarks': TEST_BENCHMARKS,
            'owners': TEST_OWNERS
        }
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))
    histogram_entity = histogram.Histogram.query().fetch()[0]
    hist = histogram_module.Histogram.FromDict(histogram_entity.data)
    self.assertEqual('68e5b3bd-829c-4f4f-be3a-98a94279ccf0',
                     hist.diagnostics['owners'].guid)
    diagnostics = histogram.SparseDiagnostic.query().fetch()
    self.assertEqual(len(diagnostics), 3)

  def testPostHistogram_StoresUnescapedStoryName(self):
    hist = histogram_module.Histogram('hist', 'count')
    hist.AddSample(42)
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name,
        generic_set.GenericSet(['http://unescaped_story']))

    test_path = 'Chromium/win7/suite/metric'
    params = [{
        'data': hist.AsDict(),
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123,
        'diagnostics': {
            'stories': hist.diagnostics.get('stories').AsDict(),
        }
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    t = utils.TestKey(test_path).get()

    self.assertEqual('http://unescaped_story', t.unescaped_story_name)

  def testPostHistogram_OnlyCreatesAvgRowForMemoryBenchmark(self):
    test_path = 'Chromium/win7/memory_desktop/memory:chrome'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 2)
    self.assertTrue(rows[1].key.parent().id().endswith('_avg'))

  def testPostHistogram_SuffixesHistogramName(self):
    test_path = 'Chromium/win7/memory_desktop/memory:chrome/bogus_page'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 2)
    parts = rows[1].key.parent().id().split('/')
    self.assertEqual(len(parts), 5)
    self.assertTrue(parts[3].endswith('_avg'))
    self.assertFalse(parts[4].endswith('_avg'))

    # Ensure we can retrive the row using container_key
    avg_test_key = utils.TestKey(rows[1].key.parent().id())
    container_key = utils.GetTestContainerKey(avg_test_key)
    row_get_by_id = graph_data.Row.get_by_id(123, parent=container_key)
    self.assertEqual(rows[1], row_get_by_id)

  def testPostHistogram_KeepsWeirdStatistics(self):
    test_path = 'Chromium/win7/memory_desktop/memory:chrome'
    hist = histogram_module.Histogram.FromDict(TEST_HISTOGRAM)
    hist.CustomizeSummaryOptions({'percentile': [0.9]})

    params = [{
        'data': hist.AsDict(),
        'test_path': test_path,
        'revision': 123,
        'benchmark_description': None
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 3)
    self.assertTrue(rows[2].key.parent().id().endswith('_pct_090'))

  def testPostHistogram_V8StatsNotSpecial(self):
    test_path = 'Chromium/win7/v8.browsing_desktop/v8-gc-blah'

    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'revision': 123,
        'benchmark_description': None
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    # We now create all the stats as described in the histogram.
    self.assertEqual(len(rows), 7)
    self.assertEqual(rows[0].key.parent().id(),
                     'Chromium/win7/v8.browsing_desktop/v8-gc-blah')

  def testPostHistogram_FiltersBenchmarkTotalDuration(self):
    test_path = 'Chromium/win7/benchmark/benchmark_total_duration'

    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'revision': 123,
        'benchmark_description': None
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 1)
    self.assertEqual(rows[0].key.parent().id(),
                     'Chromium/win7/benchmark/benchmark_total_duration')

  def testPostHistogram_CreatesNoLegacyRowsForLegacyTest(self):
    test_path = 'Chromium/win7/blink_perf.dom/foo'
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 1)

  def testPostHistogram_EmptyCreatesNoTestsOrRowsOrHistograms(self):
    test_path = 'Chromium/win7/blink_perf.dom/foo'
    hist = histogram_module.Histogram('foo', 'count')
    params = [{
        'data': hist.AsDict(),
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123
    }]
    self.testapp.post('/add_histograms_queue', json.dumps(params))

    rows = graph_data.Row.query().fetch()
    self.assertEqual(len(rows), 0)
    tests = graph_data.TestMetadata.query().fetch()
    self.assertEqual(len(tests), 0)
    hists = histogram.Histogram.query().fetch()
    self.assertEqual(len(hists), 0)

  def testGetUnitArgs_Up(self):
    unit_args = add_histograms_queue.GetUnitArgs('count_biggerIsBetter')
    self.assertEqual(anomaly.UP, unit_args['improvement_direction'])

  def testGetUnitArgs_Down(self):
    unit_args = add_histograms_queue.GetUnitArgs('count_smallerIsBetter')
    self.assertEqual(anomaly.DOWN, unit_args['improvement_direction'])

  def testGetUnitArgs_Unknown(self):
    unit_args = add_histograms_queue.GetUnitArgs('count')
    self.assertEqual(anomaly.UNKNOWN, unit_args['improvement_direction'])

  def testCreateRowEntities(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)
    stat_names_to_test_keys = {
        'avg': utils.TestKey('Chromium/win7/suite/metric_avg'),
        'std': utils.TestKey('Chromium/win7/suite/metric_std'),
        'count': utils.TestKey('Chromium/win7/suite/metric_count'),
        'max': utils.TestKey('Chromium/win7/suite/metric_max'),
        'min': utils.TestKey('Chromium/win7/suite/metric_min'),
        'sum': utils.TestKey('Chromium/win7/suite/metric_sum')
    }
    rows_to_put = add_histograms_queue.CreateRowEntities(
        TEST_HISTOGRAM, test_key, stat_names_to_test_keys, 123)
    ndb.put_multi(rows_to_put)

    rows = graph_data.Row.query().fetch()
    rows_by_path = {}
    for row in rows:
      rows_by_path[row.key.parent().id()] = row

    avg_row = rows_by_path.pop('Chromium/win7/suite/metric_avg')
    self.assertAlmostEqual(2.0, avg_row.value)
    self.assertAlmostEqual(1.0, avg_row.error)
    std_row = rows_by_path.pop('Chromium/win7/suite/metric_std')
    self.assertAlmostEqual(1.0, std_row.value)
    self.assertEqual(0.0, std_row.error)
    count_row = rows_by_path.pop('Chromium/win7/suite/metric_count')
    self.assertEqual(3, count_row.value)
    self.assertEqual(0.0, count_row.error)
    max_row = rows_by_path.pop('Chromium/win7/suite/metric_max')
    self.assertAlmostEqual(3.0, max_row.value)
    self.assertEqual(0.0, max_row.error)
    min_row = rows_by_path.pop('Chromium/win7/suite/metric_min')
    self.assertAlmostEqual(1.0, min_row.value)
    self.assertEqual(0.0, min_row.error)
    sum_row = rows_by_path.pop('Chromium/win7/suite/metric_sum')
    self.assertAlmostEqual(6.0, sum_row.value)
    self.assertEqual(0.0, sum_row.error)

    row = rows_by_path.pop('Chromium/win7/suite/metric')
    self.assertEqual(0, len(rows_by_path))
    fields = iter(row.to_dict().keys())
    d_fields = []
    r_fields = []
    a_fields = []
    for field in fields:
      if field.startswith('d_'):
        d_fields.append(field)
      elif field.startswith('r_'):
        r_fields.append(field)
      elif field.startswith('a_'):
        a_fields.append(field)

    self.assertAlmostEqual(2.0, row.value)
    self.assertAlmostEqual(1.0, row.error)

    self.assertEqual(4, len(d_fields))
    self.assertEqual(3, row.d_count)
    self.assertAlmostEqual(3.0, row.d_max)
    self.assertAlmostEqual(1.0, row.d_min)
    self.assertAlmostEqual(6.0, row.d_sum)

    self.assertEqual(2, len(r_fields))
    self.assertEqual('4cd34ad3320db114ad3a2bd2acc02aba004d0cb4', row.r_v8_rev)
    self.assertEqual('123', row.r_commit_pos)

    self.assertEqual('[Buildbot stdio](http://log.url/)', row.a_stdio_uri)

  def testCreateRowEntities_WithCustomSummaryOptions(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)

    hist = histogram_module.Histogram.FromDict(TEST_HISTOGRAM)
    hist.CustomizeSummaryOptions({
        'avg': True,
        'std': True,
        'count': True,
        'max': False,
        'min': False,
        'sum': False
    })

    stat_names_to_test_keys = {
        'avg': utils.TestKey('Chromium/win7/suite/metric_avg'),
        'std': utils.TestKey('Chromium/win7/suite/metric_std'),
        'count': utils.TestKey('Chromium/win7/suite/metric_count')
    }
    rows = add_histograms_queue.CreateRowEntities(hist.AsDict(), test_key,
                                                  stat_names_to_test_keys, 123)

    self.assertEqual(4, len(rows))

    ndb.put_multi(rows)
    row = graph_data.Row.query().fetch()[0]
    fields = iter(row.to_dict().keys())
    d_fields = [field for field in fields if field.startswith('d_')]

    self.assertEqual(1, len(d_fields))
    self.assertEqual(3, row.d_count)

  def testCreateRowEntities_UsesStandardDeviationProperty(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)

    hist = histogram_module.Histogram.FromDict(TEST_HISTOGRAM)
    hist.CustomizeSummaryOptions({
        'avg': True,
        'std': False,
        'count': False,
        'max': False,
        'min': False,
        'sum': False
    })

    stat_names_to_test_keys = {
        'avg': utils.TestKey('Chromium/win7/suite/metric_avg')
    }
    rows = add_histograms_queue.CreateRowEntities(hist.AsDict(), test_key,
                                                  stat_names_to_test_keys, 123)

    self.assertEqual(2, len(rows))

    ndb.put_multi(rows)
    row = graph_data.Row.query().fetch()[1]

    self.assertAlmostEqual(1.0, row.error)

  def testCreateRowEntities_SuffixesRefProperly(self):
    test_path0 = 'Chromium/win7/suite/metric_ref'
    test_path1 = 'Chromium/win7/suite/metric/ref'
    test_key0 = utils.TestKey(test_path0)
    test_key1 = utils.TestKey(test_path1)
    rows_to_put = add_histograms_queue.CreateRowEntities(
        TEST_HISTOGRAM, test_key0, {}, 123)
    rows_to_put += add_histograms_queue.CreateRowEntities(
        TEST_HISTOGRAM, test_key1, {}, 123)
    ndb.put_multi(rows_to_put)
    rows = graph_data.Row.query().fetch()
    for row in rows:
      self.assertTrue(row.key.parent().id().endswith('ref'))

  def testCreateRowEntities_DoesntAddRowForEmptyHistogram(self):
    hist = histogram_module.Histogram('foo', 'count').AsDict()
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)
    row = add_histograms_queue.CreateRowEntities(hist, test_key, {}, 123)

    rows = graph_data.Row.query().fetch()
    self.assertEqual(0, len(rows))
    self.assertIsNone(row)

  def testCreateRowEntities_FailsWithNonSingularRevisionInfo(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)
    hist = copy.deepcopy(TEST_HISTOGRAM)
    hist['diagnostics'][reserved_infos.CATAPULT_REVISIONS.name] = {
        'type': 'GenericSet',
        'values': [123, 456]
    }

    with self.assertRaises(add_histograms_queue.BadRequestError):
      add_histograms_queue.CreateRowEntities(hist, test_key, {}, 123).put()

  def testCreateRowEntities_AddsTraceUri(self):
    test_path = 'Chromium/win7/suite/metric/story'
    test_key = utils.TestKey(test_path)
    hist = copy.deepcopy(TEST_HISTOGRAM)

    row = add_histograms_queue.CreateRowEntities(hist, test_key, {}, 123)[0]
    row_dict = row.to_dict()

    self.assertIn('a_tracing_uri', row_dict)
    self.assertEqual(row_dict['a_tracing_uri'], 'http://google.com/')

  def testCreateRowEntities_DoesNotAddTraceUriForSummary(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)
    hist = copy.deepcopy(TEST_HISTOGRAM)
    hist['diagnostics'][reserved_infos.SUMMARY_KEYS.name] = {
        'type': 'GenericSet',
        'values': ['stories']
    }

    row = add_histograms_queue.CreateRowEntities(hist, test_key, {}, 123)[0]
    row_dict = row.to_dict()

    self.assertNotIn('a_tracing_uri', row_dict)

  def testCreateRowEntities_DoesNotAddTraceUriIfDiagnosticIsEmpty(self):
    test_path = 'Chromium/win7/suite/metric/story'
    test_key = utils.TestKey(test_path)
    hist = copy.deepcopy(TEST_HISTOGRAM)
    hist['diagnostics'][reserved_infos.STORIES.name] = {
        'type': 'GenericSet',
        'values': ['story']
    }
    hist['diagnostics'][reserved_infos.TRACE_URLS.name]['values'] = []

    row = add_histograms_queue.CreateRowEntities(hist, test_key, {}, 123)[0]
    row_dict = row.to_dict()

    self.assertNotIn('a_tracing_uri', row_dict)


@mock.patch.object(SheriffConfigClient, '__init__',
                   mock.MagicMock(return_value=None))
@mock.patch.object(SheriffConfigClient, 'Match',
                   mock.MagicMock(return_value=([], None)))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class AddHistogramsQueueTestWithUploadCompletionToken(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    testing_common.SetIsInternalUser('foo@bar.com', True)
    self.SetCurrentUser('foo@bar.com')

  def _CreateHistogramWithMeasurementAndAdd(self, status=200):
    token_id = str(uuid.uuid4())
    test_path = 'Chromium/win7/suite/metric'
    token = upload_completion_token.Token(id=token_id).put().get()
    token.AddMeasurement(test_path, False).wait()
    token.UpdateState(upload_completion_token.State.COMPLETED)

    graph_data.Bot(
        key=ndb.Key('Master', 'Chromium', 'Bot', 'win7'),
        internal_only=False).put()
    params = [{
        'data': TEST_HISTOGRAM,
        'test_path': test_path,
        'benchmark_description': None,
        'revision': 123,
        'token': token_id
    }]
    upload_data = json.dumps(params)
    self.testapp.post('/add_histograms_queue', upload_data, status=status)
    return upload_completion_token.Token.get_by_id(token_id)

  def testPostHistogram_Success(self):
    token = self._CreateHistogramWithMeasurementAndAdd()
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)
    self.assertEqual(len(token.GetMeasurements()), 1)
    self.assertEqual(token.GetMeasurements()[0].error_message, None)

  @mock.patch.object(find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock(side_effect=Exception('Test exception')))
  def testPostHistogram_Fail(self):
    token = self._CreateHistogramWithMeasurementAndAdd()
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertEqual(len(token.GetMeasurements()), 1)
    self.assertEqual(token.GetMeasurements()[0].error_message, 'Test exception')

  @mock.patch.object(
      find_anomalies, 'ProcessTestsAsync',
      mock.MagicMock(
          side_effect=IOError(('Connection aborted.',
                               Exception(104, 'Connection reset by peer')))))
  def testPostHistogram_FailWithNonStringMessage(self):
    token = self._CreateHistogramWithMeasurementAndAdd()
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertEqual(len(token.GetMeasurements()), 1)
    self.assertEqual(
        token.GetMeasurements()[0].error_message,
        str(
            IOError(('Connection aborted.',
                     Exception(104, 'Connection reset by peer')))))

  @mock.patch.object(add_histograms_queue, '_ProcessRowAndHistogram',
                     mock.MagicMock(side_effect=Exception('Test exception')))
  def testPostHistogram_FailToCreateFixture(self):
    token = self._CreateHistogramWithMeasurementAndAdd(status=500)
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertEqual(len(token.GetMeasurements()), 1)
    self.assertEqual(token.GetMeasurements()[0].error_message, 'Test exception')

  def _CreateHistogramWithMultipleMeasurementAndAdd(self, status=200):
    test_path1 = 'Chromium/win7/suite/metric1'
    test_path2 = 'Chromium/win7/suite/metric2'
    test_path3 = 'Chromium/win7/suite/metric3'

    token_id = str(uuid.uuid4())
    token = upload_completion_token.Token(id=token_id).put().get()
    token.AddMeasurement(test_path1, False).wait()
    token.AddMeasurement(test_path2, False).wait()
    token.AddMeasurement(test_path3, False).wait()
    token.UpdateState(upload_completion_token.State.COMPLETED)

    graph_data.Bot(
        key=ndb.Key('Master', 'Chromium', 'Bot', 'win7'),
        internal_only=False).put()
    params = [
        {
            'data': TEST_HISTOGRAM,
            'test_path': test_path1,
            'benchmark_description': None,
            'revision': 123,
            'token': token_id
        },
        {
            'data': TEST_HISTOGRAM,
            'test_path': test_path2,
            'benchmark_description': None,
            'revision': 5,
            'token': token_id
        },
        {
            'data': TEST_HISTOGRAM,
            'test_path': test_path3,
            'benchmark_description': None,
            'revision': 42,
            'token': token_id
        },
    ]
    upload_data = json.dumps(params)
    self.testapp.post('/add_histograms_queue', upload_data, status=status)
    return upload_completion_token.Token.get_by_id(token_id)

  def testPostMultipleHistograms_Success(self):
    token = self._CreateHistogramWithMultipleMeasurementAndAdd()
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)
    self.assertTrue(
        all(measurement.state == upload_completion_token.State.COMPLETED
            for measurement in token.GetMeasurements()))
    self.assertTrue(
        all(measurement.error_message is None
            for measurement in token.GetMeasurements()))

  @mock.patch.object(find_anomalies, 'ProcessTestsAsync',
                     mock.MagicMock(side_effect=Exception('Test exception')))
  def testPostMultipleHistogram_Fail(self):
    token = self._CreateHistogramWithMultipleMeasurementAndAdd()
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertTrue(
        all(measurement.state == upload_completion_token.State.FAILED
            for measurement in token.GetMeasurements()))
    self.assertTrue(
        all(measurement.error_message == 'Test exception'
            for measurement in token.GetMeasurements()))

  @mock.patch.object(add_histograms_queue, '_ProcessRowAndHistogram',
                     mock.MagicMock(side_effect=Exception('Test exception')))
  def testPostMultipleHistogram_FailToCreateFixture(self):
    token = self._CreateHistogramWithMultipleMeasurementAndAdd(status=500)
    self.assertEqual(token.state, upload_completion_token.State.FAILED)
    self.assertTrue(
        all(measurement.state == upload_completion_token.State.FAILED
            for measurement in token.GetMeasurements()))
    self.assertTrue(
        all(measurement.error_message == 'Test exception'
            for measurement in token.GetMeasurements()))

  @mock.patch('logging.warning')
  def testPostMultipleHistogram_MeasurementExpired(self, mock_log):
    test_path1 = 'Chromium/win7/suite/metric1'
    test_path2 = 'Chromium/win7/suite/metric2'

    token_id = str(uuid.uuid4())
    token = upload_completion_token.Token(id=token_id).put().get()
    token.AddMeasurement(test_path1, False).wait()
    measurement2 = token.AddMeasurement(test_path2, False).get_result()
    token.UpdateState(upload_completion_token.State.COMPLETED)

    measurement2.key.delete()
    measurement2 = upload_completion_token.Measurement.GetByPath(
        test_path2, token_id)
    self.assertEqual(measurement2, None)

    graph_data.Bot(
        key=ndb.Key('Master', 'Chromium', 'Bot', 'win7'),
        internal_only=False).put()
    params = [
        {
            'data': TEST_HISTOGRAM,
            'test_path': test_path1,
            'benchmark_description': None,
            'revision': 123,
            'token': token_id
        },
        {
            'data': TEST_HISTOGRAM,
            'test_path': test_path2,
            'benchmark_description': None,
            'revision': 5,
            'token': token_id
        },
    ]
    upload_data = json.dumps(params)
    self.testapp.post('/add_histograms_queue', upload_data)

    token = upload_completion_token.Token.get_by_id(token_id)
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)
    mock_log.assert_called_once_with(
        'Upload completion token measurement could not be found. '
        'Token id: %s, measurement test path: %s', token_id, test_path2)
