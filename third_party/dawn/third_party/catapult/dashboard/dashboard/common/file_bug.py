# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Consolidates the utilities for bug filing."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging
import re

from google.appengine.api import app_identity
from google.appengine.api import urlfetch
from google.appengine.ext import ndb

from dashboard import auto_bisect
from dashboard import short_uri
from dashboard.common import namespaced_stored_object
from dashboard.common import utils
from dashboard.models import bug_data
from dashboard.models import bug_label_patterns
from dashboard.models import histogram
from dashboard.services import crrev_service
from dashboard.services import gitiles_service
from dashboard.services import perf_issue_service_client
from tracing.value.diagnostics import reserved_infos

# A list of bug labels to suggest for all performance regression bugs.
_DEFAULT_LABELS = [
    'Type-Bug-Regression',
    'Pri-2',
]
_OMAHA_PROXY_URL = 'https://omahaproxy.appspot.com/all.json'


def _GetDocsForTest(test):
  test_suite = utils.TestKey('/'.join(test.id().split('/')[:3]))

  docs = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
      test_suite, [reserved_infos.DOCUMENTATION_URLS.name])

  if not docs:
    return None

  docs = docs[reserved_infos.DOCUMENTATION_URLS.name].get('values')
  return docs[0]


def _AdditionalDetails(bug_id, project_id, alerts):
  """Returns a message with additional information to add to a bug."""
  base_url = '%s/group_report' % _GetServerURL()
  bug_page_url = '%s?bug_id=%s&project_id=%s' % (base_url, bug_id, project_id)
  alert_keys = utils.ConvertBytesBeforeJsonDumps(_UrlsafeKeys(alerts))
  sid = short_uri.GetOrCreatePageState(json.dumps(alert_keys))
  alerts_url = '%s?sid=%s' % (base_url, sid)
  comment = '<b>All graphs for this bug:</b>\n  %s\n\n' % bug_page_url
  comment += (
      '(For debugging:) Original alerts at time of bug-filing:\n  %s\n' %
      alerts_url)
  bot_names = {a.bot_name for a in alerts}
  if bot_names:
    comment += '\n\nBot(s) for this bug\'s original alert(s):\n\n'
    comment += '\n'.join(sorted(bot_names))
  else:
    comment += '\nCould not extract bot names from the list of alerts.'

  docs_by_suite = {}
  for a in alerts:
    test = a.GetTestMetadataKey()

    suite = test.id().split('/')[2]
    if suite in docs_by_suite:
      continue

    docs = _GetDocsForTest(test)
    if not docs:
      continue

    docs_by_suite[suite] = docs

  for k, v in docs_by_suite.items():
    comment += '\n\n%s - %s:\n  %s' % (k, v[0], v[1])

  return comment


def _GetServerURL():
  return 'https://' + app_identity.get_default_version_hostname()


def _UrlsafeKeys(alerts):
  return [a.key.urlsafe() for a in alerts]


def _ComponentFromCrLabel(label):
  return label.replace('Cr-', '').replace('-', '>')


def FetchLabelsAndComponents(alert_keys):
  """Fetches a list of bug labels and components for the given Alert keys."""
  alerts = ndb.get_multi(alert_keys)
  subscriptions = [s for alert in alerts for s in alert.subscriptions]
  tags = set(l for s in subscriptions for l in s.bug_labels)
  bug_labels = set(_DEFAULT_LABELS)
  bug_components = set(c for s in subscriptions for c in s.bug_components)
  for tag in tags:
    if tag.startswith('Cr-'):
      bug_components.add(_ComponentFromCrLabel(tag))
    else:
      bug_labels.add(tag)
  if any(a.internal_only for a in alerts):
    # This is a Chrome-specific behavior, and should ideally be made
    # more general (maybe there should be a list in datastore of bug
    # labels to add for internal bugs).
    bug_labels.add('Restrict-View-Google')
  for test in {a.GetTestMetadataKey() for a in alerts}:
    labels_components = bug_label_patterns.GetBugLabelsForTest(test)
    for item in labels_components:
      if item.startswith('Cr-'):
        bug_components.add(_ComponentFromCrLabel(item))
      else:
        bug_labels.add(item)
  return bug_labels, bug_components


