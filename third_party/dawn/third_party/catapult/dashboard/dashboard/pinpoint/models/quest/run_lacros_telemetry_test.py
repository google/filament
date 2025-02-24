# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running Lacros perf tests in Swarming."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models.quest import run_telemetry_test

class RunLacrosTelemetryTest(run_telemetry_test.RunTelemetryTest):
  @classmethod
  def _ComputeCommand(cls, arguments):
    command = [
        'luci-auth', 'context', '--', 'vpython3',
        'bin/run_' + arguments.get('target'),
    ]
    if 'chromeos-swarming' in arguments.get('swarming_server'):
      command += ['--fetch-cros-hostname', '--fetch-cros-remote']
    else:
      command += ['--remote=variable_chromeos_device_hostname']
    relative_cwd = arguments.get('relative_cwd', 'out/Release')
    return relative_cwd, command
