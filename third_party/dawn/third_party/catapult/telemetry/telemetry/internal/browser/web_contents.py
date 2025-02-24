# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import six

from telemetry.core import exceptions

from py_trace_event import trace_event

DEFAULT_WEB_CONTENTS_TIMEOUT = 90

class ServiceWorkerState():
  # These strings should exactly match strings used in
  # wait_for_serviceworker_registration.js
  # The page did not call register().
  NOT_REGISTERED = 'not registered'
  # The page called register(), but there is not an activated service worker.
  INSTALLING = 'installing'
  # The page called register(), and has an activated service worker.
  ACTIVATED = 'activated'

# TODO(achuith, dtu, nduca): Add unit tests specifically for WebContents,
# independent of Tab.
class WebContents(six.with_metaclass(trace_event.TracedMetaClass, object)):

  """Represents web contents in the browser"""
  def __init__(self, inspector_backend):
    self._inspector_backend = inspector_backend

    with open(os.path.join(
        os.path.dirname(__file__),
        'network_quiescence.js')) as f:
      self._quiescence_js = f.read()

    with open(os.path.join(
        os.path.dirname(__file__),
        'wait_for_serviceworker_registration.js')) as f:
      self._wait_for_serviceworker_js = f.read()

    with open(os.path.join(
        os.path.dirname(__file__),
        'wait_for_frame.js')) as f:
      self._wait_for_frame_js = f.read()

    # An incrementing ID used to query frame timing javascript. Using a new id
    # with each request ensures that previously timed-out wait for frame
    # requests don't impact new requests.
    self._wait_for_frame_id = 0

  @property
  def id(self):
    """Return the unique id string for this tab object."""
    return self._inspector_backend.id

  def GetUrl(self):
    """Returns the URL to which the WebContents is connected.

    Raises:
      exceptions.Error: If there is an error in inspector backend connection.
    """
    return self._inspector_backend.url

  def GetWebviewContexts(self):
    """Returns a list of webview contexts within the current inspector backend.

    Returns:
      A list of WebContents objects representing the webview contexts.

    Raises:
      exceptions.Error: If there is an error in inspector backend connection.
    """
    webviews = []
    inspector_backends = self._inspector_backend.GetWebviewInspectorBackends()
    for inspector_backend in inspector_backends:
      webviews.append(WebContents(inspector_backend))
    return webviews

  def WaitForDocumentReadyStateToBeComplete(
      self, timeout=DEFAULT_WEB_CONTENTS_TIMEOUT):
    """Waits for the document to finish loading.

    Raises:
      exceptions.Error: See WaitForJavaScriptCondition() for a detailed list
      of possible exceptions.
    """

    self.WaitForJavaScriptCondition(
        'document.readyState == "complete"', timeout=timeout)

  def WaitForDocumentReadyStateToBeInteractiveOrBetter(
      self, timeout=DEFAULT_WEB_CONTENTS_TIMEOUT):
    """Waits for the document to be interactive.

    Raises:
      exceptions.Error: See WaitForJavaScriptCondition() for a detailed list
      of possible exceptions.
    """
    self.WaitForJavaScriptCondition(
        'document.readyState == "interactive" || '
        'document.readyState == "complete"', timeout=timeout)

  def WaitForFrameToBeDisplayed(self, timeout=DEFAULT_WEB_CONTENTS_TIMEOUT):
    """Waits for a frame to be displayed before returning.

    Raises:
      exceptions.Error: See WaitForJavaScriptCondition() for a detailed list
      of possible exceptions.
    """
    # Generate a new id for each call of this function to ensure that we track
    # each request to wait seperately.
    self._wait_for_frame_id += 1
    self.WaitForJavaScriptCondition(
        '{{ @script }}; window.__telemetry_testHasFramePassed({{ frame_id }})',
        script=self._wait_for_frame_js,
        frame_id=str(self._wait_for_frame_id),  # Place id as a str.
        timeout=timeout)

  def HasReachedQuiescence(self):
    """Determine whether the page has reached quiescence after loading.

    Returns:
      True if 2 seconds have passed since last resource received, false
      otherwise.
    Raises:
      exceptions.Error: See EvaluateJavaScript() for a detailed list of
      possible exceptions.
    """
    # Inclusion of the script that provides
    # window.__telemetry_testHasReachedNetworkQuiescence()
    # is idempotent, it's run on every call because WebContents doesn't track
    # page loads and we need to execute anew for every newly loaded page.
    return self.EvaluateJavaScript(
        '{{ @script }}; window.__telemetry_testHasReachedNetworkQuiescence()',
        script=self._quiescence_js)

  def _GetServiceWorkerState(self):
    """Returns service worker registration state.

    Returns:
      ServiceWorkerState if service worker registration state is not unexpected.
    Raises:
      exceptions.EvaluateException
      exceptions.Error: See EvaluateJavaScript() for a detailed list of
      possible exceptions.
    """
    state = self.EvaluateJavaScript(
        '{{ @script }}; window.__telemetry_getServiceWorkerState()',
        script=self._wait_for_serviceworker_js)
    if state in {ServiceWorkerState.NOT_REGISTERED,
                 ServiceWorkerState.INSTALLING, ServiceWorkerState.ACTIVATED}:
      return state
    raise exceptions.EvaluateException(state)

  def IsServiceWorkerActivatedOrNotRegistered(self):
    """Returns whether service worker is ready or not.

    Returns:
      True if the page registered a service worker and has an activated service
      worker. Also returns true if the page does not register service worker.
    Raises:
      exceptions.Error: See EvaluateJavaScript() for a detailed list of
      possible exceptions.
    """
    return self._GetServiceWorkerState() in {ServiceWorkerState.NOT_REGISTERED,
                                             ServiceWorkerState.ACTIVATED}

  def ExecuteJavaScript(self, *args, **kwargs):
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
      exceptions.EvaluationException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.ExecuteJavaScript(*args, **kwargs)

  def EvaluateJavaScript(self, *args, **kwargs):
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
      Additional keyword arguments provide values to be interpolated within
          the expression. See telemetry.util.js_template for details.

    Raises:
      py_utils.TimeoutException
      exceptions.EvaluationException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.EvaluateJavaScript(*args, **kwargs)

  def WaitForJavaScriptCondition(self, *args, **kwargs):
    """Wait for a JavaScript condition to become true.

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

    Raises:
      py_utils.TimeoutException
      exceptions.EvaluationException
      exceptions.WebSocketException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.WaitForJavaScriptCondition(*args, **kwargs)

  def AddTimelineMarker(self, marker):
    """Inject a marker in the timeline recorded during tracing.

    Args:
      marker: A string used to identify the marker in the trace.
    """
    return self._inspector_backend.AddTimelineMarker(marker)

  def EnableAllContexts(self):
    """Enable all contexts in a page. Returns all activated context ids.

    It's worth noting that this method may not reflect the state of the
    browser in real time: some of the returned contexts may no longer exist, or
    new contexts may have been activated.

    Returns:
      A list of frame context ids. Each |context_id| can be used for
      executing Javascript in the corresponding iframe through
      tab.ExecuteJavaScript(..., context_id=context_id) and
      tab.EvaluateJavaScript(..., context_id=context_id).

    Raises:
      exceptions.WebSocketDisconnected
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.EnableAllContexts()

  def WaitForNavigate(self, timeout=DEFAULT_WEB_CONTENTS_TIMEOUT):
    """Waits for the navigation to complete.

    The current page is expect to be in a navigation.
    This function returns when the navigation is complete or when
    the timeout has been exceeded.

    Raises:
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    self._inspector_backend.WaitForNavigate(timeout)

  def Navigate(self, url, script_to_evaluate_on_commit=None,
               timeout=DEFAULT_WEB_CONTENTS_TIMEOUT):
    """Navigates to url.

    If |script_to_evaluate_on_commit| is given, the script source string will be
    evaluated when the navigation is committed. This is after the context of
    the page exists, but before any script on the page itself has executed.

    Raises:
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    if not script_to_evaluate_on_commit:
      script_to_evaluate_on_commit = ''
    script_to_evaluate_on_commit = (
        self._quiescence_js + self._wait_for_serviceworker_js +
        script_to_evaluate_on_commit)
    self._inspector_backend.Navigate(url, script_to_evaluate_on_commit, timeout)

  def IsAlive(self):
    """Whether the WebContents is still operating normally.

    Since WebContents function asynchronously, this method does not guarantee
    that the WebContents will still be alive at any point in the future.

    Returns:
      A boolean indicating whether the WebContents is opearting normally.
    """
    return self._inspector_backend.IsInspectable()

  def CloseConnections(self):
    """Closes all TCP sockets held open by the browser.

    Raises:
      exceptions.DevtoolsTargetCrashException if the tab is not alive.
    """
    if not self.IsAlive():
      raise exceptions.DevtoolsTargetCrashException
    self.ExecuteJavaScript('window.chrome && chrome.benchmarking &&'
                           'chrome.benchmarking.closeConnections()')

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
      x_distance: Distance to scroll along the X axis (positive to scroll left).
      y_distance: Ddistance to scroll along the Y axis (positive to scroll up).
      x_overscroll: Number of additional pixels to scroll back along the X axis.
      y_overscroll: Number of additional pixels to scroll back along the Y axis.
      prevent_fling: Prevents a fling gesture.
      speed: Swipe speed in pixels per second.
      gesture_source_type: Which type of input events to be generated.
      repeatCount: Number of additional repeats beyond the first scroll.
      repeat_delay_ms: Number of milliseconds delay between each repeat.
      interaction_marker_name: The name of the interaction markers to generate.

    Raises:
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.SynthesizeScrollGesture(
        x=x, y=y, x_distance=x_distance, y_distance=y_distance,
        x_overscroll=x_overscroll, y_overscroll=y_overscroll,
        prevent_fling=prevent_fling, speed=speed,
        gesture_source_type=gesture_source_type, repeat_count=repeat_count,
        repeat_delay_ms=repeat_delay_ms,
        interaction_marker_name=interaction_marker_name,
        timeout=timeout)

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
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.DispatchKeyEvent(
        key_event_type=key_event_type, modifiers=modifiers, timestamp=timestamp,
        text=text, unmodified_text=unmodified_text,
        key_identifier=key_identifier, dom_code=dom_code, dom_key=dom_key,
        windows_virtual_key_code=windows_virtual_key_code,
        native_virtual_key_code=native_virtual_key_code,
        auto_repeat=auto_repeat, is_keypad=is_keypad,
        is_system_key=is_system_key, timeout=timeout)

  def EnableCast(self, presentation_url=None, timeout=60):
    """Starts observing Cast-enabled sinks.

    Args:
      presentation_url: string, the URL for making a presentation request.

    Raises:
      py_utils.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.EnableCast(presentation_url, timeout=timeout)

  def GetCastSinks(self):
    """Returns a list of available Cast-enabled sinks.

    Returns:
      The list of sinks that supports Casting.
      Returns an empty list if there is no available sink.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.GetCastSinks()

  def GetCastIssue(self):
    """Returns the error message when there is an issue while casting.

    Returns:
      The same error message as in extension dialog.
      Returns an empty string if there is no error.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.GetCastIssue()

  def SetCastSinkToUse(self, sink_name, timeout=60):
    """Sets the sink to be used for a Cast session.

    Args:
      sink_name: string, name of the sink to start a casting session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.SetCastSinkToUse(sink_name, timeout=timeout)

  def StartTabMirroring(self, sink_name, timeout=60):
    """Starts a tab mirroring session.

    Args:
      sink_name: string, name of the sink to start a mirroring session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.StartTabMirroring(sink_name, timeout=timeout)

  def StopCasting(self, sink_name, timeout=60):
    """Stops all session on a specific Cast enabled sink.

    Args:
      sink_name: string, name of the sink to stop a session.

    Raises:
      exceptions.TimeoutException
      exceptions.DevtoolsTargetCrashException
    """
    return self._inspector_backend.StopCasting(sink_name, timeout=timeout)

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
    """
    return self._inspector_backend.StartMobileDeviceEmulation(
        width=width, height=height, dsr=dsr, timeout=timeout)

  def StopMobileDeviceEmulation(self, timeout=60):
    """Stops emulation of a mobile device."""
    return self._inspector_backend.StopMobileDeviceEmulation(timeout=timeout)
