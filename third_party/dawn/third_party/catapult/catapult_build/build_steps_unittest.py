# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import unittest

from catapult_build import build_steps


class BuildStepsTest(unittest.TestCase):

  def testCatapultTestList(self):
    catapult_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    for test in build_steps._CATAPULT_TESTS:
      self.assertIn('name', test, msg=(
          'All tests in build_steps._CATAPULT_TESTS must have a name;'
          ' error in:\n %s' % test))
      self.assertIsInstance(test['name'], str, msg=(
          'Test name %s in build_steps._CATAPULT_TESTS must be a string.'
          % test['name']))
      self.assertIn('path', test, msg=(
          'All tests in build_steps._CATAPULT_TESTS must have a path '
          'relative to catapult/; error in:\n %s' % test))
      abs_path = os.path.join(catapult_dir, test['path'])
      self.assertTrue(os.path.exists(abs_path), msg=(
          'Bad path %s in build_steps._CATAPULT_TESTS; '
          ' should be relative to catapult/' % test['path']))
      if test.get('additional_args'):
        self.assertIsInstance(test['additional_args'], list, msg=(
            'additional_args %s in build_steps._CATAPULT_TESTS %s not a list'
            % (test['additional_args'], test['name'])
        ))
      if test.get('disabled'):
        self.assertIsInstance(test['disabled'], list, msg=(
            'disabled %s in build_steps._CATAPULT_TESTS for %s not a list'
            % (test['disabled'], test['name'])
        ))
        for platform in test['disabled']:
          self.assertIn(platform, ['win', 'mac', 'linux', 'android'], msg=(
              'Bad platform %s in build_steps._CATAPULT_TESTS for %s;'
              'should be one of "linux", "win", "mac"' % (
                  platform, test['name'])
          ))
