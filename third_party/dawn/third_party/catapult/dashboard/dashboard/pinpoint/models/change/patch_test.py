# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import
from dashboard.services import gerrit_service

from dashboard.pinpoint.models.change import patch
from dashboard.pinpoint.models import errors
from dashboard.pinpoint import test


def Patch(revision='abc123', server='https://codereview.com'):
  return patch.GerritPatch(server, 'repo~branch~id', revision)


_GERRIT_CHANGE_INFO = {
    '_number': 658277,
    'id': 'repo~branch~id',
    'project': 'chromium/src',
    'subject': 'Subject',
    'current_revision': 'current revision',
    'revisions': {
        'current revision': {
            '_number': 5,
            'created': '2018-02-01 23:46:56.000000000',
            'uploader': {
                'email': 'author@example.org'
            },
            'fetch': {
                'http': {
                    'url': 'https://googlesource.com/chromium/src',
                    'ref': 'refs/changes/77/658277/5',
                },
            },
            'commit_with_footers': 'Subject\n\nCommit message.\n'
                                   'Change-Id: I0123456789abcdef',
        },
        'other revision': {
            '_number': 4,
            'created': '2018-02-01 23:46:56.000000000',
            'uploader': {
                'email': 'author@example.org'
            },
            'fetch': {
                'http': {
                    'url': 'https://googlesource.com/chromium/src',
                    'ref': 'refs/changes/77/658277/4',
                },
            },
        },
        'yet another revision': {
            '_number': 3,
            'created': '2018-02-01 23:46:56.000000000',
            'uploader': {
                'email': 'author@example.org'
            },
            'fetch': {
                'http': {
                    'url': 'https://googlesource.com/chromium/src',
                    'ref': 'refs/changes/77/658277/3',
                },
            },
            'commit_with_footers': 'Subject\n\nCommit message.\n'
                                   'Change-Id: I0123456789abcdef\n',
        },
    },
}


class GerritPatchTest(test.TestCase):

  def setUp(self):
    super().setUp()
    self.get_change.return_value = _GERRIT_CHANGE_INFO

  def testPatch(self):
    p = patch.GerritPatch('https://example.com', 'abcdef', '2f0d5c7')

    other_patch = patch.GerritPatch(u'https://example.com', 'abcdef', '2f0d5c7')
    self.assertEqual(p, other_patch)
    self.assertEqual(str(p), '2f0d5c7')
    self.assertEqual(p.id_string, 'https://example.com/abcdef/2f0d5c7')

  def testPatch_WithNewlines(self):
    p = patch.GerritPatch('https://example.com', 'abcdef\n', '2f0d5c7\n')
    self.assertEqual(str(p), '2f0d5c7')
    self.assertEqual(p.id_string, 'https://example.com/abcdef/2f0d5c7')

  def testBuildParameters(self):
    p = Patch('current revision')
    expected = {
        'patch_gerrit_url': 'https://codereview.com',
        'patch_issue': 658277,
        'patch_project': 'chromium/src',
        'patch_ref': 'refs/changes/77/658277/5',
        'patch_repository_url': 'https://googlesource.com/chromium/src',
        'patch_set': 5,
        'patch_storage': 'gerrit',
    }
    self.assertEqual(p.BuildParameters(), expected)

  def testAsDict(self):
    p = Patch('current revision')
    expected = {
        'server': 'https://codereview.com',
        'change': 'repo~branch~id',
        'revision': 'current revision',
        'url': 'https://codereview.com/c/chromium/src/+/658277/5',
        'author': 'author@example.org',
        'created': '2018-02-01T23:46:56',
        'subject': 'Subject',
        'message': 'Subject\n\nCommit message.\nChange-Id: I0123456789abcdef',
    }
    self.assertEqual(p.AsDict(), expected)

  def testAsDict_MissingPatch(self):
    p = Patch('unknown revision')
    self.get_change.side_effect = gerrit_service.NotFoundError(
        msg='Change ID not found', headers=None, content=None)
    self.assertEqual(
        p.AsDict(), {
            'server': 'https://codereview.com',
            'change': 'repo~branch~id',
            'revision': 'unknown revision',
        })

  def testAsDict_WithNewlines(self):
    p = Patch(revision='current revision\n')
    expected = {
        'server': 'https://codereview.com',
        'change': 'repo~branch~id',
        'revision': 'current revision',
        'url': 'https://codereview.com/c/chromium/src/+/658277/5',
        'author': 'author@example.org',
        'created': '2018-02-01T23:46:56',
        'subject': 'Subject',
        'message': 'Subject\n\nCommit message.\nChange-Id: I0123456789abcdef',
    }
    self.assertEqual(p.AsDict(), expected)

  def testFromDataUrl(self):
    p = patch.GerritPatch.FromData('https://codereview.com/c/repo/+/658277')
    self.assertEqual(p, Patch('current revision'))

  def testFromDataDict(self):
    p = patch.GerritPatch.FromData({
        'server': 'https://codereview.com',
        'change': 658277,
        'revision': 4,
    })
    self.assertEqual(p, Patch('other revision'))

  def testFromUrl(self):
    p = patch.GerritPatch.FromUrl('https://codereview.com/c/repo/+/658277/4')
    self.assertEqual(p, Patch('other revision'))

  def testFromUrlAlternateFormat(self):
    p = patch.GerritPatch.FromUrl('https://codereview.com/c/658277')
    self.assertEqual(p, Patch('current revision'))

  def testFromUrlAlternateFormatWithRevision(self):
    p = patch.GerritPatch.FromUrl('https://codereview.com/c/658277/3')
    self.assertEqual(p, Patch('yet another revision'))

  def testFromGitClIssueUrl(self):
    p = patch.GerritPatch.FromUrl('https://codereview.com/658277/')
    self.assertEqual(p, Patch('current revision'))

  def testFromUrlWithHash(self):
    p = patch.GerritPatch.FromUrl('https://codereview.com/#/c/repo/+/658277')
    self.assertEqual(p, Patch('current revision'))

  def testFromUrlBadUrl(self):
    with self.assertRaises(errors.BuildGerritURLInvalid):
      patch.GerritPatch.FromUrl('https://example.com/not/a/gerrit/url')

  def testFromDict(self):
    p = patch.GerritPatch.FromDict({
        'server': 'https://codereview.com',
        'change': 658277,
        'revision': 4,
    })
    self.assertEqual(p, Patch('other revision'))
