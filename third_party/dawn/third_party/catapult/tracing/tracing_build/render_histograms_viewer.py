# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import logging
import re

_DATA_START = '<div id="histogram-json-data" style="display:none;"><!--'
_DATA_SEPARATOR = '--><!--'
_DATA_END_OLD = '--!></div>'
_DATA_END = '--></div>'


def ExtractJSON(results_html):
  results = []
  flags = re.MULTILINE | re.DOTALL
  start = re.search("^%s" % re.escape(_DATA_START), results_html, flags)
  if start is None:
    logging.warning('Could not find histogram data start: %s', _DATA_START)
    return []
  pattern = '^((%s|%s)|(.*?))$' % (re.escape(_DATA_END_OLD),
                                   re.escape(_DATA_END))
  # Find newlines and parse each line as separate JSON data.
  for match in re.compile(pattern, flags).finditer(results_html, start.end()+1):
    try:
      # Check if the end tag in group(2) got a match.
      if match.group(2):
        return results
      payload = match.group(3)
      if payload == _DATA_SEPARATOR:
        continue
      results.append(json.loads(payload))
    except ValueError:
      logging.warning(
          'Found existing results json, but failed to parse it: %s',
          match.group(1))
      return []
  return results


def ReadExistingResults(results_html):
  if not results_html:
    return []

  histogram_dicts = ExtractJSON(results_html)

  if not histogram_dicts:
    logging.warning('Failed to extract previous results from HTML output')
  return histogram_dicts


def RenderHistogramsViewer(histogram_dicts, output_stream, reset_results=False,
                           vulcanized_html='',
                           max_chunk_size_hint_bytes=100000000):
  """Renders a Histograms viewer to output_stream containing histogram_dicts.

  Requires a Histograms viewer to have already been vulcanized.
  The vulcanized viewer can be provided either as a string or a file.

  Args:
    histogram_dicts: list of dictionaries containing Histograms.
    output_stream: file-like stream to read existing results and write HTML.
    reset_results: whether to drop existing results.
    vulcanized_html: HTML string of vulcanized histograms viewer.
  """
  output_stream.seek(0)

  if not reset_results:
    results_html = output_stream.read()
    output_stream.seek(0)
    histogram_dicts += ReadExistingResults(results_html)

  output_stream.write(vulcanized_html)
  # Put all the serialized histograms nodes inside an html comment to avoid
  # unnecessary stress on html parsing and avoid creating throw-away dom nodes.
  output_stream.write(_DATA_START)

  chunk_size = 0
  for histogram in histogram_dicts:
    output_stream.write('\n')
    # No escaping is necessary since the data is stored inside an html comment.
    # This assumes that {hist_json} doesn't contain an html comment itself.
    hist_json = json.dumps(histogram, separators=(',', ':'))
    output_stream.write(hist_json)
    chunk_size += len(hist_json) + 1
    # Start a new comment after {max_chunk_size_hint_bytes} to avoid hitting
    # V8's max string length. Each comment can be read as individual string.
    if chunk_size > max_chunk_size_hint_bytes:
      output_stream.write('\n%s' % _DATA_SEPARATOR)
      chunk_size = 0
  output_stream.write('\n%s\n' % _DATA_END)

  # If the output file already existed and was longer than the new contents,
  # discard the old contents after this point.
  output_stream.truncate()
