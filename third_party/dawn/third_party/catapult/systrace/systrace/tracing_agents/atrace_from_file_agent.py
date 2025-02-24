# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
import os
import re
import stat
import subprocess
import sys
from six.moves.urllib.request import urlopen  # pylint: disable=import-error
from six.moves.urllib.error import URLError   # pylint: disable=import-error

import py_utils

from systrace import trace_result
from systrace import tracing_agents
from systrace.tracing_agents import atrace_agent


# ADB sends this text to indicate the beginning of the trace data.
TRACE_START_REGEXP = br'TRACE\:'
# Text that ADB sends, but does not need to be displayed to the user.
ADB_IGNORE_REGEXP = br'^capturing trace\.\.\. done|^capturing trace\.\.\.'

T2T_OUTPUT = 'trace.systrace'

def try_create_agent(options):
  if options.from_file is not None:
    with open(options.from_file, 'rb') as f_in:
      if is_perfetto(f_in):
        if convert_perfetto_trace(options.from_file):
          options.from_file = T2T_OUTPUT
        else:
          print ('Perfetto trace file: ' + options.from_file +
                 ' could not be converted.')
          sys.exit(1)
    return AtraceFromFileAgent(options)
  return False

def convert_perfetto_trace(in_file):
  traceconv_path = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          '../traceconv'))
  try:
    traceconv = urlopen('https://get.perfetto.dev/traceconv')
    with open(traceconv_path, 'w') as out:
      out.write(traceconv.read())
  except URLError:
    print('Could not download traceconv to convert the Perfetto trace.')
    sys.exit(1)
  os.chmod(traceconv_path, stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)
  return subprocess.call([traceconv_path, 'systrace', in_file, T2T_OUTPUT]) == 0

def is_perfetto(from_file):
  # Starts with a preamble for field ID=1 (TracePacket)
  if ord(from_file.read(1)) != 0x0a:
    return False
  for _ in range(10): # Check the first 10 packets are structured correctly
    # Then a var int that specifies field size
    field_size = 0
    shift = 0
    while True:
      c = ord(from_file.read(1))
      field_size |= (c & 0x7f) << shift
      shift += 7
      if not c & 0x80:
        break
    # The packet itself
    from_file.seek(field_size, os.SEEK_CUR)
    # The preamble for the next field ID=1 (TracePacket)
    if ord(from_file.read(1)) != 0x0a:
      return False
  # Go back to the beginning of the file
  from_file.seek(0)
  return True

class AtraceFromFileConfig(tracing_agents.TracingConfig):
  def __init__(self, from_file):
    tracing_agents.TracingConfig.__init__(self)
    self.fix_circular = True
    self.from_file = from_file

def add_options(parser): # pylint: disable=unused-argument
  # The atrace_from_file_agent is not currently used, so don't display
  # any options.
  return None

def get_config(options):
  return AtraceFromFileConfig(options.from_file)


class AtraceFromFileAgent(tracing_agents.TracingAgent):
  def __init__(self, options):
    super().__init__()
    self._filename = os.path.expanduser(options.from_file)
    self._trace_data = False

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StartAgentTracing(self, config, timeout=None):
    # pylint: disable=unused-argument
    return True

  @py_utils.Timeout(tracing_agents.START_STOP_TIMEOUT)
  def StopAgentTracing(self, timeout=None):
    self._trace_data = self._read_trace_data()
    return True

  def SupportsExplicitClockSync(self):
    return False

  # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
  # pylint: disable=arguments-differ
  def RecordClockSyncMarker(self, sync_id, did_record_clock_sync_callback):
    raise NotImplementedError

  @py_utils.Timeout(tracing_agents.GET_RESULTS_TIMEOUT)
  def GetResults(self, timeout=None):
    return trace_result.TraceResult('trace-data', self._trace_data)

  def _read_trace_data(self):
    with open(self._filename, 'rb') as f:
      result = f.read()
    data_start = re.search(TRACE_START_REGEXP, result).end(0)
    data = re.sub(ADB_IGNORE_REGEXP, '', result[data_start:])
    return self._preprocess_data(data)

  # pylint: disable=no-self-use
  def _preprocess_data(self, data):
    # TODO: add fix_threads and fix_tgids options back in here
    # once we embed the dump data in the file (b/27504068)
    data = atrace_agent.strip_and_decompress_trace(data)
    data = atrace_agent.fix_circular_traces(data)
    return data
