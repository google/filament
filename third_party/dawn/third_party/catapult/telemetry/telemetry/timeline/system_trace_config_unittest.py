# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
import re
from telemetry.timeline import system_trace_config


class SystemTraceConfigUnittest(unittest.TestCase):

  def assertEqualIgnoringWhitespace(self, actual, expected):

    def removeWhitespace(text):
      return re.sub(r'\s+', '', text, flags=re.UNICODE)

    return self.assertEqual(
        removeWhitespace(actual), removeWhitespace(expected))

  def testProfilingConfigWithCmdlines(self):
    config = system_trace_config.SystemTraceConfig()
    config.EnableProfiling(
        target_cmdlines=['a', 'b ', ' c', ' d ', '  e '],
        sampling_frequency_hz=None)
    self.assertEqualIgnoringWhitespace(
        config.GetTextConfig(), """
      buffers: {
          size_kb: 200000
          fill_policy: DISCARD
      }

      buffers {
          size_kb: 190464
      }

      data_sources {
          config {
              name: "linux.perf"
              target_buffer: 1
              perf_event_config {
                  all_cpus: true
                  sampling_frequency: 100
                  target_cmdline: "a"
                  target_cmdline: "b"
                  target_cmdline: "c"
                  target_cmdline: "d"
                  target_cmdline: "e"
              }
          }
      }

      duration_ms: 1800000
      """)

  def testProfilingConfigWithFrequency(self):
    config = system_trace_config.SystemTraceConfig()
    config.EnableProfiling(target_cmdlines=[], sampling_frequency_hz=1234)
    self.assertEqualIgnoringWhitespace(
        config.GetTextConfig(), """
      buffers: {
          size_kb: 200000
          fill_policy: DISCARD
      }

      buffers {
          size_kb: 190464
      }

      data_sources {
          config {
              name: "linux.perf"
              target_buffer: 1
              perf_event_config {
                  all_cpus: true
                  sampling_frequency: 1234
              }
          }
      }

      duration_ms: 1800000
      """)

  def testProfilingConfigWithCmdlinesAndFrequency(self):
    config = system_trace_config.SystemTraceConfig()
    config.EnableProfiling(
        target_cmdlines=['com.example.app'], sampling_frequency_hz=1234)
    self.assertEqualIgnoringWhitespace(
        config.GetTextConfig(), """
      buffers: {
          size_kb: 200000
          fill_policy: DISCARD
      }

      buffers {
          size_kb: 190464
      }

      data_sources {
          config {
              name: "linux.perf"
              target_buffer: 1
              perf_event_config {
                  all_cpus: true
                  sampling_frequency: 1234
                  target_cmdline: "com.example.app"
              }
          }
      }

      duration_ms: 1800000
      """)
