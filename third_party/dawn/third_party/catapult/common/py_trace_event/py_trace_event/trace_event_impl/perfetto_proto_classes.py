# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Classes representing perfetto trace protobuf messages.

This module makes use of neither python-protobuf library nor python classes
compiled from .proto definitions, because currently there's no way to
deploy those to all the places where telemetry is run.

TODO(crbug.com/944078): Remove this module after the python-protobuf library
is deployed to all the bots.

Definitions of perfetto messages can be found here:
https://android.googlesource.com/platform/external/perfetto/+/refs/heads/master/protos/perfetto/trace/
"""

from __future__ import absolute_import
from google.protobuf.internal import encoder, wire_format


# When moving from protobuf 3.0.0 to 4.21.9, encoders started taking a
# required "deterministic" argument. We use this instead of a raw True value to
# improve readability.
DETERMINISTIC = True


class TracePacket(object):
  def __init__(self):
    self.clock_snapshot = None
    self.timestamp = None
    self.timestamp_clock_id = None
    self.interned_data = None
    self.thread_descriptor = None
    self.incremental_state_cleared = None
    self.chrome_event = None
    self.track_event = None
    self.trusted_packet_sequence_id = None
    self.chrome_benchmark_metadata = None

  def encode(self):
    parts = []
    if self.chrome_event is not None:
      tag = encoder.TagBytes(5, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.chrome_event.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.clock_snapshot is not None:
      tag = encoder.TagBytes(6, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.clock_snapshot.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.timestamp is not None:
      writer = encoder.UInt64Encoder(8, False, False)
      writer(parts.append, self.timestamp, DETERMINISTIC)
    if self.trusted_packet_sequence_id is not None:
      writer = encoder.UInt32Encoder(10, False, False)
      writer(parts.append, self.trusted_packet_sequence_id, DETERMINISTIC)
    if self.track_event is not None:
      tag = encoder.TagBytes(11, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.track_event.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.interned_data is not None:
      tag = encoder.TagBytes(12, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.interned_data.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.incremental_state_cleared is not None:
      writer = encoder.BoolEncoder(41, False, False)
      writer(parts.append, self.incremental_state_cleared, DETERMINISTIC)
    if self.thread_descriptor is not None:
      tag = encoder.TagBytes(44, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.thread_descriptor.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.chrome_benchmark_metadata is not None:
      tag = encoder.TagBytes(48, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.chrome_benchmark_metadata.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.timestamp_clock_id is not None:
      writer = encoder.UInt32Encoder(58, False, False)
      writer(parts.append, self.timestamp_clock_id, DETERMINISTIC)

    return b"".join(parts)


class InternedData(object):
  def __init__(self):
    self.event_category = None
    self.legacy_event_name = None

  def encode(self):
    parts = []
    if self.event_category is not None:
      tag = encoder.TagBytes(1, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.event_category.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.legacy_event_name is not None:
      tag = encoder.TagBytes(2, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.legacy_event_name.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]

    return b"".join(parts)


class EventCategory(object):
  def __init__(self):
    self.iid = None
    self.name = None

  def encode(self):
    if (self.iid is None or self.name is None):
      raise RuntimeError("Missing mandatory fields.")

    parts = []
    writer = encoder.UInt32Encoder(1, False, False)
    writer(parts.append, self.iid, DETERMINISTIC)
    writer = encoder.StringEncoder(2, False, False)
    writer(parts.append, self.name, DETERMINISTIC)

    return b"".join(parts)


LegacyEventName = EventCategory


class ThreadDescriptor(object):
  def __init__(self):
    self.pid = None
    self.tid = None

  def encode(self):
    if (self.pid is None or self.tid is None):
      raise RuntimeError("Missing mandatory fields.")

    parts = []
    writer = encoder.UInt32Encoder(1, False, False)
    writer(parts.append, self.pid, DETERMINISTIC)
    writer = encoder.UInt32Encoder(2, False, False)
    writer(parts.append, self.tid, DETERMINISTIC)

    return b"".join(parts)


class ChromeEventBundle(object):
  def __init__(self):
    self.metadata = []

  def encode(self):
    parts = []
    for item in self.metadata:
      tag = encoder.TagBytes(2, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = item.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]

    return b"".join(parts)


class TrackEvent(object):
  def __init__(self):
    self.legacy_event = None
    self.category_iids = None
    self.debug_annotations = []

  def encode(self):
    parts = []
    if self.category_iids is not None:
      writer = encoder.UInt32Encoder(3, is_repeated=True, is_packed=False)
      writer(parts.append, self.category_iids, DETERMINISTIC)
    for annotation in self.debug_annotations:
      tag = encoder.TagBytes(4, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = annotation.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]
    if self.legacy_event is not None:
      tag = encoder.TagBytes(6, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = self.legacy_event.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]

    return b"".join(parts)


class LegacyEvent(object):
  def __init__(self):
    self.phase = None
    self.name_iid = None

  def encode(self):
    parts = []
    if self.name_iid is not None:
      writer = encoder.UInt32Encoder(1, False, False)
      writer(parts.append, self.name_iid, DETERMINISTIC)
    if self.phase is not None:
      writer = encoder.Int32Encoder(2, False, False)
      writer(parts.append, self.phase, DETERMINISTIC)

    return b"".join(parts)


class ChromeBenchmarkMetadata(object):
  def __init__(self):
    self.benchmark_start_time_us = None
    self.story_run_time_us = None
    self.benchmark_name = None
    self.benchmark_description = None
    self.story_name = None
    self.story_tags = None
    self.story_run_index = None
    self.label = None

  def encode(self):
    parts = []
    if self.benchmark_start_time_us is not None:
      writer = encoder.Int64Encoder(1, False, False)
      writer(parts.append, self.benchmark_start_time_us, DETERMINISTIC)
    if self.story_run_time_us is not None:
      writer = encoder.Int64Encoder(2, False, False)
      writer(parts.append, self.story_run_time_us, DETERMINISTIC)
    if self.benchmark_name is not None:
      writer = encoder.StringEncoder(3, False, False)
      writer(parts.append, self.benchmark_name, DETERMINISTIC)
    if self.benchmark_description is not None:
      writer = encoder.StringEncoder(4, False, False)
      writer(parts.append, self.benchmark_description, DETERMINISTIC)
    if self.label is not None:
      writer = encoder.StringEncoder(5, False, False)
      writer(parts.append, self.label, DETERMINISTIC)
    if self.story_name is not None:
      writer = encoder.StringEncoder(6, False, False)
      writer(parts.append, self.story_name, DETERMINISTIC)
    if self.story_tags is not None:
      writer = encoder.StringEncoder(7, is_repeated=True, is_packed=False)
      writer(parts.append, self.story_tags, DETERMINISTIC)
    if self.story_run_index is not None:
      writer = encoder.Int32Encoder(8, False, False)
      writer(parts.append, self.story_run_index, DETERMINISTIC)

    return b"".join(parts)


def write_trace_packet(output, trace_packet):
  tag = encoder.TagBytes(1, wire_format.WIRETYPE_LENGTH_DELIMITED)
  output.write(tag)
  binary_data = trace_packet.encode()
  encoder._EncodeVarint(output.write, len(binary_data))
  output.write(binary_data)


class DebugAnnotation(object):
  def __init__(self):
    self.name = None
    self.int_value = None
    self.double_value = None
    self.string_value = None

  def encode(self):
    if self.name is None:
      raise RuntimeError("DebugAnnotation must have a name.")
    if ((self.string_value is not None) +
        (self.int_value is not None) +
        (self.double_value is not None)) != 1:
      raise RuntimeError("DebugAnnotation must have exactly one value.")

    parts = []
    writer = encoder.StringEncoder(10, False, False)
    writer(parts.append, self.name, DETERMINISTIC)
    if self.int_value is not None:
      writer = encoder.Int64Encoder(4, False, False)
      writer(parts.append, self.int_value, DETERMINISTIC)
    if self.double_value is not None:
      writer = encoder.DoubleEncoder(5, False, False)
      writer(parts.append, self.double_value, DETERMINISTIC)
    if self.string_value is not None:
      writer = encoder.StringEncoder(6, False, False)
      writer(parts.append, self.string_value, DETERMINISTIC)

    return b"".join(parts)


class ChromeMetadata(object):
  def __init__(self):
    self.name = None
    self.string_value = None

  def encode(self):
    if self.name is None or self.string_value is None:
      raise RuntimeError("ChromeMetadata must have a name and a value.")

    parts = []
    writer = encoder.StringEncoder(1, False, False)
    writer(parts.append, self.name, DETERMINISTIC)
    writer = encoder.StringEncoder(2, False, False)
    writer(parts.append, self.string_value, DETERMINISTIC)

    return b"".join(parts)


class Clock(object):
  def __init__(self):
    self.clock_id = None
    self.timestamp = None

  def encode(self):
    if self.clock_id is None or self.timestamp is None:
      raise RuntimeError("Clock must have a clock_id and a timestamp.")

    parts = []
    writer = encoder.UInt32Encoder(1, False, False)
    writer(parts.append, self.clock_id, DETERMINISTIC)
    writer = encoder.UInt64Encoder(2, False, False)
    writer(parts.append, self.timestamp, DETERMINISTIC)

    return b"".join(parts)


class ClockSnapshot(object):
  def __init__(self):
    self.clocks = []

  def encode(self):
    if len(self.clocks) < 2:
      raise RuntimeError("ClockSnapshot must have at least two clocks.")

    parts = []
    for clock in self.clocks:
      tag = encoder.TagBytes(1, wire_format.WIRETYPE_LENGTH_DELIMITED)
      data = clock.encode()
      length = encoder._VarintBytes(len(data))
      parts += [tag, length, data]

    return b"".join(parts)
