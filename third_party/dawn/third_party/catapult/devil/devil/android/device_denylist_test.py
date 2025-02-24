#! /usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import tempfile
import unittest

from devil.android import device_denylist


class DeviceDenylistTest(unittest.TestCase):
  def testDenylistFileDoesNotExist(self):
    with tempfile.NamedTemporaryFile() as denylist_file:
      # Allow the temporary file to be deleted.
      pass

    test_denylist = device_denylist.Denylist(denylist_file.name)
    self.assertEqual({}, test_denylist.Read())

  def testDenylistFileIsEmpty(self):
    try:
      with tempfile.NamedTemporaryFile(delete=False) as denylist_file:
        # Allow the temporary file to be closed.
        pass

      test_denylist = device_denylist.Denylist(denylist_file.name)
      self.assertEqual({}, test_denylist.Read())

    finally:
      if os.path.exists(denylist_file.name):
        os.remove(denylist_file.name)


if __name__ == '__main__':
  unittest.main()
