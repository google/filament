# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import atexit
import json
import os
import sys
import time
import threading
import multiprocessing

from py_trace_event.trace_event_impl import perfetto_trace_writer
from py_trace_event import trace_time

from py_utils import lock
import six


# Trace file formats:

# Legacy format: json list of events.
# Events can be written from multiple processes, but since no process
# can be sure that it is the last one, nobody writes the closing ']'.
# So the resulting file is not technically correct json.
JSON = "json"

# Full json with events and metadata.
# This format produces correct json ready to feed into TraceDataBuilder.
# Note that it is the responsibility of the user of py_trace_event to make sure
# that trace_disable() is called after all child processes have finished.
JSON_WITH_METADATA = "json_with_metadata"

# Perfetto protobuf trace format.
PROTOBUF = "protobuf"


_lock = threading.Lock()

_enabled = False
_log_file = None

_cur_events = [] # events that have yet to be buffered
_benchmark_metadata = {}

# Default timestamp values for clock snapshot.
# If a ClockSnapshot message with these default values is emitted, Telemetry
# events' time will not be translated by trace processor, because both
# TELEMETRY and BOOTTIME timestamps are the same. This allows the old-style
# synchronization (using clock_sync events) to take place in catapult.
# If we want to actually synchronize Telemetry with other trace producers
# via clock snapshots in trace processor, we should set _boottime_ts to the
# actual BOOTTIME of the device and _emit_clock_sync to False. In this case,
# trace processor will translate both Chrome's and telemetry's timestamps
# to the device time (BOOTTIME) during proto-to-json conversion, and catapult's
# clock synchronization will not take place because we do not emit the
# clock_sync event.
# Note that we can't use both synchronization methods at the same time
# because that will lead to wrong results.
_telemetry_ts = trace_time.Now()
_boottime_ts = _telemetry_ts
_emit_clock_sync = True

_tls = threading.local() # tls used to detect forking/etc
_atexit_regsitered_for_pid = None

_control_allowed = True

_original_multiprocessing_process = multiprocessing.Process

class TraceException(Exception):
  pass

def _note(msg, *args):
  pass
#  print "%i: %s" % (os.getpid(), msg)


def _locked(fn):
  def locked_fn(*args,**kwargs):
    _lock.acquire()
    try:
      ret = fn(*args,**kwargs)
    finally:
      _lock.release()
    return ret
  return locked_fn

def _disallow_tracing_control():
  global _control_allowed
  _control_allowed = False

def trace_enable(log_file=None, format=None):
  """Enable tracing.

  Args:
    log_file: file to write trace into. Can be a file-like object,
      a name of file, or None. If None, file name is constructed
      from executable name.
    format: trace file format. See trace_event.py for available options.
  """
  global _emit_clock_sync
  if format is None:
    format = JSON
  # Can only write clock snapshots in protobuf format. In all other formats
  # should use clock_syncs.
  if format != PROTOBUF:
    _emit_clock_sync = True
  _trace_enable(log_file, format)

def _write_header():
  if _format == PROTOBUF:
    tid = threading.current_thread().ident
    perfetto_trace_writer.write_clock_snapshot(
        output=_log_file,
        tid=tid,
        telemetry_ts=_telemetry_ts,
        boottime_ts=_boottime_ts,
    )
    perfetto_trace_writer.write_thread_descriptor_event(
        output=_log_file,
        pid=os.getpid(),
        tid=tid,
        ts=trace_time.Now(),
    )
    perfetto_trace_writer.write_event(
        output=_log_file,
        ph="M",
        category="process_argv",
        name="process_argv",
        ts=trace_time.Now(),
        args={"argv": sys.argv},
        tid=tid,
    )
  else:
    if _format == JSON:
      _log_file.write('[')
    elif _format == JSON_WITH_METADATA:
      _log_file.write('{"traceEvents": [\n')
    else:
      raise TraceException("Unknown format: %s" % _format)
    json.dump({
        "ph": "M",
        "category": "process_argv",
        "pid": os.getpid(),
        "tid": threading.current_thread().ident,
        "ts": trace_time.Now(),
        "name": "process_argv",
        "args": {"argv": sys.argv},
    }, _log_file)
    _log_file.write('\n')


