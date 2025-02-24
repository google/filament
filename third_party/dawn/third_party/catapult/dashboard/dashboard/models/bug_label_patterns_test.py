# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import bug_label_patterns


class BugLabelPatternsTest(testing_common.TestCase):

  def testAddBugLabelPattern(self):
    bug_label_patterns.AddBugLabelPattern('foo', '*/*/foo')
    bug_label_patterns.AddBugLabelPattern('bar', '*/*/bar')
    bug_label_patterns.AddBugLabelPattern('bar', '*/*/bar-extra')
    self.assertEqual(1,
                     len(bug_label_patterns.BugLabelPatterns.query().fetch()))
    self.assertEqual({
        'foo': ['*/*/foo'],
        'bar': ['*/*/bar', '*/*/bar-extra']
    }, bug_label_patterns.GetBugLabelPatterns())

  def testRemoveBugLabel(self):
    bug_label_patterns.AddBugLabelPattern('foo', '*/*/foo')
    bug_label_patterns.AddBugLabelPattern('bar', '*/*/bar')
    bug_label_patterns.AddBugLabelPattern('bar', '*/*/bar-extra')
    bug_label_patterns.RemoveBugLabel('bar')
    self.assertEqual({'foo': ['*/*/foo']},
                     bug_label_patterns.GetBugLabelPatterns())

  def testRemoveBugLabel_DoesntExist_NoError(self):
    bug_label_patterns.RemoveBugLabel('bar')
    self.assertEqual({}, bug_label_patterns.GetBugLabelPatterns())

  def testGetBugLabelsForTest(self):
    bug_label_patterns.AddBugLabelPattern('foo', '*/*/foo')
    bug_label_patterns.AddBugLabelPattern('f-prefix', '*/*/f*')
    testing_common.AddTests(['M'], ['b'], {'foo': {}, 'bar': {}})
    foo_test = utils.TestKey('M/b/foo').get()
    bar_test = utils.TestKey('M/b/bar').get()
    self.assertEqual(['f-prefix', 'foo'],
                     bug_label_patterns.GetBugLabelsForTest(foo_test))
    self.assertEqual([], bug_label_patterns.GetBugLabelsForTest(bar_test))


if __name__ == '__main__':
  unittest.main()
