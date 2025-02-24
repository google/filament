#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import argparse
import json
import logging
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time

from collections import namedtuple

from hooks import install

from py_utils import binary_manager
from py_utils import dependency_util
from py_utils import xvfb


# Path to dependency manager config containing chrome binary data.
CHROME_BINARIES_CONFIG = dependency_util.ChromeBinariesConfigPath()

CHROME_CONFIG_URL = (
    'https://code.google.com/p/chromium/codesearch#chromium/src/third_party/'
    'catapult/py_utils/py_utils/chrome_binaries.json')

# Default port to run on if not auto-assigning from OS
DEFAULT_PORT = '8111'

_TIMEOUT_RETURNCODE = 124

# Mapping of sys.platform -> platform-specific names and paths.
PLATFORM_MAPPING = {
    'linux2': {
        'omaha': 'linux',
        'prefix': 'Linux_x64',
        'zip_prefix': 'linux',
        'chromepath': 'chrome-linux/chrome'
    },
    'win32': {
        'omaha': 'win',
        'prefix': 'Win',
        'zip_prefix': 'win32',
        'chromepath': 'chrome-win32\\chrome.exe',
    },
    'darwin': {
        'omaha': 'mac',
        'prefix': 'Mac',
        'zip_prefix': 'mac',
        'chromepath': ('chrome-mac/Chrome.app/Contents/MacOS/Chrome'),
        'version_path': 'chrome-mac/Chrome.app/Contents/Versions/',
        'additional_paths': [
            ('chrome-mac/Chrome.app/Contents/Versions/%VERSION%/'
             'Chrome Helper.app/Contents/MacOS/Chrome Helper'),
        ],
    },
}


class ChromeNotFound(Exception):
  pass


def IsDepotToolsPath(path):
  return os.path.isfile(os.path.join(path, 'gclient'))


def FindDepotTools():
  # Check if depot_tools is already in PYTHONPATH
  for path in sys.path:
    if path.rstrip(os.sep).endswith('depot_tools') and IsDepotToolsPath(path):
      return path

  # Check if depot_tools is in the path
  for path in os.environ['PATH'].split(os.pathsep):
    if IsDepotToolsPath(path):
      return path.rstrip(os.sep)

  return None


def GetLocalChromePath(path_from_command_line):
  if path_from_command_line:
    return path_from_command_line

  if sys.platform == 'darwin':  # Mac
    chrome_path = (
        '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome')
    if os.path.isfile(chrome_path):
      return chrome_path
  elif sys.platform.startswith('linux'):
    found = False
    try:
      with open(os.devnull, 'w') as devnull:
        found = subprocess.call(['google-chrome', '--version'],
                                stdout=devnull, stderr=devnull) == 0
    except OSError:
      pass
    if found:
      return 'google-chrome'
  elif sys.platform == 'win32':
    search_paths = [os.getenv('PROGRAMFILES(X86)'),
                    os.getenv('PROGRAMFILES'),
                    os.getenv('LOCALAPPDATA')]
    chrome_path = os.path.join('Google', 'Chrome', 'Application', 'chrome.exe')
    for search_path in search_paths:
      test_path = os.path.join(search_path, chrome_path)
      if os.path.isfile(test_path):
        return test_path
  return None


ChromeInfo = namedtuple('ChromeInfo', 'path, version')


def GetChromeInfo(args):
  """Finds chrome either locally or remotely and returns path and version info.

  Version is not reported if local chrome is used.
  """
  if args.use_local_chrome:
    chrome_path = GetLocalChromePath(args.chrome_path)
    if not chrome_path:
      raise ChromeNotFound('Could not find chrome locally. You can supply it '
                           'manually using --chrome_path')
    return ChromeInfo(path=chrome_path, version=None)
  channel = args.channel
  target = 'linux'
  if sys.platform == target and channel == 'canary':
    channel = 'dev'
  assert channel in ['stable', 'beta', 'dev', 'canary']

  binary = 'chrome'
  print('Fetching the', channel, binary, 'binary via the binary_manager.')
  chrome_manager = binary_manager.BinaryManager([CHROME_BINARIES_CONFIG])
  os_name, arch = dependency_util.GetOSAndArchForCurrentDesktopPlatform()
  chrome_path, version = chrome_manager.FetchPathWithVersion(
      '%s_%s' % (binary, channel), os_name, arch)
  print('Finished fetching the', binary, 'binary to', chrome_path)
  return ChromeInfo(path=chrome_path, version=version)


