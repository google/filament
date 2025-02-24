# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
import collections
import itertools
import json
import logging
import ntpath
import posixpath
import sys

from dashboard.common import histogram_helpers
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models.quest import execution
from dashboard.pinpoint.models.quest import quest
from dashboard.services import cas_service
from dashboard.services import isolate
from tracing.value import histogram_set
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import reserved_infos
import six


class ReadValue(quest.Quest):

  def __init__(self,
               results_filename,
               results_path,
               metric=None,
               grouping_label=None,
               trace_or_story=None,
               statistic=None,
               chart=None):
    self._results_filename = results_filename
    self._results_path = results_path
    self._metric = metric
    self._grouping_label = grouping_label
    self._trace_or_story = trace_or_story
    self._statistic = statistic
    self._chart = chart

  def __eq__(self, other):
    return (isinstance(other, type(self))
            and self._results_filename == other._results_filename
            and self.results_path == other.results_path
            and self._metric == other._metric
            and self._grouping_label == other._grouping_label
            and self._trace_or_story == other._trace_or_story
            and self._statistic == other._statistic
            and self._chart == other._chart)

  def __hash__(self):
    return hash(self.__str__())

  def __str__(self):
    return 'Get values'

  @property
  def metric(self):
    return self._chart or self._metric

  @property
  def results_path(self):
    return getattr(self, '_results_path', None)

  def Start(self, change, isolate_server=None, isolate_hash=None,
            cas_root_ref=None):
    # Here we create an execution that can handle both histograms and graph
    # json and any other format we support later.
    # TODO(dberris): It seems change is a required input, need to preserve this
    # for forward/backward compatibility.
    del change
    return ReadValueExecution(self._results_filename, self.results_path,
                              self._metric, self._grouping_label,
                              self._trace_or_story, self._statistic,
                              self._chart, isolate_server, isolate_hash,
                              cas_root_ref)

  @classmethod
  def FromDict(cls, arguments):
    benchmark = arguments.get('benchmark')
    if not benchmark:
      raise TypeError('Missing "benchmark" argument.')
    results_filename = ''
    if IsWindows(arguments):
      results_filename = ntpath.join(benchmark, 'perf_results.json')
    else:
      results_filename = posixpath.join(benchmark, 'perf_results.json')
    results_path = [benchmark, 'perf_results.json']

    metric = (arguments.get('metric') or arguments.get('chart'))
    chart = arguments.get('chart')
    grouping_label = arguments.get('grouping_label')
    trace_or_story = (arguments.get('trace') or arguments.get('story'))
    statistic = arguments.get('statistic')
    return cls(
        results_filename=results_filename,
        results_path=results_path,
        metric=metric,
        grouping_label=grouping_label,
        trace_or_story=trace_or_story,
        statistic=statistic,
        chart=chart)


