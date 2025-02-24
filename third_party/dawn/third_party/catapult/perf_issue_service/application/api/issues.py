# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from flask import make_response, Blueprint, request
from httplib2 import http
import json
import logging

from application import utils
from application.clients import issue_tracker_client

issues = Blueprint('issues', __name__)


@issues.route('/', methods=['GET'])
@issues.route('/project/<project_name>', methods=['GET'])
@utils.BearerTokenAuthorizer
def IssuesGetHandler(project_name=None):
  limit = request.args.get('limit', '2000')
  age = request.args.get('age', '3')
  status = request.args.get('status', 'open')
  labels = request.args.get('labels', '')

  client = issue_tracker_client.IssueTrackerClient(project_name)
  response = client.GetIssuesList(
    project=project_name,
    limit=limit,
    age=age,
    status=status,
    labels=labels
  )

  return make_response(response)

@issues.route('/<issue_id>/project/', methods=['GET'])
@issues.route('/<issue_id>/project/<project_name>', methods=['GET'])
@utils.BearerTokenAuthorizer
def IssuesGetByIdHandler(issue_id, project_name=None):
  # add handling before the fix on alert_group_workflow is deployed.
  if not project_name:
    project_name = 'chromium'
  client = issue_tracker_client.IssueTrackerClient(project_name, issue_id)
  response = client.GetIssue(
      issue_id=issue_id,
      project=project_name)
  return make_response(response)

@issues.route('/<issue_id>/project/<project_name>/comments', methods=['GET'])
@utils.BearerTokenAuthorizer
def CommentsHandler(issue_id, project_name):
  client = issue_tracker_client.IssueTrackerClient(project_name, issue_id)
  response = client.GetIssueComments(
      issue_id=issue_id,
      project=project_name)
  return make_response(response)

@issues.route('/', methods=['POST'])
@utils.BearerTokenAuthorizer
def IssuesPostHandler():
  try:
    data = json.loads(request.data)
  except json.JSONDecodeError as e:
    return make_response(str(e), http.HTTPStatus.BAD_REQUEST.value)

  project_name = data.get('project')
  client = issue_tracker_client.IssueTrackerClient(project_name)
  response = client.NewIssue(**data)
  return make_response(response)

@issues.route('/<issue_id>/project/<project_name>/comments', methods=['POST'])
@utils.BearerTokenAuthorizer
def CommentsPostHandler(issue_id, project_name):
  try:
    data = json.loads(request.data)
  except json.JSONDecodeError as e:
    return make_response(str(e), http.HTTPStatus.BAD_REQUEST.value)

  client = issue_tracker_client.IssueTrackerClient(project_name, issue_id)
  response = client.NewComment(
    issue_id=int(issue_id),
    project=project_name,
    **data
  )
  return make_response(response)