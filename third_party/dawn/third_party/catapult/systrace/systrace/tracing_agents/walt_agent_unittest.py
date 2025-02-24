#!/usr/bin/env python

# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import unittest

from systrace import decorators
from systrace import run_systrace
from systrace.tracing_agents import walt_agent


class WaltAgentTest(unittest.TestCase):
  """
  The WALT agent pulls the trace log from the Android phone, and does not
  communicate with the WALT device directly. This makes the agent similar
  to atrace. Since the host only connects to the Android phone, more exhaustive
  testing would require mocking DeviceUtils.
  """

  @decorators.HostOnlyTest
  def test_construct_walt_args(self):
    options, _ = run_systrace.parse_options(['./run_systrace.py',
                                                      '--walt'])
    self.assertTrue(walt_agent.get_config(options).is_walt_enabled)
    options, _ = run_systrace.parse_options(['./run_systrace.py'])
    self.assertFalse(walt_agent.get_config(options).is_walt_enabled)

  @decorators.HostOnlyTest
  def test_format_clock_sync_marker(self):
    actual_marker = walt_agent.format_clock_sync_marker(
                    'some_sync_id', 123456789012)
    expected_marker = ('<0>-0  (-----) [001] ...1  123.456789012: ' +
                       'tracing_mark_write: trace_event_clock_sync: ' +
                       'name=some_sync_id\n')
    self.assertEqual(actual_marker, expected_marker)

  @decorators.HostOnlyTest
  def test_get_results_string(self):
    agent = walt_agent.WaltAgent()
    agent._trace_contents = '<trace contents here>\n'
    agent._clock_sync_marker = '<clock sync marker here>\n'
    result = agent._get_trace_result()
    self.assertEqual(result, '# tracer: \n# clock_type=LINUX_CLOCK_MONOTONIC\n'
                      '<trace contents here>\n<clock sync marker here>\n')

if __name__ == "__main__":
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
