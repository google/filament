# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.compare import kolmogorov_smirnov


class KolmogorovSmirnovTest(unittest.TestCase):

  def testBasic(self):
    self.assertAlmostEqual(
        kolmogorov_smirnov.KolmogorovSmirnov(list(range(10)), list(range(20, 30))),
        1.8879793657162556e-05)
    self.assertAlmostEqual(
        kolmogorov_smirnov.KolmogorovSmirnov(list(range(5)), list(range(10))),
        0.26680230985258474)

  def testDuplicateValues(self):
    self.assertAlmostEqual(
        kolmogorov_smirnov.KolmogorovSmirnov([0] * 5, [1] * 5),
        0.0037813540593701006)

  def testSmallSamples(self):
    self.assertEqual(
        kolmogorov_smirnov.KolmogorovSmirnov([0], [1]), 0.2890414283708268)

  def testAllValuesIdentical(self):
    self.assertEqual(
        kolmogorov_smirnov.KolmogorovSmirnov([0] * 5, [0] * 5), 1.0)


if __name__ == '__main__':
  unittest.main()
