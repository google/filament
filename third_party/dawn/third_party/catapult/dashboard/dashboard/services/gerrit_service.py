# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for interfacing with Gerrit, a web-based code review tool for Git.

API doc: https://gerrit-review.googlesource.com/Documentation/rest-api.html
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.services import request

GERRIT_SCOPE = 'https://www.googleapis.com/auth/gerritcodereview'

NotFoundError = request.NotFoundError


def GetChange(server_url, change_id, fields=None):
  url = '%s/changes/%s' % (server_url, change_id)
  return request.RequestJson(url, use_auth=True, scope=GERRIT_SCOPE, o=fields)


def PostChangeComment(server_url, change_id, comment):
  url = '%s/a/changes/%s/revisions/current/review' % (server_url, change_id)
  request.Request(
      url,
      method='POST',
      body=comment,
      use_cache=False,
      use_auth=True,
      scope=GERRIT_SCOPE)