def KillProcess(process):
  """Kills process on all platform, including windows."""
  if sys.platform == 'win32':
    # Use taskkill on Windows to make sure process and all its subprocesses are
    # killed.
    subprocess.call(['taskkill', '/F', '/T', '/PID', str(process.pid)])
  else:
    process.kill()


def RunTests(args, chrome_path):
  """Runs tests and returns dev server return code.

  Returns _TIMEOUT_RETURNCODE if tests exceed args.timeout_sec.
  """
  user_data_dir = None
  xvfb_process = None
  chrome_process = None
  server_process = None
  timer = None
  test_start_time = time.time()
  try:
    user_data_dir = tempfile.mkdtemp()
    server_path = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), os.pardir, 'bin', 'run_dev_server')
    # TODO(anniesullie): Make OS selection of port work on Windows. See #1235.
    if sys.platform == 'win32':
      port = DEFAULT_PORT
    else:
      port = '0'
    server_command = [server_path, '--no-install-hooks', '--port', port]
    if sys.platform.startswith('win'):
      server_command = ['python.exe'] + server_command
    print('Starting dev_server...')
    server_process = subprocess.Popen(
        server_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        bufsize=1)
    time.sleep(1)
    if sys.platform != 'win32':
      output = server_process.stderr.readline()
      port = re.search(
          r'Now running on http://127.0.0.1:([\d]+)',
          output.decode('utf-8')).group(1)
    if xvfb.ShouldStartXvfb():
      print('Starting xvfb...')
      xvfb_process = xvfb.StartXvfb()
    chrome_command = [
        chrome_path,
        '--user-data-dir=%s' % user_data_dir,
        '--no-sandbox',
        '--no-experiments',
        '--no-first-run',
        '--noerrdialogs',
        '--window-size=1280,1024',
        '--enable-logging', '--v=1',
        '--enable-features=ForceWebRequestProxyForTest',
        '--force-device-scale-factor=1',
        '--use-mock-keychain',
    ]
    if args.extra_chrome_args:
      chrome_command.extend(args.extra_chrome_args.strip('"').split(' '))
    chrome_command.append(
        ('http://localhost:%s/%s/tests.html?' % (port, args.tests)) +
        'headless=true&testTypeToRun=all')

    print('Starting Chrome at path %s...' % chrome_path)
    chrome_process = subprocess.Popen(
        chrome_command, stdout=sys.stdout, stderr=sys.stderr)
    print('Chrome process command:', ' '.join(chrome_command))
    print('Waiting for tests to finish...')

    def KillServer():
      print('Timeout reached. Killing dev server...')
      KillProcess(server_process)

    timer = threading.Timer(args.timeout_sec, KillServer)
    timer.start()
    server_out, server_err = server_process.communicate()
    timed_out = not timer.is_alive()
    timer.cancel()

    # There is a very unlikely case where you see server saying "ALL_PASSED"
    # but the test still saying "timed out". This usually happens because the
    # server takes about a second to exit after printing "ALL_PASSED", and it
    # can time out within that time. Looking at the server returncode can help
    # here. The timeout should be increased if we're hitting this case.
    print("Server return code:", server_process.returncode)

    logging.error('Server stdout:\n%s', server_out)
    logging.error('Server stderr:\n%s', server_err)

    if timed_out:
      print('Tests did not finish before', args.timeout_sec, 'seconds')
      return _TIMEOUT_RETURNCODE
    if server_process.returncode == 0:
      print("Tests passed in %.2f seconds." % (time.time() - test_start_time))
    else:
      logging.error('Tests failed!')
    return server_process.returncode

  finally:
    if timer:
      timer.cancel()
    if server_process and server_process.poll is None:
      # Dev server is still running. Kill it.
      print('Killing dev server...')
      KillProcess(server_process)
    if chrome_process:
      print('Killing Chrome...')
      KillProcess(chrome_process)
    # Wait for Chrome to be killed before deleting temp Chrome dir. Only have
    # this timing issue on Windows.
    if sys.platform == 'win32':
      time.sleep(5)
    if user_data_dir:
      chrome_debug_logs = os.path.join(user_data_dir, 'chrome_debug.log')
      if os.path.exists(chrome_debug_logs):
        with open(chrome_debug_logs) as f:
          print('-------- chrome_debug.log --------')
          sys.stdout.write(f.read())
          print('-------- ---------------- --------')
          print('Chrome debug logs printed from', chrome_debug_logs)
      try:
        shutil.rmtree(user_data_dir)
      except OSError as e:
        logging.error('Error cleaning up temp dirs %s: %s', user_data_dir, e)
    if xvfb_process:
      KillProcess(xvfb_process)


