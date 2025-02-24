# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from py_utils import tempfile_ext
from tracing.mre import map_single_trace


_GET_TIMELINE_MARKERS = """
function processTrace(results, model) {
    var markers = [];
    for (const thread of model.getAllThreads()) {
        for (const event of thread.asyncSliceGroup.slices) {
            if (event.category === 'blink.console') {
                markers.push(event.title);
            }
        }
    }
    results.addPair('markers', markers);
};
"""

_GET_MEMORY_DUMP_EVENT_IDS = """
function processTrace(results, model) {
    var event_ids = [];
    for (const thread of model.getAllThreads()) {
        for (const event of thread.asyncSliceGroup.slices) {
            if (event.title === 'GlobalMemoryDump') {
                  event_ids.push(event.id);
            }
        }
    }
    results.addPair('event_ids', event_ids);
};
"""

_GET_COMPLETE_SYNC_IDS = """
function processTrace(results, model) {
    results.addPair("sync_ids", model.clockSyncManager.completeSyncIds);
};
"""


def _SerializeAndProcessTrace(trace_builder, process_trace_func_code):
  """Load data into a trace model and run a query on it.

  The current implementation works by loading the trace data into the TBMv2,
  i.e. JavaScript based, timeline model. But this is an implementation detail,
  clients for the public methods of this module should remain agnostic about
  the model being used for trace processing.
  """
  with tempfile_ext.TemporaryFileName() as trace_file:
    try:
      trace_builder.Serialize(trace_file)
      return map_single_trace.ExecuteTraceMappingCode(
          trace_file, process_trace_func_code)
    finally:
      trace_builder.CleanUpTraceData()


def ExtractTimelineMarkers(trace_builder):
  """Get a list with the titles of 'blink.console' events found in a trace.

  This will include any events that were inserted using tab.AddTimelineMarker
  while a trace was being recorded.

  Args:
    trace_builder: A TraceDataBuilder object; trace data is extracted from it
      and the builder itself is cleaned up.
  """
  return _SerializeAndProcessTrace(
      trace_builder, _GET_TIMELINE_MARKERS)['markers']


def ExtractMemoryDumpIds(trace_builder):
  """Get a list with the ids of 'GlobalMemoryDump' events found in a trace.

  Args:
    trace_builder: A TraceDataBuilder object; trace data is extracted from it
      and the builder itself is cleaned up.
  """
  event_ids = _SerializeAndProcessTrace(
      trace_builder, _GET_MEMORY_DUMP_EVENT_IDS)['event_ids']
  # Event ids look like this: 'disabled-by-default-memory-infra:87890:ptr:0x3'.
  # The part after the last ':' is the dump id.
  return [eid.rsplit(':')[-1] for eid in event_ids]


def ExtractCompleteSyncIds(trace_builder):
  """Get a list of ids of complete clock syncs.

  Complete clock syncs a those that have markers from two clock domains.

  Args:
    trace_builder: A TraceDataBuilder object; trace data is extracted from it
      and the builder itself is cleaned up.
  """
  return _SerializeAndProcessTrace(
      trace_builder, _GET_COMPLETE_SYNC_IDS)['sync_ids']
