# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import collections
import sys
import gzip
import json
import logging
import os
import shutil
import subprocess
import tempfile
import time
import traceback
import six

if sys.version_info.major == 3:
  JSON_FILE_MODE = 'w+'
else:
  JSON_FILE_MODE = 'w+b'

try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


_TRACING_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                            os.path.pardir, os.path.pardir)
_TRACE2HTML_PATH = os.path.join(_TRACING_DIR, 'bin', 'trace2html')

MIB = 1024 * 1024

class TraceDataPart(object):
  """Trace data can come from a variety of tracing agents.

  Data from each agent is collected into a trace "part" and accessed by the
  following fixed field names.
  """
  def __init__(self, raw_field_name):
    self._raw_field_name = raw_field_name

  def __repr__(self):
    return 'TraceDataPart("%s")' % self._raw_field_name

  @property
  def raw_field_name(self):
    return self._raw_field_name

  def __eq__(self, other):
    return self.raw_field_name == other.raw_field_name

  def __hash__(self):
    return hash(self.raw_field_name)


ANDROID_PROCESS_DATA_PART = TraceDataPart('androidProcessDump')
ATRACE_PART = TraceDataPart('systemTraceEvents')
ATRACE_PROCESS_DUMP_PART = TraceDataPart('atraceProcessDump')
CHROME_TRACE_PART = TraceDataPart('traceEvents')
CPU_TRACE_DATA = TraceDataPart('cpuSnapshots')
TELEMETRY_PART = TraceDataPart('telemetry')
WALT_TRACE_PART = TraceDataPart('waltTraceEvents')
CGROUP_TRACE_PART = TraceDataPart('cgroupDump')

ALL_TRACE_PARTS = {ANDROID_PROCESS_DATA_PART,
                   ATRACE_PART,
                   ATRACE_PROCESS_DUMP_PART,
                   CHROME_TRACE_PART,
                   CPU_TRACE_DATA,
                   TELEMETRY_PART}


class _TraceData(object):
  """Provides read access to traces collected from multiple tracing agents.

  Instances are created by calling the AsData() method on a TraceDataWriter.
  """
  def __init__(self, raw_data):
    self._raw_data = raw_data

  def HasTracesFor(self, part):
    return bool(self.GetTracesFor(part))

  def GetTracesFor(self, part):
    """Return the list of traces for |part| in string or dictionary forms."""
    if not isinstance(part, TraceDataPart):
      raise TypeError('part must be a TraceDataPart instance')
    return self._raw_data.get(part.raw_field_name, [])

  def GetTraceFor(self, part):
    traces = self.GetTracesFor(part)
    assert len(traces) == 1
    return traces[0]


_TraceItem = collections.namedtuple(
    '_TraceItem', ['part_name', 'handle'])


class TraceDataException(Exception):
  """Exception raised by TraceDataBuilder via RecordTraceDataException()."""


