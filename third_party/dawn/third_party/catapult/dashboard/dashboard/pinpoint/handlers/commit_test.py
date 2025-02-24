# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
from unittest import mock

from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint import test


class CommitTest(test.TestCase):

  @mock.patch.object(commit.Commit, 'FromDict')
  def testPost(self, mock_dict):
    mock_dict.return_value = mock.MagicMock()
    mock_dict.return_value.AsDict = mock.MagicMock(return_value=['abc'])

    args = '?git_hash=abc'
    data = json.loads(self.testapp.post('/api/commit' + args).body)

    self.assertEqual(mock_dict.call_args,
                     mock.call({
                         'repository': 'chromium',
                         'git_hash': 'abc'
                     }))
    self.assertEqual(['abc'], data)

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.FromDict',
              mock.MagicMock(side_effect=KeyError('foo')))
  def testPost_Fail(self):
    self.testapp.post('/api/commit', status=400)
