# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Logic for posting Job updates to the issue tracker."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import collections
import jinja2
import logging
import math
import os.path

from dashboard import update_bug_with_results
from dashboard import sheriff_config_client
from dashboard.common import cloud_metric
from dashboard.common import utils
from dashboard.models import histogram
from dashboard.models import anomaly
from dashboard.services import perf_issue_service_client
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models.change import commit as commit_module
from dashboard.pinpoint.models.change import patch as patch_module

from tracing.value.diagnostics import reserved_infos

_INFINITY = u'\u221e'
_RIGHT_ARROW = u'\u2192'

_TEMPLATE_ENV = jinja2.Environment(
    loader=jinja2.FileSystemLoader(
        searchpath=os.path.join(
            os.path.dirname(os.path.realpath(__file__)), 'templates')))
_DIFFERENCES_FOUND_TMPL = _TEMPLATE_ENV.get_template('differences_found.j2')
_CREATED_TEMPL = _TEMPLATE_ENV.get_template('job_created.j2')
_MISSING_VALUES_TMPL = _TEMPLATE_ENV.get_template('missing_values.j2')

_LABEL_EXCLUSION_SETS = [
    {
        'Pinpoint-Job-Started',
        'Pinpoint-Job-Completed',
        'Pinpoint-Job-Failed',
        'Pinpoint-Job-Pending',
        'Pinpoint-Job-Cancelled',
        'Culprit-Sandwich-Verification-Started',
    },
    {
        'Pinpoint-Culprit-Found',
        'Pinpoint-No-Repro',
        'Pinpoint-Multiple-Culprits',
        'Pinpoint-Multiple-MissingValues',
        'Culprit-Verification-No-Repro',
    },
]


def ComputeLabelUpdates(labels):
  """Builds a set of label updates for known labels applied by Pinpoint."""
  # Validate that the labels aren't found in the same sets.
  label_updates = set()
  for label in labels:
    found_in_sets = sum(
        label in label_set for label_set in _LABEL_EXCLUSION_SETS)
    if found_in_sets > 1:
      raise ValueError(
          'label "%s" is found in %s label sets' % (label, found_in_sets),)

  for label_set in _LABEL_EXCLUSION_SETS:
    label_updates |= set('-' + l for l in label_set)
  label_updates -= set('-' + l for l in labels)
  label_updates |= set(labels)
  return list(label_updates)


class JobUpdateBuilder:
  """Builder for job issue updates.

  The builder lets us collect the useful information for filing an update on an
  issue, ranging from when a job is created, updated, and when it fails.

  Intended usage looks like::

    builder = JobUpdateBuilder(...)
    issue_update_info = builder.CreationUpdate()

  In cases where we encounter a failure::

    builder.AddFailure(...)
    issue_update_info = builder.FailureUpdate()
  """

  def __init__(self, job):
    self._env = {
        'url': job.url,
        'user': job.user,
        'args': job.benchmark_arguments,
        'configuration': job.configuration if job.configuration else '(None)',
    }

  def CreationUpdate(self, pending):
    env = self._env.copy()
    env.update({'pending': pending})
    comment_text = _CREATED_TEMPL.render(**env)
    labels = ComputeLabelUpdates(['Pinpoint-Job-Pending'])
    return _BugUpdateInfo(comment_text, None, None, labels, None)


