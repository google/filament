# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import math
import unittest

from dashboard import find_step
from dashboard.common import math_utils

# Sample data where there is a small-ish step upwards around revision 304772.
_QUITE_STEPPISH = [
    (304469, 1056467.555),
    (304496, 1053394.222),
    (304521, 1052791.555),
    (304537, 1061429.777),
    (304554, 1057143.111),
    (304561, 1058818.222),
    (304571, 1052204.0),
    (304575, 1056174.222),
    (304580, 1057829.333),
    (304585, 1065234.222),
    (304591, 1049981.333),
    (304600, 1071542.222),
    (304613, 1059949.777),
    (304632, 1056992.888),
    (304650, 1061859.111),
    (304666, 1053670.222),
    (304685, 1068547.111),
    (304714, 1059868.444),
    (304739, 1056345.777),
    (304759, 1051712.0),
    (304767, 1058595.555),
    (304772, 1081055.555),
    (304783, 1080473.777),
    (304791, 1088900.0),
    (304794, 1083874.222),
    (304797, 1082668.444),
    (304809, 1086152.444),
    (304822, 1090828.444),
    (304839, 1094134.666),
    (304868, 1087627.555),
    (304892, 1085648.444),
    (304916, 1082401.777),
    (304957, 1090009.777),
    (304968, 1080076.444),
    (304984, 1085424.888),
    (304990, 1085520.444),
    (304995, 1089851.111),
    (305001, 1077174.222),
    (305003, 1085427.111),
    (305007, 1080062.222),
]

# Sample data where there is no clear step pattern.
_NOT_STEPPISH = [
    (301306, 965.568),
    (301307, 962.378),
    (301310, 962.355),
    (301313, 959.623),
    (301315, 967.824),
    (301317, 956.898),
    (301321, 1090.237),
    (301324, 957.379),
    (301330, 961.496),
    (301335, 951.585),
    (301340, 950.437),
    (301349, 952.022),
    (301354, 965.407),
    (301364, 962.668),
    (301366, 960.527),
    (301386, 1198.086),
    (301403, 979.097),
    (301405, 955.033),
    (301430, 952.072),
    (301456, 1026.557),
    (301467, 952.001),
    (301490, 993.909),
    (301529, 960.194),
    (301548, 960.707),
    (301557, 976.913),
    (301558, 980.134),
    (301568, 1144.155),
    (301576, 1234.563),
    (301589, 959.423),
    (301592, 959.784),
    (301599, 953.104),
    (301604, 978.390),
    (301628, 994.807),
    (301633, 970.342),
    (301641, 964.696),
    (301650, 960.404),
    (301665, 960.229),
    (301679, 978.075),
    (301696, 1176.206),
    (301709, 979.868),
    (301732, 973.265),
]


