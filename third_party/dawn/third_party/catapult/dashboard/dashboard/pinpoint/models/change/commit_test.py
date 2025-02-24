# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import re

from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint import test


def Commit(number, repository='chromium'):
  return commit.Commit(repository=repository, git_hash='commit_' + str(number))


class CommitTest(test.TestCase):

  def testCommit(self):
    c = commit.Commit('chromium', 'aaa7336c821888839f759c6c0a36')

    other_commit = commit.Commit(u'chromium', u'aaa7336c821888839f759c6c0a36')
    self.assertEqual(c, other_commit)
    self.assertEqual(str(c), 'chromium@aaa7336')
    self.assertEqual(c.id_string, 'chromium@aaa7336c821888839f759c6c0a36')
    self.assertEqual(c.repository, 'chromium')
    self.assertEqual(c.git_hash, 'aaa7336c821888839f759c6c0a36')
    self.assertEqual(c.repository_url, test.CHROMIUM_URL)

  def testDeps(self):
    self.file_contents.return_value = """
vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'webrtc_git': 'https://webrtc.googlesource.com',
  'webrtc_rev': 'deadbeef',
}
deps = {
  'src/v8': Var('chromium_git') + '/v8/v8.git' + '@' + 'c092edb',
  'src/third_party/lighttpd': {
    'url': Var('chromium_git') + '/deps/lighttpd.git' + '@' + '9dfa55d',
    'condition': 'checkout_mac or checkout_win',
  },
  'src/third_party/webrtc': {
    'url': '{webrtc_git}/src.git@{webrtc_rev}',
  },
  'src/third_party/intellij': {
    'packages': [{
      'package': 'chromium/third_party/intellij',
      'version': 'version:12.0-cr0',
    }],
    'condition': 'checkout_android',
    'dep_type': 'cipd',
  },
}
deps_os = {
  'win': {
    'src/third_party/cygwin':
      Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446',
  }
}
    """

    expected = frozenset((
        ('https://chromium.googlesource.com/chromium/deps/cygwin', 'c89e446'),
        ('https://chromium.googlesource.com/deps/lighttpd', '9dfa55d'),
        ('https://chromium.googlesource.com/v8/v8', 'c092edb'),
        ('https://webrtc.googlesource.com/src', 'deadbeef'),
    ))
    self.assertEqual(Commit(0).Deps(), expected)

  def testDepsNoVars(self):
    self.file_contents.return_value = """
hooks = [
  {
    'name': 'do_stuff',
    'pattern': '.',
    'action': [
        'python',
        'src/tools/do_stuff.py',
    ],
  }]
    """

    self.assertEqual(Commit(0).Deps(), frozenset([]))

  def testDepsIgnoresInvalid(self):
    # A file that fails gclient_eval.Parse's validation.
    self.file_contents.return_value = """
deps = {
  'some_dep': {'invalid_key': ''}
}
    """
    with self.assertRaises(commit.DepsParsingError):
      Commit(0).Deps()

  def testAsDict(self):
    self.commit_info.side_effect = None
    self.commit_info.return_value = {
        'author': {
            'email': 'author@chromium.org'
        },
        'commit': 'aaa7336',
        'committer': {
            'time': 'Fri Jan 01 00:01:00 2016'
        },
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Reviewed-on: https://foo/c/chromium/src/+/123\n'
                   'Cr-Commit-Position: refs/branch-heads/4242@{#437745}',
    }

    expected = {
        'repository': 'chromium',
        'git_hash': 'commit_0',
        'url': test.CHROMIUM_URL + '/+/aaa7336',
        'author': 'author@chromium.org',
        'created': '2016-01-01T00:01:00',
        'subject': 'Subject.',
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Reviewed-on: https://foo/c/chromium/src/+/123\n'
                   'Cr-Commit-Position: refs/branch-heads/4242@{#437745}',
        'review_url': 'https://foo/c/chromium/src/+/123',
        'commit_position': 437745,
        'commit_branch': 'refs/branch-heads/4242'
    }
    self.assertEqual(Commit(0).AsDict(), expected)

  def testAsDict_GitHashWithNewlines(self):
    self.commit_info.side_effect = None
    self.commit_info.return_value = {
        'author': {
            'email': 'author@chromium.org'
        },
        'commit': 'aaa7336',
        'committer': {
            'time': 'Fri Jan 01 00:01:00 2016'
        },
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Reviewed-on: https://foo/c/chromium/src/+/123\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
    }

    expected = {
        'repository': 'chromium',
        'git_hash': 'commit_0',
        'url': test.CHROMIUM_URL + '/+/aaa7336',
        'author': 'author@chromium.org',
        'created': '2016-01-01T00:01:00',
        'subject': 'Subject.',
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Reviewed-on: https://foo/c/chromium/src/+/123\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
        'review_url': 'https://foo/c/chromium/src/+/123',
        'commit_position': 437745,
        'commit_branch': 'refs/heads/main'
    }
    self.assertEqual(commit.Commit('chromium', 'commit_0\n').AsDict(), expected)

  def testAsDict_UtcOffsetPositive(self):
    self.commit_info.side_effect = None
    self.commit_info.return_value = {
        'author': {
            'email': 'author@chromium.org'
        },
        'commit': 'aaa7336',
        'committer': {
            'time': 'Fri Jan 01 00:01:00 2016 +0000'
        },
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
    }

    expected = {
        'repository': 'chromium',
        'git_hash': 'commit_0',
        'url': test.CHROMIUM_URL + '/+/aaa7336',
        'author': 'author@chromium.org',
        'created': '2016-01-01T00:01:00',
        'subject': 'Subject.',
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
        'commit_position': 437745,
        'commit_branch': 'refs/heads/main',
    }
    self.assertEqual(Commit(0).AsDict(), expected)

  def testAsDict_UtcOffsetNegative(self):
    self.commit_info.side_effect = None
    self.commit_info.return_value = {
        'author': {
            'email': 'author@chromium.org'
        },
        'commit': 'aaa7336',
        'committer': {
            'time': 'Fri Jan 01 00:01:00 2016 -0000'
        },
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
    }

    expected = {
        'repository': 'chromium',
        'git_hash': 'commit_0',
        'url': test.CHROMIUM_URL + '/+/aaa7336',
        'author': 'author@chromium.org',
        'created': '2016-01-01T00:01:00',
        'subject': 'Subject.',
        'message': 'Subject.\n\n'
                   'Commit message.\n'
                   'Cr-Commit-Position: refs/heads/main@{#437745}',
        'commit_position': 437745,
        'commit_branch': 'refs/heads/main'
    }
    self.assertEqual(Commit(0).AsDict(), expected)

  def testFromDepNewRepo(self):
    dep = commit.Dep('https://new/repository/url.git', 'git_hash')
    c = commit.Commit.FromDep(dep)
    self.assertEqual(c, commit.Commit('url', 'git_hash'))

  def testFromDepExistingRepo(self):
    c = commit.Commit.FromDep(commit.Dep(test.CHROMIUM_URL, 'commit_0'))
    self.assertEqual(c, Commit(0))

  def testFromDataUrl(self):
    c = commit.Commit.FromData(test.CHROMIUM_URL + '/+/commit_0')
    self.assertEqual(c, Commit(0))

  def testFromDataDict(self):
    c = commit.Commit.FromData({
        'repository': 'chromium',
        'git_hash': 'commit_0',
    })
    self.assertEqual(c, Commit(0))

  def testFromUrl(self):
    c = commit.Commit.FromUrl(test.CHROMIUM_URL + '/+/commit_0')
    self.assertEqual(c, Commit(0))

  def testFromUrlBadUrl(self):
    with self.assertRaises(ValueError):
      commit.Commit.FromUrl('https://example.com/not/a/gitiles/url')

  def testFromDict(self):
    c = commit.Commit.FromDict({
        'repository': 'chromium',
        'git_hash': 'commit_0',
    })
    self.assertEqual(c, Commit(0))

  def testFromDictWithRepositoryUrl(self):
    c = commit.Commit.FromDict({
        'repository': test.CHROMIUM_URL,
        'git_hash': 'commit_0',
    })
    self.assertEqual(c, Commit(0))

  def testFromDictWithNewlines(self):
    c = commit.Commit.FromDict({
        'repository': 'chromium',
        'git_hash': 'commit_0\n',
    })
    self.assertEqual(c, Commit(0))

  def testFromDictResolvesHEAD(self):
    c = commit.Commit.FromDict({
        'repository': test.CHROMIUM_URL,
        'git_hash': 'HEAD',
    })

    expected = commit.Commit('chromium', 'git hash at HEAD')
    self.assertEqual(c, expected)

  @mock.patch.object(commit.commit_cache, 'Put')
  def testFromDict_SkipsCache(self, mock_put):
    git_hash = 'ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE'
    c = commit.Commit.FromDict({
        'repository': test.CHROMIUM_URL,
        'git_hash': git_hash,
    })
    e = commit.Commit(repository='chromium', git_hash=git_hash)
    self.assertEqual(c, e)
    self.assertFalse(mock_put.called)

  @mock.patch.object(commit.commit_cache, 'Put')
  def testFromDict_Caches(self, mock_put):
    c = commit.Commit.FromDict({
        'repository': test.CHROMIUM_URL,
        'git_hash': 'HEAD',
    })

    expected = commit.Commit('chromium', 'git hash at HEAD')
    self.assertEqual(c, expected)

    mock_put.assert_called_once_with(
        'chromium@HEAD',
        mock.ANY,
        mock.ANY,
        mock.ANY,
        mock.ANY,
        mock.ANY,
        memcache_timeout=60 * 60 * 10)

  def testFromDictFailureFromUnknownRepo(self):
    with self.assertRaises(KeyError):
      commit.Commit.FromDict({
          'repository': 'unknown repo',
          'git_hash': 'git hash',
      })

  def testFromDictFailureFromUnknownCommit(self):
    self.commit_info.side_effect = KeyError()

    with self.assertRaises(KeyError):
      commit.Commit.FromDict({
          'repository': 'chromium',
          'git_hash': 'unknown git hash',
      })


