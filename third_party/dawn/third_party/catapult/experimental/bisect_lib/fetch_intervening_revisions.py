#!/usr/bin/python
#
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Gets list of revisions between two commits and their commit positions.

Example usage:
  ./fetchInterveningRevisions.py 343b531d31 7b43807df3 chromium
  ./fetchInterveningRevisions.py 235eff9574 1e4681c33f v8

Note: Another implementation of this functionality can be found in
findit/common/gitRepository.py (https://goo.gl/Rr8j9O).
"""

from __future__ import absolute_import
from __future__ import print_function
import argparse
import json
import six.moves.urllib.request
import six.moves.urllib.error
import six.moves.urllib.parse

from bisect_lib import depot_map

_GITILES_PADDING = ')]}\'\n'
_URL_TEMPLATE = ('https://chromium.googlesource.com/%s/+log/%s..%s'
                 '?format=json&n=%d')

# Gitiles paginates the list of commits; since we want to get all of the
# commits at once, the page size should be larger than the largest revision
# range that we expect to get.
_PAGE_SIZE = 512

def FetchInterveningRevisions(start, end, depot_name):
  """Fetches a list of revision in between two commits.

  Args:
    start (str): A git commit hash in the Chromium src repository.
    end (str): Another git commit hash, after start.
    depot_name (str): A respository name.

  Returns:
    A list of pairs (commit hash, commit position), from earliest to latest,
    for all commits in between the two given commits, not including either
    of the given commits.

  Raises:
    six.moves.urllib.error.URLError: The request to gitiles failed.
    ValueError: The response wasn't valid JSON.
    KeyError: The JSON didn't contain the expected data.
  """
  revisions = _FetchRangeFromGitiles(start, end, depot_name)
  # The response from gitiles includes the end revision and is ordered
  # from latest to earliest.
  return [_CommitPair(r) for r in reversed(revisions[1:])]


def _FetchRangeFromGitiles(start, end, depot_name):
  """Fetches a list of revision dicts from gitiles.

  Make multiple requests to get multiple pages, if necessary.
  """
  revisions = []
  url = _URL_TEMPLATE % (
      depot_map.DEPOT_PATH_MAP[depot_name], start, end, _PAGE_SIZE)
  current_page_url = url
  while True:
    response = six.moves.urllib.request.urlopen(current_page_url).read()
    response_json = response[len(_GITILES_PADDING):]  # Remove padding.
    response_dict = json.loads(response_json)
    revisions.extend(response_dict['log'])
    if 'next' not in response_dict:
      break
    current_page_url = url + '&s=' + response_dict['next']
  return revisions


def _CommitPair(commit_dict):
  return (commit_dict['commit'],
          _CommitPositionFromMessage(commit_dict['message']))


def _CommitPositionFromMessage(message):
  for line in reversed(message.splitlines()):
    if line.startswith('Cr-Commit-Position:'):
      return line.split('#')[1].split('}')[0]
  return None


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('start')
  parser.add_argument('end')
  parser.add_argument('depot', choices=list(depot_map.DEPOT_PATH_MAP))
  args = parser.parse_args()
  revision_pairs = FetchInterveningRevisions(args.start, args.end, args.depot)
  print(json.dumps(revision_pairs))


if __name__ == '__main__':
  main()
