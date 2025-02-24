# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import request

from dashboard.api import api_request_handler
from dashboard.pinpoint.models import change
from dashboard.common import utils


def _CheckUser():
  pass


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def CommitHandlerPost():
  repository = utils.SanitizeArgs(
      args=request.args, key_name='repository', default='chromium')
  git_hash = utils.SanitizeArgs(
      args=request.args, key_name='git_hash', default='HEAD')
  try:
    c = change.Commit.FromDict({
        'repository': repository,
        'git_hash': git_hash,
    })
    return c.AsDict()
  except KeyError as e:
    raise api_request_handler.BadRequestError('Unknown git hash: %s' %
                                              git_hash) from e
