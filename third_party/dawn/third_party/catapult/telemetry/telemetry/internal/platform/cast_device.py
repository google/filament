# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A device used for Cast"""

from __future__ import absolute_import
import os

from telemetry.core import cast_interface
from telemetry.internal.platform import device


class CastDevice(device.Device):
  def __init__(self, output_dir, runtime_exe, ip_addr):
    self._output_dir = output_dir
    self._runtime_exe = runtime_exe
    self._ip_addr = ip_addr
    super().__init__(name='cast', guid='cast')

  @classmethod
  def GetAllConnectedDevices(cls, denylist):
    return []

  @property
  def output_dir(self):
    return self._output_dir

  @property
  def runtime_exe(self):
    return self._runtime_exe

  @property
  def ip_addr(self):
    return self._ip_addr


def FindAllAvailableDevices(options):
  """Returns a list of available devices.
  """
  if (not options.cast_receiver_type or
      options.cast_receiver_type not in cast_interface.CAST_BROWSERS):
    return []

  if not options.local_cast:
    if not options.cast_device_ip:
      options.cast_device_ip = os.environ.get('CAST_DEVICE_IP')
    if not options.cast_output_dir:
      options.cast_output_dir = cast_interface.DEFAULT_CAST_CORE_DIR
    if not options.cast_runtime_exe:
      options.cast_runtime_exe = cast_interface.DEFAULT_CWR_EXE
  return [CastDevice(options.cast_output_dir, options.cast_runtime_exe,
                     options.cast_device_ip)]
