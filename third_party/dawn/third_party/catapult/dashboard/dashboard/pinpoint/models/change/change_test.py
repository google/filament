# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from collections import namedtuple
from six import BytesIO
import pickle

from dashboard.pinpoint.models.change import change
from dashboard.pinpoint.models.change import commit
from dashboard.pinpoint.models.change import commit_test
from dashboard.pinpoint.models.change import patch_test
from dashboard.pinpoint import test


def Change(chromium=None,
           catapult=None,
           another_repo=None,
           patch=False,
           variant=None):
  commits = []
  if chromium is not None:
    commits.append(commit_test.Commit(chromium))
  if catapult is not None:
    commits.append(commit_test.Commit(catapult, repository='catapult'))
  if another_repo is not None:
    commits.append(commit_test.Commit(another_repo, repository='another_repo'))
  return change.Change(
      commits, patch=patch_test.Patch() if patch else None, variant=variant)


class ChangeTest(test.TestCase):

  def testChange(self):
    base_commit = commit.Commit('chromium', 'aaa7336c821888839f759c6c0a36b56c')
    dep = commit.Commit('catapult', 'e0a2efbb3d1a81aac3c90041eefec24f066d26ba')

    # Also test the deps conversion to tuple.
    c = change.Change([base_commit, dep], patch_test.Patch())

    self.assertEqual(c, change.Change((base_commit, dep), patch_test.Patch()))
    string = 'chromium@aaa7336 catapult@e0a2efb + abc123'
    id_string = ('catapult@e0a2efbb3d1a81aac3c90041eefec24f066d26ba '
                 'chromium@aaa7336c821888839f759c6c0a36b56c + '
                 'https://codereview.com/repo~branch~id/abc123')
    self.assertEqual(str(c), string)
    self.assertEqual(c.id_string, id_string)
    self.assertEqual(c.base_commit, base_commit)
    self.assertEqual(c.last_commit, dep)
    self.assertEqual(c.deps, (dep,))
    self.assertEqual(c.commits, (base_commit, dep))
    self.assertEqual(c.patch, patch_test.Patch())

  def testChangeWithVariant(self):
    base_commit = commit.Commit('chromium', 'aaa7336c821888839f759c6c0a36b56c')
    dep = commit.Commit('catapult', 'e0a2efbb3d1a81aac3c90041eefec24f066d26ba')

    # Also test the deps conversion to tuple.
    c = change.Change([base_commit, dep], patch_test.Patch(), variant=1)

    self.assertEqual(
        c, change.Change((base_commit, dep), patch_test.Patch(), variant=1))
    string = 'chromium@aaa7336 catapult@e0a2efb + abc123 (Variant: 1)'
    id_string = ('catapult@e0a2efbb3d1a81aac3c90041eefec24f066d26ba '
                 'chromium@aaa7336c821888839f759c6c0a36b56c + '
                 'https://codereview.com/repo~branch~id/abc123')
    self.assertEqual(str(c), string)
    self.assertEqual(c.id_string, id_string)
    self.assertEqual(c.base_commit, base_commit)
    self.assertEqual(c.last_commit, dep)
    self.assertEqual(c.deps, (dep,))
    self.assertEqual(c.commits, (base_commit, dep))
    self.assertEqual(c.patch, patch_test.Patch())
    self.assertEqual(c.variant, 1)

  def testUpdate(self):
    old_commit = commit.Commit('chromium', 'aaaaaaaa')
    dep_a = commit.Commit('catapult', 'e0a2efbb')
    change_a = change.Change((old_commit, dep_a))

    new_commit = commit.Commit('chromium', 'bbbbbbbb')
    dep_b = commit.Commit('another_repo', 'e0a2efbb')
    change_b = change.Change((dep_b, new_commit), patch_test.Patch())

    expected = change.Change((new_commit, dep_a, dep_b), patch_test.Patch())
    self.assertEqual(change_a.Update(change_b), expected)

  def testUpdateWithMultiplePatches(self):
    c = Change(chromium=123, patch=True)
    with self.assertRaises(NotImplementedError):
      c.Update(c)

  def testAsDict(self):
    c = Change(chromium=123, catapult=456, patch=True)

    expected = {
        'commits': [
            {
                'author':
                    'author@chromium.org',
                'commit_branch':
                    'refs/heads/master',
                'commit_position':
                    123456,
                'git_hash':
                    'commit_123',
                'repository':
                    'chromium',
                'created':
                    '2018-01-01T00:01:00',
                'url':
                    u'https://chromium.googlesource.com/chromium/src/+/commit_123',
                'subject':
                    'Subject.',
                'review_url':
                    'https://foo.bar/+/123456',
                'change_id':
                    'If32lalatdfg325simon8943washere98j589',
                'message':
                    'Subject.\n\nCommit message.\n'
                    'Reviewed-on: https://foo.bar/+/123456\n'
                    'Change-Id: If32lalatdfg325simon8943washere98j589\n'
                    'Cr-Commit-Position: refs/heads/master@{#123456}',
            },
            {
                'author':
                    'author@chromium.org',
                'commit_branch':
                    'refs/heads/master',
                'commit_position':
                    123456,
                'git_hash':
                    'commit_456',
                'repository':
                    'catapult',
                'created':
                    '2018-01-01T00:01:00',
                'url':
                    u'https://chromium.googlesource.com/catapult/+/commit_456',
                'subject':
                    'Subject.',
                'review_url':
                    'https://foo.bar/+/123456',
                'change_id':
                    'If32lalatdfg325simon8943washere98j589',
                'message':
                    'Subject.\n\nCommit message.\n'
                    'Reviewed-on: https://foo.bar/+/123456\n'
                    'Change-Id: If32lalatdfg325simon8943washere98j589\n'
                    'Cr-Commit-Position: refs/heads/master@{#123456}',
            },
        ],
        'patch': {
            'author': 'author@codereview.com',
            'change': 'repo~branch~id',
            'revision': 'abc123',
            'server': 'https://codereview.com',
            'created': '2018-02-01T23:46:56',
            'url': 'https://codereview.com/c/project/name/+/567890/5',
            'subject': 'Patch subject.',
            'message': 'Subject\n\nCommit message.\n'
                       'Change-Id: I0123456789abcdef',
        },
    }
    self.maxDiff = None
    self.assertEqual(c.AsDict(), expected)

  def testAsDictWithVariant(self):
    c = Change(chromium=123, patch=False, variant=0)

    expected = {
        'variant':
            0,
        'commits': [{
            'author':
                'author@chromium.org',
            'commit_branch':
                'refs/heads/master',
            'commit_position':
                123456,
            'git_hash':
                'commit_123',
            'repository':
                'chromium',
            'created':
                '2018-01-01T00:01:00',
            'url':
                u'https://chromium.googlesource.com/chromium/src/+/commit_123',
            'subject':
                'Subject.',
            'review_url':
                'https://foo.bar/+/123456',
            'change_id':
                'If32lalatdfg325simon8943washere98j589',
            'message':
                'Subject.\n\nCommit message.\n'
                'Reviewed-on: https://foo.bar/+/123456\n'
                'Change-Id: If32lalatdfg325simon8943washere98j589\n'
                'Cr-Commit-Position: refs/heads/master@{#123456}',
        },],
    }
    self.maxDiff = None
    self.assertEqual(c.AsDict(), expected)

  def testFromDataUrl(self):
    c = change.Change.FromData(test.CHROMIUM_URL + '/+/commit_0')
    self.assertEqual(c, Change(0))

  def testFromDataDict(self):
    c = change.Change.FromData({
        'commits': [{
            'repository': 'chromium',
            'git_hash': 'commit_123'
        }],
    })
    self.assertEqual(c, Change(chromium=123))

  def testFromUrlCommit(self):
    c = change.Change.FromUrl(test.CHROMIUM_URL + '/+/commit_0')
    self.assertEqual(c, Change(0))

  def testFromUrlPatch(self):
    c = change.Change.FromUrl('https://codereview.com/c/repo/+/658277')
    self.assertEqual(c, Change(patch=True))

  def testFromDictWithJustOneCommit(self):
    c = change.Change.FromDict({
        'commits': [{
            'repository': 'chromium',
            'git_hash': 'commit_123'
        }],
    })
    self.assertEqual(c, Change(chromium=123))

  def testFromDictWithAllFields(self):
    self.get_change.return_value = {
        'id': 'repo~branch~id',
        'revisions': {
            'abc123': {}
        }
    }

    c = change.Change.FromDict({
        'variant': 1,
        'commits': (
            {
                'repository': 'chromium',
                'git_hash': 'commit_123'
            },
            {
                'repository': 'catapult',
                'git_hash': 'commit_456'
            },
        ),
        'patch': {
            'server': 'https://codereview.com',
            'change': 'repo~branch~id',
            'revision': 'abc123',
        },
    })

    expected = Change(chromium=123, catapult=456, patch=True, variant=1)
    self.assertEqual(c, expected)

  def testFromDictWithNonePatch(self):
    self.get_change.return_value = {
        'id': 'repo~branch~id',
        'revisions': {
            'abc123': {}
        }
    }

    c = change.Change.FromDict({
        'commits': (
            {
                'repository': 'chromium',
                'git_hash': 'commit_123'
            },
            {
                'repository': 'catapult',
                'git_hash': 'commit_456'
            },
        ),
        'patch': None,
    })

    expected = Change(chromium=123, catapult=456, patch=False)
    self.assertEqual(c, expected)


