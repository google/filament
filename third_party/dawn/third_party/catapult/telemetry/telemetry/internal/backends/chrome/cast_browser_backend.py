# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import tempfile
import time

from telemetry.internal.backends.chrome import chrome_browser_backend

DEVTOOLS_PORT = 9222

class ReceiverNotFoundException(Exception):
  pass


class CastBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  def __init__(self, cast_platform_backend, browser_options,
               browser_directory, profile_directory, casting_tab):
    super().__init__(
        cast_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        supports_extensions=False,
        supports_tab_control=False)
    self._browser_process = None
    self._cast_core_process = None
    self._casting_tab = casting_tab
    self._discovery_mode = False
    self._output_dir = cast_platform_backend.output_dir
    self._receiver_name = None
    self._runtime_exe = cast_platform_backend.runtime_exe
    self._window_visible = False

    self._log_dir = tempfile.TemporaryDirectory()
    self._cast_core_log_file = os.path.join(self._log_dir.name, 'cast_core_log')
    self._runtime_log_file = os.path.join(self._log_dir.name, 'runtime_log')

  @property
  def log_file_path(self):
    return None

  def _FindDevToolsPortAndTarget(self):
    return DEVTOOLS_PORT, None

  def _ReadReceiverName(self):
    raise NotImplementedError

  def Start(self, startup_args):
    raise NotImplementedError

  def FlingVideo(self, url):
    """Fling a video to the Cast receiver."""
    if not self._discovery_mode:
      raise Exception('Cast receiver is not in discovery mode.')
    self._casting_tab.action_runner.Navigate(url)
    self._casting_tab.action_runner.WaitForJavaScriptCondition(
        'document.readyState === "complete"')
    self._casting_tab.action_runner.EvaluateJavaScript(
        'navigator.presentation.defaultRequest.start()',
        user_gesture=True,
        promise=True)
    self.BindDevToolsClient()

  def MirrorTab(self):
    """Mirror the casting tab to the Cast receiver."""
    if not self._discovery_mode:
      raise Exception('Cast receiver is not in discovery mode.')
    self._casting_tab.action_runner.tab.StartTabMirroring(
        self._ReadReceiverName())
    self.BindDevToolsClient()

  def StopCasting(self):
    """Stop casting to the Cast receiver."""
    self._casting_tab.action_runner.tab.StopCasting(self._ReadReceiverName())

  def _WaitForSink(self, timeout=60):
    sink_name_list = []
    start_time = time.time()
    while (self._receiver_name not in sink_name_list
           and time.time() - start_time < timeout):
      self._casting_tab.action_runner.tab.EnableCast()
      self._casting_tab.action_runner.tab.SetCastSinkToUse(
          self._ReadReceiverName())
      sink_name_list = [
          sink['name'] for sink in self._casting_tab\
                                       .action_runner.tab.GetCastSinks()
      ]
      self._casting_tab.action_runner.Wait(1)
    if self._ReadReceiverName() not in sink_name_list:
      raise ReceiverNotFoundException(
          'Could not find Cast Receiver {0}.'.format(self._ReadReceiverName()))

  def BindDevToolsClient(self):
    super().BindDevToolsClient()
    self._window_visible = True

  def GetPid(self):
    return self._browser_process.pid

  def Close(self):
    super().Close()
    self._discovery_mode = False
    self._window_visible = False

  def IsBrowserRunning(self):
    return self._window_visible

  def GetStandardOutput(self):
    return 'Stdout is not available for Cast browser.'

  def GetStackTrace(self):
    return (False, 'Stack trace is not yet supported on Cast browser.')

  def SymbolizeMinidump(self, minidump_path):
    logging.info('Symbolizing Minidump is not yet supported on Cast browser.')
