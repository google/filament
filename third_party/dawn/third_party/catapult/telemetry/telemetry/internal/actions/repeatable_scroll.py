# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
import numbers

from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils
from telemetry.web_perf import timeline_interaction_record


class RepeatableScrollAction(page_action.PageAction):

  def __init__(self, x_scroll_distance_ratio=0.0, y_scroll_distance_ratio=0.5,
               repeat_count=0, repeat_delay_ms=250,
               timeout=page_action.DEFAULT_TIMEOUT,
               prevent_fling=None, speed=None):
    super().__init__(timeout=timeout)
    self._x_scroll_distance_ratio = x_scroll_distance_ratio
    self._y_scroll_distance_ratio = y_scroll_distance_ratio
    self._repeat_count = repeat_count
    self._repeat_delay_ms = repeat_delay_ms
    self._windowsize = []
    self._prevent_fling = prevent_fling
    self._speed = speed

  def WillRunAction(self, tab):
    utils.InjectJavaScript(tab, 'gesture_common.js')
    # Get the dimensions of the screen.
    self._windowsize = tab.EvaluateJavaScript(
        '[__GestureCommon_GetWindowWidth(),'
        ' __GestureCommon_GetWindowHeight()]')
    assert len(self._windowsize) == 2
    assert all(isinstance(d, numbers.Number) for d in self._windowsize)

  def RunAction(self, tab):
    # Set up a browser driven repeating scroll. The delay between the scrolls
    # should be unaffected by render thread responsivness (or lack there of).
    tab.SynthesizeScrollGesture(
        #2To3-division: this line is unchanged as result is expected floats.
        x=int(self._windowsize[0] / 2),
        y=int(self._windowsize[1] / 2),
        x_distance=int(self._x_scroll_distance_ratio * self._windowsize[0]),
        y_distance=int(-self._y_scroll_distance_ratio * self._windowsize[1]),
        prevent_fling=self._prevent_fling,
        speed=self._speed,
        repeat_count=self._repeat_count,
        repeat_delay_ms=self._repeat_delay_ms,
        interaction_marker_name=timeline_interaction_record.GetJavaScriptMarker(
            'Gesture_ScrollAction', [timeline_interaction_record.REPEATABLE]),
        timeout=self.timeout)
