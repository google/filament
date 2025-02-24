# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.internal.actions import action_runner as action_runner_module
from telemetry.testing import tab_test_case


class AndroidActionRunnerInteractionTest(tab_test_case.TabTestCase):

  @decorators.Enabled('android')
  def testSmoothScrollBy(self):
    self.Navigate('page_with_swipeables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.assertEqual(action_runner.EvaluateJavaScript('window.scrollY'), 0)
    self.assertEqual(action_runner.EvaluateJavaScript('window.scrollX'), 0)

    platform = action_runner.tab.browser.platform
    app_ui = action_runner.tab.browser.GetAppUi()
    view = app_ui.WaitForUiNode(resource_id='compositor_view_holder')
    scroll_start1 = 0.5 * (view.bounds.center + view.bounds.bottom_right)
    platform.android_action_runner.SmoothScrollBy(scroll_start1.x,
                                                  scroll_start1.y, 'down', 300)
    self.assertTrue(action_runner.EvaluateJavaScript('window.scrollY') > 0)

    scroll_start2 = 0.5 * (view.bounds.center + view.bounds.top_left)
    platform.android_action_runner.SmoothScrollBy(scroll_start2.x,
                                                  scroll_start2.y, 'up', 500)
    self.assertTrue(action_runner.EvaluateJavaScript('window.scrollY') == 0)

  @decorators.Enabled('android')
  def testInputSwipe(self):
    self.Navigate('page_with_swipeables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.assertEqual(action_runner.EvaluateJavaScript('window.scrollY'), 0)
    self.assertEqual(action_runner.EvaluateJavaScript('window.scrollX'), 0)

    platform = action_runner.tab.browser.platform
    app_ui = action_runner.tab.browser.GetAppUi()
    view = app_ui.WaitForUiNode(resource_id='compositor_view_holder')
    scroll_start1 = 0.5 * (view.bounds.center + view.bounds.bottom_right)
    scroll_end1 = scroll_start1.y - 300
    platform.android_action_runner.InputSwipe(scroll_start1.x, scroll_start1.y,
                                              scroll_start1.x, scroll_end1, 300)
    self.assertTrue(action_runner.EvaluateJavaScript('window.scrollY') > 0)

    scroll_start2 = 0.5 * (view.bounds.center + view.bounds.top_left)
    scroll_end2 = scroll_start2.y + 500
    platform.android_action_runner.InputSwipe(scroll_start2.x, scroll_start2.y,
                                              scroll_start2.x, scroll_end2, 500)
    self.assertTrue(action_runner.EvaluateJavaScript('window.scrollY') == 0)

  @decorators.Enabled('android')
  def testInputText(self):
    self.Navigate('blank.html')
    self._tab.ExecuteJavaScript(
        '(function() {'
        '  var elem = document.createElement("textarea");'
        '  document.body.appendChild(elem);'
        '  elem.focus();'
        '})();')

    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    platform = action_runner.tab.browser.platform
    platform.android_action_runner.InputText('Input spaces')
    platform.android_action_runner.InputText(', even multiple   spaces')

    # Check that the contents of the textarea is correct. It might take some
    # time until keystrokes are handled on Android.
    self._tab.WaitForJavaScriptCondition(
        ('document.querySelector("textarea").value === '
         '"Input spaces, even multiple   spaces"'),
        timeout=5)
