# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.api import utils


class ParseBoolTest(unittest.TestCase):

  def testTrueValues(self):
    self.assertTrue(utils.ParseBool('1'))
    self.assertTrue(utils.ParseBool('true'))
    self.assertTrue(utils.ParseBool('True'))
    self.assertTrue(utils.ParseBool('TRUE'))

  def testFalseValues(self):
    self.assertFalse(utils.ParseBool('0'))
    self.assertFalse(utils.ParseBool('false'))
    self.assertFalse(utils.ParseBool('False'))
    self.assertFalse(utils.ParseBool('FALSE'))

  def testNoneValueIsNone(self):
    self.assertIsNone(utils.ParseBool(None))

  def testInvalidValueRaises(self):
    with self.assertRaises(ValueError):
      utils.ParseBool('foo')


if __name__ == '__main__':
  unittest.main()
