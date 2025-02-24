#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for strip_js_comments module."""

from __future__ import absolute_import
import unittest

from py_vulcanize import strip_js_comments


# This test case tests a protected method.
# pylint: disable=W0212
class JavaScriptStripCommentTests(unittest.TestCase):
  """Test case for _strip_js_comments and _TokenizeJS."""

  def test_strip_comments(self):
    self.assertEquals(
        'A ', strip_js_comments.StripJSComments('A // foo'))
    self.assertEquals(
        'A bar', strip_js_comments.StripJSComments('A // foo\nbar'))
    self.assertEquals(
        'A  b', strip_js_comments.StripJSComments('A /* foo */ b'))
    self.assertEquals(
        'A  b', strip_js_comments.StripJSComments('A /* foo\n */ b'))

  def test_tokenize_empty(self):
    tokens = list(strip_js_comments._TokenizeJS(''))
    self.assertEquals([], tokens)

  def test_tokenize_nl(self):
    tokens = list(strip_js_comments._TokenizeJS('\n'))
    self.assertEquals(['\n'], tokens)

  def test_tokenize_slashslash_comment(self):
    tokens = list(strip_js_comments._TokenizeJS('A // foo'))
    self.assertEquals(['A ', '//', ' foo'], tokens)

  def test_tokenize_slashslash_comment_then_newline(self):
    tokens = list(strip_js_comments._TokenizeJS('A // foo\nbar'))
    self.assertEquals(['A ', '//', ' foo', '\n', 'bar'], tokens)

  def test_tokenize_cstyle_comment_one_line(self):
    tokens = list(strip_js_comments._TokenizeJS('A /* foo */'))
    self.assertEquals(['A ', '/*', ' foo ', '*/'], tokens)

  def test_tokenize_cstyle_comment_multi_line(self):
    tokens = list(strip_js_comments._TokenizeJS('A /* foo\n*bar\n*/'))
    self.assertEquals(['A ', '/*', ' foo', '\n', '*bar', '\n', '*/'], tokens)


if __name__ == '__main__':
  unittest.main()
