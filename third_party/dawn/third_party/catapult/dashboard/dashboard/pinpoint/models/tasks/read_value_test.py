# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for the Evaluator implementation of the ReadValue quest."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import itertools
import json
from unittest import mock

from dashboard.pinpoint import test
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import event as event_module
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.tasks import bisection_test_util
from dashboard.pinpoint.models.tasks import find_isolate
from dashboard.pinpoint.models.tasks import read_value
from dashboard.pinpoint.models.tasks import run_test
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

@mock.patch('dashboard.services.isolate.Retrieve')
class EvaluatorTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      self.job = job_module.Job.New((), ())
    # Set up a common evaluator for all the test cases.
    self.evaluator = evaluators.SequenceEvaluator(
        evaluators=(
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('find_isolate'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
            evaluators.FilteringEvaluator(
                predicate=evaluators.TaskTypeEq('run_test'),
                delegate=evaluators.SequenceEvaluator(
                    evaluators=(
                        bisection_test_util.FakeSuccessfulRunTest(self.job),
                        evaluators.TaskPayloadLiftingEvaluator()))),
            read_value.Evaluator(self.job),
        ))

  def PopulateTaskGraph(self,
                        benchmark=None,
                        chart=None,
                        grouping_label=None,
                        story=None,
                        statistic=None,
                        trace='some_trace',
                        mode='histogram_sets'):
    task_module.PopulateTaskGraph(
        self.job,
        read_value.CreateGraph(
            read_value.TaskOptions(
                test_options=run_test.TaskOptions(
                    build_options=find_isolate.TaskOptions(
                        builder='Some Builder',
                        target='telemetry_perf_tests',
                        bucket='luci.bucket',
                        change=change_module.Change.FromDict({
                            'commits': [{
                                'repository': 'chromium',
                                'git_hash': 'aaaaaaa',
                            }]
                        })),
                    swarming_server='some_server',
                    dimensions=[],
                    extra_args=[],
                    attempts=10),
                benchmark=benchmark,
                histogram_options=read_value.HistogramOptions(
                    grouping_label=grouping_label,
                    story=story,
                    statistic=statistic,
                    histogram_name=chart,
                ),
                graph_json_options=read_value.GraphJsonOptions(
                    chart=chart, trace=trace),
                mode=mode,
            )))

  def testEvaluateSuccess_WithData(self, isolate_retrieve):
    # Seed the response to the call to the isolate service.
    histogram = histogram_module.Histogram('some_chart', 'count')
    histogram.AddSample(0)
    histogram.AddSample(1)
    histogram.AddSample(2)
    histograms = histogram_set.HistogramSet([histogram])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['story']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))

    # Set it up so that we are building a graph that's looking for no statistic.
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))

    # Ensure we find the find a value, and the histogram (?) associated with the
    # data we're looking for.
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace',
                },
                'status': 'completed',
                'result_values': [0, 1, 2],
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_HistogramStat(self, isolate_retrieve):
    histogram = histogram_module.Histogram('some_chart', 'count')
    histogram.AddSample(0)
    histogram.AddSample(1)
    histogram.AddSample(2)
    histograms = histogram_set.HistogramSet([histogram])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['story']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='story',
        statistic='avg')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'story',
                    'statistic': 'avg',
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace',
                },
                'result_values': [1.0],
                'status': 'completed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_HistogramStoryNeedsEscape(self, isolate_retrieve):
    histogram = histogram_module.Histogram('some_chart', 'count')
    histogram.AddSample(0)
    histogram.AddSample(1)
    histogram.AddSample(2)
    histograms = histogram_set.HistogramSet([histogram])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['https://story']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='https://story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'https://story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace',
                },
                'result_values': [0, 1, 2],
                'status': 'completed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_MultipleHistograms(self, isolate_retrieve):

    def CreateHistogram(name):
      histogram = histogram_module.Histogram(name, 'count')
      histogram.AddSample(0)
      histogram.AddSample(1)
      histogram.AddSample(2)
      return histogram

    histograms = histogram_set.HistogramSet([
        CreateHistogram(name)
        for name in ('some_chart', 'some_chart', 'some_other_chart')
    ])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['story']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace'
                },
                'result_values': [0, 1, 2, 0, 1, 2],
                'status': 'completed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_HistogramsTraceUrls(self, isolate_retrieve):
    hist = histogram_module.Histogram('some_chart', 'count')
    hist.AddSample(0)
    hist.diagnostics[reserved_infos.TRACE_URLS.name] = (
        generic_set.GenericSet(['trace_url1', 'trace_url2']))
    hist2 = histogram_module.Histogram('hist2', 'count')
    hist2.diagnostics[reserved_infos.TRACE_URLS.name] = (
        generic_set.GenericSet(['trace_url3']))
    hist3 = histogram_module.Histogram('hist3', 'count')
    hist3.diagnostics[reserved_infos.TRACE_URLS.name] = (
        generic_set.GenericSet(['trace_url2']))
    histograms = histogram_set.HistogramSet([hist, hist2, hist3])
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(benchmark='some_benchmark', chart='some_chart')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace'
                },
                'result_values': [0],
                'status': 'completed',
                'tries': 1,
                'trace_urls': [{
                    'key': 'trace',
                    'value': 'trace_url1',
                    'url': 'trace_url1'
                }, {
                    'key': 'trace',
                    'value': 'trace_url2',
                    'url': 'trace_url2',
                }, {
                    'key': 'trace',
                    'value': 'trace_url3',
                    'url': 'trace_url3',
                }],
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_HistogramSkipRefTraceUrls(self, isolate_retrieve):
    hist = histogram_module.Histogram('some_chart', 'count')
    hist.AddSample(0)
    hist.diagnostics[reserved_infos.TRACE_URLS.name] = (
        generic_set.GenericSet(['trace_url1', 'trace_url2']))
    hist2 = histogram_module.Histogram('hist2', 'count')
    hist2.diagnostics[reserved_infos.TRACE_URLS.name] = (
        generic_set.GenericSet(['trace_url3']))
    hist2.diagnostics[reserved_infos.TRACE_URLS.name].guid = 'foo'
    histograms = histogram_set.HistogramSet([hist, hist2])
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(benchmark='some_benchmark', chart='some_chart')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace'
                },
                'result_values': [0],
                'status': 'completed',
                'tries': 1,
                'trace_urls': [{
                    'key': 'trace',
                    'value': 'trace_url1',
                    'url': 'trace_url1'
                }, {
                    'key': 'trace',
                    'value': 'trace_url2',
                    'url': 'trace_url2',
                }],
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_HistogramSummary(self, isolate_retrieve):
    samples = []
    hists = []
    for i in range(10):
      hist = histogram_module.Histogram('some_chart', 'count')
      hist.AddSample(0)
      hist.AddSample(1)
      hist.AddSample(2)
      hist.diagnostics[reserved_infos.STORIES.name] = (
          generic_set.GenericSet(['story%d' % i]))
      hist.diagnostics[reserved_infos.STORY_TAGS.name] = (
          generic_set.GenericSet(['group:label1']))
      hists.append(hist)
      samples.extend(hist.sample_values)

    for i in range(10):
      hist = histogram_module.Histogram('some_chart', 'count')
      hist.AddSample(0)
      hist.AddSample(1)
      hist.AddSample(2)
      hist.diagnostics[reserved_infos.STORIES.name] = (
          generic_set.GenericSet(['another_story%d' % i]))
      hist.diagnostics[reserved_infos.STORY_TAGS.name] = (
          generic_set.GenericSet(['group:label2']))
      hists.append(hist)
      samples.extend(hist.sample_values)

    histograms = histogram_set.HistogramSet(hists)
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(benchmark='some_benchmark', chart='some_chart')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace'
                },
                'result_values': [sum(samples)],
                'status': 'completed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFailure_HistogramNoSamples(self, isolate_retrieve):
    histogram = histogram_module.Histogram('some_chart', 'count')
    histograms = histogram_set.HistogramSet([histogram])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['group:label']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['https://story']))
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps(histograms.AsDicts())], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='https://story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'https://story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace'
                },
                'status': 'failed',
                'errors': [{
                    'reason': 'ReadValueNoValues',
                    'message': mock.ANY,
                }],
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFailure_EmptyHistogramSet(self, isolate_retrieve):
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps([])], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='https://story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'https://story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace',
                },
                'status': 'failed',
                'errors': [{
                    'reason': 'ReadValueNotFound',
                    'message': mock.ANY,
                }],
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFailure_HistogramNoValues(self, isolate_retrieve):
    isolate_retrieve.side_effect = itertools.chain(*itertools.repeat(
        [('{"files": {"some_benchmark/perf_results.json": '
          '{"h": "394890891823812873798734a"}}}'),
         json.dumps(
             histogram_set.HistogramSet([
                 histogram_module.Histogram('some_benchmark', 'count')
             ]).AsDicts())], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='some_chart',
        grouping_label='label',
        story='https://story')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'histogram_sets',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': 'label',
                    'story': 'https://story',
                    'statistic': None,
                    'histogram_name': 'some_chart',
                },
                'graph_json_options': {
                    'chart': 'some_chart',
                    'trace': 'some_trace',
                },
                'status': 'failed',
                'errors': [{
                    'reason': 'ReadValueNotFound',
                    'message': mock.ANY,
                }],
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateSuccess_GraphJson(self, isolate_retrieve):
    isolate_retrieve.side_effect = itertools.chain(*itertools.repeat(
        [('{"files": {"some_benchmark/perf_results.json": '
          '{"h": "394890891823812873798734a"}}}'),
         json.dumps({'chart': {
             'traces': {
                 'trace': ['126444.869721', '0.0']
             }
         }})], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='chart',
        trace='trace',
        mode='graph_json')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'graph_json',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'chart',
                },
                'graph_json_options': {
                    'chart': 'chart',
                    'trace': 'trace',
                },
                'result_values': [126444.869721],
                'status': 'completed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFailure_GraphJsonMissingFile(self, isolate_retrieve):
    isolate_retrieve.return_value = '{"files": {}}'
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='chart',
        trace='trace',
        mode='graph_json')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'graph_json',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'chart',
                },
                'graph_json_options': {
                    'chart': 'chart',
                    'trace': 'trace',
                },
                'errors': [{
                    'reason': 'ReadValueNoFile',
                    'message': mock.ANY,
                }],
                'status': 'failed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFail_GraphJsonMissingChart(self, isolate_retrieve):
    isolate_retrieve.side_effect = itertools.chain(
        *itertools.repeat([('{"files": {"some_benchmark/perf_results.json": '
                            '{"h": "394890891823812873798734a"}}}'),
                           json.dumps({})], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='chart',
        trace='trace',
        mode='graph_json')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'graph_json',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'chart',
                },
                'graph_json_options': {
                    'chart': 'chart',
                    'trace': 'trace',
                },
                'errors': [{
                    'reason': 'ReadValueChartNotFound',
                    'message': mock.ANY,
                }],
                'status': 'failed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFail_GraphJsonMissingTrace(self, isolate_retrieve):
    isolate_retrieve.side_effect = itertools.chain(*itertools.repeat(
        [('{"files": {"some_benchmark/perf_results.json": '
          '{"h": "394890891823812873798734a"}}}'),
         json.dumps({'chart': {
             'traces': {
                 'trace': ['126444.869721', '0.0']
             }
         }})], 10))
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='chart',
        trace='must_not_be_found',
        mode='graph_json')
    self.assertNotEqual({},
                        task_module.Evaluate(
                            self.job,
                            event_module.Event(
                                type='initiate', target_task=None, payload={}),
                            self.evaluator))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'graph_json',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'chart',
                },
                'graph_json_options': {
                    'chart': 'chart',
                    'trace': 'must_not_be_found',
                },
                'errors': [{
                    'reason': 'ReadValueTraceNotFound',
                    'message': mock.ANY,
                }],
                'status': 'failed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))

  def testEvaluateFailedDependency(self, *_):
    self.PopulateTaskGraph(
        benchmark='some_benchmark',
        chart='chart',
        trace='must_not_be_found',
        mode='graph_json')
    self.assertNotEqual(
        {},
        task_module.Evaluate(
            self.job,
            event_module.Event(type='initiate', target_task=None, payload={}),
            evaluators.SequenceEvaluator(
                evaluators=(
                    evaluators.FilteringEvaluator(
                        predicate=evaluators.TaskTypeEq('find_isolate'),
                        delegate=evaluators.SequenceEvaluator(
                            evaluators=(
                                bisection_test_util.FakeFoundIsolate(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
                    evaluators.FilteringEvaluator(
                        predicate=evaluators.TaskTypeEq('run_test'),
                        delegate=evaluators.SequenceEvaluator(
                            evaluators=(
                                bisection_test_util.FakeFailedRunTest(self.job),
                                evaluators.TaskPayloadLiftingEvaluator()))),
                    read_value.Evaluator(self.job),
                ))))
    self.assertEqual(
        {
            'read_value_chromium@aaaaaaa_%s' % (attempt,): {
                'benchmark': 'some_benchmark',
                'change': mock.ANY,
                'mode': 'graph_json',
                'results_filename': 'some_benchmark/perf_results.json',
                'results_path': ['some_benchmark', 'perf_results.json'],
                'histogram_options': {
                    'grouping_label': None,
                    'story': None,
                    'statistic': None,
                    'histogram_name': 'chart',
                },
                'graph_json_options': {
                    'chart': 'chart',
                    'trace': 'must_not_be_found',
                },
                'errors': [{
                    'reason': 'DependencyFailed',
                    'message': mock.ANY,
                }],
                'status': 'failed',
                'tries': 1,
                'index': attempt,
            } for attempt in range(10)
        },
        task_module.Evaluate(
            self.job,
            event_module.Event(type='select', target_task=None, payload={}),
            evaluators.Selector(task_type='read_value')))
