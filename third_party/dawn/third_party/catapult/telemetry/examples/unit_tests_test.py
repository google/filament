# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

class ExampleTests(unittest.TestCase):
  _retry = 0

  def test_retry_on_failure(self):
    cls = self.__class__
    if cls._retry == 3:
      return
    cls._retry += 1
    self.fail()

  def test_fail(self):
    self.fail()

  def test_also_fail(self):
    self.fail()

  def test_pass(self):
    pass

  def test_skip(self):
    self.skipTest('SKIPPING TEST')
