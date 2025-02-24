# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
from telemetry import decorators
from telemetry.internal.actions import page_action
from telemetry.internal.actions import pinch
from telemetry.internal.actions import utils
from telemetry.testing import tab_test_case


class PinchActionTest(tab_test_case.TabTestCase):
  def setUp(self):
    super().setUp()
    self.Navigate('zoom.html')
    utils.InjectJavaScript(self._tab, 'gesture_common.js')

  @decorators.Disabled('android')  # crbug.com/796615
  def testPinchByApiCalledWithCorrectArguments(self):
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    action_runner = self._tab.action_runner
    action_runner.ExecuteJavaScript('''
        chrome.gpuBenchmarking.pinchBy = function(
            scaleFactor, anchorLeft, anchorTop, callback, speed) {
          window.__test_scaleFactor = scaleFactor;
          window.__test_anchorLeft = anchorLeft;
          window.__test_anchorTop = anchorTop;
          window.__test_callback = callback;
          window.__test_speed = speed;
          window.__pinchActionDone = true;
        };''')
    action_runner.PinchPage(scale_factor=2)
    self.assertEqual(
        2, action_runner.EvaluateJavaScript('window.__test_scaleFactor'))
    self.assertTrue(
        action_runner.EvaluateJavaScript('!isNaN(window.__test_anchorLeft)'))
    self.assertTrue(
        action_runner.EvaluateJavaScript('!isNaN(window.__test_anchorTop)'))
    self.assertTrue(
        action_runner.EvaluateJavaScript('!!window.__test_callback'))
    self.assertEqual(
        800, action_runner.EvaluateJavaScript('window.__test_speed'))

  # TODO(bokan): It looks like pinch gestures don't quite work correctly on
  # desktop and ChromeOS. Disable for now and investigate later.
  # https://crbug.com/787615 and https://crbug.com/797834.
  @decorators.Disabled('chromeos', 'linux', 'mac', 'win')
  def testPinchScale(self):
    starting_scale = self._tab.EvaluateJavaScript('window.visualViewport.scale')

    i = pinch.PinchAction(
        scale_factor=4,
        speed_in_pixels_per_second=500)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    scale = self._tab.EvaluateJavaScript('window.visualViewport.scale')

    self.assertAlmostEqual(scale, starting_scale * 4, delta=0.2 * scale)

    starting_scale = scale

    i = pinch.PinchAction(
        scale_factor=0.5,
        speed_in_pixels_per_second=500)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    scale = self._tab.EvaluateJavaScript('window.visualViewport.scale')

    # Note: we have to use an approximate equality here. The pinch gestures
    # aren't exact. Some investigation long ago showed that the touch slop
    # distance in Android's gesture recognizer violated some assumptions made
    # by the synthetic gestures. There's (a bit of) context in
    # https://crbug.com/686390.
    # TODO(bokan): I landed changes in M65 (https://crrev.com/c/784213) that
    # should make this significantly better. We can tighten up the bounds once
    # that's in ref builds.
    #2To3-division: this line is unchanged as result is expected floats.
    self.assertAlmostEqual(scale, starting_scale / 2, delta=0.2 * scale)

  # Test that the anchor ratio correctly centers the pinch gesture at the
  # requested part of the viewport.
  # TODO(bokan): It looks like pinch gestures don't quite work correctly on
  # desktop and ChromeOS. Disable for now and investigate later.
  # https://crbug.com/787615 and https://crbug.com/797834.
  @decorators.Disabled('chromeos', 'linux', 'mac', 'win')
  def testPinchAnchor(self):
    starting_scale = self._tab.EvaluateJavaScript('window.visualViewport.scale')
    self.assertEqual(1, starting_scale)

    width = self._tab.EvaluateJavaScript('window.innerWidth')
    height = self._tab.EvaluateJavaScript('window.innerHeight')

    # Pinch zoom into the bottom right corner. Note, we can't go exactly to the
    # corner since the starting touch point would be outside the screen.
    i = pinch.PinchAction(
        left_anchor_ratio=0.9,
        top_anchor_ratio=0.9,
        scale_factor=2.5,
        speed_in_pixels_per_second=500)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    offset_x = self._tab.EvaluateJavaScript('window.visualViewport.offsetLeft')
    offset_y = self._tab.EvaluateJavaScript('window.visualViewport.offsetTop')

    self.assertGreater(offset_x, width // 2)
    self.assertGreater(offset_y, height // 2)

    # TODO(bokan): This early-out can be removed once setPageScaleFactor (M64)
    # rolls into the ref builds.
    if not self._tab.EvaluateJavaScript(
        "'setPageScaleFactor' in chrome.gpuBenchmarking"):
      return

    self._tab.EvaluateJavaScript(
        'chrome.gpuBenchmarking.setPageScaleFactor(1)')

    # Try again in the top left.
    i = pinch.PinchAction(
        left_anchor_ratio=0.1,
        top_anchor_ratio=0.1,
        scale_factor=2.5,
        speed_in_pixels_per_second=500)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    offset_x = self._tab.EvaluateJavaScript('window.visualViewport.offsetLeft')
    offset_y = self._tab.EvaluateJavaScript('window.visualViewport.offsetTop')

    self.assertLess(offset_x + width // 2.5, width // 2)
    self.assertLess(offset_y + height // 2.5, height // 2)
