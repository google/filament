# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import codecs
import csv
import gzip
import json
import logging

from six.moves import map  # pylint: disable=redefined-builtin
from tracing_build import html2trace, trace2html

GZIP_FILENAME_SUFFIX = '.gz'
HTML_FILENAME_SUFFIX = '.html'
JSON_FILENAME_SUFFIX = '.json'

TRACE_METADATA_ARG_NAME_MAP = {
    'process_sort_index': 'sort_index',
    'process_name': 'name'
}


def LoadTraces(chrome_trace_filename):
  """Load traces from a file and return them as a python list.

  There are several tools in tracing/bin that deal with reading traces from a
  file. None of them does exactly what we need here. In particular,
  merge_traces.LoadTrace will discard all traces but one if an HTML file has
  several traces.

  TODO(chiniforooshan): create a module for reading/writing different trace file
  formats and make every other tool use that module.
  """
  traces = []
  if chrome_trace_filename.endswith(GZIP_FILENAME_SUFFIX):
    with gzip.open(chrome_trace_filename, 'rb') as f:
      traces.append(json.load(f))
  elif chrome_trace_filename.endswith(HTML_FILENAME_SUFFIX):
    with codecs.open(chrome_trace_filename, mode='r', encoding='utf-8') as f:
      traces = html2trace.ReadTracesFromHTMLFile(f)
  elif chrome_trace_filename.endswith(JSON_FILENAME_SUFFIX):
    with open(chrome_trace_filename, 'r') as f:
      traces.append(json.load(f))
  else:
    raise Exception('Unknown trace file suffix: %s' % chrome_trace_filename)
  return list(map(_ConvertToDictIfNecessary, traces))


def WriteTraces(output_filename, traces):
  if output_filename.endswith(GZIP_FILENAME_SUFFIX):
    with gzip.open(output_filename, 'wb') as f:
      json.dump(_ConcatTraces(traces), f)
  elif output_filename.endswith(HTML_FILENAME_SUFFIX):
    with codecs.open(output_filename, mode='w', encoding='utf-8') as f:
      trace2html.WriteHTMLForTraceDataToFile(
          traces, 'Chrome trace with Snapdragon profiler data', f)
  elif output_filename.endswith(JSON_FILENAME_SUFFIX):
    with open(output_filename, 'w') as f:
      json.dump(_ConcatTraces(traces), f)
  else:
    raise Exception('Unknown trace file suffix: %s' % output_filename)


def LoadCSV(csv_filename):
  with open(csv_filename, 'r') as csv_file:
    reader = csv.DictReader(csv_file, delimiter=',')
    for row in reader:
      yield row


def AddSnapdragonProfilerData(traces, snapdragon_csv):
  clock_offset = None
  min_ts = None
  max_ts = 0
  max_pid = 0
  for trace in traces:
    should_use_timestamps_from_this_trace = False
    if 'metadata' in trace and 'clock-offset-since-epoch' in trace['metadata']:
      if clock_offset is not None:
        logging.warning('Found more than one clock offset')
      else:
        clock_offset = int(trace['metadata']['clock-offset-since-epoch'])
        should_use_timestamps_from_this_trace = True
    if 'traceEvents' in trace:
      for event in trace['traceEvents']:
        max_pid = max(max_pid, event['pid'], event['tid'])
        if should_use_timestamps_from_this_trace and event['ph'] != 'M':
          ts = event['ts']
          max_ts = max(ts + (event['dur'] if 'dur' in event else 0), max_ts)
          if min_ts is None or min_ts > ts:
            min_ts = ts
  if clock_offset is None:
    raise Exception('Cannot find clock offset in Chrome traces')

  process_names = {}
  num_counter_events = 0
  events = []
  for row in snapdragon_csv:
    ts = int(row['TimestampRaw']) - clock_offset
    if ts < min_ts or ts > max_ts:
      continue
    if row['Process'] not in process_names:
      max_pid += 1
      process_names[row['Process']] = max_pid
      events.append(_MetadataEvent(max_pid, 'process_sort_index', -7))
      events.append(_MetadataEvent(max_pid, 'process_name', row['Process']))
    pid = process_names[row['Process']]
    events.append(_CounterEvent(pid, ts, row['Metric'], row['Value']))
    num_counter_events += 1
  logging.info('Added %d counter events.', num_counter_events)
  traces.append({'traceEvents': events})


def _ConcatTraces(traces):
  # As long as at most one of the traces has a field with a non-list value, e.g.
  # a metadata field, we can trivially concat them.
  #
  # TODO(chiniforooshan): to support writing several traces with several
  # metadata fields in a JSON file, we can write a simple
  # trace_list_importer.html that treats each of them as a subtrace.
  #
  # Note: metadata fields should not be confused with metadata trace events.
  result = None
  for trace in traces:
    if result is None:
      result = trace.copy()
      continue
    for k, v in trace.items():
      if k in result:
        if not isinstance(v, list):
          raise Exception('Cannot concat two traces with non-list values '
                          '(e.g. two traces with metadata)')
        result[k].extend(v)
      else:
        result[k] = list(v) if isinstance(v, list) else v
  return result


def _ConvertToDictIfNecessary(trace):
  return {'traceEvents': trace} if isinstance(trace, list) else trace


def _MetadataEvent(pid, name, value):
  if name not in TRACE_METADATA_ARG_NAME_MAP:
    raise Exception('Unknown metadata name: %s' % name)
  arg_name = TRACE_METADATA_ARG_NAME_MAP[name]
  return {
      'pid': pid,
      'tid': pid,
      'ts': 0,
      'ph': 'M',
      'cat': '__metadata',
      'name': name,
      'args': {arg_name: value}
  }


def _CounterEvent(pid, timestamp, name, value):
  if not isinstance(value, float):
    value = float(value)
  return {
      'pid': pid,
      'tid': pid,
      'ts': timestamp,
      'ph': 'C',
      'name': name,
      'args': {'Value': value}
  }
