# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import page_action


class ClickElementAction(page_action.ElementPageAction):

  def RunAction(self, tab):
    code = '''
        function(element, errorMsg) {
          if (!element) {
            throw Error('Cannot find element: ' + errorMsg);
          }
          element.click();
        }'''
    # Click handler that plays media or requests fullscreen may not take
    # effects without user_gesture set to True.
    self.EvaluateCallback(tab, code, user_gesture=True)
