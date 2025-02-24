#!/usr/bin/python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Gets information about one commit from gitiles.

Example usage:
  ./fetch_revision_info.py 343b531d31 chromium
  ./fetch_revision_info.py 17b4e7450d v8
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
_URL_TEMPLATE = 'https://chromium.googlesource.com/%s/+/%s?format=json'

def FetchRevisionInfo(commit_hash, depot_name):
  """Gets information about a chromium revision."""
  path = depot_map.DEPOT_PATH_MAP[depot_name]
  url = _URL_TEMPLATE % (path, commit_hash)
  response = six.moves.urllib.request.urlopen(url).read()
  response_json = response[len(_GITILES_PADDING):]
  response_dict = json.loads(response_json)
  message = response_dict['message'].splitlines()
  subject = message[0]
  body = '\n'.join(message[1:])
  result = {
      'author': response_dict['author']['name'],
      'email': response_dict['author']['email'],
      'subject': subject,
      'body': body,
      'date': response_dict['committer']['time'],
  }
  return result


def Main():
  parser = argparse.ArgumentParser()
  parser.add_argument('commit_hash')
  parser.add_argument('depot', choices=list(depot_map.DEPOT_PATH_MAP))
  args = parser.parse_args()
  revision_info = FetchRevisionInfo(args.commit_hash, args.depot)
  print(json.dumps(revision_info))


if __name__ == '__main__':
  Main()
