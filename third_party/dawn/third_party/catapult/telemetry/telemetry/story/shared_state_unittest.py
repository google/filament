# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.internal.browser import browser_finder
from telemetry.story import shared_state
from telemetry.testing import fakes
from telemetry.util import wpr_modes

class SharedStateTests(unittest.TestCase):
  def setUp(self):
    self.options = fakes.CreateBrowserFinderOptions()
    self.options.use_live_sites = False
    self.possible_browser = browser_finder.FindBrowser(self.options)

  def testSharedStateWprOff(self):
    self.options.use_live_sites = True
    run_state = shared_state.SharedState(
        None, self.options, None, self.possible_browser)
    self.assertEqual(run_state.wpr_mode, wpr_modes.WPR_OFF)

  def testSharedStateWprReplay(self):
    run_state = shared_state.SharedState(
        None, self.options, None, self.possible_browser)
    self.assertEqual(run_state.wpr_mode, wpr_modes.WPR_REPLAY)

  def testSharedStateWprRecord(self):
    self.options.browser_options.wpr_mode = wpr_modes.WPR_RECORD
    run_state = shared_state.SharedState(
        None, self.options, None, self.possible_browser)
    self.assertEqual(run_state.wpr_mode, wpr_modes.WPR_RECORD)
