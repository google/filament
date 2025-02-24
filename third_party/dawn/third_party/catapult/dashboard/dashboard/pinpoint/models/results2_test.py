# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
import itertools
import logging
from unittest import mock
import unittest

from google.appengine.api import taskqueue

from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models import results2
from dashboard.pinpoint.models.change import change
from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint.models.quest import read_value
from dashboard.pinpoint.models.quest import run_test
from dashboard.services import swarming
from dateutil.parser import isoparse
from tracing.value import histogram_set
from tracing.value import histogram as histogram_module

# pylint: disable=too-many-lines

_TEST_START_TIME = datetime.datetime.fromtimestamp(1326244364)
_TEST_START_TIME_STR = _TEST_START_TIME.strftime('%Y-%m-%d %H:%M:%S.%f')

_ATTEMPT_DATA = {
    "executions": [{
        "result_arguments": {
            "isolate_server": "https://isolateserver.appspot.com",
            "isolate_hash": "e26a40a0d4",
        }
    }]
}

_JOB_NO_DIFFERENCES = {
    "state": [
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'next': 'same'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'next': 'same',
                'prev': 'same'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'next': 'same',
                'prev': 'same'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'prev': 'same'
            },
        },
    ],
    "quests": ["Test"],
}

_JOB_WITH_DIFFERENCES = {
    "state": [
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'next': 'same'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'prev': 'same',
                'next': 'different'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'prev': 'different',
                'next': 'different'
            },
        },
        {
            "attempts": [_ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'prev': 'different'
            },
        },
    ],
    "quests": ["Test"],
}

_JOB_MISSING_EXECUTIONS = {
    "state": [
        {
            "attempts": [_ATTEMPT_DATA, {
                "executions": []
            }],
            "change": {},
            "comparisons": {
                'next': 'same'
            },
        },
        {
            "attempts": [{
                "executions": []
            }, _ATTEMPT_DATA],
            "change": {},
            "comparisons": {
                'prev': 'same'
            },
        },
    ],
    "quests": ["Test"],
}

FakeBenchmarkArguments = collections.namedtuple(
    'FakeBenchmarkArguments', ['benchmark', 'story'])


@mock.patch.object(results2.cloudstorage, 'listbucket')
class GetCachedResults2Test(unittest.TestCase):

  def testGetCachedResults2_Cached_ReturnsResult(self, mock_cloudstorage):
    mock_cloudstorage.return_value = ['foo']

    job = _JobStub(_JOB_WITH_DIFFERENCES, '123', job_state.PERFORMANCE)
    url = results2.GetCachedResults2(job)

    self.assertEqual(
        'https://storage.cloud.google.com/results2-public/'
        '%s.html' % job.job_id, url)

  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: True)
  def testGetCachedResults2_Cached_ReturnsResult_stage(self, mock_cloudstorage):
    mock_cloudstorage.return_value = ['foo']

    job = _JobStub(_JOB_WITH_DIFFERENCES, '123', job_state.PERFORMANCE)
    url = results2.GetCachedResults2(job)

    self.assertEqual(
        'https://storage.cloud.google.com/chromeperf-staging-results2-public/'
        '%s.html' % job.job_id, url)

  @mock.patch.object(results2, 'ScheduleResults2Generation', mock.MagicMock())
  def testGetCachedResults2_Uncached_Fails(self, mock_cloudstorage):
    mock_cloudstorage.return_value = []

    job = _JobStub(_JOB_WITH_DIFFERENCES, '123', job_state.PERFORMANCE)
    url = results2.GetCachedResults2(job)

    self.assertIsNone(url)


class ScheduleResults2Generation2Test(unittest.TestCase):

  @mock.patch.object(results2.taskqueue, 'add')
  def testScheduleResults2Generation2_FailedPreviously(self, mock_add):
    mock_add.side_effect = taskqueue.TombstonedTaskError

    job = _JobStub(_JOB_WITH_DIFFERENCES, '123', job_state.PERFORMANCE)
    result = results2.ScheduleResults2Generation(job)
    self.assertFalse(result)

  @mock.patch.object(results2.taskqueue, 'add')
  def testScheduleResults2Generation2_AlreadyRunning(self, mock_add):
    mock_add.side_effect = taskqueue.TaskAlreadyExistsError

    job = _JobStub(_JOB_WITH_DIFFERENCES, '123', job_state.PERFORMANCE)
    result = results2.ScheduleResults2Generation(job)
    self.assertTrue(result)


@mock.patch.object(
    results2, 'open', mock.mock_open(read_data='fake_viewer'), create=True)
