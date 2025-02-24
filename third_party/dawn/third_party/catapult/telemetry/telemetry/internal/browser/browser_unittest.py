# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import re
import tempfile
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.internal.browser import browser as browser_module
from telemetry.internal.browser import browser_finder
from telemetry.internal.platform import gpu_device
from telemetry.internal.platform import gpu_info
from telemetry.internal.platform import system_info
from telemetry.testing import browser_test_case
from telemetry.testing import options_for_unittests

from devil.android import app_ui


class IntentionalException(Exception):
  pass


class BrowserTest(browser_test_case.BrowserTestCase):
  def testBrowserCreation(self):
    self.assertLess(0, len(self._browser.tabs))
    self.assertGreater(3, len(self._browser.tabs))

    # Different browsers boot up to different things.
    assert self._browser.tabs[0].url

  def testTypExpectationsTagsIncludesBrowserTypeTag(self):
    with mock.patch.object(
        self._browser.__class__, 'browser_type', 'reference_debug'):
      self.assertIn('reference-debug', self._browser.GetTypExpectationsTags())

  @decorators.Enabled('has tabs')
  def testNewCloseTab(self):
    existing_tab = self._browser.tabs[0]
    num_tabs = len(self._browser.tabs)
    self.assertLess(0, num_tabs)
    self.assertGreater(3, num_tabs)
    existing_tab_url = existing_tab.url

    new_tab = self._browser.tabs.New()
    self.assertEqual(num_tabs + 1, len(self._browser.tabs))
    self.assertEqual(existing_tab.url, existing_tab_url)
    self.assertEqual(new_tab.url, 'about:blank')

    new_tab.Close()
    self.assertEqual(num_tabs, len(self._browser.tabs))
    self.assertEqual(existing_tab.url, existing_tab_url)

  def testMultipleTabCalls(self):
    self._browser.tabs[0].Navigate(self.UrlOfUnittestFile('blank.html'))
    self._browser.tabs[0].WaitForDocumentReadyStateToBeInteractiveOrBetter()

  def testTabCallByReference(self):
    tab = self._browser.tabs[0]
    tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    self._browser.tabs[0].WaitForDocumentReadyStateToBeInteractiveOrBetter()

  @decorators.Enabled('has tabs')
  def testCloseReferencedTab(self):
    num_initial_tabs = len(self._browser.tabs)
    self._browser.tabs.New()
    tab = self._browser.tabs[0]
    tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    tab.Close()
    self.assertEqual(num_initial_tabs, len(self._browser.tabs))

  @decorators.Enabled('has tabs')
  def testForegroundTab(self):
    num_tabs = len(self._browser.tabs)
    self.assertLess(0, num_tabs)
    self.assertGreater(3, num_tabs)
    # The first tab is the foreground tab by default.
    original_tab = self._browser.tabs[0]
    self.assertEqual(self._browser.foreground_tab, original_tab)
    # If there are 2 tabs open, close the inactive one before creating a new
    # tab. This ensures the new tab will regain the foreground when we close
    # the original tab after re-activating it.
    if num_tabs == 2:
      self._browser.tabs[1].Close()
    new_tab = self._browser.tabs.New()
    # New tab shouls be foreground tab
    self.assertEqual(self._browser.foreground_tab, new_tab)
    # Make sure that activating the background tab makes it the foreground tab
    original_tab.Activate()
    self.assertEqual(self._browser.foreground_tab, original_tab)
    # Closing the current foreground tab should switch the foreground tab to the
    # other tab
    original_tab.Close()
    self.assertEqual(self._browser.foreground_tab, new_tab)

  def testGetSystemInfo(self):
    info = self._browser.GetSystemInfo()
    if not info:
      logging.warning(
          'Browser does not support getting system info, skipping test.')
      return

    self.assertTrue(isinstance(info, system_info.SystemInfo))
    self.assertTrue(hasattr(info, 'model_name'))
    self.assertTrue(hasattr(info, 'gpu'))
    self.assertTrue(isinstance(info.gpu, gpu_info.GPUInfo))
    self.assertTrue(hasattr(info.gpu, 'devices'))
    self.assertTrue(len(info.gpu.devices) > 0)
    for g in info.gpu.devices:
      self.assertTrue(isinstance(g, gpu_device.GPUDevice))

  def testGetSystemInfoNotCachedObject(self):
    info_a = self._browser.GetSystemInfo()
    if not info_a:
      logging.warning(
          'Browser does not support getting system info, skipping test.')
      return
    info_b = self._browser.GetSystemInfo()
    self.assertFalse(info_a is info_b)

  @decorators.Disabled('mac') # https://crbug.com/1286458
  def testSystemInfoModelNameOnMac(self):
    if self._browser.platform.GetOSName() != 'mac':
      self.skipTest('This test is only run on macOS')
      return

    info = self._browser.GetSystemInfo()
    if not info:
      logging.warning(
          'Browser does not support getting system info, skipping test.')
      return

    model_name_re = r"[a-zA-Z]* [0-9.]*"
    self.assertNotEqual(
        re.match(model_name_re, info.model_name),
        None,
        'Invalid Mac model name: %s' % info.model_name)

  @decorators.Enabled('android')
  def testGetAppUi(self):
    self.assertTrue(self._browser.supports_app_ui_interactions)
    ui = self._browser.GetAppUi()
    self.assertTrue(isinstance(ui, app_ui.AppUi))
    self.assertIsNotNone(ui.WaitForUiNode(resource_id='action_bar_root'))


