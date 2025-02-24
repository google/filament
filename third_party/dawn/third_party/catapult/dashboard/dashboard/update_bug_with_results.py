# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for a cron job to update bugs after bisects."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging

from google.appengine.ext import ndb

from dashboard.common import layered_cache
from dashboard.models import anomaly
from dashboard.models import bug_data
from dashboard.services import perf_issue_service_client

_COMMIT_HASH_CACHE_KEY = 'commit_hash_%s'

_NOT_DUPLICATE_MULTIPLE_BUGS_MSG = """
Possible duplicate of crbug.com/%s, but not merging issues due to multiple
culprits in destination issue.
"""


def GetMergeIssueDetails(commit_cache_key):
  """Get's the issue this one might be merged into.

  Returns: A dict with the following fields:
    issue: The issue details from the issue tracker service.
    id: The id of the issue we should merge into. This may be set to None if
        either there is no other bug with this culprit, or we shouldn't try to
        merge into that bug.
    comments: Additional comments to add to the bug.
  """
  merge_issue_key = layered_cache.GetExternal(commit_cache_key)
  if not merge_issue_key:
    logging.debug('GetMergeIssueDetails: Could not get commit cache key "%s".',
                 commit_cache_key)
    return {'issue': {}, 'projectId': None, 'id': None, 'comments': ''}

  try:
    project, issue_id = merge_issue_key.split(':')
  except ValueError:
    project = 'chromium'
    issue_id = merge_issue_key

  merge_issue = perf_issue_service_client.GetIssue(
      issue_id, project_name=project)
  if not merge_issue:
    logging.debug('GetMergeIssueDetails: Could not get issue with id "%s"'\
                  'and merge_issue_key "%s"',
                 issue_id, merge_issue_key)
    return {'issue': {}, 'projectId': None, 'id': None, 'comments': ''}

  # Check if we can duplicate this issue against an existing issue.
  merge_issue_id = None
  additional_comments = ""

  # We won't duplicate against an issue that itself is already
  # a duplicate though. Could follow the whole chain through but we'll
  # just keep things simple and flat for now.
  if merge_issue.get('status') != perf_issue_service_client.STATUS_DUPLICATE:
    merge_issue_id = str(merge_issue.get('id'))
    project = merge_issue.get('projectId', 'chromium')

  return {
      'issue': merge_issue,
      'projectId': project,
      'id': merge_issue_id,
      'comments': additional_comments
  }


class IssueInfo(collections.namedtuple('IssueInfo', ('project', 'issue_id'))):
  __slots__ = ()


def UpdateMergeIssue(commit_cache_key,
                     merge_details,
                     bug_id,
                     project='chromium'):
  logging.debug('UpdateMergeIssue: "%s", "%s", "%s", "%s"',
                bug_id,
                commit_cache_key,
                merge_details.get('id'),
                merge_details.get('issue', {}).get('id'))
  if merge_details.get('issue', {}).get('id') is None:
    _UpdateCacheKeyForIssue(merge_details.get('id'), commit_cache_key, bug_id,
                            project)
    return

  # If the issue we were going to merge into was itself a duplicate, we don't
  # dup against it but we also don't merge existing anomalies to it or cache it.
  if merge_details['issue'].get('status') == (
      perf_issue_service_client.STATUS_DUPLICATE):
    return

  _MapAnomaliesAndUpdateBug(
      dest=IssueInfo(
          merge_details.get('projectId', 'chromium'),
          int(merge_details['issue']['id'])),
      source=IssueInfo(project, bug_id))


def _MapAnomaliesAndUpdateBug(dest, source):
  if dest.issue_id:
    _MapAnomaliesToMergeIntoBug(dest, source)
    # Mark the duplicate bug's Bug entity status as closed so that
    # it doesn't get auto triaged.
    bug = bug_data.Get(project=source.project, bug_id=source.issue_id)
    if bug:
      bug.status = bug_data.BUG_STATUS_CLOSED
      bug.put()


def _UpdateCacheKeyForIssue(merge_issue_id, commit_cache_key, bug_id, project):
  # Cache the commit info and bug ID to datastore when there is no duplicate
  # issue that this issue is getting merged into. This has to be done only
  # after the issue is updated successfully with bisect information.
  if commit_cache_key and not merge_issue_id:
    issue_info = IssueInfo(project, bug_id)
    layered_cache.SetExternal(
        commit_cache_key, '%s:%d' % (issue_info), days_to_keep=30)
    logging.info('Cached bug %s and commit info %s in the datastore.',
                 issue_info, commit_cache_key)


def _MapAnomaliesToMergeIntoBug(dest_issue, source_issue):
  """Maps anomalies from source bug to destination bug.

  Args:
    dest_issue: an IssueInfo with both the project and issue id.
    source_issue: an IssueInfo with both the project and issue id.
  """
  anomalies, _, _ = anomaly.Anomaly.QueryAsync(
      bug_id=int(source_issue.issue_id),
      project_id=(source_issue.project or 'chromium'),
  ).get_result()

  bug_id = int(dest_issue.issue_id)
  for a in anomalies:
    a.bug_id = bug_id
    a.project_id = (dest_issue.project or 'chromium')

  ndb.put_multi(anomalies)


def _GetCommitHashCacheKey(git_hash):
  """Gets a commit hash cache key for the given bisect results output.

  Args:
    results_data: Bisect results data.

  Returns:
    A string to use as a layered_cache key, or None if we don't want
    to merge any bugs based on this bisect result.
  """
  if not git_hash:
    return None
  return _COMMIT_HASH_CACHE_KEY % git_hash
