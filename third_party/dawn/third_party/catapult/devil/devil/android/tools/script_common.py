# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

from devil import devil_env
from devil.android import device_denylist
from devil.android import device_errors
from devil.android import device_utils


def AddEnvironmentArguments(parser):
  """Adds environment-specific arguments to the provided parser.

  After adding these arguments, you must pass the user-specified values when
  initializing devil. See the InitializeEnvironment() to determine how to do so.

  Args:
    parser: an instance of argparse.ArgumentParser
  """
  parser.add_argument(
      '--adb-path', type=os.path.realpath, help='Path to the adb binary')


def InitializeEnvironment(args):
  """Initializes devil based on the args added by AddEnvironmentArguments().

  This initializes devil, and configures it to use the adb binary specified by
  the '--adb-path' flag (if provided by the user, otherwise this defaults to
  devil's copy of adb). Although this is one possible way to initialize devil,
  you should check if your project has prefered ways to initialize devil (ex.
  the chromium project uses devil_chromium.Initialize() to have different
  defaults for dependencies).

  This method requires having previously called AddEnvironmentArguments() on the
  relevant argparse.ArgumentParser.

  Note: you should only initialize devil once, and subsequent calls to any
  method wrapping devil_env.config.Initialize() will have no effect.

  Args:
    args: the parsed args returned by an argparse.ArgumentParser
  """
  devil_dynamic_config = devil_env.EmptyConfig()
  if args.adb_path:
    devil_dynamic_config['dependencies'].update(
        devil_env.LocalConfigItem('adb', devil_env.GetPlatform(),
                                  args.adb_path))

  devil_env.config.Initialize(configs=[devil_dynamic_config])


def AddDeviceArguments(parser):
  """Adds device and denylist arguments to the provided parser.

  Args:
    parser: an instance of argparse.ArgumentParser
  """
  parser.add_argument(
      '-d',
      '--device',
      dest='devices',
      action='append',
      default=[],
      help='Serial number of the Android device to use. (default: use all)')

  parser.add_argument('--denylist-file',
                      help='Device denylist JSON file.')


def GetDevices(requested_devices, denylist_file):
  """Gets a list of healthy devices matching the given parameters."""
  if not isinstance(denylist_file, device_denylist.Denylist):
    denylist_file = (device_denylist.Denylist(denylist_file)
                     if denylist_file else None)

  devices = device_utils.DeviceUtils.HealthyDevices(denylist_file)
  if not devices:
    raise device_errors.NoDevicesError()
  if requested_devices:
    requested = set(requested_devices)
    available = set(str(d) for d in devices)
    missing = requested.difference(available)
    if missing:
      raise device_errors.DeviceUnreachableError(next(iter(missing)))
    return sorted(
        device_utils.DeviceUtils(d) for d in available.intersection(requested))
  return devices
