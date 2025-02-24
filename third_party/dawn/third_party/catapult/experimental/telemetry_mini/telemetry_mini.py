#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This is intended to be a very trimmed down, single-file, hackable, and easy
# to understand version of Telemetry. It's able to run simple user stories on
# Android, grab traces, and extract metrics from them. May be useful to
# diagnose issues with Chrome, reproduce regressions or prototype new user
# stories.
#
# Known limitations: Does not use WPR, so it does need to hit the live network
# to load pages.

from __future__ import absolute_import
from __future__ import print_function
import collections
import contextlib
import functools
import six.moves.http_client
import json
import logging
import os
import pipes
import posixpath
import re
import socket
import subprocess
import tempfile
import time
import websocket  # pylint: disable=import-error
from xml.etree import ElementTree as element_tree
import six
from six.moves import range


KEYCODE_HOME = 3
KEYCODE_BACK = 4
KEYCODE_APP_SWITCH = 187

# Parse rectangle bounds given as: '[left,top][right,bottom]'.
RE_BOUNDS = re.compile(
    r'\[(?P<left>\d+),(?P<top>\d+)\]\[(?P<right>\d+),(?P<bottom>\d+)\]')

# TODO: Maybe replace with a true on-device temp file.
UI_DUMP_TEMP = '/data/local/tmp/tm_ui_dump.xml'


def RetryOn(exc_type=(), returns_falsy=False, retries=5):
  """Decorator to retry a function in case of errors or falsy values.

  Implements exponential backoff between retries.

  Args:
    exc_type: Type of exceptions to catch and retry on. May also pass a tuple
      of exceptions to catch and retry on any of them. Defaults to catching no
      exceptions at all.
    returns_falsy: If True then the function will be retried until it stops
      returning a "falsy" value (e.g. None, False, 0, [], etc.). If equal to
      'raise' and the function keeps returning falsy values after all retries,
      then the decorator will raise a ValueError.
    retries: Max number of retry attempts. After exhausting that number of
      attempts the function will be called with no safeguards: any exceptions
      will be raised and falsy values returned to the caller (except when
      returns_falsy='raise').
  """
  def Decorator(f):
    @functools.wraps(f)
    def Wrapper(*args, **kwargs):
      wait = 1
      this_retries = kwargs.pop('retries', retries)
      for _ in range(this_retries):
        retry_reason = None
        try:
          value = f(*args, **kwargs)
        except exc_type as exc:
          retry_reason = 'raised %s' % type(exc).__name__
        if retry_reason is None:
          if returns_falsy and not value:
            retry_reason = 'returned %r' % value
          else:
            return value  # Success!
        logging.info('%s %s, will retry in %d second%s ...',
                     f.__name__, retry_reason, wait, '' if wait == 1 else 's')
        time.sleep(wait)
        wait *= 2
      value = f(*args, **kwargs)  # Last try to run with no safeguards.
      if returns_falsy == 'raise' and not value:
        raise ValueError('%s returned %r' % (f.__name__, value))
      return value
    return Wrapper
  return Decorator


class AdbCommandError(Exception):
  pass


