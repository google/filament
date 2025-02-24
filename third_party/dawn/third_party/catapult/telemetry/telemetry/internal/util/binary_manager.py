# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import contextlib
import logging
import os
from typing import Optional

import py_utils
from py_utils import binary_manager
from py_utils import cloud_storage
from py_utils import dependency_util
import dependency_manager
from dependency_manager import base_config

from devil import devil_env


from telemetry.core import exceptions
from telemetry.core import util


TELEMETRY_PROJECT_CONFIG = os.path.join(
    util.GetTelemetryDir(), 'telemetry', 'binary_dependencies.json')


CHROME_BINARY_CONFIG = os.path.join(util.GetCatapultDir(), 'common', 'py_utils',
                                    'py_utils', 'chrome_binaries.json')


SUPPORTED_DEP_PLATFORMS = (
    'linux_aarch64', 'linux_x86_64', 'linux_armv7l', 'linux_mips',
    'mac_x86_64', 'mac_arm64',
    'win_x86', 'win_AMD64',
    'android_arm64-v8a', 'android_armeabi-v7a', 'android_arm', 'android_x64',
    'android_x86'
)

PLATFORMS_TO_DOWNLOAD_FOLDER_MAP = {
    'linux_aarch64': 'bin/linux/aarch64',
    'linux_x86_64': 'bin/linux/x86_64',
    'linux_armv7l': 'bin/linux/armv7l',
    'linux_mips': 'bin/linux/mips',
    'mac_x86_64': 'bin/mac/x86_64',
    'mac_arm64': 'bin/mac/arm64',
    'win_x86': 'bin/win/x86',
    'win_AMD64': 'bin/win/AMD64',
    'android_arm64-v8a': 'bin/android/arm64-v8a',
    'android_armeabi-v7a': 'bin/android/armeabi-v7a',
    'android_arm': 'bin/android/arm',
    'android_x64': 'bin/android/x64',
    'android_x86': 'bin/android/x86',
}

NoPathFoundError = dependency_manager.NoPathFoundError
CloudStorageError = dependency_manager.CloudStorageError


_binary_manager: Optional[binary_manager.BinaryManager] = None
_installed_helpers = set()


TELEMETRY_BINARY_BASE_CS_FOLDER = 'binary_dependencies'
TELEMETRY_BINARY_CS_BUCKET = cloud_storage.PUBLIC_BUCKET

def NeedsInit():
  return not _binary_manager


def InitDependencyManager(client_configs):
  if GetBinaryManager():
    raise exceptions.InitializationError(
        'Trying to re-initialize the binary manager with config %s'
        % client_configs)
  configs = []
  if client_configs:
    configs += client_configs
  configs += [TELEMETRY_PROJECT_CONFIG, CHROME_BINARY_CONFIG]
  SetBinaryManager(binary_manager.BinaryManager(configs))

  devil_env.config.Initialize()


@contextlib.contextmanager
def TemporarilyReplaceBinaryManager(manager):
  old_manager = GetBinaryManager()
  try:
    SetBinaryManager(manager)
    yield
  finally:
    SetBinaryManager(old_manager)


def GetBinaryManager():
  return _binary_manager


def SetBinaryManager(manager):
  global _binary_manager # pylint: disable=global-statement
  _binary_manager = manager


def _IsChromeOSLocalMode(os_name):
  """Determines if we're running telemetry on a Chrome OS device.

  Used to differentiate local mode (telemetry running on the CrOS DUT) from
  remote mode (running telemetry on another platform that communicates with
  the CrOS DUT over SSH).
  """
  return os_name == 'chromeos' and py_utils.GetHostOsName() == 'chromeos'


def FetchPath(binary_name, os_name, arch, os_version=None):
  """ Return a path to the appropriate executable for <binary_name>, downloading
      from cloud storage if needed, or None if it cannot be found.
  """
  if GetBinaryManager() is None:
    raise exceptions.InitializationError(
        'Called FetchPath with uninitialized binary manager.')
  return GetBinaryManager().FetchPath(
      binary_name, 'linux' if _IsChromeOSLocalMode(os_name) else os_name,
      arch, os_version)


def LocalPath(binary_name, os_name, arch, os_version=None):
  """ Return a local path to the given binary name, or None if an executable
      cannot be found. Will not download the executable.
      """
  if GetBinaryManager() is None:
    raise exceptions.InitializationError(
        'Called LocalPath with uninitialized binary manager.')
  return GetBinaryManager().LocalPath(binary_name, os_name, arch, os_version)