class DifferencesFoundBugUpdateBuilder:
  """Builder for bug updates about differences found in a metric.

  Accumulate the found differences into this with AddDifference(), then call
  BuildUpdate() to get the bug update to send.

  So intended usage looks like::

    builder = DifferencesFoundBugUpdateBuilder()
    for d in FindDifferences():
      ...
      builder.AddDifference(d.commit, d.values_a, d.values_b)
    issue_update_info = builder.BuildUpdate(tags, url)
  """

  def __init__(self, metric):
    self._metric = metric
    self._differences = []
    self._examined_count = None
    self._cached_ordered_diffs_by_delta = None
    self._cached_commits_with_no_values = None

  def SetExaminedCount(self, examined_count):
    self._examined_count = examined_count

  def AddDifference(self,
                    change,
                    values_a,
                    values_b,
                    kind=None,
                    commit_dict=None):
    """Add a difference (a commit where the metric changed significantly).

    Args:
      change: a Change.
      values_a: (list) result values for the prior commit.
      values_b: (list) result values for this commit.
      kind: commit kind.
      commit_dict: commit dictionary.
    """
    if not kind and not commit_dict:
      if change.patch:
        kind = 'patch'
        commit_dict = {
            'server': change.patch.server,
            'change': change.patch.change,
            'revision': change.patch.revision,
        }
      else:
        kind = 'commit'
        commit_dict = {
            'repository': change.last_commit.repository,
            'git_hash': change.last_commit.git_hash,
        }

    # Store just the commit repository + hash to ensure we don't attempt to
    # serialize too much data into datastore.  See https://crbug.com/1140309.
    self._differences.append(_Difference(kind, commit_dict, values_a, values_b))
    self._cached_ordered_diffs_by_delta = None

  def BuildUpdate(self, tags, url, improvement_dir, sandwiched=False):
    """Return _BugUpdateInfo for the differences."""
    if len(self._differences) == 0:
      raise ValueError("BuildUpdate called with 0 differences")
    differences = self._OrderedDifferencesByDelta(improvement_dir)
    missing_values = self._DifferencesWithNoValues()
    owner, cc_list, notify_why_text = self._PeopleToNotify(improvement_dir)
    status = None

    # Here we're only going to consider the cases where we find differences
    # that have non-empty values, to consider whether we've found no, a single,
    # or multiple culprits.
    if differences:
      if sandwiched:
        labels = [
            'Pinpoint-Culprit-Found'
            if len(differences) == 1 else 'Pinpoint-Multiple-Culprits',
            'Culprit-Verification-Completed',
        ]
      else:
        labels = [
            'Pinpoint-Culprit-Found'
            if len(differences) == 1 else 'Pinpoint-Multiple-Culprits',
            'Pinpoint-Job-Completed',
        ]
      if missing_values:
        labels.append('Pinpoint-Multiple-MissingValues')
      labels = ComputeLabelUpdates(labels)
      status = 'Assigned'
      comment_text = _DIFFERENCES_FOUND_TMPL.render(
          differences=differences,
          url=url,
          metric=self._metric,
          notify_why_text=notify_why_text,
          doc_links=_FormatDocumentationUrls(tags),
          examined_count=self._examined_count,
          missing_values=missing_values,
      )
      if tags and tags.get('auto_bisection') == 'true':
        cloud_metric.PublishAutoTriagedIssue(
            cloud_metric.AUTO_TRIAGE_CULPRIT_FOUND)
    elif missing_values:
      status = 'Assigned'
      if sandwiched:
        labels = ComputeLabelUpdates([
            'Pinpoint-Multiple-MissingValues', 'Culprit-Verification-Completed'
        ])
      else:
        labels = ComputeLabelUpdates(
            ['Pinpoint-Multiple-MissingValues', 'Pinpoint-Job-Completed'])
      comment_text = _MISSING_VALUES_TMPL.render(
          missing_values=missing_values,
          metric=self._metric,
          url=url,
      )

    return _BugUpdateInfo(comment_text, owner, cc_list, labels, status)

  def GetCommits(self):
    commits = [
        commit_module.Commit(**diff.commit_dict)
        for diff in self._differences
        if diff.commit_kind == 'commit'
    ]
    logging.debug('[GroupingQuality] %s commits are loaded', len(commits))
    return commits

  def GenerateCommitCacheKey(self):
    commit_cache_key = None
    if len(self._differences) == 1:
      commit_cache_key = update_bug_with_results._GetCommitHashCacheKey(
          self._differences[0].commit_info.get('git_hash'))
    return commit_cache_key

  def _OrderedDifferencesByDelta(self, improvement_dir):
    """Return the list of differences sorted by absolute change."""
    if self._cached_ordered_diffs_by_delta is not None:
      return self._cached_ordered_diffs_by_delta

    diffs_with_deltas = [(diff.MeanDelta(), diff)
                         for diff in self._differences
                         if diff.values_a and diff.values_b]
    if improvement_dir == anomaly.UP:
      # improvement is positive, regression is negative
      ordered_diffs = [
          diff for _, diff in sorted(diffs_with_deltas, key=lambda i: i[0])
      ]
    elif improvement_dir == anomaly.DOWN:
      ordered_diffs = [
          diff for _, diff in sorted(
              diffs_with_deltas, key=lambda i: i[0], reverse=True)
      ]
    else:
      ordered_diffs = [
          diff for _, diff in sorted(
              diffs_with_deltas, key=lambda i: abs(i[0]), reverse=True)
      ]
    self._cached_ordered_diffs_by_delta = ordered_diffs
    return ordered_diffs

  def _DifferencesWithNoValues(self):
    """Return the list of differences where one side has no values."""
    if self._cached_commits_with_no_values is not None:
      return self._cached_commits_with_no_values

    self._cached_commits_with_no_values = [
        diff for diff in self._differences
        if not (diff.values_a and diff.values_b)
    ]
    return self._cached_commits_with_no_values

  def _PeopleToNotify(self, improvement_dir):
    """Return the people to notify for these differences.

    This looks at the top commits (by absolute change), and returns a tuple of:
      * owner (str, will be ignored if the bug is already assigned)
      * cc_list (list, authors of the top 2 commits)
      * why_text (str, text explaining why this owner was chosen)
    """
    ordered_commits = [
        diff.commit_info
        for diff in self._OrderedDifferencesByDelta(improvement_dir)
    ] + [diff.commit_info for diff in self._DifferencesWithNoValues()]

    # CC the folks in the top N commits.  N is scaled by the number of commits
    # (fewer than 10 means N=1, fewer than 100 means N=2, etc.)
    commits_cap = int(math.floor(math.log10(len(ordered_commits)))) + 1
    cc_list = set()
    for commit in ordered_commits[:commits_cap]:
      cc_list.add(commit['author'])

    # Assign to the author of the top commit.  If that is an autoroll, assign to
    # a sheriff instead.
    why_text = ''
    top_commit = ordered_commits[0]
    owner = top_commit['author']
    sheriff = utils.GetSheriffForAutorollCommit(owner, top_commit['message'])
    if sheriff:
      owner = sheriff
      why_text = 'Assigning to sheriff %s because "%s" is a roll.' % (
          sheriff, top_commit['subject'])

    return owner, cc_list, why_text


