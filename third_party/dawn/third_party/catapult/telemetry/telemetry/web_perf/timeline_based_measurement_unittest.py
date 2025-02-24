# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import shutil
import tempfile
import unittest

from telemetry import benchmark
from telemetry import decorators
from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.testing import options_for_unittests
from telemetry.testing import test_stories
from telemetry.web_perf import timeline_based_measurement


class TestTimelineBenchmark(benchmark.Benchmark):
  def __init__(self, story_run_side_effect=None):
    super().__init__()
    self._story_run_side_effect = story_run_side_effect

  def CreateStorySet(self, _):
    return test_stories.SinglePageStorySet(
        story_run_side_effect=self._story_run_side_effect)

  def CreateCoreTimelineBasedMeasurementOptions(self):
    options = timeline_based_measurement.Options()
    options.config.enable_chrome_trace = True
    # Increase buffer size to deal with flaky test cases running into the
    # buffer limit (crbug.com/1193748).
    options.config.chrome_trace_config.SetTraceBufferSizeInKb(400 * 1024)
    options.SetTimelineBasedMetrics(['sampleMetric'])
    return options

  @classmethod
  def Name(cls):
    return 'test_timeline_benchmark'


class TimelineBasedMeasurementTest(unittest.TestCase):
  """Tests for TimelineBasedMeasurement which allows to record traces."""

  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp())

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  def RunBenchmarkAndReadResults(self, test_benchmark):
    story_runner.RunBenchmark(test_benchmark, self.options)
    test_results = results_options.ReadTestResults(
        self.options.intermediate_dir)
    self.assertEqual(len(test_results), 1)
    return test_results[0]

  @decorators.Disabled('chromeos')  # crbug.com/1191132
  @decorators.Isolated
  def testTraceCaptureUponSuccess(self):
    test_benchmark = TestTimelineBenchmark()
    results = self.RunBenchmarkAndReadResults(test_benchmark)
    self.assertEqual(results['status'], 'PASS')
    # Assert that we can find a Chrome trace.
    self.assertTrue(any(
        n.startswith('trace/traceEvents') for n in results['outputArtifacts']))

  @decorators.Isolated
  def testTraceCaptureUponFailure(self):
    test_benchmark = TestTimelineBenchmark(
        story_run_side_effect=lambda a: a.TapElement('#does-not-exist'))
    results = self.RunBenchmarkAndReadResults(test_benchmark)
    self.assertEqual(results['status'], 'FAIL')
    # Assert that we can find a Chrome trace.
    self.assertTrue(any(
        n.startswith('trace/traceEvents') for n in results['outputArtifacts']))
