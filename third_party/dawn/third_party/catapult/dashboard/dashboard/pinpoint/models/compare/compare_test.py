# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.compare import compare


class CompareTest(unittest.TestCase):

  def testNoValuesA(self):
    comparison = compare.Compare([], [0] * 10, 10, 'functional', 1)
    self.assertEqual(comparison.result, compare.UNKNOWN)
    self.assertIsNone(comparison.p_value)

  def testNoValuesB(self):
    comparison = compare.Compare(list(range(10)), [], 10, 'performance', 1)
    self.assertEqual(comparison.result, compare.UNKNOWN)
    self.assertIsNone(comparison.p_value)


class FunctionalTest(unittest.TestCase):

  def testDifferent(self):
    comparison = compare.Compare([0] * 10, [1] * 10, 10, 'functional', 0.5)
    self.assertEqual(comparison.result, compare.DIFFERENT)
    self.assertLessEqual(comparison.p_value, comparison.low_threshold)

  def testUnknown(self):
    comparison = compare.Compare([0] * 10, [0] * 9 + [1], 10, 'functional', 0.5)
    self.assertEqual(comparison.result, compare.UNKNOWN)
    self.assertLessEqual(comparison.p_value, comparison.high_threshold)

  def testSame(self):
    comparison = compare.Compare([0] * 10, [0] * 10, 10, 'functional', 0.5)
    self.assertEqual(comparison.result, compare.SAME)
    self.assertGreater(comparison.p_value, comparison.high_threshold)


class PerformanceTest(unittest.TestCase):

  def testDifferent(self):
    comparison = compare.Compare(list(range(10)), list(range(7, 17)), 10, 'performance', 1)
    self.assertEqual(comparison.result, compare.DIFFERENT)
    self.assertLessEqual(comparison.p_value, comparison.low_threshold)

  def testUnknown(self):
    comparison = compare.Compare(list(range(10)), list(range(3, 13)), 10, 'performance', 1)
    self.assertEqual(comparison.result, compare.UNKNOWN)
    self.assertLessEqual(comparison.p_value, comparison.high_threshold)

  def testSame(self):
    comparison = compare.Compare(list(range(10)), list(range(10)), 10, 'performance', 1)
    self.assertEqual(comparison.result, compare.SAME)
    self.assertGreater(comparison.p_value, comparison.high_threshold)
