# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_gtest
from dashboard.pinpoint.models.quest import run_performance_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'benchmark': 'some_benchmark',
    'target': 'foo_test',
}
_BASE_EXTRA_ARGS = [
    '--gtest_repeat=1', '--gtest-benchmark-name', 'some_benchmark'
] + run_gtest._DEFAULT_EXTRA_ARGS + run_performance_test._DEFAULT_EXTRA_ARGS
_GTEST_COMMAND = ['luci-auth', 'context', '--', 'foo_test']
_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_gtest.RunGTest.FromDict(_BASE_ARGUMENTS)
    expected = run_gtest.RunGTest('server', run_test_test.DIMENSIONS,
                                  _BASE_EXTRA_ARGS, _BASE_SWARMING_TAGS,
                                  _GTEST_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)

  def testAllArguments(self):
    arguments = dict(_BASE_ARGUMENTS)
    arguments['test'] = 'test_name'
    quest = run_gtest.RunGTest.FromDict(arguments)

    extra_args = ['--gtest_filter=test_name'] + _BASE_EXTRA_ARGS
    expected = run_gtest.RunGTest('server', run_test_test.DIMENSIONS,
                                  extra_args, _BASE_SWARMING_TAGS,
                                  _GTEST_COMMAND, 'out/Release')
    self.assertEqual(quest, expected)
