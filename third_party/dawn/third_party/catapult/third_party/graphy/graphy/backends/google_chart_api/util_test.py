#!/usr/bin/python2.4
#
# Copyright 2008 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Unittest for Graphy and Google Chart API backend."""

import string
import unittest

from graphy import graphy_test
from graphy.backends.google_chart_api import util


class SimpleEncoderTest(graphy_test.GraphyTest):

  def setUp(self):
    self.simple = util.SimpleDataEncoder()

  def testEmpty(self):
    self.assertEqual('', self.simple.Encode([]))

  def testSingle(self):
    self.assertEqual('A', self.simple.Encode([0]))

  def testFull(self):
    full = string.ascii_uppercase + string.ascii_lowercase + string.digits
    self.assertEqual(full, self.simple.Encode(range(0, 62)))

  def testRoundingError(self):
    """Scaling might give us some rounding error.  Make sure that the encoder
    deals with it properly.
    """
    a = [-1, 0, 0, 1, 60, 61, 61, 62]
    b = [-0.999999, -0.00001, 0.00001, 0.99998,
         60.00001, 60.99999, 61.00001, 61.99998]
    self.assertEqual(self.simple.Encode(a), self.simple.Encode(b))

  def testFloats(self):
    ints   = [1, 2, 3, 4]
    floats = [1.1, 2.1, 3.1, 4.1]
    self.assertEqual(self.simple.Encode(ints), self.simple.Encode(floats))

  def testOutOfRangeDropped(self):
    """Confirm that values outside of min/max are left blank."""
    nums = [-79, -1, 0, 1, 61, 62, 1012]
    self.assertEqual('__AB9__', self.simple.Encode(nums))

  def testNoneDropped(self):
    """Confirm that the value None is left blank."""
    self.assertEqual('_JI_H', self.simple.Encode([None, 9, 8, None, 7]))


class EnhandedEncoderTest(graphy_test.GraphyTest):

  def setUp(self):
    self.encoder = util.EnhancedDataEncoder()

  def testEmpty(self):
    self.assertEqual('', self.encoder.Encode([]))

  def testFull(self):
    full = ''.join(self.encoder.code)
    self.assertEqual(full, self.encoder.Encode(range(0, 4096)))

  def testOutOfRangeDropped(self):
    nums = [-79, -1, 0, 1, 61, 4096, 10012]
    self.assertEqual('____AAABA9____', self.encoder.Encode(nums))

  def testNoneDropped(self):
    self.assertEqual('__AJAI__AH', self.encoder.Encode([None, 9, 8, None, 7]))


class ScaleTest(graphy_test.GraphyTest):

  """Test scaling."""

  def testScaleIntegerData(self):
    scale = util.ScaleData
    # Identity
    self.assertEqual([1, 2, 3], scale([1, 2, 3], 1, 3, 1, 3))
    self.assertEqual([-1, 0, 1], scale([-1, 0, 1], -1, 1, -1, 1))

    # Translate
    self.assertEqual([4, 5, 6], scale([1, 2, 3], 1, 3, 4, 6))
    self.assertEqual([-3, -2, -1], scale([1, 2, 3], 1, 3, -3, -1))

    # Scale
    self.assertEqual([1, 3.5, 6], scale([1, 2, 3], 1, 3, 1, 6))
    self.assertEqual([-6, 0, 6], scale([1, 2, 3], 1, 3, -6, 6))

    # Scale and Translate
    self.assertEqual([100, 200, 300], scale([1, 2, 3], 1, 3, 100, 300))

  def testScaleDataWithDifferentMinMax(self):
    scale = util.ScaleData
    self.assertEqual([1.5, 2, 2.5], scale([1, 2, 3], 0, 4, 1, 3))
    self.assertEqual([-2, 2, 6], scale([0, 2, 4], 1, 3, 0, 4))

  def testScaleFloatingPointData(self):
    scale = util.ScaleData
    data = [-3.14, -2.72, 0, 2.72, 3.14]
    scaled_e = 5 + 5 * 2.72 / 3.14
    expected_data = [0, 10 - scaled_e, 5, scaled_e, 10]
    actual_data = scale(data, -3.14, 3.14, 0, 10)
    for expected, actual in zip(expected_data, actual_data):
      self.assertAlmostEqual(expected, actual)

  def testScaleDataOverRealRange(self):
    scale = util.ScaleData
    self.assertEqual([0, 30.5, 61], scale([1, 2, 3], 1, 3, 0, 61))

  def testScalingLotsOfData(self):
    data = range(0, 100)
    expected = range(-100, 100, 2)
    actual = util.ScaleData(data, 0, 100, -100, 100)
    self.assertEqual(expected, actual)


class NameTest(graphy_test.GraphyTest):

  """Test long/short parameter names."""

  def testLongNames(self):
    params = dict(size='S', data='D', chg='G')
    params = util.ShortenParameterNames(params)
    self.assertEqual(dict(chs='S', chd='D', chg='G'), params)

  def testCantUseBothLongAndShortName(self):
    """Make sure we don't let the user specify both the long and the short
    version of a parameter.  (If we did, which one would we pick?)
    """
    params = dict(size='long', chs='short')
    self.assertRaises(KeyError, util.ShortenParameterNames, params)


if __name__ == '__main__':
  unittest.main()
