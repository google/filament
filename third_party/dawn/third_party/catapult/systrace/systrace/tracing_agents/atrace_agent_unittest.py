#!/usr/bin/env python

# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import unittest
import six

from systrace import decorators
from systrace import run_systrace
from systrace import util
from systrace.tracing_agents import atrace_agent

from devil.android import device_utils
from devil.android.sdk import intent
from py_utils import tempfile_ext


DEVICE_SERIAL = 'AG8404EC0444AGC'
ATRACE_ARGS = ['atrace', '-z', '-t', '10', '-b', '4096']
CATEGORIES = ['sched', 'gfx', 'view', 'wm']
ADB_SHELL = ['adb', '-s', DEVICE_SERIAL, 'shell']

SYSTRACE_CMD = ['./run_systrace.py', '--time', '10', '-o', 'out.html', '-e',
                DEVICE_SERIAL] + CATEGORIES
TRACE_ARGS = (ATRACE_ARGS + CATEGORIES)

TEST_DIR = os.path.join(os.path.dirname(__file__), os.pardir, 'test_data')
ATRACE_DATA = os.path.join(TEST_DIR, 'atrace_data')
ATRACE_DATA_RAW = os.path.join(TEST_DIR, 'atrace_data_raw')
ATRACE_DATA_STRIPPED = os.path.join(TEST_DIR, 'atrace_data_stripped')
ATRACE_PROCFS_DUMP = os.path.join(TEST_DIR, 'atrace_procfs_dump')
ATRACE_EXTRACTED_TGIDS = os.path.join(TEST_DIR, 'atrace_extracted_tgids')
ATRACE_MISSING_TGIDS = os.path.join(TEST_DIR, 'atrace_missing_tgids')
ATRACE_FIXED_TGIDS = os.path.join(TEST_DIR, 'atrace_fixed_tgids')

def eval_dict_string_and_ensure_binary(dict_str):
  str_dict = eval(dict_str)
  binary_dict = {}
  for k, v in str_dict.items():
    binary_dict[six.ensure_binary(k)] = six.ensure_binary(v)
  return binary_dict

class AtraceAgentTest(unittest.TestCase):

  # TODO(washingtonp): These end-to-end tests do not work on the Trybot server
  # because adb cannot be found on the Trybot servers. Figure out what the
  # issue is and update this test.
  @decorators.Disabled
  def test_tracing(self):
    TRACE_BUFFER_SIZE = '16384'
    TRACE_TIME = '5'

    devices = device_utils.DeviceUtils.HealthyDevices()
    package_info = util.get_supported_browsers()['stable']
    device = devices[0]
    with tempfile_ext.TemporaryFileName() as output_file_name:
      # Launch the browser before tracing.
      device.StartActivity(
          intent.Intent(activity=package_info.activity,
                        package=package_info.package,
                        data='about:blank',
                        extras={'create_new_tab': True}),
          blocking=True, force_stop=True)

      # Run atrace agent.
      run_systrace.main_impl(['./run_systrace.py',
                              '-b',
                              TRACE_BUFFER_SIZE,
                              '-t',
                              TRACE_TIME,
                              '-o',
                              output_file_name,
                              '-e',
                              str(device),
                              '--atrace-categories=gfx,input,view'])

      # Verify results.
      with open(output_file_name, 'r') as f:
        full_trace = f.read()
      self.assertTrue('CPU#' in full_trace)

  @decorators.HostOnlyTest
  def test_construct_atrace_args(self):
    options, categories = run_systrace.parse_options(SYSTRACE_CMD)
    options.atrace_categories = categories
    tracer_args = atrace_agent._construct_atrace_args(options, categories)
    self.assertEqual(' '.join(TRACE_ARGS), ' '.join(tracer_args))

  @decorators.HostOnlyTest
  def test_strip_and_decompress_trace(self):
    with open(ATRACE_DATA_RAW, 'rb') as f1, \
        open(ATRACE_DATA_STRIPPED, 'rb') as f2:
      atrace_data_raw = f1.read()
      atrace_data_stripped = f2.read()

      trace_data = atrace_agent.strip_and_decompress_trace(atrace_data_raw)
      self.assertEqual(atrace_data_stripped, trace_data)

  @decorators.HostOnlyTest
  def test_extract_tgids(self):
    with open(ATRACE_PROCFS_DUMP, 'r') as f1, \
        open(ATRACE_EXTRACTED_TGIDS, 'r') as f2:

      atrace_procfs_dump = f1.read()
      atrace_procfs_extracted = f2.read()

      tgids = eval_dict_string_and_ensure_binary(atrace_procfs_extracted)
      result = atrace_agent.extract_tgids(atrace_procfs_dump.splitlines())

      self.assertEqual(result, tgids)

  @decorators.HostOnlyTest
  def test_fix_missing_tgids(self):
    with open(ATRACE_EXTRACTED_TGIDS, 'r') as f1, \
        open(ATRACE_MISSING_TGIDS, 'rb') as f2, \
        open(ATRACE_FIXED_TGIDS, 'rb') as f3:

      atrace_data = f2.read()
      tgid_map = eval_dict_string_and_ensure_binary(f1.read())
      fixed = f3.read()

      res = atrace_agent.fix_missing_tgids(atrace_data, tgid_map)
      self.assertEqual(res, fixed)


if __name__ == "__main__":
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