class AdbMini(object):
  ADB_BIN = 'adb'

  @classmethod
  def RunBaseCommand(cls, *args):
    cmd = [cls.ADB_BIN]
    cmd.extend(args)
    logging.info('$ adb %s', ' '.join(pipes.quote(a) for a in args))
    return subprocess.check_output(cmd, stderr=subprocess.STDOUT)

  @classmethod
  def GetDevices(cls):
    for line in cls.RunBaseCommand('devices').splitlines()[1:]:
      cols = line.split()
      if cols and cols[-1] == 'device':
        yield cls(cols[0])

  def __init__(self, serial):
    self.serial = serial

  def RunCommand(self, *args):
    return type(self).RunBaseCommand('-s', self.serial, *args)

  def RunShellCommand(self, *args):
    return self.RunCommand('shell', *args)

  def ListPath(self, path):
    return [
        line.split(' ')[-1]
        for line in self.RunCommand('ls', path).splitlines()]

  def WriteText(self, text, path):
    self.RunShellCommand(
        'echo -n %s > %s' % (pipes.quote(text), pipes.quote(path)))

  def ListPackages(self, name_filter=None, only_enabled=False):
    """Return a list of packages available on the device."""
    args = ['pm', 'list', 'packages']
    if only_enabled:
      args.append('-e')
    if name_filter:
      args.append(name_filter)
    lines = self.RunShellCommand(*args).splitlines()
    prefix = 'package:'
    return [line[len(prefix):] for line in lines if line.startswith(prefix)]

  def ProcessStatus(self):
    """Return a defaultdict mapping of {process_name: list_of_pids}."""
    result = collections.defaultdict(list)
    # TODO: May not work on earlier Android verions without -e support.
    for line in self.RunShellCommand('ps', '-e').splitlines():
      row = line.split(None, 8)
      try:
        pid = int(row[1])
        process_name = row[-1]
      except Exception:  # pylint: disable=broad-except
        continue
      result[process_name].append(pid)
    return result

  @RetryOn(AdbCommandError)
  def GetUiScreenDump(self):
    """Return the root XML node with screen captured from the device."""
    self.RunShellCommand('rm', '-f', UI_DUMP_TEMP)
    output = self.RunShellCommand('uiautomator', 'dump', UI_DUMP_TEMP).strip()

    if output.startswith('ERROR:'):
      # uiautomator may fail if device is not in idle state, e.g. animations
      # or video playing. Retry if that's the case.
      raise AdbCommandError(output)

    with tempfile.NamedTemporaryFile(suffix='.xml') as f:
      f.close()
      self.RunCommand('pull', UI_DUMP_TEMP, f.name)
      return element_tree.parse(f.name)

  def HasUiElement(self, attr_values):
    """Check whether a UI element is visible on the screen."""
    root = self.GetUiScreenDump()
    for node in root.iter():
      if all(node.get(k) == v for k, v in attr_values):
        return node
    return None

  @RetryOn(LookupError)
  def FindUiElement(self, *args, **kwargs):
    """Find a UI element on the screen, retrying if not yet visible."""
    node = self.HasUiElement(*args, **kwargs)
    if node is None:
      raise LookupError('Specified UI element not found')
    return node

  def TapUiElement(self, *args, **kwargs):
    """Tap on a UI element found on screen."""
    node = self.FindUiElement(*args, **kwargs)
    m = RE_BOUNDS.match(node.get('bounds'))
    left, top, right, bottom = (int(v) for v in m.groups())
    x, y = (left + right) / 2, (top + bottom) / 2
    self.RunShellCommand('input', 'tap', str(x), str(y))


def _UserAction(f):
  """Decorator to add repeat, and action_delay options to user action methods.

  Note: It's advisable for decorated methods to include a catch-all **kwargs,
  even if just to check it's empty.

  This is a workaround for https://github.com/PyCQA/pylint/issues/258 in which
  decorators confuse pylint and trigger spurious 'unexpected-keyword-arg'
  on method calls that use the extra options provided by this decorator.
  """
  @functools.wraps(f)
  def Wrapper(self, *args, **kwargs):
    repeat = kwargs.pop('repeat', 1)
    action_delay = kwargs.pop('action_delay', None)
    for _ in range(repeat):
      f(self, *args, **kwargs)
      self.Idle(action_delay)
  return Wrapper


