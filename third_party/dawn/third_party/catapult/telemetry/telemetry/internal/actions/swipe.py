# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils
from telemetry.util import js_template


class SwipeAction(page_action.ElementPageAction):

  def __init__(self,
               selector=None,
               text=None,
               element_function=None,
               left_start_ratio=0.5,
               top_start_ratio=0.5,
               direction='left',
               distance=100,
               speed_in_pixels_per_second=800,
               synthetic_gesture_source=page_action.GESTURE_SOURCE_DEFAULT,
               vsync_offset_ms=0.0,
               input_event_pattern=page_action.INPUT_EVENT_PATTERN_DEFAULT,
               timeout=page_action.DEFAULT_TIMEOUT):
    super().__init__(selector, text, element_function, timeout)
    if direction not in ['down', 'up', 'left', 'right']:
      raise page_action.PageActionNotSupported(
          'Invalid swipe direction: %s' % direction)
    self._left_start_ratio = left_start_ratio
    self._top_start_ratio = top_start_ratio
    self._direction = direction
    self._distance = distance
    self._speed = speed_in_pixels_per_second
    self._synthetic_gesture_source = (
        'chrome.gpuBenchmarking.%s_INPUT' % synthetic_gesture_source)
    self._vsync_offset_ms = vsync_offset_ms
    self._input_event_pattern = (
        'chrome.gpuBenchmarking.%s_INPUT_PATTERN' % input_event_pattern)

  def WillRunAction(self, tab):
    utils.InjectJavaScript(tab, 'gesture_common.js')
    utils.InjectJavaScript(tab, 'swipe.js')

    # Fail if browser doesn't support synthetic swipe gestures.
    if not tab.EvaluateJavaScript('window.__SwipeAction_SupportedByBrowser()'):
      raise page_action.PageActionNotSupported(
          'Synthetic swipe not supported for this browser')

    if self._synthetic_gesture_source == 'chrome.gpuBenchmarking.MOUSE_INPUT':
      raise page_action.PageActionNotSupported(
          'Swipe page action does not support mouse input')

    if not page_action.IsGestureSourceTypeSupported(tab, 'touch'):
      raise page_action.PageActionNotSupported(
          'Touch input not supported for this browser')

    tab.ExecuteJavaScript("""
        window.__swipeActionDone = false;
        window.__swipeAction = new __SwipeAction(function() {
          window.__swipeActionDone = true;
        });""")

  def RunAction(self, tab):
    if not self.HasElementSelector():
      self._element_function = '(document.scrollingElement || document.body)'
    code = js_template.Render(
        """
        function(element, info) {
          if (!element) {
            throw Error('Cannot find element: ' + info);
          }
          window.__swipeAction.start({
            element: element,
            left_start_ratio: {{ left_start_ratio }},
            top_start_ratio: {{ top_start_ratio }},
            direction: {{ direction }},
            distance: {{ distance }},
            speed: {{ speed }},
            vsync_offset_ms: {{ vsync_offset_ms }},
            input_event_pattern: {{ @input_event_pattern }}
          });
        }""",
        left_start_ratio=self._left_start_ratio,
        top_start_ratio=self._top_start_ratio,
        direction=self._direction,
        distance=self._distance,
        speed=self._speed,
        vsync_offset_ms=self._vsync_offset_ms,
        input_event_pattern=self._input_event_pattern)
    self.EvaluateCallback(tab, code)
    tab.WaitForJavaScriptCondition(
        'window.__swipeActionDone', timeout=self.timeout)
