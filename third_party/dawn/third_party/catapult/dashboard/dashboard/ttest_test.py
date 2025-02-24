# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard import ttest


class TTestTest(unittest.TestCase):
  """Tests for the t-test functions."""

  def setUp(self):
    """Sets the t-table values for the tests below."""
    table_patch = mock.patch.object(ttest, '_TABLE', [
        (1, [0, 6.314, 12.71, 31.82, 63.66, 318.31]),
        (2, [0, 2.920, 4.303, 6.965, 9.925, 22.327]),
        (3, [0, 2.353, 3.182, 4.541, 5.841, 10.215]),
        (4, [0, 2.132, 2.776, 3.747, 4.604, 7.173]),
        (10, [0, 1.372, 1.812, 2.228, 2.764, 3.169]),
        (100, [0, 1.290, 1.660, 1.984, 2.364, 2.626]),
    ])
    table_patch.start()
    self.addCleanup(table_patch.stop)
    two_tail_patch = mock.patch.object(ttest, '_TWO_TAIL',
                                       [1, 0.2, 0.1, 0.05, 0.02, 0.01])
    two_tail_patch.start()
    self.addCleanup(two_tail_patch.stop)

  def testWelchsTTest(self):
    """Tests the t value and degrees of freedom output of Welch's t-test."""
    # The t-value can be checked with scipy.stats.ttest_ind(equal_var=False).
    # However the t-value output by scipy.stats.ttest_ind is -6.32455532034.
    # This implementation produces slightly different results.
    result = ttest.WelchsTTest([2, 3, 2, 3, 2, 3], [4, 5, 4, 5, 4, 5])
    self.assertAlmostEqual(10.0, result.df)
    self.assertAlmostEqual(-6.325, result.t, delta=1.0)

  def testWelchsTTest_EmptySample_RaisesError(self):
    """An error should be raised when an empty sample is passed in."""
    with self.assertRaises(RuntimeError):
      ttest.WelchsTTest([], [])
    with self.assertRaises(RuntimeError):
      ttest.WelchsTTest([], [1, 2, 3])
    with self.assertRaises(RuntimeError):
      ttest.WelchsTTest([1, 2, 3], [])

  def testTTest_EqualSamples_PValueIsOne(self):
    """Checks that t = 0 and p = 1 when the samples are the same."""
    result = ttest.WelchsTTest([1, 2, 3], [1, 2, 3])
    self.assertEqual(0, result.t)
    self.assertEqual(1, result.p)

  def testTTest_VeryDifferentSamples_PValueIsLow(self):
    """Checks that p is very low when the samples are clearly different."""
    result = ttest.WelchsTTest([100, 101, 100, 101, 100],
                               [1, 2, 1, 2, 1, 2, 1, 2])
    self.assertLessEqual(250, result.t)
    self.assertLessEqual(0.01, result.p)

  def testTTest_DifferentVariance(self):
    """Verifies that higher variance -> higher p value."""
    result_low_var = ttest.WelchsTTest([2, 3, 2, 3], [4, 5, 4, 5])
    result_high_var = ttest.WelchsTTest([1, 4, 1, 4], [3, 6, 3, 6])
    self.assertLess(result_low_var.p, result_high_var.p)

  def testTTest_DifferentSampleSize(self):
    """Verifies that smaller sample size -> higher p value."""
    result_larger_sample = ttest.WelchsTTest([2, 3, 2, 3], [4, 5, 4, 5])
    result_smaller_sample = ttest.WelchsTTest([2, 3, 2, 3], [4, 5])
    self.assertLess(result_larger_sample.p, result_smaller_sample.p)

  def testTTest_DifferentMeanDifference(self):
    """Verifies that smaller difference between means -> higher p value."""
    result_far_means = ttest.WelchsTTest([2, 3, 2, 3], [5, 6, 5, 6])
    result_near_means = ttest.WelchsTTest([2, 3, 2, 3], [3, 4, 3, 4])
    self.assertLess(result_far_means.p, result_near_means.p)

  def testTValue(self):
    """Tests calculation of the t-value using Welch's formula."""
    # Results can be verified by directly plugging variables into Welch's
    # equation (e.g. using a calculator or the Python interpreter).
    stats1 = ttest.SampleStats(mean=0.299, var=0.05, size=150)
    stats2 = ttest.SampleStats(mean=0.307, var=0.08, size=165)
    # Note that a negative t-value is obtained when the first sample has a
    # smaller mean than the second, otherwise a positive value is returned.
    self.assertAlmostEqual(-0.27968236,
                           ttest._TValue(stats1=stats1, stats2=stats2))
    self.assertAlmostEqual(0.27968236,
                           ttest._TValue(stats1=stats2, stats2=stats1))

  def testTValue_ConstantSamples_ResultIsInfinity(self):
    """If there is no variation, infinity is used as the t-statistic value."""
    stats = ttest.SampleStats(mean=1.0, var=0, size=10)
    self.assertEqual(float('inf'), ttest._TValue(stats, stats))

  def testDegreesOfFreedom(self):
    """Tests calculation of estimated degrees of freedom."""
    # The formula used to estimate degrees of freedom for independent-samples
    # t-test is called the Welch-Satterthwaite equation. Note that since the
    # Welch-Satterthwaite equation gives an estimate of degrees of freedom,
    # the result is a floating-point number and not an integer.
    stats1 = ttest.SampleStats(mean=0.299, var=0.05, size=150)
    stats2 = ttest.SampleStats(mean=0.307, var=0.08, size=165)
    self.assertAlmostEqual(307.19879975,
                           ttest._DegreesOfFreedom(stats1, stats2))

  def testDegreesOfFreedom_ZeroVariance_ResultIsOne(self):
    """The lowest possible value is returned for df if variance is zero."""
    stats = ttest.SampleStats(mean=1.0, var=0, size=10)
    self.assertEqual(1.0, ttest._DegreesOfFreedom(stats, stats))

  def testDegreesOfFreedom_SmallSample_RaisesError(self):
    """Degrees of freedom can't be calculated if sample size is too small."""
    size_0 = ttest.SampleStats(mean=0, var=0, size=0)
    size_1 = ttest.SampleStats(mean=1.0, var=0, size=1)
    size_5 = ttest.SampleStats(mean=2.0, var=0.5, size=5)

    # An error is raised if the size of one of the samples is too small.
    with self.assertRaises(RuntimeError):
      ttest._DegreesOfFreedom(size_0, size_5)
    with self.assertRaises(RuntimeError):
      ttest._DegreesOfFreedom(size_1, size_5)
    with self.assertRaises(RuntimeError):
      ttest._DegreesOfFreedom(size_5, size_0)
    with self.assertRaises(RuntimeError):
      ttest._DegreesOfFreedom(size_5, size_1)

    # If both of the samples have a variance of 0, no error is raised.
    self.assertEqual(1.0, ttest._DegreesOfFreedom(size_1, size_1))


