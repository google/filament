# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.internal.actions import tap
from telemetry.testing import tab_test_case

class TapActionTest(tab_test_case.TabTestCase):

  def _PerformTapAction(self, *args, **kwargs):
    action = tap.TapAction(*args, **kwargs)
    action.WillRunAction(self._tab)
    action.RunAction(self._tab)

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  # https://crbug.com/634343 (Windows)
  # https://crbug.com/1445887 (ChromeOS)
  @decorators.Disabled('android', 'win', 'chromeos')
  def testTapSinglePage(self):
    self.Navigate('page_with_clickables.html')

    self._tab.ExecuteJavaScript('valueSettableByTest = 1;')
    self._PerformTapAction('#test')
    self.assertEqual(1, self._tab.EvaluateJavaScript('valueToTest'))

    self._tab.ExecuteJavaScript('valueSettableByTest = 2;')
    self._PerformTapAction(text='Click/tap me')
    self.assertEqual(2, self._tab.EvaluateJavaScript('valueToTest'))

    self._tab.ExecuteJavaScript('valueSettableByTest = 3;')
    self._PerformTapAction(element_function='document.body.firstElementChild')
    self.assertEqual(3, self._tab.EvaluateJavaScript('valueToTest'))

  # http://crbug.com/634343, http://crbug.com/1254282
  @decorators.Disabled('win', 'chromeos')
  def testTapNavigate(self):
    self.Navigate('page_with_link.html')
    self._PerformTapAction(selector='#clickme')
    self._tab.WaitForJavaScriptCondition(
        'document.location.pathname === "/blank.html"', timeout=5)
    self._tab.WaitForJavaScriptCondition(
        'document.readyState === "complete"', timeout=5)
