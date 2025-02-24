# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import io
import logging
import os
import re
import subprocess
import tempfile

from telemetry.core import fuchsia_interface
from telemetry.internal.backends.chrome import chrome_browser_backend
from telemetry.internal.backends.chrome import minidump_finder
from telemetry.internal.platform import fuchsia_platform_backend as fuchsia_platform_backend_module

import py_utils


WEB_ENGINE_SHELL = 'web-engine-shell'
CAST_STREAMING_SHELL = 'cast-streaming-shell'
FUCHSIA_CHROME = 'fuchsia-chrome'


class FuchsiaBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  def __init__(self, fuchsia_platform_backend, browser_options,
               browser_directory, profile_directory):
    assert isinstance(fuchsia_platform_backend,
                      fuchsia_platform_backend_module.FuchsiaPlatformBackend)
    super().__init__(
        fuchsia_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        supports_extensions=False,
        supports_tab_control=True)
    self._command_runner = fuchsia_platform_backend.command_runner
    # A list of browser_type-specific ids.txt files used for symbolization.
    self._browser_id_files = None
    # A temporary file into which the browser's stdout/stderr are written. In
    # case of error, the contents returned by GetStandardOutput.
    self._browser_log_file = None
    # A system-wide log-listener used when starting Chrome.
    self._log_listener_proc = None
    # The process under test; Chrome browser or a shell.
    self._browser_process = None
    # The symbolizer process.
    self._symbolizer_proc = None
    # The process that writes the unsymbolized log output to the temp file.
    self._browser_log_writer = None
    # Cached contents of symbolized stdout/stderr in case of error.
    self._browser_log = ''
    self._devtools_port = None
    if os.environ.get('CHROMIUM_OUTPUT_DIR'):
      self._output_dir = os.environ.get('CHROMIUM_OUTPUT_DIR')
    else:
      self._output_dir = os.path.abspath(
          os.path.dirname(fuchsia_platform_backend.ssh_config))

  @property
  def log_file_path(self):
    return None

  def _FindDevToolsPortAndTarget(self):
    return self._devtools_port, None

  def DumpMemory(self, timeout=None, detail_level=None, deterministic=False):
    if detail_level is None:
      detail_level = 'light'
    return self.devtools_client.DumpMemory(timeout=timeout,
                                           detail_level=detail_level,
                                           deterministic=deterministic)

  def _ReadDevToolsPortFromLogFile(self, search_regex):
    def TryReadingPort():
      tokens = None
      for line in log_file:
        tokens = re.search(search_regex, line)
        if tokens:
          break
      return int(tokens.group(1)) if tokens else None
    with open(self._browser_log_file.name, encoding='utf-8') as log_file:
      return py_utils.WaitFor(TryReadingPort, timeout=180)

  def _ReadDevToolsPort(self):
    if (self.browser_type == WEB_ENGINE_SHELL or
        self.browser_type == CAST_STREAMING_SHELL):
      search_regex = r'Remote debugging port: (\d+)'
    else:
      search_regex = r'DevTools listening on ws://127.0.0.1:(\d+)/devtools.*'
    return self._ReadDevToolsPortFromLogFile(search_regex)

  def _StartWebEngineShell(self, startup_args):
    """Returns a stream from which browser logs can be read."""
    browser_cmd = [
        'test', 'run',
        'fuchsia-pkg://%s/web_engine_shell#meta/web_engine_shell.cm' %
        fuchsia_interface.FUCHSIA_REPO
    ]

    # Flags forwarded to the web_engine_shell component.
    browser_cmd.extend([
        '--',
        '--web-engine-package-name=web_engine_with_webui',
        '--remote-debugging-port=0',
        '--enable-web-instance-tmp',
        '--with-webui',
        'about:blank'
    ])

    # Use flags used on WebEngine in production devices.
    browser_cmd.extend([
        '--',
        '--enable-low-end-device-mode',
        '--force-gpu-mem-available-mb=64',
        '--force-gpu-mem-discardable-limit-mb=32',
        '--force-max-texture-size=2048',
        '--gpu-rasterization-msaa-sample-count=0',
        '--min-height-for-gpu-raster-tile=128',
        '--webgl-msaa-sample-count=0',
        '--max-decoded-image-size-mb=10'
    ])
    if startup_args:
      browser_cmd.extend(startup_args)
    # ffx merges stdout and stderr of the child proc into its own stdout.
    self._browser_process = self._command_runner.run_continuous_ffx_command(
        browser_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return self._browser_process.stdout

  def _StartCastStreamingShell(self, startup_args):
    browser_cmd = [
        'test',
        'run',
        'fuchsia-pkg://%s/cast_streaming_shell#meta/cast_streaming_shell.cm' %
        fuchsia_interface.FUCHSIA_REPO,
    ]

    # Flags forwarded to the cast_streaming_shell component.
    browser_cmd.extend([
        '--',
        '--remote-debugging-port=0',
    ])

    # Use flags used on WebEngine in production devices.
    browser_cmd.extend([
        '--',
        '--enable-low-end-device-mode',
        '--force-gpu-mem-available-mb=64',
        '--force-gpu-mem-discardable-limit-mb=32',
        '--force-max-texture-size=2048',
        '--gpu-rasterization-msaa-sample-count=0',
        '--min-height-for-gpu-raster-tile=128',
        '--webgl-msaa-sample-count=0',
        '--max-decoded-image-size-mb=10',
    ])
    if startup_args:
      browser_cmd.extend(startup_args)
    self._browser_process = self._command_runner.run_continuous_ffx_command(
        browser_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return self._browser_process.stdout

  def _StartChrome(self, startup_args):
    """Returns a stream from which browser logs can be read."""
    browser_cmd = [
        'session', 'add',
        'fuchsia-pkg://%s/chrome#meta/chrome_v1.cmx' %
        fuchsia_interface.FUCHSIA_REPO, '--', 'about:blank',
        '--remote-debugging-port=0', '--enable-logging'
    ]
    if startup_args:
      browser_cmd.extend(startup_args)

    # Log the browser from this point on from system-logs.
    logging_cmd = [
        'log_listener',
        '--since_now'
    ]
    # Combine to STDOUT, as this is used for symbolization.
    self._log_listener_proc = self._command_runner.RunCommandPiped(
        logging_cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT)

    logging.debug('Browser command: %s', ' '.join(browser_cmd))
    self._browser_process = self._command_runner.run_continuous_ffx_command(
        browser_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return self._log_listener_proc.stdout

  def Start(self, startup_args):
    output_root = os.path.join(self._output_dir, 'gen', 'fuchsia_web')
    if self.browser_options.show_stdout:
      logging.warning('Printing browser output into stdout is not supported ' +
                      'by fuchsia due to the known flakiness caused by tee. ' +
                      'More details can be found in http://crbug.com/1280703.')

    # Create a temporary file into which the browser's logs are written. The top
    # of the file is parsed during startup to find the devtools port, and the
    # entirety of the file is returned by GetStandardOutput in case of
    # unexpected failure.
    self._browser_log_file = tempfile.NamedTemporaryFile()
    try:
      if self.browser_type == WEB_ENGINE_SHELL:
        browser_log_stream = self._StartWebEngineShell(startup_args)
        self._browser_id_files = [
            os.path.join(output_root, 'shell', 'web_engine_shell', 'ids.txt'),
            os.path.join(output_root, 'webengine', 'web_engine_with_webui',
                         'ids.txt'),
        ]
      elif self.browser_type == CAST_STREAMING_SHELL:
        browser_log_stream = self._StartCastStreamingShell(startup_args)
        self._browser_id_files = [
            os.path.join(output_root, 'shell', 'cast_streaming_shell',
                'ids.txt'),
            os.path.join(output_root, 'webengine', 'web_engine', 'ids.txt'),
        ]
      else:
        browser_log_stream = self._StartChrome(startup_args)
        self._browser_id_files = [
            os.path.join(self._output_dir, 'gen', 'chrome', 'app', 'chrome',
                         'ids.txt'),
        ]

      # Run stdout of the browser proc through the symbolizer and into the temp
      # output file.
      # TODO(crbug.com/1280703): The logic should respect
      # self.browser_options.show_stdout, but using tee may cause resource
      # exhausted failure randomly.
      # E.g. https://forums.gentoo.org/viewtopic-t-1094162-start-0.html
      # The ideal solution is to use a file-like object implementation of tee
      # rather than the tee binary itself.
      self._symbolizer_proc = \
        fuchsia_interface.StartSymbolizerForProcessIfPossible(
              browser_log_stream,
              self._browser_log_file,
              self._browser_id_files)
      if not self._symbolizer_proc:
        # Failed to start the symbolizer, so just use the raw browser output.
        logging.warning('Failed to start symbolizer process; browser output '
                        'will not be symbolized.')
        # Cat the unsymbolized output to the temp file.
        self._browser_log_writer = subprocess.Popen('cat',
                                     stdin=browser_log_stream,
                                     stdout=self._browser_log_file,
                                     stderr=subprocess.PIPE)

      self._dump_finder = minidump_finder.MinidumpFinder(
          self.browser.platform.GetOSName(),
          self.browser.platform.GetArchName())
      self._devtools_port = self._ReadDevToolsPort()
      self.BindDevToolsClient()

      # Start tracing if startup tracing attempted but did not actually start.
      # This occurs when no ChromeTraceConfig is present yet current_state is
      # non-None.
      tracing_backend = self._platform_backend.tracing_controller_backend
      current_state = tracing_backend.current_state
      if (not tracing_backend.GetChromeTraceConfig() and
          current_state is not None):
        tracing_backend.StopTracing()
        tracing_backend.StartTracing(current_state.config,
                                     current_state.timeout)

    except Exception as e:
      logging.exception(e)
      logging.error('The browser failed to start. Output of the browser: \n%s' %
                    self.GetStandardOutput())
      self.Close()
      raise

  def GetPid(self):
    # TODO(crbug.com/1297717): This does not work if the browser process is
    # kicked off via ffx session add, as that process is on the host, and
    # exits immediately.
    return self._browser_process.pid

  def Background(self):
    raise NotImplementedError

  def _CloseOnDeviceBrowsers(self):
    if (self.browser_type == WEB_ENGINE_SHELL or
        self.browser_type == CAST_STREAMING_SHELL):
      if self._browser_process:
        logging.info('Terminating %s.cm', self.browser_type)
        # Send SIGTERM first since that gives `ffx test run` a chance to
        # gracefully cancel the component under test.
        self._browser_process.terminate()
        try:
          self._browser_process.wait(5)
        except subprocess.TimeoutExpired:
          logging.info('%s.cm still running after 5s; killing it',
                       self.browser_type)
          self._browser_process.kill()
          self._browser_process.wait()
      close_cmd = ['killall', 'web_instance.cmx']
    else:
      close_cmd = ['killall', 'chrome_v1.cmx']
    self._command_runner.RunCommand(
        close_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

  def Close(self):
    super().Close()

    self._CloseOnDeviceBrowsers()
    self._browser_process = None

    if self._log_listener_proc:
      self._log_listener_proc.kill()
    self._log_listener_proc = None

    # Kill the symbolizer proc, which feeds into the log writer (below).
    if self._symbolizer_proc:
      self._symbolizer_proc.kill()
    self._symbolizer_proc = None

    # Kill the process writing to the temporary output file.
    if self._browser_log_writer:
      self._browser_log_writer.kill()
    self._browser_log_writer = None

    # Delete the temporary output file.
    if self._browser_log_file:
      self._browser_log_file.close()
      self._browser_log_file = None

    self._browser_log = ''
    self._browser_id_files = None
    self._devtools_port = None

  def IsBrowserRunning(self):
    # TODO(crbug.com/1297717): this does not capture if the process is still
    # running if its kicked off via ffx session add.
    return bool(self._browser_process)

  def GetStandardOutput(self):
    # Cache the browser's output on the first call. There may not be an output
    # file at all in certain early failure modes.
    if not self._browser_log and self._browser_log_file:
      self._CloseOnDeviceBrowsers()

      self._browser_log_file.flush()

      output_data = None

      self._browser_log_file.seek(0, io.SEEK_SET)
      output_data = self._browser_log_file.read().decode()

      # Cache the data in case of repeat calls.
      self._browser_log = output_data

    return self._browser_log

  def SymbolizeMinidump(self, minidump_path):
    logging.warning('Symbolizing Minidump not supported on Fuchsia.')

  def _GetBrowserExecutablePath(self):
    return None
