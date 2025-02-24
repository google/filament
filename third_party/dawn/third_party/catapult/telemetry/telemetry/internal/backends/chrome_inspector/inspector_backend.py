# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import functools
import json
import logging
import socket
import sys
import time
import six
from six.moves import input # pylint: disable=redefined-builtin
from six.moves.urllib.parse import urlparse

from py_trace_event import trace_event

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends.chrome_inspector import devtools_http
from telemetry.internal.backends.chrome_inspector import inspector_console
from telemetry.internal.backends.chrome_inspector import inspector_log
from telemetry.internal.backends.chrome_inspector import inspector_memory
from telemetry.internal.backends.chrome_inspector import inspector_page
from telemetry.internal.backends.chrome_inspector import inspector_runtime
from telemetry.internal.backends.chrome_inspector import inspector_serviceworker
from telemetry.internal.backends.chrome_inspector import inspector_storage
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import websocket
from telemetry.util import js_template

import py_utils


def _HandleInspectorWebSocketExceptions(func):
  """Decorator for converting inspector_websocket exceptions.

  When an inspector_websocket exception is thrown in the original function,
  this decorator converts it into a telemetry exception and adds debugging
  information.
  """
  @functools.wraps(func)
  def Inner(inspector_backend, *args, **kwargs):
    try:
      return func(inspector_backend, *args, **kwargs)
    except (socket.error, inspector_websocket.WebSocketException,
            inspector_websocket.WebSocketDisconnected) as e:
      inspector_backend._ConvertExceptionFromInspectorWebsocket(e)
    # Will never actually get hit, but Pylint doesn't realize that the above
    # except re-raises the exception.
    raise RuntimeError()

  return Inner


