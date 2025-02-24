#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A script to replace a system app while running a command."""

import argparse
import contextlib
import logging
import os
import posixpath
import re
import sys

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil.android import apk_helper
from devil.android import decorators
from devil.android import device_errors
from devil.android import device_temp_file
from devil.android.sdk import version_codes
from devil.android.sdk import adb_wrapper
from devil.android.tools import script_common
from devil.utils import cmd_helper
from devil.utils import parallelizer
from devil.utils import run_tests_helper

logger = logging.getLogger(__name__)

# Some system apps aren't actually installed in the /system/ directory, so
# special case them here with the correct install location.
SPECIAL_SYSTEM_APP_LOCATIONS = {
    # Older versions of ArCore were installed in /data/app/ regardless of
    # whether they were system apps or not. Newer versions install in /system/
    # if they are system apps, and in /data/app/ if they aren't. Some newer
    # devices/OSes install in /product/app/ for system apps, as well.
    'com.google.ar.core': ['/data/app/', '/system/', '/product/app/'],
    # On older versions of VrCore, the system app version is installed in
    # /system/ like normal. However, at some point, this moved to /data/.
    # So, we have to handle both cases. Like ArCore, this means we'll end up
    # removing even non-system versions due to this, but it doesn't cause any
    # issues.
    'com.google.vr.core': ['/data/app/', '/system/'],
}

# Gets app path and package name pm list packages -f output.
_PM_LIST_PACKAGE_PATH_RE = re.compile(r'^\s*package:(\S+)=(\S+)\s*$')


def RemoveSystemApps(device, package_names):
  """Removes the given system apps.

  Args:
    device: (device_utils.DeviceUtils) the device for which the given
      system app should be removed.
    package_name: (iterable of strs) the names of the packages to remove.
  """
  system_package_paths = _FindSystemPackagePaths(device, package_names)
  if system_package_paths:
    with EnableSystemAppModification(device):
      device.RemovePath(system_package_paths, force=True, recursive=True)


def InstallPrivilegedApps(device, apk_tuples):
  """Install given apps as privileged apps.

  The apks will be installed to the given system image partition on device.
  If the package names already exists, they will be removed before installation.

  See source.android.com/devices/tech/config/perms-allowlist for what privileged
  apps are.

  Args:
    device: (device_utils.DeviceUtils) the device to install the privileged apps
    apk_tuples: a list of (apk_path, device_partition) tuples where |apk_path|
      is a string containing the path to the APK, and |device_partition| is a
      string indicating the system image partition on device that contains
      "priv-app" directory, e.g. "/system", "/product"
  """
  apk_metadata = []
  # Retrieve the package path ahead, since `pm` does not work properly after
  # EnableSystemAppModification
  for apk_path, partition in apk_tuples:
    package_name = apk_helper.GetPackageName(apk_path)
    package_paths = _FindSystemPackagePaths(device, [package_name])
    device_path = '%s/priv-app' % partition.rstrip('/')
    apk_metadata.append([apk_path, device_path, package_name, package_paths])
  with EnableSystemAppModification(device):
    for apk_path, device_path, package_name, package_paths in apk_metadata:
      # Remove the existing packages so that the pushed apks can be installed.
      if package_paths:
        logger.info('Removing package %s from device.', package_name)
        device.RemovePath(package_paths, force=True, recursive=True)
      logger.info('Pushing package %s (%s) to device path %s.', package_name,
                  apk_path, device_path)
      device.adb.Push(apk_path, device_path)
    device.RunShellCommand(['am', 'restart'])


@contextlib.contextmanager
def ReplaceSystemApp(device, replacement_apk, install_timeout=None):
  """A context manager that replaces the given system app while in scope.

  Args:
    device: (device_utils.DeviceUtils) the device for which the given
      system app should be replaced.
    replacement_apk: (str) the path to the APK to use as a replacement.
  """
  package_name = apk_helper.GetPackageName(replacement_apk)
  storage_dir = device_temp_file.NamedDeviceTemporaryDirectory(device.adb)
  relocate_app = _RelocateApp(device, package_name, storage_dir.name)
  install_app = _TemporarilyInstallApp(device, replacement_apk, install_timeout)
  with storage_dir, relocate_app, install_app:
    yield