@_locked
def _trace_enable(log_file=None, format=None):
  global _format
  _format = format
  global _enabled
  if _enabled:
    raise TraceException("Already enabled")
  if not _control_allowed:
    raise TraceException("Tracing control not allowed in child processes.")
  _enabled = True
  global _log_file
  if log_file == None:
    if sys.argv[0] == '':
      n = 'trace_event'
    else:
      n = sys.argv[0]
    if _format == PROTOBUF:
      log_file = open("%s.pb" % n, "ab", False)
    else:
      log_file = open("%s.json" % n, "a")
  elif isinstance(log_file, six.string_types):
    if _format == PROTOBUF:
      log_file = open("%s" % log_file, "ab", False)
    else:
      log_file = open("%s" % log_file, "a")
  elif not hasattr(log_file, 'fileno'):
    raise TraceException(
        "Log file must be None, a string, or file-like object with a fileno()")

  _note("trace_event: tracelog name is %s" % log_file)

  _log_file = log_file
  with lock.FileLock(_log_file, lock.LOCK_EX):
    _log_file.seek(0, os.SEEK_END)

    lastpos = _log_file.tell()
    creator = lastpos == 0
    if creator:
      _note("trace_event: Opened new tracelog, lastpos=%i", lastpos)
      _write_header()
    else:
      _note("trace_event: Opened existing tracelog")
    _log_file.flush()
  # Monkeypatch in our process replacement for the multiprocessing.Process class
  from . import multiprocessing_shim
  if multiprocessing.Process != multiprocessing_shim.ProcessShim:
      multiprocessing.Process = multiprocessing_shim.ProcessShim

@_locked
def trace_flush():
  if _enabled:
    _flush()

@_locked
def trace_disable():
  global _enabled
  if not _control_allowed:
    raise TraceException("Tracing control not allowed in child processes.")
  if not _enabled:
    return
  _enabled = False
  _flush(close=True)
  # Clear the collected interned data so that the next trace session
  # could start from a clean state.
  perfetto_trace_writer.reset_global_state()
  multiprocessing.Process = _original_multiprocessing_process

def _write_cur_events():
  if _format == PROTOBUF:
    for e in _cur_events:
      perfetto_trace_writer.write_event(
          output=_log_file,
          ph=e["ph"],
          category=e["category"],
          name=e["name"],
          ts=e["ts"],
          args=e["args"],
          tid=threading.current_thread().ident,
      )
  elif _format in (JSON, JSON_WITH_METADATA):
    for e in _cur_events:
      _log_file.write(",\n")
      json.dump(e, _log_file)
  else:
    raise TraceException("Unknown format: %s" % _format)
  del _cur_events[:]

def _write_footer():
  if _format in [JSON, PROTOBUF]:
    # In JSON format we might not be the only process writing to this logfile.
    # So, we will simply close the file rather than writing the trailing ] that
    # it technically requires. The trace viewer understands this and
    # will insert a trailing ] during loading.
    # In PROTOBUF format there's no need for a footer. The metadata has already
    # been written in a special proto message.
    pass
  elif _format == JSON_WITH_METADATA:
    _log_file.write('],\n"metadata": ')
    json.dump(_benchmark_metadata, _log_file)
    _log_file.write('}')
  else:
    raise TraceException("Unknown format: %s" % _format)

def _flush(close=False):
  global _log_file
  with lock.FileLock(_log_file, lock.LOCK_EX):
    _log_file.seek(0, os.SEEK_END)
    if len(_cur_events):
      _write_cur_events()
    if close:
      _write_footer()
    _log_file.flush()

  if close:
    _note("trace_event: Closed")
    _log_file.close()
    _log_file = None
  else:
    _note("trace_event: Flushed")

@_locked
def trace_is_enabled():
  return _enabled

@_locked
def add_trace_event(ph, ts, category, name, args=None):
  global _enabled
  if not _enabled:
    return
  if not hasattr(_tls, 'pid') or _tls.pid != os.getpid():
    _tls.pid = os.getpid()
    global _atexit_regsitered_for_pid
    if _tls.pid != _atexit_regsitered_for_pid:
      _atexit_regsitered_for_pid = _tls.pid
      atexit.register(_trace_disable_atexit)
      _tls.pid = os.getpid()
      del _cur_events[:] # we forked, clear the event buffer!
    tid = threading.current_thread().ident
    if not tid:
      tid = os.getpid()
    _tls.tid = tid

  _cur_events.append({
      "ph": ph,
      "category": category,
      "pid": _tls.pid,
      "tid": _tls.tid,
      "ts": ts,
      "name": name,
      "args": args or {},
  });

def trace_begin(name, args=None):
  add_trace_event("B", trace_time.Now(), "python", name, args)

def trace_end(name, args=None):
  add_trace_event("E", trace_time.Now(), "python", name, args)

def trace_set_thread_name(thread_name):
  add_trace_event("M", trace_time.Now(), "__metadata", "thread_name",
                  {"name": thread_name})

