# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from catapult_build import html_checks


class MockAffectedFile(object):

  def __init__(self, path, lines):
    self.path = path
    self.lines = lines

  def NewContents(self):
    return (l for l in self.lines)

  def LocalPath(self):
    return self.path


class MockInputApi(object):

  def __init__(self, affected_files):
    self.affected_files = affected_files

  def AffectedFiles(self, file_filter=None, **_):
    if file_filter:
      return [f for f in self.affected_files if file_filter(f)]
    return self.affected_files


class MockOutputApi(object):

  def PresubmitError(self, error_text):
    return error_text


class HtmlChecksTest(unittest.TestCase):

  def testRunChecksShowsErrorForWrongDoctype(self):
    f = MockAffectedFile('foo/x.html', ['<!DOCTYPE XHTML1.0>'])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual(1, len(errors))

  def testRunChecksReturnsErrorForEmptyFile(self):
    f = MockAffectedFile('foo/x.html', [])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual(1, len(errors))

  def testRunChecksNoErrorsForFileWithCorrectDocstring(self):
    f = MockAffectedFile('foo/x.html', ['<!DOCTYPE html> '])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual([], errors)

  def testRunChecksAcceptsDifferentCapitalization(self):
    f = MockAffectedFile('foo/x.html', ['<!doctype HtMl> '])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual([], errors)

  def testRunChecksAcceptsCommentsBeforeDoctype(self):
    f = MockAffectedFile('foo/x.html', ['<!-- asdf -->\n<!doctype html> '])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual([], errors)

  def testRunChecksSkipsFilesInExcludedPaths(self):
    f = MockAffectedFile('foo/x.html', ['<!DOCTYPE html XHTML1.0>'])
    errors = html_checks.RunChecks(
        MockInputApi([f]), MockOutputApi(), excluded_paths=['^foo/.*'])
    self.assertEqual([], errors)

  def testRunChecksSkipsNonHtmlFiles(self):
    f = MockAffectedFile('foo/bar.py', ['#!/usr/bin/python', 'print 10'])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual([], errors)

  def testRunChecksShowsErrorForOutOfOrderImports(self):
    f = MockAffectedFile('foo/x.html', [
        '<!DOCTYPE html>',
        '<link rel="import" href="b.html">',
        '<link rel="import" href="a.html">',
    ])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual(1, len(errors))

  def testRunChecksSkipsSuppressedOutOfOrderImports(self):
    f = MockAffectedFile('foo/x.html', [
        '<!DOCTYPE html>',
        '<link rel="import" href="b.html" data-suppress-import-order>',
        '<link rel="import" href="a.html">',
    ])
    errors = html_checks.RunChecks(MockInputApi([f]), MockOutputApi())
    self.assertEqual([], errors)
