# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import datetime
import io
import logging
import os
import os.path
import platform
import random
import re
import shutil
import signal
import subprocess
import sys
import tempfile

import py_utils
from py_utils import cloud_storage
from py_utils import exc_util

from telemetry.core import exceptions
from telemetry.internal.backends.chrome import chrome_browser_backend
from telemetry.internal.backends.chrome import minidump_finder
from telemetry.internal.backends.chrome import desktop_minidump_symbolizer
from telemetry.internal.util import format_for_logging


DEVTOOLS_ACTIVE_PORT_FILE = 'DevToolsActivePort'
UI_DEVTOOLS_ACTIVE_PORT_FILE = 'UIDevToolsActivePort'

class DesktopBrowserBackend(chrome_browser_backend.ChromeBrowserBackend):
  """The backend for controlling a locally-executed browser instance, on Linux,
  Mac or Windows.
  """
  def __init__(self, desktop_platform_backend, browser_options,
               browser_directory, profile_directory,
               executable, flash_path, is_content_shell,
               build_dir=None):
    super().__init__(
        desktop_platform_backend,
        browser_options=browser_options,
        browser_directory=browser_directory,
        profile_directory=profile_directory,
        supports_extensions=not is_content_shell,
        supports_tab_control=not is_content_shell,
        build_dir=build_dir)
    self._executable = executable
    self._flash_path = flash_path
    self._is_content_shell = is_content_shell

    # Initialize fields so that an explosion during init doesn't break in Close.
    self._proc = None
    self._tmp_output_file = None
    # pylint: disable=invalid-name
    self._minidump_path_crashpad_retrieval = {}
    # pylint: enable=invalid-name

    if not self._executable:
      raise Exception('Cannot create browser, no executable found!')

    if self._flash_path and not os.path.exists(self._flash_path):
      raise RuntimeError('Flash path does not exist: %s' % self._flash_path)

    if self.is_logging_enabled:
      self._log_file_path = os.path.join(tempfile.mkdtemp(), 'chrome.log')
    else:
      self._log_file_path = None

  @property
  def is_logging_enabled(self):
    return self.browser_options.logging_verbosity in [
        self.browser_options.NON_VERBOSE_LOGGING,
        self.browser_options.VERBOSE_LOGGING,
        self.browser_options.SUPER_VERBOSE_LOGGING]

  @property
  def log_file_path(self):
    return self._log_file_path

  @property
  def processes(self):
    class Process:
      def __init__(self, s):
        # We want to get 3 pieces of information from 'ps aux':
        # - PID of the processes
        # - Type of process (e.g. 'renderer', etc)
        # - RSS of the process
        self.name = re.search(r'--type=(\w+)', s).group(1)
        self.pid = re.search(r' (\d+) ', s).group(1)
        # For RSS, we need a more complicated pattern, since multiple parts of
        # the output are just digits.
        REGEXP = (r'\d+\.\d+'  # %CPU
                  r'\s+'
                  r'\d+\.\d+'  # %MEM
                  r'\s+'
                  r'\d+'       # VSZ
                  r'\s+'
                  r'(\d+)')
        EXAMPLE = ('root           1  0.0  0.0 166760 12228 ?'
                   '        Ss   01:50   0:14 /sbin/init splash')
        assert re.search(REGEXP, EXAMPLE).group(1) == '12228'
        self.rss = re.search(REGEXP, s).group(1)
    tmp = subprocess.getoutput('ps -aux | grep chrome')
    return [Process(line) for line in tmp.split('\n') if '--type=' in line]

  @property
  def supports_uploading_logs(self):
    return (self.browser_options.logs_cloud_bucket and self.log_file_path and
            os.path.isfile(self.log_file_path))

  def _GetDevToolsActivePortPath(self):
    return os.path.join(self.profile_directory, DEVTOOLS_ACTIVE_PORT_FILE)

  def _FindDevToolsPortAndTarget(self):
    devtools_file_path = self._GetDevToolsActivePortPath()
    if not os.path.isfile(devtools_file_path):
      raise EnvironmentError('DevTools file doest not exist yet')
    # Attempt to avoid reading the file until it's populated.
    # Both stat and open may raise IOError if not ready, the caller will retry.
    lines = None
    if os.stat(devtools_file_path).st_size > 0:
      with open(devtools_file_path) as f:
        lines = [line.rstrip() for line in f]
    if not lines:
      raise EnvironmentError('DevTools file empty')

    devtools_port = int(lines[0])
    browser_target = lines[1] if len(lines) >= 2 else None
    return devtools_port, browser_target

  def _FindUIDevtoolsPort(self):
    devtools_file_path = os.path.join(self.profile_directory,
                                      UI_DEVTOOLS_ACTIVE_PORT_FILE)
    if not os.path.isfile(devtools_file_path):
      raise EnvironmentError('UIDevTools file does not exist yet')
    lines = None
    if os.stat(devtools_file_path).st_size > 0:
      with open(devtools_file_path) as f:
        lines = [line.rstrip() for line in f]
    if not lines:
      raise EnvironmentError('UIDevTools file empty')
    devtools_port = int(lines[0])
    return devtools_port

  def Start(self, startup_args):
    assert not self._proc, 'Must call Close() before Start()'

    self._dump_finder = minidump_finder.MinidumpFinder(
        self.browser.platform.GetOSName(), self.browser.platform.GetArchName())

    # macOS displays a blocking crash resume dialog that we need to suppress.
    if self.browser.platform.GetOSName() == 'mac':
      # Default write expects either the application name or the
      # path to the application. self._executable has the path to the app
      # with a few other bits tagged on after .app. Thus, we shorten the path
      # to end with .app. If this is ineffective on your mac, please delete
      # the saved state of the browser you are testing on here:
      # /Users/.../Library/Saved\ Application State/...
      # http://stackoverflow.com/questions/20226802
      dialog_path = re.sub(r'\.app\/.*', '.app', self._executable)
      subprocess.check_call([
          'defaults', 'write', '-app', dialog_path, 'NSQuitAlwaysKeepsWindows',
          '-bool', 'false'
      ])

    cmd = [self._executable]
    if self.browser.platform.GetOSName() == 'mac':
      if int(os.environ.get('START_BROWSER_WITH_DEFAULT_PRIORITY', '0')):
        # Start chrome on mac using `open`, when running benchmarks
        # so that it starts with default priority. See crbug/1454294
        executable_path = self._executable
        macos_version =  platform.mac_ver()[0]
        macos_major_version = int(macos_version[:macos_version.find('.')])
        # `open` seem to require absolute paths for mac version 13+
        if macos_major_version>= 13:
          executable_path = os.path.abspath(self._executable)
        cmd = ['open', '-n', '-W', '-a', executable_path, '--args']
      cmd.append('--use-mock-keychain')  # crbug.com/865247
    cmd.extend(startup_args)
    cmd.append('about:blank')
    env = os.environ.copy()
    env['CHROME_HEADLESS'] = '1'  # Don't upload minidumps.
    env['BREAKPAD_DUMP_LOCATION'] = self._tmp_minidump_dir
    if self.is_logging_enabled:
      sys.stderr.write(
          'Chrome log file will be saved in %s\n' % self.log_file_path)
      env['CHROME_LOG_FILE'] = self.log_file_path
    # Make sure we have predictable language settings that don't differ from the
    # recording.
    for name in ('LC_ALL', 'LC_MESSAGES', 'LANG'):
      encoding = 'en_US.UTF-8'
      if env.get(name, encoding) != encoding:
        logging.warning('Overriding env[%s]=="%s" with default value "%s"',
                        name, env[name], encoding)
      env[name] = 'en_US.UTF-8'

    self.LogStartCommand(cmd, env)

    if not self.browser_options.show_stdout:
      self._tmp_output_file = tempfile.NamedTemporaryFile('w')
      self._proc = subprocess.Popen(
          cmd, stdout=self._tmp_output_file, stderr=subprocess.STDOUT, env=env)
    else:
      # There is weird behavior on Windows where stream redirection does not
      # work as expected if we let the subprocess use the defaults. This results
      # in browser logging not being visible on Windows on swarming. Explicitly
      # setting the streams works around this. The except is in case we are
      # being run through typ, whose _TeedStream replaces the default streams.
      # This can't be used for subprocesses since it is all in-memory, and thus
      # does not have a fileno.
      if sys.platform == 'win32':
        try:
          self._proc = subprocess.Popen(
              cmd, stdout=sys.stdout, stderr=sys.stderr, env=env)
        except io.UnsupportedOperation:
          self._proc = subprocess.Popen(
              cmd, stdout=sys.__stdout__, stderr=sys.__stderr__, env=env)
      else:
        self._proc = subprocess.Popen(cmd, env=env)

    self.BindDevToolsClient()
    # browser is foregrounded by default on Windows and Linux, but not Mac.
    if self.browser.platform.GetOSName() == 'mac':
      subprocess.Popen([
          'osascript', '-e',
          ('tell application "%s" to activate' % self._executable)
      ])
    if self._supports_extensions:
      self._WaitForExtensionsToLoad()

  def LogStartCommand(self, command, env):
    """Log the command used to start Chrome.

    In order to keep the length of logs down (see crbug.com/943650),
    we sometimes trim the start command depending on browser_options.
    The command may change between runs, but usually in innocuous ways like
    --user-data-dir changes to a new temporary directory. Some benchmarks
    do use different startup arguments for different stories, but this is
    discouraged. This method could be changed to print arguments that are
    different since the last run if need be.
    """
    formatted_command = format_for_logging.ShellFormat(
        command, trim=self.browser_options.trim_logs)
    logging.info('Starting Chrome: %s\n', formatted_command)
    if not self.browser_options.trim_logs:
      logging.info('Chrome Env: %s', env)

  def BindDevToolsClient(self):
    # In addition to the work performed by the base class, quickly check if
    # the browser process is still alive.
    if not self.IsBrowserRunning():
      raise exceptions.ProcessGoneException(
          'Return code: %d' % self._proc.returncode)
    super().BindDevToolsClient()

  def GetPid(self):
    if self._proc:
      return self._proc.pid
    return None

  def IsBrowserRunning(self):
    return self._proc and self._proc.poll() is None

  def GetStandardOutput(self):
    if not self._tmp_output_file:
      if self.browser_options.show_stdout:
        # This can happen in the case that loading the Chrome binary fails.
        # We print rather than using logging here, because that makes a
        # recursive call to this function.
        print("Can't get standard output with --show-stdout", file=sys.stderr)
      return ''
    self._tmp_output_file.flush()
    try:
      with open(self._tmp_output_file.name) as f:
        return f.read()
    except IOError:
      return ''

  def _IsExecutableStripped(self):
    if self.browser.platform.GetOSName() == 'mac':
      try:
        symbols = subprocess.check_output(['/usr/bin/nm', self._executable])
      except subprocess.CalledProcessError as err:
        logging.warning(
            'Error when checking whether executable is stripped: %s',
            err.output)
        # Just assume that binary is stripped to skip breakpad symbol generation
        # if this check failed.
        return True
      num_symbols = len(symbols.splitlines())
      # We assume that if there are more than 10 symbols the executable is not
      # stripped.
      return num_symbols < 10
    return False

  def _GetStackFromMinidump(self, minidump):
    # Create an executable-specific directory if necessary to store symbols
    # for re-use. We purposefully don't clean this up so that future
    # tests can continue to use the same symbols that are unique to the
    # executable.
    symbols_dir = self._CreateExecutableUniqueDirectory('chrome_symbols_')
    dump_symbolizer = desktop_minidump_symbolizer.DesktopMinidumpSymbolizer(
        self.browser.platform.GetOSName(),
        self.browser.platform.GetArchName(),
        self._dump_finder, self.build_dir, symbols_dir=symbols_dir)
    return dump_symbolizer.SymbolizeMinidump(minidump)

  def _GetBrowserExecutablePath(self):
    return self._executable

  def _UploadMinidumpToCloudStorage(self, minidump_path):
    """ Upload minidump_path to cloud storage and return the cloud storage url.
    """
    remote_path = ('minidump-%s-%i.dmp' %
                   (datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S'),
                    random.randint(0, 1000000)))
    try:
      return cloud_storage.Insert(cloud_storage.TELEMETRY_OUTPUT, remote_path,
                                  minidump_path)
    except cloud_storage.CloudStorageError as err:
      logging.error('Cloud storage error while trying to upload dump: %s',
                    repr(err))
      return '<Missing link>'

  def SymbolizeMinidump(self, minidump_path):
    return self._InternalSymbolizeMinidump(minidump_path)

  def _InternalSymbolizeMinidump(self, minidump_path):
    cloud_storage_link = self._UploadMinidumpToCloudStorage(minidump_path)

    stack = self._GetStackFromMinidump(minidump_path)
    if not stack:
      error_message = ('Failed to symbolize minidump. Raw stack is uploaded to'
                       ' cloud storage: %s.' % cloud_storage_link)
      return (False, error_message)

    self._symbolized_minidump_paths.add(minidump_path)
    return (True, stack)

  def _TryCooperativeShutdown(self):
    if self.browser.platform.IsCooperativeShutdownSupported():
      # Ideally there would be a portable, cooperative shutdown
      # mechanism for the browser. This seems difficult to do
      # correctly for all embedders of the content API. The only known
      # problem with unclean shutdown of the browser process is on
      # Windows, where suspended child processes frequently leak. For
      # now, just solve this particular problem. See Issue 424024.
      if self.browser.platform.CooperativelyShutdown(self._proc, "chrome"):
        try:
          # Use a long timeout to handle slow Windows debug
          # (see crbug.com/815004)
          # Allow specifying a custom shutdown timeout via the
          # 'CHROME_SHUTDOWN_TIMEOUT' environment variable.
          # TODO(sebmarchand): Remove this now that there's an option to shut
          # down Chrome via Devtools.
          py_utils.WaitFor(
              lambda: not self.IsBrowserRunning(),
              timeout=int(os.getenv('CHROME_SHUTDOWN_TIMEOUT', '15')))
          logging.info('Successfully shut down browser cooperatively')
        except py_utils.TimeoutException as e:
          logging.warning('Failed to cooperatively shutdown. ' +
                          'Proceeding to terminate: ' + str(e))

  def Background(self):
    raise NotImplementedError

  @exc_util.BestEffort
  def Close(self):
    super().Close()

    # First, try to cooperatively shutdown.
    if self.IsBrowserRunning():
      self._TryCooperativeShutdown()

    # Second, try to politely shutdown with SIGINT.  Use SIGINT instead of
    # SIGTERM (or terminate()) here since the browser treats SIGTERM as a more
    # urgent shutdown signal and may not free all resources.
    if self.IsBrowserRunning() and self.browser.platform.GetOSName() != 'win':
      self._proc.send_signal(signal.SIGINT)
      try:
        py_utils.WaitFor(lambda: not self.IsBrowserRunning(),
                         timeout=int(os.getenv('CHROME_SHUTDOWN_TIMEOUT', '5'))
                        )
        self._proc = None
      except py_utils.TimeoutException:
        logging.warning('Failed to gracefully shutdown.')

    # Shutdown aggressively if all above failed.
    if self.IsBrowserRunning():
      logging.warning('Proceed to kill the browser.')
      self._proc.kill()
    self._proc = None

    if self._tmp_output_file:
      self._tmp_output_file.close()
      self._tmp_output_file = None

    if self._tmp_minidump_dir:
      shutil.rmtree(self._tmp_minidump_dir, ignore_errors=True)
      self._tmp_minidump_dir = None
