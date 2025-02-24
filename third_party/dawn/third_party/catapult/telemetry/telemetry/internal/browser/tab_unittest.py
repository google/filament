# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json
import logging
import time

import six.moves.urllib.parse # pylint: disable=import-error

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.browser.web_contents import ServiceWorkerState
from telemetry.testing import tab_test_case
from telemetry.util import image_util

import py_utils


def _IsDocumentVisible(tab):
  return not tab.EvaluateJavaScript('document.hidden || document.webkitHidden')

def GetMilestoneNumber(browser):
  version_info = browser.GetVersionInfo()
  version = version_info.get('Browser', 'Chrome/0.0.0.0')
  slash = version.find('/')
  if slash < 0:
    return 0
  dot = version.find('.', slash + 2)
  if dot < 0:
    return 0
  try:
    milestone = int(version[slash + 1:dot])
    return milestone
  except ValueError:
    return 0


class TabTest(tab_test_case.TabTestCase):
  def testNavigateAndWaitForCompleteState(self):
    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    self._tab.WaitForDocumentReadyStateToBeComplete()

  def testNavigateAndWaitForInteractiveState(self):
    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    self._tab.WaitForDocumentReadyStateToBeInteractiveOrBetter()

  def testTabBrowserIsRightBrowser(self):
    self.assertEqual(self._tab.browser, self._browser)

  def testRendererCrash(self):
    self.assertRaises(exceptions.DevtoolsTargetCrashException,
                      lambda: self._tab.Navigate('chrome://crash',
                                                 timeout=30))
    # This is expected to produce a single minidump, so ignore it so that the
    # post-test cleanup doesn't complain about unsymbolized minidumps.
    minidumps = self._tab.browser.GetAllMinidumpPaths()
    if len(minidumps) == 1:
      # If we don't have a minidump, no need to do anything. If we have more
      # than one, then we should leave them alone and let the cleanup fail,
      # as that implies that something went wrong and we currently don't have
      # a good way to distinguish the expected minidump from unexpected ones.
      self._tab.browser.IgnoreMinidump(minidumps[0])

  def testTimeoutExceptionIncludeConsoleMessage(self):
    self._tab.EvaluateJavaScript("""
        window.__set_timeout_called = false;
        function buggyReference() {
          window.__set_timeout_called = true;
          if (window.__one.not_defined === undefined)
             window.__one = 1;
        }
        setTimeout(buggyReference, 200);""")
    self._tab.WaitForJavaScriptCondition(
        'window.__set_timeout_called === true', timeout=5)
    with self.assertRaises(py_utils.TimeoutException) as context:
      self._tab.WaitForJavaScriptCondition(
          'window.__one === 1', timeout=1)
      self.assertIn(
          ("(error) :5: Uncaught TypeError: Cannot read property 'not_defined' "
           'of undefined\n'),
          context.exception.message)

  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos') # https://crbug.com/947675
  def testActivateTab(self):
    py_utils.WaitFor(lambda: _IsDocumentVisible(self._tab), timeout=5)
    new_tab = self._browser.tabs.New()
    new_tab.Navigate('about:blank')
    py_utils.WaitFor(lambda: _IsDocumentVisible(new_tab), timeout=5)
    self.assertFalse(_IsDocumentVisible(self._tab))
    self._tab.Activate()
    py_utils.WaitFor(lambda: _IsDocumentVisible(self._tab), timeout=5)
    self.assertFalse(_IsDocumentVisible(new_tab))

  def testTabUrl(self):
    self.assertEqual(self._tab.url, 'about:blank')
    url = self.UrlOfUnittestFile('blank.html')
    self._tab.Navigate(url)
    self.assertEqual(self._tab.url, url)

  @decorators.Disabled('android') # https://crbug.com/463933
  @decorators.Disabled('all') # Temporary disabled for Chromium changes
  def testTabIsAlive(self):
    self.assertEqual(self._tab.url, 'about:blank')
    self.assertTrue(self._tab.IsAlive())

    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    self.assertTrue(self._tab.IsAlive())

    self.assertRaises(
        exceptions.DevtoolsTargetCrashException,
        lambda: self._tab.Navigate(self.UrlOfUnittestFile('chrome://crash')))
    self.assertFalse(self._tab.IsAlive())