class MidpointTest(test.TestCase):

  def testSuccess(self):
    midpoint = commit.Commit.Midpoint(Commit(1), Commit(9))
    self.assertEqual(midpoint, Commit(5))

  def testSameCommit(self):
    midpoint = commit.Commit.Midpoint(Commit(1), Commit(1))
    self.assertEqual(midpoint, Commit(1))

  def testAdjacentCommits(self):
    midpoint = commit.Commit.Midpoint(Commit(1), Commit(2))
    self.assertEqual(midpoint, Commit(1))

  def testRaisesWithDifferingRepositories(self):
    with self.assertRaises(commit.NonLinearError):
      commit.Commit.Midpoint(Commit(1), Commit(2, repository='catapult'))

  def testRaisesWithEmptyRange(self):
    self.commit_range.side_effect = None
    self.commit_range.return_value = []

    with self.assertRaises(commit.NonLinearError):
      commit.Commit.Midpoint(Commit(1), Commit(9))

  def testMidpointWithMergeCommits(self):
    midpoint = commit.Commit.Midpoint(
        commit.Commit(repository='chromium', git_hash='commit_0'),
        commit.Commit(repository='chromium', git_hash='mc_4'))
    self.assertEqual(midpoint,
                     commit.Commit(repository='chromium', git_hash='commit_2'))

  @mock.patch.object(commit.deferred, 'defer')
  def testMidpointCachesData(self, mock_defer):
    c1 = commit.Commit(repository='chromium', git_hash='commit_0')
    c2 = commit.Commit(repository='chromium', git_hash='mc_4')
    midpoint = commit.Commit.Midpoint(c1, c2)

    commits = commit.Commit.CommitRange(c1, c2)
    commits.pop(0)

    mock_defer.assert_called_once_with(commit._CacheCommitDetails,
                                       midpoint.repository, commits)

  def testContinuousMidpointWithMergeCommits(self):

    def _Midpoints(start_point, end_point):
      midpoint = commit.Commit.Midpoint(start_point, end_point)
      while midpoint.git_hash != start_point.git_hash:
        yield midpoint
        start_point = midpoint
        midpoint = commit.Commit.Midpoint(start_point, end_point)

    start = commit.Commit(repository='chromium', git_hash='mc_1')
    end = commit.Commit(repository='chromium', git_hash='mc_100')
    count = 0
    matcher = re.compile(r'^mc_\d+$')
    for midpoint in _Midpoints(start, end):
      self.assertRegex(midpoint.git_hash, matcher)
      count += 1
      self.assertLess(count, 100)

    # Check that the midpoint search algorithm runs in log2(N) time.
    self.assertGreater(count, 0)
    self.assertLess(count, 8)
