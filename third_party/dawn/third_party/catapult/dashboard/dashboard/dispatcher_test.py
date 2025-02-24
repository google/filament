# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import os
import unittest


class DispatcherTest(unittest.TestCase):

  def testImport(self):
    # load_from_prod requires this:
    os.environ['APPLICATION_ID'] = 'testbed-test'

    # pylint: disable=import-outside-toplevel
    from dashboard import dispatcher  # pylint: disable=unused-import


if __name__ == '__main__':
  unittest.main()