class AndroidActions(object):
  APP_SWITCHER_CLEAR_ALL = [
      ('resource-id', 'com.android.systemui:id/button'),
      ('text', 'CLEAR ALL')]
  APP_SWITCHER_NO_RECENT = [
      ('package', 'com.android.systemui'),
      ('text', 'No recent items')]

  def __init__(self, device, user_action_delay=1):
    self.device = device
    self.user_action_delay = user_action_delay

  def Idle(self, duration=None):
    if duration is None:
      duration = self.user_action_delay
    if duration:
      time.sleep(duration)

  @_UserAction
  def GoHome(self, **kwargs):
    assert not kwargs  # See @_UserAction
    self.device.RunShellCommand('input', 'keyevent', str(KEYCODE_HOME))

  @_UserAction
  def GoBack(self, **kwargs):
    assert not kwargs  # See @_UserAction
    self.device.RunShellCommand('input', 'keyevent', str(KEYCODE_BACK))

  @_UserAction
  def GoAppSwitcher(self, **kwargs):
    assert not kwargs  # See @_UserAction
    self.device.RunShellCommand('input', 'keyevent', str(KEYCODE_APP_SWITCH))

  @_UserAction
  def StartActivity(
      self, data_uri, action='android.intent.action.VIEW', **kwargs):
    assert not kwargs  # See @_UserAction
    self.device.RunShellCommand('am', 'start', '-a', action, '-d', data_uri)

  @_UserAction
  def TapUiElement(self, attr_values, **kwargs):
    self.device.TapUiElement(attr_values, **kwargs)

  def TapHomeScreenShortcut(self, description, **kwargs):
    self.TapUiElement([
        ('package', 'com.android.launcher3'),
        ('class', 'android.widget.TextView'),
        ('content-desc', description)
    ], **kwargs)

  def TapAppSwitcherTitle(self, text, **kwargs):
    self.TapUiElement([
        ('resource-id', 'com.android.systemui:id/title'),
        ('text', text)
    ], **kwargs)

  def TapAppSwitcherClearAll(self, **kwargs):
    self.TapUiElement(self.APP_SWITCHER_CLEAR_ALL, **kwargs)

  @_UserAction
  def SwipeUp(self, **kwargs):
    assert not kwargs  # See @_UserAction
    # Hardcoded values for 480x854 screen size; should work reasonably on
    # other screen sizes.
    # Command args: swipe <x1> <y1> <x2> <y2> [duration(ms)]
    self.device.RunShellCommand(
        'input', 'swipe', '240', '568', '240', '284', '400')

  @_UserAction
  def SwipeDown(self, **kwargs):
    assert not kwargs  # See @_UserAction
    # Hardcoded values for 480x854 screen size; should work reasonably on
    # other screen sizes.
    # Command args: swipe <x1> <y1> <x2> <y2> [duration(ms)]
    self.device.RunShellCommand(
        'input', 'swipe', '240', '284', '240', '568', '400')

  def ClearRecentApps(self):
    self.GoAppSwitcher()
    if self.device.HasUiElement(self.APP_SWITCHER_NO_RECENT):
      self.GoHome()
    else:
      # Sometimes we need to swipe down several times until the "Clear All"
      # button becomes visible.
      for _ in range(5):
        try:
          self.TapAppSwitcherClearAll(retries=0)  # If not found raise error.
          return  # Success!
        except LookupError:
          self.SwipeDown()  # Swipe down a bit more.
      self.TapAppSwitcherClearAll()  # Last try! If not found raises error.


class JavaScriptError(Exception):
  pass


class DevToolsWebSocket(object):
  def __init__(self, url):
    self._url = url
    self._socket = None
    self._cmdid = 0

  def __enter__(self):
    self.Open()
    return self

  def __exit__(self, *args, **kwargs):
    self.Close()

  @RetryOn(socket.error)
  def Open(self):
    assert self._socket is None
    self._socket = websocket.create_connection(self._url)

  def Close(self):
    if self._socket is not None:
      self._socket.close()
      self._socket = None

  def Send(self, method, **kwargs):
    logging.info(
        '%s: %s(%s)', self._url, method,
        ', '.join('%s=%r' % (k, v) for k, v in sorted(six.iteritems(kwargs))))
    self._cmdid += 1
    self._socket.send(json.dumps(
        {'id': self._cmdid, 'method': method, 'params': kwargs}))
    resp = self.Recv()
    assert resp['id'] == self._cmdid
    return resp.get('result')

  def Recv(self):
    return json.loads(self._socket.recv())

  def EvaluateJavaScript(self, expression):
    resp = self.Send(
        'Runtime.evaluate', expression=expression, returnByValue=True)
    if 'exceptionDetails' in resp:
      raise JavaScriptError(resp['result']['description'])
    return resp['result'].get('value')

  @RetryOn(returns_falsy='raise')
  def WaitForJavaScriptCondition(self, *args, **kwargs):
    return self.EvaluateJavaScript(*args, **kwargs)

  def RequestMemoryDump(self):
    resp = self.Send('Tracing.requestMemoryDump')
    assert resp['success']

  def CollectTrace(self, trace_file):
    """Stop tracing and collect the trace."""
    with open(trace_file, 'wb') as f:
      # Call to Tracing.start is needed to update the transfer mode.
      self.Send('Tracing.start', transferMode='ReturnAsStream', traceConfig={})
      self.Send('Tracing.end')
      resp = self.Recv()
      assert resp['method'] == 'Tracing.tracingComplete'
      stream_handle = resp['params']['stream']
      try:
        resp = {'eof': False}
        num_bytes = 0
        while not resp['eof']:
          resp = self.Send('IO.read', handle=stream_handle)
          data = resp['data'].encode('utf-8')
          f.write(data)
          num_bytes += len(data)
        logging.info(
            'Collected trace of %.1f MiB', float(num_bytes) / (1024 * 1024))
      finally:
        self.Send('IO.close', handle=stream_handle)


