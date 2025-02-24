#!/usr/bin/python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest

# pylint: disable=wrong-import-position
from unittest import mock

from bisect_lib import fetch_revision_info
# pylint: enable=wrong-import-position

_TEST_DATA_PATH = os.path.join(os.path.dirname(__file__), 'test_data')
_MOCK_RESPONSE_PATH = os.path.join(_TEST_DATA_PATH, 'MOCK_INFO_RESPONSE_FILE')


class ChromiumRevisionsTest(unittest.TestCase):

  def testRevisionInfo(self):
    commit_hash = 'c89130e28fd01062104e1be7f3a6fc3abbb80ca9'
    with mock.patch('six.moves.urllib.request.urlopen', mock.MagicMock(
        return_value=open(_MOCK_RESPONSE_PATH))):
      revision_info = fetch_revision_info.FetchRevisionInfo(
          commit_hash, depot_name='chromium')
    self.assertEqual(
        {
            'body': ('\nHiding actions without the toolbar redesign '
                     'means removing them entirely, so if\nthey exist '
                     'in the toolbar, they are considered \'visible\' '
                     '(even if they are in\nthe chevron).\n\n'
                     'BUG=544859\nBUG=548160\n\nReview URL: '
                     'https://codereview.chromium.org/1414343003\n\n'
                     'Cr-Commit-Position: refs/heads/master@{#356400}'),
            'date': 'Tue Oct 27 21:26:30 2015',
            'subject': ('[Extensions] Fix hiding browser actions '
                        'without the toolbar redesign'),
            'email': 'rdevlin.cronin@chromium.org',
            'author': 'rdevlin.cronin'
        },
        revision_info)


if __name__ == '__main__':
  unittest.main()
