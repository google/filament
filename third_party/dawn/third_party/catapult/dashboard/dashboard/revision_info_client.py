# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Client for getting revision info."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import six

from google.appengine.ext import ndb

from dashboard.common import namespaced_stored_object
from dashboard.common import utils
from dashboard.models import graph_data

# TODO(fancl): Move revision info into a seperate config service

# The revision info (stored in datastore) is a dict mapping of revision type,
# which should be a string starting with "r_", to a dict of properties for
# that revision, including "name" and "url".
REVISION_INFO_KEY = 'revision_info'


def GetRevisionInfoConfig():
  return namespaced_stored_object.Get(REVISION_INFO_KEY) or {}


def GetRevisions(test_key, revision):
  row = graph_data.Row.query(
      ndb.AND(graph_data.Row.parent_test == utils.OldStyleTestKey(test_key),
              graph_data.Row.revision == revision)).get()
  if row is None:
    return {}
  return {k: v for k, v in row.to_dict().items() if k.startswith('r_')}


def GetRangeRevisionInfo(test_key, start, end):
  revision_info = GetRevisionInfoConfig()
  # Start position is the first commit position in the revision range. So the
  # position before start is the one we want.
  revision_start = GetRevisions(test_key, start - 1)
  revision_end = GetRevisions(test_key, end)
  infos = []
  for k, info in revision_info.items():
    if k not in revision_start or k not in revision_end:
      continue
    url = info.get('url', '')
    info['url'] = url.replace(
        '{{R1}}', six.ensure_str(revision_start[k])).replace(
            '{{R2}}', six.ensure_str(revision_end[k])).replace('{{n}}', '1000')
    infos.append(info)
  return infos
