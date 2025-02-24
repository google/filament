# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.pinpoint.models.quest import run_browser_test
from dashboard.pinpoint.models.quest import run_test_test

_BASE_ARGUMENTS = {
    'swarming_server': 'server',
    'dimensions': run_test_test.DIMENSIONS,
    'target': 'some_browser_test',
}

_BASE_EXTRA_ARGS = run_browser_test._DEFAULT_EXTRA_ARGS

_BASE_SWARMING_TAGS = {}


class FromDictTest(unittest.TestCase):

  def testMinimumArguments(self):
    quest = run_browser_test.RunBrowserTest.FromDict(_BASE_ARGUMENTS)
    expected = run_browser_test.RunBrowserTest(
        'server', run_test_test.DIMENSIONS, _BASE_EXTRA_ARGS,
        _BASE_SWARMING_TAGS,
        ['luci-auth', 'context', '--', 'some_browser_test'], 'out/Release')
    self.assertEqual(quest, expected)

  def testAllArguments(self):
    arguments = dict(_BASE_ARGUMENTS)
    filter_string = 'BrowserTestClass*:SomeOtherBrowserTestClass*'
    arguments['test-filter'] = filter_string
    quest = run_browser_test.RunBrowserTest.FromDict(arguments)

    extra_args = ['--gtest_filter=%s' % filter_string] + _BASE_EXTRA_ARGS
    expected = run_browser_test.RunBrowserTest(
        'server', run_test_test.DIMENSIONS, extra_args, _BASE_SWARMING_TAGS,
        ['luci-auth', 'context', '--', 'some_browser_test'], 'out/Release')
    self.assertEqual(quest, expected)