class GpuTabTest(tab_test_case.TabTestCase):
  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.AppendExtraBrowserArgs('--enable-gpu-benchmarking')

  # Test flaky on mac: crbug.com/358664, chromeos: crbug.com/483212.
  @decorators.Disabled('android', 'mac', 'chromeos')
  @decorators.Disabled('win')  # catapult/issues/2282
  def testScreenshot(self):
    if not self._tab.screenshot_supported:
      logging.warning('Browser does not support screenshots, skipping test.')
      return

    self.Navigate('green_rect.html')
    pixel_ratio = self._tab.EvaluateJavaScript('window.devicePixelRatio || 1')

    screenshot = self._tab.Screenshot(5)
    assert screenshot is not None
    image_util.GetPixelColor(
        screenshot, 0 * pixel_ratio, 0 * pixel_ratio).AssertIsRGB(
            0, 255, 0, tolerance=2)
    image_util.GetPixelColor(
        screenshot, 31 * pixel_ratio, 31 * pixel_ratio).AssertIsRGB(
            0, 255, 0, tolerance=2)
    image_util.GetPixelColor(
        screenshot, 32 * pixel_ratio, 32 * pixel_ratio).AssertIsRGB(
            255, 255, 255, tolerance=2)


class ServiceWorkerTabTest(tab_test_case.TabTestCase):
  def testIsServiceWorkerActivatedOrNotRegistered(self):
    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    py_utils.WaitFor(self._tab.IsServiceWorkerActivatedOrNotRegistered,
                     timeout=10)
    self.assertEqual(self._tab._GetServiceWorkerState(),
                     ServiceWorkerState.NOT_REGISTERED)
    self._tab.ExecuteJavaScript(
        'navigator.serviceWorker.register("{{ @scriptURL }}");',
        scriptURL=self.UrlOfUnittestFile('blank.js'))
    py_utils.WaitFor(self._tab.IsServiceWorkerActivatedOrNotRegistered,
                     timeout=10)
    self.assertEqual(self._tab._GetServiceWorkerState(),
                     ServiceWorkerState.ACTIVATED)

  def testClearDataForOrigin(self):
    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    self._tab.ExecuteJavaScript(
        ('var asyncOperationDone = false;'
         'var isServiceWorkerRegisteredForThisOrigin = false;'
         'navigator.serviceWorker.register("{{ @scriptURL }}").then(_ => {'
         'asyncOperationDone = true; });'),
        scriptURL=self.UrlOfUnittestFile('blank.js'))
    self._tab.WaitForJavaScriptCondition('asyncOperationDone')
    check_registration = 'asyncOperationDone = false; \
        isServiceWorkerRegisteredForThisOrigin = false; \
        navigator.serviceWorker.getRegistration().then( \
            (reg) => { \
                asyncOperationDone = true; \
                isServiceWorkerRegisteredForThisOrigin = reg ? true : false;});'
    self._tab.ExecuteJavaScript(check_registration)
    self._tab.WaitForJavaScriptCondition('asyncOperationDone')
    self.assertTrue(self._tab.EvaluateJavaScript(
        'isServiceWorkerRegisteredForThisOrigin;'))
    py_utils.WaitFor(self._tab.IsServiceWorkerActivatedOrNotRegistered,
                     timeout=10)
    self._tab.ClearDataForOrigin(self.UrlOfUnittestFile(''))
    time.sleep(1)
    self._tab.ExecuteJavaScript(check_registration)
    self._tab.WaitForJavaScriptCondition('asyncOperationDone')
    self.assertFalse(self._tab.EvaluateJavaScript(
        'isServiceWorkerRegisteredForThisOrigin;'))