class TraceDataBuilder(object):
  """TraceDataBuilder helps build up a trace from multiple trace agents.

  Note: the collected trace data is maintained in a set of temporary files to
  be later processed e.g. by the Serialize() method. To ensure proper clean up
  of such files clients must call the CleanUpTraceData() method or, even easier,
  use the context manager API, e.g.:

      with trace_data.TraceDataBuilder() as builder:
        builder.AddTraceFor(trace_part, data)
        builder.Serialize(output_file)
  """
  def __init__(self):
    self._traces = []
    self._frozen = False
    self._temp_dir = tempfile.mkdtemp()
    self._exceptions = []

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.CleanUpTraceData()

  def OpenTraceHandleFor(self, part, suffix, mode=None):
    """Open a file handle for writing trace data into it.

    Args:
      part: A TraceDataPart instance.
      suffix: A string used as file extension and identifier for the format
        of the trace contents, e.g. '.json'. Can also append '.gz' to
        indicate gzipped content, e.g. '.json.gz'.
    """
    if not isinstance(part, TraceDataPart):
      raise TypeError('part must be a TraceDataPart instance')
    if self._frozen:
      raise RuntimeError('trace data builder is no longer open for writing')
    if mode:
      trace = _TraceItem(
          part_name=part.raw_field_name,
          handle=tempfile.NamedTemporaryFile(
              mode=mode, delete=False, dir=self._temp_dir, suffix=suffix))
    else:
      trace = _TraceItem(
          part_name=part.raw_field_name,
          handle=tempfile.NamedTemporaryFile(
              delete=False, dir=self._temp_dir, suffix=suffix))
    self._traces.append(trace)
    return trace.handle

  def AddTraceFileFor(self, part, trace_file):
    """Move a file with trace data into this builder.

    This is useful for situations where a client might want to start collecting
    trace data into a file, even before the TraceDataBuilder itself is created.

    Args:
      part: A TraceDataPart instance.
      trace_file: A path to a file containing trace data. Note: for efficiency
        the file is moved rather than copied into the builder. Therefore the
        source file will no longer exist after calling this method; and the
        lifetime of the trace data will thereafter be managed by this builder.
    """
    _, suffix = os.path.splitext(trace_file)
    with self.OpenTraceHandleFor(part, suffix) as handle:
      pass
    if os.name == 'nt':
      # On windows os.rename won't overwrite, so the destination path needs to
      # be removed first.
      os.remove(handle.name)
    os.rename(trace_file, handle.name)

  def AddTraceFor(self, part, data, allow_unstructured=False):
    """Record new trace data into this builder.

    Args:
      part: A TraceDataPart instance.
      data: The trace data to write: a json-serializable dict, or unstructured
        text data as a string.
      allow_unstructured: This must be set to True to allow passing
        unstructured text data as input. Note: the use of this flag is
        discouraged and only exists to support legacy clients; new tracing
        agents should all produce structured trace data (e.g. proto or json).
    """
    if isinstance(data, StringTypes):
      if not allow_unstructured:
        raise ValueError('must pass allow_unstructured=True for text data')
      do_write = lambda d, f: f.write(d)
      suffix = '.txt'  # Used for atrace and systrace data.
    elif isinstance(data, dict):
      do_write = json.dump
      suffix = '.json'
    else:
      raise TypeError('invalid trace data type')
    with self.OpenTraceHandleFor(part, suffix, JSON_FILE_MODE) as handle:
      do_write(data, handle)

  def Freeze(self):
    """Do not allow writing any more data into this builder."""
    self._frozen = True
    return self

  def CleanUpTraceData(self):
    """Clean up resources used by the data builder.

    Will also re-raise any exceptions previously added by
    RecordTraceCollectionException().
    """
    if self._traces is None:
      return  # Already cleaned up.
    self.Freeze()
    for trace in self._traces:
      # Make sure all trace handles are closed. It's fine if we close some
      # of them multiple times.
      trace.handle.close()
    shutil.rmtree(self._temp_dir)
    self._temp_dir = None
    self._traces = None

    if self._exceptions:
      raise TraceDataException(
          'Exceptions raised during trace data collection:\n' +
          '\n'.join(self._exceptions))

  def Serialize(self, file_path, trace_title=None):
    """Serialize the trace data to a file in HTML format."""
    self.Freeze()
    assert self._traces, 'trace data has already been cleaned up'

    trace_files = [trace.handle.name for trace in self._traces]
    SerializeAsHtml(trace_files, file_path, trace_title)

  def AsData(self):
    """Allow in-memory access to read the collected JSON trace data.

    This method is only provided for writing tests which require read access
    to the collected trace data (e.g. for tracing agents to test they correctly
    write data), and to support legacy TBMv1 metric computation. Only traces
    in JSON format are supported.

    Be careful: this may require a lot of memory if the traces to process are
    very large. This has lead in the past to OOM errors (e.g. crbug/672097).

    TODO(crbug/928278): Ideally, this method should be removed when it can be
    entirely replaced by calls to an external trace processor.
    """
    self.Freeze()
    assert self._traces, 'trace data has already been cleaned up'

    raw_data = {}
    for trace in self._traces:
      is_compressed_json = trace.handle.name.endswith('.json.gz')
      is_json = trace.handle.name.endswith('.json') or is_compressed_json
      if is_json:
        traces_for_part = raw_data.setdefault(trace.part_name, [])
        opener = gzip.open if is_compressed_json else open
        with opener(trace.handle.name, 'rb') as f:
          traces_for_part.append(json.load(f))
      else:
        logging.info('Skipping over non-json trace: %s', trace.handle.name)
    return _TraceData(raw_data)

  def IterTraceParts(self):
    """Iterates over trace parts.

    Return value: iterator over pairs (part_name, file_path).
    """
    for trace in self._traces:
      yield trace.part_name, trace.handle.name

  def RecordTraceDataException(self):
    """Records the most recent exception to be re-raised during cleanup.

    Exceptions raised during trace data collection can be stored temporarily
    in the builder. They will be re-raised when the builder is cleaned up later.
    This way, any collected trace data can still be retained before the
    benchmark is aborted.

    This method is intended to be called from within an "except" handler, e.g.:
      try:
        # Collect trace data.
      except Exception: # pylint: disable=broad-except
        builder.RecordTraceDataException()
    """
    self._exceptions.append(traceback.format_exc())


def CreateTestTrace(number=1):
  """Convenient helper method to create trace data objects for testing.

  Objects are created via the usual trace data writing route, so clients are
  also responsible for cleaning up trace data themselves.

  Clients are meant to treat these test traces as opaque. No guarantees are
  made about their contents, which they shouldn't try to read.
  """
  builder = TraceDataBuilder()
  builder.AddTraceFor(CHROME_TRACE_PART, {'traceEvents': [{'test': number}]})
  return builder.Freeze()


def CreateFromRawChromeEvents(events):
  """Convenient helper to create trace data objects from raw Chrome events.

  This bypasses trace data writing, going directly to the in-memory json trace
  representation, so there is no need for trace file cleanup.

  This is used only for testing legacy clients that still read trace data.
  """
  assert isinstance(events, list)
  return _TraceData({
      CHROME_TRACE_PART.raw_field_name: [{'traceEvents': events}]})


def SerializeAsHtml(trace_files, html_file, trace_title=None):
  """Serialize a set of traces to a single file in HTML format.

  Args:
    trace_files: a list of file names, each containing a trace from
        one of the tracing agents.
    html_file: a name of the output file.
    trace_title: optional. A title for the resulting trace.
  """
  if not trace_files:
    raise ValueError('trace files list is empty')

  input_size = sum(os.path.getsize(trace_file) for trace_file in trace_files)

  cmd = [sys.executable]
  cmd.append(_TRACE2HTML_PATH)
  cmd.extend(trace_files)
  cmd.extend(['--output', html_file])
  if trace_title is not None:
    cmd.extend(['--title', trace_title])

  start_time = time.time()
  subprocess.check_output(cmd)
  elapsed_time = time.time() - start_time
  logging.info('trace2html processed %.01f MiB of trace data in %.02f seconds.',
               1.0 * input_size / MIB, elapsed_time)