class GenerateResults2Test(testing_common.TestCase):

  @mock.patch.object(
      results2, '_FetchHistograms',
      mock.MagicMock(return_value=[
          results2.HistogramData(None, ['a', 'b'])
      ]))
  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  def testPost_Renders(self, mock_render):
    job = _JobStub(None, '123', job_state.PERFORMANCE)
    results2.GenerateResults2(job)

    mock_render.assert_called_with([['a', 'b']],
                                   mock.ANY,
                                   reset_results=True,
                                   vulcanized_html='fake_viewer')

    results = results2.CachedResults2.query().fetch()
    self.assertEqual(1, len(results))

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  def testTypeDispatch_LegacyHistogramExecution(self, mock_json, mock_render):
    job = _JobStub(
        None, '123', job_state.PERFORMANCE,
        _JobStateFake({
            'f00c0de': [{
                'executions': [
                    read_value._ReadHistogramsJsonValueExecution(
                        'fake_filename', 'fake_metric', 'fake_grouping',
                        'fake_trace_or_story', 'avg', 'https://isolate_server',
                        'deadc0decafef00d')
                ]
            }]
        }))
    histograms = []

    def TraverseHistograms(hists, *unused_args, **unused_kw_args):
      for histogram in hists:
        histograms.append(histogram)

    mock_render.side_effect = TraverseHistograms
    histogram = histogram_module.Histogram('histogram', 'count')
    histogram.AddSample(0)
    histogram.AddSample(1)
    histogram.AddSample(2)
    expected_histogram_set = histogram_set.HistogramSet([histogram])
    mock_json.return_value = expected_histogram_set.AsDicts()
    results2.GenerateResults2(job)
    mock_render.assert_called_with(
        mock.ANY, mock.ANY, reset_results=True, vulcanized_html='fake_viewer')
    results = results2.CachedResults2.query().fetch()
    self.assertEqual(1, len(results))
    self.assertEqual(expected_histogram_set.AsDicts(), histograms)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  def testTypeDispatch_LegacyGraphJsonExecution(self, mock_json, mock_render):
    job = _JobStub(
        None, '123', job_state.PERFORMANCE,
        _JobStateFake({
            'f00c0de': [{
                'executions': [
                    read_value._ReadGraphJsonValueExecution(
                        'fake_filename', 'fake_chart', 'fake_trace',
                        'https://isolate_server', 'deadc0decafef00d')
                ]
            }]
        }))
    histograms = []

    def TraverseHistograms(hists, *unused_args, **unused_kw_args):
      for histogram in hists:
        histograms.append(histogram)

    mock_render.side_effect = TraverseHistograms
    mock_json.return_value = {
        'fake_chart': {
            'traces': {
                'fake_trace': ['12345.6789', '0.0']
            }
        }
    }

    results2.GenerateResults2(job)
    mock_render.assert_called_with(
        mock.ANY, mock.ANY, reset_results=True, vulcanized_html='fake_viewer')
    results = results2.CachedResults2.query().fetch()
    self.assertEqual(1, len(results))
    # TODO(dberris): check more precisely the contents of the histograms.
    self.assertEqual([mock.ANY, mock.ANY], histograms)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  def testTypeDispatch_ReadValueExecution(self, mock_json, mock_render):
    job = _JobStub(
        None, '123', job_state.PERFORMANCE,
        _JobStateFake({
            'f00c0de': [{
                'executions': [
                    read_value.ReadValueExecution(
                        'fake_filename', ['fake_filename'], 'fake_metric',
                        'fake_grouping_label', 'fake_trace_or_story', 'avg',
                        'fake_chart', 'https://isolate_server',
                        'deadc0decafef00d')
                ]
            }]
        }))
    histograms = []

    def TraverseHistograms(hists, *args, **kw_args):
      del args
      del kw_args
      for histogram in hists:
        histograms.append(histogram)

    mock_render.side_effect = TraverseHistograms
    histogram = histogram_module.Histogram('histogram', 'count')
    histogram.AddSample(0)
    histogram.AddSample(1)
    histogram.AddSample(2)
    expected_histogram_set = histogram_set.HistogramSet([histogram])
    mock_json.return_value = expected_histogram_set.AsDicts()
    results2.GenerateResults2(job)
    mock_render.assert_called_with(
        mock.ANY, mock.ANY, reset_results=True, vulcanized_html='fake_viewer')
    results = results2.CachedResults2.query().fetch()
    self.assertEqual(1, len(results))
    self.assertEqual(expected_histogram_set.AsDicts(), histograms)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  def testTypeDispatch_ReadValueExecution_MultipleChanges(
      self, mock_json, mock_render):
    job = _JobStub(
        None, '123', job_state.PERFORMANCE,
        _JobStateFake({
            'f00c0de': [{
                'executions': [
                    read_value.ReadValueExecution(
                        'fake_filename', ['fake_filename'], 'fake_metric',
                        'fake_grouping_label', 'fake_trace_or_story', 'avg',
                        'fake_chart', 'https://isolate_server',
                        'deadc0decafef00d')
                ]
            }],
            'badc0de': [{
                'executions': [
                    read_value.ReadValueExecution(
                        'fake_filename', ['fake_filename'], 'fake_metric',
                        'fake_grouping_label', 'fake_trace_or_story', 'avg',
                        'fake_chart', 'https://isolate_server',
                        'deadc0decafef00d')
                ]
            }]
        }))
    histograms = []

    def TraverseHistograms(hists, *args, **kw_args):
      del args
      del kw_args
      for histogram in hists:
        histograms.append(histogram)

    mock_render.side_effect = TraverseHistograms
    histogram_a = histogram_module.Histogram('histogram', 'count')
    histogram_a.AddSample(0)
    histogram_a.AddSample(1)
    histogram_a.AddSample(2)
    expected_histogram_set_a = histogram_set.HistogramSet([histogram_a])
    histogram_b = histogram_module.Histogram('histogram', 'count')
    histogram_b.AddSample(0)
    histogram_b.AddSample(1)
    histogram_b.AddSample(2)
    expected_histogram_set_b = histogram_set.HistogramSet([histogram_b])

    mock_json.side_effect = (expected_histogram_set_a.AsDicts(),
                             expected_histogram_set_b.AsDicts())
    results2.GenerateResults2(job)
    mock_render.assert_called_with(
        mock.ANY, mock.ANY, reset_results=True, vulcanized_html='fake_viewer')
    results = results2.CachedResults2.query().fetch()
    self.assertEqual(1, len(results))
    self.assertEqual(
        expected_histogram_set_a.AsDicts() + expected_histogram_set_b.AsDicts(),
        histograms)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_CWV(self, mock_commit_info, mock_swarming,
                                     mock_json, mock_render, mock_bqinsert):
    expected_histogram_set = histogram_set.HistogramSet([
        _CreateHistogram('largestContentfulPaint', 42),
        _CreateHistogram('timeToFirstContentfulPaint', 11),
        _CreateHistogram('overallCumulativeLayoutShift', 22),
        _CreateHistogram('totalBlockingTime', 33),
        _CreateHistogram('someUselessMetric', 42)
    ])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set)

    expected_rows = [{
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['os1', 'os2']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 0
            },
            'checkout': {
                'repo': 'fakerepo',
                'git_hash': 'fakehashA',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'branch': 'refs/heads/main'
            }
        },
        'measures': {
            'core_web_vitals': {
                'timeToFirstContentfulPaint': 11.0,
                'totalBlockingTime': 33.0,
                'largestContentfulPaint': 42.0,
                'overallCumulativeLayoutShift': 22.0
            },
            'speedometer2': {},
            'motionmark': {},
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }, {
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['os1', 'os2']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 1
            },
            'checkout': {
                'patch_gerrit_revision': 'fake_patch_set',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'patch_gerrit_change': 'fake_patch_issue',
                'repo': 'fakeRepo',
                'branch': 'refs/heads/main',
                'git_hash': 'fakehashB'
            }
        },
        'measures': {
            'core_web_vitals': {
                'timeToFirstContentfulPaint': 11.0,
                'totalBlockingTime': 33.0,
                'largestContentfulPaint': 42.0,
                'overallCumulativeLayoutShift': 22.0
            },
            'speedometer2': {},
            'motionmark': {},
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }]

    results2.GenerateResults2(job)
    self.maxDiff = None
    self.assertCountEqual(mock_bqinsert.call_args_list[0][0][3], expected_rows)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_Speedometer2(self, mock_commit_info,
                                              mock_swarming, mock_json,
                                              mock_render, mock_bqinsert):
    expected_histogram_set = histogram_set.HistogramSet([
        _CreateHistogram('Angular2-TypeScript-TodoMVC', 1),
        _CreateHistogram('AngularJS-TodoMVC', 2),
        _CreateHistogram('BackboneJS-TodoMVC', 3),
        _CreateHistogram('Elm-TodoMVC', 4),
        _CreateHistogram('EmberJS-Debug-TodoMVC', 5),
        _CreateHistogram('EmberJS-TodoMVC', 6),
        _CreateHistogram('Flight-TodoMVC', 7),
        _CreateHistogram('Inferno-TodoMVC', 8),
        _CreateHistogram('jQuery-TodoMVC', 9),
        _CreateHistogram('Preact-TodoMVC', 10),
        _CreateHistogram('React-Redux-TodoMVC', 11),
        _CreateHistogram('React-TodoMVC', 12),
        _CreateHistogram('Vanilla-ES2015-Babel-Webpack-TodoMVC', 13),
        _CreateHistogram('Vanilla-ES2015-TodoMVC', 14),
        _CreateHistogram('VanillaJS-TodoMVC', 15),
        _CreateHistogram('VueJS-TodoMVC', 16),
        _CreateHistogram('RunsPerMinute', 17)
    ])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set, set_device_os=False)

    expected_rows = [{
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 0
            },
            'checkout': {
                'repo': 'fakerepo',
                'git_hash': 'fakehashA',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'branch': 'refs/heads/main'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {
                'Angular2_TypeScript_TodoMVC': 1,
                'AngularJS_TodoMVC': 2,
                'BackboneJS_TodoMVC': 3,
                'Elm_TodoMVC': 4,
                'EmberJS_Debug_TodoMVC': 5,
                'EmberJS_TodoMVC': 6,
                'Flight_TodoMVC': 7,
                'Inferno_TodoMVC': 8,
                'jQuery_TodoMVC': 9,
                'Preact_TodoMVC': 10,
                'React_Redux_TodoMVC': 11,
                'React_TodoMVC': 12,
                'RunsPerMinute': 17,
                'Vanilla_ES2015_Babel_Webpack_TodoMVC': 13,
                'Vanilla_ES2015_TodoMVC': 14,
                'VanillaJS_TodoMVC': 15,
                'VueJS_TodoMVC': 16
            },
            'motionmark': {},
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }, {
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 1
            },
            'checkout': {
                'patch_gerrit_revision': 'fake_patch_set',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'patch_gerrit_change': 'fake_patch_issue',
                'repo': 'fakeRepo',
                'branch': 'refs/heads/main',
                'git_hash': 'fakehashB'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {
                'Angular2_TypeScript_TodoMVC': 1,
                'AngularJS_TodoMVC': 2,
                'BackboneJS_TodoMVC': 3,
                'Elm_TodoMVC': 4,
                'EmberJS_Debug_TodoMVC': 5,
                'EmberJS_TodoMVC': 6,
                'Flight_TodoMVC': 7,
                'Inferno_TodoMVC': 8,
                'jQuery_TodoMVC': 9,
                'Preact_TodoMVC': 10,
                'React_Redux_TodoMVC': 11,
                'React_TodoMVC': 12,
                'RunsPerMinute': 17,
                'Vanilla_ES2015_Babel_Webpack_TodoMVC': 13,
                'Vanilla_ES2015_TodoMVC': 14,
                'VanillaJS_TodoMVC': 15,
                'VueJS_TodoMVC': 16
            },
            'motionmark': {},
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }]

    results2.GenerateResults2(job)
    self.maxDiff = None
    self.assertCountEqual(mock_bqinsert.call_args_list[0][0][3], expected_rows)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_MotionMark(self, mock_commit_info,
                                            mock_swarming, mock_json,
                                            mock_render, mock_bqinsert):
    expected_histogram_set = histogram_set.HistogramSet([
        _CreateHistogram('motionmark', 1),
    ])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set, set_device_os=False)

    expected_rows = [{
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 0
            },
            'checkout': {
                'repo': 'fakerepo',
                'git_hash': 'fakehashA',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'branch': 'refs/heads/main'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {},
            'motionmark': {
                'motionmark': 1
            },
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }, {
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 1
            },
            'checkout': {
                'patch_gerrit_revision': 'fake_patch_set',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'patch_gerrit_change': 'fake_patch_issue',
                'repo': 'fakeRepo',
                'branch': 'refs/heads/main',
                'git_hash': 'fakehashB'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {},
            'motionmark': {
                'motionmark': 1
            },
            'jetstream2': {},
        },
        'run_id': 'fake_job_id'
    }]

    results2.GenerateResults2(job)
    self.maxDiff = None
    self.assertCountEqual(mock_bqinsert.call_args_list[0][0][3], expected_rows)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_Jetstream2(self, mock_commit_info,
                                            mock_swarming, mock_json,
                                            mock_render, mock_bqinsert):
    expected_histogram_set = histogram_set.HistogramSet([
        _CreateHistogram('Score', 1),
        _CreateHistogram('3d-cube-SP.Average', 2),
        _CreateHistogram('3d-raytrace-SP.Average', 3),
        _CreateHistogram('Air.Average', 4),
        _CreateHistogram('Babylon.Average', 5),
        _CreateHistogram('Basic.Average', 6),
        _CreateHistogram('Box2D.Average', 7),
        _CreateHistogram('FlightPlanner.Average', 8),
        _CreateHistogram('HashSet-wasm.Runtime', 9),
        _CreateHistogram('ML.Average', 10),
        _CreateHistogram('OfflineAssembler.Average', 11),
        _CreateHistogram('UniPoker.Average', 12),
        _CreateHistogram('WSL.MainRun', 13),
        _CreateHistogram('acorn-wtb.Average', 14),
        _CreateHistogram('ai-astar.Average', 15),
        _CreateHistogram('async-fs.Average', 16),
        _CreateHistogram('babylon-wtb.Average', 17),
        _CreateHistogram('base64-SP.Average', 18),
        _CreateHistogram('bomb-workers.Average', 19),
        _CreateHistogram('cdjs.Average', 20),
        _CreateHistogram('chai-wtb.Average', 21),
        _CreateHistogram('coffeescript-wtb.Average', 22),
        _CreateHistogram('crypto-aes-SP.Average', 23),
        _CreateHistogram('crypto-md5-SP.Average', 24),
        _CreateHistogram('crypto-shal-SP.Average', 25),
        _CreateHistogram('crypto.Average', 26),
        _CreateHistogram('date-format-tofte-SP.Average', 27),
        _CreateHistogram('date-format-xparb-SP.Average', 28),
        _CreateHistogram('delta-blue.Average', 29),
        _CreateHistogram('earley-boyer.Average', 30),
        _CreateHistogram('espree-wtb.Average', 31),
        _CreateHistogram('first-inspector-code-load.Average', 32),
        _CreateHistogram('float-mm_c.Average', 33),
        _CreateHistogram('gaussian-blur.Average', 34),
        _CreateHistogram('gbemu.Average', 35),
        _CreateHistogram('gcc-loops-wasm.Runtime', 36),
        _CreateHistogram('hash-map.Average', 37),
        _CreateHistogram('jshint-wtb.Average', 38),
        _CreateHistogram('json-parse-inspector.Average', 39),
        _CreateHistogram('json-stringify-inspector.Average', 40),
        _CreateHistogram('lebab-wtb.Average', 41),
        _CreateHistogram('mandreel.Average', 42),
        _CreateHistogram('multi-inspector-code-load.Average', 43),
        _CreateHistogram('n-body-SP.Average', 44),
        _CreateHistogram('navier-stokes.Average', 45),
        _CreateHistogram('octane-code-load.Average', 46),
        _CreateHistogram('octane-zlib.Average', 47),
        _CreateHistogram('pdfjs.Average', 48),
        _CreateHistogram('prepack-wtb.Average', 49),
        _CreateHistogram('quicksort-wasm.Runtime', 50),
        _CreateHistogram('raytrace.Average', 51),
        _CreateHistogram('regex-dna-SP.Average', 52),
        _CreateHistogram('regexp.Average', 53),
        _CreateHistogram('richards-wasm.Runtime', 54),
        _CreateHistogram('richards.Average', 55),
        _CreateHistogram('segmentation.Average', 56),
        _CreateHistogram('splay.Average', 57),
        _CreateHistogram('stanford-crypto-aes.Average', 58),
        _CreateHistogram('stanford-crypto-pbkdf2.Average', 59),
        _CreateHistogram('stanford-crypto-sha256.Average', 60),
        _CreateHistogram('string-unpack-code-SP.Average', 61),
        _CreateHistogram('tagcloud-SP.Average', 62),
        _CreateHistogram('tsf-wasm.Runtime', 63),
        _CreateHistogram('typescript.Average', 64),
        _CreateHistogram('uglify-js-wtb.Average', 65)
    ])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set, set_device_os=False)

    expected_rows = [{
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 0
            },
            'checkout': {
                'repo': 'fakerepo',
                'git_hash': 'fakehashA',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'branch': 'refs/heads/main'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {},
            'motionmark': {},
            'jetstream2': {
                'Score': 1,
                'cube_3d_SP_Average': 2,
                'raytrace_3d_SP_Average': 3,
                'Air_Average': 4,
                'Babylon_Average': 5,
                'Basic_Average': 6,
                'Box2D_Average': 7,
                'FlightPlanner_Average': 8,
                'HashSet_wasm_Runtime': 9,
                'ML_Average': 10,
                'OfflineAssembler_Average': 11,
                'UniPoker_Average': 12,
                'WSL_MainRun': 13,
                'acorn_wtb_Average': 14,
                'ai_astar_Average': 15,
                'async_fs_Average': 16,
                'babylon_wtb_Average': 17,
                'base64_SP_Average': 18,
                'bomb_workers_Average': 19,
                'cdjs_Average': 20,
                'chai_wtb_Average': 21,
                'coffeescript_wtb_Average': 22,
                'crypto_aes_SP_Average': 23,
                'crypto_md5_SP_Average': 24,
                'crypto_shal_SP_Average': 25,
                'crypto_Average': 26,
                'date_format_tofte_SP_Average': 27,
                'date_format_xparb_SP_Average': 28,
                'delta_blue_Average': 29,
                'earley_boyer_Average': 30,
                'espree_wtb_Average': 31,
                'first_inspector_code_load_Average': 32,
                'float_mm_c_Average': 33,
                'gaussian_blur_Average': 34,
                'gbemu_Average': 35,
                'gcc_loops_wasm_Runtime': 36,
                'hash_map_Average': 37,
                'jshint_wtb_Average': 38,
                'json_parse_inspector_Average': 39,
                'json_stringify_inspector_Average': 40,
                'lebab_wtb_Average': 41,
                'mandreel_Average': 42,
                'multi_inspector_code_load_Average': 43,
                'n_body_SP_Average': 44,
                'navier_stokes_Average': 45,
                'octane_code_load_Average': 46,
                'octane_zlib_Average': 47,
                'pdfjs_Average': 48,
                'prepack_wtb_Average': 49,
                'quicksort_wasm_Runtime': 50,
                'raytrace_Average': 51,
                'regex_dna_SP_Average': 52,
                'regexp_Average': 53,
                'richards_wasm_Runtime': 54,
                'richards_Average': 55,
                'segmentation_Average': 56,
                'splay_Average': 57,
                'stanford_crypto_aes_Average': 58,
                'stanford_crypto_pbkdf2_Average': 59,
                'stanford_crypto_sha256_Average': 60,
                'string_unpack_code_SP_Average': 61,
                'tagcloud_SP_Average': 62,
                'tsf_wasm_Runtime': 63,
                'typescript_Average': 64,
                'uglify_js_wtb_Average': 65
            },
        },
        'run_id': 'fake_job_id'
    }, {
        'job_start_time': _TEST_START_TIME_STR,
        'batch_id': 'fake_batch_id',
        'attempt_count': 2,
        'dims': {
            'start_time': '2022-06-09 20:21:22.123456',
            'swarming_task_id': 'a4b',
            'device': {
                'cfg': 'fake_configuration',
                'swarming_bot_id': 'fake_id',
                'os': ['base_os']
            },
            'test_info': {
                'story': 'fake_story',
                'benchmark': 'fake_benchmark'
            },
            'pairing': {
                'replica': 0,
                'variant': 1
            },
            'checkout': {
                'patch_gerrit_revision': 'fake_patch_set',
                'commit_position': 437745,
                'commit_created': '2021-12-08 00:00:00.000000',
                'patch_gerrit_change': 'fake_patch_issue',
                'repo': 'fakeRepo',
                'branch': 'refs/heads/main',
                'git_hash': 'fakehashB'
            }
        },
        'measures': {
            'core_web_vitals': {},
            'speedometer2': {},
            'motionmark': {},
            'jetstream2': {
                'Score': 1,
                'cube_3d_SP_Average': 2,
                'raytrace_3d_SP_Average': 3,
                'Air_Average': 4,
                'Babylon_Average': 5,
                'Basic_Average': 6,
                'Box2D_Average': 7,
                'FlightPlanner_Average': 8,
                'HashSet_wasm_Runtime': 9,
                'ML_Average': 10,
                'OfflineAssembler_Average': 11,
                'UniPoker_Average': 12,
                'WSL_MainRun': 13,
                'acorn_wtb_Average': 14,
                'ai_astar_Average': 15,
                'async_fs_Average': 16,
                'babylon_wtb_Average': 17,
                'base64_SP_Average': 18,
                'bomb_workers_Average': 19,
                'cdjs_Average': 20,
                'chai_wtb_Average': 21,
                'coffeescript_wtb_Average': 22,
                'crypto_aes_SP_Average': 23,
                'crypto_md5_SP_Average': 24,
                'crypto_shal_SP_Average': 25,
                'crypto_Average': 26,
                'date_format_tofte_SP_Average': 27,
                'date_format_xparb_SP_Average': 28,
                'delta_blue_Average': 29,
                'earley_boyer_Average': 30,
                'espree_wtb_Average': 31,
                'first_inspector_code_load_Average': 32,
                'float_mm_c_Average': 33,
                'gaussian_blur_Average': 34,
                'gbemu_Average': 35,
                'gcc_loops_wasm_Runtime': 36,
                'hash_map_Average': 37,
                'jshint_wtb_Average': 38,
                'json_parse_inspector_Average': 39,
                'json_stringify_inspector_Average': 40,
                'lebab_wtb_Average': 41,
                'mandreel_Average': 42,
                'multi_inspector_code_load_Average': 43,
                'n_body_SP_Average': 44,
                'navier_stokes_Average': 45,
                'octane_code_load_Average': 46,
                'octane_zlib_Average': 47,
                'pdfjs_Average': 48,
                'prepack_wtb_Average': 49,
                'quicksort_wasm_Runtime': 50,
                'raytrace_Average': 51,
                'regex_dna_SP_Average': 52,
                'regexp_Average': 53,
                'richards_wasm_Runtime': 54,
                'richards_Average': 55,
                'segmentation_Average': 56,
                'splay_Average': 57,
                'stanford_crypto_aes_Average': 58,
                'stanford_crypto_pbkdf2_Average': 59,
                'stanford_crypto_sha256_Average': 60,
                'string_unpack_code_SP_Average': 61,
                'tagcloud_SP_Average': 62,
                'tsf_wasm_Runtime': 63,
                'typescript_Average': 64,
                'uglify_js_wtb_Average': 65
            },
        },
        'run_id': 'fake_job_id'
    }]

    results2.GenerateResults2(job)
    self.maxDiff = None
    self.assertCountEqual(mock_bqinsert.call_args_list[0][0][3], expected_rows)

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_NoRows(self, mock_commit_info, mock_swarming,
                                        mock_json, mock_render, mock_bqinsert):
    useless_histogram = histogram_module.Histogram('someUselessMetric', 'count')
    useless_histogram.AddSample(42)
    expected_histogram_set = histogram_set.HistogramSet([useless_histogram])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set)

    results2.GenerateResults2(job)
    self.assertEqual(1, len(mock_bqinsert.call_args_list))

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_RequestInsertBQRows')
  @mock.patch.object(results2, '_BQService')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_CH_Many_Rows(self, mock_commit_info,
                                           mock_swarming, mock_json,
                                           mock_render, mock_bqservice,
                                           mock_reqbqinsert):
    histograms = []
    for i in range(1001):
      some_histogram = histogram_module.Histogram('someMetric-' + str(i),
                                                  'count')
      some_histogram.AddSample(i)
      histograms.append(some_histogram)
    expected_histogram_set = histogram_set.HistogramSet(histograms)
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set)

    results2.GenerateResults2(job)
    self.assertEqual(1, len(mock_bqservice.call_args_list))
    self.assertEqual(3, len(mock_reqbqinsert.call_args_list))

  @mock.patch.object(results2, '_GcsFileStream', mock.MagicMock())
  @mock.patch.object(results2, '_InsertBQRows')
  @mock.patch.object(results2.render_histograms_viewer,
                     'RenderHistogramsViewer')
  @mock.patch.object(results2, '_JsonFromExecution')
  @mock.patch.object(swarming, 'Swarming')
  @mock.patch.object(commit.Commit, 'GetOrCacheCommitInfo')
  def testTypeDispatch_PushBQ_General(self, mock_commit_info, mock_swarming,
                                      mock_json, mock_render, mock_bqinsert):
    expected_histogram_set = histogram_set.HistogramSet([
        _CreateHistogram('largestContentfulPaint', 42),
        _CreateHistogram('someUselessMetric', 42)
    ])
    job = _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                       expected_histogram_set)

    ck_a = {
        'repo': 'fakerepo',
        'git_hash': 'fakehashA',
        'commit_position': 437745,
        'branch': 'refs/heads/main',
        'commit_created': '2021-12-08 00:00:00.000000'
    }
    ck_b = {
        'patch_gerrit_revision': 'fake_patch_set',
        'commit_position': 437745,
        'patch_gerrit_change': 'fake_patch_issue',
        'repo': 'fakeRepo',
        'branch': 'refs/heads/main',
        'git_hash': 'fakehashB',
        'commit_created': '2021-12-08 00:00:00.000000'
    }

    expected_rows = [
        _CreateGeneralRow(ck_a, 0, 'largestContentfulPaint', [42]),
        _CreateGeneralRow(ck_a, 0, 'someUselessMetric', [42]),
        _CreateGeneralRow(ck_b, 1, 'largestContentfulPaint', [42]),
        _CreateGeneralRow(ck_b, 1, 'someUselessMetric', [42])
    ]

    results2.GenerateResults2(job)
    self.maxDiff = None
    self.assertCountEqual(mock_bqinsert.call_args_list[1][0][3], expected_rows)


