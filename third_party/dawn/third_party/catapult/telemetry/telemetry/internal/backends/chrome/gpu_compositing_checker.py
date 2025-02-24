# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class GpuCompositingAssertionFailure(AssertionError):
  pass


def AssertGpuCompositingEnabled(system_info):
  """ Assert the data in |system_info| shows GPU compositing is enabled.

  Args:
    system_info: an instance telemetry.internal.platform.system_info.SystemInfo

  Raises:
    EnvironmentError if GPU compositing is not enabled.
  """
  if system_info.gpu.feature_status['gpu_compositing'] != 'enabled':
    raise GpuCompositingAssertionFailure(
        'GPU compositing is not enabled on the system')
