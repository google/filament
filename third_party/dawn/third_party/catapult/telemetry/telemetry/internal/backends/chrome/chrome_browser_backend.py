# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import hashlib
import logging
import os
import pprint
import shlex
import shutil
import socket
import tempfile

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.backends import browser_backend
from telemetry.internal.backends.chrome import extension_backend
from telemetry.internal.backends.chrome import tab_list_backend
from telemetry.internal.backends.chrome_inspector import devtools_client_backend
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import ui_devtools_client_backend
from telemetry.internal.browser import web_contents

import py_utils


class ChromeBrowserBackend(browser_backend.BrowserBackend):
  """An abstract class for chrome browser backends. Provides basic functionality
  once a remote-debugger port has been established."""
  # It is OK to have abstract methods. pylint: disable=abstract-method

  def __init__(self, platform_backend, browser_options,
               browser_directory, profile_directory,
               supports_extensions, supports_tab_control, build_dir=None,
               enable_tracing=True):
    """
    Args:
      platform_backend: The platform_backend.PlatformBackend instance to use.
      browser_options: The browser_options.BrowserOptions instance to use.
      browser_directory: A string containing a path to the directory where the
          the browser is installed. This is typically the directory containing
          the browser executable, but not guaranteed.
      profile_directory: A string containing a path to the directory to store
          browser profile information in.
      supports_extensions: A boolean indicating whether the browser supports
          extensions.
      supports_tab_control: A boolean indicating whether the browser supports
          the concept of tabs.
      build_dir: A string containing a path to the directory that the browser
          was built in, for finding debug artifacts. Can be None if the browser
          was not locally built, or the directory otherwise cannot be
          determined.
      enable_tracing: Defines if a tracing_client is created.
    """
    super().__init__(
        platform_backend=platform_backend,
        browser_options=browser_options,
        supports_extensions=supports_extensions,
        tab_list_backend=tab_list_backend.TabListBackend)
    self._browser_directory = browser_directory
    self._enable_tracing = enable_tracing
    self._profile_directory = profile_directory
    self._supports_tab_control = supports_tab_control
    self._build_dir = build_dir

    self._devtools_client = None
    self._ui_devtools_client = None

    self._extensions_to_load = browser_options.extensions_to_load
    if not supports_extensions and len(self._extensions_to_load) > 0:
      raise browser_backend.ExtensionsNotSupportedException(
          'Extensions are not supported on the selected browser')

    if self.browser_options.dont_override_profile:
      logging.warning('Not overriding profile. This can cause unexpected '
                      'effects due to profile-specific settings, such as '
                      'about:flags settings, cookies, and extensions.')

  @property
  def build_dir(self):
    return self._build_dir

  @property
  def devtools_client(self):
    return self._devtools_client

  @property
  @decorators.Cache
  def extension_backend(self):
    if not self.supports_extensions:
      return None
    return extension_backend.ExtensionBackendDict(self)

  def _ArgsNeedProxyServer(self, args):
    """Returns True if args for Chrome indicate the need for proxy server."""
    if '--enable-spdy-proxy-auth' in args:
      return True
    return [arg for arg in args if arg.startswith('--proxy-server=')]

  def HasDevToolsConnection(self):
    return self._devtools_client and self._devtools_client.IsAlive()

  def _FindDevToolsPortAndTarget(self):
    """Clients should return a (devtools_port, browser_target) pair.

    May also raise EnvironmentError (IOError or OSError) if this information
    could not be determined; the call will be retried until it succeeds or
    a timeout is met.
    """
    raise NotImplementedError

  def _GetDevToolsClient(self):
    # If the agent does not appear to be ready, it could be because we got the
    # details of an older agent that no longer exists. It's thus important to
    # re-read and update the port and target on each retry.
    try:
      devtools_port, browser_target = self._FindDevToolsPortAndTarget()
    except EnvironmentError:
      return None  # Port information not ready, will retry.

    return devtools_client_backend.GetDevToolsBackEndIfReady(
        devtools_port=devtools_port,
        app_backend=self,
        browser_target=browser_target,
        enable_tracing=self._enable_tracing)

  def BindDevToolsClient(self):
    """Find an existing DevTools agent and bind this browser backend to it."""
    if self._devtools_client:
      # In case we are launching a second browser instance (as is done by
      # the CrOS backend), ensure that the old devtools_client is closed,
      # otherwise re-creating it will fail.
      self._devtools_client.Close()
      self._devtools_client = None

    try:
      self._devtools_client = py_utils.WaitFor(
          self._GetDevToolsClient,
          timeout=self.browser_options.browser_startup_timeout)
    except (py_utils.TimeoutException, exceptions.ProcessGoneException) as e:
      if not self.IsBrowserRunning():
        logging.exception(e)  # crbug.com/940075
        raise exceptions.BrowserGoneException(self.browser, e)
      raise exceptions.BrowserConnectionGoneException(self.browser, e)

  def _WaitForExtensionsToLoad(self):
    """ Wait for all extensions to load.
    Be sure to check whether the browser_backend supports_extensions before
    calling this method.
    """
    assert self._supports_extensions
    assert self._devtools_client, (
        'Waiting for extensions required devtool client to be initiated first')
    try:
      py_utils.WaitFor(self._AllExtensionsLoaded, timeout=60)
    except py_utils.TimeoutException:
      logging.error('ExtensionsToLoad: ' + repr(
          [e.extension_id for e in self._extensions_to_load]))
      logging.error('Extension list: ' + pprint.pformat(
          self.extension_backend, indent=4))
      raise

  def _AllExtensionsLoaded(self):
    # Extension pages are loaded from an about:blank page,
    # so we need to check that the document URL is the extension
    # page in addition to the ready state.
    for e in self._extensions_to_load:
      try:
        extension_objects = self.extension_backend[e.extension_id]
      except KeyError:
        return False
      for extension_object in extension_objects:
        try:
          res = extension_object.EvaluateJavaScript(
              """
              document.URL.lastIndexOf({{ url }}, 0) == 0 &&
              (document.readyState == 'complete' ||
               document.readyState == 'interactive')
              """,
              url='chrome-extension://%s/' % e.extension_id)
        except exceptions.EvaluateException:
          # If the inspected page is not ready, we will get an error
          # when we evaluate a JS expression, but we can just keep polling
          # until the page is ready (crbug.com/251913).
          res = None

        # TODO(tengs): We don't have full support for getting the Chrome
        # version before launch, so for now we use a generic workaround to
        # check for an extension binding bug in old versions of Chrome.
        # See crbug.com/263162 for details.
        if res and extension_object.EvaluateJavaScript(
            'chrome.runtime == null'):
          extension_object.Reload()
        if not res:
          return False
    return True

  @property
  def browser_directory(self):
    return self._browser_directory

  @property
  def profile_directory(self):
    return self._profile_directory

  @property
  def supports_tab_control(self):
    return self._supports_tab_control

  def GetProcessName(self, cmd_line):
    """Returns a user-friendly name for the process of the given |cmd_line|."""
    if not cmd_line:
      # TODO(tonyg): Eventually we should make all of these known and add an
      # assertion.
      return 'unknown'
    if 'nacl_helper_bootstrap' in cmd_line:
      return 'nacl_helper_bootstrap'
    if ':sandboxed_process' in cmd_line:
      return 'renderer'
    if ':privileged_process' in cmd_line:
      return 'gpu-process'
    args = shlex.split(cmd_line)
    types = [arg.split('=')[1] for arg in args if arg.startswith('--type=')]
    if not types:
      return 'browser'
    return types[0]

  @staticmethod
  def GetThreadType(thread_name):
    if not thread_name:
      return 'unknown'
    if (thread_name.startswith('Chrome_ChildIO') or
        thread_name.startswith('Chrome_IO')):
      return 'io'
    if thread_name.startswith('Compositor'):
      return 'compositor'
    if thread_name.startswith('CrGpuMain'):
      return 'gpu'
    if (thread_name.startswith('ChildProcessMai') or
        thread_name.startswith('CrRendererMain')):
      return 'main'
    if thread_name.startswith('RenderThread'):
      return 'render'
    return 'unknown'

  def Close(self):
    # If Chrome tracing is running, flush the trace before closing the browser.
    tracing_backend = self._platform_backend.tracing_controller_backend
    if tracing_backend.is_chrome_tracing_running:
      tracing_backend.FlushTracing()

    if self._devtools_client:
      # Telemetry closes Chrome by killing the Chrome process. This doesn't
      # happen on mac if START_BROWSER_WITH_DEFAULT_PRIORITY, because chrome
      # is then started using `open` command and telemetry only kills the
      # 'open' command, leaving Chrome still running.
      # Therefore force a clean shutdown on mac.
      if ('ENSURE_CLEAN_CHROME_SHUTDOWN' in os.environ or
          (os.environ.get('START_BROWSER_WITH_DEFAULT_PRIORITY', False) and
           self.browser.platform.GetOSName() == 'mac')):
        # Forces a clean shutdown by sending a command to close the browser via
        # the devtools client. Uses a long timeout as a clean shutdown can
        # sometime take a long time to complete.
        self._devtools_client.CloseBrowser()
        py_utils.WaitFor(lambda: not self.IsBrowserRunning(), 300)
      self._devtools_client.Close()
      self._devtools_client = None

    if self._ui_devtools_client:
      self._ui_devtools_client.Close()
      self._ui_devtools_client = None


  def GetSystemInfo(self):
    try:
      return self.devtools_client.GetSystemInfo()
    except (inspector_websocket.WebSocketException, socket.error) as e:
      if not self.IsBrowserRunning():
        raise exceptions.BrowserGoneException(self.browser, e)
      raise exceptions.BrowserConnectionGoneException(self.browser, e)

  def GetVersionInfo(self):
    try:
      return self.devtools_client.GetVersion()
    except (inspector_websocket.WebSocketException, socket.error) as e:
      if not self.IsBrowserRunning():
        raise exceptions.BrowserGoneException(self.browser, e)
      raise exceptions.BrowserConnectionGoneException(self.browser, e)

  @property
  def supports_memory_dumping(self):
    return True

  def DumpMemory(self, timeout=None, detail_level=None, deterministic=False):
    return self.devtools_client.DumpMemory(timeout=timeout,
                                           detail_level=detail_level,
                                           deterministic=deterministic)

  @property
  def supports_overriding_memory_pressure_notifications(self):
    return True

  def SetMemoryPressureNotificationsSuppressed(
      self, suppressed, timeout=web_contents.DEFAULT_WEB_CONTENTS_TIMEOUT):
    self.devtools_client.SetMemoryPressureNotificationsSuppressed(
        suppressed, timeout)

  def SimulateMemoryPressureNotification(
      self, pressure_level, timeout=web_contents.DEFAULT_WEB_CONTENTS_TIMEOUT):
    self.devtools_client.SimulateMemoryPressureNotification(
        pressure_level, timeout)

  def GetDirectoryPathsToFlushOsPageCacheFor(self):
    """ Return a list of directories to purge from OS page cache.

    Will only be called when page cache clearing is necessary for a benchmark.
    The caller will then attempt to purge all files from OS page cache for each
    returned directory recursively.
    """
    paths_to_flush = []
    if self.profile_directory:
      paths_to_flush.append(self.profile_directory)
    if self.browser_directory:
      paths_to_flush.append(self.browser_directory)
    return paths_to_flush

  @property
  def supports_cpu_metrics(self):
    return True

  @property
  def supports_memory_metrics(self):
    return True

  def ExecuteBrowserCommand(self, command_id, timeout):
    self.devtools_client.ExecuteBrowserCommand(command_id, timeout)

  def SetDownloadBehavior(self, behavior, downloadPath, timeout):
    self.devtools_client.SetDownloadBehavior(behavior, downloadPath, timeout)

  def _GetUIDevtoolsBackend(self):
    try:
      port = self._FindUIDevtoolsPort()
    except EnvironmentError:
      return None
    return ui_devtools_client_backend.GetUIDevtoolsBackend(
        port, self)

  def GetUIDevtoolsBackend(self):
    if not self._ui_devtools_client:
      try:
        self._ui_devtools_client = py_utils.WaitFor(
            self._GetUIDevtoolsBackend,
            timeout=10)
      except Exception as e:
        raise Exception('%s Did you launch browser with '
                        '--enable-ui-devtools=0?' % e) from e
    return self._ui_devtools_client

  def GetWindowForTarget(self, target_id):
    return self.devtools_client.GetWindowForTarget(target_id)

  def SetWindowBounds(self, window_id, bounds):
    self.devtools_client.SetWindowBounds(window_id, bounds)

  def _CreateExecutableUniqueDirectory(self, prefix):
    """Creates a semi-permanent directory unique to the browser executable.

    This directory will persist between different tests, and potentially
    be available between different test suites, but is liable to be cleaned
    up by the OS at any point outside of a test suite's run.

    Args:
      prefix: A string to include before the unique identifier in the
          directory name.

    Returns:
      A string containing an absolute path to the created directory, or None if
      no such directory can be created due to the browser executable being
      unknown.
    """
    executable = self._GetBrowserExecutablePath()
    if not executable:
      return None
    hashfunc = hashlib.sha1()
    with open(executable, 'rb') as infile:
      hashfunc.update(infile.read())
    symbols_dirname = prefix + hashfunc.hexdigest()
    # We can't use mkdtemp() directly since that will result in the directory
    # being different, and thus not shared. So, create an unused directory
    # and use the same parent directory.
    unused_dir = tempfile.mkdtemp().rstrip(os.path.sep)
    symbols_dir = os.path.join(os.path.dirname(unused_dir), symbols_dirname)
    if not os.path.exists(symbols_dir) or not os.path.isdir(symbols_dir):
      os.makedirs(symbols_dir)
    shutil.rmtree(unused_dir)
    return symbols_dir

  def _GetBrowserExecutablePath(self):
    """Gets the path to the browser executable used for testing.

    Returns:
      A string containing the path to the executable being used for testing, or
      None if it cannot be determined.
    """
    raise NotImplementedError()