class AndroidApp(object):
  # Override this value with path to directory where APKs to install are found.
  APKS_DIR = NotImplemented

  PACKAGE_NAME = NotImplemented
  APK_FILENAME = None

  def __init__(self, device):
    self.device = device

  def ForceStop(self):
    self.device.RunShellCommand('am', 'force-stop', self.PACKAGE_NAME)

  def Install(self):
    assert self.APK_FILENAME is not None, 'No APK to install available'
    apk_path = os.path.join(self.APKS_DIR, self.APK_FILENAME)
    logging.warning('Installing %s from %s', self.PACKAGE_NAME, apk_path)
    assert os.path.isfile(apk_path), 'File not found: %s' % apk_path
    self.device.RunCommand('install', '-r', '-d', apk_path)

  def Uninstall(self):
    logging.warning('Uninstalling %s', self.PACKAGE_NAME)
    self.device.RunCommand('uninstall', self.PACKAGE_NAME)


class ChromiumApp(AndroidApp):
  PACKAGE_NAME = 'org.chromium.chrome'
  APK_FILENAME = 'ChromePublic.apk'
  COMMAND_LINE_FILE = '/data/local/tmp/chrome-command-line'
  TRACE_CONFIG_FILE = '/data/local/chrome-trace-config.json'

  def __init__(self, *args, **kwargs):
    super(ChromiumApp, self).__init__(*args, **kwargs)
    self._devtools_local_port = None
    self._browser_flags = None
    self._trace_config = None
    self.startup_time = None

  def RemoveProfile(self):
    # TODO: Path to profile may need to be updated on newer Android versions.
    profile_dir = posixpath.join('/data/data', self.PACKAGE_NAME)
    filenames = self.device.ListPath(profile_dir)
    args = ['rm', '-r']
    args.extend(
        posixpath.join(profile_dir, f)
        for f in filenames if f not in ['.', '..', 'lib'])
    self.device.RunShellCommand(*args)

  @contextlib.contextmanager
  def CommandLineFlags(self):
    command_line = ' '.join(['_'] + self._browser_flags)
    self.device.WriteText(command_line, self.COMMAND_LINE_FILE)
    try:
      yield
    finally:
      self.device.RunShellCommand('rm', '-f', self.COMMAND_LINE_FILE)

  def SetBrowserFlags(self, browser_flags):
    self._browser_flags = browser_flags

  def SetTraceConfig(self, trace_config):
    self._trace_config = trace_config

  def SetDevToolsLocalPort(self, port):
    self._devtools_local_port = port

  def GetDevToolsLocalAddr(self, host='localhost'):
    assert self._devtools_local_port is not None
    return '%s:%d' % (host, self._devtools_local_port)

  def GetDevToolsRemoteAddr(self):
    return 'localabstract:chrome_devtools_remote'

  @contextlib.contextmanager
  def PortForwarding(self):
    """Setup port forwarding to connect with DevTools on remote device."""
    local = self.GetDevToolsLocalAddr('tcp')
    remote = self.GetDevToolsRemoteAddr()
    self.device.RunCommand('forward', '--no-rebind', local, remote)
    try:
      yield
    finally:
      self.device.RunCommand('forward', '--remove', local)

  @contextlib.contextmanager
  def StartupTracing(self):
    self.device.WriteText(
        json.dumps({'trace_config': self._trace_config}),
        self.TRACE_CONFIG_FILE)
    try:
      yield
    finally:
      self.device.RunShellCommand('rm', '-f', self.TRACE_CONFIG_FILE)

  @contextlib.contextmanager
  def Session(self):
    """A context manager to guard the lifetime of a browser process.

    Ensures that command line flags and port forwarding are ready, the browser
    is not alive before starting, it has a clear profile to begin with, and is
    finally closed when done.

    It does not, however, launch the browser itself. This must be done by the
    context managed code.

    To the extent possible, measurements from browsers launched within
    different sessions are meant to be independent of each other.
    """
    # Removing the profile breaks Chrome Shortcuts on the Home Screen.
    # TODO: Figure out a way to automatically create the shortcuts before
    # running the story.
    # self.RemoveProfile()
    with self.CommandLineFlags():
      with self.StartupTracing():
        # Ensure browser is closed after setting command line flags and
        # trace config to ensure they are read on startup.
        self.ForceStop()
        with self.PortForwarding():
          try:
            yield
          finally:
            self.ForceStop()

  def WaitForCurrentPageReady(self):
    with self.CurrentPage() as page_dev:
      page_dev.WaitForJavaScriptCondition('document.readyState == "complete"')

  def IterPages(self):
    """Iterate over inspectable pages available through DevTools.

    Note: does not actually connect to the page until you "enter" it within
    a managed context.
    """
    for page in self.DevToolsRequest():
      if page['type'] == 'page':
        yield self.DevToolsSocket(page['webSocketDebuggerUrl'])

  def CurrentPage(self):
    """Get a DevToolsWebSocket to the current page on the browser."""
    return next(self.IterPages())

  def CollectTrace(self, trace_file):
    with self.DevToolsSocket() as browser_dev:
      browser_dev.CollectTrace(trace_file)

  def DevToolsSocket(self, path='browser'):
    # TODO(crbug.com/753842): Default browser path may need to be adjusted
    # to include GUID.
    if path.startswith('ws://'):
      url = path
    else:
      url = ('ws://%s/devtools/' % self.GetDevToolsLocalAddr()) + path
    return DevToolsWebSocket(url)

  @RetryOn(socket.error)
  def DevToolsRequest(self, path=''):
    url = '/json'
    if path:
      url = posixpath.join(url, path)

    conn = six.moves.http_client.HTTPConnection(self.GetDevToolsLocalAddr())
    try:
      conn.request('GET', url)
      return json.load(conn.getresponse())
    finally:
      conn.close()


