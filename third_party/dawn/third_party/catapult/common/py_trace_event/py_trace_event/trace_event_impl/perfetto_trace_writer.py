# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Functions to write trace data in perfetto protobuf format.
"""

from __future__ import absolute_import
import collections

from . import perfetto_proto_classes as proto
import six

CLOCK_BOOTTIME = 6
CLOCK_TELEMETRY = 64


def reset_global_state():
  global _interned_categories_by_tid
  global _interned_event_names_by_tid
  global _next_sequence_id
  global _sequence_ids

  # Dicts of strings for interning.
  # Note that each thread has its own interning index.
  _interned_categories_by_tid = collections.defaultdict(dict)
  _interned_event_names_by_tid = collections.defaultdict(dict)

  # Trusted sequence ids from telemetry should not overlap with
  # trusted sequence ids from other trace producers. Chrome assigns
  # sequence ids incrementally starting from 1 and we expect all its ids
  # to be well below 10000. Starting from 2^20 will give us enough
  # confidence that it will not overlap.
  _next_sequence_id = 1<<20
  _sequence_ids = {}


reset_global_state()


def _get_sequence_id(tid):
  global _sequence_ids
  global _next_sequence_id
  if tid not in _sequence_ids:
    _sequence_ids[tid] = _next_sequence_id
    _next_sequence_id += 1
  return _sequence_ids[tid]


def _intern_category(category, trace_packet, tid):
  global _interned_categories_by_tid
  categories = _interned_categories_by_tid[tid]
  if category not in categories:
    # note that interning indices start from 1
    categories[category] = len(categories) + 1
    if trace_packet.interned_data is None:
      trace_packet.interned_data = proto.InternedData()
    trace_packet.interned_data.event_category = proto.EventCategory()
    trace_packet.interned_data.event_category.iid = categories[category]
    trace_packet.interned_data.event_category.name = category
  return categories[category]


def _intern_event_name(event_name, trace_packet, tid):
  global _interned_event_names_by_tid
  event_names = _interned_event_names_by_tid[tid]
  if event_name not in event_names:
    # note that interning indices start from 1
    event_names[event_name] = len(event_names) + 1
    if trace_packet.interned_data is None:
      trace_packet.interned_data = proto.InternedData()
    trace_packet.interned_data.legacy_event_name = proto.LegacyEventName()
    trace_packet.interned_data.legacy_event_name.iid = event_names[event_name]
    trace_packet.interned_data.legacy_event_name.name = event_name
  return event_names[event_name]


def write_thread_descriptor_event(output, pid, tid, ts):
  """Write the first event in a sequence.

  Call this function before writing any other events.
  Note that this function is NOT thread-safe.

  Args:
    output: a file-like object to write events into.
    pid: process ID.
    tid: thread ID.
    ts: timestamp in microseconds.
  """
  thread_descriptor_packet = proto.TracePacket()
  thread_descriptor_packet.trusted_packet_sequence_id = _get_sequence_id(tid)
  thread_descriptor_packet.timestamp = int(ts * 1e3)
  thread_descriptor_packet.timestamp_clock_id = CLOCK_TELEMETRY

  thread_descriptor_packet.thread_descriptor = proto.ThreadDescriptor()
  thread_descriptor_packet.thread_descriptor.pid = pid
  # Thread ID from threading module doesn't fit into int32.
  # But we don't need the exact thread ID, just some number to
  # distinguish one thread from another. We assume that the last 31 bits
  # will do for that purpose.
  thread_descriptor_packet.thread_descriptor.tid = tid & 0x7FFFFFFF
  thread_descriptor_packet.incremental_state_cleared = True;

  proto.write_trace_packet(output, thread_descriptor_packet)


def write_event(output, ph, category, name, ts, args, tid):
  """Write a trace event.

  Note that this function is NOT thread-safe.

  Args:
    output: a file-like object to write events into.
    ph: phase of event.
    category: category of event.
    name: event name.
    ts: timestamp in microseconds.
    args: dict of arbitrary key-values to be stored as DebugAnnotations.
    tid: thread ID.
  """
  packet = proto.TracePacket()
  packet.trusted_packet_sequence_id = _get_sequence_id(tid)
  packet.timestamp = int(ts * 1e3)
  packet.timestamp_clock_id = CLOCK_TELEMETRY

  packet.track_event = proto.TrackEvent()
  packet.track_event.category_iids = [_intern_category(category, packet, tid)]
  legacy_event = proto.LegacyEvent()
  legacy_event.phase = ord(ph)
  legacy_event.name_iid = _intern_event_name(name, packet, tid)
  packet.track_event.legacy_event = legacy_event

  for name, value in six.iteritems(args):
    debug_annotation = proto.DebugAnnotation()
    debug_annotation.name = name
    if isinstance(value, int):
      debug_annotation.int_value = value
    elif isinstance(value, float):
      debug_annotation.double_value = value
    else:
      debug_annotation.string_value = str(value)
    packet.track_event.debug_annotations.append(debug_annotation)

  proto.write_trace_packet(output, packet)


def write_chrome_metadata(output, clock_domain):
  """Write a chrome trace event with metadata.

  Args:
    output: a file-like object to write events into.
    clock_domain: a string representing the trace clock domain.
  """
  chrome_metadata = proto.ChromeMetadata()
  chrome_metadata.name = 'clock-domain'
  chrome_metadata.string_value = clock_domain
  chrome_event = proto.ChromeEventBundle()
  chrome_event.metadata.append(chrome_metadata)
  packet = proto.TracePacket()
  packet.chrome_event = chrome_event
  proto.write_trace_packet(output, packet)


def write_metadata(
    output,
    benchmark_start_time_us,
    story_run_time_us,
    benchmark_name,
    benchmark_description,
    story_name,
    story_tags,
    story_run_index,
    label=None,
):
  """Write a ChromeBenchmarkMetadata message."""
  metadata = proto.ChromeBenchmarkMetadata()
  metadata.benchmark_start_time_us = int(benchmark_start_time_us)
  metadata.story_run_time_us = int(story_run_time_us)
  metadata.benchmark_name = benchmark_name
  metadata.benchmark_description = benchmark_description
  metadata.story_name = story_name
  metadata.story_tags = list(story_tags)
  metadata.story_run_index = int(story_run_index)
  if label is not None:
    metadata.label = label

  packet = proto.TracePacket()
  packet.chrome_benchmark_metadata = metadata
  proto.write_trace_packet(output, packet)


def write_clock_snapshot(
    output,
    tid,
    telemetry_ts=None,
    boottime_ts=None,
):
  """Write a ClockSnapshot message.

  Note that this function is NOT thread-safe.

  Args:
    output: a file-like object to write events into.
    telemetry_ts: host BOOTTIME timestamp in microseconds.
    boottime_ts: device BOOTTIME timestamp in microseconds.
  """
  clock_snapshot = proto.ClockSnapshot()
  if telemetry_ts is not None:
    clock = proto.Clock()
    clock.clock_id = CLOCK_TELEMETRY
    clock.timestamp = int(telemetry_ts * 1e3)
    clock_snapshot.clocks.append(clock)
  if boottime_ts is not None:
    clock = proto.Clock()
    clock.clock_id = CLOCK_BOOTTIME
    clock.timestamp = int(boottime_ts * 1e3)
    clock_snapshot.clocks.append(clock)
  packet = proto.TracePacket()
  packet.trusted_packet_sequence_id = _get_sequence_id(tid)
  packet.clock_snapshot = clock_snapshot
  proto.write_trace_packet(output, packet)