class _Difference:

  # Define this as a class attribute so that accessing it never fails with
  # AttributeError, even if working with a serialized version of _Difference
  # that didn't define them.
  _cached_commit = None

  def __init__(self, commit_kind, commit_dict, values_a, values_b):
    self.commit_kind = commit_kind
    self.commit_dict = commit_dict
    self.values_a = values_a
    self.values_b = values_b

  @property
  def commit_info(self):
    if 'commit_info' in self.__dict__:
      # Older versions of this object had this value serialized, so just use
      # that.
      # TODO: Delete this code path once we're sure all old deferred calls using
      # this have expired.
      return self.__dict__['commit_info']
    if self._cached_commit is None:
      if self.commit_kind == 'patch':
        commit = patch_module.GerritPatch(**self.commit_dict)
      elif self.commit_kind == 'commit':
        commit = commit_module.Commit(**self.commit_dict)
      else:
        assert False, "Unexpectied commit_kind: " + str(self.commit_kind)
      self._cached_commit = commit.AsDict()

    return self._cached_commit

  def MeanDelta(self):
    return job_state.Mean(self.values_b) - job_state.Mean(self.values_a)

  def Formatted(self):
    if self.values_a:
      mean_a = job_state.Mean(self.values_a)
      formatted_a = '%.4g' % mean_a
    else:
      mean_a = None
      formatted_a = 'No values'

    if self.values_b:
      mean_b = job_state.Mean(self.values_b)
      formatted_b = '%.4g' % mean_b
    else:
      mean_b = None
      formatted_b = 'No values'

    difference = ''
    if self.values_a and self.values_b:
      difference = ' (%+.4g)' % (mean_b - mean_a)
      if mean_a:
        difference += ' (%+.4g%%)' % ((mean_b - mean_a) / mean_a * 100)
      else:
        difference += ' (+%s%%)' % _INFINITY
    return '%s %s %s%s' % (formatted_a, _RIGHT_ARROW, formatted_b, difference)


class _BugUpdateInfo(
    collections.namedtuple('_BugUpdateInfo', [
        'comment_text',
        'owner',
        'cc_list',
        'labels',
        'status',
    ])):
  """An update to post to a bug.

  This is the return type of DifferencesFoundBugUpdateBuilder.BuildUpdate.
  """


def _ComputePostMergeDetails(commit_cache_key, cc_list):
  merge_details = {}
  if commit_cache_key:
    merge_details = update_bug_with_results.GetMergeIssueDetails(
        commit_cache_key)
    if merge_details['id']:
      cc_list = set()
  return merge_details, cc_list


def _GetBugData(bug_id, project='chromium'):
  if not bug_id:
    return None

  issue_data = perf_issue_service_client.GetIssue(bug_id, project_name=project)
  if not issue_data:
    return None

  return issue_data


def _FormatDocumentationUrls(tags):
  if not tags:
    return ''

  # TODO(simonhatch): Tags isn't the best way to get at this, but wait until
  # we move this back into the dashboard so we have a better way of getting
  # at the test path.
  # crbug.com/876899
  test_path = tags.get('test_path')
  if not test_path:
    return ''

  test_suite = utils.TestKey('/'.join(test_path.split('/')[:3]))
  docs = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
      test_suite, [reserved_infos.DOCUMENTATION_URLS.name])

  if not docs:
    return ''

  docs = docs[reserved_infos.DOCUMENTATION_URLS.name].get('values')
  footer = '\n\n%s:\n  %s' % (docs[0][0], docs[0][1])
  return footer