def _CreateGeneralRow(checkout, variant, metric, values):
  return {
      'job_start_time': _TEST_START_TIME_STR,
      'batch_id': 'fake_batch_id',
      'attempt_count': 2,
      'dims': {
          'start_time': '2022-06-09 20:21:22.123456',
          'swarming_task_id': 'a4b',
          'device': {
              'cfg': 'fake_configuration',
              'swarming_bot_id': 'fake_id',
              'os': ['os1', 'os2']
          },
          'test_info': {
              'story': 'fake_story',
              'benchmark': 'fake_benchmark'
          },
          'pairing': {
              'replica': 0,
              'variant': variant
          },
          'checkout': checkout
      },
      'run_id': 'fake_job_id',
      'metric': metric,
      'values': values
  }


def _CreateHistogram(name, val):
  h = histogram_module.Histogram(name, 'count')
  h.AddSample(val)
  return h


def _SetupBQTest(mock_commit_info, mock_swarming, mock_render, mock_json,
                 expected_histogram_set, set_device_os=True):
  mock_commit_info.return_value = {
      'author': {
          'email': 'author@chromium.org'
      },
      'created': isoparse('2021-12-08'),
      'commit': 'aaa7336',
      'committer': {
          'time': 'Fri Jan 01 00:01:00 2016'
      },
      'message': 'Subject.\n\n'
                 'Commit message.\n'
                 'Reviewed-on: https://foo/c/chromium/src/+/123\n'
                 'Cr-Commit-Position: refs/heads/main@{#437745}',
  }

  test_execution = run_test._RunTestExecution(None, "fake_server", None, None,
                                              None, None, None, None, None)
  test_execution._task_id = "fake_task"

  commit_a = commit.Commit("fakerepo", "fakehashA")
  change_a = change.Change([commit_a], variant=0)
  commit_b = commit.Commit("fakeRepo", "fakehashB")
  patch_b = FakePatch("fakePatchServer", "fakePatchNo", "fakePatchRev")
  change_b = change.Change([commit_b], patch_b, variant=1)

  benchmark_arguments = FakeBenchmarkArguments("fake_benchmark", "fake_story")
  job = _JobStub(
      None,
      'fake_job_id',
      None,
      _JobStateFake({
          change_a: [{
              'executions': [
                  test_execution,
                  read_value.ReadValueExecution(
                      'fake_filename', ['fake_filename'], 'fake_metric',
                      'fake_grouping_label', 'fake_trace_or_story', 'avg',
                      'fake_chart', 'https://isolate_server',
                      'deadc0decafef00d')
              ]
          }],
          change_b: [{
              'executions': [
                  test_execution,
                  read_value.ReadValueExecution(
                      'fake_filename', ['fake_filename'], 'fake_metric',
                      'fake_grouping_label', 'fake_trace_or_story', 'avg',
                      'fake_chart', 'https://isolate_server',
                      'deadc0decafef00d')
              ]
          }],
      }),
      benchmark_arguments=benchmark_arguments,
      batch_id="fake_batch_id",
      configuration="fake_configuration")
  histograms = []

  def TraverseHistograms(hists, *args, **kw_args):
    del args
    del kw_args
    for histogram in hists:
      histograms.append(histogram)

  task_mock = mock.Mock()
  bot_dimensions = [
      {
          "key": "device_type",
          "value": "type"
      },
      {
          "key": "os",
          "value": ["base_os"]
      },
      {
          "key": "id",
          "value": ["fake_id"]
      }
  ]
  if set_device_os:
    bot_dimensions.append({
        "key": "device_os",
        "value": ["os1", "os2"]
        })
  task_mock.Result.return_value = {
      "bot_dimensions": bot_dimensions,
      "started_ts": u"2022-06-09T20:21:22.123456",
      "run_id": "a4b"
  }
  mock_swarming.return_value.Task.return_value = task_mock
  mock_render.side_effect = TraverseHistograms
  mock_json.return_value = expected_histogram_set.AsDicts()
  return job


