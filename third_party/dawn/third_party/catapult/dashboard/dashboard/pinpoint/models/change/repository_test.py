# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models.change import repository
from dashboard.pinpoint import test


class RepositoryTest(test.TestCase):

  def testRepositoryUrl(self):
    self.assertEqual(repository.RepositoryUrl('chromium'), test.CHROMIUM_URL)

  def testRepositoryUrlRaisesWithUnknownName(self):
    with self.assertRaises(KeyError):
      repository.RepositoryUrl('not chromium')

  def testRepository(self):
    name = repository.RepositoryName(test.CHROMIUM_URL + '.git')
    self.assertEqual(name, 'chromium')

  def testRepositoryRaisesWithUnknownUrl(self):
    with self.assertRaises(KeyError):
      repository.RepositoryName('https://googlesource.com/nonexistent/repo')

  def testAddRepository(self):
    name = repository.RepositoryName(
        'https://example/repo', add_if_missing=True)
    self.assertEqual(name, 'repo')

    self.assertEqual(repository.RepositoryUrl('repo'), 'https://example/repo')
    self.assertEqual(repository.RepositoryName('https://example/repo'), 'repo')

  def testAddRepositoryRaisesWithDuplicateName(self):
    name = repository.RepositoryName(
      'https://example/chromium', add_if_missing=True)
    self.assertEqual(name, 'chromium')

    self.assertEqual(
      repository.RepositoryUrl('chromium'), 'https://example/chromium')
    self.assertEqual(
      repository.RepositoryName('https://example/chromium'), 'chromium')
