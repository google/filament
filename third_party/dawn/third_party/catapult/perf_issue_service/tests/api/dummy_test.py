# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from application.app import create_app

class DummyTest(unittest.TestCase):
  def setUp(self):
    self.client = create_app().test_client()

  def testDummy(self):
    response = self.client.get('/')
    data = response.get_data(as_text=True)

    self.assertEqual(response.status_code, 200)
    self.assertEqual(data, 'welcome')