def FetchBugComponents(alert_keys):
  """Fetches the ownership bug components of the most recent alert on a per-test
     path basis from the given alert keys.
  """
  alerts = ndb.get_multi(alert_keys)
  sorted_alerts = reversed(sorted(alerts, key=lambda alert: alert.timestamp))

  most_recent_components = {}

  for alert in sorted_alerts:
    alert_test = alert.test.id()
    if (alert.ownership and alert.ownership.get('component')
        and most_recent_components.get(alert_test) is None):
      if isinstance(alert.ownership['component'], list):
        most_recent_components[alert_test] = alert.ownership['component'][0]
      else:
        most_recent_components[alert_test] = alert.ownership['component']

  return set(most_recent_components.values())


def _MilestoneLabel(alerts):
  """Returns a milestone label string, or None.

  Because revision numbers for other repos may not be easily reconcilable with
  Chromium milestones, do not label them (see
  https://github.com/catapult-project/catapult/issues/2906).
  """
  revisions = [a.end_revision for a in alerts if hasattr(a, 'end_revision')]
  if not revisions:
    return None
  end_revision = min(revisions)
  for a in alerts:
    if a.end_revision == end_revision:
      row_key = utils.GetRowKey(a.test, a.end_revision)
      row = row_key.get()
      if hasattr(row, 'r_commit_pos'):
        end_revision = row.r_commit_pos
      else:
        return None
      break
  try:
    milestone = _GetMilestoneForRevision(end_revision)
  except KeyError:
    logging.error('List of versions not in the expected format')
  if not milestone:
    return None
  logging.info('Matched rev %s to milestone %s.', end_revision, milestone)
  return 'M-%d' % milestone


def _GetMilestoneForRevision(revision):
  """Finds the oldest milestone for a given revision from OmahaProxy.

  The purpose of this function is to resolve the milestone that would be blocked
  by a suspected regression. We do this by locating in the list of current
  versions, regardless of platform and channel, all the version strings (e.g.
  36.0.1234.56) that match revisions (commit positions) later than the earliest
  possible end_revision of the suspected regression; we then parse out the
  first numeric part of such strings, assume it to be the corresponding
  milestone, and return the lowest one in the set.

  Args:
    revision: An integer or string containing an integer.

  Returns:
    An integer representing the lowest milestone matching the given revision or
    the highest milestone if the given revision exceeds all defined milestones.
    Note that the default is 0 when no milestones at all are found. If the
    given revision is None, then None is returned.
  """
  if revision is None:
    return None
  milestones = set()
  default_milestone = 0
  all_versions = _GetAllCurrentVersionsFromOmahaProxy()
  for os in all_versions:
    for version in os['versions']:
      try:
        milestone = int(version['current_version'].split('.')[0])
        version_commit = version.get('branch_base_position')
        if version_commit and int(revision) < int(version_commit):
          milestones.add(milestone)
        if milestone > default_milestone:
          default_milestone = milestone
      except ValueError:
        # Sometimes 'N/A' is given. We ignore these entries.
        logging.warning('Could not cast one of: %s, %s, %s as an int', revision,
                        version['branch_base_position'],
                        version['current_version'].split('.')[0])
  if milestones:
    return min(milestones)
  return default_milestone


def _GetAllCurrentVersionsFromOmahaProxy():
  """Retrieves a the list current versions from OmahaProxy and parses it."""
  try:
    response = urlfetch.fetch(_OMAHA_PROXY_URL)
    if response.status_code == 200:
      return json.loads(response.content)
  except urlfetch.Error:
    logging.error('Error pulling list of current versions (omahaproxy).')
  except ValueError:
    logging.error('OmahaProxy did not return valid JSON.')
  return []


def _GetSingleCLForAnomalies(alerts):
  """If all anomalies were caused by the same culprit, return it. Else None."""
  revision = alerts[0].start_revision
  if not all(a.start_revision == revision and a.end_revision == revision
             for a in alerts):
    return None
  return revision


