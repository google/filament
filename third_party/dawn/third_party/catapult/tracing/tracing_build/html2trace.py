# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import base64
import gzip
import io
import json
import re
from six.moves import map  # pylint: disable=redefined-builtin
from six.moves import range  # pylint: disable=redefined-builtin


GZIP_HEADER_BYTES = b'\x1f\x8b'


# Regular expressions for matching the beginning and end of trace data in HTML
# traces. See tracing/extras/importer/trace2html_importer.html.
TRACE_DATA_START_LINE_RE = re.compile(
    r'^<\s*script id="viewer-data" type="(application\/json|text\/plain)">\r?$')
TRACE_DATA_END_LINE_RE = re.compile(r'^<\/\s*script>\r?$')


def IsHTMLTrace(trace_file_handle):
  trace_file_handle.seek(0)
  for line in trace_file_handle:
    line = line.strip()
    if not line:
      continue
    return line == '<!DOCTYPE html>'


def CopyTraceDataFromHTMLFilePath(html_file_handle, trace_path,
                                  gzipped_output=False):
  """Copies trace data from an existing HTML file into new trace file(s).

  If |html_file_handle| doesn't contain any trace data blocks, this function
  throws an exception. If |html_file_handle| contains more than one trace data
  block, the first block will be extracted into |trace_path| and the rest will
  be extracted into separate files |trace_path|.1, |trace_path|.2, etc.

  The contents of each trace data block is decoded and, if |gzipped_output| is
  false, inflated before it's stored in a trace file.

  This function returns a list of paths of the saved trace files ([|trace_path|,
  |trace_path|.1, |trace_path|.2, ...]).
  """
  trace_data_list = _ExtractTraceDataFromHTMLFile(html_file_handle,
                                                  unzip_data=not gzipped_output)
  saved_paths = []
  for i, trace_data in enumerate(trace_data_list):
    saved_path = trace_path if i == 0 else '%s.%d' % (trace_path, i)
    saved_paths.append(saved_path)
    with open(saved_path, 'wb' if gzipped_output else 'w') as trace_file:
      if gzipped_output:
        trace_file.write(trace_data.read())
      else:
        trace_file.write(trace_data.read().decode("utf-8"))
  return saved_paths


def ReadTracesFromHTMLFile(file_handle):
  """Returns a list of inflated JSON traces extracted from an HTML file."""
  return [json.load(t) for t in _ExtractTraceDataFromHTMLFile(file_handle)]


def _ExtractTraceDataFromHTMLFile(html_file_handle, unzip_data=True):
  assert IsHTMLTrace(html_file_handle)
  html_file_handle.seek(0)
  lines = html_file_handle.readlines()

  start_indices = [i for i in range(len(lines))
                   if TRACE_DATA_START_LINE_RE.match(lines[i])]
  if not start_indices:
    raise Exception('File %r does not contain trace data')

  decoded_data_list = []
  for start_index in start_indices:
    end_index = next(i for i in range(start_index + 1, len(lines))
                     if TRACE_DATA_END_LINE_RE.match(lines[i]))
    encoded_data = '\n'.join(lines[start_index + 1:end_index]).strip()
    decoded_data_list.append(io.BytesIO(base64.b64decode(encoded_data)))

  if unzip_data:
    return list(map(_UnzipFileIfNecessary, decoded_data_list))
  return list(map(_ZipFileIfNecessary, decoded_data_list))


def _UnzipFileIfNecessary(original_file):
  if _IsFileZipped(original_file):
    return gzip.GzipFile(fileobj=original_file)
  return original_file


def _ZipFileIfNecessary(original_file):
  if _IsFileZipped(original_file):
    return original_file
  zipped_file = io.BytesIO()
  with gzip.GzipFile(fileobj=zipped_file, mode='wb') as gzip_wrapper:
    gzip_wrapper.write(original_file.read())
  zipped_file.seek(0)
  return zipped_file


def _IsFileZipped(f):
  is_gzipped = f.read(len(GZIP_HEADER_BYTES)) == GZIP_HEADER_BYTES
  f.seek(0)
  return is_gzipped
