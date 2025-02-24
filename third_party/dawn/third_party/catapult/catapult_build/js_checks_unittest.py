# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from catapult_build import js_checks


class JsChecksTest(unittest.TestCase):

  def testCheckStrictModeReturnsNoErrorsWhenAllScriptElementsAreStrict(self):
    contents = """
        <script> 'use strict'; var a = 1 + 1;
        </script>
        <br>
        <script>
        'use strict';
        var b = 2 + 2;
        </script>
    """
    self.assertEqual(
        [], js_checks.CheckStrictMode(contents, is_html_file=True))

  def testCheckStrictModeReturnsNoErrorsWhenThereAreNoScriptTags(self):
    contents = """
        <div></div>
    """
    self.assertEqual(
        [], js_checks.CheckStrictMode(contents, is_html_file=True))

  def testCheckStrictModeReturnsNoErrorsWhenJSFileIsStrict(self):
    contents = """
        'use strict';
        var a = 1 + 1;
        var b = 2 + 2;
    """
    self.assertEqual(
        [], js_checks.CheckStrictMode(contents, is_html_file=False))

  def testCheckStrictModeWhenThereIsACommentAboveTheDeclaration(self):
    contents = """
        // This is a comment at the top
        /* another comment which
           spans two lines */
        'use strict';
        var a = 1 + 1;
        var b = 2 + 2;
    """
    self.assertEqual(
        [], js_checks.CheckStrictMode(contents, is_html_file=False))

  def testCheckStrictModeDoesntCheckExternalScriptElements(self):
    contents = """
        <script src="external.js"></script>
    """
    self.assertEqual(
        [], js_checks.CheckStrictMode(contents, is_html_file=True))

  def testCheckStrictModeReturnsAnErrorWhenOneScriptElementIsNotStrict(self):
    contents = """
        <script> 'use strict'; var a = 1 + 1;
        </script>
        <br>
        <script>
        var b = 2 + 2;
        </script>
    """
    self.assertEqual(
        1, len(js_checks.CheckStrictMode(contents, is_html_file=True)))

  def testCheckStrictModeReturnsAnErrorWhenJSFileIsNonStrict(self):
    contents = """
        var a = 1 + 1;
        var b = 2 + 2;
    """
    self.assertEqual(
        1, len(js_checks.CheckStrictMode(contents, is_html_file=False)))