class SharedStorageTabTest(tab_test_case.TabTestCase):
  def __init__(self, *args):
    super().__init__(*args)
    self._shared_storage_testable = False

  def setUp(self):
    super().setUp()
    self._shared_storage_testable = False
    self._tab.Navigate(self.UrlOfUnittestFile('blank.html'))
    enabled = self._tab.EvaluateJavaScript('Boolean(window.sharedStorage)')
    if not enabled:
      # Shared Storage is not enabled. The browser used may be too old.
      milestone = GetMilestoneNumber(self._browser)
      if milestone < 94:
        logging.warning('Browser does not support shared storage, '
                        + 'skipping test.')
        return
      version_info = str(self._browser.GetVersionInfo())
      message = "Shared Storage is not enabled. " + version_info
      raise exceptions.StoryActionError(message)
    try:
      self._tab.EnableSharedStorageNotifications()
    except exceptions.StoryActionError as sae:
      if "'Storage.setSharedStorageTracking' wasn't found" in str(sae):
        # Shared Storage tracking in DevTools may not be implemented in this
        # version of the browser.
        milestone = GetMilestoneNumber(self._browser)
        if milestone < 109:
          logging.warning('Browser does not support shared storage tracking, '
                        + 'skipping test.')
          return
        version_info = str(self._browser.GetVersionInfo())
        message = "Shared Storage tracking is not enabled. " + version_info
        raise exceptions.StoryActionError(message)
      raise sae
    try:
      self._tab.EvaluateJavaScript("window.sharedStorage.set('test', 'set')",
                               promise=True)
    except exceptions.EvaluateException as ee:
      if 'sharedStorage is disabled' in str(ee):
        version_info = str(self._browser.GetVersionInfo())
        logging.warning(''.join(['JavaScript exception while trying to use ',
                                 'shared storage indicated it is disabled; ',
                                 'skipping test.\n', version_info,
                                 '\n', repr(ee)]))
        return
    self._shared_storage_testable = True
    self._tab.ClearSharedStorageNotifications()

  def tearDown(self):
    if self._shared_storage_testable:
      self._tab.DisableSharedStorageNotifications()
      self.assertFalse(self._tab.shared_storage_notifications_enabled)
      self._tab.ClearSharedStorageNotifications()
    super().tearDown()

  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.AppendExtraBrowserArgs([
      ''.join(['--enable-features=',
               'SharedStorageAPI:ExposeDebugMessageForSettingsStatus/true,',
               'FencedFrames:implementation_type/mparch,',
               'FencedFramesDefaultMode,',
               'PrivacySandboxAdsAPIsOverride,',
               'DefaultAllowPrivacySandboxAttestations']),
      '--enable-privacy-sandbox-ads-apis'
    ])

  @property
  def origin(self):
    parse_result = six.moves.urllib.parse.urlparse(self._tab.url)
    return '://'.join([parse_result[0], parse_result[1]])

  def VerifyMetadata(self, metadata, expected_length=1,
                     expected_remaining_budget=12):
    self.assertIsInstance(metadata, dict)
    self.assertIn("creationTime", metadata)
    self.assertIn("length", metadata)
    self.assertIn("remainingBudget", metadata)
    self.assertEqual(metadata.get("length"), expected_length)
    self.assertEqual(metadata.get("remainingBudget"), expected_remaining_budget)

  def VerifyEntries(self, entries, expected_entries=None):
    # `expected_entries` must be provided as a list.
    self.assertIsInstance(expected_entries, list)

    self.assertIsInstance(entries, list)
    self.assertEqual(len(entries), len(expected_entries))
    entries_json = sorted([json.dumps(entry) for entry in entries])
    expected_json = sorted([json.dumps(entry) for entry in expected_entries])
    self.assertEqual(entries_json, expected_json)

  def testWaitForSharedStorageEventsStrict_Passes(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('a', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.append('c', 'd')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.delete('a')",
                               promise=True)
    expected_events = [{
        'type': 'documentSet',
        'params': {
            'key': 'a',
            'value': 'b',
            'ignoreIfPresent': False
        }
    }, {
        'type': 'documentAppend',
        'params': {
            'key': 'c',
            'value': 'd'
        }
    }, {
        'type': 'documentDelete'
    }]
    self._tab.WaitForSharedStorageEvents(expected_events, mode='strict')

  def testWaitForSharedStorageEventsStrict_Fails(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('a', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.delete('a')",
                               promise=True)
    expected_events = [{
        'type': 'documentDelete'
    }, {
        'type': 'documentSet',
        'params': {
            'key': 'a',
            'value': 'b',
            'ignoreIfPresent': False
        }
    }]
    with self.assertRaises(py_utils.TimeoutException):
      self._tab.WaitForSharedStorageEvents(expected_events, mode='strict',
                                         timeout=10)

  def testWaitForSharedStorageEventsRelaxed_Passes(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('a', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.append('c', 'd')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.delete('a')",
                               promise=True)
    expected_events = [{'type': 'documentAppend'},
                       {'type': 'documentDelete',
                        'params': {'key': 'a'}}]
    self._tab.WaitForSharedStorageEvents(expected_events, mode='relaxed')

  def testWaitForSharedStorageEventsRelaxed_Fails(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('a', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.delete('a')",
                               promise=True)
    expected_events = [{
        'type': 'documentSet',
        'params': {
            'key': 'a',
            'value': 'b',
            'ignoreIfPresent': False
        }
    }, {
        'type': 'documentDelete',
        'params': {
            'key': 'c'
        }
    }]
    with self.assertRaises(py_utils.TimeoutException):
      self._tab.WaitForSharedStorageEvents(expected_events, mode='relaxed',
                                         timeout=10)

  def testGetSharedStorageMetadata_Simple(self):
    if not self._shared_storage_testable:
      return

    metadata = self._tab.GetSharedStorageMetadata(self.origin)
    self.VerifyMetadata(metadata, expected_length=1,
                        expected_remaining_budget=12)

  def testGetSharedStorageEntries_Simple(self):
    if not self._shared_storage_testable:
      return

    entries = self._tab.GetSharedStorageEntries(self.origin)
    self.VerifyEntries(entries,
                       expected_entries=[{'key': 'test', 'value': 'set'}])

  def testGetSharedStorageMetadata_SetAdditional(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('z', 'a')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.append('y', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript(
        "window.sharedStorage.set('x', 'c', {ignoreIfPresent: true})",
        promise=True)
    expected_events = [{
        'type': 'documentSet',
        'params': {
            'key': 'z',
            'value': 'a',
            'ignoreIfPresent': False
        }
    }, {
        'type': 'documentAppend',
        'params': {
            'key': 'y',
            'value': 'b'
        }
    }, {
        'type': 'documentSet',
        'params': {
            'key': 'x',
            'value': 'c',
            'ignoreIfPresent': True
        }
    }]
    self._tab.WaitForSharedStorageEvents(expected_events, mode='strict')

    metadata = self._tab.GetSharedStorageMetadata(self.origin)
    self.VerifyMetadata(metadata, expected_length=4,
                        expected_remaining_budget=12)

  def testGetSharedStorageEntries_SetAdditional(self):
    if not self._shared_storage_testable:
      return

    self.assertTrue(self._tab.shared_storage_notifications_enabled)
    self._tab.EvaluateJavaScript("window.sharedStorage.set('z', 'a')",
                               promise=True)
    self._tab.EvaluateJavaScript("window.sharedStorage.append('y', 'b')",
                               promise=True)
    self._tab.EvaluateJavaScript(
        "window.sharedStorage.set('x', 'c', {ignoreIfPresent: true})",
        promise=True)
    expected_events = [{
        'type': 'documentSet',
        'params': {
            'key': 'z',
            'value': 'a',
            'ignoreIfPresent': False
        }
    }, {
        'type': 'documentAppend',
        'params': {
            'key': 'y',
            'value': 'b'
        }
    }, {
        'type': 'documentSet',
        'params': {
            'key': 'x',
            'value': 'c',
            'ignoreIfPresent': True
        }
    }]
    self._tab.WaitForSharedStorageEvents(expected_events, mode='strict')

    entries = self._tab.GetSharedStorageEntries(self.origin)
    self.VerifyEntries(entries,
                       expected_entries=[{'key': 'test', 'value': 'set'},
                                         {'key': 'z', 'value': 'a'},
                                         {'key': 'y', 'value': 'b'},
                                         {'key': 'x', 'value': 'c'}])

  def testGetSharedStorageMetadata_NotFound(self):
    if not self._shared_storage_testable:
      return

    origin_not_using_shared_storage = 'https://google.com'
    metadata = self._tab.GetSharedStorageMetadata(
      origin_not_using_shared_storage)
    self.VerifyMetadata(metadata, expected_length=0,
                        expected_remaining_budget=None)

  def testGetSharedStorageEntries_NotFound(self):
    if not self._shared_storage_testable:
      return

    origin_not_using_shared_storage = 'https://google.com'
    entries = self._tab.GetSharedStorageEntries(
      origin_not_using_shared_storage)
    self.VerifyEntries(entries, expected_entries=[])
