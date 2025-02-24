#!/usr/bin/env python

# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import gzip
import json
import os

try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO
import io

import six

from systrace import tracing_controller
from systrace import trace_result
from tracing.trace_data import trace_data


# TODO(alexandermont): Current version of trace viewer does not support
# the controller tracing agent output. Thus we use this variable to
# suppress this tracing agent's output. This should be removed once
# trace viewer is working again.
OUTPUT_CONTROLLER_TRACE_ = False
CONTROLLER_TRACE_DATA_KEY = 'controllerTraceDataKey'
_SYSTRACE_TO_TRACE_DATA_NAME_MAPPING = {
    'androidProcessDump': trace_data.ANDROID_PROCESS_DATA_PART,
    'atraceProcessDump': trace_data.ATRACE_PROCESS_DUMP_PART,
    'systemTraceEvents': trace_data.ATRACE_PART,
    'systraceController': trace_data.TELEMETRY_PART,
    'traceEvents': trace_data.CHROME_TRACE_PART,
    'waltTrace': trace_data.WALT_TRACE_PART,
    'cgroupDump': trace_data.CGROUP_TRACE_PART,
}
_SYSTRACE_HEADER = 'Systrace'


def NewGenerateHTMLOutput(trace_results, output_file_name):
  with trace_data.TraceDataBuilder() as builder:
    for trace in trace_results:
      trace_data_part = _SYSTRACE_TO_TRACE_DATA_NAME_MAPPING.get(
          trace.source_name)
      builder.AddTraceFor(
          trace_data_part, trace.raw_data, allow_unstructured=True)
    builder.Serialize(output_file_name, _SYSTRACE_HEADER)


def GenerateHTMLOutput(trace_results, output_file_name):
  """Write the results of systrace to an HTML file.

  Args:
      trace_results: A list of TraceResults.
      output_file_name: The name of the HTML file that the trace viewer
          results should be written to.
  """
  def _ReadAsset(src_dir, filename):
    with io.open(os.path.join(src_dir, filename), encoding='utf-8') as f:
      return six.ensure_str(f.read())

  # TODO(rnephew): The tracing output formatter is able to handle a single
  # systrace trace just as well as it handles multiple traces. The obvious thing
  # to do here would be to use it all for all systrace output: however, we want
  # to continue using the legacy way of formatting systrace output when a single
  # systrace and the tracing controller trace are present in order to match the
  # Java verison of systrace. Java systrace is expected to be deleted at a later
  # date. We should consolidate this logic when that happens.

  if len(trace_results) > 4:
    NewGenerateHTMLOutput(trace_results, output_file_name)
    return os.path.abspath(output_file_name)

  systrace_dir = os.path.abspath(os.path.dirname(__file__))

  try:
    # TODO(https://crbug.com/1262296): Update this after Python2 trybots retire.
    # pylint: disable=import-outside-toplevel
    from systrace import update_systrace_trace_viewer
  except ImportError:
    pass
  else:
    update_systrace_trace_viewer.update()

  trace_viewer_html = _ReadAsset(systrace_dir, 'systrace_trace_viewer.html')

  # Open the file in binary mode to prevent python from changing the
  # line endings, then write the prefix.
  systrace_dir = os.path.abspath(os.path.dirname(__file__))
  html_prefix = _ReadAsset(systrace_dir, 'prefix.html.template')
  html_suffix = _ReadAsset(systrace_dir, 'suffix.html')
  trace_viewer_html = _ReadAsset(systrace_dir,
                                  'systrace_trace_viewer.html')
  catapult_root = os.path.abspath(os.path.dirname(os.path.dirname(
                                  os.path.dirname(__file__))))
  polymer_dir = os.path.join(catapult_root, 'third_party', 'polymer',
                             'components', 'webcomponentsjs')
  webcomponent_v0_polyfill = _ReadAsset(polymer_dir, 'webcomponents.min.js')

  # Add the polyfill
  html_output = html_prefix.replace('{{WEBCOMPONENTS_V0_POLYFILL_JS}}',
                                    webcomponent_v0_polyfill)

  # Open the file in binary mode to prevent python from changing the
  # line endings, then write the prefix.
  html_file = open(output_file_name, 'wb')
  html_file.write(
    six.ensure_binary(
      html_output.replace('{{SYSTRACE_TRACE_VIEWER_HTML}}', trace_viewer_html)))



  # Write the trace data itself. There is a separate section of the form
  # <script class="trace-data" type="application/text"> ... </script>
  # for each tracing agent (including the controller tracing agent).
  html_file.write(b'<!-- BEGIN TRACE -->\n')
  for result in trace_results:
    html_file.write(b'  <script class="trace-data" type="application/text">\n')
    html_file.write(six.ensure_binary(_ConvertToHtmlString(result.raw_data)))
    html_file.write(b'  </script>\n')
  html_file.write(b'<!-- END TRACE -->\n')

  # Write the suffix and finish.
  html_file.write(six.ensure_binary(html_suffix))
  html_file.close()

  final_path = os.path.abspath(output_file_name)
  return final_path