def _ComputeAutobisectUpdate(tags):
  if tags.get('auto_bisection') != 'true':
    return None

  test_key = tags.get('test_path')
  if not test_key:
    return None

  client = sheriff_config_client.GetSheriffConfigClient()
  matched_configs, _ = client.Match(test_key, check=True)

  logging.debug('[DelayReporting] matched config: %s', matched_configs)

  components, ccs, labels = set(), set(), set()

  for config in matched_configs:
    if config.bug_components:
      components.update(config.bug_components)
    if config.bug_cc_emails:
      ccs.update(config.bug_cc_emails)
    if config.bug_labels:
      labels.update(config.bug_labels)

  return list(components), list(ccs), list(labels)


def GetAlertGroupingQuality(bug_update_builder, tags, url):
  if not tags or tags.get('auto_bisection') != 'true':
    logging.debug(
      '[GroupingQuality] Skipping for non-auto-bisection. Job tags: %s', tags)
    return
  job_id = url.split('/')[-1]
  for commit in bug_update_builder.GetCommits():
    resp = perf_issue_service_client.GetAlertGroupQuality(job_id, commit)
    logging.debug('[GroupingQuality] Grouping quality result: %s.', resp)


def UpdatePostAndMergeDeferred(bug_update_builder,
                               bug_id,
                               tags,
                               url,
                               project,
                               improvement_dir,
                               sandwiched=False):
  if not bug_id:
    return
  commit_cache_key = bug_update_builder.GenerateCommitCacheKey()
  if not commit_cache_key:
    logging.debug('UpdatePostAndMergeDeferred: commit_cache_key is None. Bug: "%s"',
                  bug_id)
  bug_update = bug_update_builder.BuildUpdate(tags, url, improvement_dir,
                                              sandwiched)
  merge_details, cc_list = _ComputePostMergeDetails(
      commit_cache_key,
      bug_update.cc_list,
  )

  bug_data = _GetBugData(bug_id, project)
  if not bug_data:
    return
  owner, current_bug_status = bug_data.get('owner'), bug_data.get('status')
  if not current_bug_status:
    return

  status = None
  bug_owner = None

  if current_bug_status in ['Untriaged', 'Unconfirmed', 'Available']:
    # Set the bug status and owner if this bug is opened and unowned.
    status = bug_update.status
    bug_owner = bug_update.owner
  elif current_bug_status == 'Assigned':
    # Always set the owner, and move the current owner to CC.
    bug_owner = bug_update.owner
    if owner:
      logging.debug(
          'Current owner for issue %s:%s = %s',
          project,
          bug_id,
          owner,
      )
      cc_list.add(owner.get('email', ''))

  current_bug_labels = bug_data.get('labels', [])

  # If the label 'Chromeperf-Delay-Reporting' exists in the filed issue,
  # the issue is not reported to end users yet.
  # We need to report by adding the following info from the subscription:
  # component, cc list and labels.
  components = []
  labels = bug_update.labels
  try:
    if utils.DELAY_REPORTING_LABEL in current_bug_labels:
      logging.debug('[DelayReporting] Job tags: %s', tags)
      auto_bisect_updates = _ComputeAutobisectUpdate(tags)

      if not auto_bisect_updates:
        logging.warning(
            '[DelayReporting] Missing info needed for delayed reporting.')
      else:
        components = auto_bisect_updates[0]
        new_cc_list = auto_bisect_updates[1]
        new_labels = auto_bisect_updates[2]
        # Speed>Regressions is a place holder when creating an unreported issue.
        components.append('-Speed>Regressions')

        logging.info(
            '[DelayReporting] Issue ID: %s. Components: %s, CC: %s, Labels: %s.',
            bug_id, components, new_cc_list, new_labels)

        cc_list.update(set(new_cc_list))
        labels_set = set(labels)
        labels_set.update(set(new_labels))
        labels = list(labels_set)

  except Exception as e:  # pylint: disable=broad-except
    logging.warning(
        '[DelayReporting] Failed to compute auto bisect info. Bug ID: %s. %s',
        bug_id, str(e))

  if len(current_bug_labels) > 0 and 'DoNotNotify' in current_bug_labels:
    logging.info(
        '[DoNotNotify] Removing owner: %s, cc_list: %s and components: %s '
        'for bug_id: %s in project: %s', bug_owner, cc_list, components, bug_id,
        project)
    bug_owner = ''
    cc_list = set()
    components = []
    # We cannot have "Assigned" status with no owner.
    if status == 'Assigned':
      status = 'Available'

  # Get Alert Grouping Quality for auto-bisects
  GetAlertGroupingQuality(
      bug_update_builder=bug_update_builder, tags=tags, url=url)

  perf_issue_service_client.PostIssueComment(
      issue_id=bug_id,
      project_name=project,
      comment=bug_update.comment_text,
      status=status,
      cc=sorted(cc_list),
      components=components,
      owner=bug_owner,
      labels=labels,
      merge_issue=merge_details.get('id'))
  update_bug_with_results.UpdateMergeIssue(
      commit_cache_key, merge_details, bug_id, project=project)
