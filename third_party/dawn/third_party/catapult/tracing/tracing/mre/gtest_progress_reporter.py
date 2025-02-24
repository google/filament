# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import time
import sys

from tracing.mre import progress_reporter


class GTestRunReporter(progress_reporter.RunReporter):

  def __init__(self, canonical_url, output_stream, timestamp):
    super(GTestRunReporter, self).__init__(canonical_url)
    self._output_stream = output_stream
    self._timestamp = timestamp

  def _GetMs(self):
    assert self._timestamp is not None, 'Did not call WillRun.'
    return (time.time() - self._timestamp) * 1000

  def DidAddFailure(self, failure):
    super(GTestRunReporter, self).DidAddFailure(failure)
    print(failure.stack.encode('utf-8'), file=self._output_stream)
    self._output_stream.flush()

  def DidRun(self, run_failed):
    super(GTestRunReporter, self).DidRun(run_failed)
    if run_failed:
      print('[  FAILED  ] %s (%0.f ms)' % (self.canonical_url, self._GetMs()),
            file=self._output_stream)
    else:
      print('[       OK ] %s (%0.f ms)' % (self.canonical_url, self._GetMs()),
            file=self._output_stream)
    self._output_stream.flush()


class GTestProgressReporter(progress_reporter.ProgressReporter):
  """A progress reporter that outputs the progress report in gtest style.

  Be careful each print should only handle one string. Otherwise, the output
  might be interrupted by Chrome logging, and the output interpretation might
  be incorrect. For example:
      print("[ OK ]", testname, file=self._output_stream)
  should be written as
      print("[ OK ] %s" % testname, file=self._output_stream)
  """

  def __init__(self, output_stream=sys.stdout):
    super(GTestProgressReporter, self).__init__()
    self._output_stream = output_stream

  def WillRun(self, canonical_url):
    super(GTestProgressReporter, self).WillRun(canonical_url)
    print('[ RUN      ] %s' % canonical_url.encode('utf-8'),
          file=self._output_stream)
    self._output_stream.flush()
    return GTestRunReporter(canonical_url, self._output_stream, time.time())

  def DidFinishAllRuns(self, result_list):
    super(GTestProgressReporter, self).DidFinishAllRuns(result_list)
    successful_runs = 0
    failed_canonical_urls = []
    failed_runs = 0
    for run in result_list:
      if len(run.failures) != 0:
        failed_runs += 1
        for f in run.failures:
          failed_canonical_urls.append(f.trace_canonical_url)
      else:
        successful_runs += 1

    unit = 'test' if successful_runs == 1 else 'tests'
    print('[  PASSED  ] %d %s.' % (successful_runs, unit),
          file=self._output_stream)
    if len(failed_canonical_urls) > 0:
      unit = 'test' if len(failed_canonical_urls) == 1 else 'tests'
      print('[  FAILED  ] %d %s, listed below:' % (failed_runs, unit),
            file=self._output_stream)
      for failed_canonical_url in failed_canonical_urls:
        print('[  FAILED  ]  %s' % failed_canonical_url.encode('utf-8'),
              file=self._output_stream)
      print(file=self._output_stream)
      count = len(failed_canonical_urls)
      unit = 'TEST' if count == 1 else 'TESTS'
      print('%d FAILED %s' % (count, unit), file=self._output_stream)
    print(file=self._output_stream)

    self._output_stream.flush()
