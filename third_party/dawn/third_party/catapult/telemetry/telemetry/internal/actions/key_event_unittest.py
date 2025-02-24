# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import time

from telemetry import decorators
from telemetry.internal.actions import key_event
from telemetry.internal.actions import utils
from telemetry.testing import tab_test_case


class KeyPressActionTest(tab_test_case.TabTestCase):

  @property
  def _scroll_position(self):
    return self._tab.EvaluateJavaScript(
        'document.documentElement.scrollTop || document.body.scrollTop')

  @property
  def _window_height(self):
    return self._tab.EvaluateJavaScript('__GestureCommon_GetWindowHeight()')

  def _PressKey(self, key):
    action = key_event.KeyPressAction(key)
    action.WillRunAction(self._tab)
    action.RunAction(self._tab)

  @classmethod
  def setUpClass(cls):
    # Browser uses Google API keys to get access to Google services.
    # If browser was built without Google API keys, then on start browser
    # would try to fetch Google API keys from env variables. If those variables
    # are not set a warning "Google API Keys Missing" will appear at the start.
    # This warning disappears if any action is performed in the browser window.
    # This behaviour causes flaky bug in tests. The warning occupies part
    # of the window space and because of this method "_window_height" returns
    # different values when the warning is on the screen and
    # when it disappeared. It is critical for tests with scroll position checks
    # like testPressEndAndHome.
    # Tests in KeyPressActionTest class do not use any Google APIs,
    # so it is not needed to provide Google API keys to browser
    # to run those tests. If browser was built with Google API keys,
    # the warning would not be shown on start, so it is no need to do anything.
    # Otherwise we need to set specific env variables to 'no'
    # to disable warning.
    os.environ['GOOGLE_API_KEY'] = 'no'
    os.environ['GOOGLE_DEFAULT_CLIENT_ID'] = 'no'
    os.environ['GOOGLE_DEFAULT_CLIENT_SECRET'] = 'no'
    super().setUpClass()

  def setUp(self):
    tab_test_case.TabTestCase.setUp(self)
    self.Navigate('blank.html')
    utils.InjectJavaScript(self._tab, 'gesture_common.js')

  # https://github.com/catapult-project/catapult/issues/3099
  # crbug.com/1005062
  @decorators.Disabled('android', 'chromeos', 'mac')
  def testPressEndAndHome(self):
    # Make page taller than the window so it's scrollable.
    self._tab.ExecuteJavaScript(
        'document.body.style.height ='
        '(3 * __GestureCommon_GetWindowHeight() + 1) + "px";')

    # Check that the browser is currently showing the top of the page and that
    # the page has non-trivial height.
    self.assertEqual(0, self._scroll_position)
    self.assertLess(50, self._window_height)

    self._PressKey('End')

    # Scroll happens *after* key press returns, so we need to wait a little.
    time.sleep(1)

    # We can only expect the bottom scroll position to be approximatly equal.
    self.assertAlmostEqual(
        2 * self._window_height, self._scroll_position, delta=20)

    self._PressKey('Home')

    # Scroll happens *after* key press returns, so we need to wait a little.
    time.sleep(1)

    self.assertEqual(self._scroll_position, 0)

  def testTextEntry(self):
    # Add an input box to the page.
    self._tab.ExecuteJavaScript(
        '(function() {'
        '  var elem = document.createElement("textarea");'
        '  document.body.appendChild(elem);'
        '  elem.focus();'
        '})();')

    # Simulate typing a sentence.
    for char in 'Hello, World!':
      self._PressKey(char)

    # Make changes to the sentence using special keys.
    for _ in range(6):
      self._PressKey('ArrowLeft')
    self._PressKey('Backspace')
    self._PressKey('Return')

    # Check that the contents of the textarea is correct. It might take a second
    # until all keystrokes have been handled by the browser (crbug.com/630017).
    self._tab.WaitForJavaScriptCondition(
        'document.querySelector("textarea").value === "Hello,\\nWorld!"',
        timeout=1)

  def testPressUnknownKey(self):
    with self.assertRaises(ValueError):
      self._PressKey('UnknownKeyName')
