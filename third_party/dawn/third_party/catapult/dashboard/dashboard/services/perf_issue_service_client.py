# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# adding even we are running in python 3 to avoid pylint 2.7 complains.
from __future__ import absolute_import

import logging
import urllib.parse

from dashboard.common import cloud_metric
from dashboard.common import utils
from dashboard.services import request

STATUS_DUPLICATE = 'Duplicate'

if utils.IsStagingEnvironment():
  _SERVICE_URL = 'https://perf-issue-service-dot-chromeperf-stage.uc.r.appspot.com/'
else:
  _SERVICE_URL = 'https://perf-issue-service-dot-chromeperf.appspot.com/'

_ISSUES_PERFIX = 'issues/'
_ALERT_GROUP_PREFIX = 'alert_groups/'


def GetIssues(**kwargs):
  url = _SERVICE_URL + _ISSUES_PERFIX
  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetIssues', 'GET', url,
                                                 kwargs)
    resp = request.RequestJson(url, method='GET', **kwargs)
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures('GetIssues', 'GET', url,
                                                        kwargs)
    logging.error('[PerfIssueService] Error requesting issue list (%s): %s',
                  kwargs, str(e))
    return []


def GetIssue(issue_id, project_name='chromium'):
  # Normalize the project_name in case it is empty or None.
  project_name = 'chromium' if project_name is None or not project_name.strip(
  ) else project_name

  url = _SERVICE_URL + _ISSUES_PERFIX
  url += '%s/project/%s' % (issue_id, project_name)
  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetIssue', 'GET', url, {
        'issue_id': issue_id,
        'project_name': project_name
    })
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetIssue', 'GET', url, {
            'issue_id': issue_id,
            'project_name': project_name
        })
    logging.error(
        '[PerfIssueService] Error requesting issue (id: %s, project: %s): %s',
        issue_id, project_name, str(e))
    return None


def GetIssueComments(issue_id, project_name='chromium'):
  # Normalize the project_name in case it is empty or None.
  project_name = 'chromium' if project_name is None or not project_name.strip(
  ) else project_name

  url = _SERVICE_URL + _ISSUES_PERFIX
  url += '%s/project/%s/comments' % (issue_id, project_name)
  try:
    cloud_metric.PublishPerfIssueServiceRequests(
        'GetIssueComments', 'GET', url, {
            'issue_id': issue_id,
            'project_name': project_name
        })
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetIssueComments', 'GET', url, {
            'issue_id': issue_id,
            'project_name': project_name
        })
    logging.error(
        '[PerfIssueService] Error requesting comments (id: %s, project: %s): %s',
        issue_id, project_name, str(e))
    return []


def PostIssue(**kwargs):
  url = _SERVICE_URL + _ISSUES_PERFIX
  try:
    cloud_metric.PublishPerfIssueServiceRequests('PostIssue', 'POST', url,
                                                 kwargs)
    resp = request.RequestJson(url, method='POST', body=kwargs)
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures('PostIssue', 'POST',
                                                        url, kwargs)
    logging.error('[PerfIssueService] Error requesting new issue (%s): %s',
                  kwargs, str(e))
    return []


def PostIssueComment(issue_id, project_name='chromium', **kwargs):
  # Normalize the project_name in case it is empty or None.
  project_name = 'chromium' if project_name is None or not project_name.strip(
  ) else project_name

  url = _SERVICE_URL + _ISSUES_PERFIX
  url += '%s/project/%s/comments' % (issue_id, project_name)
  try:
    cloud_metric.PublishPerfIssueServiceRequests('PostIssueComment', 'POST',
                                                 url, kwargs)
    resp = request.RequestJson(url, method='POST', body=kwargs)
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures('PostIssueComment',
                                                        'POST', url, kwargs)
    logging.error(
        '[PerfIssueService] Error requesting new issue comment (id: %s, project: %s data: %s): %s',
        issue_id, project_name, kwargs, str(e))
    return []


def GetDuplicateGroupKeys(group_key):
  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += '%s/duplicates' % group_key
  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetDuplicateGroupKeys', 'GET',
                                                 url, {'group_key': group_key})
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetDuplicateGroupKeys', 'GET', url, {'group_key': group_key})
    logging.error(
        '[PerfIssueService] Error requesting duplicates by group key: %s. %s',
        group_key, str(e))
    return []


