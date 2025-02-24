# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import re
import socket

from py_utils import exc_util
from py_utils import retry_util
from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends import browser_backend
from telemetry.internal.backends.chrome_inspector import devtools_http
from telemetry.internal.backends.chrome_inspector import inspector_backend
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import memory_backend
from telemetry.internal.backends.chrome_inspector import native_profiling_backend
from telemetry.internal.backends.chrome_inspector import system_info_backend
from telemetry.internal.backends.chrome_inspector import tracing_backend
from telemetry.internal.backends.chrome_inspector import window_manager_backend
from telemetry.internal.platform.tracing_agent import (
    chrome_tracing_devtools_manager)


class TabNotFoundError(exceptions.Error):
  pass


class UnsupportedVersionError(exceptions.Error):
  pass


# Only versions of Chrome from M58 and above are supported. Older versions
# did not support many of the modern features currently in use by Telemetry.
MIN_SUPPORTED_MAJOR_NUMBER = 58

# The first WebSocket connections or calls against a newly-started
# browser, specifically in Debug builds, can take a long time. Give
# them 60s to complete instead of the default 10s used in many places
# in this file.
_FIRST_CALL_TIMEOUT = 60

# These are possible exceptions raised when the DevTools agent is not ready
# to accept incomming connections.
_DEVTOOLS_CONNECTION_ERRORS = (
    devtools_http.DevToolsClientConnectionError,
    inspector_websocket.WebSocketException,
    socket.error)


def GetDevToolsBackEndIfReady(devtools_port,
                              app_backend,
                              browser_target=None,
                              enable_tracing=True):
  client = _DevToolsClientBackend(app_backend)
  try:
    client.Connect(devtools_port, browser_target, enable_tracing)
    logging.info('DevTools agent ready at %s', client)
  except _DEVTOOLS_CONNECTION_ERRORS as exc:
    logging.info('DevTools agent at %s not ready yet: %s', client, exc)
    client = None
  return client


class BrowserTargetNotFoundException(Exception):
  pass

