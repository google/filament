# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models.change.change import Change
from dashboard.pinpoint.models.change.commit import Commit
from dashboard.pinpoint.models.change.commit import NonLinearError
from dashboard.pinpoint.models.change.commit import DepsParsingError
from dashboard.pinpoint.models.change.patch import GerritPatch


def ReconstituteChange(change_dict):
  return Change(
      commits=[
          Commit(
              repository=commit.get('repository'),
              git_hash=commit.get('git_hash'))
          for commit in change_dict.get('commits')
      ],
      patch=change_dict.get('patch'))
