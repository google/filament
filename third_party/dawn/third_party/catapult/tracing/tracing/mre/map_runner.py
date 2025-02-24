# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import multiprocessing
import sys

from tracing.mre import gtest_progress_reporter
from tracing.mre import map_single_trace
from tracing.mre import threaded_work_queue

AUTO_JOB_COUNT = -1


class MapError(Exception):

  def __init__(self, *args):
    super(MapError, self).__init__(*args)
    self.canonical_url = None


class MapRunner(object):
  def __init__(self, trace_handles, job,
               stop_on_error=False, progress_reporter=None,
               jobs=AUTO_JOB_COUNT,
               output_formatters=None,
               extra_import_options=None):
    self._job = job
    self._stop_on_error = stop_on_error
    self._failed_canonical_url_to_dump = None
    if progress_reporter is None:
      self._progress_reporter = gtest_progress_reporter.GTestProgressReporter(
          sys.stdout)
    else:
      self._progress_reporter = progress_reporter
    self._output_formatters = output_formatters or []
    self._extra_import_options = extra_import_options

    self._trace_handles = trace_handles
    self._num_traces_merged_into_results = 0
    self._map_results = None
    self._map_results_file = None

    if jobs == AUTO_JOB_COUNT:
      jobs = min(len(self._trace_handles), multiprocessing.cpu_count())
    self._wq = threaded_work_queue.ThreadedWorkQueue(num_threads=jobs)

  def _ProcessOneTrace(self, trace_handle):
    canonical_url = trace_handle.canonical_url
    run_reporter = self._progress_reporter.WillRun(canonical_url)
    result = map_single_trace.MapSingleTrace(
        trace_handle,
        self._job,
        extra_import_options=self._extra_import_options)

    had_failure = len(result.failures) > 0

    for f in result.failures:
      run_reporter.DidAddFailure(f)
    run_reporter.DidRun(had_failure)

    self._wq.PostMainThreadTask(
        self._MergeResultIntoMaster, result, trace_handle)

  def _MergeResultIntoMaster(self, result, trace_handle):
    self._map_results[trace_handle.canonical_url] = result

    had_failure = len(result.failures) > 0
    if self._stop_on_error and had_failure:
      err = MapError("Mapping error")
      self._AbortMappingDueStopOnError(err)
      raise err

    self._num_traces_merged_into_results += 1
    if self._num_traces_merged_into_results == len(self._trace_handles):
      self._wq.PostMainThreadTask(self._AllMappingDone)

  def _AbortMappingDueStopOnError(self, err):
    self._wq.Stop(err)

  def _AllMappingDone(self):
    self._wq.Stop()

  def RunMapper(self):
    self._map_results = {}

    if not self._trace_handles:
      err = MapError("No trace handles specified.")
      raise err

    if self._job.map_function_handle:
      for trace_handle in self._trace_handles:
        self._wq.PostAnyThreadTask(self._ProcessOneTrace, trace_handle)

      self._wq.Run()

    return self._map_results

  def Run(self):
    results_by_trace = self.RunMapper()
    results = list(results_by_trace.values())

    for of in self._output_formatters:
      of.Format(results)

    return results
