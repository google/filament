# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.actions import action_runner
from telemetry.internal.browser import web_contents

DEFAULT_TAB_TIMEOUT = 60


class Tab(web_contents.WebContents):
  """Represents a tab in the browser

  The important parts of the Tab object are in the runtime and page objects.
  E.g.:
      # Navigates the tab to a given url.
      tab.Navigate('http://www.google.com/')

      # Evaluates 1+1 in the tab's JavaScript context.
      tab.Evaluate('1+1')
  """
  def __init__(self, inspector_backend, tab_list_backend, browser):
    super().__init__(inspector_backend)
    self._tab_list_backend = tab_list_backend
    self._browser = browser
    self._action_runner = action_runner.ActionRunner(self)

  @property
  def browser(self):
    """The browser in which this tab resides."""
    return self._browser

  @property
  def action_runner(self):
    return self._action_runner

  @property
  def url(self):
    """Returns the URL of the tab, as reported by devtools.

    Raises:
      devtools_http.DevToolsClientConnectionError
    """
    return self._inspector_backend.url

  @property
  def dom_stats(self):
    """A dictionary populated with measured DOM statistics.

    Currently this dictionary contains:
    {
      'document_count': integer,
      'node_count': integer,
      'event_listener_count': integer
    }

    Raises:
      inspector_memory.InspectorMemoryException
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    dom_counters = self._inspector_backend.GetDOMStats(
        timeout=DEFAULT_TAB_TIMEOUT)
    assert (len(dom_counters) == 3 and
            all(x in dom_counters for x in ['document_count', 'node_count',
                                             'event_listener_count']))
    return dom_counters

  def PrepareForLeakDetection(self):
    self._inspector_backend.PrepareForLeakDetection(
        timeout=DEFAULT_TAB_TIMEOUT)

  def Activate(self):
    """Brings this tab to the foreground asynchronously.

    Not all browsers or browser versions support this method.
    Be sure to check browser.supports_tab_control.

    Raises:
      devtools_http.DevToolsClientConnectionError
      devtools_client_backend.TabNotFoundError
      tab_list_backend.TabUnexpectedResponseException
    """
    self._tab_list_backend.ActivateTab(self.id)

  def Close(self, keep_one=True, timeout=300):
    """Closes this tab.

    Not all browsers or browser versions support this method.
    Be sure to check browser.supports_tab_control.

    Args:
      keep_one: Whether to make sure to keep one tab open. On some platforms
        closing the last tab causes the browser to be closed, to prevent this
        the default is to open a new tab before closing the last one.

    Raises:
      devtools_http.DevToolsClientConnectionError
      devtools_client_backend.TabNotFoundError
      tab_list_backend.TabUnexpectedResponseException
      exceptions.TimeoutException
    """
    if keep_one and len(self._tab_list_backend) <= 1:
      self._tab_list_backend.New(in_new_window=False, timeout=timeout, url=None)
    self._tab_list_backend.CloseTab(self.id, timeout)

  @property
  def screenshot_supported(self):
    """True if the browser instance is capable of capturing screenshots."""
    return self._inspector_backend.screenshot_supported

  def Screenshot(self, timeout=DEFAULT_TAB_TIMEOUT):
    """Capture a screenshot of the tab's visible contents.

    Returns:
      A telemetry.core.Bitmap.
    Raises:
      exceptions.WebSocketDisconnected
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.Screenshot(timeout)

  def FullScreenshot(self, timeout=DEFAULT_TAB_TIMEOUT):
    """Capture a screenshot of the tab's full contents.

    Returns:
      A telemetry.core.Bitmap.
    Raises:
      exceptions.WebSocketDisconnected
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.FullScreenshot(timeout)

  def CollectGarbage(self):
    """Forces a garbage collection.

    Raises:
      exceptions.WebSocketDisconnected
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    self._inspector_backend.CollectGarbage()

  def ClearCache(self, force):
    """Clears the browser's networking related disk, memory and other caches.

    Args:
      force: Iff true, navigates to about:blank which destroys the previous
          renderer, ensuring that even "live" resources in the memory cache are
          cleared.

    Raises:
      exceptions.EvaluateException
      exceptions.WebSocketDisconnected
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
      errors.DeviceUnresponsiveError
    """
    self.browser.platform.FlushDnsCache()
    self.ExecuteJavaScript("""
        if (window.chrome && chrome.benchmarking &&
            chrome.benchmarking.clearCache) {
          chrome.benchmarking.clearCache();
          chrome.benchmarking.clearPredictorCache();
          chrome.benchmarking.clearHostResolverCache();
        }
    """)
    if force:
      self.Navigate('about:blank')

  def ClearDataForOrigin(self, url, timeout=DEFAULT_TAB_TIMEOUT):
    """Clears storage data for the origin of url.

    With assigning 'all' to params.storageTypes, Storage.clearDataForOrigin
    clears all storage of app cache, cookies, file systems, indexed db,
    local storage, shader cache, web sql, service workers and cache storage.
    See StorageHandler::ClearDataForOrigin() for more details.

    Raises:
      exceptions.StoryActionError
    """
    return self._inspector_backend.ClearDataForOrigin(url, timeout)

  def StopAllServiceWorkers(self, timeout=DEFAULT_TAB_TIMEOUT):
    """Stops all service workers.

    Raises:
      exceptions.StoryActionError
    """
    return self._inspector_backend.StopAllServiceWorkers(timeout)

  def EnableSharedStorageNotifications(self, timeout=DEFAULT_TAB_TIMEOUT):
    """Enables shared storage notifications from the DevTools Protocol for
    this tab.

    Raises:
      exceptions.StoryActionError
      AssertionError
    """
    self._inspector_backend.EnableSharedStorageNotifications(timeout)

  def DisableSharedStorageNotifications(self, timeout=DEFAULT_TAB_TIMEOUT):
    """Disables shared storage notifications from the DevTools Protocol for
    this tab.

    Raises:
      exceptions.StoryActionError
      AssertionError
    """
    self._inspector_backend.DisableSharedStorageNotifications(timeout)

  @property
  def shared_storage_notifications(self):
    """
    Returns:
      list of params objects (i.e. `msg['params']`) from all shared storage
      event notifications `msg` that have arrived for this tab since shared
      storage tracking was enabled and notifications were last cleared.
    """
    return self._inspector_backend.shared_storage_notifications

  def ClearSharedStorageNotifications(self):
    """Clears any previously received shared storage otifications for this tab.
    """
    self._inspector_backend.ClearSharedStorageNotifications()

  @property
  def shared_storage_notifications_enabled(self):
    """
    Returns:
      boolean denoting whether shared storage event notifications are enabled.
    """
    return self._inspector_backend.shared_storage_notifications_enabled

  def GetSharedStorageMetadata(self, origin, timeout=DEFAULT_TAB_TIMEOUT):
    return self._inspector_backend.GetSharedStorageMetadata(origin=origin,
                                                  timeout=timeout)

  def GetSharedStorageEntries(self, origin, timeout=DEFAULT_TAB_TIMEOUT):
    return self._inspector_backend.GetSharedStorageEntries(origin=origin,
                                                  timeout=timeout)

  def WaitForSharedStorageEvents(self,
                                 expected_events,
                                 mode='strict',
                                 timeout=DEFAULT_TAB_TIMEOUT):
    """Wait for list of expected Shared Storage notifications to be received.

    Example:
      event_list = [
        {'type': 'documentAppend', 'params': {'key': 'a', 'value': 'b'}},
        {'type': 'documentDelete', 'params': {'key': 'a'}},
      ]
      runner.WaitForSharedStorageEvents(event_list)

    Args:
      expected_events: The expected event list, provided as a list of
        dictionaries of event params.
          Note that for an expected event and an actually received event to
          match, the actually received event must contain all of the key-value
          pairs listed in the expected event, but the actually received event
          may also contain additional keys that were not listed.
      mode: 'strict' or 'relaxed'.
        - 'strict': expected events must exactly match received events in terms
          of params listed (although any unlisted parameters can differ) and
          order
        - 'relaxed': expected events must exactly match some sublist of received
          events in terms of params listed (although any unlisted parameters
          can differ) and order. Additional events are allowed to be received
          that were not expected.
      timeout: The timeout for waiting for event(s).

    Returns:
      A boolean denoting whether or not the expected event(s) were received
      within the timeout.

    Raises:
      py_utils.TimeoutException
      exceptions.StoryActionError
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.WaitForSharedStorageEvents(expected_events,
                                                              mode,
                                                              timeout)