class ReadValueExecution(execution.Execution):

  def __init__(self, results_filename, results_path, metric, grouping_label,
               trace_or_story, statistic, chart, isolate_server, isolate_hash,
               cas_root_ref=None):
    super().__init__()
    self._results_filename = results_filename
    self._results_path = results_path
    self._metric = metric
    self._grouping_label = grouping_label
    self._trace_or_story = trace_or_story
    self._statistic = statistic
    self._chart = chart
    self._isolate_server = isolate_server
    self._isolate_hash = isolate_hash
    self._cas_root_ref = cas_root_ref
    self._trace_urls = []
    self._mode = None

  @property
  def mode(self):
    return getattr(self, '_mode', None)

  @property
  def cas_root_ref(self):
    return getattr(self, '_cas_root_ref', None)

  @property
  def results_path(self):
    return getattr(self, '_results_path', None)

  def _AsDict(self):
    return [{
        'key': 'trace',
        'value': trace_url['name'],
        'url': trace_url['url'],
    } for trace_url in self._trace_urls]

  def _Poll(self):
    if self.cas_root_ref:
      isolate_output = RetrieveCASOutput(
          self.cas_root_ref,
          self.results_path,
      )
    else:
      isolate_output = RetrieveIsolateOutput(
          self._isolate_server,
          self._isolate_hash,
          self._results_filename,
      )

    result_values = []
    histogram_exception = None
    graph_json_exception = None
    json_data = None
    proto_data = None
    try:
      json_data = json.loads(isolate_output)
    except ValueError:
      proto_data = isolate_output

    try:
      logging.debug('Attempting to parse as a HistogramSet.')
      result_values = self._ParseHistograms(json_data, proto_data)
      self._mode = 'histograms'
      logging.debug('Succeess.')
    except (errors.ReadValueNotFound, errors.ReadValueNoValues,
            errors.FatalError):
      raise
    except (errors.InformationalError, errors.ReadValueUnknownFormat) as e:
      # In case we encounter any other error, we should log and proceed.
      logging.error('Failed parsing histograms: %s', e)
      histogram_exception = sys.exc_info()[1]

    if histogram_exception:
      try:
        logging.debug('Attempting to parse as GraphJSON.')
        result_values = self._ParseGraphJson(json_data)
        self._mode = 'graphjson'
        logging.debug('Succeess.')
      # We're catching a subset of InformationalError here. The latest version
      # of pylint will handle this use case.
      # pylint: disable=try-except-raise
      except (errors.ReadValueChartNotFound, errors.ReadValueTraceNotFound,
              errors.FatalError):
        raise
      except errors.InformationalError as e:
        logging.error('Failed parsing histograms: %s', e)
        graph_json_exception = sys.exc_info()[1]

    if histogram_exception and graph_json_exception:
      raise errors.ReadValueUnknownFormat(self._results_filename)

    self._Complete(result_values=tuple(result_values))

  def _ParseHistograms(self, json_data, proto_data):
    histograms = histogram_set.HistogramSet()
    try:
      if json_data is not None:
        histograms.ImportDicts(json_data)
      elif proto_data is not None:
        histograms.ImportProto(proto_data)
    except BaseException as e:
      raise errors.ReadValueUnknownFormat(self._results_filename) from e

    self._trace_urls = FindTraceUrls(histograms)
    histograms_by_path = CreateHistogramSetByTestPathDict(histograms)
    histograms_by_path_optional_grouping_label = (
        CreateHistogramSetByTestPathDict(
            histograms, ignore_grouping_label=True))
    test_paths_to_match = {
        histogram_helpers.ComputeTestPathFromComponents(
            self._metric,
            grouping_label=self._grouping_label,
            story_name=self._trace_or_story),
        histogram_helpers.ComputeTestPathFromComponents(
            self._metric,
            grouping_label=self._grouping_label,
            story_name=self._trace_or_story,
            needs_escape=False)
    }
    logging.debug('Test paths to match: %s', test_paths_to_match)

    try:
      result_values = ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path, self._metric,
          self._grouping_label, self._trace_or_story, self._statistic)
    except errors.ReadValueNotFound:
      result_values = ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path_optional_grouping_label,
          self._metric, None, self._trace_or_story, self._statistic)
    return result_values

  def _ParseGraphJson(self, json_data):
    if not self._chart and not self._trace_or_story:
      return []
    if self._chart not in json_data:
      raise errors.ReadValueChartNotFound(self._chart)
    if self._trace_or_story not in json_data[self._chart]['traces']:
      raise errors.ReadValueTraceNotFound(self._trace_or_story)
    return [float(json_data[self._chart]['traces'][self._trace_or_story][0])]


def CreateHistogramSetByTestPathDict(histograms, ignore_grouping_label=False):
  histograms_by_path = collections.defaultdict(list)
  for h in histograms:
    histograms_by_path[histogram_helpers.ComputeTestPath(
        h, ignore_grouping_label)].append(h)
  return histograms_by_path


