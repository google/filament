# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.testing import tab_test_case
from telemetry.util import image_util


class InspectorPageTest(tab_test_case.TabTestCase):
  def testPageNavigateToNormalUrl(self):
    self.Navigate('blank.html')

  def testCustomActionToNavigate(self):
    self.Navigate('page_with_link.html')
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.location.pathname;'),
        '/page_with_link.html')

    self._tab.ExecuteJavaScript('document.getElementById("clickme").click();')
    self._tab.WaitForNavigate()

    self.assertEqual(
        self._tab.EvaluateJavaScript('document.location.pathname;'),
        '/blank.html')

  def testScriptToEvaluateOnCommit(self):
    self.Navigate('blank.html',
                  script_to_evaluate_on_commit='var foo = "bar";')
    self._tab.WaitForDocumentReadyStateToBeComplete()
    self.assertEqual(self._tab.EvaluateJavaScript('foo'), 'bar')

  # crbug.com/483212, crbug.com/736628, crbug.com/758408.
  @decorators.Disabled('chromeos', 'mac', 'win')
  def testCaptureScreenshot(self):
    if not self._tab.screenshot_supported:
      return
    self.Navigate('green_rect.html')
    res = image_util.Pixels(self._tab.Screenshot())
    self.assertEqual(0x00, res[0])
    self.assertEqual(0xFF, res[1])
    self.assertEqual(0x00, res[2])
