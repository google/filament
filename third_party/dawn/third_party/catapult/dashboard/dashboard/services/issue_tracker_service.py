# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a layer of abstraction for the issue tracker API."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from six.moves import http_client
import json
import logging

from apiclient import discovery
from apiclient import errors
from dashboard.common import utils

_DISCOVERY_URI = ('https://monorail-prod.appspot.com'
                  '/_ah/api/discovery/v1/apis/{api}/{apiVersion}/rest')

STATUS_DUPLICATE = 'Duplicate'
MAX_DISCOVERY_RETRIES = 3
MAX_REQUEST_RETRIES = 5


class IssueTrackerService:
  """Class for updating bug issues."""

  def __init__(self):
    """Initializes an object for adding and updating bugs on the issue tracker.

    This object can be re-used to make multiple requests without calling
    apliclient.discovery.build multiple times.

    This class makes requests to the Monorail API.
    API explorer: https://goo.gl/xWd0dX

    Args:
      http: A Http object that requests will be made through; this should be an
          Http object that's already authenticated via OAuth2.
    """
    http = utils.ServiceAccountHttp()
    http.timeout = 30

    # Retry connecting at least 3 times.
    attempt = 1
    while attempt != MAX_DISCOVERY_RETRIES:
      try:
        self._service = discovery.build(
            'monorail', 'v1', discoveryServiceUrl=_DISCOVERY_URI, http=http)
        break
      except http_client.HTTPException as e:
        logging.error('Attempt #%d: %s', attempt, e)
        if attempt == MAX_DISCOVERY_RETRIES:
          raise
      attempt += 1

  def AddBugComment(self,
                    bug_id,
                    comment,
                    project='chromium',
                    summary=None,
                    status=None,
                    cc_list=None,
                    merge_issue=None,
                    components=None,
                    labels=None,
                    owner=None,
                    send_email=True):
    """Adds a comment with the bisect results to the given bug.

    Args:
      bug_id: Bug ID of the issue to update.
      comment: Bisect results information.
      status: A string status for bug, e.g. Assigned, Duplicate, WontFix, etc.
      cc_list: List of email addresses of users to add to the CC list.
      merge_issue: ID of the issue to be merged into; specifying this option
          implies that the status should be "Duplicate".
      components: List of components to add/remove from the issue.
      labels: List of labels for bug.
      owner: Owner of the bug.
      send_email: True to send email to bug cc list, False otherwise.

    Returns:
      True if successful, False otherwise.
    """
    if not bug_id or bug_id < 0:
      return False

    body = {'content': comment}
    updates = {}
    # Mark issue as duplicate when relevant bug ID is found in the datastore.
    # Avoid marking an issue as duplicate of itself.
    if merge_issue and int(merge_issue) != bug_id:
      status = STATUS_DUPLICATE
      updates['mergedInto'] = '%s:%s' % (project, merge_issue)
      logging.info('Bug %s marked as duplicate of %s', bug_id, merge_issue)
    if summary:
      updates['summary'] = summary
    if status:
      updates['status'] = status
    if cc_list:
      updates['cc'] = cc_list
    if labels:
      updates['labels'] = labels
    if owner:
      updates['owner'] = owner
    if components:
      updates['components'] = components
    body['updates'] = updates

    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    return self._MakeCommentRequest(
        bug_id, body, project=project, send_email=send_email)

  def List(self, project='chromium', **kwargs):
    """Makes a request to the issue tracker to list bugs."""
    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    request = self._service.issues().list(projectId=project, **kwargs)
    return self._ExecuteRequest(request)

  def GetIssue(self, issue_id, project='chromium'):
    """Makes a request to the issue tracker to get an issue."""
    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    request = self._service.issues().get(projectId=project, issueId=issue_id)
    return self._ExecuteRequest(request)

  def _MakeCommentRequest(self,
                          bug_id,
                          body,
                          project='chromium',
                          retry=True,
                          send_email=True):
    """Makes a request to the issue tracker to update a bug.

    Args:
      bug_id: Bug ID of the issue.
      body: Dict of comment parameters.
      project: The project id in Monorail.
      retry: True to retry on failure, False otherwise.
      send_email: True to send email to bug cc list, False otherwise.

    Returns:
      True if successful posted a comment or issue was deleted.  False if
      making a comment failed unexpectedly.
    """
    request = self._service.issues().comments().insert(
        projectId=project, issueId=bug_id, sendEmail=send_email, body=body)
    try:
      response = self._ExecuteRequest(request)
      logging.debug('Monorail response = %s', response)
      if response is not None:
        return True
    except errors.HttpError as e:
      logging.error('Monorail error: %s', e)
      reason = _GetErrorReason(e)
      if reason is None:
        reason = ''
      # Retry without owner if we cannot set owner to this issue.
      if retry and 'The user does not exist' in reason:
        # Remove both the owner and the cc list.
        # TODO (crbug.com/806392): We should probably figure out which user it
        # is rather than removing all of them.
        if 'owner' in body['updates']:
          del body['updates']['owner']
        if 'cc' in body['updates']:
          del body['updates']['cc']
        return self._MakeCommentRequest(bug_id, body, retry=False)
      if retry and 'Issue owner must be a project member' in reason:
        # Remove the owner but retain the cc list.
        if 'owner' in body['updates']:
          del body['updates']['owner']
        return self._MakeCommentRequest(bug_id, body, retry=False)
      # This error reason is received when issue is deleted.
      if 'User is not allowed to view this issue' in reason:
        logging.warning('Unable to update bug %s with body %s', bug_id, body)
        return True
    logging.error(
        'Error updating issue %s:%s with body %s',
        project,
        bug_id,
        body,
    )
    return False

  def NewBug(self,
             title,
             description,
             project='chromium',
             labels=None,
             components=None,
             owner=None,
             cc=None,
             status=None):
    """Creates a new bug.

    Args:
      title: The short title text of the bug.
      description: The body text for the bug.
      project: The project id in Monorail.
      labels: Starting labels for the bug.
      components: Starting components for the bug.
      owner: Starting owner account name.
      cc: list of email addresses to CC on the bug.
      status: defaults to Assigned if owner else Unconfirmed.

    Returns:
      A dict containing the bug_id (if successful), or the error message if not.
    """
    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    body = {
        'title': title,
        'summary': title,
        'description': description,
        'labels': labels or [],
        'components': components or [],
        'status': status or ('Assigned' if owner else 'Unconfirmed'),
        'projectId': project,
    }
    if owner:
      body['owner'] = {'name': owner}
    if cc:
      # We deduplicate the CC'ed emails to avoid having to forward
      # those to the issue tracker.
      accounts = set(email.strip() for email in cc)
      body['cc'] = [{'name': account} for account in accounts if account]

    return self._MakeCreateRequest(body, project)

  def _MakeCreateRequest(self, body, project):
    """Makes a request to create a new bug.

    Args:
      body: The request body parameter dictionary.
      project: The projectId parameter.

    Returns:
      A dict containing the bug_id and project_id (if successful), or the error
      message if not.
    """
    request = self._service.issues().insert(
        projectId=project, sendEmail=True, body=body)
    logging.info('Making create issue request with body %s', body)
    logging.info('Issue tracker project = %s', project)
    try:
      response = self._ExecuteRequest(request)
      if response and 'id' in response:
        return {'bug_id': response['id'], 'project_id': project}
      logging.error('Failed to create new bug; response %s', response)
    except errors.HttpError as e:
      reason = _GetErrorReason(e)
      return {'error': reason}
    except http_client.HTTPException as e:
      return {'error': str(e)}
    return {'error': 'Unknown failure creating issue.'}

  def GetIssueComments(self, bug_id, project='chromium'):
    """Gets all the comments for the given bug.

    Args:
      bug_id: Bug ID of the issue to update.

    Returns:
      A list of comments
    """
    if not bug_id or bug_id < 0:
      return None
    response = self._MakeGetCommentsRequest(bug_id, project=project)
    if not response:
      return None
    return [{
        'id': r['id'],
        'author': r['author'].get('name'),
        'content': r['content'],
        'published': r['published'],
        'updates': r['updates']
    } for r in response.get('items')]

  def GetLastBugCommentsAndTimestamp(self, bug_id, project='chromium'):
    """Gets last updated comments and timestamp in the given bug.

    Args:
      bug_id: Bug ID of the issue to update.

    Returns:
      A dictionary with last comment and timestamp, or None on failure.
    """
    if not bug_id or bug_id < 0:
      return None
    response = self._MakeGetCommentsRequest(bug_id, project=project)
    if response and all(
        v in list(response.keys()) for v in ['totalResults', 'items']):
      bug_comments = response.get('items')[response.get('totalResults') - 1]
      if bug_comments.get('content') and bug_comments.get('published'):
        return {
            'comment': bug_comments.get('content'),
            'timestamp': bug_comments.get('published')
        }
    return None

  def _MakeGetCommentsRequest(self, bug_id, project):
    """Makes a request to the issue tracker to get comments in the bug."""
    # TODO (prasadv): By default the max number of comments retrieved in
    # one request is 100. Since bisect-fyi jobs may have more then 100
    # comments for now we set this maxResults count as 10000.
    # Remove this max count once we find a way to clear old comments
    # on FYI issues.
    request = self._service.issues().comments().list(
        projectId=project, issueId=bug_id, maxResults=10000)
    return self._ExecuteRequest(request)

  def _ExecuteRequest(self, request):
    """Makes a request to the issue tracker.

    Args:
      request: The request object, which has a execute method.

    Returns:
      The response if there was one, or else None.
    """
    response = request.execute(
        num_retries=MAX_REQUEST_RETRIES, http=utils.ServiceAccountHttp())
    return response


def _GetErrorReason(request_error):
  if request_error.resp.get('content-type', '').startswith('application/json'):
    error_json = json.loads(request_error.content).get('error')
    if error_json:
      return error_json.get('message')
  return None