def FindTraceUrls(histograms):
  # Get and cache any trace URLs.
  unique_trace_urls = set()
  trace_names = {}
  for hist in histograms:
    trace_urls = hist.diagnostics.get(reserved_infos.TRACE_URLS.name)
    # TODO(simonhatch): Remove this sometime after May 2018. We had a
    # brief period where the histograms generated by tests had invalid
    # trace_urls diagnostics. If the diagnostic we get back is just a ref,
    # then skip.
    # https://github.com/catapult-project/catapult/issues/4243
    if trace_urls and not isinstance(trace_urls, diagnostic_ref.DiagnosticRef):
      unique_trace_urls.update(trace_urls)
      for url in trace_urls:
        trace_name = hist.diagnostics.get(reserved_infos.STORIES.name)
        if not trace_name:
          trace_name = url.split('/')[-1]
        else:
          trace_name = trace_name.GetOnlyElement()
        trace_names[url] = trace_name

  sorted_urls = sorted(unique_trace_urls)

  return [{'name': trace_names[t], 'url': t} for t in sorted_urls]


def _GetValuesOrStatistic(statistic, hist):
  if not statistic:
    return hist.sample_values

  if not hist.sample_values:
    return []

  # TODO(simonhatch): Use Histogram.getStatisticScalar when it's ported from
  # js.
  if statistic == 'avg':
    return [hist.running.mean]
  if statistic == 'min':
    return [hist.running.min]
  if statistic == 'max':
    return [hist.running.max]
  if statistic == 'sum':
    return [hist.running.sum]
  if statistic == 'std':
    return [hist.running.stddev]
  if statistic == 'count':
    return [hist.running.count]
  raise errors.ReadValueUnknownStat(statistic)


def IsWindows(arguments):
  dimensions = arguments.get('dimensions', ())
  if isinstance(dimensions, six.string_types):
    dimensions = json.loads(dimensions)
  for dimension in dimensions:
    if dimension['key'] == 'os' and dimension['value'].startswith('Win'):
      return True
  return False


def RetrieveIsolateOutput(isolate_server, isolate_hash, filename):
  logging.debug('Retrieving output (%s, %s, %s)', isolate_server, isolate_hash,
                filename)

  retrieve_result = isolate.Retrieve(isolate_server, isolate_hash)
  response = json.loads(retrieve_result)
  output_files = response.get('files', {})

  if filename not in output_files:
    if 'performance_browser_tests' not in filename:
      raise errors.ReadValueNoFile(filename)

    # TODO(simonhatch): Remove this once crbug.com/947501 is resolved.
    filename = filename.replace('performance_browser_tests', 'browser_tests')
    if filename not in output_files:
      raise errors.ReadValueNoFile(filename)

  output_isolate_hash = output_files[filename]['h']
  logging.debug('Retrieving %s', output_isolate_hash)

  return isolate.Retrieve(isolate_server, output_isolate_hash)


def RetrieveCASOutput(cas_root_ref, path):
  logging.debug('Retrieving output (%s, %s)', cas_root_ref, path)

  cas_client = cas_service.GetRBECASService()

  def _GetTree(cas_ref):
    return cas_client.GetTree(cas_ref)[0]['directories'][0]

  def _GetNodeByName(name, nodes):
    for node in nodes:
      if node['name'] == name:
        return node
    raise errors.ReadValueNoFile(path)

  tree = _GetTree(cas_root_ref)
  for name in path[:-1]:
    node = _GetNodeByName(name, tree['directories'])
    tree = _GetTree({
        'casInstance': cas_root_ref['casInstance'],
        'digest': node['digest'],
    })

  node = _GetNodeByName(path[-1], tree['files'])
  response = cas_client.BatchRead(cas_root_ref['casInstance'], [node['digest']])
  data = response['responses'][0].get('data', '')
  return base64.b64decode(data)


def RetrieveOutputJsonFromCAS(cas_root_ref, path):
  output = RetrieveCASOutput(cas_root_ref, path)
  return json.loads(output)


def RetrieveOutputJson(isolate_server, isolate_hash, filename):
  isolate_output = RetrieveIsolateOutput(isolate_server, isolate_hash, filename)

  # TODO(dberris): Use incremental json parsing through a file interface, to
  # avoid having to load the whole string contents in memory. See
  # https://crbug.com/998517 for more context.
  return json.loads(isolate_output)