OldChange = namedtuple('OldChange', ('commits', 'patch'))


class CustomUnpickler(pickle.Unpickler):

  def find_class(self, module, name):
    # If we see the Change types we're looking for, we should make it load the
    # actual Change name.
    if name == 'OldChange':
      return change.Change
    return pickle.Unpickler.find_class(self, module, name)


class PickleTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None

  def testBackwardsCompatibility(self):
    old_commit = commit.Commit('chromium', 'aaaaaaaa')
    old = OldChange([old_commit], None)
    string_io = BytesIO()
    p = pickle.Pickler(string_io)
    p.dump(old)

    # Now attempt to unpickle into the current definition, and ensure it works
    # just fine.
    pickled_data = string_io.getvalue()
    new = CustomUnpickler(BytesIO(pickled_data)).load()
    self.assertEqual(old.patch, new.patch)
    self.assertEqual(len(old.commits), len(new.commits))
    self.assertIsNone(new.change_label)
    self.assertIsNone(new.change_args)
    self.assertEqual(type(new), change.Change)


class MidpointTest(test.TestCase):

  def setUp(self):
    super().setUp()

    def _FileContents(repository_url, git_hash, path):
      del path
      if repository_url != test.CHROMIUM_URL:
        return 'deps = {}'
      if int(git_hash.split('_')[1]) <= 4:  # DEPS roll at chromium@5
        return 'deps = {"chromium/catapult": "%s@commit_0"}' % (
            test.CATAPULT_URL + '.git')
      return 'deps = {"chromium/catapult": "%s@commit_9"}' % test.CATAPULT_URL

    self.file_contents.side_effect = _FileContents

  def testDifferingPatch(self):
    with self.assertRaises(commit.NonLinearError):
      change.Change.Midpoint(Change(0), Change(2, patch=True))

  def testDifferingRepository(self):
    with self.assertRaises(commit.NonLinearError):
      change.Change.Midpoint(Change(0), Change(another_repo=123))

  def testDifferingCommitCount(self):
    with self.assertRaises(commit.NonLinearError):
      change.Change.Midpoint(Change(0), Change(9, another_repo=123))

  def testSameChange(self):
    with self.assertRaises(commit.NonLinearError):
      change.Change.Midpoint(Change(0), Change(0))

  def testAdjacentWithNoDepsRoll(self):
    with self.assertRaises(commit.NonLinearError):
      change.Change.Midpoint(Change(0), Change(1))

  def testAdjacentWithDepsRoll(self):
    midpoint = change.Change.Midpoint(Change(4), Change(5))
    self.assertEqual(midpoint, Change(4, catapult=4))

  def testNotAdjacent(self):
    midpoint = change.Change.Midpoint(Change(0), Change(9))
    self.assertEqual(midpoint, Change(4))

  def testDepsRollLeft(self):
    midpoint = change.Change.Midpoint(Change(4), Change(4, catapult=4))
    self.assertEqual(midpoint, Change(4, catapult=2))

  def testDepsRollRight(self):
    midpoint = change.Change.Midpoint(Change(4, catapult=4), Change(5))
    self.assertEqual(midpoint, Change(4, catapult=6))

  def testAdjacentWithDepsRollAndDepAlreadyOverridden(self):
    midpoint = change.Change.Midpoint(Change(4), Change(5, catapult=4))
    self.assertEqual(midpoint, Change(4, catapult=2))
