# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A Fuchsia device instance"""

from __future__ import absolute_import
import logging
import platform

from telemetry.core import fuchsia_interface
from telemetry.internal.platform import device


class FuchsiaDevice(device.Device):

  def __init__(self, target_id):
    super().__init__(name='Fuchsia with host: %s' % target_id, guid=target_id)
    self._target_id = target_id

  @classmethod
  def GetAllConnectedDevices(cls, denylist):
    return []

  @property
  def target_id(self):
    return self._target_id


def FindAllAvailableDevices(options):
  """Returns a list of available device types."""

  # Will not find Fuchsia devices if Fuchsia browser is not specified.
  # This means that unless specifying browser=web-engine-shell, the user
  # will not see web-engine-shell as an available browser.
  if options.browser_type not in fuchsia_interface.FUCHSIA_BROWSERS:
    return []

  if (platform.system() != 'Linux' or (
      platform.machine() != 'x86_64' and platform.machine() != 'aarch64')):
    logging.warning(
        'Fuchsia in Telemetry only supports Linux x64 or arm64 hosts.')
    return []

  return [FuchsiaDevice(target_id=options.fuchsia_target_id)]
