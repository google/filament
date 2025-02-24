# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=too-many-lines
"""The workflow to manipulate an AlertGroup.

We want to separate the workflow from data model. So workflow
is used to implement all AlertGroup's state transitions.

A typical workflow includes these steps:
- Associate anomalies to an AlertGroup
- Update related issues
- Trigger auto-triage if necessary
- Trigger auto-bisection if necessary
- Manage an AlertGroup's lifecycle

`AlertGroupWorkflow(group).Process()` is enough for most of use cases.
But it provides the ability to mock any input and any service, which makes
testing easier and we can have a more predictable behaviour.
"""
# pylint: disable=too-many-lines

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
import itertools
import jinja2
import json
import logging
import os
import six

from google.appengine.api import datastore_errors
from google.appengine.ext import ndb

from dashboard import pinpoint_request
from dashboard import sheriff_config_client
from dashboard import revision_info_client
from dashboard.common import cloud_metric
from dashboard.common import feature_flags
from dashboard.common import file_bug
from dashboard.common import sandwich_allowlist
from dashboard.common import utils
from dashboard.models import alert_group
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import skia_helper
from dashboard.models import subscription
from dashboard.services import crrev_service
from dashboard.services import gitiles_service
from dashboard.services import perf_issue_service_client
from dashboard.services import pinpoint_service
from dashboard.services import request
from dashboard.services import workflow_service

# Templates used for rendering issue contents
_TEMPLATE_LOADER = jinja2.FileSystemLoader(
    searchpath=os.path.join(os.path.dirname(os.path.realpath(__file__))))
_TEMPLATE_ENV = jinja2.Environment(loader=_TEMPLATE_LOADER)
_TEMPLATE_ISSUE_TITLE = jinja2.Template(
    '[{{ group.subscription_name }}]: '
    '[{{ regressions|length }}] regressions in {{ group.name }}')
_TEMPLATE_ISSUE_CONTENT = _TEMPLATE_ENV.get_template(
    'alert_groups_bug_description.j2')
_TEMPLATE_ISSUE_COMMENT = _TEMPLATE_ENV.get_template(
    'alert_groups_bug_comment.j2')
_TEMPLATE_REOPEN_COMMENT = _TEMPLATE_ENV.get_template('reopen_issue_comment.j2')
_TEMPLATE_AUTO_REGRESSION_VERIFICATION_COMMENT = _TEMPLATE_ENV.get_template(
    'auto_regression_verification_comment.j2')
_TEMPLATE_AUTO_BISECT_COMMENT = _TEMPLATE_ENV.get_template(
    'auto_bisect_comment.j2')
_TEMPLATE_GROUP_WAS_MERGED = _TEMPLATE_ENV.get_template(
    'alert_groups_merge_bug_comment.j2')

# Waiting 7 days to gather more potential alerts. Just choose a long
# enough time and all alerts arrive after archived shouldn't be silent
# merged.
_ALERT_GROUP_ACTIVE_WINDOW = datetime.timedelta(days=7)

# (2020-05-01) Only ~62% issues' alerts are triggered in one hour.
# But we don't want to wait all these long tail alerts finished.
# 20 minutes are enough for a single bot.
#
# SELECT APPROX_QUANTILES(diff, 100) as percentiles
# FROM (
#   SELECT TIMESTAMP_DIFF(MAX(timestamp), MIN(timestamp), MINUTE) as diff
#   FROM chromeperf.chromeperf_dashboard_data.anomalies
#   WHERE 'Chromium Perf Sheriff' IN UNNEST(subscription_names)
#         AND bug_id IS NOT NULL AND timestamp > '2020-03-01'
#   GROUP BY bug_id
# )
_ALERT_GROUP_TRIAGE_DELAY = datetime.timedelta(minutes=20)

# The score is based on overall 60% reproduction rate of pinpoint bisection.
_ALERT_GROUP_DEFAULT_SIGNAL_QUALITY_SCORE = 0.6

# Emoji to set sandwich-related issue comments apart from other comments
# visually.
_SANDWICH = u'\U0001f96a'


class SignalQualityScore(ndb.Model):
  score = ndb.FloatProperty()
  updated_time = ndb.DateTimeProperty()


class InvalidPinpointRequest(Exception):
  pass


