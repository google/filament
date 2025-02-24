#!/usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import logging
import os
import sys
import unittest

from .log import *
from .parsed_trace_events import *
from py_utils import tempfile_ext


class LogIOTest(unittest.TestCase):
  def test_enable_with_file(self):
    with tempfile_ext.TemporaryFileName() as filename:
      trace_enable(open(filename, 'w+'))
      trace_disable()
      e = ParsedTraceEvents(trace_filename=filename)
      self.assertTrue(len(e) > 0)

  def test_enable_with_filename(self):
    with tempfile_ext.TemporaryFileName() as filename:
      trace_enable(filename)
      trace_disable()
      e = ParsedTraceEvents(trace_filename=filename)
      self.assertTrue(len(e) > 0)

  def test_enable_with_implicit_filename(self):
    expected_filename = "%s.json" % sys.argv[0]
    def do_work():
      trace_enable()
      trace_disable()
      e = ParsedTraceEvents(trace_filename=expected_filename)
      self.assertTrue(len(e) > 0)
    try:
      do_work()
    finally:
      if os.path.exists(expected_filename):
        os.unlink(expected_filename)

if __name__ == '__main__':
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)