def GetCommitInfoForAlert(alert, crrev=None, gitiles=None):
  crrev = crrev or crrev_service
  gitiles = gitiles or gitiles_service
  repository_url = None
  repositories = namespaced_stored_object.Get('repositories')
  test_path = utils.TestPath(alert.test)
  if test_path.startswith('ChromiumPerf'):
    repository_url = repositories['chromium']['repository_url']
  elif test_path.startswith('ClankInternal'):
    repository_url = repositories['clank']['repository_url']
  if not repository_url:
    # Can't get committer info from this repository.
    return None

  rev = str(auto_bisect.GetRevisionForBisect(alert.end_revision, alert.test))

  if (re.match(r'^[0-9]{5,7}$', rev)
      and repository_url == repositories['chromium']['repository_url']):
    # This is a commit position, need the git hash.
    result = crrev.GetNumbering(
        number=rev,
        numbering_identifier='refs/heads/main',
        numbering_type='COMMIT_POSITION',
        project='chromium',
        repo='chromium/src')
    rev = result['git_sha']
  if not re.match(r'[a-fA-F0-9]{40}$', rev):
    # This still isn't a git hash; can't assign bug.
    return None
  return gitiles.CommitInfo(repository_url, rev)


def AssignBugToCLAuthor(bug_id, commit_info, labels=None, project='chromium'):
  """Assigns the bug to the author of the given revision."""
  author = commit_info['author']['email']
  message = commit_info['message']

  # Check first whether the assignee is an auto-roll, and get the alternative
  # result/assignee.
  alternative_assignee = utils.GetSheriffForAutorollCommit(
      author, message)
  author = alternative_assignee or author

  perf_issue_service_client.PostIssueComment(
      bug_id,
      project,
      comment='Assigning to %s because this is the only CL in range:\n%s' %
      (author, message),
      status='Assigned',
      labels=labels,
      owner=author,
  )


def FileBug(owner,
            cc,
            summary,
            description,
            project_id,
            labels,
            components,
            urlsafe_keys,
            needs_bisect=True):
  alert_keys = [ndb.Key(urlsafe=k) for k in urlsafe_keys]
  alerts = ndb.get_multi(alert_keys)

  if not description:
    description = 'See the link to graphs below.'

  milestone_label = _MilestoneLabel(alerts)
  if milestone_label:
    labels.append(milestone_label)

  new_bug_response = perf_issue_service_client.PostIssue(
      title=summary,
      description=description,
      project=project_id or 'chromium',
      labels=labels,
      components=components,
      owner=owner,
      cc=[email for email in cc.split(',') if email.strip()])

  if 'error' in new_bug_response:
    return {'error': new_bug_response['error']}

  bug_id = new_bug_response['issue_id']
  bug_data.Bug.New(bug_id=bug_id, project=project_id or 'chromium').put()

  for a in alerts:
    a.bug_id = bug_id
    a.project_id = project_id

  ndb.put_multi(alerts)
  comment_body = _AdditionalDetails(bug_id, project_id, alerts)

  # Add the bug comment with the service account, so that there are no
  # permissions issues.
  perf_issue_service_client.PostIssueComment(
      bug_id, project_id, comment=comment_body)
  template_params = {'bug_id': bug_id, 'project_id': project_id}
  if all(k.kind() == 'Anomaly' for k in alert_keys):
    logging.info('Kicking bisect for bug %s', bug_id)
    culprit_rev = _GetSingleCLForAnomalies(alerts)
    if culprit_rev is not None:
      commit_info = GetCommitInfoForAlert(alerts[0])
      if commit_info:
        needs_bisect = False
        AssignBugToCLAuthor(bug_id, commit_info)
    if needs_bisect:
      bisect_result = auto_bisect.StartNewBisectForBug(bug_id, project_id)
      if 'error' in bisect_result:
        logging.info('Failed to kick bisect for %s', bug_id)
        template_params['bisect_error'] = bisect_result['error']
      else:
        logging.info('Successfully kicked bisect for %s', bug_id)
        template_params.update(bisect_result)
  else:
    kinds = set()
    for k in alert_keys:
      kinds.add(k.kind())
    logging.info(
        'Didn\'t kick bisect for bug id %s because alerts had kinds %s', bug_id,
        list(kinds))
  return template_params
