# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Checks to use in PRESUBMIT.py for HTML style violations."""

import collections
import difflib
import re

import bs4

from catapult_build import parse_html


def RunChecks(input_api, output_api, excluded_paths=None):

  def ShouldCheck(affected_file):
    path = affected_file.LocalPath()
    if not path.endswith('.html'):
      return False
    if not excluded_paths:
      return True
    return not any(re.match(pattern, path) for pattern in excluded_paths)

  affected_files = input_api.AffectedFiles(
      file_filter=ShouldCheck, include_deletes=False)
  results = []
  for f in affected_files:
    CheckAffectedFile(f, results, output_api)
  return results


def CheckAffectedFile(affected_file, results, output_api):
  path = affected_file.LocalPath()
  soup = parse_html.BeautifulSoup('\n'.join(affected_file.NewContents()))
  for check in [CheckDoctype, CheckImportOrder]:
    check(path, soup, results, output_api)


def CheckDoctype(path, soup, results, output_api):
  if _HasHtml5Declaration(soup):
    return
  error_text = 'Could not find "<!DOCTYPE html>" in %s.' % path
  results.append(output_api.PresubmitError(error_text))


def _HasHtml5Declaration(soup):
  for item in soup.contents:
    if isinstance(item, bs4.Doctype) and item.lower() == 'html':
      return True
  return False


def CheckImportOrder(path, soup, results, output_api):
  grouped_hrefs = collections.defaultdict(list)  # Link rel -> [link hrefs].
  for link in soup.find_all('link'):
    if link.get('data-suppress-import-order') is not None:
      continue

    grouped_hrefs[','.join(link.get('rel'))].append(link.get('href'))

  for rel, actual_hrefs in grouped_hrefs.items():
    expected_hrefs = list(sorted(set(actual_hrefs)))
    if actual_hrefs != expected_hrefs:
      error_text = (
          'Invalid "%s" link sort order in %s:\n' % (rel, path) + ' ' +
          '\n  '.join(difflib.ndiff(actual_hrefs, expected_hrefs)) +
          '\nIf this error is invalid, you can suppress it by adding a ' +
          '"data-suppress-import-order" attribute to the out-of-order <link> ' +
          'element.')
      results.append(output_api.PresubmitError(error_text))