def GetCanonicalGroupByIssue(current_group_key, issue_id, project_name):
  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += '%s/canonical/issue_id/%s/project_name/%s' % (current_group_key,
                                                       issue_id, project_name)
  try:
    cloud_metric.PublishPerfIssueServiceRequests(
        'GetCanonicalGroupByIssue', 'GET', url, {
            'current_group_key': current_group_key,
            'issue_id': issue_id,
            'project_name': project_name
        })
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetCanonicalGroupByIssue', 'GET', url, {
            'current_group_key': current_group_key,
            'issue_id': issue_id,
            'project_name': project_name
        })
    logging.error(
        '[PerfIssueService] Error requesting canonical group. Group: %s. ID: %s. Project: %s. %s',
        current_group_key, issue_id, project_name, str(e))
    return []


def GetAnomaliesByAlertGroupID(group_id):
  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += '%s/anomalies' % group_id
  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetAnomaliesByAlertGroupID',
                                                 'GET', url,
                                                 {'group_id': group_id})
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetAnomaliesByAlertGroupID', 'GET', url, {'group_id': group_id})
    logging.error(
        '[PerfIssueService] Error requesting anomalies by group id: %s. %s',
        group_id, str(e))
    return []


def GetAlertGroupsForAnomaly(anomaly):
  test_key = utils.TestPath(anomaly.test)
  start_rev = anomaly.start_revision
  end_rev = anomaly.end_revision
  subs_name = anomaly.matching_subscription.name

  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  # use quote() instead of quote_plus. otherwise test_key with spaces
  # will be encoded to '+'.
  if subs_name:
    url += 'test/%s/start/%s/end/%s/sub/%s' % (urllib.parse.quote(test_key),
                                               start_rev, end_rev, subs_name)
  else:
    url += 'test/%s/start/%s/end/%s' % (urllib.parse.quote(test_key), start_rev,
                                        end_rev)

  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetAlertGroupsForAnomaly',
                                                 'GET', url,
                                                 {'test_key': test_key})
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetAlertGroupsForAnomaly', 'GET', url, {'test_key': test_key})
    logging.error(
        '[PerfIssueService] Error requesting groups by anomaly: %s. %s',
        test_key, str(e))
    return []


def GetAllActiveAlertGroups(group_type: int):
  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += 'all'

  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetAllActiveAlertGroups',
                                                 'GET', url, {})
    resp = request.RequestJson(url, method='GET', group_type=group_type)
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures(
        'GetAllActiveAlertGroups', 'GET', url, {})
    logging.error('[PerfIssueService] Error requesting all groups: %s', str(e))
    return []


def PostUngroupedAlerts(group_type: int):
  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += 'ungrouped'

  try:
    cloud_metric.PublishPerfIssueServiceRequests('PostUngroupedAlerts', 'POST',
                                                 url, {})
    resp = request.RequestJson(url, method='POST', group_type=group_type)
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures('PostUngroupedAlerts',
                                                        'POST', url, {})
    logging.error('[PerfIssueService] Error updating ungrouped alerts: %s',
                  str(e))
    return {}


def GetAlertGroupQuality(job_id, commit):
  commit_dict = commit.AsDict()
  logging.debug(
      '[GroupingQuality] Getting grouping quality for %s on commit %s.', job_id,
      commit_dict)
  commit_position = commit_dict.get('commit_position', None)
  if not commit_position:
    logging.debug(
        '[GroupingQuality] commit position is not found in commit %s.', commit)
    return {}

  url = _SERVICE_URL + _ALERT_GROUP_PREFIX
  url += 'alert_group_quality/job_id/%s/commit/%s' % (job_id, commit_position)

  try:
    cloud_metric.PublishPerfIssueServiceRequests('GetAlertGroupQuality', 'GET',
                                                 url, {})
    resp = request.RequestJson(url, method='GET')
    return resp
  except request.RequestError as e:
    cloud_metric.PublishPerfIssueServiceRequestFailures('GetAlertGroupQuality',
                                                        'GET', url, {})
    logging.error('[PerfIssueService] Error getting alert group quality: %s',
                  str(e))
    return {}
