# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from py_vulcanize import js_utils


class ValidateStrictModeTests(unittest.TestCase):

  def testEscapeJSIfNeeded(self):
    self.assertEqual(
        '<script>var foo;<\/script>',
        js_utils.EscapeJSIfNeeded('<script>var foo;</script>'))
    self.assertEqual(
        '<script>var foo;<\/script>',
        js_utils.EscapeJSIfNeeded('<script>var foo;<\/script>'))
