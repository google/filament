# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.util import matching


class BenchmarkFoo():
  """ Benchmark Foo for testing."""
  @classmethod
  def Name(cls):
    return 'FooBenchmark'


class BenchmarkBar():
  """ Benchmark Bar for testing long description line."""
  @classmethod
  def Name(cls):
    return 'BarBenchmarkkkkk'


class UnusualBenchmark():
  @classmethod
  def Name(cls):
    return 'I have a very unusual name'


class CommandLineUnittest(unittest.TestCase):
  def testGetMostLikelyMatchedObject(self):
    # Test moved from telemetry/benchmark_runner_unittest.py
    all_benchmarks = [BenchmarkFoo, BenchmarkBar, UnusualBenchmark]
    self.assertEqual([BenchmarkFoo, BenchmarkBar],
                     matching.GetMostLikelyMatchedObject(
                         all_benchmarks,
                         'BenchmarkFooz',
                         name_func=lambda x: x.Name()))

    self.assertEqual([BenchmarkBar, BenchmarkFoo],
                     matching.GetMostLikelyMatchedObject(
                         all_benchmarks,
                         'BarBenchmark',
                         name_func=lambda x: x.Name()))

    self.assertEqual([UnusualBenchmark],
                     matching.GetMostLikelyMatchedObject(
                         all_benchmarks,
                         'unusual',
                         name_func=lambda x: x.Name()))
