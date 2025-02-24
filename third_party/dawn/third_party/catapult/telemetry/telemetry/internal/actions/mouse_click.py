# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import page_action
from telemetry.internal.actions import utils


class MouseClickAction(page_action.ElementPageAction):

  def WillRunAction(self, tab):
    """Load the mouse click JS code prior to running the action."""
    super().WillRunAction(tab)
    utils.InjectJavaScript(tab, 'mouse_click.js')
    tab.ExecuteJavaScript("""
        window.__mouseClickActionDone = false;
        window.__mouseClickAction = new __MouseClickAction(function() {
          window.__mouseClickActionDone = true;
        });""")

  def RunAction(self, tab):
    code = '''
        function(element, info) {
          if (!element) {
            throw Error('Cannot find element: ' + info);
          }
          window.__mouseClickAction.start({
            element: element
          });
        }'''
    self.EvaluateCallback(tab, code)
    tab.WaitForJavaScriptCondition(
        'window.__mouseClickActionDone', timeout=self.timeout)
