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
from dashboard.services import request


class CommitsHandlerTest(test.TestCase):

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.AsDict',
              mock.MagicMock(side_effect=[{
                  'git_hash': 'bar'
              }, {
                  'git_hash': 'foo'
              }]))
  @mock.patch(
      'dashboard.pinpoint.models.change.commit.Commit.FromDict',
      return_value=commit.Commit('chromium', 'xxx'))
  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.CommitRange',
              mock.MagicMock(return_value=[{
                  'commit': 'bar'
              }]))
  def testPost(self, mock_from_dict):

    args = '?start_git_hash=foo&end_git_hash=bar'
    data = json.loads(self.testapp.post('/api/commits' + args).body)

    self.assertEqual(mock_from_dict.call_args_list, [
        mock.call({
            'repository': 'chromium',
            'git_hash': 'foo'
        }),
        mock.call({
            'repository': 'chromium',
            'git_hash': 'bar'
        })
    ])
    self.assertEqual(2, len(data))
    self.assertEqual({'git_hash': 'foo'}, data[0])
    self.assertEqual({'git_hash': 'bar'}, data[1])
    self.assertEqual('bar', data[1]['git_hash'])

  @mock.patch('dashboard.pinpoint.models.change.commit.Commit.FromDict',
              mock.MagicMock(side_effect=request.RequestError('foo', '', '')))
  def testPost_Fail(self):
    self.testapp.post('/api/commits', status=400)