class LookupPValueTest(unittest.TestCase):

  def setUp(self):
    """Sets the t-table values for the tests below."""
    table_patch = mock.patch.object(ttest, '_TABLE', [
        (1, [0, 6.314, 12.71, 31.82, 63.66, 318.31]),
        (2, [0, 2.920, 4.303, 6.965, 9.925, 22.327]),
        (3, [0, 2.353, 3.182, 4.541, 5.841, 10.215]),
        (4, [0, 2.132, 2.776, 3.747, 4.604, 7.173]),
        (10, [0, 1.372, 1.812, 2.228, 2.764, 3.169]),
        (100, [0, 1.290, 1.660, 1.984, 2.364, 2.626]),
    ])
    table_patch.start()
    self.addCleanup(table_patch.stop)
    two_tail_patch = mock.patch.object(ttest, '_TWO_TAIL',
                                       [1, 0.2, 0.1, 0.05, 0.02, 0.01])
    two_tail_patch.start()
    self.addCleanup(two_tail_patch.stop)

  def testLookupPValue_ExactMatchInTable(self):
    """Tests looking up an entry that is in the table."""
    self.assertEqual(0.1, ttest._LookupPValue(3.182, 3.0))
    self.assertEqual(0.1, ttest._LookupPValue(-3.182, 3.0))

  def testLookupPValue_TValueBetweenTwoValues_SmallerColumnIsUsed(self):
    # The second column is used because 3.1 is below 4.303,
    # so the next-lowest t-value, 2.920, is used.
    self.assertEqual(0.2, ttest._LookupPValue(3.1, 2.0))
    self.assertEqual(0.2, ttest._LookupPValue(-3.1, 2.0))

  def testLookup_DFBetweenTwoValues_SmallerRowIsUsed(self):
    self.assertEqual(0.05, ttest._LookupPValue(2.228, 45.0))
    self.assertEqual(0.05, ttest._LookupPValue(-2.228, 45.0))

  def testLookup_DFAndTValueBetweenTwoValues_SmallerRowAndColumnIsUsed(self):
    self.assertEqual(0.1, ttest._LookupPValue(2.0, 45.0))
    self.assertEqual(0.1, ttest._LookupPValue(-2.0, 45.0))

  def testLookupPValue_LargeTValue_LastColumnIsUsed(self):
    # The smallest possible p-value will be used when t is large.
    self.assertEqual(0.01, ttest._LookupPValue(500.0, 1.0))
    self.assertEqual(0.01, ttest._LookupPValue(-500.0, 1.0))

  def testLookupPValue_ZeroTValue_FirstColumnIsUsed(self):
    # The largest possible p-value will be used when t is zero.
    self.assertEqual(1.0, ttest._LookupPValue(0.0, 1.0))
    self.assertEqual(1.0, ttest._LookupPValue(0.0, 2.0))

  def testLookupPValue_SmallTValue_FirstColumnIsUsed(self):
    # The largest possible p-value will be used when t is almost zero.
    self.assertEqual(1.0, ttest._LookupPValue(0.1, 2.0))
    self.assertEqual(1.0, ttest._LookupPValue(-0.1, 2.0))

  def testLookupPValue_LargeDegreesOfFreedom_LastRowIsUsed(self):
    # The last row of the table should be used.
    self.assertEqual(0.02, ttest._LookupPValue(2.365, 100.0))


if __name__ == '__main__':
  unittest.main()
