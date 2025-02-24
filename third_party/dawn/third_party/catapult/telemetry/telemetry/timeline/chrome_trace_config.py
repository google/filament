# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import re
import six
from six.moves import map # pylint: disable=redefined-builtin

from telemetry.timeline import chrome_trace_category_filter

TRACE_BUFFER_SIZE_IN_KB = 'trace_buffer_size_in_kb'

RECORD_MODE = 'record_mode'

RECORD_CONTINUOUSLY = 'record-continuously'
RECORD_UNTIL_FULL = 'record-until-full'
RECORD_AS_MUCH_AS_POSSIBLE = 'record-as-much-as-possible'
ECHO_TO_CONSOLE = 'trace-to-console'

RECORD_MODES = {
    RECORD_UNTIL_FULL,
    RECORD_CONTINUOUSLY,
    RECORD_AS_MUCH_AS_POSSIBLE,
    ECHO_TO_CONSOLE,
}

ENABLE_SYSTRACE_PARAM = 'enable_systrace'
UMA_HISTOGRAM_NAMES_PARAM = 'histogram_names'

def _ConvertStringToCamelCase(string):
  """Convert an underscore/hyphen-case string to its camel-case counterpart.

  This function is the inverse of Chromium's ConvertFromCamelCase function
  in src/content/browser/devtools/protocol/tracing_handler.cc.
  """
  parts = re.split(r'[-_]', string)
  return parts[0] + ''.join([p.title() for p in parts[1:]])

# TODO(crbug.com/971471): Don't do this anymore.
def _ConvertDictKeysToCamelCaseRecursively(data):
  """Recursively convert dictionary keys from underscore/hyphen- to camel-case.

  This function is the inverse of Chromium's ConvertDictKeyStyle function
  in src/content/browser/devtools/protocol/tracing_handler.cc.
  """
  if isinstance(data, dict):
    return {_ConvertStringToCamelCase(k):
            _ConvertDictKeysToCamelCaseRecursively(v)
            for k, v in six.iteritems(data)}
  if isinstance(data, list):
    return list(map(_ConvertDictKeysToCamelCaseRecursively, data))
  return data


class ChromeTraceConfig():
  """Stores configuration options specific to the Chrome tracing agent.

  This produces the trace config JSON string for tracing in Chrome.

  Attributes:
    record_mode: can be any mode in RECORD_MODES. This corresponds to
        record modes in chrome.
    category_filter: Object that specifies which tracing categories to trace.
  """

  def __init__(self):
    self._record_mode = RECORD_CONTINUOUSLY
    self._category_filter = (
        chrome_trace_category_filter.ChromeTraceCategoryFilter())
    self._memory_dump_config = None
    self._enable_systrace = False
    self._uma_histogram_names = []
    self._trace_buffer_size_in_kb = None
    self._trace_format = None

  @property
  def trace_format(self):
    return self._trace_format

  def SetProtoTraceFormat(self):
    self._trace_format = 'proto'

  def SetJsonTraceFormat(self):
    self._trace_format = 'json'

  def SetLowOverheadFilter(self):
    self._category_filter = (
        chrome_trace_category_filter.CreateLowOverheadFilter())

  def SetDefaultOverheadFilter(self):
    self._category_filter = (
        chrome_trace_category_filter.CreateDefaultOverheadFilter())

  def SetDebugOverheadFilter(self):
    self._category_filter = (
        chrome_trace_category_filter.CreateDebugOverheadFilter())

  @property
  def category_filter(self):
    return self._category_filter

  @property
  def enable_systrace(self):
    return self._enable_systrace

  def SetCategoryFilter(self, cf):
    if isinstance(cf, chrome_trace_category_filter.ChromeTraceCategoryFilter):
      self._category_filter = cf
    else:
      raise TypeError(
          'Must pass SetCategoryFilter a ChromeTraceCategoryFilter instance')

  def SetMemoryDumpConfig(self, dump_config):
    """Memory dump config stores the triggers for memory dumps."""
    if isinstance(dump_config, MemoryDumpConfig):
      self._memory_dump_config = dump_config
    else:
      raise TypeError(
          'Must pass SetMemoryDumpConfig a MemoryDumpConfig instance')

  def SetEnableSystrace(self):
    self._enable_systrace = True

  def SetTraceBufferSizeInKb(self, size):
    self._trace_buffer_size_in_kb = size

  def EnableUMAHistograms(self, *args):
    for uma_histogram_name in args:
      self._uma_histogram_names.append(uma_histogram_name)

  def HasUMAHistograms(self):
    return len(self._uma_histogram_names) != 0

  @property
  def record_mode(self):
    return self._record_mode

  @record_mode.setter
  def record_mode(self, value):
    assert value in RECORD_MODES
    self._record_mode = value

  def GetChromeTraceConfigForStartupTracing(self):
    """Map the config to a JSON string for startup tracing.

    All keys in the returned dictionary use underscore-case (e.g.
    'record_mode'). In addition, the 'record_mode' value uses hyphen-case
    (e.g. 'record-until-full').
    """
    result = {
        RECORD_MODE: self._record_mode
    }
    result.update(self._category_filter.GetDictForChromeTracing())
    if self._memory_dump_config:
      result.update(self._memory_dump_config.GetDictForChromeTracing())
    if self._enable_systrace:
      result[ENABLE_SYSTRACE_PARAM] = True
    if self._uma_histogram_names:
      result[UMA_HISTOGRAM_NAMES_PARAM] = self._uma_histogram_names
    if self._trace_buffer_size_in_kb:
      result[TRACE_BUFFER_SIZE_IN_KB] = self._trace_buffer_size_in_kb
    return result

  def GetChromeTraceConfigForDevTools(self):
    """Map the config to a DevTools API config dictionary.

    All keys in the returned dictionary use camel-case (e.g. 'recordMode').
    In addition, the 'recordMode' value also uses camel-case (e.g.
    'recordUntilFull'). This is to invert the camel-case ->
    underscore/hyphen-delimited mapping performed in Chromium devtools.
    """
    result = self.GetChromeTraceConfigForStartupTracing()
    if result[RECORD_MODE]:
      result[RECORD_MODE] = _ConvertStringToCamelCase(
          result[RECORD_MODE])
    if self._enable_systrace:
      result.update({ENABLE_SYSTRACE_PARAM: True})
    return _ConvertDictKeysToCamelCaseRecursively(result)


class MemoryDumpConfig():
  """Stores the triggers for memory dumps in ChromeTraceConfig."""
  def __init__(self):
    self._triggers = []

  def AddTrigger(self, mode, periodic_interval_ms):
    """Adds a new trigger to config.

    Args:
      periodic_interval_ms: Dump time period in milliseconds.
      level_of_detail: Memory dump level of detail string.
          Valid arguments are "background", "light" and "detailed".
    """
    assert mode in ['background', 'light', 'detailed']
    assert periodic_interval_ms > 0
    self._triggers.append({'mode': mode,
                           'periodic_interval_ms': periodic_interval_ms})

  def GetDictForChromeTracing(self):
    """Returns the dump config as dictionary for chrome tracing."""
    # An empty trigger list would mean no periodic memory dumps.
    return {'memory_dump_config': {'triggers': self._triggers}}
