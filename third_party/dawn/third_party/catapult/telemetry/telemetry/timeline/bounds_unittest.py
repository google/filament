# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.timeline import bounds


class BoundsTests(unittest.TestCase):

  def testGetOverlap(self):
    # Non overlap cases.
    self.assertEqual(0, bounds.Bounds.GetOverlap(10, 20, 30, 40))
    self.assertEqual(0, bounds.Bounds.GetOverlap(30, 40, 10, 20))
    # Overlap cases.
    self.assertEqual(10, bounds.Bounds.GetOverlap(10, 30, 20, 40))
    self.assertEqual(10, bounds.Bounds.GetOverlap(20, 40, 10, 30))
    # Inclusive cases.
    self.assertEqual(10, bounds.Bounds.GetOverlap(10, 40, 20, 30))
    self.assertEqual(10, bounds.Bounds.GetOverlap(20, 30, 10, 40))