def ExtractValuesFromHistograms(test_paths_to_match, histograms_by_path,
                                histogram_name, grouping_label, story,
                                statistic):
  result_values = []
  matching_histograms = list(
      itertools.chain.from_iterable(
          histograms_by_path.get(histogram)
          for histogram in test_paths_to_match
          if histogram in histograms_by_path))
  logging.debug('Histograms in results: %s', list(histograms_by_path.keys()))
  if matching_histograms:
    logging.debug('Found %s matching histograms: %s', len(matching_histograms),
                  [h.name for h in matching_histograms])
    for h in matching_histograms:
      result_values.extend(_GetValuesOrStatistic(statistic, h))
  elif histogram_name:
    # Histograms don't exist, which means this is summary
    summary_value = []
    for test_path, histograms_for_test_path in histograms_by_path.items():
      for test_path_to_match in test_paths_to_match:
        if test_path.startswith(test_path_to_match):
          for h in histograms_for_test_path:
            summary_value.extend(_GetValuesOrStatistic(statistic, h))
            matching_histograms.append(h)

    logging.debug('Found %s matching summary histograms',
                  len(matching_histograms))
    if summary_value:
      result_values.append(sum(summary_value))

    logging.debug('result values: %s', result_values)

  if not result_values and histogram_name:
    if matching_histograms:
      raise errors.ReadValueNoValues()

    conditions = {'histogram': histogram_name}
    if grouping_label:
      conditions['grouping_label'] = grouping_label
    if story:
      conditions['story'] = story
    reason = ', '.join(list(':'.join(i) for i in conditions.items()))
    raise errors.ReadValueNotFound(reason)
  return result_values


# WARNING: EVERYTHING AFTER THIS LINE IS DEPRECATED
################################################################################
# These are legacy quests and executions which are still in the pickled state of
# JobState objects in Datastore. We're leaving these around for a time until we
# can determine that it's safe to remove these types.
#
class ReadHistogramsJsonValue(quest.Quest):

  def __init__(self,
               results_filename,
               hist_name=None,
               grouping_label=None,
               trace_or_story=None,
               statistic=None):
    self._results_filename = results_filename
    self._hist_name = hist_name
    self._grouping_label = grouping_label
    self._trace_or_story = trace_or_story
    self._statistic = statistic

  def __eq__(self, other):
    return (isinstance(other, type(self))
            and self._results_filename == other._results_filename
            and self._hist_name == other._hist_name
            and self._grouping_label == other._grouping_label
            and self.trace_or_story == other.trace_or_story
            and self._statistic == other._statistic)

  def __hash__(self):
    return hash(self.__str__())

  def __str__(self):
    return 'Get results'

  @property
  def metric(self):
    return self._hist_name

  @property
  def trace_or_story(self):
    if getattr(self, '_trace_or_story', None) is None:
      self._trace_or_story = getattr(self, '_trace', None)
    return self._trace_or_story

  def Start(self, change, isolate_server, isolate_hash):
    del change

    return _ReadHistogramsJsonValueExecution(
        self._results_filename, self._hist_name, self._grouping_label,
        self.trace_or_story, self._statistic, isolate_server, isolate_hash)

  @classmethod
  def FromDict(cls, arguments):
    benchmark = arguments.get('benchmark')
    if not benchmark:
      raise TypeError('Missing "benchmark" argument.')
    if IsWindows(arguments):
      results_filename = ntpath.join(benchmark, 'perf_results.json')
    else:
      results_filename = posixpath.join(benchmark, 'perf_results.json')

    chart = arguments.get('chart')
    grouping_label = arguments.get('grouping_label')

    # Some benchmarks do not have a 'trace' associated with them, but do have a
    # 'story' which the Dashboard can sometimes provide. Let's support getting
    # the story in case the 'trace' is not provided. See crbug.com/1023408 for
    # more details in the investigation.
    trace_or_story = (arguments.get('trace') or arguments.get('story'))
    statistic = arguments.get('statistic')

    return cls(results_filename, chart, grouping_label, trace_or_story,
               statistic)


