# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A Telemetry page_action that performs the "seek" action on media elements.

Action parameters are:
- seconds: The media time to seek to. Test fails if not provided.
- selector: If no selector is defined then the action attempts to seek the first
            media element on the page. If 'all' then seek all media elements.
- timeout_in_seconds: Maximum waiting time for the "seeked" event
                      (dispatched when the seeked operation completes)
                      to be fired.  0 means do not wait.
- log_time: If true the seek time is recorded, otherwise media
            measurement will not be aware of the seek action. Used to
            perform multiple seeks. Default true.
- label: A suffix string to name the seek perf measurement.
"""

from __future__ import absolute_import
from telemetry.core import exceptions
from telemetry.internal.actions import media_action
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils


class SeekAction(media_action.MediaAction):

  def __init__(self,
               seconds,
               selector=None,
               timeout_in_seconds=0,
               log_time=True,
               label=''):
    super().__init__(timeout=timeout_in_seconds)
    self._seconds = seconds
    self._selector = selector if selector else ''
    self._log_time = log_time
    self._label = label

  def WillRunAction(self, tab):
    """Load the media metrics JS code prior to running the action."""
    super().WillRunAction(tab)
    utils.InjectJavaScript(tab, 'seek.js')

  def RunAction(self, tab):
    try:
      tab.ExecuteJavaScript(
          'window.__seekMedia('
          '{{ selector }}, {{ seconds }}, {{ log_time }}, {{ label}});',
          selector=self._selector,
          seconds=str(self._seconds),
          log_time=self._log_time,
          label=self._label)
      if self.timeout > 0:
        self.WaitForEvent(tab, self._selector, 'seeked', self.timeout)
    except exceptions.EvaluateException as e:
      raise page_action.PageActionFailed(
          'Cannot seek media element(s) with selector = %s.' %
          self._selector) from e

  def __str__(self):
    return "%s(%s)" % (self.__class__.__name__, self._selector)
