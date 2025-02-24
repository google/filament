# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running Fuchsia perf tests in Swarming."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import copy
import json

from dashboard.pinpoint.models.quest import run_telemetry_test
import six

_DEFAULT_EXTRA_ARGS = [
    '-d',
    '--os-check=update',
]
IMAGE_FLAG = '--system-image-dir='
DEFAULT_IMAGE_PATH = ('../../third_party/fuchsia-sdk/images-internal/%s/%s')
IMAGE_MAP = {
    'astro': ('astro-release', 'smart_display_eng_arrested'),
    'nelson': ('nelson-release', 'smart_display_m3_eng_paused'),
    'sherlock': ('sherlock-release', 'smart_display_max_eng_arrested'),
}

PB_IMAGE_MAP = {
    'atlas': 'workstation_eng.chromebook-x64',
    'nuc': 'workstation_eng.x64',
}

_DEFAULT_EXEC_PREFIX = 'bin/run_'


class RunWebEngineTelemetryTest(run_telemetry_test.RunTelemetryTest):

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    extra_test_args = super(RunWebEngineTelemetryTest,
                            cls)._ExtraTestArgs(arguments)
    image_path = None
    dimensions = arguments.get('dimensions')
    if isinstance(dimensions, six.string_types):
      dimensions = json.loads(dimensions)
    for key_value in dimensions:
      if key_value['key'] == 'device_type':
        board = key_value['value'].lower()
        if IMAGE_MAP.get(board):
          image_path = DEFAULT_IMAGE_PATH % IMAGE_MAP[board]
        elif PB_IMAGE_MAP.get(board):
          image_path = PB_IMAGE_MAP[board]
        else:
          raise NotImplementedError('Board %s is not supported' % board)
        break
    extra_test_args += copy.copy(_DEFAULT_EXTRA_ARGS)
    if image_path:
      extra_test_args.append(IMAGE_FLAG + image_path)
    return extra_test_args

  @classmethod
  def _ComputeCommand(cls, arguments):
    benchmark = arguments.get('benchmark')
    command = [
        'luci-auth',
        'context',
        '--',
        'vpython3',
        '../../testing/test_env.py',
        '../../testing/scripts/run_performance_tests.py',
    ]
    if benchmark in run_telemetry_test.GTEST_EXECUTABLE_NAME:
      command.extend([
          _DEFAULT_EXEC_PREFIX +
          run_telemetry_test.GTEST_EXECUTABLE_NAME[benchmark]
      ])
    else:
      command.extend([
          '../../content/test/gpu/run_telemetry_benchmark_fuchsia.py',
      ])
    relative_cwd = arguments.get('relative_cwd', 'out/Release')
    return relative_cwd, command
