# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import logging
import sys

import six


class Error(Exception):
  """Base class for Telemetry exceptions."""

  def __init__(self, msg=''):
    super().__init__(msg)
    self._debugging_messages = []

  def AddDebuggingMessage(self, msg):
    """Adds a message to the description of the exception.

    Many Telemetry exceptions arise from failures in another application. These
    failures are difficult to pinpoint. This method allows Telemetry classes to
    append useful debugging information to the exception. This method also logs
    information about the location from where it was called.
    """
    frame = sys._getframe(1)
    line_number = frame.f_lineno
    file_name = frame.f_code.co_filename
    function_name = frame.f_code.co_name
    call_site = '%s:%s %s' % (file_name, line_number, function_name)
    annotated_message = '(%s) %s' % (call_site, msg)

    self._debugging_messages.append(annotated_message)

  def _get_debugging_messages(self, base_output):
    divider = '\n' + '*' * 80 + '\n'
    output = base_output
    for message in self._debugging_messages:
      output += divider
      output += message
    return output

  def __str__(self):
    return self._get_debugging_messages(super().__str__())

  def __repr__(self):
    return self._get_debugging_messages(super().__repr__())


class PlatformError(Error):
  """ Represents an exception thrown when constructing platform. """


class TimeoutException(Error):
  """The operation failed to complete because of a timeout.

  It is possible that waiting for a longer period of time would result in a
  successful operation.
  """


class AppCrashException(Error):

  def __init__(self, app=None, msg=''):
    super().__init__(msg)
    self._msg = msg
    self._is_valid_dump = False
    self._stack_trace = []
    self._app_stdout = []
    self._system_log = '(Not implemented)'

    if app:
      debug_data = app.CollectDebugData(logging.ERROR)
      self._system_log = debug_data.system_log or self._system_log
      if not isinstance(self._system_log, six.string_types):
        self._system_log = self._system_log.decode('utf-8')
      self._app_stdout = debug_data.stdout
      if not isinstance(self._app_stdout, six.string_types):
        self._app_stdout = self._app_stdout.decode('utf-8')
      self._app_stdout = self._app_stdout.splitlines()
      self._is_valid_dump = bool(debug_data.symbolized_minidumps)
      self._stack_trace = '\n'.join(
          debug_data.symbolized_minidumps).splitlines()


  @property
  def stack_trace(self):
    return self._stack_trace

  @property
  def is_valid_dump(self):
    return self._is_valid_dump

  def __str__(self):
    divider = '*' * 80
    debug_messages = []
    debug_messages.append(super().__str__())
    debug_messages.append('Found Minidump: %s' % self._is_valid_dump)
    debug_messages.append('Stack Trace:')
    debug_messages.append(divider)
    if self.is_valid_dump:
      # CollectDebugData will handle this already.
      debug_messages.append('Stack trace(s) already output above.')
    else:
      debug_messages.append('Unable to get stack trace.')
    debug_messages.append(divider)
    debug_messages.append('Standard output:')
    debug_messages.append(divider)
    debug_messages.extend(('\t%s' % l) for l in self._app_stdout)
    debug_messages.append(divider)
    debug_messages.append('System log:')
    debug_messages.append(self._system_log)
    return '\n'.join(debug_messages)

class DevtoolsTargetCrashException(AppCrashException):
  """Represents a crash of the current devtools target but not the overall app.

  This can be a tab or a WebView. In this state, the tab/WebView is
  gone, but the underlying browser is still alive.
  """

  def __init__(self, app, msg='Devtools target crashed'):
    super().__init__(app, msg)

class DevtoolsTargetClosedException(Error):
  """Represents an error when Devtools target navigated or closed.
  """

class BrowserGoneException(AppCrashException):
  """Represents a crash of the entire browser.

  In this state, all bets are pretty much off."""

  def __init__(self, app, msg='Browser crashed'):
    super().__init__(app, msg)


class BrowserConnectionGoneException(BrowserGoneException):
  """Represents a browser that still exists but cannot be reached."""

  def __init__(self, app, msg='Browser exists but the connection is gone'):
    super().__init__(app, msg)


class TabMissingError(Error):
  """Represents an error when an expected browser tab is not found."""


class ProcessGoneException(Error):
  """Represents a process that no longer exists for an unknown reason."""


class IntentionalException(Error):
  """Represent an exception raised by a unittest which is not printed."""


class InitializationError(Error):

  def __init__(self, string):
    super().__init__(string)


class LoginException(Error):
  pass


class EvaluateException(Error):
  def __init__(self, text='', class_name='', description=None):
    super().__init__(text)
    self._class_name = class_name
    self._description = description

  def __str__(self):
    output = super().__str__()
    if self._class_name and self._description:
      output += '%s:\n%s' % (self._class_name, self._description)
    return output


class StoryActionError(Error):
  """Represents an error when trying to perform an action on a story."""


class TracingException(Error):
  """Represents an error that ocurred while collecting or flushing traces."""


class AtraceTracingError(TracingException):
  """Represents an error that ocurred while collecting traces with Atrace."""


class PathMissingError(Error):
  """Represents an exception thrown when an expected path doesn't exist."""


class UnknownPackageError(Error):
  """Represents an exception when encountering an unsupported Android APK."""


class PackageDetectionError(Error):
  """Represents an error when parsing an Android APK's package."""


class AndroidDeviceParsingError(Error):
  """Represents an error when parsing output from an android device."""