def _FindSystemPackagePaths(device, system_package_list):
  """Finds all system paths for the given packages."""
  found_paths = []
  for system_package in system_package_list:
    paths = _GetApplicationPaths(device, system_package)
    p = _GetSystemPath(system_package, paths)
    if p:
      found_paths.append(p)
  # Certain versions of certain apps such as ARCore can have multiple APKs. If
  # they aren't all removed, the app is still considered installed, so ensure
  # we're removing all other APKs from the application's directory to fully
  # uninstall it.
  all_paths = found_paths.copy()
  for p in found_paths:
    if not p.endswith('.apk'):
      continue
    dirname = posixpath.dirname(p)
    for f in device.ListDirectory(dirname, as_root=True):
      filepath = posixpath.join(dirname, f)
      if filepath == p or not filepath.endswith('.apk'):
        continue
      all_paths.append(filepath)
  return all_paths


# Find all application paths, even those flagged as uninstalled, as these
# would still block another package with the same name from installation
# if they differ in signing keys.
# TODO(aluo): Move this into device_utils.py
def _GetApplicationPaths(device, package):
  paths = []
  lines = device.RunShellCommand(
      ['pm', 'list', 'packages', '-f', '-u', package], check_return=True)
  for line in lines:
    match = re.match(_PM_LIST_PACKAGE_PATH_RE, line)
    if match:
      path = match.group(1)
      package_name = match.group(2)
      if package_name == package:
        paths.append(path)
  return paths


def _GetSystemPath(package, paths):
  for p in paths:
    app_locations = SPECIAL_SYSTEM_APP_LOCATIONS.get(package,
                                                     ['/system/', '/product/'])
    for location in app_locations:
      if p.startswith(location):
        return p
  return None


_MODIFICATION_TIMEOUT = 300
_MODIFICATION_RETRIES = 2
_ENABLE_MODIFICATION_PROP = 'devil.modify_sys_apps'


# timeout and retries are both required by the decorator, but neither
# are used within the body of the function.
# pylint: disable=unused-argument


@decorators.WithTimeoutAndConditionalRetries(
    adb_wrapper.ShouldRetryAfterAdbServerRestart)
def _SetUpSystemAppModification(device, timeout=None, retries=None):
  # Ensure that the device is online & available before proceeding to
  # handle the case where something fails in the middle of set up and
  # triggers a retry.
  device.WaitUntilFullyBooted()

  # All calls that could potentially need root should run with as_root=True, but
  # it looks like some parts of Telemetry work as-is by implicitly assuming that
  # root is already granted if it's necessary. The reboot can mess with this, so
  # as a workaround, check whether we're starting with root already, and if so,
  # restore the device to that state at the end.
  should_restore_root = device.HasRoot()
  device.EnableRoot()
  if not device.HasRoot():
    raise device_errors.CommandFailedError(
        'Failed to enable modification of system apps on non-rooted device',
        str(device))

  try:
    # Disable Marshmallow's Verity security feature
    if device.build_version_sdk >= version_codes.MARSHMALLOW:
      # Additional step for emulator with Android >= Q (API 29). For more
      # details, see https://issuetracker.google.com/issues/144891973#comment31
      if device.build_version_sdk >= version_codes.Q and device.is_emulator:
        logger.info('Disabling Verication on %s', device.serial)
        device.RunShellCommand(['avbctl', 'disable-verification'],
                               check_return=True)
      logger.info('Disabling Verity on %s', device.serial)
      device.adb.DisableVerity()
      device.Reboot()
      device.WaitUntilFullyBooted()
      device.EnableRoot()

    device.adb.Remount()
    device.RunShellCommand(['stop'], check_return=True)
    device.SetProp(_ENABLE_MODIFICATION_PROP, '1')
  except device_errors.CommandFailedError:
    if device.is_emulator:
      # Point the user to documentation, since there's a good chance they can
      # workaround this on an emulator.
      docs_url = ('https://chromium.googlesource.com/chromium/src/+/'
                  'HEAD/docs/android_emulator.md#writable-system-partition')
      logger.error(
          'Did you start the emulator with "-writable-system?"\n'
          'See %s\n', docs_url)
    raise

  return should_restore_root


@decorators.WithTimeoutAndConditionalRetries(
    adb_wrapper.ShouldRetryAfterAdbServerRestart)
def _TearDownSystemAppModification(device,
                                   should_restore_root,
                                   timeout=None,
                                   retries=None):
  try:
    # The function may be re-entered after the the device loses root
    # privilege. For instance if the adb server is restarted before
    # re-entering the function then the device may lose root privilege.
    # Therefore we need to do a sanity check for root privilege
    # on the device and then re-enable root privilege if the device
    # does not have it.
    if not device.HasRoot():
      logger.warning('Need to re-enable root.')
      device.EnableRoot()

      if not device.HasRoot():
        raise device_errors.CommandFailedError(
          ('Failed to tear down modification of '
           'system apps on non-rooted device.'),
          str(device))

    device.SetProp(_ENABLE_MODIFICATION_PROP, '0')
    device.Reboot()
    device.WaitUntilFullyBooted()
    if should_restore_root:
      device.EnableRoot()
  except device_errors.CommandTimeoutError:
    logger.error('Timed out while tearing down system app modification.')
    logger.error('  device state: %s', device.adb.GetState())
    raise


