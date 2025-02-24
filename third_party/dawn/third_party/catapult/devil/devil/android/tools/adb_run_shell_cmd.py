#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function

import argparse
import json
import os
import sys

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil.android import device_utils
from devil.android.tools import script_common
from devil.utils import logging_common


def main():
  parser = argparse.ArgumentParser(
      'Run an adb shell command on selected devices')
  parser.add_argument('cmd', help='Adb shell command to run.', nargs="+")
  logging_common.AddLoggingArguments(parser)
  script_common.AddDeviceArguments(parser)
  script_common.AddEnvironmentArguments(parser)
  parser.add_argument('--as-root', action='store_true', help='Run as root.')
  parser.add_argument('--json-output', help='File to dump json output to.')
  args = parser.parse_args()

  logging_common.InitializeLogging(args)
  script_common.InitializeEnvironment(args)

  devices = script_common.GetDevices(args.devices, args.denylist_file)
  p_out = (device_utils.DeviceUtils.parallel(devices).RunShellCommand(
      args.cmd, large_output=True, as_root=args.as_root,
      check_return=True).pGet(None))

  data = {}
  for device, output in zip(devices, p_out):
    for line in output:
      print('%s: %s' % (device, line))
    data[str(device)] = output

  if args.json_output:
    with open(args.json_output, 'w') as f:
      json.dump(data, f)

  return 0


if __name__ == '__main__':
  sys.exit(main())
