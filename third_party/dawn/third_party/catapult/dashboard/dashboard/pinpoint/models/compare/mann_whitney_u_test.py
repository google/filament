# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.compare import mann_whitney_u


class MannWhitneyUTest(unittest.TestCase):

  def testBasic(self):
    self.assertAlmostEqual(
        mann_whitney_u.MannWhitneyU(list(range(10)), list(range(20, 30))),
        0.00018267179110955002)
    self.assertAlmostEqual(
        mann_whitney_u.MannWhitneyU(list(range(5)), list(range(10))), 0.13986357686781267)

  def testDuplicateValues(self):
    self.assertAlmostEqual(
        mann_whitney_u.MannWhitneyU([0] * 5, [1] * 5), 0.0039767517097886512)

  def testSmallSamples(self):
    self.assertEqual(mann_whitney_u.MannWhitneyU([0], [1]), 1.0)

  def testAllValuesIdentical(self):
    self.assertEqual(mann_whitney_u.MannWhitneyU([0] * 5, [0] * 5), 1.0)


if __name__ == '__main__':
  unittest.main()