# pylint: enable=unused-argument


@contextlib.contextmanager
def EnableSystemAppModification(device):
  """A context manager that allows system apps to be modified while in scope.

  Args:
    device: (device_utils.DeviceUtils) the device
  """
  if device.GetProp(_ENABLE_MODIFICATION_PROP) == '1':
    yield
    return

  should_restore_root = _SetUpSystemAppModification(
      device, timeout=_MODIFICATION_TIMEOUT, retries=_MODIFICATION_RETRIES)
  try:
    yield
  finally:
    _TearDownSystemAppModification(
        device,
        should_restore_root,
        timeout=_MODIFICATION_TIMEOUT,
        retries=_MODIFICATION_RETRIES)


@contextlib.contextmanager
def _RelocateApp(device, package_name, relocate_to):
  """A context manager that relocates an app while in scope."""
  relocation_map = {}
  system_package_paths = _FindSystemPackagePaths(device, [package_name])
  if system_package_paths:
    logger.info('Relocating system package "%s"', package_name)
    relocation_map = {
        p: posixpath.join(relocate_to, posixpath.relpath(p, '/'))
        for p in system_package_paths
    }
    relocation_dirs = [posixpath.dirname(d) for _, d in relocation_map.items()]
    device.RunShellCommand(['mkdir', '-p'] + relocation_dirs, check_return=True)
    _MoveApp(device, relocation_map)
  else:
    logger.info('No system package "%s"', package_name)

  try:
    yield
  finally:
    _MoveApp(device, {v: k for k, v in relocation_map.items()})


@contextlib.contextmanager
def _TemporarilyInstallApp(device, apk, install_timeout=None):
  """A context manager that installs an app while in scope."""
  if install_timeout is None:
    device.Install(apk, reinstall=True)
  else:
    device.Install(apk, reinstall=True, timeout=install_timeout)

  try:
    yield
  finally:
    device.Uninstall(apk_helper.GetPackageName(apk))


def _MoveApp(device, relocation_map):
  """Moves an app according to the provided relocation map.

  Args:
    device: (device_utils.DeviceUtils)
    relocation_map: (dict) A dict that maps src to dest
  """
  movements = ['mv %s %s' % (k, v) for k, v in relocation_map.items()]
  cmd = ' && '.join(movements)
  with EnableSystemAppModification(device):
    device.RunShellCommand(cmd, as_root=True, check_return=True, shell=True)


def main(raw_args):
  parser = argparse.ArgumentParser()
  subparsers = parser.add_subparsers()

  def add_common_arguments(p):
    script_common.AddDeviceArguments(p)
    script_common.AddEnvironmentArguments(p)
    p.add_argument(
        '-v',
        '--verbose',
        action='count',
        default=0,
        help='Print more information.')
    p.add_argument('command', nargs='*')

  @contextlib.contextmanager
  def remove_system_app(device, args):
    RemoveSystemApps(device, args.packages)
    yield

  remove_parser = subparsers.add_parser('remove')
  remove_parser.add_argument(
      '--package',
      dest='packages',
      nargs='*',
      required=True,
      help='The system package(s) to remove.')
  add_common_arguments(remove_parser)
  remove_parser.set_defaults(func=remove_system_app)

  @contextlib.contextmanager
  def replace_system_app(device, args):
    with ReplaceSystemApp(device, args.replace_with):
      yield

  replace_parser = subparsers.add_parser('replace')
  replace_parser.add_argument(
      '--package', help='DEPRECATED. The system package to replace.')
  replace_parser.add_argument(
      '--replace-with',
      metavar='APK',
      required=True,
      help='The APK with which the existing system app should be replaced.')
  add_common_arguments(replace_parser)
  replace_parser.set_defaults(func=replace_system_app)

  args = parser.parse_args(raw_args)

  run_tests_helper.SetLogLevel(args.verbose)
  script_common.InitializeEnvironment(args)

  devices = script_common.GetDevices(args.devices, args.denylist_file)
  parallel_devices = parallelizer.SyncParallelizer(
      [args.func(d, args) for d in devices])
  with parallel_devices:
    if args.command:
      return cmd_helper.Call(args.command)
    return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
