# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a layer of abstraction for the issue tracker API."""

import logging

from application.clients import monorail_client
from application.clients import buganizer_client

BUGANIZER_PROJECTS = {
  "fuchsia": "buganizer",
  "chromium": "buganizer",
  "webrtc": "buganizer",
  # Test
  "MigratedProject": "buganizer",
  "ReadOnlyProject": "none"
}

class IssueTrackerClient:
  """Class for updating perf issues."""

  def __init__(self, project_name='chromium', issue_id=None):
    '''Choose between Monorail and Buganizer clients.

    1. If the issue id is a Buganizer ID, the value should be assigned after
       migration. We will use Buganizer client regardless of the project value.
    2. If the issue id is a Monorail ID, we need the project value to tell
       whether the project is migrated. If it is, use Buganizer client,
       otherwise Monorail client.
    '''
    if issue_id and int(issue_id) > 2000000:
      logging.debug(
        '[PerfIssueService] Project %s, issue id %s. Using Buganizer client.',
        project_name, issue_id)
      self._client = buganizer_client.BuganizerClient()
      return

    issue_tracker_service = self._GetIssueTrackerByProject(project_name)
    if issue_tracker_service == 'monorail':
      self._client = monorail_client.MonorailClient()
    elif issue_tracker_service == 'buganizer':
      self._client = buganizer_client.BuganizerClient()
    else:
      raise NotImplementedError(
        'Unknow issue tracker service target: %s', issue_tracker_service)


  def _GetIssueTrackerByProject(self, project_name):
    issue_tracker = BUGANIZER_PROJECTS.get(project_name, 'monorail')
    logging.debug(
      '[PerfIssueService] Project %s is using %s as issue tracker.',
      project_name, issue_tracker)
    return issue_tracker


  def GetIssuesList(self, **kwargs):
    """Makes a request to the issue tracker to list issues."""
    return self._client.GetIssuesList(**kwargs)

  def GetIssue(self, **kwargs):
    """Makes a request to the issue tracker to get an issue."""
    return self._client.GetIssue(**kwargs)

  def GetIssueComments(self, **kwargs):
    """Gets all the comments for the given issue."""
    return self._client.GetIssueComments(**kwargs)

  def NewIssue(self, **kwargs):
    """Create a new issue."""
    return self._client.NewIssue(**kwargs)

  def NewComment(self, **kwargs):
    """Create a new comment for the targeted issue"""
    return self._client.NewComment(**kwargs)

