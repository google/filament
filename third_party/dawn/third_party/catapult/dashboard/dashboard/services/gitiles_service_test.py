# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.services import gerrit_service
from dashboard.services import gitiles_service


class GitilesTest(unittest.TestCase):

  def setUp(self):
    patcher = mock.patch('dashboard.services.request.RequestJson')
    self._request_json = patcher.start()
    self.addCleanup(patcher.stop)

    patcher = mock.patch('dashboard.services.request.Request')
    self._request = patcher.start()
    self.addCleanup(patcher.stop)

  def testCommitInfo(self):
    return_value = {
        'commit':
            'commit_hash',
        'tree':
            'tree_hash',
        'parents': ['parent_hash'],
        'author': {
            'name': 'username',
            'email': 'email@chromium.org',
            'time': 'Fri Jan 01 00:00:00 2016',
        },
        'committer': {
            'name': 'Commit bot',
            'email': 'commit-bot@chromium.org',
            'time': 'Fri Jan 01 00:01:00 2016',
        },
        'message':
            'Subject.\n\nCommit message.',
        'tree_diff': [{
            'type': 'modify',
            'old_id': 'old_hash',
            'old_mode': 33188,
            'old_path': 'a/b/c.py',
            'new_id': 'new_hash',
            'new_mode': 33188,
            'new_path': 'a/b/c.py',
        },],
    }
    self._request_json.return_value = return_value
    self.assertEqual(
        gitiles_service.CommitInfo('https://chromium.googlesource.com/repo',
                                   'commit_hash'), return_value)
    self._request_json.assert_called_once_with(
        'https://chromium.googlesource.com/repo/+/commit_hash?format=JSON',
        use_cache=False,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)

  def testCommitRange(self):
    return_value = {
        'log': [
            {
                'commit': 'commit_2_hash',
                'tree': 'tree_2_hash',
                'parents': ['parent_2_hash'],
                'author': {
                    'name': 'username',
                    'email': 'email@chromium.org',
                    'time': 'Sat Jan 02 00:00:00 2016',
                },
                'committer': {
                    'name': 'Commit bot',
                    'email': 'commit-bot@chromium.org',
                    'time': 'Sat Jan 02 00:01:00 2016',
                },
                'message': 'Subject.\n\nCommit message.',
            },
            {
                'commit': 'commit_1_hash',
                'tree': 'tree_1_hash',
                'parents': ['parent_1_hash'],
                'author': {
                    'name': 'username',
                    'email': 'email@chromium.org',
                    'time': 'Fri Jan 01 00:00:00 2016',
                },
                'committer': {
                    'name': 'Commit bot',
                    'email': 'commit-bot@chromium.org',
                    'time': 'Fri Jan 01 00:01:00 2016',
                },
                'message': 'Subject.\n\nCommit message.',
            },
        ],
    }
    self._request_json.return_value = return_value
    self.assertEqual(
        gitiles_service.CommitRange('https://chromium.googlesource.com/repo',
                                    'commit_0_hash', 'commit_2_hash'),
        return_value['log'])
    self._request_json.assert_called_once_with(
        'https://chromium.googlesource.com/repo/+log/'
        'commit_0_hash..commit_2_hash?format=JSON',
        use_cache=False,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)

  def testCommitRangePaginated(self):
    return_value_1 = {
        'log': [
            {
                'commit': 'commit_4_hash'
            },
            {
                'commit': 'commit_3_hash'
            },
        ],
        'next': 'commit_2_hash',
    }
    return_value_2 = {
        'log': [
            {
                'commit': 'commit_2_hash'
            },
            {
                'commit': 'commit_1_hash'
            },
        ],
    }

    self._request_json.side_effect = return_value_1, return_value_2

    self.assertEqual(
        gitiles_service.CommitRange('https://chromium.googlesource.com/repo',
                                    'commit_0_hash', 'commit_4_hash'),
        return_value_1['log'] + return_value_2['log'])

  def testFileContents(self):
    self._request.return_value = 'aGVsbG8='
    self.assertEqual(
        gitiles_service.FileContents('https://chromium.googlesource.com/repo',
                                     'commit_hash', 'path'), 'hello')
    self._request.assert_called_once_with(
        'https://chromium.googlesource.com/repo/+/commit_hash/path?format=TEXT',
        use_cache=False,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)

  def testCache(self):
    self._request_json.return_value = {'log': []}
    self._request.return_value = 'aGVsbG8='

    repository = 'https://chromium.googlesource.com/repo'
    git_hash = '3a44bc56c4efa42a900a1c22b001559b81e457e9'

    gitiles_service.CommitInfo(repository, git_hash)
    self._request_json.assert_called_with(
        '%s/+/%s?format=JSON' % (repository, git_hash),
        use_cache=True,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)

    gitiles_service.CommitRange(repository, git_hash, git_hash)
    self._request_json.assert_called_with(
        '%s/+log/%s..%s?format=JSON' % (repository, git_hash, git_hash),
        use_cache=True,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)

    gitiles_service.FileContents(repository, git_hash, 'path')
    self._request.assert_called_with(
        '%s/+/%s/path?format=TEXT' % (repository, git_hash),
        use_cache=True,
        use_auth=True,
        scope=gerrit_service.GERRIT_SCOPE)
