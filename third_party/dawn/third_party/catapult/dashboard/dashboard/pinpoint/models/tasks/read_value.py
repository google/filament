# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging
import ntpath
import posixpath

from dashboard.common import histogram_helpers
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.quest import read_value as read_value_quest
from dashboard.pinpoint.models.tasks import find_isolate
from dashboard.pinpoint.models.tasks import run_test
from tracing.value import histogram_set

HistogramOptions = collections.namedtuple(
    'HistogramOptions',
    ('grouping_label', 'story', 'statistic', 'histogram_name'))

GraphJsonOptions = collections.namedtuple('GraphJsonOptions',
                                          ('chart', 'trace'))

TaskOptions = collections.namedtuple(
    'TaskOptions', ('test_options', 'benchmark', 'histogram_options',
                    'graph_json_options', 'mode'))


class CompleteReadValueAction(
    collections.namedtuple('CompleteReadValueAction',
                           ('job', 'task', 'state'))):
  __slots__ = ()

  def __str__(self):
    return 'CompleteReadValueAction(job = %s, task = %s)' % (self.job.job_id,
                                                             self.task.id)

  @task_module.LogStateTransitionFailures
  def __call__(self, _):
    task_module.UpdateTask(
        self.job, self.task.id, new_state=self.state, payload=self.task.payload)


class ReadValueEvaluator(
    collections.namedtuple('ReadValueEvaluator', ('job',))):
  __slots__ = ()

  def CompleteWithError(self, task, reason, message):
    task.payload.update({
        'tries':
            task.payload.get('tries', 0) + 1,
        'errors':
            task.payload.get('errors', []) + [{
                'reason': reason,
                'message': message
            }]
    })
    return [CompleteReadValueAction(self.job, task, 'failed')]

  def __call__(self, task, _, accumulator):
    # TODO(dberris): Validate!
    # Outline:
    #   - Retrieve the data given the options.
    #   - Parse the data from the result file.
    #   - Update the status and payload with an action.

    if task.status in {'completed', 'failed'}:
      return None
    dep = accumulator.get(task.dependencies[0], {})
    isolate_server = dep.get('isolate_server')
    isolate_hash = dep.get('isolate_hash')
    cas_root_ref = dep.get('cas_root_ref')
    dependency_status = dep.get('status', 'failed')
    if dependency_status == 'failed':
      return self.CompleteWithError(
          task, 'DependencyFailed',
          'Task dependency "%s" ended in failed status.' %
          (task.dependencies[0],))

    if dependency_status in {'pending', 'ongoing'}:
      return None

    try:
      if cas_root_ref:
        data = read_value_quest.RetrieveOutputJsonFromCAS(
            cas_root_ref, task.payload.get('results_path'))
      else:
        data = read_value_quest.RetrieveOutputJson(
            isolate_server, isolate_hash, task.payload.get('results_filename'))
      if task.payload.get('mode') == 'histogram_sets':
        return self.HandleHistogramSets(task, data)
      if task.payload.get('mode') == 'graph_json':
        return self.HandleGraphJson(task, data)
      return self.CompleteWithError(
          task, 'UnsupportedMode',
          ('Pinpoint only currently supports reading '
           'HistogramSets and GraphJSON formatted files.'))
    except (errors.FatalError, errors.InformationalError,
            errors.RecoverableError) as e:
      return self.CompleteWithError(task, type(e).__name__, str(e))

  def HandleHistogramSets(self, task, histogram_dicts):
    histogram_options = task.payload.get('histogram_options', {})
    grouping_label = histogram_options.get('grouping_label', '')
    histogram_name = histogram_options.get('histogram_name')
    story = histogram_options.get('story')
    statistic = histogram_options.get('statistic')
    histograms = histogram_set.HistogramSet()
    histograms.ImportDicts(histogram_dicts)
    histograms_by_path = read_value_quest.CreateHistogramSetByTestPathDict(
        histograms)
    histograms_by_path_optional_grouping_label = (
        read_value_quest.CreateHistogramSetByTestPathDict(
            histograms, ignore_grouping_label=True))
    trace_urls = read_value_quest.FindTraceUrls(histograms)
    test_paths_to_match = {
        histogram_helpers.ComputeTestPathFromComponents(
            histogram_name, grouping_label=grouping_label, story_name=story),
        histogram_helpers.ComputeTestPathFromComponents(
            histogram_name,
            grouping_label=grouping_label,
            story_name=story,
            needs_escape=False)
    }
    logging.debug('Test paths to match: %s', test_paths_to_match)
    try:
      result_values = read_value_quest.ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path, histogram_name,
          grouping_label, story, statistic)
    except errors.ReadValueNotFound:
      result_values = read_value_quest.ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path_optional_grouping_label,
          histogram_name, None, story, statistic)
    logging.debug('Results: %s', result_values)
    task.payload.update({
        'result_values': result_values,
        'tries': 1,
    })
    if trace_urls:
      task.payload['trace_urls'] = [{
          'key': 'trace',
          'value': url['name'],
          'url': url['url'],
      } for url in trace_urls]
    return [CompleteReadValueAction(self.job, task, 'completed')]

  def HandleGraphJson(self, task, data):
    chart = task.payload.get('graph_json_options', {}).get('chart', '')
    trace = task.payload.get('graph_json_options', {}).get('trace', '')
    if not chart and not trace:
      task.payload.update({
          'result_values': [],
          'tries': task.payload.get('tries', 0) + 1
      })
      return [CompleteReadValueAction(self.job, task, 'completed')]

    if chart not in data:
      raise errors.ReadValueChartNotFound(chart)
    if trace not in data[chart]['traces']:
      raise errors.ReadValueTraceNotFound(trace)
    task.payload.update({
        'result_values': [float(data[chart]['traces'][trace][0])],
        'tries': task.payload.get('tries', 0) + 1
    })
    return [CompleteReadValueAction(self.job, task, 'completed')]


