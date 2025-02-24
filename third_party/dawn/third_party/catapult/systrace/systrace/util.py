# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=deprecated-module
import optparse
import os
import sys

from devil.android.constants import chrome
from devil.android import device_utils, device_errors

# TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
# pylint: disable=arguments-differ
class OptionParserIgnoreErrors(optparse.OptionParser):
  """Wrapper for OptionParser that ignores errors and produces no output."""

  def error(self, msg):
    pass

  def exit(self, status=0, msg=None):
    pass

  def print_usage(self, out_file=None):
    pass

  def print_help(self, out_file=None):
    pass

  def print_version(self, out_file=None):
    pass


def run_adb_shell(shell_args, device_serial):
  """Runs "adb shell" with the given arguments.

  Args:
    shell_args: array of arguments to pass to adb shell.
    device_serial: if not empty, will add the appropriate command-line
        parameters so that adb targets the given device.
  Returns:
    A tuple containing the adb output (stdout & stderr) and the return code
    from adb.  Will exit if adb fails to start.
  """
  adb_output = []
  adb_return_code = 0
  device = device_utils.DeviceUtils.HealthyDevices(device_arg=device_serial)[0]
  try:
    adb_output = device.RunShellCommand(shell_args, shell=False,
                                        check_return=True, raw_output=True)
  except device_errors.AdbShellCommandFailedError as error:
    adb_return_code = error.status
    adb_output = error.output

  return (adb_output, adb_return_code)


def get_tracing_path(device_serial=None):
  """Uses adb to attempt to determine tracing path. The newest kernel doesn't
     support mounting debugfs, so the Android master uses tracefs to replace it.

  Returns:
    /sys/kernel/debug/tracing for device with debugfs mount support;
    /sys/kernel/tracing for device with tracefs support;
    /sys/kernel/debug/tracing if support can't be determined.
  """
  mount_info_args = ['mount']

  if device_serial is None:
    parser = OptionParserIgnoreErrors()
    parser.add_option('-e', '--serial', dest='device_serial', type='string')
    options, _ = parser.parse_args()
    device_serial = options.device_serial

  adb_output, adb_return_code = run_adb_shell(mount_info_args, device_serial, )
  if adb_return_code == 0 and 'tracefs on /sys/kernel/tracing' in adb_output:
    return '/sys/kernel/tracing'
  return '/sys/kernel/debug/tracing'


def get_device_sdk_version():
  """Uses adb to attempt to determine the SDK version of a running device."""

  getprop_args = ['getprop', 'ro.build.version.sdk']

  # get_device_sdk_version() is called before we even parse our command-line
  # args.  Therefore, parse just the device serial number part of the
  # command-line so we can send the adb command to the correct device.
  parser = OptionParserIgnoreErrors()
  parser.add_option('-e', '--serial', dest='device_serial', type='string')
  options, unused_args = parser.parse_args()  # pylint: disable=unused-variable

  success = False

  adb_output, adb_return_code = run_adb_shell(getprop_args,
                                              options.device_serial)

  if adb_return_code == 0:
    # ADB may print output other than the version number (e.g. it chould
    # print a message about starting the ADB server).
    # Break the ADB output into white-space delimited segments.
    parsed_output = str.split(adb_output)
    if parsed_output:
      # Assume that the version number is the last thing printed by ADB.
      version_string = parsed_output[-1]
      if version_string:
        try:
          # Try to convert the text into an integer.
          version = int(version_string)
        except ValueError:
          version = -1
        else:
          success = True

  if not success:
    print(adb_output, file=sys.stderr)
    raise Exception("Failed to get device sdk version")

  return version


def get_supported_browsers():
  """Returns the package names of all supported browsers."""
  # Add aliases for backwards compatibility.
  supported_browsers = {
    'stable': chrome.PACKAGE_INFO['chrome_stable'],
    'beta': chrome.PACKAGE_INFO['chrome_beta'],
    'dev': chrome.PACKAGE_INFO['chrome_dev'],
    'build': chrome.PACKAGE_INFO['chrome'],
  }
  supported_browsers.update(chrome.PACKAGE_INFO)
  return supported_browsers


def get_default_serial():
  if 'ANDROID_SERIAL' in os.environ:
    return os.environ['ANDROID_SERIAL']
  return None


def get_main_options(parser):
  parser.add_option('-o', dest='output_file', help='write trace output to FILE',
                    default=None, metavar='FILE')
  parser.add_option('-t', '--time', dest='trace_time', type='int',
                    help='trace for N seconds', metavar='N')
  parser.add_option('-j', '--json', dest='write_json',
                    default=False, action='store_true',
                    help='write a JSON file')
  parser.add_option('--link-assets', dest='link_assets', default=False,
                    action='store_true',
                    help='(deprecated)')
  parser.add_option('--from-file', dest='from_file', action='store',
                    help='read the trace from a file (compressed) rather than'
                    'running a live trace')
  parser.add_option('--asset-dir', dest='asset_dir', default='trace-viewer',
                    type='string', help='(deprecated)')
  parser.add_option('-e', '--serial', dest='device_serial_number',
                    default=get_default_serial(),
                    type='string', help='adb device serial number')
  parser.add_option('--target', dest='target', default='android', type='string',
                    help='choose tracing target (android or linux)')
  parser.add_option('--timeout', dest='timeout', type='int',
                    help='timeout for start and stop tracing (seconds)')
  parser.add_option('--collection-timeout', dest='collection_timeout',
                    type='int', help='timeout for data collection (seconds)')
  parser.add_option('-a', '--app', dest='app_name', default=None,
                    type='string', action='store',
                    help='enable application-level tracing for '
                    'comma-separated list of app cmdlines')
  parser.add_option('-t', '--time', dest='trace_time', type='int',
                    help='trace for N seconds', metavar='N')
  parser.add_option('-b', '--buf-size', dest='trace_buf_size',
                    type='int', help='use a trace buffer size '
                    ' of N KB', metavar='N')
  return parser
