# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=deprecated-module
import optparse
import os
import re
import sys

import py_utils

from devil.android import flag_changer
from devil.android.constants import webapk
from devil.android.perf import cache_control
from devil.android.sdk import intent

from systrace import trace_result
from systrace import tracing_agents

SRC_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), *['..']*4))
TESTING_VARIATIONS_DIR = os.path.join(SRC_DIR, 'testing', 'variations')

VARIATIONS_DIR = os.path.join(SRC_DIR, 'tools', 'variations')
sys.path.append(VARIATIONS_DIR)

try:
  import fieldtrial_util  # pylint: disable=wrong-import-position,import-error
except ImportError:
  # The |fieldtrial_util| is not available when catapult is outside the Chromium
  # tree.
  fieldtrial_util = None

class ChromeStartupTracingAgent(tracing_agents.TracingAgent):
  def __init__(self, device, package_info, webapk_package, cold, url,
               categories=None, trace_time=None, trace_format='html'):
    tracing_agents.TracingAgent.__init__(self)
    self._device = device
    self._package_info = package_info
    self._webapk_package = webapk_package
    self._cold = cold
    self._logcat_monitor = self._device.GetLogcatMonitor()
    self._url = url
    self._chrome_categories = categories
    self._trace_time = trace_time
    self._trace_format = trace_format
    self._trace_file = None
    self._trace_finish_re = re.compile(r' Completed startup tracing to (.*)')
    self._flag_changer = flag_changer.FlagChanger(
      self._device, self._package_info.cmdline_file)

  def __repr__(self):
    return 'Browser Startup Trace'

  def _SetupTracing(self):
    # TODO(lizeb): Figure out how to clean up the command-line file when
    # _TearDownTracing() is not executed in StopTracing().
    flags_to_add = ['--enable-perfetto', '--disable-fre']
    if self._chrome_categories is None:
      flags_to_add.append('--trace-startup')
    else:
      flags_to_add.append('--trace-startup={}'.format(self._chrome_categories))
    if self._trace_time is not None:
      flags_to_add.append('--trace-startup-duration={}'
                          .format(self._trace_time))
    if self._trace_format == 'proto':
      flags_to_add.append('--trace-startup-format=proto')
    elif self._trace_format == 'html' or self._trace_format == 'json':
      flags_to_add.append('--trace-startup-format=json')
    else:
      raise ValueError("Format '{}' is not supported." \
                        .format(self._trace_format))
    if fieldtrial_util:
      flags_to_add.extend(fieldtrial_util.GenerateArgs(
          os.path.join(TESTING_VARIATIONS_DIR,
                       'fieldtrial_testing_config.json'),
          'android'))

    # Perform flag difference so we only add flags not already on the
    # command line and remove flags that we aren't also trying to add.
    current_flags = self._flag_changer.GetCurrentFlags()
    flags_to_remove = find_tracing_flags(current_flags)
    flags_to_add_diff = list(set(flags_to_add) - set(current_flags))
    flags_to_remove_diff = list(set(flags_to_remove) - set(flags_to_add))
    self._flag_changer.PushFlags(add=flags_to_add_diff,
                                remove=flags_to_remove_diff)

    self._device.ForceStop(self._package_info.package)
    if self._webapk_package:
      self._device.ForceStop(self._webapk_package)
      logging.warning('Forces to stop the WebAPK and the browser provided by '
                      '--browser: %s. Please make sure that this browser '
                      'matches the host browser of the WebAPK %s. ',
                      self._package_info.package,
                      self._webapk_package)
    if self._cold:
      self._device.EnableRoot()
      cache_control.CacheControl(self._device).DropRamCaches()
    launch_intent = None
    if self._webapk_package:
      launch_intent = intent.Intent(
          package=self._webapk_package,
          activity=webapk.WEBAPK_MAIN_ACTIVITY,
          data=self._url)
    elif self._url == '':
      launch_intent = intent.Intent(
          action='android.intent.action.MAIN',
          package=self._package_info.package,
          activity=self._package_info.activity)
    else:
      launch_intent = intent.Intent(
          package=self._package_info.package,
          activity=self._package_info.activity,
          data=self._url,
          extras={'create_new_tab': True})
    self._logcat_monitor.Start()
    self._device.StartActivity(launch_intent, blocking=True)

  def _TearDownTracing(self):
    self._flag_changer.Restore()

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    self._SetupTracing()
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    try:
      self._trace_file = self._logcat_monitor.WaitFor(
          self._trace_finish_re).group(1)
    finally:
      self._TearDownTracing()
    return True

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    with open(self._PullTrace(), 'rb') as f:
      trace_data = f.read()
    return trace_result.TraceResult('traceEvents', trace_data)

  def _PullTrace(self):
    trace_file = self._trace_file.replace('/storage/emulated/0/', '/sdcard/')
    host_file = os.path.join(os.path.curdir, os.path.basename(trace_file))
    self._device.PullFile(trace_file, host_file)
    return host_file

  def SupportsExplicitClockSync(self):
    return False

  def RecordClockSyncMarker(self, sync_id, did_record_sync_marker_callback):
    # pylint: disable=unused-argument
    assert self.SupportsExplicitClockSync(), ('Clock sync marker cannot be '
        'recorded since explicit clock sync is not supported.')


class ChromeStartupConfig(tracing_agents.TracingConfig):
  def __init__(self, device, package_info, webapk_package, cold, url,
               chrome_categories, trace_time, trace_format='html'):
    tracing_agents.TracingConfig.__init__(self)
    self.device = device
    self.package_info = package_info
    self.webapk_package = webapk_package
    self.cold = cold
    self.url = url
    self.chrome_categories = chrome_categories
    self.trace_time = trace_time
    self.trace_format = trace_format


def find_tracing_flags(current_flags):
  """Finds tracing flags on the current command line."""
  tracing_flag_prefixes = ['--trace-startup', '--enable-tracing']
  tracing_flags = []
  for flag in current_flags:
    for prefix in tracing_flag_prefixes:
      if flag.startswith(prefix):
        tracing_flags.append(flag)
        break
  return tracing_flags

def try_create_agent(config):
  return ChromeStartupTracingAgent(config.device, config.package_info,
                                   config.webapk_package, config.cold,
                                   config.url, config.chrome_categories,
                                   config.trace_time, config.trace_format)

def add_options(parser):
  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=deprecated-module
  options = optparse.OptionGroup(parser, 'Chrome startup tracing')
  options.add_option('--url', help='URL to visit on startup. Default: '
                     'https://www.google.com. An empty URL launches Chrome '
                     'with a MAIN action instead of VIEW.',
                     default='https://www.google.com', metavar='URL')
  options.add_option('--cold', help='Flush the OS page cache before starting '
                     'the browser. Note that this require a device with root '
                     'access.', default=False, action='store_true')
  options.add_option('--webapk-package', help='Specify the package name '
                     'of the WebAPK to launch the given URL. An empty URL '
                     'laucnhes the host browser of the WebAPK with an new '
                     'tab.', default=None)

  return options

def get_config(options):
  return ChromeStartupConfig(options.device, options.package_info,
                             options.webapk_package, options.cold,
                             options.url, options.chrome_categories,
                             options.trace_time, options.trace_format)