class AlertGroupWorkflow:
  """Workflow used to manipulate the AlertGroup.

  Workflow will assume the group passed from caller is same as the group in
  datastore. It may update the group in datastore multiple times during the
  process.
  """

  class Config(
      collections.namedtuple('WorkflowConfig',
                             ('active_window', 'triage_delay'))):
    __slots__ = ()

  class GroupUpdate(
      collections.namedtuple('GroupUpdate',
                             ('now', 'anomalies', 'issue', 'canonical_group'))):
    __slots__ = ()

    def __new__(cls, now, anomalies, issue, canonical_group=None):
      return super(AlertGroupWorkflow.GroupUpdate,
                   cls).__new__(cls, now, anomalies, issue, canonical_group)

  class BenchmarkDetails(
      collections.namedtuple('BenchmarkDetails',
                             ('name', 'owners', 'regressions', 'info_blurb'))):
    __slots__ = ()

  class BugUpdateDetails(
      collections.namedtuple('BugUpdateDetails',
                             ('components', 'cc', 'labels'))):
    __slots__ = ()

  def __init__(
      self,
      group,
      config=None,
      sheriff_config=None,
      pinpoint=None,
      cloud_workflows=None,
      crrev=None,
      gitiles=None,
      revision_info=None,
      service_account=None,
  ):
    self._group = group
    self._config = config or self.Config(
        active_window=_ALERT_GROUP_ACTIVE_WINDOW,
        triage_delay=_ALERT_GROUP_TRIAGE_DELAY,
    )
    self._sheriff_config = (
        sheriff_config or sheriff_config_client.GetSheriffConfigClient())
    self._pinpoint = pinpoint or pinpoint_service
    self._cloud_workflows = cloud_workflows or workflow_service
    self._crrev = crrev or crrev_service
    self._gitiles = gitiles or gitiles_service
    self._revision_info = revision_info or revision_info_client
    self._service_account = service_account or utils.ServiceAccountEmail

  def _FindCanonicalGroup(self, issue):
    """Finds the canonical issue group if any.

    Args:
      issue: Monorail API issue json. If the issue has any comments the json
        should contain additional 'comments' key with the list of Monorail API
        comments jsons.

    Returns:
      AlertGroup object or None if the issue is not duplicate, canonical issue
      has no corresponding group or duplicate chain forms a loop.
    """
    if issue.get('status') != perf_issue_service_client.STATUS_DUPLICATE:
      return None

    merged_into = issue.get('mergedInto', {}).get('issueId', None)
    if not merged_into:
      return None
    logging.info('Found canonical issue for the groups\' issue: %d',
                 merged_into)

    merged_issue_project = issue.get('mergedInto',
                                     {}).get('projectId',
                                             self._group.bug.project)
    query = alert_group.AlertGroup.query(
        alert_group.AlertGroup.active == True,
        alert_group.AlertGroup.bug.project == merged_issue_project,
        alert_group.AlertGroup.bug.bug_id == merged_into)
    query_result = query.fetch(limit=1)
    if not query_result:
      return None

    canonical_group = query_result[0]
    visited = set()
    while canonical_group.canonical_group:
      visited.add(canonical_group.key)
      next_group_key = canonical_group.canonical_group
      # Visited check is just precaution.
      # If it is true - the system previously failed to prevent loop creation.
      if next_group_key == self._group.key or next_group_key in visited:
        logging.warning(
            'Alert group auto merge failed. Found a loop while '
            'searching for a canonical group for %r', self._group)
        return None
      canonical_group = next_group_key.get()

    # Parity check for canonical group
    try:
      canonical_group_new = perf_issue_service_client.GetCanonicalGroupByIssue(
          self._group.key.string_id(), merged_into, merged_issue_project)
      canonical_group_key = canonical_group_new.get('key')
      original_canonical_key = canonical_group.key.string_id()
      if original_canonical_key != canonical_group_key:
        logging.warning('Imparity found for GetCanonicalGroupByIssue. %s, %s',
                        original_canonical_key, canonical_group_key)
        cloud_metric.PublishPerfIssueServiceGroupingImpariry(
            'GetCanonicalGroupByIssue')
      logging.info('Found canonical group: %s', canonical_group_key)
      canonical_group = ndb.Key('AlertGroup', canonical_group_key).get()

      return canonical_group
    except Exception as e:  # pylint: disable=broad-except
      logging.warning('Parity logic failed in GetCanonicalGroupByIssue. %s',
                      str(e))


  def _FindDuplicateGroupKeys(self):
    try:
      group_keys = perf_issue_service_client.GetDuplicateGroupKeys(
          self._group.key.string_id())
      return group_keys
    except (ValueError, datastore_errors.BadValueError):
      # only 'ungrouped' has integer key, which we should not find duplicate.
      logging.debug('[GroupingDebug] Failed to get duplicate groups. %s',
                    self._group.key)
      return []

  def _FindDuplicateGroups(self):
    query = alert_group.AlertGroup.query(
        alert_group.AlertGroup.active == True,
        alert_group.AlertGroup.canonical_group == self._group.key)
    return query.fetch()

  def _FindRelatedAnomalies(self, groups):
    query = anomaly.Anomaly.query(
        anomaly.Anomaly.groups.IN([g.key for g in groups]))
    return query.fetch()

  def _PrepareGroupUpdate(self):
    """Prepares default input for the workflow Process

    Returns:
      GroupUpdate object that contains list of related anomalies,
      Monorail API issue json and canonical AlertGroup if any.
    """
    duplicate_groups = self._FindDuplicateGroups()
    duplicate_group_keys = []
    # Parity check for duplicated groups
    try:
      duplicate_group_keys = self._FindDuplicateGroupKeys()
      original_keys = [g.key.string_id() for g in duplicate_groups]
      if sorted(duplicate_group_keys) != sorted(original_keys):
        logging.warning('Imparity found for _FindDuplicateGroups. %s, %s',
                        duplicate_group_keys, original_keys)
        cloud_metric.PublishPerfIssueServiceGroupingImpariry(
            '_FindDuplicateGroups')
    except Exception as e:  # pylint: disable=broad-except
      logging.warning('Parity logic failed in _FindDuplicateGroups(%s). %s.',
                      self._group.key, str(e))

    duplicate_groups = [
        ndb.Key('AlertGroup', k).get() for k in duplicate_group_keys
    ]
    anomalies = self._FindRelatedAnomalies([self._group] + duplicate_groups)
    logging.debug(
        '[GroupingDebug] Anomalies %s found for group %s and duplicates %s',
        anomalies, self._group.key, duplicate_group_keys)

    now = datetime.datetime.utcnow()
    issue = None
    canonical_group = None
    log_template = 'Group status: %s. ID: %s'
    logging.debug(log_template, self._group.status, self._group.key)
    if self._group.status in {
        self._group.Status.triaged, self._group.Status.bisected,
        self._group.Status.closed, self._group.Status.sandwiched
    }:
      project_name = self._group.bug.project or 'chromium'
      issue = perf_issue_service_client.GetIssue(
          self._group.bug.bug_id, project_name=project_name)
      if issue:
        issue['comments'] = perf_issue_service_client.GetIssueComments(
            self._group.bug.bug_id, project_name=project_name)
        canonical_group = self._FindCanonicalGroup(issue)
        logging.debug('Update.issue: %s', issue)
    return self.GroupUpdate(now, anomalies, issue, canonical_group)

  def Process(self, update=None):
    """Process the workflow.

    The workflow promises to only depend on the provided update and injected
    dependencies. The workflow steps will always be reproducible if all the
    inputs are the same.

    Process will always update the group and store once the steps have
    completed.

    The update argument can be a prepared GroupUpdate instance or None (if
    None, then Process will prepare the update itself).

    Returns the key for the associated group when the workflow was
    initialized."""

    logging.info('Processing workflow for group %s', self._group.key)
    update = update or self._PrepareGroupUpdate()
    logging.info('%d anomalies', len(update.anomalies))

    # TODO(crbug.com/1240370): understand why Datastore query may return empty
    # anomalies list.
    if (not update.anomalies and self._group.anomalies
        and self._group.group_type != alert_group.AlertGroup.Type.reserved):
      logging.error(
          'No anomalies detected. Skipping this run for %s. with anomalies %s ',
          self._group.key, self._group.anomalies)
      return self._group.key

    # Process input before we start processing the group.
    for a in update.anomalies:
      subscriptions, _ = self._sheriff_config.Match(
          a.test.string_id(), check=True)
      a.subscriptions = subscriptions
      matching_subs = [
          s for s in subscriptions if s.name == self._group.subscription_name
      ]
      a.auto_triage_enable = any(s.auto_triage_enable for s in matching_subs)
      if a.auto_triage_enable:
        logging.info('auto_triage_enable for %s due to subscription: %s',
                     a.test.string_id(),
                     [s.name for s in matching_subs if s.auto_triage_enable])

      a.auto_merge_enable = any(s.auto_merge_enable for s in matching_subs)

      if a.auto_merge_enable:
        logging.info('auto_merge_enable for %s due to subscription: %s',
                     a.test.string_id(),
                     [s.name for s in matching_subs if s.auto_merge_enable])

      a.auto_bisect_enable = any(s.auto_bisect_enable for s in matching_subs)
      a.relative_delta = (
          abs(a.absolute_delta / float(a.median_before_anomaly))
          if a.median_before_anomaly != 0. else float('Inf'))

    # anomaly.groups are updated in upload-processing. Here we update
    # the group.anomalies
    added = self._UpdateAnomalies(update.anomalies)

    if update.issue:
      group_merged = self._UpdateCanonicalGroup(update.anomalies,
                                                update.canonical_group)
      # Update the group status.
      self._UpdateStatus(update.issue)
      # Update the anomalies to associate with an issue.
      self._UpdateAnomaliesIssues(update.anomalies, update.canonical_group)

      # Current group is a duplicate.
      if self._group.canonical_group is not None:
        if group_merged:
          logging.info('Merged group %s into group %s',
                       self._group.key.string_id(),
                       update.canonical_group.key.string_id())
          self._FileDuplicatedNotification(update.canonical_group)
        self._UpdateDuplicateIssue(update.anomalies, added)
        assert (self._group.status == self._group.Status.closed), (
            'The issue is closed as duplicate (\'state\' is \'closed\'). '
            'However the groups\' status doesn\'t match the issue status')

      elif self._UpdateIssue(update.issue, update.anomalies, added):
        # Only operate on alert group if nothing updated to prevent flooding
        # monorail if some operations keep failing.
        return self._CommitGroup()

    regressions, _ = self._GetRegressions(update.anomalies)
    group = self._group
    if group.updated + self._config.active_window <= update.now:
      self._Archive()
    elif group.created + self._config.triage_delay <= update.now and (
        group.status in {group.Status.untriaged}):
      logging.info('created: %s, triage_delay: %s", now: %s, status: %s',
                   group.created, self._config.triage_delay, update.now,
                   group.status)
      self._TryTriage(update.now, update.anomalies)
    elif len(self._CheckSandwichAllowlist(regressions)) > 0 and (
        group.status == group.Status.triaged):
      logging.info('attempting sandwich verification for AlertGroup: %s',
                   self._group.key.string_id())
      sandwiched = self._TryVerifyRegression(update)
      if not sandwiched and not any(a.auto_bisect_enable
                                    for a in update.anomalies):
        self._UpdateIssue(update.issue, update.anomalies, added)
    elif group.status in {group.Status.sandwiched, group.Status.triaged}:
      self._TryBisect(update)
    return self._CommitGroup()

  def _CommitGroup(self):
    logging.debug('[GroupDebug] Group %s commited.', self._group.key)
    return self._group.put()

  def _UpdateAnomalies(self, anomalies):
    added = [a for a in anomalies if a.key not in self._group.anomalies]
    self._group.anomalies = [a.key for a in anomalies]
    logging.debug('[GroupingDebug] Group %s is associated with anomalies %s.',
                  self._group.key, self._group.anomalies)
    return added

  def _UpdateStatus(self, issue):
    if issue.get('state') == 'closed':
      self._group.status = self._group.Status.closed
    elif self._group.status == self._group.Status.closed:
      self._group.status = self._group.Status.triaged

  def _UpdateCanonicalGroup(self, anomalies, canonical_group):
    # If canonical_group is None, self._group will be separated from its'
    # canonical group. Since we only rely on _group.canonical_group for
    # determining duplicate status, setting canonical_group to None will
    # separate the groups. Anomalies that were added to the canonical group
    # during merged perios can't be removed.
    if canonical_group is None:
      self._group.canonical_group = None
      return False
    # Only merge groups if there is at least one anomaly that allows merge.
    if (self._group.canonical_group != canonical_group.key
        and any(a.auto_merge_enable for a in anomalies)):
      self._group.canonical_group = canonical_group.key
      return True
    return False

  def _UpdateAnomaliesIssues(self, anomalies, canonical_group):
    for a in anomalies:
      if not a.auto_triage_enable:
        continue
      if canonical_group is not None and a.auto_merge_enable:
        a.project_id = canonical_group.project_id
        a.bug_id = canonical_group.bug.bug_id
      elif a.bug_id is None:
        a.project_id = self._group.project_id
        a.bug_id = self._group.bug.bug_id

    # Write back bug_id to anomalies. We can't do it when anomaly is
    # found because group may be updating at the same time.
    ndb.put_multi(anomalies)

  def _UpdateIssue(self, issue, anomalies, added):
    """Update the status of the monorail issue.

    Returns True if the issue was changed.
    """
    # Check whether all the anomalies associated have been marked recovered.
    if all(a.recovered for a in anomalies if not a.is_improvement):
      if issue.get('state') == 'open':
        self._CloseBecauseRecovered()
      return True

    new_regressions, subscriptions = self._GetRegressions(added)
    all_regressions, _ = self._GetRegressions(anomalies)

    # Only update issue if there is at least one new regression
    if not new_regressions:
      return False

    closed_by_pinpoint = False
    for c in sorted(
        issue.get('comments') or [], key=lambda c: c["id"], reverse=True):
      if c.get('updates', {}).get('status') in ('WontFix', 'Fixed', 'Verified',
                                                'Invalid', 'Duplicate', 'Done'):
        closed_by_pinpoint = (
            c.get('author') in [
                self._service_account(), utils.LEGACY_SERVICE_ACCOUNT
            ])
        break

    has_new_regression = any(a.auto_bisect_enable
                             for a in anomalies
                             if not a.is_improvement and not a.recovered)

    if (issue.get('state') == 'closed' and closed_by_pinpoint
        and has_new_regression):
      self._ReopenWithNewRegressions(all_regressions, new_regressions,
                                     subscriptions)
    else:
      self._FileNormalUpdate(all_regressions, new_regressions, subscriptions)
    return True

  def _UpdateDuplicateIssue(self, anomalies, added):
    new_regressions, subscriptions = self._GetRegressions(added)
    all_regressions, _ = self._GetRegressions(anomalies)

    # Only update issue if there is at least one regression
    if not new_regressions:
      return

    self._FileNormalUpdate(
        all_regressions,
        new_regressions,
        subscriptions,
        new_regression_notification=False)

  def _CloseBecauseRecovered(self):
    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment=('All regressions for this issue have been marked '
                 'recovered; closing.'),
        status='WontFix',
        labels='Chromeperf-Auto-Closed',
        send_email=False,
    )

  def _ReopenWithNewRegressions(self, all_regressions, added, subscriptions):
    summary = _TEMPLATE_ISSUE_TITLE.render(
        self._GetTemplateArgs(all_regressions))

    template_args = self._GetTemplateArgs(added)

    skia_args = self._GetSkiaTemplateArgs(added)
    template_args.update(skia_args)

    comment = _TEMPLATE_REOPEN_COMMENT.render(template_args)
    components, cc, _ = self._ComputeBugUpdate(subscriptions, added)
    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment=comment,
        title=summary,
        components=components,
        labels=['Chromeperf-Auto-Reopened'],
        status='Unconfirmed',
        cc=cc,
        send_email=False,
    )

  def _UpdateRegressionVerification(self, execution, regression, update):
    '''Update regression verification results in monorail.

    Args:
      execution - the response from workflow_client.GetExecution()
      regression - the candidate regression that was sent for verification
      update - the alert group workflow update being processed
    Returns:
      True if the workflow completed with status of SUCCESS and
        was able to reproduce the anomaly.
      True if the worfklow completed with status of FAILED or CANCELLED.
      False for any other condtion.
    '''
    status = 'Unconfirmed'
    components = []
    proceed_with_bisect = False
    if execution['state'] == workflow_service.EXECUTION_STATE_ACTIVE:
      return proceed_with_bisect
    if execution['state']  == workflow_service.EXECUTION_STATE_SUCCEEDED:
      results_dict = json.loads(execution['result'])
      if 'decision' in results_dict:
        decision = results_dict['decision']
      else:
        raise ValueError('execution %s result is missing parameters: %s' %
                         (execution['name'], results_dict))
      logging.info(
          'Regression verification %s for project: %s and '
          'bug: %s succeeded with repro decision %s.', execution['name'],
          self._group.project_id, self._group.bug.bug_id, decision)
      if decision:
        comment = ('%s Regression verification %s job %s for test: %s\n'
                   'reproduced the regression with statistic: %s.\n'
                   'Proceed to bisection.' %
                   (_SANDWICH, execution['name'], results_dict['job_id'],
                    utils.TestPath(regression.test), results_dict['statistic']))
        label = ['Regression-Verification-Repro']
        status = 'Untriaged'
        proceed_with_bisect = True
        if (update.issue
            and utils.DELAY_REPORTING_LABEL in update.issue.get('labels')):
          # components will be added when culprit found in bisect.
          components = []
        else:
          components = list(
              self._GetComponentsFromSubscriptions(regression.subscriptions)
              # Intentionally ignoring _GetComponentsFromRegressions in this
              # case for sandwiched regressions. See
              # https://bugs.chromium.org/p/chromium/issues/detail?id=1459035
          )
      else:
        comment = ('%s Regression verification %s job %s for test: %s\n'
                   'did NOT reproduce the regression with statistic: %s.\n'
                   'Issue closed.' %
                   (_SANDWICH, execution['name'], results_dict['job_id'],
                    utils.TestPath(regression.test), results_dict['statistic']))
        label = ['Regression-Verification-No-Repro', 'Chromeperf-Auto-Closed']
        status = 'WontFix'
        self._group.updated = update.now
        self._group.status = self._group.Status.closed
        self._CommitGroup()
    elif execution['state'] == workflow_service.EXECUTION_STATE_FAILED:
      logging.error(
          'Regression verification %s for project: %s and '
          'bug: %s failed with error %s.', execution['name'],
          self._group.project_id, self._group.bug.bug_id, execution['error'])
      comment = (
          '%s Regression verification %s for test: %s\n'
          'failed. Do not proceed to bisection.' %
          (_SANDWICH, execution['name'], utils.TestPath(regression.test)))
      label = ['Regression-Verification-Failed', 'Chromeperf-Auto-Closed']
      status = 'WontFix'
      self._group.updated = update.now
      self._group.status = self._group.Status.closed
      self._CommitGroup()
    elif execution['state'] == workflow_service.EXECUTION_STATE_CANCELLED:
      logging.info(
          'Regression verification %s for project: %s and '
          'bug: %s cancelled with error %s.', execution['name'],
          self._group.project_id, self._group.bug.bug_id, execution['error'])
      comment = ('%s Regression verification %s for test: %s\n'
                 'cancelled with message %s. Proceed to bisection.' %
                 (_SANDWICH, execution['name'], utils.TestPath(
                     regression.test), execution['error']))
      label = ['Regression-Verification-Cancelled']
      proceed_with_bisect = True

    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        components=components,
        comment=comment,
        labels=label,
        status=status,
        send_email=False,
    )

    return proceed_with_bisect

  def _FileNormalUpdate(self,
                        all_regressions,
                        added,
                        subscriptions,
                        new_regression_notification=True):
    summary = _TEMPLATE_ISSUE_TITLE.render(
        self._GetTemplateArgs(all_regressions))
    comment = None
    if new_regression_notification:
      template_args = self._GetTemplateArgs(added)
      skia_args = self._GetSkiaTemplateArgs(added)
      template_args.update(skia_args)
      comment = _TEMPLATE_ISSUE_COMMENT.render(template_args)
    components, cc, labels = self._ComputeBugUpdate(subscriptions, added)
    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment=comment,
        title=summary,
        labels=labels,
        cc=cc,
        components=components,
        send_email=False,
    )

  def _FileDuplicatedNotification(self, canonical_group):
    comment = _TEMPLATE_GROUP_WAS_MERGED.render({
        'group': self._group,
        'canonical_group': canonical_group,
    })
    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment=comment,
        send_email=False,
    )

  def _GetRegressions(self, anomalies):
    regressions = []
    subscriptions_dict = {}
    for a in anomalies:
      logging.info(
          ('GetRegressions: auto_triage_enable is %s for anomaly %s due '
           'to subscription: %s'), a.auto_triage_enable, a.test.string_id(),
          [s.name for s in a.subscriptions])

      subscriptions_dict.update({s.name: s for s in a.subscriptions})
      if not a.is_improvement and not a.recovered and a.auto_triage_enable:
        regressions.append(a)
    return (regressions, list(subscriptions_dict.values()))

  @classmethod
  def _GetBenchmarksFromRegressions(cls, regressions):
    benchmarks_dict = dict()
    for regression in regressions:
      name = regression.benchmark_name
      emails = []
      info_blurb = None
      if regression.ownership:
        emails = regression.ownership.get('emails') or []
        info_blurb = regression.ownership.get('info_blurb') or ''
      benchmark = benchmarks_dict.get(
          name, cls.BenchmarkDetails(name, list(set(emails)), list(),
                                     info_blurb))
      benchmark.regressions.append(regression)
      benchmarks_dict[name] = benchmark
    return list(benchmarks_dict.values())

  def _ComputeBugUpdate(self,
                        subscriptions,
                        regressions,
                        delay_reporting=False):
    if delay_reporting:
      logging.debug('[DelayReporting] Ignoring reporting info for group %s',
                    self._group.key)
      components = [utils.DELAY_REPORTING_PLACEHOLDER]
      cc = []
      labels = [utils.DELAY_REPORTING_LABEL]
    else:
      # NOTE: Previous sandwich_allowlist checks resulted in this
      # logic ignoring _GetComponentsFromRegressions for sandwich-able
      # retressions. It no longer ignores these, so some sandwiched regressions
      # may now have components assigned due to data uploaded from benchmark
      # runners, rather than what's specified in the sheriff config.
      # NOTE 2: Regression issues should now only get components assigned in
      # three cases:
      #   - regressions are NOT sandwich-able, due to _CheckSandwichAllowlist
      #       results
      #   - regressions are sandwich-able, AND issue has the
      #       Regression-Verification-Repro label
      #   - regressions are auto_triage=True AND auto_bisect=False
      verifiable_regressions = self._CheckSandwichAllowlist(regressions)
      components = []
      if len(verifiable_regressions) == 0:
        components = set(
            self._GetComponentsFromSubscriptions(subscriptions)
            | self._GetComponentsFromRegressions(regressions))
        if len(components) != 1:
          logging.warning('Invalid component count is found for bug update: %s',
                          components)
          cloud_metric.PublishPerfIssueInvalidComponentCount(len(components))
      components = list(components)
      cc = list(set(e for s in subscriptions for e in s.bug_cc_emails))
      labels = list(set(l for s in subscriptions for l in s.bug_labels))

    if any(r for r in regressions if r.source and r.source == 'skia'):
      # If any priority is specified in the labels, let's remove it
      # since we want the skia bugs to be low priority.
      for l in labels:
        if l.startswith('Pri-'):
          labels.remove(l)
      labels.append('DoNotNotify')
      labels.append('Pri-3')

    labels.append('Chromeperf-Auto-Triaged')
    # We layer on some default labels if they don't conflict with any of the
    # provided ones.
    if not any(l.startswith('Pri-') for l in labels):
      labels.append('Pri-2')
    if not any(l.startswith('Type-') for l in labels):
      labels.append('Type-Bug-Regression')
    if any(s.visibility == subscription.VISIBILITY.INTERNAL_ONLY
           for s in subscriptions):
      labels = list(set(labels) | {'Restrict-View-Google'})
    return self.BugUpdateDetails(components, cc, labels)

  def _GetComponentsFromSubscriptions(self, subscriptions):
    components = set(c for s in subscriptions for c in s.bug_components)
    if components:
      bug_id = self._group.bug or 'New'
      subsciption_names = [s.name for s in subscriptions]
      logging.debug(
          ('Components added from subscriptions. Bug: %s, subscriptions: %s, '
           'components: %s'), bug_id, subsciption_names, components)
    return components

  def _GetComponentsFromRegressions(self, regressions):
    components = []
    for r in regressions:
      component = r.ownership and r.ownership.get('component')
      if not component:
        continue
      if isinstance(component, list) and component:
        components.append(component[0])
      elif component:
        components.append(component)
    if components:
      bug_id = self._group.bug or 'New'
      benchmarks = [r.benchmark_name for r in regressions]
      logging.debug(
          ('Components added from benchmark.Info. Bug: %s, benchmarks: %s, '
           'components: %s'), bug_id, benchmarks, components)
    return set(components)

  def _GetTemplateArgs(self, regressions):
    # Preparing template arguments used in rendering issue's title and content.
    regressions.sort(key=lambda x: x.relative_delta, reverse=True)
    benchmarks = self._GetBenchmarksFromRegressions(regressions)
    return {
        # Current AlertGroup used for rendering templates
        'group': self._group,

        # Performance regressions sorted by relative difference
        'regressions': regressions,

        # Benchmarks that occur in regressions, including names, owners, and
        # information blurbs.
        'benchmarks': benchmarks,

        # Parse the real unit (remove things like smallerIsBetter)
        'parse_unit': lambda s: (s or '').rsplit('_', 1)[0],
    }

  def _Archive(self):
    logging.debug('Archiving group: %s', self._group.key)
    self._group.active = False

  def _TryTriage(self, now, anomalies):
    bug, anomalies = self._FileIssue(anomalies)
    if not bug:
      logging.debug('[GroupingDebug] No issue created for %s.', self._group.key)
      return

    cloud_metric.PublishAutoTriagedIssue(cloud_metric.AUTO_TRIAGE_CREATED)
    # Update the issue associated with his group, before we continue.
    self._group.bug = bug
    self._group.updated = now
    self._group.status = self._group.Status.triaged
    self._CommitGroup()

    # Link the bug to auto-triage enabled anomalies.
    for a in anomalies:
      if a.bug_id is None and a.auto_triage_enable:
        a.project_id = bug.project
        a.bug_id = bug.bug_id
    ndb.put_multi(anomalies)

  def _AssignIssue(self, regression):
    commit_info = file_bug.GetCommitInfoForAlert(regression, self._crrev,
                                                 self._gitiles)
    if not commit_info:
      return False
    assert self._group.bug is not None

    file_bug.AssignBugToCLAuthor(
        self._group.bug.bug_id,
        commit_info,
        labels=['Chromeperf-Auto-Assigned'],
        project=self._group.project_id)
    return True

  def _CheckSandwichAllowlist(self, regressions):
    """Filter list of regressions against the sandwich verification
    allowlist and improvement direction.

    Args:
      regressions: A list of regressions in the anomaly group.

    Returns:
      allowed_regressions: A list of sandwich verifiable regressions.
    """
    allowed_regressions = []
    if not feature_flags.SANDWICH_VERIFICATION:
      return allowed_regressions

    for regression in regressions:
      if not isinstance(regression, anomaly.Anomaly):
        raise TypeError('%s is not anomaly.Anomaly' % type(regression))

      if (regression.auto_triage_enable and regression.auto_bisect_enable
          and sandwich_allowlist.CheckAllowlist(self._group.subscription_name,
                                                regression.benchmark_name,
                                                regression.bot_name)):
        allowed_regressions.append(regression)

    return allowed_regressions

  def _TryVerifyRegression(self, update):
    """Verify the selected regression using the sandwich verification workflow.

    Args:
      update: An alert group containing anomalies and potential regressions

    Returns:
      True or False, indicating whether or not it started a pinpoint job.
    """
    # Do not run sandwiching if anomaly subscription opts out of culprit finding
    if (update.issue
        and 'Chromeperf-Auto-BisectOptOut' in update.issue.get('labels')):
      return False

    # check if any regressions qualify for verification
    regressions, _ = self._GetRegressions(update.anomalies)
    verifiable_regressions = self._CheckSandwichAllowlist(regressions)
    regression = self._SelectAutoBisectRegression(verifiable_regressions)

    if not regression:
      return False

    start_git_hash = pinpoint_request.ResolveToGitHash(
        regression.start_revision - 1,
        regression.benchmark_name,
        crrev=self._crrev)
    end_git_hash = pinpoint_request.ResolveToGitHash(
        regression.end_revision, regression.benchmark_name, crrev=self._crrev)

    _, chart, _ = utils.ParseTelemetryMetricParts(
        regression.test.get().test_path)
    chart, statistic = utils.ParseStatisticNameFromChart(chart)

    improvement_dir = self._GetImprovementDirection(regression)
    logging.debug('Alert Group Workflow Debug - got improvement_direction: %s',
                  improvement_dir)
    create_exectution_req = {
        'benchmark':
            regression.benchmark_name,
        'bot_name':
            regression.bot_name,
        'story':
            regression.test.get().unescaped_story_name,
        'measurement':
            chart,
        'target':
            pinpoint_request.GetIsolateTarget(regression.bot_name,
                                              regression.benchmark_name),
        'start_git_hash':
            start_git_hash,
        'end_git_hash':
            end_git_hash,
        'project':
            self._group.project_id,
        'improvement_dir':
            improvement_dir,
    }
    # Simultaneously trigger culprit finder (aka sandwich verification) in skia.
    # We currently ignore the result, just to collect data.
    try:
      skia_pp_req = pinpoint_service.UpdateSkiaCulpritFinderRequest(
          create_exectution_req, regression, self._group.bug.bug_id, statistic)
      results = self._pinpoint.NewJobInSkia(skia_pp_req)
      logging.info('[Pinpoint Skia] Triggering %s', results)
    except Exception as e:  # pylint: disable=broad-except
      # Caught all exceptions as we only need to trigger and log the runs.
      msg = '[Pinpoint Skia] Error on triggering: %s\n%s'
      logging.warning(msg, create_exectution_req, e)

    logging.debug(
        ('Alert Group Workflow Debug - creating verification workflow with '
         'request: %s'), create_exectution_req)

    try:
      sandwich_execution_id = self._cloud_workflows.CreateExecution(
          create_exectution_req)
      self._group.sandwich_verification_workflow_id = sandwich_execution_id

      self._group.status = self._group.Status.sandwiched
      logging.info('sandwich_execution_id: %s', sandwich_execution_id)

      self._group.updated = update.now

      self._CommitGroup()
      perf_issue_service_client.PostIssueComment(
          self._group.bug.bug_id,
          self._group.project_id,
          comment=_TEMPLATE_AUTO_REGRESSION_VERIFICATION_COMMENT.render({
              'test': utils.TestPath(regression.test),
              'verification_workflow_id': sandwich_execution_id
          }),
          # Do not set labels yet on this issue, since we're only starting
          # the sandwich verification to try and repro the regression before
          # we alert any humans to the situation.
          send_email=False,
      )

      return True
    except request.NotFoundError:
      return False
    except request.RequestError:
      return False

  def _GetImprovementDirection(self, regression):
    if regression is None:
      return 'UNKNOWN'

    test_path = utils.TestPath(regression.test)
    if test_path is None:
      return 'UNKNOWN'
    logging.debug('Alert Group Workflow Debug - got test_path: %s', test_path)

    t = graph_data.TestMetadata.get_by_id(test_path)
    if t is not None:
      logging.debug(
          'Alert Group Workflow Debug - got improvement_direction: %s',
          t.improvement_direction)
      if t.improvement_direction == anomaly.UP:
        return 'UP'
      if t.improvement_direction == anomaly.DOWN:
        return 'DOWN'
    return 'UNKNOWN'

  def _TryBisect(self, update):
    if (update.issue
        and 'Chromeperf-Auto-BisectOptOut' in update.issue.get('labels')):
      return

    try:
      regressions, _ = self._GetRegressions(update.anomalies)
      regression = self._SelectAutoBisectRegression(regressions)

      # Do nothing if none of the regressions should be auto-bisected.
      if regression is None:
        return

      if self._group.status == self._group.Status.sandwiched:
        # Get the verdict from the workflow execution results.
        try:
          sandwich_exec = self._cloud_workflows.GetExecution(
              self._group.sandwich_verification_workflow_id)
        except request.NotFoundError:
          logging.error(
              ('cloud workflow service could not find sandwich verification '
               'execution ID %s'),
              self._group.sandwich_verification_workflow_id)
          return

        proceed_with_bisect = self._UpdateRegressionVerification(
            sandwich_exec, regression, update)
        if not proceed_with_bisect:
          return

    # We'll only bisect a range if the range at least one point.
      if regression.start_revision == regression.end_revision:
        # At this point we've decided that the range of the commits is a single
        # point, so we don't bother bisecting.
        if not self._AssignIssue(regression):
          self._UpdateWithBisectError(
              update.now, 'Cannot find assignee for regression at %s.' %
              (regression.end_revision,))
        else:
          self._group.updated = update.now
          self._group.status = self._group.Status.bisected
          self._CommitGroup()
        return

      job_id = self._StartPinpointBisectJob(regression)
      cloud_metric.PublishAutoTriagedIssue(cloud_metric.AUTO_TRIAGE_BISECTED)
    except InvalidPinpointRequest as error:
      self._UpdateWithBisectError(update.now, error)
      return

    # Update the issue associated with his group, before we continue.
    self._group.bisection_ids.append(job_id)
    self._group.updated = update.now
    self._group.status = self._group.Status.bisected
    self._CommitGroup()
    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment=_TEMPLATE_AUTO_BISECT_COMMENT.render(
            {'test': utils.TestPath(regression.test)}),
        labels=['Chromeperf-Auto-Bisected'],
        send_email=False,
    )

    regression.pinpoint_bisects.append(job_id)
    regression.put()

  def _GetSkiaTemplateArgs(self, regressions):
    """Return skia template args to append to the existing template args"""
    masters = set()
    for r in regressions:
      if r.master_name:
        masters.add(r.master_name)

    template_args = {}
    try:
      # Add the public url only if at least one of the anomalies in the group
      # are public
      if any(not r.test.get().internal_only for r in regressions):
        skia_urls_public = skia_helper.GetSkiaUrlsForAlertGroup(
            self._group.key.string_id(), False, list(masters))
        template_args['skia_urls_text_public'] = skia_urls_public
    except Exception:  #pylint: disable=broad-except
      template_args['skia_urls_text_public'] = None
    try:
      skia_urls_internal = skia_helper.GetSkiaUrlsForAlertGroup(
          self._group.key.string_id(), True, list(masters))
      template_args['skia_urls_text_internal'] = skia_urls_internal
    except Exception:  # pylint: disable=broad-except
      template_args['skia_urls_text_internal'] = None
    return template_args

  def _FileIssue(self, anomalies):
    regressions, subscriptions = self._GetRegressions(anomalies)
    # Only file a issue if there is at least one regression
    # We can't use subsciptions' auto_triage_enable here because it's
    # merged across anomalies.
    if not any(r.auto_triage_enable for r in regressions):
      return None, []

    auto_triage_regressions = []
    for r in regressions:
      if r.auto_triage_enable:
        auto_triage_regressions.append(r)

    logging.info('auto_triage_enabled due to %s', auto_triage_regressions)
    template_args = self._GetTemplateArgs(regressions)
    top_regression = template_args['regressions'][0]
    template_args['revision_infos'] = self._revision_info.GetRangeRevisionInfo(
        top_regression.test,
        top_regression.start_revision,
        top_regression.end_revision,
    )

    skia_args = self._GetSkiaTemplateArgs(regressions)
    template_args.update(skia_args)

    # Rendering issue's title and content
    title = _TEMPLATE_ISSUE_TITLE.render(template_args)
    description = _TEMPLATE_ISSUE_CONTENT.render(template_args)

    # DelayReporting: we decided to delay the reporting until we find a root
    # cause if any. The reason is: the alert group can be a false positive and
    # thus the issue created is a cannot-reproduce.
    # To delay the reporting, the fields which trigger notifications (emails)
    # will be removed when the issue is created. Those fields will be updated
    # when bisection finds a culprit. A label 'Chromeperf-Delay-Reporting' is
    # added to tell Pinpoint to do the reporting.
    # If auto-bisect is not enabled, we will still do reporting because we
    # cannot rely on manual bisects to add those inf.

    # NOTICE that we do not have benchmark class in Pinpoint workflow and thus
    # do not have the component info from the @benchmark.info. E.g.:
    # https://source.chromium.org/chromium/chromium/src/+/main:
    # tools/perf/benchmarks/jetstream2.py;l=44
    # We will only add component, cc and labels based on the settings from
    # the Sheriff Config.

    should_delay_reporting = utils.ShouldDelayIssueReporting()
    if should_delay_reporting:
      should_bisect = any(
          r.auto_bisect_enable and not r.is_improvement for r in regressions)
      logging.debug('[DelayReporting] should_bisect %s for group %s.',
                    should_bisect, self._group.key)
      should_delay_reporting = should_delay_reporting and should_bisect
    # Fetching issue labels, components and cc from subscriptions and owner
    components, cc, labels = self._ComputeBugUpdate(subscriptions, regressions,
                                                    should_delay_reporting)
    logging.info('Creating a new issue for AlertGroup %s', self._group.key)

    response = perf_issue_service_client.PostIssue(
        title=title,
        description=description,
        labels=labels,
        components=components,
        cc=cc,
        project=self._group.project_id)
    logging.debug('[GroupingDebug] PostIssue response for %s: %s',
                  self._group.key, response)
    if 'error' in response:
      logging.warning('AlertGroup %s file bug failed: %s', self._group.key,
                      response['error'])
      return None, []

    # Update the issue associated witht his group, before we continue.
    return alert_group.BugInfo(
        project=self._group.project_id,
        bug_id=response['issue_id'],
    ), anomalies

  def _StartPinpointBisectJob(self, regression):
    pp_request = None
    try:
      pp_request = self._NewPinpointRequest(regression)
    except pinpoint_request.InvalidParamsError as e:
      six.raise_from(
          InvalidPinpointRequest('Invalid pinpoint request: %s' % (e,)), e)
    try:
      results = self._pinpoint.NewJob(pp_request)
    except pinpoint_request.InvalidParamsError as e:
      six.raise_from(
          InvalidPinpointRequest('Invalid pinpoint request: %s' % (e,)), e)

    if 'jobId' not in results:
      raise InvalidPinpointRequest('Start pinpoint bisection failed: %s' %
                                   (results,))

    return results.get('jobId')

  def _SelectAutoBisectRegression(self, regressions):
    # Select valid regressions for bisection:
    # 1. auto_bisect_enable
    # 2. has a valid bug_id
    # 3. hasn't start a bisection
    # 4. is not a summary metric (has story)
    filtered_regressions = []
    for r in regressions:
      if not r.bug_id:
        logging.error('No bug_id found in anomaly %s', r.key.id())
        continue
      if (r.auto_bisect_enable and r.bug_id > 0
          and not set(r.pinpoint_bisects) & set(self._group.bisection_ids)
          and r.test.get().unescaped_story_name):
        filtered_regressions.append(r)
    regressions = filtered_regressions

    if not regressions:
      return None

    max_regression = None
    max_count = 0

    scores = ndb.get_multi(
        ndb.Key(
            'SignalQuality',
            utils.TestPath(r.test),
            'SignalQualityScore',
            '0',
        ) for r in regressions)
    scores_dict = {s.key.parent().string_id(): s.score for s in scores if s}

    def MaxRegression(x, y):
      if x is None or y is None:
        return x or y

      get_score = lambda a: scores_dict.get(
          utils.TestPath(a.test),
          _ALERT_GROUP_DEFAULT_SIGNAL_QUALITY_SCORE)

      if x.relative_delta == float('Inf'):
        if y.relative_delta == float('Inf'):
          return max(x, y, key=lambda a: (get_score(a), a.absolute_delta))
        return y
      if y.relative_delta == float('Inf'):
        return x
      return max(x, y, key=lambda a: (get_score(a), a.relative_delta))

    bot_name = lambda r: r.bot_name
    for _, rs in itertools.groupby(
        sorted(regressions, key=bot_name), key=bot_name):
      count = 0
      group_max = None
      for r in rs:
        count += 1
        group_max = MaxRegression(group_max, r)
      if count >= max_count:
        max_count = count
        max_regression = MaxRegression(max_regression, group_max)
    return max_regression

  def _NewPinpointRequest(self, alert):
    start_git_hash = pinpoint_request.ResolveToGitHash(
        alert.start_revision - 1, alert.benchmark_name, crrev=self._crrev)
    end_git_hash = pinpoint_request.ResolveToGitHash(
        alert.end_revision, alert.benchmark_name, crrev=self._crrev)
    logging.info(
        """
        Making new pinpoint request. Alert start revision: %s; end revision: %s.
         Pinpoint start hash (one position back): %s, end hash: %s'
         """, alert.start_revision, alert.end_revision, start_git_hash,
        end_git_hash)

    # Pinpoint also requires you specify which isolate target to run the
    # test, so we derive that from the suite name. Eventually, this would
    # ideally be stored in a SparseDiagnostic but for now we can guess. Also,
    # Pinpoint only currently works well with Telemetry targets, so we only run
    # benchmarks that are not explicitly denylisted.
    target = pinpoint_request.GetIsolateTarget(alert.bot_name,
                                               alert.benchmark_name)
    if not target:
      return None

    tags = {
        'test_path': utils.TestPath(alert.test),
        'alert': six.ensure_str(alert.key.urlsafe()),
        'auto_bisection': 'true',
    }
    if alert.source and alert.source == 'skia':
      alert_magnitude = None
      # Adding the tag below to distinguish jobs created for skia regressions.
      # This should help us measure the culprit detection rate separately for
      # skia regressions.
      tags['source'] = 'skia'
      job_name = '[Skia] Auto-Bisection on %s/%s' % (alert.bot_name,
                                            alert.benchmark_name)
    else:
      alert_magnitude = alert.median_after_anomaly - alert.median_before_anomaly
      job_name = 'Auto-Bisection on %s/%s' % (alert.bot_name,
                                            alert.benchmark_name)

    if self._group.status == self._group.Status.sandwiched:
      tags['sandwiched'] = 'true'
    return pinpoint_service.MakeBisectionRequest(
        test=alert.test.get(),
        commit_range=pinpoint_service.CommitRange(
            start=start_git_hash, end=end_git_hash),
        issue=anomaly.Issue(
            project_id=self._group.bug.project,
            issue_id=self._group.bug.bug_id,
        ),
        comparison_mode='performance',
        target=target,
        comparison_magnitude=alert_magnitude,
        name=job_name,
        priority=10,
        tags=tags,
    )

  def _UpdateWithBisectError(self, now, error, labels=None):
    self._group.updated = now
    self._group.status = self._group.Status.bisected
    self._CommitGroup()

    perf_issue_service_client.PostIssueComment(
        self._group.bug.bug_id,
        self._group.project_id,
        comment='Auto-Bisection failed with the following message:\n\n'
        '%s\n\nNot retrying' % (error,),
        labels=labels if labels else ['Chromeperf-Auto-NeedsAttention'],
    )
