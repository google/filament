# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import sys

import six
from tracing.mre import file_handle
from tracing.mre import function_handle
from tracing.mre import gtest_progress_reporter
from tracing.mre import job as job_module
from tracing.mre import map_runner
from tracing.mre import map_single_trace


try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


_METRIC_MAP_FUNCTION_FILENAME = 'metric_map_function.html'

_METRIC_MAP_FUNCTION_NAME = 'metricMapFunction'

def _GetMetricsDir():
  return os.path.dirname(os.path.abspath(__file__))

def _GetMetricRunnerHandle(metrics):
  assert isinstance(metrics, list)
  for metric in metrics:
    assert isinstance(metric, StringTypes)
  metrics_dir = _GetMetricsDir()
  metric_mapper_path = os.path.join(metrics_dir, _METRIC_MAP_FUNCTION_FILENAME)

  modules_to_load = [function_handle.ModuleToLoad(filename=metric_mapper_path)]
  options = {'metrics': metrics}
  map_function_handle = function_handle.FunctionHandle(
      modules_to_load, _METRIC_MAP_FUNCTION_NAME, options)

  return job_module.Job(map_function_handle, None)

def RunMetric(filename, metrics, extra_import_options=None,
              report_progress=True, canonical_url=None):
  filename_url = 'file://' + filename
  if canonical_url is None:
    canonical_url = filename_url
  trace_handle = file_handle.URLFileHandle(canonical_url, filename_url)
  result = RunMetricOnTraceHandles(
      [trace_handle], metrics, extra_import_options, report_progress)
  return result[canonical_url]

def RunMetricOnSingleTrace(filename, metrics, extra_import_options=None,
                           canonical_url=None, timeout=None):
  """A simplified RunMetric() that skips using MapRunner.

  This avoids the complexity of multithreading and progress
  reporting.
  """
  filename_url = 'file://' + filename
  if canonical_url is None:
    canonical_url = filename_url
  trace_handle = file_handle.URLFileHandle(canonical_url, filename_url)
  job = _GetMetricRunnerHandle(metrics)
  return map_single_trace.MapSingleTrace(
      trace_handle, job, extra_import_options=extra_import_options,
      timeout=timeout)

def RunMetricOnTraceHandles(trace_handles, metrics, extra_import_options=None,
                            report_progress=True):
  job = _GetMetricRunnerHandle(metrics)
  with open(os.devnull, 'w') as devnull_f:
    o_stream = sys.stdout
    if not report_progress:
      o_stream = devnull_f

    runner = map_runner.MapRunner(
        trace_handles, job, extra_import_options=extra_import_options,
        progress_reporter=gtest_progress_reporter.GTestProgressReporter(
            output_stream=o_stream))
    map_results = runner.RunMapper()

  return map_results

def RunMetricOnTraces(filenames, metrics,
                      extra_import_options=None, report_progress=True):
  trace_handles = []
  for filename in filenames:
    filename_url = 'file://' + filename
    trace_handles.append(file_handle.URLFileHandle(filename_url, filename_url))

  return RunMetricOnTraceHandles(trace_handles, metrics, extra_import_options,
                                 report_progress)
