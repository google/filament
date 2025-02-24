# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import codecs
import gzip
import io
import os
import shutil
import tempfile
import unittest

from tracing_build import trace2html

class Trace2HTMLTests(unittest.TestCase):
  SIMPLE_TRACE_PATH = os.path.join(
      os.path.dirname(__file__),
      '..', 'test_data', 'simple_trace.json')
  BIG_TRACE_PATH = os.path.join(
      os.path.dirname(__file__),
      '..', 'test_data', 'big_trace.json')
  NON_JSON_TRACE_PATH = os.path.join(
      os.path.dirname(__file__),
      '..', 'test_data', 'android_systrace.txt')

  def setUp(self):
    self._tmpdir = tempfile.mkdtemp()

  def tearDown(self):
    shutil.rmtree(self._tmpdir, ignore_errors=True)

  def testGzippedTraceIsntDoubleGzipped(self):
    # |input_filename| will contain plain JSON data at one point, then gzipped
    # JSON data at another point for reasons that will be explained below.
    input_filename = os.path.join(self._tmpdir, 'GzippedTraceIsntDoubleGzipped')

    output_filename = os.path.join(
        self._tmpdir, 'GzippedTraceIsntDoubleGzipped.html')

    # trace2html-ify SIMPLE_TRACE, but from a controlled input filename so
    # that when ViewerDataScript gzips it, it uses the same filename for both
    # the unzipped SIMPLE_TRACE here and the gzipped SIMPLE_TRACE below.
    open(input_filename, 'w').write(open(self.SIMPLE_TRACE_PATH).read())
    with codecs.open(output_filename, 'w', encoding='utf-8') as output_file:
      trace2html.WriteHTMLForTracesToFile([input_filename], output_file)

    # Hash the contents of the output file that was generated from an unzipped
    # json input file.
    unzipped_hash = hash(
        io.open(output_filename, 'r', encoding='utf-8').read())

    os.unlink(output_filename)

    # Gzip SIMPLE_TRACE into |input_filename|.
    # trace2html should automatically gunzip it and start building the html from
    # the same input as if the input weren't gzipped.
    with gzip.GzipFile(input_filename, mode='wb') as input_gzipfile:
      input_gzipfile.write(open(self.SIMPLE_TRACE_PATH).read().encode('utf-8'))

    # trace2html-ify the zipped version of SIMPLE_TRACE from the same input
    # filename as the unzipped version so that the gzipping process is stable.
    with codecs.open(output_filename, 'w', encoding='utf-8') as output_file:
      trace2html.WriteHTMLForTracesToFile([input_filename], output_file)

    # Hash the contents of the output file that was generated from the zipped
    # json input file.
    zipped_hash = hash(io.open(output_filename, 'r', encoding='utf-8').read())

    # Compare the hashes, not the file contents directly so that, if they are
    # different, python shouldn't try to print megabytes of html.
    self.assertEqual(unzipped_hash, zipped_hash)

  def testWriteHTMLForTracesToFile(self):
    output_filename = os.path.join(
        self._tmpdir, 'WriteHTMLForTracesToFile.html')
    with codecs.open(output_filename, 'w', encoding='utf-8') as output_file:
      trace2html.WriteHTMLForTracesToFile([
          self.BIG_TRACE_PATH,
          self.SIMPLE_TRACE_PATH,
          self.NON_JSON_TRACE_PATH
      ], output_file)
