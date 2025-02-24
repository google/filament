# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for getting commit information from Gitiles."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.services import request

_URL = 'https://cr-rev.appspot.com/_ah/api/crrev/v1/'


def GetNumbering(number, numbering_identifier, numbering_type, project, repo):
  params = {
      'number': number,
      'numbering_identifier': numbering_identifier,
      'numbering_type': numbering_type,
      'project': project,
      'repo': repo
  }

  return request.RequestJson(_URL + 'get_numbering', 'GET', **params)


def GetCommit(git_sha):
  return request.RequestJson(_URL + 'commit/%s' % git_sha, 'GET', None)
