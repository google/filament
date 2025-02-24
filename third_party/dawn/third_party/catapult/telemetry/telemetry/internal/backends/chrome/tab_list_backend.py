# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.core import exceptions
from telemetry.internal.backends.chrome_inspector import inspector_backend_list
from telemetry.internal.browser import tab

import py_utils


class TabUnexpectedResponseException(exceptions.DevtoolsTargetCrashException):
  pass


class TabListBackend(inspector_backend_list.InspectorBackendList):
  """A dynamic sequence of tab.Tabs in UI order."""

  def New(self, in_new_window, timeout, url):
    """Makes a new tab of specified type.

    Args:
      in_new_window: If True, opens the tab in a popup window. Otherwise, opens
        in current window.
      timeout: Seconds to wait for the new tab request to complete.

    Returns:
      The Tab object of the successfully created tab.

    Raises:
      devtools_http.DevToolsClientConnectionError
      exceptions.EvaluateException: for the current implementation of opening
        a tab in a new window.
    """
    if not self._browser_backend.supports_tab_control:
      raise NotImplementedError("Browser doesn't support tab control.")
    response = self._browser_backend.devtools_client.RequestNewTab(
        timeout, in_new_window=in_new_window, url=url)
    if 'error' in response:
      raise TabUnexpectedResponseException(
          app=self._browser_backend.browser,
          msg='Received response: %s' % response)
    last_exception = None

    def GetTabBackend():
      nonlocal last_exception
      try:
        return self.GetBackendFromContextId(response['result']['targetId'])
      except KeyError as e:
        last_exception = e
        return None

    try:
      return py_utils.WaitFor(GetTabBackend, 1)
    except py_utils.TimeoutException:
      raise TabUnexpectedResponseException(
        app=self._browser_backend.browser,
        msg='Received response: %s' % response) from last_exception


  def CloseTab(self, tab_id, timeout=300):
    """Closes the tab with the given debugger_url.

    Raises:
      devtools_http.DevToolsClientConnectionError
      devtools_client_backend.TabNotFoundError
      TabUnexpectedResponseException
      py_utils.TimeoutException
    """
    assert self._browser_backend.supports_tab_control

    response = self._browser_backend.devtools_client.CloseTab(tab_id, timeout)

    if response not in ('Target is closing', b'Target is closing'):
      raise TabUnexpectedResponseException(
          app=self._browser_backend.browser,
          msg='Received response: %s' % response)

    py_utils.WaitFor(lambda: tab_id not in self.IterContextIds(), timeout=5)

  def ActivateTab(self, tab_id, timeout=30):
    """Activates the tab with the given debugger_url.

    Raises:
      devtools_http.DevToolsClientConnectionError
      devtools_client_backend.TabNotFoundError
      TabUnexpectedResponseException
    """
    assert self._browser_backend.supports_tab_control

    response = self._browser_backend.devtools_client.ActivateTab(tab_id,
                                                                 timeout)

    if response not in ('Target activated', b'Target activated'):
      raise TabUnexpectedResponseException(
          app=self._browser_backend.browser,
          msg='Received response: %s' % response)

    # Activate tab call is synchronous, so wait to make sure that Chrome
    # have time to promote this tab to foreground.
    py_utils.WaitFor(
        lambda: tab_id == self._browser_backend.browser.foreground_tab.id,
        timeout=5)

  def Get(self, index, ret):
    """Returns self[index] if it exists, or ret if index is out of bounds."""
    if len(self) <= index:
      return ret
    return self[index]

  def ShouldIncludeContext(self, context):
    if 'type' in context:
      return (context['type'] == 'page' or
              context['url'] == 'chrome://media-router/' or
              (self._browser_backend.browser.supports_inspecting_webui and
               context['url'].startswith('chrome://')))
    # TODO: For compatibility with Chrome before r177683.
    # This check is not completely correct, see crbug.com/190592.
    return not context['url'].startswith('chrome-extension://')

  def CreateWrapper(self, inspector_backend_instance):
    return tab.Tab(inspector_backend_instance, self,
                   self._browser_backend.browser)

  def _HandleDevToolsConnectionError(self, error):
    if not self._browser_backend.IsAppRunning():
      error.AddDebuggingMessage('The browser is not running. It probably '
                                'crashed.')
    elif not self._browser_backend.HasDevToolsConnection():
      error.AddDebuggingMessage('The browser exists but cannot be reached.')
    else:
      error.AddDebuggingMessage('The browser exists and can be reached. '
                                'The devtools target probably crashed.')