class _DevToolsClientBackend():
  """An object that communicates with Chrome's devtools.

  This class owns a map of InspectorBackends. It is responsible for creating
  and destroying them.
  """
  def __init__(self, app_backend):
    """Create an object able to connect with the DevTools agent.

    Args:
      app_backend: The app that contains the DevTools agent.
    """
    self._app_backend = app_backend
    self._browser_target = None
    self._forwarder = None
    self._devtools_http = None
    self._browser_websocket = None
    self._created = False
    self._local_port = None
    self._remote_port = None

    # Other backends.
    self._tracing_backend = None
    self._memory_backend = None
    self._native_profiling_backend = None
    self._system_info_backend = None
    self._wm_backend = None
    self._devtools_context_map_backend = _DevToolsContextMapBackend(self)

  def __str__(self):
    s = self.browser_target_url
    if self.local_port != self.remote_port:
      s = '%s (remote=%s)' % (s, self.remote_port)
    return s

  @property
  def local_port(self):
    return self._local_port

  @property
  def remote_port(self):
    return self._remote_port

  @property
  def browser_target_url(self):
    # For Fuchsia and Cast browsers, get the browser_target through a JSON
    # request
    if self.platform_backend.GetOSName() in ['fuchsia', 'castos']:
      resp = self.GetVersion()
      if 'webSocketDebuggerUrl' in resp:
        return resp['webSocketDebuggerUrl']
      raise BrowserTargetNotFoundException('Could not get the browser target.')
    return 'ws://127.0.0.1:%i%s' % (self._local_port, self._browser_target)

  @property
  def app_backend(self):
    return self._app_backend

  @property
  def platform_backend(self):
    return self._app_backend.platform_backend

  @property
  def supports_overriding_memory_pressure_notifications(self):
    return (
        isinstance(self.app_backend, browser_backend.BrowserBackend)
        and self.app_backend.supports_overriding_memory_pressure_notifications)

  @property
  def is_tracing_running(self):
    return self._tracing_backend.is_tracing_running

  @property
  def has_tracing_client(self):
    return self._tracing_backend is not None

  def Connect(self, devtools_port, browser_target, enable_tracing=True):
    try:
      self._Connect(devtools_port, browser_target, enable_tracing)
    except:
      self.Close()  # Close any connections made if failed to connect to all.
      raise

  @retry_util.RetryOnException(devtools_http.DevToolsClientUrlError, retries=3)
  def _WaitForConnection(self, retries=None):
    del retries
    self._devtools_http.Request('')

  def _SetUpPortForwarding(self, devtools_port):
    self._forwarder = self.platform_backend.forwarder_factory.Create(
        local_port=None,  # Forwarder will choose an available port.
        remote_port=devtools_port, reverse=True)
    self._local_port = self._forwarder._local_port
    self._remote_port = self._forwarder._remote_port
    self._devtools_http = devtools_http.DevToolsHttp(self.local_port)

    # For Fuchsia, wait until port forwarding has started working.
    if self.platform_backend.GetOSName() == 'fuchsia':
      self._WaitForConnection()

  def _Connect(self, devtools_port, browser_target, enable_tracing):
    """Attempt to connect to the DevTools client.

    Args:
      devtools_port: The devtools_port uniquely identifies the DevTools agent.
      browser_target: An optional string to override the default path used to
        establish a websocket connection with the browser inspector.
      enable_tracing: Defines if a tracing_client is created.

    Raises:
      Any of _DEVTOOLS_CONNECTION_ERRORS if failed to establish the connection.
    """
    self._browser_target = browser_target or '/devtools/browser'
    self._SetUpPortForwarding(devtools_port)

    # If the agent is not alive and ready, trying to get the major number will
    # raise a devtools_http.DevToolsClientConnectionError.
    major_number = self.GetChromeMajorNumber()
    if major_number < MIN_SUPPORTED_MAJOR_NUMBER:
      raise UnsupportedVersionError(
          'Chrome major number %d is no longer supported' % major_number)

    # Ensure that the inspector websocket is ready. This may raise a
    # inspector_websocket.WebSocketException or socket.error if not ready.
    self._browser_websocket = inspector_websocket.InspectorWebsocket()
    self._browser_websocket.Connect(self.browser_target_url, timeout=10)

    chrome_tracing_devtools_manager.RegisterDevToolsClient(self)

    # If there is a trace_config it means that Telemetry has already started
    # Chrome tracing via a startup config. The TracingBackend also needs needs
    # this config to initialize itself correctly.
    if enable_tracing:
      trace_config = (
          self.platform_backend.tracing_controller_backend \
              .GetChromeTraceConfig())
      self._tracing_backend = tracing_backend.TracingBackend(
          self._browser_websocket, trace_config)

  @exc_util.BestEffort
  def Close(self):
    if self._tracing_backend is not None:
      self._tracing_backend.Close()
      self._tracing_backend = None
    if self._memory_backend is not None:
      self._memory_backend.Close()
      self._memory_backend = None
    if self._native_profiling_backend is not None:
      self._native_profiling_backend.Close()
      self._native_profiling_backend = None
    if self._system_info_backend is not None:
      self._system_info_backend.Close()
      self._system_info_backend = None
    if self._wm_backend is not None:
      self._wm_backend.Close()
      self._wm_backend = None

    if self._devtools_context_map_backend is not None:
      self._devtools_context_map_backend.Clear()
      self._devtools_context_map_backend = None

    # Close the DevTools connections last (in case the backends above still
    # need to interact with them while closing).
    if self._browser_websocket is not None:
      self._browser_websocket.Disconnect()
      self._browser_websocket = None
    if self._devtools_http is not None:
      self._devtools_http.Disconnect()
      self._devtools_http = None
    if self._forwarder is not None:
      self._forwarder.Close()
      self._forwarder = None

  def CloseBrowser(self):
    """Close the browser instance."""
    request = {
        'method': 'Browser.close',
    }
    self._browser_websocket.SyncRequest(request, timeout=60)

  def IsAlive(self):
    """Whether the DevTools server is available and connectable."""
    if self._devtools_http is None:
      return False
    try:
      self._devtools_http.Request('')
    except devtools_http.DevToolsClientConnectionError:
      return False
    else:
      return True

  @decorators.Cache
  def GetVersion(self):
    """Return the version dict as provided by the DevTools agent."""
    return self._devtools_http.RequestJson('version')

  def GetChromeMajorNumber(self):
    # Detect version information.
    resp = self.GetVersion()
    if 'Protocol-Version' in resp:
      if 'Browser' in resp:
        major_number_match = re.search(r'.+/(\d+)\.\d+\.\d+\.\d+',
                                       resp['Browser'])
      if not major_number_match and 'User-Agent' in resp:
        major_number_match = re.search(
            r'Chrome/(\d+)\.\d+\.\d+\.\d+ (Mobile )?Safari',
            resp['User-Agent'])

      if major_number_match:
        major_number = int(major_number_match.group(1))
        if major_number:
          return major_number

    # Major number can't be determined, so fail any major number checks.
    return 0

  def _ListInspectableContexts(self):
    return self._devtools_http.RequestJson('')

  def RequestNewTab(self, timeout, in_new_window=False, url=None):
    """Creates a new tab, either in new window or current window.

    Returns:
      A dict of a parsed JSON object as returned by DevTools. Example:
      If an error is present, the dict will contain an 'error' key.
      If no error is present, the result is present in the 'result' key:
      {
        "result": {
          "targetId": "id-string"  # This is the ID for the tab.
        }
      }
    """
    request = {
        'method': 'Target.createTarget',
        'params': {
            'url': url if url else 'about:blank',
            'newWindow': in_new_window
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout)

  def CloseTab(self, tab_id, timeout):
    """Closes the tab with the given id.

    Raises:
      devtools_http.DevToolsClientConnectionError
      TabNotFoundError
    """
    try:
      return self._devtools_http.Request(
          'close/%s' % tab_id, timeout=timeout)
    except devtools_http.DevToolsClientUrlError as e:
      raise TabNotFoundError(
          'Unable to close tab, tab id not found: %s' % tab_id) from e

  def ActivateTab(self, tab_id, timeout):
    """Activates the tab with the given id.

    Raises:
      devtools_http.DevToolsClientConnectionError
      TabNotFoundError
    """
    try:
      return self._devtools_http.Request(
          'activate/%s' % tab_id, timeout=timeout)
    except devtools_http.DevToolsClientUrlError as e:
      raise TabNotFoundError(
          'Unable to activate tab, tab id not found: %s' % tab_id) from e

  def GetUrl(self, tab_id):
    """Returns the URL of the tab with |tab_id|, as reported by devtools.

    Raises:
      devtools_http.DevToolsClientConnectionError
    """
    for c in self._ListInspectableContexts():
      if c['id'] == tab_id:
        return c['url']
    return None

  def IsInspectable(self, tab_id):
    """Whether the tab with |tab_id| is inspectable, as reported by devtools.

    Raises:
      devtools_http.DevToolsClientConnectionError
    """
    contexts = self._ListInspectableContexts()
    return tab_id in [c['id'] for c in contexts]

  def GetUpdatedInspectableContexts(self):
    """Returns an updated instance of _DevToolsContextMapBackend."""
    contexts = self._ListInspectableContexts()
    self._devtools_context_map_backend._Update(contexts)
    return self._devtools_context_map_backend

  def _CreateWindowManagerBackendIfNeeded(self):
    if not self._wm_backend:
      self._wm_backend = window_manager_backend.WindowManagerBackend(
          self._browser_websocket)

  def _CreateMemoryBackendIfNeeded(self):
    assert self.supports_overriding_memory_pressure_notifications
    if not self._memory_backend:
      self._memory_backend = memory_backend.MemoryBackend(
          self._browser_websocket)

  def _CreateNativeProfilingBackendIfNeeded(self):
    if not self._native_profiling_backend:
      self._native_profiling_backend = (
          native_profiling_backend.NativeProfilingBackend(
              self._browser_websocket))

  def _CreateSystemInfoBackendIfNeeded(self):
    if not self._system_info_backend:
      self._system_info_backend = system_info_backend.SystemInfoBackend(
          self.browser_target_url)

  def StartChromeTracing(self, trace_config, transfer_mode=None, timeout=20):
    """
    Args:
        trace_config: An tracing_config.TracingConfig instance.
        transfer_mode: Defaults to using 'ReturnAsStream' transfer mode
          for Chrome tracing. Can be set to 'ReportEvents'.
        timeout: Time waited for websocket to receive a response.
    """
    if not self._tracing_backend:
      return None

    assert trace_config and trace_config.enable_chrome_trace
    return self._tracing_backend.StartTracing(
        trace_config.chrome_trace_config, transfer_mode, timeout)

  def RecordChromeClockSyncMarker(self, sync_id):
    assert self.is_tracing_running, 'Tracing must be running to clock sync.'
    self._tracing_backend.RecordClockSyncMarker(sync_id)

  def StopChromeTracing(self):
    if not self._tracing_backend:
      return

    assert self.is_tracing_running
    try:
      backend = self.FirstTabBackend()
      if backend is not None:
        backend.AddTimelineMarker('first-renderer-thread')
        backend.AddTimelineMarker(backend.id)
      else:
        logging.warning('No page inspector backend found.')
    finally:
      self._tracing_backend.StopTracing()

  def _IterInspectorBackends(self, types):
    """Iterate over inspector backends from this client.

    Note: The devtools client might list contexts which, howerver, do not yet
    have a live DevTools instance to connect to (e.g. background tabs which may
    have been discarded or not yet created). In such case this method will hang
    and eventually timeout when trying to create an inspector backend to
    communicate with such contexts.
    """
    context_map = self.GetUpdatedInspectableContexts()
    for context in context_map.contexts:
      if context['type'] in types:
        yield context_map.GetInspectorBackend(context['id'])

  def FirstTabBackend(self):
    """Obtain the inspector backend for the firstly created tab."""
    return next(self._IterInspectorBackends(['page']), None)

  # TODO(cbruni): lover timeout faster investigating  crbug.com/1395482
  def CollectChromeTracingData(self, trace_data_builder, timeout=240):
    if not self._tracing_backend:
      return

    self._tracing_backend.CollectTraceData(trace_data_builder, timeout)

  # This call may be made early during browser bringup and may cause the
  # GPU process to launch, which takes a long time in Debug builds and
  # has been seen to frequently exceed the default 10s timeout used
  # throughout this file. Use a larger timeout by default. Callers
  # typically do not override this.
  def GetSystemInfo(self, timeout=_FIRST_CALL_TIMEOUT):
    self._CreateSystemInfoBackendIfNeeded()
    return self._system_info_backend.GetSystemInfo(timeout)

  def DumpMemory(self, timeout=None, detail_level=None, deterministic=False):
    """Dumps memory.

    Args:
      timeout: seconds to wait between websocket responses.
      detail_level: Level of detail in memory dump. One of ['detailed',
      'light', 'background']. Defaults to 'detailed'.

    Returns:
      GUID of the generated dump if successful, None otherwise.

    Raises:
      TracingTimeoutException: If more than |timeout| seconds has passed
      since the last time any data is received.
      TracingUnrecoverableException: If there is a websocket error.
      TracingUnexpectedResponseException: If the response contains an error
      or does not contain the expected result.
    """
    if not self._tracing_backend:
      return None

    return self._tracing_backend.DumpMemory(
        timeout=timeout,
        detail_level=detail_level,
        deterministic=deterministic)

  def SetMemoryPressureNotificationsSuppressed(self, suppressed, timeout=30):
    """Enable/disable suppressing memory pressure notifications.

    Args:
      suppressed: If true, memory pressure notifications will be suppressed.
      timeout: The timeout in seconds.

    Raises:
      MemoryTimeoutException: If more than |timeout| seconds has passed
      since the last time any data is received.
      MemoryUnrecoverableException: If there is a websocket error.
      MemoryUnexpectedResponseException: If the response contains an error
      or does not contain the expected result.
    """
    self._CreateMemoryBackendIfNeeded()
    return self._memory_backend.SetMemoryPressureNotificationsSuppressed(
        suppressed, timeout)

  def SimulateMemoryPressureNotification(self, pressure_level, timeout=30):
    """Simulate a memory pressure notification.

    Args:
      pressure level: The memory pressure level of the notification ('moderate'
      or 'critical').
      timeout: The timeout in seconds.

    Raises:
      MemoryTimeoutException: If more than |timeout| seconds has passed
      since the last time any data is received.
      MemoryUnrecoverableException: If there is a websocket error.
      MemoryUnexpectedResponseException: If the response contains an error
      or does not contain the expected result.
    """
    self._CreateMemoryBackendIfNeeded()
    return self._memory_backend.SimulateMemoryPressureNotification(
        pressure_level, timeout)

  def DumpProfilingDataOfAllProcesses(self, timeout):
    """Causes all profiling data of all Chrome processes to be dumped to disk.

    This should only be called by an Android backend.
    """
    self._CreateNativeProfilingBackendIfNeeded()
    return self._native_profiling_backend.DumpProfilingDataOfAllProcesses(
        timeout)

  @property
  def window_manager_backend(self):
    """Return the window manager backend.

    This should be called by a CrOS backend only.
    """
    self._CreateWindowManagerBackendIfNeeded()
    return self._wm_backend

  def ExecuteBrowserCommand(self, command_id, timeout):
    request = {
        'method': 'Browser.executeBrowserCommand',
        'params': {
            'commandId': command_id,
        }
    }
    self._browser_websocket.SyncRequest(request, timeout)

  def SetDownloadBehavior(self, behavior, downloadPath, timeout):
    request = {
        'method': 'Browser.setDownloadBehavior',
        'params': {
            'behavior': behavior,
            'downloadPath': downloadPath,
        }
    }
    self._browser_websocket.SyncRequest(request, timeout)

  def GetWindowForTarget(self, target_id):
    request = {
        'method': 'Browser.getWindowForTarget',
        'params': {
            'targetId': target_id
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout=30)

  def SetWindowBounds(self, window_id, bounds):
    request = {
        'method': 'Browser.setWindowBounds',
        'params': {
            'windowId': window_id,
            'bounds': bounds
        }
    }
    self._browser_websocket.SyncRequest(request, timeout=30)


class _DevToolsContextMapBackend():
  def __init__(self, devtools_client):
    self._devtools_client = devtools_client
    self._contexts = None
    self._inspector_backends_dict = {}

  @property
  def contexts(self):
    """The most up to date contexts data.

    Returned in the order returned by devtools agent."""
    return self._contexts

  def GetContextInfo(self, context_id):
    for context in self._contexts:
      if context['id'] == context_id:
        return context
    raise KeyError('Cannot find a context with id=%s' % context_id)

  def GetInspectorBackend(self, context_id):
    """Gets an InspectorBackend instance for the given context_id.

    This lazily creates InspectorBackend for the context_id if it does
    not exist yet. Otherwise, it will return the cached instance."""
    if context_id in self._inspector_backends_dict:
      return self._inspector_backends_dict[context_id]

    for context in self._contexts:
      if context['id'] == context_id:
        new_backend = inspector_backend.InspectorBackend(
            self._devtools_client, context)
        self._inspector_backends_dict[context_id] = new_backend
        return new_backend

    raise KeyError('Cannot find a context with id=%s' % context_id)

  def _Update(self, contexts):
    # Remove InspectorBackend that is not in the current inspectable
    # contexts list.
    context_ids = [context['id'] for context in contexts]
    for context_id in list(self._inspector_backends_dict.keys()):
      if context_id not in context_ids:
        backend = self._inspector_backends_dict[context_id]
        backend.Disconnect()
        del self._inspector_backends_dict[context_id]

    valid_contexts = []
    for context in contexts:
      # If the context does not have webSocketDebuggerUrl, skip it.
      # If an InspectorBackend is already created for the tab,
      # webSocketDebuggerUrl will be missing, and this is expected.
      context_id = context['id']
      if context_id not in self._inspector_backends_dict:
        if 'webSocketDebuggerUrl' not in context:
          logging.debug('webSocketDebuggerUrl missing, removing %s',
                        context_id)
          continue
      valid_contexts.append(context)
    self._contexts = valid_contexts

  def Clear(self):
    for backend in self._inspector_backends_dict.values():
      backend.Disconnect()
    self._inspector_backends_dict = {}
    self._contexts = None
