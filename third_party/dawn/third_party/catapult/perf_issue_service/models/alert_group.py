# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging

from application.clients import datastore_client
from application.clients import sheriff_config_client

DEFAULT_GROUP_TYPE = datastore_client.AlertGroupType.test_suite
DEFAULT_UNGROUPED_GROUP_NAME = 'Ungrouped'
SKIA_UNGROUPED_GROUP_NAME = 'Ungrouped_Skia'

UNGROUPED_GROUP_MAPPING = {
  datastore_client.AlertGroupType.test_suite: DEFAULT_UNGROUPED_GROUP_NAME,
  datastore_client.AlertGroupType.test_suite_skia: SKIA_UNGROUPED_GROUP_NAME
}

class NoEntityFoundException(Exception):
  pass


class SheriffConfigRequestException(Exception):
  pass

class AlertGroup:
  ds_client = datastore_client.DataStoreClient()

  @classmethod
  def FindDuplicates(cls, group_id):
    '''Find the alert groups which are using the current group as canonical.

    Args:
      group_id: the id of the current group.

    Return:
      a list of groud ids.
    '''
    filters = [('canonical_group', '=', cls.ds_client.AlertGroupKey(group_id))]
    duplicates = list(cls.ds_client.QueryAlertGroup(extra_filters=filters))

    return [cls.ds_client.GetEntityId(g, key_type='name') for g in duplicates]


  @classmethod
  def FindCanonicalGroupByIssue(cls, current_group_id, issue_id, project_name):
    '''Find the canonical group which the current group's issue is merged to.

    Consider an alert group GA has a filed issue A, another group GB has filed
    issue B. If A is merged into B, then GB is consider the canonical group of
    GA.

    Args:
      current_group_id: the id of the current group
      issue_id: the id of the issue which is merged to.
      project_name: the monorail project name of the issue.

    Returns:
      The id of the canonical group.
    '''
    filters = [
      ('bug.bug_id', '=', issue_id),
      ('bug.project', '=', project_name)
    ]

    query_result = list(cls.ds_client.QueryAlertGroup(extra_filters=filters, limit=1))

    if not query_result:
      return None

    canonical_group = query_result[0]
    visited = {current_group_id}
    while dict(canonical_group).get('canonical_group'):
      visited.add(canonical_group.key.name)
      next_group_key = dict(canonical_group).get('canonical_group')
      # Visited check is just precaution.
      # If it is true - the system previously failed to prevent loop creation.
      if next_group_key.name in visited:
        logging.warning(
            'Alert group auto merge failed. Found a loop while '
            'searching for a canonical group for %r', current_group_id)
        return None
      canonical_group = cls.ds_client.GetEntityByKey(next_group_key)

    return canonical_group.key.name


  @classmethod
  def GetAnomaliesByID(cls, group_id):
    ''' Given a group id, return a list of anomaly id.

    Args:
      group_id: the id of the alert group

    Returns:
      A list of anomaly IDs.
    '''
    group_key = cls.ds_client.AlertGroupKey(group_id)
    group = cls.ds_client.GetEntityByKey(group_key)
    if group:
      return [a.id for a in group.get('anomalies')]
    raise NoEntityFoundException('No Alert Group Found with id: %s', group_id)

  @classmethod
  def Get(cls, group_name, group_type, active=True):
    ''' Get a list of alert groups by the group name and group type

    Args:
      group_name: the name value of the alert group
      group_type: the group type, which should be either 0 (test_suite)
                  or 2 (reserved)
    Returns:
      A list of ids of the matched alert groups
    '''
    filters = [
      ('name', '=', group_name)
    ]
    groups = list(cls.ds_client.QueryAlertGroup(active=active, extra_filters=filters))

    return [g for g in groups if g.get('group_type') == group_type]


  @classmethod
  def GetGroupsForAnomaly(
    cls, test_key, start_rev, end_rev, create_on_ungrouped=False, parity=False,
    group_type=datastore_client.AlertGroupType.test_suite,
    subscription_name=None):
    ''' Find the alert groups for the anomaly.

    Given the test_key and revision range of an anomaly:
    1. find the subscriptions.
    2. for each subscriptions, find the groups for the anomaly.
        if no existing group is found:
         - if create_on_ungrouped is True, create a new group.
         - otherwise, use the 'ungrouped'.

    Args:
      test_key: the key of a test metadata
      start_rev: the start revision of the anomaly
      end_rev: the end revision of the anomaly
      create_on_ungrouped: the located subscription will have a new alert
          group created if this value is true; otherwise, the existing
          'upgrouped' group will be used.
      parity: testing flag for result parity
      group_type: the group type to look for.
      subscription_name: the matching subscription name of the anomaly.

    Returns:
      a list of group ids.
    '''
    logging.debug('GetGroupsForAnomaly starts with %s, %s, %s, %s, %s',
                   test_key, start_rev, end_rev, group_type, subscription_name)

    sc_client = sheriff_config_client.GetSheriffConfigClient()
    matched_configs, err_msg = sc_client.Match(test_key)

    if err_msg is not None:
      raise SheriffConfigRequestException(err_msg)

    if not matched_configs:
      return [], []

    start_rev = int(start_rev)
    end_rev = int(end_rev)
    master_name = test_key.split('/')[0]
    benchmark_name = test_key.split('/')[2]

    existing_groups = cls.Get(benchmark_name, 0)
    result_groups = set()
    new_groups = set()

    logging.debug(
      '[Grouping] Matching between %s subscriptions and %s groups (%s)',
      len(matched_configs), len(existing_groups), benchmark_name)
    logging.debug(
      '[Grouping] All matched subscriptions: %s',
      [config['subscription'].get('name', 'nil') for config in matched_configs]
    )
    logging.debug(
      '[Grouping] All existing groups: %s',
      [g.key.name for g in existing_groups]
    )

    for config in matched_configs:
      s = config['subscription']
      if subscription_name and s.get('name') != subscription_name:
        continue
      has_overlapped = False
      for g in existing_groups:
        if 'project_id' not in g:
          # crbug/1475410. We noticed some alert group has no 'project_id'.
          logging.warning('Project_id field does not exist in group: %s', g)
        # replace empty project id using the default 'chromium'
        project_id_in_group = g.get('project_id', '') or 'chromium'
        project_id_in_subscription = s.get('monorail_project_id', '') or 'chromium'
        if (g['domain'] == master_name and
            g['subscription_name'] == s.get('name') and
            project_id_in_group == project_id_in_subscription and
            max(g['revision']['start'], start_rev) <= min(g['revision']['end'], end_rev) and
            (abs(g['revision']['start'] - start_rev) + abs(g['revision']['end'] - end_rev) <= 100 or g['domain'] != 'ChromiumPerf')):
          has_overlapped = True
          result_groups.add(g.key.name)
      if not has_overlapped:
        if create_on_ungrouped:
          new_group = cls.ds_client.NewAlertGroup(
            benchmark_name=benchmark_name,
            master_name=master_name,
            subscription_name=s.get('name'),
            group_type=group_type,
            project_id=s.get('monorail_project_id', ''),
            start_rev=start_rev,
            end_rev=end_rev
          )
          logging.info('Saving new group %s', new_group.key.name)
          if parity:
            new_groups.add(new_group.key.name)
          cls.ds_client.SaveAlertGroup(new_group)
          result_groups.add(new_group.key.name)
        else:
          # return the id of the 'ungrouped'
          ungrouped = cls._GetUngroupedGroup(group_type)
          if ungrouped:
            ungrouped_id = cls.ds_client.GetEntityId(ungrouped)
            result_groups.add(ungrouped_id)

    logging.debug('GetGroupsForAnomaly returning %s', result_groups)
    return list(result_groups), list(new_groups)


  @classmethod
  def GetAll(cls, group_type: int = datastore_client.AlertGroupType.test_suite):
    """Fetch all active alert groups

    Returns:
      A list of active alert groups
    """
    groups = list(cls.ds_client.QueryAlertGroup())
    group_keys = []
    for g in groups:
      if g.get('group_type') == group_type:
        group_keys.append(cls.ds_client.GetEntityId(g))

    # In the current use cases, we need to load the 'ungrouped' as well.
    ungrouped = cls._GetUngroupedGroup(group_type)
    if ungrouped:
      group_keys.append(cls.ds_client.GetEntityId(ungrouped))

    return group_keys


  @classmethod
  def _GetUngroupedGroup(cls, group_type):
    ''' Get the "ungrouped" group corresponding to the specified group type

    The alert_group named "ungrouped" contains the alerts for further
    processing in the next iteration of of dashboard-alert-groups-update
    cron job.

    Args:
      group_type: The type of the alert group
    Returns:
      The corresponding 'ungrouped' entity for the group type if exists,
      otherwise create a new entity and return None.
    '''
    group_name = cls._GetUngroupedGroupName(group_type)
    logging.debug('[GroupingDebug] Loading ungrouped %s.', group_name)
    if not group_name:
      return []
    ungrouped_groups = cls.Get(group_name, datastore_client.AlertGroupType.ungrouped)
    if not ungrouped_groups:
      # initiate when there is no active group called 'Ungrouped'.
      logging.debug('[GroupingDebug] Creating new ungrouped entity.')
      new_group = cls.ds_client.NewAlertGroup(
        benchmark_name=group_name,
        group_type=datastore_client.AlertGroupType.ungrouped
      )
      cls.ds_client.SaveAlertGroup(new_group)
      return None
    if len(ungrouped_groups) != 1:
      logging.warning('More than one active groups are named %s.', group_name)
    ungrouped = ungrouped_groups[0]
    logging.debug('[GroupingDebug] Loaded ungrouped %s', ungrouped)
    return ungrouped

  @classmethod
  def ProcessUngroupedAlerts(cls, group_type:int):
    """ Process each of the alert which needs a new group

    This alerts are added to an 'ungrouped' group during anomaly detection
    when no existing group is found to add them to.

    Args:
      group_type: Type of the alert group to process
    """
    IS_PARITY = True
    ungrouped = cls._GetUngroupedGroup(group_type)
    if not ungrouped:
      logging.debug('[GroupingDebug] No Ungouped is found.')
      return

    ungrouped_anomalies = cls.ds_client.GetMultiEntitiesByKeys(dict(ungrouped).get('anomalies'))
    logging.info('Loaded %i ungrouped alerts for group type %i. ID(%s)',
                  len(ungrouped_anomalies), group_type, cls.ds_client.GetEntityId(ungrouped))

    parity_results = {}
    for anomaly in ungrouped_anomalies:
      group_ids, new_ids = cls.GetGroupsForAnomaly(
        anomaly['test'].name, anomaly['start_revision'], anomaly['end_revision'],
        create_on_ungrouped=True, parity=IS_PARITY, group_type=group_type,
        subscription_name=anomaly.get('matching_subscription', {}).get('name')
        )
      anomaly['groups'] = [cls.ds_client.AlertGroupKey(group_id) for group_id in group_ids]
      logging.debug(
        '[GroupingDebug] Ungrouped anomaly %s is associated with %s',
        cls.ds_client.GetEntityId(anomaly), anomaly['groups']
      )
      if IS_PARITY:
        anomaly_id = cls.ds_client.GetEntityId(anomaly)
        parity_results[anomaly_id] = {
          "existing_groups": list(set(group_ids) - set(new_ids)),
          "new_groups": new_ids
        }
      cls.ds_client.SaveAnomaly(anomaly)

    return parity_results

  def _GetUngroupedGroupName(group_type:int=datastore_client.AlertGroupType.test_suite):
    group_name = UNGROUPED_GROUP_MAPPING.get(group_type, None)
    if not group_name:
      logging.warning('Unsupported group type: %i', group_type)
    return group_name

  @classmethod
  def GetAlertGroupQuality(cls, job_id, commit_position):
    filters = [
      ('bisection_ids', '=', job_id)
    ]

    query_result = list(cls.ds_client.QueryAlertGroup(extra_filters=filters, limit=1))

    if not query_result:
      logging.info(
        '[GroupingQuality] Cannot find an alert group with bisection id %s',
        job_id)
      return 'No alert group is found.', 404

    if len(query_result) > 1:
      logging.warning(
        '[GroupingQuality] More than one group (%s) with the same bisect (%s)',
        [g.key for g in query_result], job_id
      )

    group = query_result[0]
    pos = int(commit_position)
    out_of_range_group_revision = False

    if 'revision' in group:
      group_repo = group['revision'].get('repository', 'unknown')
      if group_repo != 'chromium':
        logging.debug(
          '[GroupingQuality] Only support chromium repo. Skipping: %s',
          group_repo)
        return 'Non-chromium repo is skipped.'

      if group['revision']['start'] > pos or group['revision']['end'] < pos:
        out_of_range_group_revision = True
        logging.debug(
          '[GroupingQuality] The culprit commit %s is outside the group (%s) revision range %s:%s.',
          pos, group.key, group['revision']['start'], group['revision']['end']
          )

    anomaly_keys = group.get('anomalies', [])
    logging.debug('[GroupingQuality]: Found %s anomalies for group %s',
                  len(anomaly_keys), group.key)
    anomalies = cls.ds_client.GetMultiEntitiesByKeys(anomaly_keys)

    out_of_range_anomalies = []
    for anomaly in anomalies:
      if anomaly['start_revision'] > pos or anomaly['end_revision'] < pos:
        out_of_range_anomalies.append(anomaly.key)
    if out_of_range_anomalies:
      message = (
          '[GroupingQuality]: The following anomalies are grouped in %s, '
          'but their revision ranges have no overlaps with the culprit commit '
          'position %s, found in bisect job %s: %s')
      logging.debug(message, group.key, commit_position, job_id,
                      out_of_range_anomalies)
    if out_of_range_anomalies or out_of_range_group_revision:
      return 'Out of range is found.'
    return 'All good.'