def Main(argv):
  parser = argparse.ArgumentParser(
      description='Run dev_server tests for a project.')
  parser.add_argument('--chrome_path', type=str,
                      help='Path to Chrome browser binary.')
  parser.add_argument('--no-use-local-chrome',
                      dest='use_local_chrome', action='store_false',
                      help='Use chrome binary fetched from cloud storage '
                      'instead of chrome available on the system.')
  parser.add_argument(
      '--no-install-hooks', dest='install_hooks', action='store_false')
  parser.add_argument('--tests', type=str,
                      help='Set of tests to run (tracing or perf_insights)')
  parser.add_argument('--channel', type=str, default='stable',
                      help='Chrome channel to run (stable or canary)')
  parser.add_argument('--presentation-json', type=str,
                      help='Recipe presentation-json output file path')
  parser.add_argument('--timeout-sec', type=float, default=float('inf'),
                      help='Timeout for running all tests, in seconds')
  parser.add_argument('--timeout-retries', type=int, default=0,
                      help='Number of times to retry if tests time out.'
                      'Default 0 (no retries)')
  parser.add_argument('--extra-chrome-args', type=str,
                      help='Extra args to pass to chrome.')
  parser.set_defaults(install_hooks=True)
  parser.set_defaults(use_local_chrome=True)
  args = parser.parse_args(argv[1:])

  # TODO(crbug.com/1132884) Test consistently fails with canary channel on Mac.
  if args.channel == 'canary' and sys.platform == 'darwin':
    print ('Skipping canary channel tests on MacOS')
    sys.exit(0)

  if args.install_hooks:
    install.InstallHooks()

  chrome_info = GetChromeInfo(args)
  print('Using chrome at path', chrome_info.path)
  if not args.use_local_chrome:
    print ('Chrome version', chrome_info.version, '| channel ', args.channel)
  attempts_left = max(0, args.timeout_retries) + 1
  return_code = None
  while attempts_left:
    print(attempts_left, 'attempts left. Running tests...')
    return_code = RunTests(args, chrome_info.path)
    if return_code == _TIMEOUT_RETURNCODE:
      attempts_left -= 1
      continue
    break
  else:
    logging.error('Tests timed out every time. Retried %d times.',
                  args.timeout_retries)
    return_code = 1
  if args.presentation_json:
    with open(args.presentation_json, 'w') as recipe_out:
      # Add a link to the buildbot status for the step saying which version
      # of Chrome the test ran on. The actual linking feature is not used,
      # but there isn't a way to just add text.
      link_name = 'Chrome Version %s' % chrome_info.version
      presentation_info = {'links': {link_name: CHROME_CONFIG_URL}}
      json.dump(presentation_info, recipe_out)
  sys.exit(return_code)