class FindStepTest(unittest.TestCase):

  def testFindStep_EmptyList_ReturnsNone(self):
    self.assertIsNone(find_step.FindStep([]))

  def testFindStep_LengthOneList_ReturnsNone(self):
    self.assertIsNone(find_step.FindStep([(1, 1.0)]))

  def testFindStep_LengthTwoList_FindsStep(self):
    self.assertEqual(2, (find_step.FindStep([(1, 1.0), (2, 2.0)])))

  def testFindStep_QuiteSteppishExample(self):
    self.assertEqual(304772, find_step.FindStep(_QUITE_STEPPISH))
    y_values = [y for _, y in _QUITE_STEPPISH]
    self.assertEqual(21, find_step._MinimizeDistanceFromStep(y_values))
    self.assertAlmostEqual(4863.05166991,
                           find_step._DistanceFromStep(y_values, 21))
    self.assertAlmostEqual(5.55211054,
                           find_step._RegressionSizeScore(y_values, 21))

  def testFindStep_NotSteppishExample(self):
    self.assertIsNone(find_step.FindStep(_NOT_STEPPISH))
    y_values = [y for _, y in _NOT_STEPPISH]
    self.assertEqual(15, find_step._MinimizeDistanceFromStep(y_values))
    self.assertAlmostEqual(67.44240311,
                           find_step._DistanceFromStep(y_values, 15))
    self.assertAlmostEqual(0.53773162,
                           find_step._RegressionSizeScore(y_values, 15))

  def testSteppiness_PerfectStep_ReturnsOne(self):
    step = [3, 3, 3, 3, 10, 10, 10]
    self.assertEqual(1.0, find_step.Steppiness(step, 4))

  def testSteppiness_ImperfectStep_ReturnsValueBetweenZeroAndOne(self):
    step = [1, 2, 1, 2, 4, 3, 5, 4]
    self.assertAlmostEqual(0.56005865, find_step.Steppiness(step, 4))

  def testSteppiness_LengthOfSeriesDoesntAffectScore(self):
    step = [1, 2, 1, 2, 4, 3, 5, 4]
    step_long = [1, 2, 1, 2, 2, 1, 2, 1, 4, 3, 5, 4, 5, 3, 4, 4]
    self.assertAlmostEqual(
        find_step.Steppiness(step, 4), find_step.Steppiness(step_long, 8))

  def testSteppiness_UpDownPattern_ReturnsZero(self):
    step = [1, 0, 1, 0, 1, 0, 1, 0]
    self.assertAlmostEqual(0.0, find_step.Steppiness(step, 4))

  def testNormalize_EmptyInput_ReturnsEmptyList(self):
    self.assertEqual(0, len(find_step._Normalize([])))

  def testNormalizeZeroVariance(self):
    # A list with a standard deviation of zero cannot be normalized because
    # the standard deviation cannot be made equal to 1.
    normalized = find_step._Normalize([5.0])
    self.assertEqual(1, len(normalized))
    self.assertTrue(math.isnan(normalized[0]))

    normalized = find_step._Normalize([5.0, 5.0])
    self.assertEqual(2, len(normalized))
    self.assertTrue(math.isnan(normalized[0]))
    self.assertTrue(math.isnan(normalized[1]))

  def testNormalize_ShortList(self):
    self.assertEqual([-1, 1], find_step._Normalize([3, 4]))
    self.assertEqual([-1.2247448713915889, 0, 1.2247448713915889],
                     find_step._Normalize([3, 4, 5]))

  def testNormalize_ResultMeanIsZeroAndStdDevIsOne(self):
    # When a data series is normalized, it is guaranteed that the result
    # should have a mean of 0.0 and a standard deviation and variance of 1.0.
    _, y_values = list(zip(*_QUITE_STEPPISH))
    normalized = find_step._Normalize(y_values)
    self.assertAlmostEqual(0.0, math_utils.Mean(normalized))
    self.assertAlmostEqual(1.0, math_utils.StandardDeviation(normalized))

  def testMinimizeDistanceFromStep(self):
    self.assertRaises(ValueError, find_step._MinimizeDistanceFromStep, [])
    self.assertRaises(ValueError, find_step._MinimizeDistanceFromStep, [1])
    self.assertEqual(1, find_step._MinimizeDistanceFromStep([1, 2]))
    self.assertEqual(3, find_step._MinimizeDistanceFromStep([1, 2, 3, 5]))
    self.assertEqual(
        3, find_step._MinimizeDistanceFromStep([2.3, 2.0, 2.2, 4.4, 4.0]))

  def testRegressionSizeScore(self):
    self.assertEqual(0.0, find_step._RegressionSizeScore([1, 1], 1))
    self.assertEqual(float('inf'), find_step._RegressionSizeScore([1, 2], 1))
    self.assertEqual(7.0, find_step._RegressionSizeScore([3, 5, 10, 12], 2))
    self.assertLess(
        find_step._RegressionSizeScore([3, 5, 10, 12], 2),
        find_step._RegressionSizeScore([3, 4, 10, 11], 2))
    self.assertLess(
        find_step._RegressionSizeScore([3, 5, 10, 12], 2),
        find_step._RegressionSizeScore([3, 5, 20, 22], 2))

  def testDistanceFromStep(self):
    self.assertEqual(0.0, find_step._DistanceFromStep([], 0))
    self.assertEqual(0.0, find_step._DistanceFromStep([0, 1], 1))
    self.assertEqual(0.0, find_step._DistanceFromStep([4, 4, 2, 2], 2))
    self.assertEqual(1.0, find_step._DistanceFromStep([3, 5, 10, 12], 2))

  def testStepFunctionValues(self):
    self.assertEqual([], find_step._StepFunctionValues([], 0))
    self.assertEqual([2, 2, 2, 4], find_step._StepFunctionValues([1, 2, 3, 4],
                                                                 3))
    self.assertEqual([1.5, 1.5, 3.5, 3.5],
                     find_step._StepFunctionValues([1, 2, 3, 4], 2))

  def testRootMeanSquareDeviation(self):
    self.assertEqual(0.0, find_step._RootMeanSquareDeviation([], []))
    self.assertEqual(1.0, find_step._RootMeanSquareDeviation([1, 2], [2, 3]))
    self.assertEqual(2.0, find_step._RootMeanSquareDeviation([5.5], [3.5]))

  def testSumOfSquaredResiduals(self):
    self.assertEqual(0.0, find_step._SumOfSquaredResiduals([], []))
    self.assertEqual(0.0, find_step._SumOfSquaredResiduals([1, 2, 3], []))
    self.assertEqual(0.0, find_step._SumOfSquaredResiduals([1, 2], [1, 2]))
    self.assertEqual(2.0, find_step._SumOfSquaredResiduals([1, 2], [2, 3]))
    self.assertEqual(4.0, find_step._SumOfSquaredResiduals([5.5], [3.5]))
    self.assertEqual(8.0, find_step._SumOfSquaredResiduals([5.5, 4], [3.5, 6]))


if __name__ == '__main__':
  unittest.main()
