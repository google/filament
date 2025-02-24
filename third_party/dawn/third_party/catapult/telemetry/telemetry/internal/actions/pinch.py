# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils
from telemetry.util import js_template


class PinchAction(page_action.PageAction):

  def __init__(self,
               left_anchor_ratio=0.5,
               top_anchor_ratio=0.5,
               scale_factor=None,
               speed_in_pixels_per_second=800,
               synthetic_gesture_source=page_action.GESTURE_SOURCE_DEFAULT,
               vsync_offset_ms=0.0,
               input_event_pattern=page_action.INPUT_EVENT_PATTERN_DEFAULT,
               timeout=page_action.DEFAULT_TIMEOUT):
    super().__init__(timeout=timeout)
    self._left_anchor_ratio = left_anchor_ratio
    self._top_anchor_ratio = top_anchor_ratio
    self._scale_factor = scale_factor
    self._speed = speed_in_pixels_per_second
    self._synthetic_gesture_source = (
        'chrome.gpuBenchmarking.%s_INPUT' % synthetic_gesture_source)
    self._vsync_offset_ms = vsync_offset_ms
    self._input_event_pattern = (
        'chrome.gpuBenchmarking.%s_INPUT_PATTERN' % input_event_pattern)

  def WillRunAction(self, tab):
    utils.InjectJavaScript(tab, 'gesture_common.js')
    utils.InjectJavaScript(tab, 'pinch.js')

    # Fail if browser doesn't support synthetic pinch gestures.
    if not tab.EvaluateJavaScript('window.__PinchAction_SupportedByBrowser()'):
      raise page_action.PageActionNotSupported(
          'Synthetic pinch not supported for this browser')

    tab.ExecuteJavaScript("""
        window.__pinchActionDone = false;
        window.__pinchAction = new __PinchAction(function() {
          window.__pinchActionDone = true;
        });""")

  @staticmethod
  def _GetDefaultScaleFactorForPage(tab):
    current_scale_factor = tab.EvaluateJavaScript("""
        "visualViewport" in window
            ? visualViewport.scale
            : window.outerWidth / window.innerWidth'""")
    #2To3-division: this line is unchanged as result is expected floats.
    return 3.0 / current_scale_factor

  def RunAction(self, tab):
    scale_factor = (self._scale_factor if self._scale_factor else
                    PinchAction._GetDefaultScaleFactorForPage(tab))
    code = js_template.Render(
        """
        window.__pinchAction.start({
          left_anchor_ratio: {{ left_anchor_ratio }},
          top_anchor_ratio: {{ top_anchor_ratio }},
          scale_factor: {{ scale_factor }},
          speed: {{ speed }},
          vsync_offset_ms: {{ vsync_offset_ms }},
          input_event_pattern: {{ @input_event_pattern }}
        });""",
        left_anchor_ratio=self._left_anchor_ratio,
        top_anchor_ratio=self._top_anchor_ratio,
        scale_factor=scale_factor,
        speed=self._speed,
        vsync_offset_ms=self._vsync_offset_ms,
        input_event_pattern=self._input_event_pattern)
    tab.EvaluateJavaScript(code)
    tab.WaitForJavaScriptCondition(
        'window.__pinchActionDone', timeout=self.timeout)
