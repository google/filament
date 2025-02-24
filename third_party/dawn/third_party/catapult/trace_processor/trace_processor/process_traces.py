# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import argparse
import json
import os
import subprocess
import sys

from tracing.mre import function_handle
from tracing.mre import map_runner
from tracing.mre import progress_reporter
from tracing.mre import file_handle
from tracing.mre import job as job_module
from tracing.metrics import discover
from tracing.metrics import metric_runner


def _GetListFromFileOrDir(trace_file_or_dir):
  if os.path.isdir(trace_file_or_dir):
    traces = [os.path.join(trace_file_or_dir, trace)
        for trace in os.listdir(trace_file_or_dir)]
  else:
    traces = [trace_file_or_dir]
  return traces


def _DumpToOutputJson(results, output_file):
  if output_file:
    with open(output_file, 'w') as f:
      json.dump(results, f, indent=2)
  else:
    print json.dumps(results, indent=2)


def _GetExitCodeForResults(results):
  if not any(result.failures for result in results.values()):
    return 0
  else:
    return 255


def _ProcessTracesWithMetric(metric_name, traces, output_file):
  results = metric_runner.RunMetricOnTraces(traces, [metric_name])
  results_dict = {k: v.AsDict() for k, v in results.iteritems()}
  _DumpToOutputJson(results_dict, output_file)

  return _GetExitCodeForResults(results)


def _ProcessTracesWithMapper(mapper_handle_str, traces, output_file):
  map_handle = function_handle.FunctionHandle.FromUserFriendlyString(
      mapper_handle_str)
  job = job_module.Job(map_handle)

  trace_handles = [
      file_handle.URLFileHandle(f, 'file://%s' % f) for f in traces]

  runner = map_runner.MapRunner(
      trace_handles, job,
      progress_reporter=progress_reporter.ProgressReporter())

  results = runner.RunMapper()
  results_dict = {k: v.AsDict() for k, v in results.iteritems()}
  _DumpToOutputJson(results_dict, output_file)

  return _GetExitCodeForResults(results)


def Main():
  all_metrics = discover.DiscoverMetrics(['/tracing/metrics/all_metrics.html'])

  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--metric_name',
      help='Metric name, valid choices are: %s' % ', '.join(all_metrics))
  parser.add_argument(
      '--mapper_handle',
      help='Mapper handle, in the format path/to/handle.html:handleFunction')
  parser.add_argument(
      'trace_file_or_dir',
      help='Path to trace file or directory of trace files.')
  parser.add_argument(
      '--output_file',
      help='Path to output file to store results.')
  args = parser.parse_args()

  if args.metric_name and args.mapper_handle:
    parser.error('Specify either metric or mapper handle, not both.')

  if not args.metric_name and not args.mapper_handle:
    parser.error('Specify either metric or mapper handle.')

  traces = _GetListFromFileOrDir(os.path.abspath(args.trace_file_or_dir))

  if args.output_file:
    args.output_file = os.path.abspath(args.output_file)

  if args.metric_name:
    # Didn't put in choices because the commandline help is super ugly and
    # repetitive.
    if not args.metric_name in all_metrics:
      parser.error('Invalid metric specified.')

    return _ProcessTracesWithMetric(
        args.metric_name, traces, args.output_file)
  elif args.mapper_handle:
    return _ProcessTracesWithMapper(
        args.mapper_handle, traces, args.output_file)
