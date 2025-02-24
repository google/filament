# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging

from devil import base_error
from devil.android import device_errors

logger = logging.getLogger(__name__)


def RetryOnSystemCrash(f, device, retries=3):
  """Retries the given function on a device crash.

  If the provided function fails with a DeviceUnreachableError, this will wait
  for the device to come back online, then retry the function.

  Note that this uses the same retry scheme as timeout_retry.Run.

  Args:
    f: a unary callable that takes an instance of device_utils.DeviceUtils.
    device: an instance of device_utils.DeviceUtils.
    retries: the number of retries.
  Returns:
    Whatever f returns.
  """
  num_try = 1
  while True:
    try:
      return f(device)
    except device_errors.DeviceUnreachableError:
      if num_try > retries:
        logger.error('%d consecutive device crashes. No longer retrying.',
                     num_try)
        raise
      try:
        logger.warning('Device is unreachable. Waiting for recovery...')
        # Treat the device being unreachable as an unexpected reboot and clear
        # any cached state.
        device.ClearCache()
        device.WaitUntilFullyBooted()
      except base_error.BaseError:
        logger.exception('Device never recovered. X(')
    num_try += 1