def trace_add_benchmark_metadata(
    benchmark_start_time_us,
    story_run_time_us,
    benchmark_name,
    benchmark_description,
    story_name,
    story_tags,
    story_run_index,
    label=None,
):
  """Add benchmark metadata to be written to trace file.

  Args:
    benchmark_start_time_us: Benchmark start time in microseconds.
    story_run_time_us: Story start time in microseconds.
    benchmark_name: Name of the benchmark.
    benchmark_description: Description of the benchmark.
    story_name: Name of the story.
    story_tags: List of story tags.
    story_run_index: Index of the story run.
    label: Optional label.
    had_failures: Whether this story run failed.
  """
  global _benchmark_metadata
  if _format == PROTOBUF:
    # Write metadata immediately.
    perfetto_trace_writer.write_metadata(
        output=_log_file,
        benchmark_start_time_us=benchmark_start_time_us,
        story_run_time_us=story_run_time_us,
        benchmark_name=benchmark_name,
        benchmark_description=benchmark_description,
        story_name=story_name,
        story_tags=story_tags,
        story_run_index=story_run_index,
        label=label,
    )
    if _emit_clock_sync:
      perfetto_trace_writer.write_chrome_metadata(
          output=_log_file,
          clock_domain="TELEMETRY",
      )
  elif _format == JSON_WITH_METADATA:
    # Store metadata to write it in the footer.
    telemetry_metadata_for_json = {
        "benchmarkStart": benchmark_start_time_us / 1000.0,
        "traceStart": story_run_time_us / 1000.0,
        "benchmarks": [benchmark_name],
        "benchmarkDescriptions": [benchmark_description],
        "stories": [story_name],
        "storyTags": story_tags,
        "storysetRepeats": [story_run_index],
    }
    if label:
      telemetry_metadata_for_json["labels"] = [label]

    assert _emit_clock_sync
    _benchmark_metadata = {
        # TODO(crbug.com/948633): For right now, we use "TELEMETRY" as the
        # clock domain to guarantee that Telemetry is given its own clock
        # domain. Telemetry isn't really a clock domain, though: it's a
        # system that USES a clock domain like LINUX_CLOCK_MONOTONIC or
        # WIN_QPC. However, there's a chance that a Telemetry controller
        # running on Linux (using LINUX_CLOCK_MONOTONIC) is interacting
        # with an Android phone (also using LINUX_CLOCK_MONOTONIC, but
        # on a different machine). The current logic collapses clock
        # domains based solely on the clock domain string, but we really
        # should to collapse based on some (device ID, clock domain ID)
        # tuple. Giving Telemetry its own clock domain is a work-around
        # for this.
        "clock-domain": "TELEMETRY",
        "telemetry": telemetry_metadata_for_json,
    }
  elif _format == JSON:
    raise TraceException("Can't write metadata in JSON format")
  else:
    raise TraceException("Unknown format: %s" % _format)

def trace_set_clock_snapshot(telemetry_ts, boottime_ts):
  """Set timestamps to be written in a ClockSnapshot message.

  This function must be called before the trace start. When trace starts,
  a ClockSnapshot message with given timestamps will be written in protobuf
  format. In json format, nothing will happen. Use clock_sync function
  for clock synchronization in json format.

  Args:
    telemetry_ts: BOOTTIME of the device where Telemetry runs.
    boottime_ts: BOOTTIME of the device where the tested browser runs.
  """
  global _telemetry_ts
  global _boottime_ts
  global _emit_clock_sync
  global _enabled
  if _enabled:
    raise TraceException("Can't set the clock snapshot after trace started.")
  _telemetry_ts = telemetry_ts
  _boottime_ts = boottime_ts
  _emit_clock_sync = False

def clock_sync(sync_id, issue_ts=None):
  """Add a clock sync event to the trace log.

  Adds a clock sync event if the TBMv2-style synchronization is enabled.
  It's enabled in two cases:
    1) Trace format is json.
    2) The clock snapshot was not set before the trace start.

  Args:
    sync_id: ID of clock sync event.
    issue_ts: Time at which clock sync was issued, in microseconds.
  """
  if _emit_clock_sync:
    time_stamp = trace_time.Now()
    args_to_log = {"sync_id": sync_id}
    if issue_ts: # Issuer if issue_ts is set, else reciever.
      assert issue_ts <= time_stamp
      args_to_log["issue_ts"] = issue_ts
    add_trace_event("c", time_stamp, "python", "clock_sync", args_to_log)

def _trace_disable_atexit():
  trace_disable()

def is_tracing_controllable():
  global _control_allowed
  return _control_allowed