def FetchBinaryDependencies(platform, client_configs,
    fetch_reference_chrome_binary, dependency_filter=None):
  """ Fetch all binary dependencies for the given |platform|.

  Note: we don't fetch browser binaries by default because the size of the
  binary is about 2Gb, and it requires cloud storage permission to
  chrome-telemetry bucket.

  Args:
    platform: an instance of telemetry.core.platform
    client_configs: A list of paths (string) to dependencies json files.
    fetch_reference_chrome_binary: whether to fetch reference chrome binary for
      the given platform.
    dependency_filter: A list of dependency names to limit fetch to - if not
      provided will fetch all dependencies for the given platform.
  """
  configs = [
      dependency_manager.BaseConfig(TELEMETRY_PROJECT_CONFIG),
  ]
  dep_manager = dependency_manager.DependencyManager(configs)
  os_name = platform.GetOSName()
  # If we're running directly on a Chrome OS device, fetch the binaries for
  # linux instead, which should be compatible with CrOS. Otherwise, if we're
  # running remotely on CrOS, fetch the binaries for the host platform like
  # we do with android below.
  if _IsChromeOSLocalMode(os_name):
    os_name = 'linux'
  target_platform = '%s_%s' % (os_name, platform.GetArchName())
  dep_manager.PrefetchPaths(target_platform, dependency_filter)

  host_platform = None
  fetch_devil_deps = False
  if os_name in ('android', 'chromeos'):
    host_platform = '%s_%s' % (
        py_utils.GetHostOsName(), py_utils.GetHostArchName())
    dep_manager.PrefetchPaths(host_platform, dependency_filter)
    if os_name == 'android':
      if host_platform == 'linux_x86_64':
        fetch_devil_deps = True
      else:
        logging.error('Devil only supports 64 bit linux as a host platform. '
                      'Android tests may fail.')

  if fetch_reference_chrome_binary:
    _FetchReferenceBrowserBinary(platform)

  # For now, handle client config separately because the BUILD.gn & .isolate of
  # telemetry tests in chromium src failed to include the files specified in its
  # client config.
  # (https://github.com/catapult-project/catapult/issues/2192)
  # For now this is ok because the client configs usually don't include cloud
  # storage infos.
  # TODO(crbug.com/1111556): remove the logic of swallowing exception once the
  # issue is fixed on Chromium side.
  if client_configs:
    manager = dependency_manager.DependencyManager(
        list(dependency_manager.BaseConfig(c) for c in client_configs))
    try:
      manager.PrefetchPaths(target_platform)
      if host_platform is not None:
        manager.PrefetchPaths(host_platform)

    except dependency_manager.NoPathFoundError as e:
      logging.error('Error when trying to prefetch paths for %s: %s',
                    target_platform, e)

  if fetch_devil_deps:
    devil_env.config.Initialize()
    devil_env.config.PrefetchPaths(arch=platform.GetArchName())
    devil_env.config.PrefetchPaths()


def ReinstallAndroidHelperIfNeeded(binary_name, install_path, device):
  """ Install a binary helper to a specific location.

  Args:
    binary_name: (str) The name of the binary from binary_dependencies.json
    install_path: (str) The path to install the binary at
    device: (device_utils.DeviceUtils) a device to install the helper to
  Raises:
    Exception: When the binary could not be fetched or could not be pushed to
        the device.
  """
  if (device.serial, install_path) in _installed_helpers:
    return
  host_path = FetchPath(binary_name, 'android', device.GetABI())
  if not host_path:
    raise Exception(
        '%s binary could not be fetched as %s' % (binary_name, host_path))
  device.PushChangedFiles([(host_path, install_path)])
  device.RunShellCommand(['chmod', '777', install_path], check_return=True)
  _installed_helpers.add((device.serial, install_path))


def _FetchReferenceBrowserBinary(platform):
  os_name = platform.GetOSName()
  if _IsChromeOSLocalMode(os_name):
    os_name = 'linux'
  arch_name = platform.GetArchName()
  manager = binary_manager.BinaryManager(
      [CHROME_BINARY_CONFIG])
  if os_name == 'android':
    os_version = dependency_util.GetChromeApkOsVersion(
        platform.GetOSVersionName())
    manager.FetchPath(
        'chrome_stable', os_name, arch_name, os_version)
  else:
    manager.FetchPath(
        'chrome_stable', os_name, arch_name)


def UpdateDependency(dependency, dep_local_path, version,
                     os_name=None, arch_name=None):
  config = os.path.join(
      util.GetTelemetryDir(), 'telemetry', 'binary_dependencies.json')

  if not os_name:
    assert not arch_name, 'arch_name is specified but not os_name'
    os_name = py_utils.GetHostOsName()
    arch_name = py_utils.GetHostArchName()
  else:
    assert arch_name, 'os_name is specified but not arch_name'

  dep_platform = '%s_%s' % (os_name, arch_name)

  c = base_config.BaseConfig(config, writable=True)
  try:
    old_version = c.GetVersion(dependency, dep_platform)
    print('Updating from version: {}'.format(old_version))
  except ValueError as e:
    raise RuntimeError(
        ('binary_dependencies.json entry for %s missing or invalid; please add '
         'it first! (need download_path and path_within_archive)') %
        dep_platform) from e

  if dep_local_path:
    c.AddCloudStorageDependencyUpdateJob(
        dependency, dep_platform, dep_local_path, version=version,
        execute_job=True)