def _ConvertToHtmlString(result):
  """Convert a trace result to the format to be output into HTML.

  If the trace result is a dictionary or list, JSON-encode it.
  If the trace result is a string, leave it unchanged.
  """
  if isinstance(result, (dict, list)):
    return json.dumps(result)
  if isinstance(result, six.string_types):
    return result
  if isinstance(result, bytes):
    return result.decode('utf-8')
  raise ValueError('Invalid trace result format for HTML output')

def GenerateJSONOutput(trace_results, output_file_name):
  """Write the results of systrace to a JSON file.

  Args:
      trace_results: A list of TraceResults.
      output_file_name: The name of the JSON file that the trace viewer
          results should be written to.
  """
  results = _ConvertTraceListToDictionary(trace_results)
  results[CONTROLLER_TRACE_DATA_KEY] = (
      tracing_controller.TRACE_DATA_CONTROLLER_NAME)
  with open(output_file_name, 'w') as json_file:
    json.dump(results, json_file)
  final_path = os.path.abspath(output_file_name)
  return final_path

def MergeTraceResultsIfNeeded(trace_results):
  """Merge a list of trace data, if possible. This function can take any list
     of trace data, but it will only merge the JSON data (since that's all
     we can merge).

     Args:
        trace_results: A list of TraceResults containing trace data.
  """
  if len(trace_results) <= 1:
    return trace_results
  merge_candidates = []
  for result in trace_results:
    # Try to detect a JSON file cheaply since that's all we can merge.
    if result.raw_data[0] != '{':
      continue
    try:
      json_data = json.loads(result.raw_data)
    except ValueError:
      continue
    merge_candidates.append(trace_result.TraceResult(result.source_name,
                                                     json_data))

  if len(merge_candidates) <= 1:
    return trace_results

  other_results = [r for r in trace_results
                   if not r.source_name in
                   [c.source_name for c in merge_candidates]]

  merged_data = merge_candidates[0].raw_data

  for candidate in merge_candidates[1:]:
    json_data = candidate.raw_data
    for key, value in json_data.items():
      if not str(key) in merged_data or not merged_data[str(key)]:
        merged_data[str(key)] = value

  return ([trace_result.TraceResult('merged-data', json.dumps(merged_data))]
              + other_results)

def _EncodeTraceData(trace_string):
  compressed_trace = StringIO()
  with gzip.GzipFile(fileobj=compressed_trace, mode='w') as f:
    f.write(trace_string)
  b64_content = base64.b64encode(compressed_trace.getvalue())
  return b64_content

def _ConvertTraceListToDictionary(trace_list):
  trace_dict = {}
  for trace in trace_list:
    trace_dict[trace.source_name] = trace.raw_data
  return trace_dict
