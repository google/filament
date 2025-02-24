# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six

from py_trace_event import trace_event

from telemetry import decorators
from telemetry.util import js_template

GESTURE_SOURCE_DEFAULT = 'DEFAULT'
GESTURE_SOURCE_MOUSE = 'MOUSE'
GESTURE_SOURCE_TOUCH = 'TOUCH'
SUPPORTED_GESTURE_SOURCES = (GESTURE_SOURCE_DEFAULT, GESTURE_SOURCE_MOUSE,
                             GESTURE_SOURCE_TOUCH)
INPUT_EVENT_PATTERN_DEFAULT = 'DEFAULT'
INPUT_EVENT_PATTERN_ONE_PER_VSYNC = 'ONE_PER_VSYNC'
INPUT_EVENT_PATTERN_TWO_PER_VSYNC = 'TWO_PER_VSYNC'
INPUT_EVENT_PATTERN_EVERY_OTHER_VSYNC = 'EVERY_OTHER_VSYNC'
SUPPORTED_INPUT_EVENT_PATTERNS = (INPUT_EVENT_PATTERN_DEFAULT,
                                  INPUT_EVENT_PATTERN_ONE_PER_VSYNC,
                                  INPUT_EVENT_PATTERN_TWO_PER_VSYNC,
                                  INPUT_EVENT_PATTERN_EVERY_OTHER_VSYNC)
DEFAULT_TIMEOUT = 60


class PageActionNotSupported(Exception):
  pass


class PageActionFailed(Exception):
  pass


class PageAction(six.with_metaclass(trace_event.TracedMetaClass, object)):
  """Represents an action that a user might try to perform to a page."""

  def __init__(self, timeout=DEFAULT_TIMEOUT):
    """Initialization function.

    Args:
      timeout: number of seconds to wait for the action to finish.
    """
    self.timeout = timeout

  def WillRunAction(self, tab):
    """Override to do action-specific setup before
    Test.WillRunAction is called."""

  def RunAction(self, tab):
    raise NotImplementedError()

  def CleanUp(self, tab):
    pass

  def __str__(self):
    return self.__class__.__name__


class ElementPageAction(PageAction):
  """A PageAction which acts on DOM elements"""

  def __init__(self, selector=None, text=None, element_function=None,
               timeout=DEFAULT_TIMEOUT):
    super().__init__(timeout)
    self._selector = selector
    self._text = text
    self._element_function = element_function

  def RunAction(self, tab):
    raise NotImplementedError()

  def HasElementSelector(self):
    return (self._selector is not None or
            self._text is not None or
            self._element_function is not None)

  def EvaluateCallback(self, tab, code, **kwargs):
    return EvaluateCallbackWithElement(
        tab, code, selector=self._selector, text=self._text,
        element_function=self._element_function, **kwargs)

  def __str__(self):
    query_string = ''
    if self._selector is not None:
      query_string = self._selector
    if self._text is not None:
      query_string = 'text=%s' % self._text
    if self._element_function:
      query_string = 'element_function=%s' % self._element_function
    return '%s(%s)' % (self.__class__.__name__, query_string)


def EvaluateCallbackWithElement(
    tab, callback_js, selector=None, text=None, element_function=None,
    wait=False, timeout_in_seconds=DEFAULT_TIMEOUT, user_gesture=False):
  """Evaluates the JavaScript callback with the given element.

  The element may be selected via selector, text, or element_function.
  Only one of these arguments must be specified.

  Returns:
    The callback's return value, if any. The return value must be
    convertible to JSON.

  Args:
    tab: A telemetry.core.Tab object.
    callback_js: The JavaScript callback to call (as string).
        The callback receive 2 parameters: the element, and information
        string about what method was used to retrieve the element.
        Example: '''
          function(element, info) {
            if (!element) {
              throw Error('Can not find element: ' + info);
            }
            element.click()
          }'''
    selector: A CSS selector describing the element.
    text: The element must contains this exact text.
    element_function: A JavaScript function (as string) that is used
        to retrieve the element. For example:
        '(function() { return foo.element; })()'.
    wait: Whether to wait for the return value to be true.
    timeout_in_seconds: The timeout for wait (if waiting).
    user_gesture: Whether execution should be treated as initiated by user
        in the UI. Code that plays media or requests fullscreen may not take
        effects without user_gesture set to True.
  """
  count = 0
  info_msg = ''
  if element_function is not None:
    count = count + 1
    info_msg = js_template.Render(
        'using element_function: {{ @code }}', code=element_function)
  if selector is not None:
    count = count + 1
    info_msg = js_template.Render(
        'using selector {{ selector }}', selector=selector)
    element_function = js_template.Render(
        'document.querySelector({{ selector }})', selector=selector)
  if text is not None:
    count = count + 1
    info_msg = js_template.Render(
        'using exact text match {{ text }}', text=text)
    element_function = js_template.Render(
        """
        (function() {
          function _findElement(element, text) {
            if (element.innerHTML == text) {
              return element;
            }

            var childNodes = element.childNodes;
            for (var i = 0, len = childNodes.length; i < len; ++i) {
              var found = _findElement(childNodes[i], text);
              if (found) {
                return found;
              }
            }
            return null;
          }
          return _findElement(document, {{ text }});
        })()""",
        text=text)

  if count != 1:
    raise PageActionFailed(
        'Must specify 1 way to retrieve element, but %s was specified.' % count)

  code = js_template.Render(
      """
      (function() {
        var element = {{ @element_function }};
        var callback = {{ @callback_js }};
        return callback(element, {{ info_msg }});
      })()""",
      element_function=element_function,
      callback_js=callback_js,
      info_msg=info_msg)

  if wait:
    tab.WaitForJavaScriptCondition(code, timeout=timeout_in_seconds)
    return True
  return tab.EvaluateJavaScript(code, user_gesture=user_gesture)


@decorators.Cache
def IsGestureSourceTypeSupported(tab, gesture_source_type):
  # TODO(dominikg): remove once support for
  #                 'chrome.gpuBenchmarking.gestureSourceTypeSupported' has
  #                 been rolled into reference build.
  if tab.EvaluateJavaScript("""
      typeof chrome.gpuBenchmarking.gestureSourceTypeSupported ===
          'undefined'"""):
    return (tab.browser.platform.GetOSName() != 'mac' or
            gesture_source_type.lower() != 'touch')

  return tab.EvaluateJavaScript(
      """
      chrome.gpuBenchmarking.gestureSourceTypeSupported(
          chrome.gpuBenchmarking.{{ @gesture_source_type }}_INPUT)""",
      gesture_source_type=gesture_source_type.upper())
