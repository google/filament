# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
import sys

from telemetry.internal.util import path
from telemetry.testing import options_for_unittests


class ProgressReporter():
  def __init__(self, output_stream):
    self._output_stream = output_stream

  def StartTest(self, test):
    pass

  def StartTestSuite(self, suite):
    pass

  def StartTestRun(self):
    pass

  def StopTest(self, test):
    pass

  def StopTestSuite(self, suite):
    pass

  def StopTestRun(self, result):
    pass

  def Error(self, test, err):
    pass

  def Failure(self, test, err):
    pass

  def Success(self, test):
    pass

  def Skip(self, test, reason):
    pass


class TestSuite(unittest.TestSuite):
  """TestSuite that can delegate start and stop calls to a TestResult object."""
  def run(self, result):  # pylint: disable=arguments-differ
    if hasattr(result, 'startTestSuite'):
      result.startTestSuite(self)
    result = super().run(result)
    if hasattr(result, 'stopTestSuite'):
      result.stopTestSuite(self)
    return result


class TestRunner():
  def run(self, test, progress_reporters, repeat_count, args):
    sys.path.append(path.GetUnittestDataDir())
    result = TestResult(progress_reporters)
    result.startTestRun()
    try:
      options_for_unittests.Push(args)
      for _ in range(repeat_count):
        test(result)
    finally:
      options_for_unittests.Pop()
      result.stopTestRun()

    return result


class TestResult(unittest.TestResult):
  def __init__(self, progress_reporters):
    super().__init__()
    self.successes = []
    self._progress_reporters = progress_reporters

  @property
  def failures_and_errors(self):
    return self.failures + self.errors

  def startTest(self, test):
    super().startTest(test)
    for progress_reporter in self._progress_reporters:
      progress_reporter.StartTest(test)

  def startTestSuite(self, suite):
    for progress_reporter in self._progress_reporters:
      progress_reporter.StartTestSuite(suite)

  def startTestRun(self):
    super().startTestRun()
    for progress_reporter in self._progress_reporters:
      progress_reporter.StartTestRun()

  def stopTest(self, test):
    super().stopTest(test)
    for progress_reporter in self._progress_reporters:
      progress_reporter.StopTest(test)

  def stopTestSuite(self, suite):
    for progress_reporter in self._progress_reporters:
      progress_reporter.StopTestSuite(suite)

  def stopTestRun(self):
    super().stopTestRun()
    for progress_reporter in self._progress_reporters:
      progress_reporter.StopTestRun(self)

  def addError(self, test, err):
    super().addError(test, err)
    for progress_reporter in self._progress_reporters:
      progress_reporter.Error(test, err)

  def addFailure(self, test, err):
    super().addFailure(test, err)
    for progress_reporter in self._progress_reporters:
      progress_reporter.Failure(test, err)

  def addSuccess(self, test):
    super().addSuccess(test)
    self.successes.append(test)
    for progress_reporter in self._progress_reporters:
      progress_reporter.Success(test)

  def addSkip(self, test, reason):
    super().addSkip(test, reason)
    for progress_reporter in self._progress_reporters:
      progress_reporter.Skip(test, reason)
