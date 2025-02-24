# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import logging
import unittest

try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO

from py_utils import logging_util


class LoggingUtilTest(unittest.TestCase):
  def testCapture(self):
    s = StringIO()
    with logging_util.CaptureLogs(s):
      logging.fatal('test')

    # Only assert ends with, since the logging message by default has the date
    # in it.
    self.assertTrue(s.getvalue().endswith('test\n'))


if __name__ == '__main__':
  unittest.main()
