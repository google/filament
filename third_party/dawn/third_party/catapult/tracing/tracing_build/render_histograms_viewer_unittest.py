# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import codecs
import json
import unittest
import os
import tempfile

from tracing_build import render_histograms_viewer
from six.moves import range


class ResultsRendererTest(unittest.TestCase):

  # Renamed between Python 2 and Python 3.
  try:
    assertCountEqual = unittest.TestCase.assertItemsEqual
  except AttributeError:
    pass

  def setUp(self):
    tmp = tempfile.NamedTemporaryFile(delete=False)
    tmp.close()
    self.output_file = tmp.name
    self.output_stream = codecs.open(self.output_file,
                                     mode='r+',
                                     encoding='utf-8')

  def GetOutputFileContent(self):
    self.output_stream.seek(0)
    return self.output_stream.read()

  def tearDown(self):
    self.output_stream.close()
    os.remove(self.output_file)

  def testBasic(self):
    value0 = {'foo': 0}
    value0_json = json.dumps(value0, separators=(',', ':'))

    render_histograms_viewer.RenderHistogramsViewer(
        [], self.output_stream, False)
    self.output_stream.seek(0)
    self.assertCountEqual([], render_histograms_viewer.ReadExistingResults(
        self.output_stream.read()))
    render_histograms_viewer.RenderHistogramsViewer(
        [value0], self.output_stream, False)
    self.output_stream.seek(0)
    self.assertCountEqual(
        [value0],
        render_histograms_viewer.ReadExistingResults(self.output_stream.read()))
    self.assertIn(value0_json, self.GetOutputFileContent())

  def testBasicWithSeparatorOften(self):
    data_list = [{'foo': i} for i in range(11)]
    render_histograms_viewer.RenderHistogramsViewer(
        [], self.output_stream, False)
    self.output_stream.seek(0)
    self.assertCountEqual([], render_histograms_viewer.ReadExistingResults(
        self.output_stream.read()))
    # Write payload, forcing a new chunk after ever single item
    render_histograms_viewer.RenderHistogramsViewer(
        data_list, self.output_stream, False, max_chunk_size_hint_bytes=1)
    self.output_stream.seek(0)
    self.assertCountEqual(
        data_list,
        render_histograms_viewer.ReadExistingResults(
            self.output_stream.read()))

    for data in data_list:
      data_json = json.dumps(data, separators=(',', ':'))
      self.assertIn(data_json, self.GetOutputFileContent())

  def testBasicWithSeparator(self):
    data_list = [{'foo': i} for i in range(11)]
    render_histograms_viewer.RenderHistogramsViewer(
        [], self.output_stream, False)
    self.output_stream.seek(0)
    self.assertCountEqual([], render_histograms_viewer.ReadExistingResults(
        self.output_stream.read()))
    # Write payload, forcing a new chunk after a few items
    item_json = json.dumps(data_list[2], separators=(',', ':'))
    render_histograms_viewer.RenderHistogramsViewer(
        data_list,
        self.output_stream,
        False,
        max_chunk_size_hint_bytes=len(item_json) * 3)
    self.output_stream.seek(0)
    self.assertCountEqual(
        data_list,
        render_histograms_viewer.ReadExistingResults(
            self.output_stream.read()))
    for data in data_list:
      data_json = json.dumps(data, separators=(',', ':'))
      self.assertIn(data_json, self.GetOutputFileContent())

  def testExistingResults(self):
    value0 = {'foo': 0}
    value0_json = json.dumps(value0, separators=(',', ':'))

    value1 = {'bar': 1}
    value1_json = json.dumps(value1, separators=(',', ':'))

    render_histograms_viewer.RenderHistogramsViewer(
        [value0], self.output_stream, False)
    render_histograms_viewer.RenderHistogramsViewer(
        [value1], self.output_stream, False)
    self.output_stream.seek(0)
    self.assertCountEqual(
        [value0, value1],
        render_histograms_viewer.ReadExistingResults(self.output_stream.read()))
    self.assertIn(value0_json, self.GetOutputFileContent())
    self.assertIn(value1_json, self.GetOutputFileContent())

  def testExistingResultsReset(self):
    value0 = {'foo': 0}
    value0_json = json.dumps(value0, separators=(',', ':'))

    value1 = {'bar': 1}
    value1_json = json.dumps(value1, separators=(',', ':'))

    render_histograms_viewer.RenderHistogramsViewer(
        [value0], self.output_stream, False)
    render_histograms_viewer.RenderHistogramsViewer(
        [value1], self.output_stream, True)
    self.output_stream.seek(0)
    self.assertCountEqual(
        [value1],
        render_histograms_viewer.ReadExistingResults(self.output_stream.read()))
    self.assertNotIn(value0_json, self.GetOutputFileContent())
    self.assertIn(value1_json, self.GetOutputFileContent())

  def testHtmlEscape(self):
    # No escaping is needed since data is stored in an html comment.
    render_histograms_viewer.RenderHistogramsViewer(
        [{'name': '<a><b>'}], self.output_stream, False)
    self.assertIn('<a><b>', self.GetOutputFileContent())
