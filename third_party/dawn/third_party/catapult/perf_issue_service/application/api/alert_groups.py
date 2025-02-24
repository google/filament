# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from flask import make_response, Blueprint, request
import logging

from models import alert_group
from application import utils

alert_groups = Blueprint('alert_groups', __name__)

@alert_groups.route('/<group_id>/duplicates', methods=['GET'])
@utils.BearerTokenAuthorizer
def FindDuplicatesHandler(group_id):
  duplicate_keys = alert_group.AlertGroup.FindDuplicates(group_id)

  return make_response(duplicate_keys)


@alert_groups.route('/<current_group_key>/canonical/issue_id/<issue_id>/project_name/<project_name>', methods=['GET'])
@utils.BearerTokenAuthorizer
def FindCanonicalGroupHandler(current_group_key, issue_id, project_name):
  canonical_group = alert_group.AlertGroup.FindCanonicalGroupByIssue(current_group_key, int(issue_id), project_name)

  if canonical_group:
    return make_response({'key': canonical_group})
  return make_response({'key': ''})


@alert_groups.route('/<group_id>/anomalies', methods=['GET'])
@utils.BearerTokenAuthorizer
def GetAnomaliesHandler(group_id):
  try:
    group_id = int(group_id)
  except ValueError:
    logging.debug('Using group id %s as string.', group_id)

  try:
    anomalies = alert_group.AlertGroup.GetAnomaliesByID(group_id)
  except alert_group.NoEntityFoundException as e:
    return make_response(str(e), 404)
  return make_response(anomalies)


@alert_groups.route('/test/<path:test_key>/start/<start_rev>/end/<end_rev>', methods=['GET'])
@alert_groups.route('/test/<path:test_key>/start/<start_rev>/end/<end_rev>/sub/<subscription_name>', methods=['GET'])
@utils.BearerTokenAuthorizer
def GetGroupsForAnomalyHandler(test_key, start_rev, end_rev, subscription_name=None):
  try:
    group_type = request.args.get('group_type', 0)
    # TODO: remove the _ when parity is done.
    group_keys, _ = alert_group.AlertGroup.GetGroupsForAnomaly(
      test_key, start_rev, end_rev,
      group_type=int(group_type),
      subscription_name=subscription_name)
  except alert_group.SheriffConfigRequestException as e:
    return make_response(str(e), 500)

  return make_response(group_keys)

@alert_groups.route('/all', methods=['GET'])
@utils.BearerTokenAuthorizer
def GetAllActiveGroups():
  group_type = request.args.get('group_type')
  if not group_type:
    group_type = alert_group.DEFAULT_GROUP_TYPE

  all_group_keys = alert_group.AlertGroup.GetAll(int(group_type))

  return make_response(all_group_keys)


@alert_groups.route('/ungrouped', methods=['POST'])
@utils.BearerTokenAuthorizer
def PostUngroupedGroupsHandler():
  group_type = request.args.get('group_type')
  if not group_type:
    group_type = alert_group.DEFAULT_GROUP_TYPE

  parity_results = alert_group.AlertGroup.ProcessUngroupedAlerts(int(group_type))

  return make_response(parity_results)


@alert_groups.route('/alert_group_quality/job_id/<job_id>/commit/<commit_position>', methods=['GET'])
@utils.BearerTokenAuthorizer
def GetAlertGroupQuality(job_id, commit_position):
  result = alert_group.AlertGroup.GetAlertGroupQuality(job_id, commit_position)

  return make_response({'result': result})