# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a layer of abstraction for the buganizer API."""

# pylint: disable=line-too-long

from http import client as http_client
import json
import logging

from apiclient import discovery
from apiclient import errors
from application import utils
from application import buganizer_utils as b_utils


_DISCOVERY_URI = ('https://issuetracker.googleapis.com/$discovery/rest?version=v1&labels=GOOGLE_PUBLIC')

BUGANIZER_SCOPES = 'https://www.googleapis.com/auth/buganizer'
MAX_DISCOVERY_RETRIES = 3
MAX_REQUEST_RETRIES = 5


class NoComponentException(Exception):
  pass


class BuganizerClient:
  """Class for updating perf issues."""

  def __init__(self):
    """Initializes an object for communicate to the Buganizer.
    """
    http = utils.ServiceAccountHttp(BUGANIZER_SCOPES)
    http.timeout = 30

    # Retry connecting at least 3 times.
    attempt = 1
    while attempt != MAX_DISCOVERY_RETRIES:
      try:
        self._service = discovery.build(
            'issuetracker', 'v1', discoveryServiceUrl=_DISCOVERY_URI, http=http)
        break
      except http_client.HTTPException as e:
        logging.error('Attempt #%d: %s', attempt, e)
        if attempt == MAX_DISCOVERY_RETRIES:
          raise
      attempt += 1


  def GetIssuesList(self, limit, age, status, labels, project='chromium'):
    """Makes a request to the issue tracker to list issues.

    Args:
      limit: the limit of results to fetch
      age: the number of days after the issue was created
      status: the status of the issues
      labels: the labels (hotlists) of the issue.
              The term 'labels' is what we use in Monorail and it is no longer
              available in Buganizer. Instead, we will use hotlists.
      project: the project name in Monorail. It is not needed in Buganizer. We
              will use it to look for the corresponding 'component'.

    Returns:
      a list of issues.
      (The issues are now in Monorail format before consumers are updated.)
    """
    project = 'chromium' if project is None or not project.strip() else project

    components = b_utils.FindBuganizerComponents(monorail_project_name=project)
    if not components:
      logging.warning(
        '[Buganizer API] Failed to find components for the given project: %s',
        project)
      return []
    components_string = '|'.join(components)
    query_string = 'componentid:%s' % components_string

    # by default, buganizer return all.
    if status and status != 'all':
      query_string += ' AND status:%s' % status

    if age:
      query_string += ' AND created:%sd' % age

    if labels:
      label_list = labels.split(',')
      hotlists, extra_labels = b_utils.FindBuganizerHotlists(label_list)
      if hotlists:
        query_string += ' AND hotlistid:%s' % '|'.join(hotlists)
      if extra_labels:
        custom_field_name = b_utils.GetCustomField(project)
        query_string += ' AND %s:(%s)' % (custom_field_name, '|'.join(extra_labels))


    logging.info('[PerfIssueService] GetIssueList Query: %s', query_string)
    request = self._service.issues().list(
      query=query_string,
      pageSize=min(500, int(limit)),
      view='FULL'
    )
    response = self._ExecuteRequest(request)

    buganizer_issues = response.get('issues', []) if response else []

    logging.debug('Buganizer Issues: %s', buganizer_issues)

    monorail_issues = [
      b_utils.ReconcileBuganizerIssue(issue) for issue in buganizer_issues]

    return monorail_issues


  def GetIssue(self, issue_id, project='chromium'):
    """Makes a request to the issue tracker to get an issue.

    Args:
      issue_id: the id of the issue

    Returns:
      an issue.
      (The issues are now in Monorail format before consumers are updated.)
    """
    err_msg = 'Cannot find the migrated id for crbug %s in %s' % (
      issue_id, project)
    issue_id = b_utils.FindBuganizerIdByMonorailId(project, issue_id)
    if not issue_id:
      return {'error': err_msg}

    request = self._service.issues().get(issueId=issue_id, view='FULL')
    buganizer_issue = self._ExecuteRequest(request)

    logging.debug('Buganizer Issue for %s: %s', issue_id, buganizer_issue)

    monorail_issue = b_utils.ReconcileBuganizerIssue(buganizer_issue)

    return monorail_issue


  def GetIssueComments(self, issue_id, project='chromium'):
    """Gets all the comments for the given issue.

    The GetIssueComments is used only in the alert group workflow to check
    whether the issue is closed by Pinpoint (chromeperf's service account).
    Unlike Monorail, status update and comments are two independent workflows
    in Buganizer. As the existing workflow only looks for the status update,
    we will get the issue updates instead of issue comments.
    To avoid changes on client side, I keep the field name in 'comments' in
    the return value.

    Args:
      issue_id: the id of the issue.

    Returns:
      a list of updates of the issue.
    (The updates are now in Monorail format before consumers are updated.)
    """
    err_msg = 'Cannot find the migrated id for crbug %s in %s' % (
      issue_id, project)
    issue_id = b_utils.FindBuganizerIdByMonorailId(project, issue_id)
    if not issue_id:
      return {'error': err_msg}

    request = self._service.issues().issueUpdates().list(issueId=issue_id)
    response = self._ExecuteRequest(request)

    logging.debug('Buganizer Comments for %s: %s', issue_id, response)

    schema = self._service._schema.get('IssueState')
    status_enum = schema['properties']['status']['enum']

    if not response:
      return []

    issueUpdates = response.get('issueUpdates', [])
    # the issue updates are in reverse order that index 0 is the latest.
    # reverse the order to follow the monorail style.
    issueUpdates.reverse()
    comments = []
    for index, update in enumerate(issueUpdates):
      comment = {
        'id': index,
        'author': update.get('author', {}).get('emailAddress', ''),
        'content': update.get('issueComment', {}).get('comment', ''),
        'published': update.get('timestamp'),
        'updates': b_utils.GetBuganizerStatusUpdate(update, status_enum) or {}
      }
      comments.append(comment)
    return comments


  def NewIssue(self,
             title,
             description,
             project='chromium',
             labels=None,
             components=None,
             owner=None,
             cc=None,
             status=None):
    ''' Create an issue on Buganizer

    While the API looks the same as in monorail_client, similar to what we do
    in reconciling buganizer data, we need to reconstruct the data in the
    reversed way: from the monorail fashion to the buganizer fashion.
    The issueState property should always exist for an Issue, and these
    properties are required for an issueState:
      title, componentId, status, type, severity, priority.

    Args:
      title: a string as the issue title.
      description: a string as the initial description of the issue.
      project: this is no longer needed in Buganizer when creating an issue.
          It will be used look for project specific mappings between Monorail
          and Buganizer (which are not available yet).
      labels: a list of Monorail labels, each of which will be mapped to a
          Buganizer hotlist id.
      components: a list of component names in Monorail. The size of the list
          should always be 1 as required by Buganizer.
      owner: the email address of the issue owner/assignee.
      cc: a list of email address to which the issue update is cc'ed.
      status: the initial status of the issue

    Returns:
      {'issue_id': id, 'project_id': project_name} if succeeded; otherwise
      {'error': error_msg}
    '''
    if not components:
      raise NoComponentException(
        'Componenet ID is required when creating a new issue on Buganizer.')
    if len(components)>1:
      logging.warning(
        '[PerfIssueService] %s components on NewIssue. Using the first one: %s',
        len(components), components)
    buganizer_component_id = b_utils.FindBuganizerComponentId(components[0])

    if owner:
      monorail_status = 'Assigned'
    elif not status:
      monorail_status = 'Unconfirmed'
    else:
      monorail_status = status
    buganizer_status = b_utils.FindBuganizerStatus(monorail_status)

    priority  = 'P%s' % b_utils.LoadPriorityFromMonorailLabels(labels)

    new_issue_state = {
      'title': title,
      'componentId': buganizer_component_id,
      'status': buganizer_status,
      'type': 'BUG',
      'severity': 'S2',
      'priority': priority,
    }

    new_description = {
      'comment': description
    }

    if owner:
      new_issue_state['assignee'] = {
        'emailAddress': owner
      }
    if cc:
      emails = set(email.strip() for email in cc if email.strip())
      new_issue_state['ccs'] = [
        {'emailAddress': email} for email in emails if email
      ]

    # Add the service account as collaborator in order to keep access
    # to the issue even if it is set limit_view_trusted.
    service_account = {'emailAddress': utils.ServiceAccount()}
    pinpoint_sa = {'emailAddress': utils.PINPOINT_SKIA_SERVICE_ACCOUNT}
    new_issue_state['collaborators'] = [
        service_account,
        pinpoint_sa,
    ]

    if labels and 'Restrict-View-Google' in labels:
      access_limit = {
        'accessLevel': 'LIMIT_VIEW_TRUSTED'
      }
      new_issue_state['accessLimit'] = access_limit
      labels.remove('Restrict-View-Google')

    if labels:
      labels = [label for label in labels if not label.startswith('Pri-')]
      hotlist_list, extra_labels = b_utils.FindBuganizerHotlists(labels)
      new_issue_state['hotlistIds'] = [hotlist for hotlist in hotlist_list]

      custom_field_id = b_utils.GetCustomFieldId(project)
      custom_field_value = {
        'customFieldId': custom_field_id,
        'repeatedTextValue': {
          'values': extra_labels
        }
      }
      new_issue_state['customFields'] = [custom_field_value]

    new_issue = {
      'issueState': new_issue_state,
      'issueComment': new_description
    }

    logging.info('[PerfIssueService] PostIssue request: %s', new_issue)
    request = self._service.issues().create(body=new_issue)

    try:
      response = self._ExecuteRequest(request)
      logging.debug('[PerfIssueService] PostIssue response: %s', response)
      if response and 'issueId' in response:
        return {'issue_id': int(response['issueId']), 'project_id': project}
      logging.error('Failed to create new issue; response %s', response)
    except errors.HttpError as e:
      reason = self._GetErrorReason(e)
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
                  send_email='True'):
    ''' Add a new comment for an existing issue

    Adding a new comment using the same set of arguments as in Monorail.
    Those arguments will be restructured before sending to Buganizer. In
    Buganizer, we need to have an issueState with the values to add, and
    another issueState with the values to remove.

    Notice that there are rules to follow, otherwise we will see 'invalid
    argument' error. We don't have a complete list of the rules yet.

    Args:
      issue_id: the id of the issue.
      project: this is no longer needed in Buganizer when creating an issue.
          It will be used look for project specific mappings between Monorail
          and Buganizer (which are not available yet).
      comment: the new comment to add
      title: the new title of the issue
      status: the new Monorail status of the issue.
          Monorail status will be mapped to Buganizer status.
      merge_issue: the target issue which the current issue is being merged to.
      owner: the new assignee of the issue.
      cc: a list of users to CC the updates.
      components: a list of Monorail components.
          Monorail components will be mapped to Buganizer components.
      labels: a list of Monorail labels.
          Monorail labels will be mapped to Buganizer hotlists. Notice that
          a format like -Some-Label is valid in Monorail. We need to add those
          to the to-be-removed state.
      send_email: whether to send email for a specific update.
          In Buganizer we cannot force sending email to users. Instead, each
          user chooses to receive emails based on the uers's role (assignee,
          reporter, cc, etc.). Each update in Buganizer has a significance
          level. By default, user receive only Major updates unless the user is
          the assignee. Here, we send the update's significance level to be
          Minor in those cases when send_email is set False, otherwise MAJOR
          to enforce emails.

    Returns:
      The updates Issue, or error message.
    '''
    if not issue_id or issue_id < 0:
      return {
        'error': '[PerfIssueService] Missing issue id on PostIssueComment'
        }
    err_msg = 'Cannot find the migrated id for crbug %s in %s' % (
      issue_id, project)
    issue_id = b_utils.FindBuganizerIdByMonorailId(project, issue_id)
    if not issue_id:
      return {'error': err_msg}

    add_issue_state, remove_issue_state = {}, {}

    significance_override = 'MAJOR'
    if str(send_email).lower() == 'false':
      significance_override = 'MINOR'

    if title:
      add_issue_state['title'] = title

    if status:
      add_issue_state['status'] = b_utils.FindBuganizerStatus(status)

    if owner:
      add_issue_state['assignee'] = {'emailAddress': owner}

    if cc:
      ccs_to_remove = [
        email[1:] for email in cc if email.startswith('-') and len(email)>1
      ]
      ccs_to_add = [
        email for email in cc if email and not email.startswith('-')
      ]
      if ccs_to_add:
        add_issue_state['ccs'] = [
          {'emailAddress': email} for email in ccs_to_add
        ]
      if ccs_to_remove:
        remove_issue_state['ccs'] = [
          {'emailAddress': email} for email in ccs_to_remove
        ]

    if labels and any(label.startswith('Pri-') for label in labels):
      priority = 'P%s' % b_utils.LoadPriorityFromMonorailLabels(labels)
      add_issue_state['priority'] = priority
      labels = [label for label in labels if not label.startswith('Pri-')]

    # Update the access limit if 'Restrict-View-Google' exists
    if labels and 'Restrict-View-Google' in labels:
      access_limit = {
        'accessLevel': 'LIMIT_VIEW_TRUSTED'
      }
      update_issue_access_request = {
        'issueAccessLimit': access_limit
      }
      logging.debug(
        '[PerfIssueService] Updating Access level to trusted only: %s',
        update_issue_access_request)
      request = self._service.issues().updateIssueAccessLimit(
        issueId=str(issue_id), body=update_issue_access_request)
      response = self._ExecuteRequest(request)
      logging.debug('[PerfIssueService] Update access response %s', response)

      labels.remove('Restrict-View-Google')

    if components:
      if len(components)>1:
        logging.warning(
          '[PerfIssueService] More than 1 components on issue create. Using the first one.')
      new_component_id = b_utils.FindBuganizerComponentId(components[0])
      move_issue_request = {
        'componentId': str(new_component_id),
        'significanceOverride': significance_override
      }
      logging.debug('Moving issue %s to component %s',
                    issue_id, new_component_id)
      request = self._service.issues().move(
        issueId=str(issue_id), body=move_issue_request)
      response = self._ExecuteRequest(request)
      logging.debug('[PerfIssueService] Move issue response %s', response)

    if labels:
      labels_to_remove = [
        label[1:] for label in labels if label.startswith('-') and len(label)>1
      ]
      hotlists_to_remove, extra_labels_to_remove = b_utils.FindBuganizerHotlists(labels_to_remove)
      labels_to_add = [
        label for label in labels if label and not label.startswith('-')
      ]
      hotlists_to_add, extra_labels_to_add = b_utils.FindBuganizerHotlists(labels_to_add)

      if extra_labels_to_add or extra_labels_to_remove:
        get_request = self._service.issues().get(issueId=str(issue_id))
        current_state = self._ExecuteRequest(get_request)
        logging.debug('[PerfIssueService] IssueState: %s', current_state)
        custom_field_id = b_utils.GetCustomFieldId(project)
        all_custom_fields = current_state['issueState'].get('customFields', [])
        custom_labels = []
        for custom_field in all_custom_fields:
          if custom_field['customFieldId'] == str(custom_field_id):
            custom_labels = custom_field['repeatedTextValue'].get('values', [])
        logging.debug('[PerfIssueService] Loaded labels %s.', custom_labels)
        if extra_labels_to_add:
          custom_labels = set(custom_labels) | set(extra_labels_to_add)
        if extra_labels_to_remove:
          custom_labels = set(custom_labels) - set(extra_labels_to_remove)
        logging.debug('[PerfIssueService] New labels %s', custom_labels)

        custom_field_value = {
          'customFieldId': custom_field_id,
          'repeatedTextValue': {
            'values': list(custom_labels)
          }
        }
        add_issue_state['customFields'] = [custom_field_value]

      for hotlist_id in hotlists_to_add:
        hotlist_entry_request = {
          'hotlistEntry': {'issueId': issue_id},
          'significanceOverride': significance_override
        }
        request = self._service.hotlists().createEntries(
          hotlistId=hotlist_id, body=hotlist_entry_request)
        response = self._ExecuteRequest(request)
        logging.debug('[PerfIssueService] Add hotlist response: %s', response)
      for hotlist_id in hotlists_to_remove:
        request = self._service.hotlists().entries().delete(
          hotlistId=str(hotlist_id), issueId=str(issue_id),
          significanceOverride=significance_override)
        response = self._ExecuteRequest(request)
        logging.debug('[PerfIssueService] Delete hotlist response: %s', response)

    if merge_issue and int(merge_issue) != issue_id:
      merge_request = {
        'targetId': merge_issue,
        'significanceOverride': significance_override
      }
      request = self._service.issues().duplicate(
        issueId=str(issue_id), body=merge_request)
      response = self._ExecuteRequest(request)
      logging.info(
        '[PerfIssueService] Issue %s marked as duplicate of %s',
        issue_id, merge_issue)
      logging.debug('[PerfIssueSeervice] Merge response: %s', response)
      if add_issue_state:
        if 'assignee' in add_issue_state:
          logging.debug(
            '[PerfIssueSeervice] Assignee %s removed before merge.',
            add_issue_state.pop('assignee'))
        if 'status' in add_issue_state:
          logging.debug(
            '[PerfIssueSeervice] Status %s removed before merge.',
            add_issue_state.pop('status'))

    modify_request = {}
    if comment:
      modify_request['issueComment'] = {'comment': comment}
    if add_issue_state:
      modify_request['addMask'] = ','.join(add_issue_state.keys())
      modify_request['add'] = add_issue_state
    if remove_issue_state:
      modify_request['removeMask'] = ','.join(remove_issue_state.keys())
      modify_request['remove'] = remove_issue_state

    if not modify_request:
      return {}

    modify_request['significanceOverride'] = significance_override

    response = self._MakeCommentRequest(issue_id, modify_request)

    return response


  def _MakeCommentRequest(self, issue_id, modify_request, retry=True):
    try:
      logging.debug('[PerfIssueService] Post comment request body %s', modify_request)
      request = self._service.issues().modify(issueId=str(issue_id), body=modify_request)
      response = self._ExecuteRequest(request)
      logging.debug('[PerfIssueService] Post comment response %s', response)
      if response:
        return response
    except errors.HttpError as e:
      logging.warning(
        '[PerfIssueService] Buganizer error on post comments: %s', str(e))
      reason = self._GetErrorReason(e)
      if reason is None:
        reason = ''
      if retry and 'The user does not exist' in reason:
        logging.debug(
          'Removing assignee and ccs from issue modify request: %s',
          modify_request)
        if 'assignee' in modify_request.get('addMask', ''):
          current_add_mask_list = modify_request.get('addMask').split(',')
          current_add_mask_list.remove('assignee')
          new_add_mask = ','.join(current_add_mask_list)
          modify_request['addMask'] = new_add_mask
          del modify_request['add']['assignee']
        if 'ccs' in modify_request.get('addMask', ''):
          current_add_mask_list = modify_request.get('addMask').split(',')
          current_add_mask_list.remove('ccs')
          new_add_mask = ','.join(current_add_mask_list)
          modify_request['ccs'] = new_add_mask
          del modify_request['add']['ccs']
        return self._MakeCommentRequest(issue_id, modify_request, retry=False)

    err_msg = 'Error updating issue %s with body %s' % (
      issue_id, modify_request)
    logging.error(err_msg)
    return {'error': err_msg}


  def _ExecuteRequest(self, request):
    """Makes a request to the issue tracker.

    Args:
      request: The request object, which has a execute method.

    Returns:
      The response if there was one, or else None.
    """
    response = request.execute(
        num_retries=MAX_REQUEST_RETRIES,
        http=utils.ServiceAccountHttp(BUGANIZER_SCOPES))
    return response


  def _GetErrorReason(self, request_error):
    if request_error.resp.get('content-type', '').startswith('application/json'):
      error_json = json.loads(request_error.content).get('error')
      if error_json:
        return error_json.get('message')
    return None
