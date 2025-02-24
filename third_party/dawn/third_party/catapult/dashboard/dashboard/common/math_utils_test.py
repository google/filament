# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import math
import unittest

from dashboard.common import math_utils


class MathUtilsTest(unittest.TestCase):
  """Tests for mathematical utility functions."""

  def testMean_EmptyInput_ReturnsNan(self):
    self.assertTrue(math.isnan(math_utils.Mean([])))

  def testMean_OneValue(self):
    self.assertEqual(3.0, math_utils.Mean([3]))

  def testMean_ShortList(self):
    self.assertEqual(0.5, math_utils.Mean([-3, 0, 1, 4]))

  def testMean_ShortList_SameAsAlternativeImplementation(self):
    alternate_mean = lambda xs: sum(xs) / float(len(xs))
    self.assertEqual(
        alternate_mean([-1, 0.12, 0.72, 3.3, 8, 32, 439]),
        math_utils.Mean([-1, 0.12, 0.72, 3.3, 8, 32, 439]))

  def testMedian_EmptyInput_ReturnsNan(self):
    self.assertTrue(math.isnan(math_utils.Median([])))

  def testMedian_OneValue(self):
    self.assertEqual(3.0, math_utils.Median([3]))

  def testMedian_OddLengthList_UsesMiddleValue(self):
    self.assertEqual(4.0, math_utils.Median([1, 4, 16]))

  def testMedian_EvenLengthList_UsesMeanOfMiddleTwoValues(self):
    self.assertEqual(10.0, math_utils.Median([1, 4, 16, 145]))

  def testVariance_EmptyList_ReturnsNan(self):
    self.assertTrue(math.isnan(math_utils.Variance([])))

  def testVariance_OneValue_ReturnsZero(self):
    self.assertEqual(0.0, math_utils.Variance([0]))
    self.assertEqual(0.0, math_utils.Variance([4.3]))

  def testVariance_ShortList_UsesPopulationVariance(self):
    self.assertAlmostEqual(6.25, sum([12.25, 0.25, 0.25, 12.25]) / 4.0)
    self.assertAlmostEqual(6.25, math_utils.Variance([-3, 0, 1, 4]))

  def testStandardDeviation_EmptyInput_ReturnsNan(self):
    self.assertTrue(math.isnan(math_utils.StandardDeviation([])))

  def testStandardDeviation_OneValue_ReturnsZero(self):
    self.assertEqual(0.0, math_utils.StandardDeviation([4.3]))

  def testStandardDeviation_UsesPopulationStandardDeviation(self):
    self.assertAlmostEqual(2.5, math.sqrt(6.25))
    self.assertAlmostEqual(2.5, math_utils.StandardDeviation([-3, 0, 1, 4]))

  def testDivide_ByZero_ReturnsZero(self):
    self.assertTrue(math.isnan(math_utils.Divide(1, 0)))

  def testDivide_UsesFloatArithmetic(self):
    self.assertEqual(1.5, math_utils.Divide(3, 2))

  def testRelativeChange(self):
    # The relative difference is with respect to the first number, and the
    # absolute value is taken. So 1 means doubling, and 0.5 means halving.
    self.assertEqual(1, math_utils.RelativeChange(32, 64))
    self.assertEqual(0.5, math_utils.RelativeChange(64, 32))

  def testRelativeChange_NoChange_ReturnsZero(self):
    self.assertEqual(0, math_utils.RelativeChange(7, 7))

  def testRelativeChange_FromZero_ReturnsInf(self):
    self.assertEqual(float('inf'), math_utils.RelativeChange(0, 1))

  def testIqr(self):
    self.assertEqual(4, math_utils.Iqr(list(range(8, 0, -1))))

  def testPercentile_RoundIndex(self):
    self.assertEqual(2, math_utils.Percentile(list(range(5)), 0.5))

  def testPercentile_Interpolation(self):
    self.assertEqual(5.1, math_utils.Percentile(list(range(8)), 0.7))

  def testPercentile_Min(self):
    self.assertEqual(0, math_utils.Percentile(list(range(8)), 0))

  def testPercentile_Max(self):
    self.assertEqual(7, math_utils.Percentile(list(range(8)), 1))


if __name__ == '__main__':
  unittest.main()
