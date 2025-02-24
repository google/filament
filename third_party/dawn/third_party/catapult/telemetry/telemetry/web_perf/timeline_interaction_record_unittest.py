# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.web_perf import timeline_interaction_record as tir_module


class TimelineInteractionRecordTests(unittest.TestCase):
  def testGetJavaScriptMarker(self):
    repeatable_marker = tir_module.GetJavaScriptMarker(
        'MyLabel', [tir_module.REPEATABLE])
    self.assertEqual('Interaction.MyLabel/repeatable', repeatable_marker)
