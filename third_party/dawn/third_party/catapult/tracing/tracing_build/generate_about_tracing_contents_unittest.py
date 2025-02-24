# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest
import shutil
import tempfile

from tracing_build import generate_about_tracing_contents

class GenerateAboutTracingContentsUnittTest(unittest.TestCase):

  def testSmoke(self):
    try:
      tmpdir = tempfile.mkdtemp()
      res = generate_about_tracing_contents.Main(['--outdir', tmpdir])
      assert res == 0
    finally:
      shutil.rmtree(tmpdir)
