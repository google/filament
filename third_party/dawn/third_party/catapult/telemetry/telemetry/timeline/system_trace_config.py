# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json
import re


class SystemTraceConfig():
  """Stores configuration options for Perfetto tracing agent."""

  class ProfilingArgs():

    def __init__(self, target_cmdlines, sampling_frequency_hz):
      """Arguments for Perfetto profiling (callstack sampling).

      Args:
        target_cmdlines: A list of processes/apps that should be profiled for
          callstack sampling.
        sampling_frequency_hz: Frequency for sampling callstacks.
      """
      self.target_cmdlines = target_cmdlines \
        if target_cmdlines is not None else []
      self.sampling_frequency_hz = sampling_frequency_hz \
        if sampling_frequency_hz is not None else 100

    # Ideally, we'd create a config by setting fiels in a proto message. Since
    # we don't currently do that, this makes the config a bit more robust to
    # invalid user input.
    def _ProcessCmdlineForConfig(self, target_cmdline):
      return re.sub(r"\s+", "", target_cmdline, flags=re.UNICODE)

    def GenerateTextConfigForCmdlines(self):
      text_config = ""
      for target_cmdline in self.target_cmdlines:
        processed_cmdline = self._ProcessCmdlineForConfig(target_cmdline)
        if processed_cmdline:
          text_config += "target_cmdline: \"{}\"\n".format(processed_cmdline)
      return text_config.strip()

  def __init__(self):
    self._enable_chrome = False
    self._enable_power = False
    self._enable_sys_stats_cpu = False
    self._enable_ftrace_cpu = False
    self._enable_ftrace_sched = False
    self._chrome_config = None
    # Not None iff profiling (callstack sampling) is enabled.
    self._profiling_args = None
    self._text_config = None

  def GetTextConfig(self):
    if self._text_config is not None:
      return self._text_config

    text_config = """
        buffers: {
            size_kb: 200000
            fill_policy: DISCARD
        }
    """

    if self._profiling_args is not None:
      # Use separate buffers for data related to callstack sampling to avoid
      # interference with other data being collected in the trace.
      text_config += """
          buffers {
              size_kb: 190464
          }
      """
      text_config += """
          data_sources {{
              config {{
                  name: "linux.perf"
                  target_buffer: 1
                  perf_event_config {{
                      all_cpus: true
                      sampling_frequency: {frequency}
                      {target_cmdlines}
                  }}
              }}
          }}
      """.format(
          frequency=self._profiling_args.sampling_frequency_hz,
          target_cmdlines=self._profiling_args.GenerateTextConfigForCmdlines())

    text_config += """
        duration_ms: 1800000
    """

    if self._enable_chrome:
      json_config = self._chrome_config.GetChromeTraceConfigForStartupTracing()
      # Note: The inner json.dumps is to serialize the chrome_trace_config dict
      # into a json string. The second outer json.dumps is to convert that to
      # a string literal to paste into the text proto config.
      json_config = json.dumps(
          json.dumps(json_config, sort_keys=True, separators=(",", ":")))
      text_config += """
          data_sources: {
              config {
                  name: "org.chromium.trace_event"
                  chrome_config {
                      trace_config: %s
                  }
              }
          }
          data_sources: {
              config {
                  name: "org.chromium.trace_metadata"
                  chrome_config {
                      trace_config: %s
                  }
              }
          }
      """ % (json_config, json_config)

    if self._enable_power:
      text_config += """
        data_sources: {
            config {
                name: "android.power"
                android_power_config {
                    battery_poll_ms: 1000
                    battery_counters: BATTERY_COUNTER_CAPACITY_PERCENT
                    battery_counters: BATTERY_COUNTER_CHARGE
                    battery_counters: BATTERY_COUNTER_CURRENT
                    collect_power_rails: true
                }
            }
        }
    """

    if self._enable_sys_stats_cpu:
      text_config += """
          data_sources: {
              config {
                  name: "linux.sys_stats"
                  sys_stats_config {
                      stat_period_ms: 1000
                      stat_counters: STAT_CPU_TIMES
                      stat_counters: STAT_FORK_COUNT
                  }
              }
          }
      """

    if self._enable_ftrace_cpu or self._enable_ftrace_sched:
      text_config += """
        data_sources: {
            config {
                name: "linux.ftrace"
                ftrace_config {
                    ftrace_events: "power/suspend_resume"
      """

      if self._enable_ftrace_cpu:
        text_config += """
                    ftrace_events: "power/cpu_frequency"
                    ftrace_events: "power/cpu_idle"
        """

      if self._enable_ftrace_sched:
        text_config += """
                    ftrace_events: "sched/sched_switch"
                    ftrace_events: "sched/sched_process_exit"
                    ftrace_events: "sched/sched_process_free"
                    ftrace_events: "task/task_newtask"
                    ftrace_events: "task/task_rename"
        """

      text_config += "}}}\n"

    return text_config

  def EnableChrome(self, chrome_trace_config):
    self._enable_chrome = True
    self._chrome_config = chrome_trace_config

  def EnablePower(self):
    self._enable_power = True

  def EnableSysStatsCpu(self):
    self._enable_sys_stats_cpu = True

  def EnableFtraceCpu(self):
    self._enable_ftrace_cpu = True

  def EnableFtraceSched(self):
    self._enable_ftrace_sched = True

  def EnableProfiling(self, target_cmdlines, sampling_frequency_hz):
    """Enables Perfetto CPU profiling (callstack sampling).

    Please see https://perfetto.dev/docs/quickstart/callstack-sampling for more
    details.

    Args:
      target_cmdlines: A list of strings indicating processes/apps that should
        be profiled for callstack sampling.
      sampling_frequency_hz: Frequency for sampling callstacks.
    """
    self._profiling_args = self.ProfilingArgs(target_cmdlines,
                                              sampling_frequency_hz)
  def SetTextConfig(self, text_config):
    """Overrides the Perfetto configuration to the given value.

    Set to None to remove the override. Useful for Perfetto power users that
    want to be in full control of what is sent to the Perfetto tracing agent.
    """
    self._text_config = text_config
