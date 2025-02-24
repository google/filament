# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils
from telemetry.util import js_template


class TapAction(page_action.ElementPageAction):

  def __init__(self,
               selector=None,
               text=None,
               element_function=None,
               left_position_percentage=0.5,
               top_position_percentage=0.5,
               duration_ms=50,
               synthetic_gesture_source=page_action.GESTURE_SOURCE_DEFAULT,
               timeout=page_action.DEFAULT_TIMEOUT):
    super().__init__(selector, text, element_function, timeout)
    self._left_position_percentage = left_position_percentage
    self._top_position_percentage = top_position_percentage
    self._duration_ms = duration_ms
    self._synthetic_gesture_source = (
        'chrome.gpuBenchmarking.%s_INPUT' % synthetic_gesture_source)

  def WillRunAction(self, tab):
    utils.InjectJavaScript(tab, 'gesture_common.js')
    utils.InjectJavaScript(tab, 'tap.js')

    # Fail if browser doesn't support synthetic tap gestures.
    if not tab.EvaluateJavaScript('window.__TapAction_SupportedByBrowser()'):
      raise page_action.PageActionNotSupported(
          'Synthetic tap not supported for this browser')

    tab.ExecuteJavaScript("""
        window.__tapActionDone = false;
        window.__tapAction = new __TapAction(function() {
          window.__tapActionDone = true;
        });""")

  def RunAction(self, tab):
    if not self.HasElementSelector():
      self._element_function = 'document.body'

    code = js_template.Render(
        """
        function(element, errorMsg) {
          if (!element) {
            throw Error('Cannot find element: ' + errorMsg);
          }
          window.__tapAction.start({
            element: element,
            left_position_percentage: {{ left_position_percentage }},
            top_position_percentage: {{ top_position_percentage }},
            duration_ms: {{ duration_ms }},
            gesture_source_type: {{ @gesture_source_type }}
          });
        }""",
        left_position_percentage=self._left_position_percentage,
        top_position_percentage=self._top_position_percentage,
        duration_ms=self._duration_ms,
        gesture_source_type=self._synthetic_gesture_source)

    self.EvaluateCallback(tab, code)
    # The second disjunct handles the case where the tap action leads to an
    # immediate navigation (in which case the expression below might already be
    # evaluated on the new page).
    tab.WaitForJavaScriptCondition(
        'window.__tapActionDone || window.__tapAction === undefined',
        timeout=self.timeout)