class CommandLineBrowserTest(browser_test_case.BrowserTestCase):
  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.AppendExtraBrowserArgs('--user-agent=telemetry')

  @decorators.Disabled('system-guest', 'cros-chrome-guest')  # crbug.com/985125
  def testCommandLineOverriding(self):
    # This test starts the browser with --user-agent=telemetry. This tests
    # whether the user agent is then set.
    t = self._browser.tabs[0]
    t.Navigate(self.UrlOfUnittestFile('blank.html'))
    t.WaitForDocumentReadyStateToBeInteractiveOrBetter()
    self.assertEqual(t.EvaluateJavaScript('navigator.userAgent'), 'telemetry')


class DirtyProfileBrowserTest(browser_test_case.BrowserTestCase):
  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.profile_type = 'small_profile'

  @decorators.Disabled('chromeos')  # crbug.com/243912
  def testDirtyProfileCreation(self):
    self.assertEqual(1, len(self._browser.tabs))


class BrowserLoggingTest(browser_test_case.BrowserTestCase):
  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.logging_verbosity = options.VERBOSE_LOGGING

  @decorators.Disabled('chromeos', 'android')
  def testLogFileExist(self):
    self.assertTrue(
        os.path.isfile(self._browser._browser_backend.log_file_path))


class BrowserCreationTest(unittest.TestCase):
  def setUp(self):
    self.mock_browser_backend = mock.MagicMock()
    self.mock_platform_backend = mock.MagicMock()
    self.fake_startup_args = ['--foo', '--bar=2']

  def testCleanedUpCalledWhenExceptionRaisedInBrowserCreation(self):
    self.mock_browser_backend.SetBrowser.side_effect = (
        IntentionalException('Boom!'))
    with self.assertRaises(IntentionalException):
      browser_module.Browser(
          self.mock_browser_backend, self.mock_platform_backend,
          self.fake_startup_args)
    self.assertTrue(self.mock_browser_backend.Close.called)

  def testOriginalExceptionNotSwallow(self):
    self.mock_browser_backend.SetBrowser.side_effect = (
        IntentionalException('Boom!'))
    self.mock_platform_backend.WillCloseBrowser.side_effect = (
        IntentionalException('Cannot close browser!'))
    with self.assertRaises(IntentionalException) as context:
      browser_module.Browser(
          self.mock_browser_backend, self.mock_platform_backend,
          self.fake_startup_args)
    self.assertIn('Boom!', repr(context.exception))


class TestBrowserCreation(unittest.TestCase):

  def setUp(self):
    self.finder_options = options_for_unittests.GetCopy()
    self.browser_to_create = browser_finder.FindBrowser(self.finder_options)
    self.browser_to_create.platform.network_controller.Open()

  @property
  def browser_options(self):
    return self.finder_options.browser_options

  def tearDown(self):
    self.browser_to_create.platform.network_controller.Close()

  def testCreateWithBrowserSession(self):
    with self.browser_to_create.BrowserSession(self.browser_options) as browser:
      tab = browser.tabs.New()
      tab.Navigate('about:blank')
      self.assertEqual(2, tab.EvaluateJavaScript('1 + 1'))

  def testCreateWithBadOptionsRaises(self):
    with self.assertRaises(AssertionError):
      # It's an error to pass finder_options instead of browser_options.
      with self.browser_to_create.BrowserSession(self.finder_options):
        pass  # Do nothing.

  @decorators.Disabled('chromeos')  # crbug.com/1014115
  def testCreateBrowserTwice(self):
    try:
      self.browser_to_create.SetUpEnvironment(self.browser_options)
      for _ in range(2):
        browser = self.browser_to_create.Create()
        tab = browser.tabs.New()
        tab.Navigate('about:blank')
        self.assertEqual(2, tab.EvaluateJavaScript('1 + 1'))
        browser.Close()
    finally:
      self.browser_to_create.CleanUpEnvironment()

  @decorators.Enabled('linux')
  # TODO(crbug.com/782691): enable this on Win
  # TODO(ashleymarie): Re-enable on mac (BUG=catapult:#3523)
  @decorators.Isolated
  def testBrowserNotLeakingTempFiles(self):
    before_browser_run_temp_dir_content = os.listdir(tempfile.tempdir)
    with self.browser_to_create.BrowserSession(self.browser_options) as browser:
      tab = browser.tabs.New()
      tab.Navigate('about:blank')
      self.assertEqual(2, tab.EvaluateJavaScript('1 + 1'))
    after_browser_run_temp_dir_content = os.listdir(tempfile.tempdir)
    self.assertEqual(before_browser_run_temp_dir_content,
                     after_browser_run_temp_dir_content)