class Evaluator(evaluators.FilteringEvaluator):

  def __init__(self, job):
    super().__init__(
        predicate=evaluators.All(
            evaluators.TaskTypeEq('read_value'),
            evaluators.TaskStatusIn({'pending'})),
        delegate=evaluators.SequenceEvaluator(
            evaluators=(evaluators.TaskPayloadLiftingEvaluator(),
                        ReadValueEvaluator(job))))


def ResultSerializer(task, _, accumulator):
  results = accumulator.setdefault(task.id, {})
  results.update({
      'completed':
          task.status in {'completed', 'failed', 'cancelled'},
      'exception':
          ','.join(e.get('reason') for e in task.payload.get('errors', [])) or
          None,
      'details': []
  })

  trace_urls = task.payload.get('trace_urls')
  if trace_urls:
    results['details'].extend(trace_urls)


class Serializer(evaluators.FilteringEvaluator):

  def __init__(self):
    super().__init__(
        predicate=evaluators.All(
            evaluators.TaskTypeEq('read_value'),
            evaluators.TaskStatusIn(
                {'ongoing', 'failed', 'completed', 'cancelled'}),
        ),
        delegate=ResultSerializer)


def CreateGraph(options):
  if not isinstance(options, TaskOptions):
    raise ValueError('options must be an instance of read_value.TaskOptions')
  subgraph = run_test.CreateGraph(options.test_options)
  path = None
  if read_value_quest.IsWindows({'dimensions': options.test_options.dimensions
                                }):
    path = ntpath.join(options.benchmark, 'perf_results.json')
  else:
    path = posixpath.join(options.benchmark, 'perf_results.json')
  results_path = [options.benchmark, 'perf_results.json']

  # We create a 1:1 mapping between a read_value task and a run_test task.
  def GenerateVertexAndDep(attempts):
    for attempt in range(attempts):
      change_id = find_isolate.ChangeId(
          options.test_options.build_options.change)
      read_value_id = 'read_value_%s_%s' % (change_id, attempt)
      run_test_id = run_test.TaskId(change_id, attempt)
      yield (task_module.TaskVertex(
          id=read_value_id,
          vertex_type='read_value',
          payload={
              'benchmark': options.benchmark,
              'mode': options.mode,
              # TODO(fancl): remove results_filename because retrieving file in
              # RBE-CAS is platform independent.
              'results_filename': path,
              'results_path': results_path,
              'histogram_options': options.histogram_options._asdict(),
              'graph_json_options': options.graph_json_options._asdict(),
              'change': options.test_options.build_options.change.AsDict(),
              'index': attempt,
          }), task_module.Dependency(from_=read_value_id, to=run_test_id))

  for vertex, edge in GenerateVertexAndDep(options.test_options.attempts):
    subgraph.vertices.append(vertex)
    subgraph.edges.append(edge)

  return subgraph
