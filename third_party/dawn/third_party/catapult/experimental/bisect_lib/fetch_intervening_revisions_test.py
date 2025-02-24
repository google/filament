#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest

# pylint: disable=wrong-import-position
from unittest import mock

from bisect_lib import fetch_intervening_revisions
# pylint: enable=wrong-import-position

_TEST_DATA = os.path.join(os.path.dirname(__file__), 'test_data')


class FetchInterveningRevisionsTest(unittest.TestCase):

  def testFetchInterveningRevisions(self):
    response = open(os.path.join(_TEST_DATA, 'MOCK_RANGE_RESPONSE_1'))
    with mock.patch('six.moves.urllib.request.urlopen', mock.MagicMock(return_value=response)):
      revs = fetch_intervening_revisions.FetchInterveningRevisions(
          '53fc07eb478520a80af6bf8b62be259bb55db0f1',
          'c89130e28fd01062104e1be7f3a6fc3abbb80ca9',
          depot_name='chromium')
    self.assertEqual(
        revs, [
            ('32ce3b13924d84004a3e05c35942626cbe93cbbd', '356382'),
            ('07a6d9854efab6677b880defa924758334cfd47d', '356383'),
            ('22e49fb496d6ffa122c470f6071d47ccb4ccb672', '356384'),
            ('5dbc149bebecea186b693b3d780b6965eeffed0f', '356385'),
            ('ebd5f102ee89a4be5c98815c02c444fbf2b6b040', '356386'),
            ('84f6037e951c21a3b00bd3ddd034f258da6839b5', '356387'),
            ('48c1471f1f503246dd66753a4c7588d77282d2df', '356388'),
            ('66aeb2b7084850d09f3fccc7d7467b57e4da1882', '356389'),
            ('01542ac6d0fbec6aa78e33e6c7ec49a582072ea9', '356390'),
            ('8414732168a8867a5d6bd45eaade68a5820a9e34', '356391'),
            ('4f81be50501fbc02d7e44df0d56032e5885e19b6', '356392'),
            ('7bd1741893bd4e233b5562a6926d7e395d558343', '356393'),
            ('ee261f306c3c66e96339aa1026d62a6d953302fe', '356394'),
            ('f1c777e3f97a16cc6a3aa922a23602fa59412989', '356395'),
            ('8fcc8af20a3d41b0512e3b1486e4dc7de528a72b', '356396'),
            ('3861789af25e2d3502f0fb7080da5785d31308aa', '356397'),
            ('6feaa73a54d0515ad2940709161ca0a5ad91d1f8', '356398'),
            ('2e93263dc74f0496100435e1fd7232e9e8323af0', '356399')
        ])

  def testFetchInterveningRevisionsPagination(self):

    def MockUrlopen(url):
      if 's=' not in url:
        return open(os.path.join(_TEST_DATA, 'MOCK_RANGE_RESPONSE_2_PAGE_1'))
      return open(os.path.join(_TEST_DATA, 'MOCK_RANGE_RESPONSE_2_PAGE_2'))

    with mock.patch('six.moves.urllib.request.urlopen', MockUrlopen):
      revs = fetch_intervening_revisions.FetchInterveningRevisions(
          '7bd1741893bd4e233b5562a6926d7e395d558343',
          '3861789af25e2d3502f0fb7080da5785d31308aa',
          depot_name='chromium')
    self.assertEqual(
        revs, [
            ('ee261f306c3c66e96339aa1026d62a6d953302fe', '356394'),
            ('f1c777e3f97a16cc6a3aa922a23602fa59412989', '356395'),
            ('8fcc8af20a3d41b0512e3b1486e4dc7de528a72b', '356396'),
        ])


if __name__ == '__main__':
  unittest.main()
