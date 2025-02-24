# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard.common import testing_common
from dashboard.models import graph_data


class GraphDataTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetIsInternalUser('x@google.com', True)

  def testPutTestTruncatesDescription(self):
    master = graph_data.Master(id='M').put()
    graph_data.Bot(parent=master, id='b').put()
    long_string = 500 * 'x'
    too_long = long_string + 'y'
    t = graph_data.TestMetadata(id='M/b/a', description=too_long)
    t.UpdateSheriff()
    key = t.put()
    self.assertEqual(long_string, key.get().description)

  def testBotInternalOnly(self):
    master = graph_data.Master(id='M').put()
    graph_data.Bot(parent=master, id='B1').put()
    graph_data.Bot(parent=master, id='B2', internal_only=True).put()

    # Test default value internal_only is False
    internal_only = graph_data.Bot.GetInternalOnlySync('M', 'B1')
    self.assertFalse(internal_only)

    # Test bot with internal_only True returns True.
    self.SetCurrentUser('x@google.com')
    internal_only = graph_data.Bot.GetInternalOnlySync('M', 'B2')
    self.assertTrue(internal_only)

    # Test default internal_only value for non-existing Bot is True
    internal_only = graph_data.Bot.GetInternalOnlySync('M1', 'B1')
    self.assertTrue(internal_only)


if __name__ == '__main__':
  unittest.main()