class ChromeApp(ChromiumApp):
  PACKAGE_NAME = 'com.google.android.apps.chrome'
  APK_FILENAME = 'Chrome.apk'


class SystemChromeApp(ChromiumApp):
  PACKAGE_NAME = 'com.android.chrome'
  APK_FILENAME = None

  def Install(self):
    # System Chrome app cannot be (un)installed, so we enable/disable instead.
    logging.warning('Enabling %s', self.PACKAGE_NAME)
    self.device.RunShellCommand('pm', 'enable', self.PACKAGE_NAME)

  def Uninstall(self):
    # System Chrome app cannot be (un)installed, so we enable/disable instead.
    logging.warning('Disabling %s', self.PACKAGE_NAME)
    self.device.RunShellCommand('pm', 'disable', self.PACKAGE_NAME)


class UserStory(object):
  def __init__(self, browser):
    self.device = browser.device
    self.browser = browser
    self.actions = AndroidActions(self.device)

  def Run(self, trace_file):
    with self.browser.Session():
      self.RunPrepareSteps()
      try:
        self.RunStorySteps()
        self.browser.CollectTrace(trace_file)
      except Exception as exc:
        # Helps to pin point in the logs the moment where the story failed,
        # before any of the finally blocks get to be executed.
        logging.error('Aborting story due to %s.', type(exc).__name__)
        raise
      finally:
        self.RunCleanupSteps()

  def RunPrepareSteps(self):
    """Subclasses may override to perform actions before running the story."""

  def RunStorySteps(self):
    """Subclasses should override this method to implement the story.

    The steps must:
    - at some point cause the browser to be launched, and
    - make sure the browser remains alive when done (even if backgrounded).
    """
    raise NotImplementedError

  def RunCleanupSteps(self):
    """Subclasses may override to perform actions after running the story.

    Note: This will be called even if an exception was raised during the
    execution of RunStorySteps (but not for errors in RunPrepareSteps).
    """


def ReadProcessMetrics(trace_file):
  """Return a list of {"name": process_name, metric: value} dicts."""
  with open(trace_file) as f:
    trace = json.load(f)

  processes = collections.defaultdict(dict)
  for event in trace['traceEvents']:
    if event['ph'] == 'v':
      # Extract any metrics you may need from the trace.
      value = event['args']['dumps']['allocators'][
          'java_heap/allocated_objects']['attrs']['size']
      assert value['units'] == 'bytes'
      processes[event['pid']]['java_heap'] = int(value['value'], 16)
    elif event['ph'] == 'M' and event['name'] == 'process_name':
      processes[event['pid']]['name'] = event['args']['name']

  return list(processes.values())


def RunStories(browser, stories, repeat, output_dir):
  for repeat_idx in range(1, repeat + 1):
    for story_cls in stories:
      trace_file = os.path.join(
          output_dir, 'trace_%s_%d.json' % (story_cls.NAME, repeat_idx))
      print('[ RUN      ]', story_cls.NAME)
      status = '[       OK ]'
      start = time.time()
      try:
        story_cls(browser).Run(trace_file)
      except Exception:  # pylint: disable=broad-except
        logging.exception('Exception raised while running story')
        status = '[  FAILED  ]'
      finally:
        elapsed = '(%.1f secs)' % (time.time() - start)
        print(status, story_cls.NAME, elapsed)
