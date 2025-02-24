# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime

from dashboard.pinpoint.models.change import commit_cache
from dashboard.pinpoint import test


class CommitCacheTest(test.TestCase):

  def testPutAndGet(self):
    created = datetime.datetime.now()
    commit_cache.Put('id string', 'https://example.url', 'author@example.org',
                     created, 'Subject.', 'Subject.\n\nMessage.')

    expected = {
        'url': 'https://example.url',
        'author': 'author@example.org',
        'created': created,
        'subject': 'Subject.',
        'message': 'Subject.\n\nMessage.',
    }
    self.assertEqual(commit_cache.Get('id string'), expected)

  def testCommitNotFound(self):
    with self.assertRaises(KeyError):
      commit_cache.Get('id string')
