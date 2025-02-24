# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import codecs
import json
import logging
import os

from six.moves import map  # pylint: disable=redefined-builtin
from tracing_build import html2trace
from tracing_build import trace2html


def GetFileSizeInMb(path):
  return os.path.getsize(path) >> 20


def Main(argv):
  parser = argparse.ArgumentParser(
      description='Slim a trace to a more managable size')
  parser.add_argument('trace_path', metavar='TRACE_PATH', type=str,
                      help='trace file path (input).')
  options = parser.parse_args(argv[1:])

  trace_path = os.path.abspath(options.trace_path)

  orignal_trace_name = os.path.splitext(os.path.basename(trace_path))[0]
  slimmed_trace_path = os.path.join(
      os.path.dirname(trace_path), 'slimmed_%s.html' % orignal_trace_name)

  with codecs.open(trace_path, mode='r', encoding='utf-8') as f:
    SlimTrace(f, slimmed_trace_path)

  print(
      ('Original trace %s (%s Mb)' % (trace_path, GetFileSizeInMb(trace_path))))
  print(('Slimmed trace file://%s (%s Mb)' %
         (slimmed_trace_path, GetFileSizeInMb(slimmed_trace_path))))


def SlimTraceEventsList(events_list):
  filtered_events = []
  # Filter out all events of phase complete that takes less than 20
  # microseconds.
  for e in events_list:
    dur = e.get('dur', 0)
    if e['ph'] != 'X' or dur >= 20:
      filtered_events.append(e)
  return filtered_events


def SlimSingleTrace(trace_data):
  if isinstance(trace_data, dict):
    trace_data['traceEvents'] = SlimTraceEventsList(trace_data['traceEvents'])
  elif isinstance(trace_data, list) and isinstance(trace_data[0], dict):
    trace_data = SlimTraceEventsList(trace_data)
  else:
    logging.warning('Cannot slim trace %s...', trace_data[:10])
  return trace_data


class TraceExtractor(object):
  def CanExtractFile(self, trace_file_handle):
    raise NotImplementedError

  def ExtractTracesFromFile(self, trace_file_handle):
    raise NotImplementedError


class HTMLTraceExtractor(TraceExtractor):
  def CanExtractFile(self, trace_file_handle):
    return html2trace.IsHTMLTrace(trace_file_handle)

  def ExtractTracesFromFile(self, trace_file_handle):
    return html2trace.ReadTracesFromHTMLFile(trace_file_handle)


class JsonTraceExtractor(TraceExtractor):
  def CanExtractFile(self, trace_file_handle):
    trace_file_handle.seek(0)
    begin_char = trace_file_handle.read(1)
    trace_file_handle.seek(-1, 2)
    end_char = trace_file_handle.read(1)
    return ((begin_char == '{' and end_char == '}') or
            (begin_char == '[' and end_char == ']'))

  def ExtractTracesFromFile(self, trace_file_handle):
    trace_file_handle.seek(0)
    return [json.load(trace_file_handle)]


ALL_TRACE_EXTRACTORS = [
    HTMLTraceExtractor(),
    JsonTraceExtractor()
]


def SlimTrace(trace_file_handle, slimmed_trace_path):
  traces = None
  for extractor in ALL_TRACE_EXTRACTORS:
    if extractor.CanExtractFile(trace_file_handle):
      traces = extractor.ExtractTracesFromFile(trace_file_handle)
      break

  if traces is None:
    raise Exception('Cannot extrac trace from %s' % trace_file_handle.name)

  slimmed_traces = list(map(SlimSingleTrace, traces))

  with codecs.open(slimmed_trace_path, mode='w', encoding='utf-8') as f:
    trace2html.WriteHTMLForTraceDataToFile(
        slimmed_traces, title='', output_file=f)
