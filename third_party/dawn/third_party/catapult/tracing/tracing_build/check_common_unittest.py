# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing_build import check_common


class CheckCommonUnitTest(unittest.TestCase):

  def testFilesSorted(self):
    error = check_common.CheckListedFilesSorted('foo.gyp', 'tracing_pdf_files',
                                                ['/dir/file.pdf',
                                                 '/dir/another_file.pdf'])
    expected_error = '''In group tracing_pdf_files from file foo.gyp,\
 filenames aren't sorted.

First mismatch:
  /dir/file.pdf

Current listing:
  /dir/file.pdf
  /dir/another_file.pdf

Correct listing:
  /dir/another_file.pdf
  /dir/file.pdf\n\n'''
    assert error == expected_error