class _ReadHistogramsJsonValueExecution(execution.Execution):

  def __init__(self, results_filename, hist_name, grouping_label,
               trace_or_story, statistic, isolate_server, isolate_hash):
    super().__init__()
    self._results_filename = results_filename
    self._hist_name = hist_name
    self._grouping_label = grouping_label
    self._trace_or_story = trace_or_story
    self._statistic = statistic
    self._isolate_server = isolate_server
    self._isolate_hash = isolate_hash

    self._trace_urls = []

  @property
  def trace_or_story(self):
    if getattr(self, '_trace_or_story', None) is None:
      self._trace_or_story = getattr(self, '_trace', '')
    return self._trace_or_story

  def _AsDict(self):
    return [{
        'key': 'trace',
        'value': trace_url['name'],
        'url': trace_url['url'],
    } for trace_url in self._trace_urls]

  def _Poll(self):
    histogram_dicts = RetrieveOutputJson(self._isolate_server,
                                         self._isolate_hash,
                                         self._results_filename)
    histograms = histogram_set.HistogramSet()
    histograms.ImportDicts(histogram_dicts)

    histograms_by_path = CreateHistogramSetByTestPathDict(histograms)
    histograms_by_path_optional_grouping_label = (
        CreateHistogramSetByTestPathDict(
            histograms, ignore_grouping_label=True))
    self._trace_urls = FindTraceUrls(histograms)

    test_paths_to_match = {
        histogram_helpers.ComputeTestPathFromComponents(
            self._hist_name,
            grouping_label=self._grouping_label,
            story_name=self._trace_or_story),
        histogram_helpers.ComputeTestPathFromComponents(
            self._hist_name,
            grouping_label=self._grouping_label,
            story_name=self._trace_or_story,
            needs_escape=False)
    }
    logging.debug('Test paths to match: %s', test_paths_to_match)

    # Have to pull out either the raw sample values, or the statistic
    try:
      result_values = ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path, self._hist_name,
          self._grouping_label, self._trace_or_story, self._statistic)
    except errors.ReadValueNotFound:
      # In case we didn't find any result_values, we should try finding the
      # histograms without the grouping label applied.
      result_values = ExtractValuesFromHistograms(
          test_paths_to_match, histograms_by_path_optional_grouping_label,
          self._hist_name, None, self._trace_or_story, self._statistic)

    self._Complete(result_values=tuple(result_values))


class ReadGraphJsonValue(quest.Quest):

  def __init__(self, results_filename, chart, trace):
    self._results_filename = results_filename
    self._chart = chart
    self._trace = trace

  def __eq__(self, other):
    return (isinstance(other, type(self))
            and self._results_filename == other._results_filename
            and self._chart == other._chart and self._trace == other._trace)

  def __hash__(self):
    return hash(self.__str__())

  def __str__(self):
    return 'Get results'

  @property
  def metric(self):
    return self._chart

  def Start(self, change, isolate_server, isolate_hash):
    del change

    return _ReadGraphJsonValueExecution(self._results_filename, self._chart,
                                        self._trace, isolate_server,
                                        isolate_hash)

  @classmethod
  def FromDict(cls, arguments):
    benchmark = arguments.get('benchmark')
    if not benchmark:
      raise TypeError('Missing "benchmark" argument.')
    if IsWindows(arguments):
      results_filename = ntpath.join(benchmark, 'perf_results.json')
    else:
      results_filename = posixpath.join(benchmark, 'perf_results.json')

    chart = arguments.get('chart')
    trace = arguments.get('trace')

    return cls(results_filename, chart, trace)


class _ReadGraphJsonValueExecution(execution.Execution):

  def __init__(self, results_filename, chart, trace, isolate_server,
               isolate_hash):
    super().__init__()
    self._results_filename = results_filename
    self._chart = chart
    self._trace = trace
    self._isolate_server = isolate_server
    self._isolate_hash = isolate_hash

  def _AsDict(self):
    return {'isolate_server': self._isolate_server}

  def _Poll(self):
    graphjson = RetrieveOutputJson(self._isolate_server, self._isolate_hash,
                                   self._results_filename)

    if not self._chart and not self._trace:
      self._Complete(result_values=tuple([]))
      return

    if self._chart not in graphjson:
      raise errors.ReadValueChartNotFound(self._chart)
    if self._trace not in graphjson[self._chart]['traces']:
      raise errors.ReadValueTraceNotFound(self._trace)
    result_value = float(graphjson[self._chart]['traces'][self._trace][0])

    self._Complete(result_values=(result_value,))
