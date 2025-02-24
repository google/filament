# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
from telemetry import decorators

from telemetry.internal.actions import page_action
from telemetry.internal.actions import repeatable_scroll
from telemetry.internal.actions import utils
from telemetry.internal.browser import browser_info as browser_info_module
from telemetry.testing import tab_test_case


class RepeatableScrollActionTest(tab_test_case.TabTestCase):

  def setUp(self):
    tab_test_case.TabTestCase.setUp(self)
    self.Navigate('blank.html')
    utils.InjectJavaScript(self._tab, 'gesture_common.js')

    # Make page taller than window so it's scrollable.
    self._original_height = int(
        self._tab.EvaluateJavaScript('__GestureCommon_GetScrollableHeight()'))
    self._tab.ExecuteJavaScript(
        'document.body.style.height ='
        '(3 * __GestureCommon_GetWindowHeight() + 1) + "px";')
    self._new_height = int(
        self._tab.EvaluateJavaScript('__GestureCommon_GetScrollableHeight()'))
    self._available_scroll = self._new_height - self._original_height

    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollTop'), 0)

    self._browser_info = browser_info_module.BrowserInfo(self._tab.browser)
    self._window_height = int(
        self._tab.EvaluateJavaScript('__GestureCommon_GetWindowHeight()'))

  # Test flaky on chromeos: https://crbug.com/826527.
  @decorators.Disabled('chromeos')
  def testRepeatableScrollActionNoRepeats(self):
    expected_scroll = (self._window_height // 2) - 1
    self.assertLess(
        expected_scroll,
        self._available_scroll,
        msg='cannot run test because available scroll is too low.'
        ' Available:%d; Expected:%d' % (self._available_scroll,
                                        expected_scroll))

    i = repeatable_scroll.RepeatableScrollAction(y_scroll_distance_ratio=0.5)
    i.WillRunAction(self._tab)

    i.RunAction(self._tab)

    scroll_position = self._tab.EvaluateJavaScript(
        'document.scrollingElement.scrollTop')
    # We can only expect the final scroll position to be approximatly equal.
    self.assertTrue(
        abs(scroll_position - expected_scroll) < 20,
        msg='scroll_position=%d;expected %d' % (scroll_position,
                                                expected_scroll))

  # Flaky on chromeos: https://crbug.com/932104.
  @decorators.Disabled('chromeos')
  def testRepeatableScrollActionTwoRepeats(self):
    expected_scroll = ((self._window_height // 2) - 1) * 3
    self.assertLess(
        expected_scroll,
        self._available_scroll,
        msg='cannot run test because available scroll is too low.'
        ' Available:%d; Expected:%d' % (self._available_scroll,
                                        expected_scroll))

    i = repeatable_scroll.RepeatableScrollAction(
        y_scroll_distance_ratio=0.5, repeat_count=2, repeat_delay_ms=1)
    i.WillRunAction(self._tab)

    i.RunAction(self._tab)

    scroll_position = self._tab.EvaluateJavaScript(
        'document.scrollingElement.scrollTop')
    # We can only expect the final scroll position to be approximatly equal.
    self.assertTrue(
        abs(scroll_position - expected_scroll) < 20,
        msg='scroll_position=%d;expected %d' % (scroll_position,
                                                expected_scroll))

  # Regression test for crbug.com/627166
  # TODO(ulan): enable for Android after catapult:#2475 is fixed.
  @decorators.Disabled('all')
  def testRepeatableScrollActionNoRepeatsZoomed(self):
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      self.skipText('touch gestures not supported')

    self._tab.action_runner.PinchPage(scale_factor=0.1)

    inner_height = self._tab.EvaluateJavaScript('window.innerHeight')
    outer_height = self._tab.EvaluateJavaScript('window.outerHeight')

    self.assertGreater(inner_height, outer_height)

    i = repeatable_scroll.RepeatableScrollAction(y_scroll_distance_ratio=0.5)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)
    # If scroll coordinates are computed incorrectly Chrome will crash with
    # [FATAL:synthetic_gesture_target_base.cc(62)] Check failed:
    # web_touch.touches[i].state != WebTouchPoint::StatePressed ||
    # PointIsWithinContents(web_touch.touches[i].position.x,
    # web_touch.touches[i].position.y). Touch coordinates are not within content
    # bounds on TouchStart.
