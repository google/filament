# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

from py_vulcanize import strip_js_comments

from catapult_build import parse_html


class JSChecker(object):

  def __init__(self, input_api, output_api, file_filter=None):
    self.input_api = input_api
    self.output_api = output_api
    if file_filter:
      self.file_filter = file_filter
    else:
      self.file_filter = lambda x: True

  def RunChecks(self):
    """Checks for violations of the Chromium JavaScript style guide.

    See:
    http://chromium.org/developers/web-development-style-guide#TOC-JavaScript
    """
    results = []

    affected_files = self.input_api.AffectedFiles(
        file_filter=self.file_filter,
        include_deletes=False)

    def ShouldCheck(f):
      if f.LocalPath().endswith('.js'):
        return True
      if f.LocalPath().endswith('.html'):
        return True
      return False

    affected_js_files = [f for f in affected_files if ShouldCheck(f)]
    error_lines = []
    for f in affected_js_files:
      contents = list(f.NewContents())
      error_lines += CheckStrictMode(
          '\n'.join(contents),
          is_html_file=f.LocalPath().endswith('.html'))

    if error_lines:
      error_lines.insert(0, 'Found JavaScript style violations:')
      results.append(
          _MakeErrorOrWarning(self.output_api, '\n'.join(error_lines)))

    return results


def _ErrorHighlight(start, length):
  """Produces a row of '^'s to underline part of a string."""
  return start * ' ' + length * '^'


def _MakeErrorOrWarning(output_api, error_text):
  return output_api.PresubmitError(error_text)


def CheckStrictMode(contents, is_html_file=False):
  statements_to_check = []
  if is_html_file:
    statements_to_check.extend(_FirstStatementsInScriptElements(contents))
  else:
    statements_to_check.append(_FirstStatement(contents))
  error_lines = []
  for s in statements_to_check:
    if s != "'use strict'":
      error_lines.append('Expected "\'use strict\'" as first statement, '
                         'but found "%s" instead.' % s)
  return error_lines


def _FirstStatementsInScriptElements(contents):
  """Returns a list of first statements found in each <script> element."""
  soup = parse_html.BeautifulSoup(contents)
  script_elements = soup.find_all('script', src=None)
  return [_FirstStatement(e.get_text()) for e in script_elements]


def _FirstStatement(contents):
  """Extracts the first statement in some JS source code."""
  stripped_contents = strip_js_comments.StripJSComments(contents).strip()
  matches = re.match('^(.*?);', stripped_contents, re.DOTALL)
  if not matches:
    return ''
  return matches.group(1).strip()


def RunChecks(input_api, output_api, excluded_paths=None):

  def ShouldCheck(affected_file):
    if not excluded_paths:
      return True
    path = affected_file.LocalPath()
    return not any(re.match(pattern, path) for pattern in excluded_paths)

  return JSChecker(input_api, output_api, file_filter=ShouldCheck).RunChecks()
