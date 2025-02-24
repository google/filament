# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The database models for bug data."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.ext import ndb

BUG_STATUS_OPENED = 'opened'
BUG_STATUS_CLOSED = 'closed'
BUG_STATUS_RECOVERED = 'recovered'

BISECT_STATUS_STARTED = 'started'
BISECT_STATUS_COMPLETED = 'completed'
BISECT_STATUS_FAILED = 'failed'


class Bug(ndb.Model):
  """Information about a bug created in issue tracker.

  Keys for Bug entities will be in the form ndb.Key('Bug', <bug_id>).
  """
  status = ndb.StringProperty(
      default=BUG_STATUS_OPENED,
      choices=[
          BUG_STATUS_OPENED,
          BUG_STATUS_CLOSED,
          BUG_STATUS_RECOVERED,
      ],
      indexed=True)

  # Status of the latest bisect run for this bug
  # (e.g., started, failed, completed).
  latest_bisect_status = ndb.StringProperty(
      default=None,
      choices=[
          BISECT_STATUS_STARTED,
          BISECT_STATUS_COMPLETED,
          BISECT_STATUS_FAILED,
      ],
      indexed=True)

  # The time that the Bug entity was created.
  timestamp = ndb.DateTimeProperty(indexed=True, auto_now_add=True)

  @classmethod
  def New(cls, project, bug_id):
    if not project:
      raise ValueError('project is required')
    if not bug_id:
      raise ValueError('bug_id is required')
    return cls(id='%s:%d' % (project, int(bug_id)))


def SetBisectStatus(bug_id, status, project):
  """Sets the bisect status for a Bug entity."""
  if bug_id is None or bug_id < 0:
    return
  bug = Get(project, bug_id)
  if bug:
    bug.latest_bisect_status = status
    bug.put()
  else:
    logging.error('Bug %s does not exist.', bug_id)


def Key(project, bug_id):
  if not project:
    raise ValueError('project must not be empty or None')
  if not bug_id or bug_id <= 0:
    raise ValueError(
        'bug_id must not be empty, zero, None and must be positive')
  return ndb.Key('Bug', '%s:%d' % (project, bug_id))


def Get(project, bug_id):
  if not project:
    raise ValueError('project must not be empty or None')
  if not bug_id or bug_id <= 0:
    raise ValueError(
        'bug_id must not be empty, zero, None and must be positive')
  # Due to legacy reasons, the key might just be an issue id
  # or a combination of the issue id and project.
  bug = ndb.Key('Bug', '%d' % (bug_id,)).get()
  return bug or Key(project=project, bug_id=bug_id).get()