class FakePatch(
    collections.namedtuple('GerritPatch', ('server', 'change', 'revision'))):

  def BuildParameters(self):
    return {
        "patch_gerrit_url": "fake_gerrit_url",
        "project": "fake_project",
        "patch_issue": "fake_patch_issue",
        "patch_set": "fake_patch_set"
    }


class _AttemptFake:

  def __init__(self, attempt):
    self._attempt = attempt

  @property
  def executions(self):
    logging.debug('Attempt.executions = %s', self._attempt['executions'])
    return self._attempt['executions']

  def __str__(self):
    return '%s' % (self._attempt,)


class _JobStateFake:

  def __init__(self, attempts):
    self._attempts = {
        change: [_AttemptFake(attempt)]
        for change, attempt_list in attempts.items() for attempt in attempt_list
    }
    logging.debug('JobStateFake = %s', self._attempts)
    self.attempt_count = len(self._attempts)

  @property
  def _changes(self):
    changes = list(self._attempts.keys())
    logging.debug('JobStateFake._changes = %s', changes)
    return changes

  def Differences(self):

    def Pairwise(iterable):
      a, b = itertools.tee(iterable)
      next(b, None)
      return zip(a, b)

    return list(Pairwise(list(self._attempts.keys())))


class _JobStub:

  def __init__(self,
               job_dict,
               job_id,
               comparison_mode,
               state=None,
               batch_id=None,
               configuration=None,
               benchmark_arguments=None):
    self._job_dict = job_dict
    self.comparison_mode = comparison_mode
    self.job_id = job_id
    self.state = state
    self.batch_id = batch_id
    self.configuration = configuration
    self.benchmark_arguments = benchmark_arguments
    self.started_time = _TEST_START_TIME

  def AsDict(self, options=None):
    del options
    return self._job_dict