class InspectorBackend(six.with_metaclass(trace_event.TracedMetaClass, object)):
  """Class for communicating with a devtools client.

  The owner of an instance of this class is responsible for calling
  Disconnect() before disposing of the instance.
  """

  def __init__(self, devtools_client, context, timeout=120):
    self._websocket = inspector_websocket.InspectorWebsocket()
    self._websocket.RegisterDomain(
        'Inspector', self._HandleInspectorDomainNotification)
    self._cast_issue_message, self._cast_sink_list = None, []
    self._websocket.RegisterDomain('Cast', self._HandleCastDomainNotification)

    self._devtools_client = devtools_client
    # Be careful when using the context object, since the data may be
    # outdated since this is never updated once InspectorBackend is
    # created. Consider an updating strategy for this. (For an example
    # of the subtlety, see the logic for self.url property.)
    self._context = context

    logging.debug('InspectorBackend._Connect() to %s', self.debugger_url)
    try:
      self._websocket.Connect(self.debugger_url, timeout)
      self._console = inspector_console.InspectorConsole(self._websocket)
      self._log = inspector_log.InspectorLog(self._websocket)
      self._memory = inspector_memory.InspectorMemory(self._websocket)
      self._page = inspector_page.InspectorPage(
          self._websocket, timeout=timeout)
      self._runtime = inspector_runtime.InspectorRuntime(self._websocket)
      self._serviceworker = inspector_serviceworker.InspectorServiceWorker(
          self._websocket, timeout=timeout)
      self._storage = inspector_storage.InspectorStorage(self._websocket)
    except (inspector_websocket.WebSocketException, exceptions.TimeoutException,
            py_utils.TimeoutException) as e:
      self._ConvertExceptionFromInspectorWebsocket(e)

  def Disconnect(self):
    """Disconnects the inspector websocket.

    This method intentionally leaves the self._websocket object around, so that
    future calls it to it will fail with a relevant error.
    """
    self._DisconnectWithoutTracing()

  def _DisconnectWithoutTracing(self):
    # All methods in this class are automatically traced unless they start with
    # _ due to using TracedMetaClass as a meta class. This causes issues with
    # the destructor, as we can deadlock when trying to acquire the tracing lock
    # if another test has already started and is starting tracing up when the
    # destructor is called. So, make all code called from __del__ untraced.
    if self._websocket:
      self._websocket.Disconnect()

  def __del__(self):
    self._DisconnectWithoutTracing()

  @property
  def app(self):
    return self._devtools_client.app_backend.app

  @property
  def url(self):
    """Returns the URL of the tab, as reported by devtools.

    Raises:
      devtools_http.DevToolsClientConnectionError
    """
    return self._devtools_client.GetUrl(self.id)

  @property
  def id(self): # pylint: disable=invalid-name
    return self._context['id']

  @property
  def debugger_url(self):
    return self._context['webSocketDebuggerUrl']

  def StopAllServiceWorkers(self, timeout):
    self._serviceworker.StopAllWorkers(timeout)

  def ClearDataForOrigin(self, url, timeout):
    self._storage.ClearDataForOrigin(url, timeout)

  def EnableSharedStorageNotifications(self, timeout=60):
    self._storage.EnableSharedStorageNotifications(timeout)

  def DisableSharedStorageNotifications(self, timeout=60):
    self._storage.DisableSharedStorageNotifications(timeout)

  @property
  def shared_storage_notifications(self):
    return self._storage.shared_storage_notifications

  def ClearSharedStorageNotifications(self):
    self._storage.ClearSharedStorageNotifications()

  @property
  def shared_storage_notifications_enabled(self):
    return self._storage.shared_storage_notifications_enabled

  def GetSharedStorageMetadata(self, origin, timeout=60):
    return self._storage.GetSharedStorageMetadata(origin=origin,
                                                  timeout=timeout)

  def GetSharedStorageEntries(self, origin, timeout=60):
    return self._storage.GetSharedStorageEntries(origin=origin,
                                                  timeout=timeout)

  def GetWebviewInspectorBackends(self):
    """Returns a list of InspectorBackend instances associated with webviews.

    Raises:
      devtools_http.DevToolsClientConnectionError
    """
    inspector_backends = []
    devtools_context_map = self._devtools_client.GetUpdatedInspectableContexts()
    for context in devtools_context_map.contexts:
      if context['type'] == 'webview':
        inspector_backends.append(
            devtools_context_map.GetInspectorBackend(context['id']))
    return inspector_backends

  def IsInspectable(self):
    """Whether the tab is inspectable, as reported by devtools."""
    try:
      return self._devtools_client.IsInspectable(self.id)
    except devtools_http.DevToolsClientConnectionError:
      return False

  # Public methods implemented in JavaScript.

  @property
  @decorators.Cache
  def screenshot_supported(self):
    return True

  @_HandleInspectorWebSocketExceptions
  def Screenshot(self, timeout):
    assert self.screenshot_supported, 'Browser does not support screenshotting'
    return self._page.CaptureScreenshot(timeout)

  @_HandleInspectorWebSocketExceptions
  def FullScreenshot(self, timeout):
    assert self.screenshot_supported, 'Browser does not support screenshotting'
    return self._page.CaptureFullScreenshot(timeout)

  # Memory public methods.

  @_HandleInspectorWebSocketExceptions
  def GetDOMStats(self, timeout):
    """Gets memory stats from the DOM.

    Raises:
      inspector_memory.InspectorMemoryException
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    dom_counters = self._memory.GetDOMCounters(timeout)
    return {
        'document_count': dom_counters['documents'],
        'node_count': dom_counters['nodes'],
        'event_listener_count': dom_counters['jsEventListeners']
    }

  @_HandleInspectorWebSocketExceptions
  def PrepareForLeakDetection(self, timeout):
    self._memory.PrepareForLeakDetection(timeout)

  # Page public methods.

  @_HandleInspectorWebSocketExceptions
  def WaitForNavigate(self, timeout):
    self._page.WaitForNavigate(timeout)

  @_HandleInspectorWebSocketExceptions
  def Navigate(self, url, script_to_evaluate_on_commit, timeout):
    self._page.Navigate(url, script_to_evaluate_on_commit, timeout)

  # Console public methods.

  @_HandleInspectorWebSocketExceptions
  def GetCurrentConsoleOutputBuffer(self, timeout=10):
    return self._console.GetCurrentConsoleOutputBuffer(timeout)

  # Runtime public methods.

  @_HandleInspectorWebSocketExceptions
  def ExecuteJavaScript(self, statement, **kwargs):
    """Executes a given JavaScript statement. Does not return the result.

    Example: runner.ExecuteJavaScript('var foo = {{ value }};', value='hi');

    Args:
      statement: The statement to execute (provided as a string).

    Optional keyword args:
      timeout: The number of seconds to wait for the statement to execute.
      context_id: The id of an iframe where to execute the code; the main page
          has context_id=1, the first iframe context_id=2, etc.
      user_gesture: Whether execution should be treated as initiated by user
          in the UI. Code that plays media or requests fullscreen may not take
          effects without user_gesture set to True.
      Additional keyword arguments provide values to be interpolated within
          the statement. See telemetry.util.js_template for details.

    Raises:
      py_utils.TimeoutException
      exceptions.EvaluateException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    # Use the default both when timeout=None or the option is ommited.
    timeout = kwargs.pop('timeout', None) or 60
    context_id = kwargs.pop('context_id', None)
    user_gesture = kwargs.pop('user_gesture', None) or False
    statement = js_template.Render(statement, **kwargs)
    self._runtime.Execute(statement, context_id, timeout,
                          user_gesture=user_gesture)

  def EvaluateJavaScript(self, expression, **kwargs):
    """Returns the result of evaluating a given JavaScript expression.

    Example: runner.ExecuteJavaScript('document.location.href');

    Args:
      expression: The expression to execute (provided as a string).

    Optional keyword args:
      timeout: The number of seconds to wait for the expression to evaluate.
      context_id: The id of an iframe where to execute the code; the main page
          has context_id=1, the first iframe context_id=2, etc.
      user_gesture: Whether execution should be treated as initiated by user
          in the UI. Code that plays media or requests fullscreen may not take
          effects without user_gesture set to True.
      promise: Whether the execution is a javascript promise, and should
          wait for the promise to resolve prior to returning.
      Additional keyword arguments provide values to be interpolated within
          the expression. See telemetry.util.js_template for details.

    Raises:
      py_utils.TimeoutException
      exceptions.EvaluateException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    # Use the default both when timeout=None or the option is ommited.
    timeout = kwargs.pop('timeout', None) or 60
    context_id = kwargs.pop('context_id', None)
    user_gesture = kwargs.pop('user_gesture', None) or False
    promise = kwargs.pop('promise', None) or False
    expression = js_template.Render(expression, **kwargs)
    return self._EvaluateJavaScript(expression, context_id, timeout,
                                    user_gesture=user_gesture,
                                    promise=promise)

  def WaitForJavaScriptCondition(self, condition, **kwargs):
    """Wait for a JavaScript condition to become truthy.

    Example: runner.WaitForJavaScriptCondition('window.foo == 10');

    Args:
      condition: The JavaScript condition (provided as string).

    Optional keyword args:
      timeout: The number in seconds to wait for the condition to become
          True (default to 60).
      context_id: The id of an iframe where to execute the code; the main page
          has context_id=1, the first iframe context_id=2, etc.
      Additional keyword arguments provide values to be interpolated within
          the expression. See telemetry.util.js_template for details.

    Returns:
      The value returned by the JavaScript condition that got interpreted as
      true.

    Raises:
      py_utils.TimeoutException
      exceptions.EvaluateException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    # Use the default both when timeout=None or the option is ommited.
    timeout = kwargs.pop('timeout', None) or 60
    context_id = kwargs.pop('context_id', None)
    condition = js_template.Render(condition, **kwargs)

    def IsJavaScriptExpressionTrue():
      try:
        return self._EvaluateJavaScript(condition, context_id, timeout)
      except exceptions.DevtoolsTargetClosedException:
        # Ignore errors caused by navigation.
        return False

    try:
      return py_utils.WaitFor(IsJavaScriptExpressionTrue, timeout)
    except py_utils.TimeoutException as toe:
      # Try to make timeouts a little more actionable by dumping console output.
      debug_message = None
      try:
        debug_message = (
            'Console output:\n%s' %
            self.GetCurrentConsoleOutputBuffer())
      except Exception as e: # pylint: disable=broad-except
        debug_message = (
            'Exception thrown when trying to capture console output: %s' %
            repr(e))
      # Rethrow with the original stack trace for better debugging.
      six.reraise(
          py_utils.TimeoutException,
          py_utils.TimeoutException(
              'Timeout after %ss while waiting for JavaScript:'
              % timeout + condition + '\n' + repr(toe) + '\n' + debug_message
          ),
          sys.exc_info()[2]
      )


  def AddTimelineMarker(self, marker):
    return self.ExecuteJavaScript(
        """
        console.time({{ marker }});
        console.timeEnd({{ marker }});
        """,
        marker=str(marker))

  def WaitForSharedStorageEvents(self,
                                 expected_events,
                                 mode='strict',
                                 timeout=60):
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

    if mode not in ['strict', 'relaxed']:
      raise exceptions.StoryActionError('mode %s is unrecognized' % mode)

    def AreEventsEquivalent(expected_event, actual_event):
      # For `expected_event` and `actual_event` as dictionaries.
      for param_key in expected_event:
        if not param_key in actual_event:
          return False
        if expected_event[param_key] != actual_event[param_key]:
          return False

      # It's permitted for `actual_event` to have keys that are not listed in
      # `expected_event`.
      return True

    def AreEventsReceivedStrict(actual_events):
      if len(expected_events) != len(actual_events):
        return False
      for i, expected_event in enumerate(expected_events):
        if not AreEventsEquivalent(expected_event, actual_events[i]):
          return False
      return True

    def AreEventsReceivedRelaxed(actual_events):
      if len(expected_events) > len(actual_events):
        return False
      actual_index = 0
      for expected_event in expected_events:
        while actual_index < len(actual_events) and not AreEventsEquivalent(
            expected_event, actual_events[actual_index]):
          actual_index += 1
        if actual_index == len(actual_events):
          return False
        actual_index += 1
      return True

    def AreEventsReceived():
      try:
        received_events = self.shared_storage_notifications
        if mode == 'strict':
          return AreEventsReceivedStrict(received_events)
        return AreEventsReceivedRelaxed(received_events)
      except exceptions.DevtoolsTargetClosedException:
        # Ignore errors caused by navigation.
        return False

    if not self.shared_storage_notifications_enabled:
      message = "".join(['Did not enable shared storage notifications:\n',
                         'call EnableSharedStorageNotifications() before ',
                         'any shared storage calls that you want to track'])
      raise exceptions.StoryActionError(message)

    try:
      return py_utils.WaitFor(AreEventsReceived, timeout)
    except py_utils.TimeoutException as toe:
      metadata_dump = ''
      try:
        # Try to request shared storage metadata. This will send a sync request
        # to the websocket that will also open the websocket to receive any
        # backlogged messages.
        parse_result = urlparse(self.url)
        origin = '://'.join([parse_result[0], parse_result[1]])
        metadata = self.GetSharedStorageMetadata(origin)
        metadata_dump = json.dumps(metadata, indent=2)
        if AreEventsReceived():
          logging.info('Expected events were received after getting metadata: ')
          logging.info(metadata_dump)
          return True
      except Exception as me: # pylint: disable=broad-except
        # If we are unable to get metadata, we move on. We don't want to
        # confuse the debug message below by adding in another exception,
        # so we keep it separate.
        logging.info('Unable to get metadata: %s' % repr(me))
      # Try to make timeouts a little more actionable by dumping console output.
      debug_message = None
      try:
        debug_message = (
            'Console output:\n%s' %
            self.GetCurrentConsoleOutputBuffer())
      except Exception as e: # pylint: disable=broad-except
        debug_message = (
            'Exception thrown when trying to capture console output: %s' %
            repr(e))
      # Rethrow with the original stack trace for better debugging.
      actual_events = self.shared_storage_notifications
      six.reraise(
          py_utils.TimeoutException,
          py_utils.TimeoutException(
              ''.join(['Timeout after %d while waiting ' % timeout,
                       'for Expected Shared Storage Events:\n',
                       json.dumps(expected_events, indent=2), '\n',
                       'Actual Events Received:\n',
                       json.dumps(actual_events, indent=2),
                       '\n', 'Metadata:\n', metadata_dump, '\n', repr(toe),
                       '\n', debug_message])
          ),
          sys.exc_info()[2]
      )

  @_HandleInspectorWebSocketExceptions
  def EnableAllContexts(self):
    """Allows access to iframes.

    Raises:
      exceptions.WebSocketDisconnected
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._runtime.EnableAllContexts()

  @_HandleInspectorWebSocketExceptions
  def SynthesizeScrollGesture(
      self, x=100, y=800, x_distance=0, y_distance=-500,
      x_overscroll=None, y_overscroll=None,
      prevent_fling=None, speed=None,
      gesture_source_type=None, repeat_count=None,
      repeat_delay_ms=None, interaction_marker_name=None,
      timeout=60):
    """Runs an inspector command that causes a repeatable browser driven scroll.

    Args:
      x: X coordinate of the start of the gesture in CSS pixels.
      y: Y coordinate of the start of the gesture in CSS pixels.
      xDistance: Distance to scroll along the X axis (positive to scroll left).
      yDistance: Distance to scroll along the Y axis (positive to scroll up).
      x_overscroll: Number of additional pixels to scroll back along the X axis.
      yOverscroll: Number of additional pixels to scroll back along the Y axis.
      preventFling: Prevents a fling gesture.
      speed: Swipe speed in pixels per second.
      gesture_source_type: Which type of input events to be generated.
      repeat_count: Number of additional repeats beyond the first scroll.
      repeat_delay_ms: Number of milliseconds delay between each repeat.
      interaction_marker_name: The name of the interaction markers to generate.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {
        'x': x,
        'y': y,
        'xDistance': x_distance,
        'yDistance': y_distance
    }

    if prevent_fling is not None:
      params['preventFling'] = prevent_fling

    if x_overscroll is not None:
      params['xOverscroll'] = x_overscroll

    if y_overscroll is not None:
      params['yOverscroll'] = y_overscroll

    if speed is not None:
      params['speed'] = speed

    if repeat_count is not None:
      params['repeatCount'] = repeat_count

    if gesture_source_type is not None:
      params['gestureSourceType'] = gesture_source_type

    if repeat_delay_ms is not None:
      params['repeatDelayMs'] = repeat_delay_ms

    if interaction_marker_name is not None:
      params['interactionMarkerName'] = interaction_marker_name

    scroll_command = {
        'method': 'Input.synthesizeScrollGesture',
        'params': params
    }
    return self._runtime.RunInspectorCommand(scroll_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def DispatchKeyEvent(
      self, key_event_type='char', modifiers=None,
      timestamp=None, text=None, unmodified_text=None,
      key_identifier=None, dom_code=None, dom_key=None,
      windows_virtual_key_code=None, native_virtual_key_code=None,
      auto_repeat=None, is_keypad=None, is_system_key=None,
      timeout=60):
    """Dispatches a key event to the page.

    Args:
      type: Type of the key event. Allowed values: 'keyDown', 'keyUp',
          'rawKeyDown', 'char'.
      modifiers: Bit field representing pressed modifier keys. Alt=1, Ctrl=2,
          Meta/Command=4, Shift=8 (default: 0).
      timestamp: Time at which the event occurred. Measured in UTC time in
          seconds since January 1, 1970 (default: current time).
      text: Text as generated by processing a virtual key code with a keyboard
          layout. Not needed for for keyUp and rawKeyDown events (default: '').
      unmodified_text: Text that would have been generated by the keyboard if no
          modifiers were pressed (except for shift). Useful for shortcut
          (accelerator) key handling (default: "").
      key_identifier: Unique key identifier (e.g., 'U+0041') (default: '').
      windows_virtual_key_code: Windows virtual key code (default: 0).
      native_virtual_key_code: Native virtual key code (default: 0).
      auto_repeat: Whether the event was generated from auto repeat (default:
          False).
      is_keypad: Whether the event was generated from the keypad (default:
          False).
      is_system_key: Whether the event was a system key event (default: False).

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {
        'type': key_event_type,
    }

    if modifiers is not None:
      params['modifiers'] = modifiers
    if timestamp is not None:
      params['timestamp'] = timestamp
    if text is not None:
      params['text'] = text
    if unmodified_text is not None:
      params['unmodifiedText'] = unmodified_text
    if key_identifier is not None:
      params['keyIdentifier'] = key_identifier
    if dom_code is not None:
      params['code'] = dom_code
    if dom_key is not None:
      params['key'] = dom_key
    if windows_virtual_key_code is not None:
      params['windowsVirtualKeyCode'] = windows_virtual_key_code
    if native_virtual_key_code is not None:
      params['nativeVirtualKeyCode'] = native_virtual_key_code
    if auto_repeat is not None:
      params['autoRepeat'] = auto_repeat
    if is_keypad is not None:
      params['isKeypad'] = is_keypad
    if is_system_key is not None:
      params['isSystemKey'] = is_system_key

    key_command = {
        'method': 'Input.dispatchKeyEvent',
        'params': params
    }
    return self._runtime.RunInspectorCommand(key_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def EnableCast(self, presentation_url, timeout=60):
    """Starts observing Cast-enabled sinks.

    Args:
      presentation_url: string, the URL for making a presentation request.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {'presentationUrl': presentation_url} if presentation_url else {}
    enable_command = {
        'method': 'Cast.enable',
        'params': params
    }
    return self._runtime.RunInspectorCommand(enable_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def GetCastSinks(self):
    """Returns a list of available Cast-enabled sinks.

    Returns:
      The list of sinks that supports Casting.
      Returns an empty list if there is no available sink.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._cast_sink_list

  @_HandleInspectorWebSocketExceptions
  def GetCastIssue(self):
    """Returns the error message when there is an issue while casting.

    Returns:
      The same error message as in extension dialog.
      Returns an empty string if there is no error.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._cast_issue_message

  @_HandleInspectorWebSocketExceptions
  def SetCastSinkToUse(self, sink_name, timeout=60):
    """Sets the sink to be used for a Cast session.

    Args:
      sink_name: string, name of the sink to start a casting session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {'sinkName': sink_name}
    set_sink_command = {
        'method': 'Cast.setSinkToUse',
        'params': params
    }
    return self._runtime.RunInspectorCommand(set_sink_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def StartTabMirroring(self, sink_name, timeout=60):
    """Starts a tab mirroring session.

    Args:
      sink_name: string, name of the sink to start a mirroring session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {'sinkName': sink_name}
    start_mirroring_command = {
        'method': 'Cast.startTabMirroring',
        'params': params
    }
    return self._runtime.RunInspectorCommand(start_mirroring_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def StopCasting(self, sink_name, timeout=60):
    """Stops all session on a specific Cast enabled sink.

    Args:
      sink_name: string, name of the sink to stop a session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {'sinkName': sink_name}
    stop_casting_command = {
        'method': 'Cast.stopCasting',
        'params': params
    }
    return self._runtime.RunInspectorCommand(stop_casting_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def StartMobileDeviceEmulation(
      self, width=360, height=640, dsr=2, timeout=60):
    """Emulates a mobile device.

    This method is intended for benchmarks used to gather non-performance
    metrics only. Mobile emulation is not guaranteed to have the same
    performance characteristics as real devices.

    Example device parameters:
    https://gist.github.com/devinmancuso/0c94410cb14c83ddad6f

    Args:
      width: Screen width.
      height: Screen height.
      dsr: Screen device scale factor.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {
        'width': width,
        'height': height,
        'deviceScaleFactor': dsr,
        'mobile': True,
    }
    emulate_command = {
        'method': 'Emulation.setDeviceMetricsOverride',
        'params': params,
    }
    return self._runtime.RunInspectorCommand(emulate_command, timeout)

  @_HandleInspectorWebSocketExceptions
  def StopMobileDeviceEmulation(self, timeout=60):
    """Stops emulation of a mobile device.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    params = {
        'width': 0,
        'height': 0,
        'deviceScaleFactor': 0,
        'mobile': False,
    }
    emulate_command = {
        'method': 'Emulation.setDeviceMetricsOverride',
        'params': params,
    }
    return self._runtime.RunInspectorCommand(emulate_command, timeout)

  # Methods used internally by other backends.

  def _HandleInspectorDomainNotification(self, res):
    if (res['method'] == 'Inspector.detached' and
        res.get('params', {}).get('reason', '') == 'replaced_with_devtools'):
      self._WaitForInspectorToGoAway()
      return
    if res['method'] == 'Inspector.targetCrashed':
      exception = exceptions.DevtoolsTargetCrashException(self.app)
      self._AddDebuggingInformation(exception)
      raise exception

  def _WaitForInspectorToGoAway(self):
    self._websocket.Disconnect()
    input('The connection to Chrome was lost to the inspector ui.\n'
          'Please close the inspector and press enter to resume '
          'Telemetry run...')
    raise exceptions.DevtoolsTargetCrashException(
        self.app, 'Devtool connection with the browser was interrupted due to '
        'the opening of an inspector.')

  def _HandleCastDomainNotification(self, msg):
    """Runs an inspector command that starts observing Cast-enabled sinks.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    if msg['method'] == 'Cast.sinksUpdated':
      self._cast_sink_list = msg['params'].get('sinks', [])
    elif msg['method'] == 'Cast.issueUpdated':
      self._cast_issue_message = msg['params']

  def _ConvertExceptionFromInspectorWebsocket(self, error):
    """Converts an Exception from inspector_websocket.

    This method always raises a Telemetry exception. It appends debugging
    information. The exact exception raised depends on |error|.

    Args:
      error: An instance of socket.error or
        inspector_websocket.WebSocketException.
    Raises:
      exceptions.TimeoutException: A timeout occurred.
      exceptions.DevtoolsTargetCrashException: On any other error, the most
        likely explanation is that the devtool's target crashed.
    """
    if isinstance(error, inspector_websocket.WebSocketException):
      new_error = exceptions.TimeoutException()
      new_error.AddDebuggingMessage(exceptions.AppCrashException(
          self.app, 'The app is probably crashed:\n'))
    else:
      new_error = exceptions.DevtoolsTargetCrashException(self.app)

    original_error_msg = 'Original exception:\n' + str(error)
    new_error.AddDebuggingMessage(original_error_msg)
    self._AddDebuggingInformation(new_error)

    six.reraise(type(new_error), new_error, sys.exc_info()[2])

  def _AddDebuggingInformation(self, error):
    """Adds debugging information to error.

    Args:
      error: An instance of exceptions.Error.
    """
    if self.IsInspectable():
      msg = (
          'Received a socket error in the browser connection and the tab '
          'still exists. The operation probably timed out.'
      )
    else:
      msg = (
          'Received a socket error in the browser connection and the tab no '
          'longer exists. The tab probably crashed.'
      )
    error.AddDebuggingMessage(msg)
    error.AddDebuggingMessage('Debugger url: %s' % self.debugger_url)

  @_HandleInspectorWebSocketExceptions
  def _EvaluateJavaScript(self, expression, context_id, timeout,
                          user_gesture=False, promise=False):
    try:
      return self._runtime.Evaluate(expression, context_id, timeout,
                                    user_gesture, promise)
    except inspector_websocket.WebSocketException as e:
      if issubclass(e.websocket_error_type,
                    websocket.WebSocketConnectionClosedException):
        logging.error('Inspector connection lost.')
        # Any attempt to send further requests (e.g., to crash processes as
        # below) is guaranteed to fail, so simply propagate the exception.
        raise e
      logging.error('Renderer process hung; forcibly crashing it and '
                    'GPU process. Note that GPU process minidumps '
                    'require --enable-gpu-benchmarking browser arg.')
      # In Telemetry-based GPU tests, the GPU process is likely hung, and that's
      # the reason the renderer process is hung. Crash it so we can see a
      # symbolized minidump. From manual testing, it is important that this be
      # done before crashing the renderer process, or the GPU process's minidump
      # doesn't show up for some reason.
      self._runtime.CrashGpuProcess(timeout)
      # Assume the renderer's main thread is hung. Try to use DevTools to crash
      # the target renderer process (on its IO thread) so we get a minidump we
      # can symbolize.
      self._runtime.CrashRendererProcess(context_id, timeout)
      # Wait several seconds for these minidumps to be written, so the calling
      # code has a better chance of discovering them.
      time.sleep(5)
      raise e

  @_HandleInspectorWebSocketExceptions
  def CollectGarbage(self):
    self._page.CollectGarbage()
