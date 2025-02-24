# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
}

_BASE_EXTRA_ARGS = run_performance_test._DEFAULT_EXTRA_ARGS

_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_performance_test.RunPerformanceTest.FromDict(_BASE_ARGUMENTS)
    expected = run_performance_test.RunPerformanceTest('server',
                                                       run_test_test.DIMENSIONS,
                                                       _BASE_EXTRA_ARGS,
                                                       _BASE_SWARMING_TAGS,
                                                       None, None)
    self.assertEqual(quest, expected)
