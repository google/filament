# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a layer of abstraction for the issue tracker API."""

from http import client as http_client
import json
import logging

from apiclient import discovery
from apiclient import errors
from application import utils

_DISCOVERY_URI = ('https://monorail-prod.appspot.com'
                  '/_ah/api/discovery/v1/apis/{api}/{apiVersion}/rest')

STATUS_DUPLICATE = 'Duplicate'
MAX_DISCOVERY_RETRIES = 3
MAX_REQUEST_RETRIES = 5


class MonorailClient:
  """Class for updating perf issues."""

  def __init__(self):
    """Initializes an object for communicate to the issue tracker.

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

  def GetIssuesList(self, project, limit, age, status, labels):
    """Makes a request to the issue tracker to list issues."""
    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    request = self._service.issues().list(
      projectId=project,
      q='opened>today-%s' % age,
      can=status,
      label=labels,
      maxResults=limit,
      sort='-id'
    )
    # request = self._service.issues().list(projectId=project, **kwargs)
    response = self._ExecuteRequest(request)
    return response.get('items', []) if response else []

  def GetIssue(self, issue_id, project='chromium'):
    """Makes a request to the issue tracker to get an issue."""
    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project
    request = self._service.issues().get(projectId=project, issueId=issue_id)
    return self._ExecuteRequest(request)

  def GetIssueComments(self, issue_id, project='chromium'):
    """Gets all the comments for the given issue.

    Args:
      issue_id: Issue ID of the issue to update.

    Returns:
      A list of comments
    """
    request = self._service.issues().comments().list(
        projectId=project, issueId=issue_id, maxResults=1000)
    response = self._ExecuteRequest(request)
    if not response:
      return None
    return [{
        'id': r['id'],
        'author': r['author'].get('name'),
        'content': r['content'],
        'published': r['published'],
        'updates': r['updates']
    } for r in response.get('items')]

  def NewIssue(self,
             title,
             description,
             project='chromium',
             labels=None,
             components=None,
             owner=None,
             cc=None,
             status=None):
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
      accounts = set(email.strip() for email in cc if email.strip())
      body['cc'] = [{'name': account} for account in accounts if account]

    request = self._service.issues().insert(
        projectId=project, sendEmail=True, body=body)
    logging.info('Making create issue request with body %s', body)
    logging.info('Issue tracker project = %s', project)
    try:
      response = self._ExecuteRequest(request)
      if response and 'id' in response:
        return {'issue_id': response['id'], 'project_id': project}
      logging.error('Failed to create new issue; response %s', response)
    except errors.HttpError as e:
      reason = _GetErrorReason(e)
      return {'error': reason}
    except http_client.HTTPException as e:
      return {'error': str(e)}
    return {'error': 'Unknown failure creating issue.'}

  def NewComment(self,
                  issue_id,
                  project='chromium',
                  comment='',
                  title=None,
                  status=None,
                  merge_issue=None,
                  owner=None,
                  cc=None,
                  components=None,
                  labels=None,
                  send_email=True):
    if not issue_id or issue_id < 0:
      return {'error': 'Missing issue id.'}

    # Normalize the project in case it is empty or None.
    project = 'chromium' if project is None or not project.strip() else project

    body = {'content': comment}
    updates = {}
    # Mark issue as duplicate when relevant issue ID is found in the datastore.
    # Avoid marking an issue as duplicate of itself.
    if merge_issue and int(merge_issue) != issue_id:
      status = STATUS_DUPLICATE
      updates['mergedInto'] = '%s:%s' % (project, merge_issue)
      logging.info('Issue %s marked as duplicate of %s', issue_id, merge_issue)
    if title:
      updates['summary'] = title
    if status:
      updates['status'] = status
    if cc:
      updates['cc'] = cc
    if labels:
      updates['labels'] = labels
    if owner:
      updates['owner'] = owner
    if components:
      updates['components'] = components
    body['updates'] = updates

    return self._MakeCommentRequest(
        issue_id, body, project=project, send_email=send_email)

  def _MakeCommentRequest(self,
                          issue_id,
                          body,
                          project='chromium',
                          retry=True,
                          send_email=True):
    request = self._service.issues().comments().insert(
        projectId=project, issueId=issue_id, sendEmail=send_email, body=body)
    try:
      response = self._ExecuteRequest(request)
      logging.debug('Monorail response = %s', response)
      if response is not None:
        return {
          'Comment id': response['id'],
          'Content': response['content']
        }
    except errors.HttpError as e:
      logging.warning('Seeing Monorail error: %s. Could be handleable', str(e))
      reason = _GetErrorReason(e)
      if reason is None:
        reason = ''
      # Retry without owner if we cannot set owner to this issue.
      if retry and 'The user does not exist' in reason:
        # Remove both the owner and the cc list.
        # TODO (crbug.com/806392): We should probably figure out which user it
        # is rather than removing all of them.
        if 'owner' in body['updates']:
          logging.debug(
            'Removing owner %s as possible invalid user.', body['updates']['owner'])
          del body['updates']['owner']
        if 'cc' in body['updates']:
          logging.debug(
            'Removing cc list %s as possible invalid user.', body['updates']['cc'])
          del body['updates']['cc']
        return self._MakeCommentRequest(issue_id, body, retry=False)
      if retry and 'Issue owner must be a project member' in reason:
        # Remove the owner but retain the cc list.
        if 'owner' in body['updates']:
          logging.debug(
            'Removing owner %s as non-project member.', body['updates']['owner'])
          del body['updates']['owner']
        return self._MakeCommentRequest(issue_id, body, retry=False)
      # This error reason is received when issue is deleted.
      if 'User is not allowed to view this issue' in reason:
        logging.warning(
          'Unable to update issue %s with body %s', issue_id, body)
        return {'error': reason}

    err_msg = 'Error updating issue %s:%s with body %s' % (
      project, issue_id, body)
    logging.error(err_msg
    )
    return {'error': err_msg}

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
