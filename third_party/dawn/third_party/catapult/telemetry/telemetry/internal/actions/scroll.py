# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils
from telemetry.util import js_template


class ScrollAction(page_action.ElementPageAction):
  # TODO(chrishenry): Ignore attributes, to be deleted when usage in
  # other repo is cleaned up.
  def __init__(self,
               selector=None,
               text=None,
               element_function=None,
               left_start_ratio=0.5,
               top_start_ratio=0.5,
               direction='down',
               distance=None,
               distance_expr=None,
               speed_in_pixels_per_second=800,
               use_touch=False,
               synthetic_gesture_source=page_action.GESTURE_SOURCE_DEFAULT,
               vsync_offset_ms=0.0,
               input_event_pattern=page_action.INPUT_EVENT_PATTERN_DEFAULT,
               timeout=page_action.DEFAULT_TIMEOUT):
    super().__init__(
        selector, text, element_function, timeout=timeout)
    if direction not in ('down', 'up', 'left', 'right', 'downleft', 'downright',
                         'upleft', 'upright'):
      raise page_action.PageActionNotSupported(
          'Invalid scroll direction: %s' % direction)
    self._left_start_ratio = left_start_ratio
    self._top_start_ratio = top_start_ratio
    self._direction = direction
    self._speed = speed_in_pixels_per_second
    self._use_touch = use_touch
    self._synthetic_gesture_source = (
        'chrome.gpuBenchmarking.%s_INPUT' % synthetic_gesture_source)
    self._vsync_offset_ms = vsync_offset_ms
    self._input_event_pattern = (
        'chrome.gpuBenchmarking.%s_INPUT_PATTERN' % input_event_pattern)

    self._distance_func = js_template.RenderValue(None)
    if distance:
      assert not distance_expr
      distance_expr = str(distance)
    if distance_expr:
      self._distance_func = js_template.Render(
          'function() { return 0 + {{ @expr }}; }', expr=distance_expr)

  def WillRunAction(self, tab):
    utils.InjectJavaScript(tab, 'gesture_common.js')
    utils.InjectJavaScript(tab, 'scroll.js')

    # Fail if browser doesn't support synthetic scroll gestures.
    if not tab.EvaluateJavaScript('window.__ScrollAction_SupportedByBrowser()'):
      raise page_action.PageActionNotSupported(
          'Synthetic scroll not supported for this browser')

    # Fail if this action requires touch and we can't send touch events.
    if self._use_touch:
      if not page_action.IsGestureSourceTypeSupported(tab, 'touch'):
        raise page_action.PageActionNotSupported(
            'Touch scroll not supported for this browser')

      if (self._synthetic_gesture_source ==
          'chrome.gpuBenchmarking.MOUSE_INPUT'):
        raise page_action.PageActionNotSupported(
            'Scroll requires touch on this page but mouse input was requested')

    tab.ExecuteJavaScript(
        """
        window.__scrollActionDone = false;
        window.__scrollAction = new __ScrollAction(
            {{ @callback }}, {{ @distance }});""",
        callback='function() { window.__scrollActionDone = true; }',
        distance=self._distance_func)

  def RunAction(self, tab):
    if not self.HasElementSelector():
      self._element_function = '(document.scrollingElement || document.body)'

    gesture_source_type = self._synthetic_gesture_source
    if self._use_touch:
      gesture_source_type = 'chrome.gpuBenchmarking.TOUCH_INPUT'

    code = js_template.Render(
        """
        function(element, info) {
          if (!element) {
            throw Error('Cannot find element: ' + info);
          }
          window.__scrollAction.start({
            element: element,
            left_start_ratio: {{ left_start_ratio }},
            top_start_ratio: {{ top_start_ratio }},
            direction: {{ direction }},
            speed: {{ speed }},
            gesture_source_type: {{ @gesture_source_type }},
            vsync_offset_ms: {{ vsync_offset_ms }},
            input_event_pattern: {{ @input_event_pattern }}
          });
        }""",
        left_start_ratio=self._left_start_ratio,
        top_start_ratio=self._top_start_ratio,
        direction=self._direction,
        speed=self._speed,
        gesture_source_type=gesture_source_type,
        vsync_offset_ms=self._vsync_offset_ms,
        input_event_pattern=self._input_event_pattern)
    self.EvaluateCallback(tab, code)
    tab.WaitForJavaScriptCondition(
        'window.__scrollActionDone', timeout=self.timeout)
