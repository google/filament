# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard import update_bug_with_results
from dashboard.common import layered_cache
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.services import perf_issue_service_client

_BUGS = {
    '12345': {
        'status': 'Opened',
        'id': 12345,
        'projectId': 'chromium',
    },
    '54321': {
        'status': perf_issue_service_client.STATUS_DUPLICATE,
        'id': 54321,
        'projectId': 'chromium'
    }
}


def _GetIssue(bug_id, project_name='chromium'):
  del project_name
  return _BUGS.get(bug_id)

# In this class, we patch apiclient.discovery.build so as to not make network
# requests, which are normally made when the IssueTrackerService is initialized.
@mock.patch('apiclient.discovery.build', mock.MagicMock())
@mock.patch('dashboard.services.perf_issue_service_client.GetIssue', _GetIssue)
@mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
class UpdateBugWithResultsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetCurrentUser('internal@chromium.org', is_admin=True)
    namespaced_stored_object.Set(
        'repositories', {
            'chromium': {
                'repository_url':
                    'https://chromium.googlesource.com/chromium/src'
            },
        })
    self.a1 = anomaly.Anomaly(
        bug_id=12345,
        test=utils.TestKey(
            'master/bot/test_suite/measurement/test_case')).put()
    self.a2 = anomaly.Anomaly(
        bug_id=54321,
        test=utils.TestKey(
            'master/bot/test_suite/measurement/test_case')).put()
    issue = {
        'id': 12345,
        'status': 'Assigned',
    }
    self.merge_details = {
        'issue': issue,
        'projectId': 'chromium',
        'id': 54321,
        'comments': ''
    }
    self.commit_cache_key = 'abcdef'

  def testGetMergeIssueDetailsFromNonExistentCacheKey(self):
    # Commit key is not in cache so can't get merge details.
    merge_details = update_bug_with_results.GetMergeIssueDetails(
        self.commit_cache_key)
    self.assertEqual(merge_details, {
        'issue': {},
        'projectId': None,
        'id': None,
        'comments': ''
    })

  def testGetMergeIssueDetailsFromNonExistentIssue(self):
    # Commit key is in cache but associated issue doesn't exist in the
    # issue tracker.
    layered_cache.SetExternal(self.commit_cache_key, 'chromium:23132')
    merge_details = update_bug_with_results.GetMergeIssueDetails(
        self.commit_cache_key)
    self.assertEqual(merge_details, {
        'issue': {},
        'projectId': None,
        'id': None,
        'comments': ''
    })

  def testGetMergeIssueDetailsFromDuplicateIssue(self):
    # Able to retrieve merge details from valid issue, but issue is already
    # a duplicate. Don't merge into a duplicate.
    layered_cache.SetExternal(self.commit_cache_key, 'chromium:54321')
    merge_details = update_bug_with_results.GetMergeIssueDetails(
        self.commit_cache_key)
    self.assertEqual(
        merge_details, {
            'issue': _BUGS['54321'],
            'projectId': 'chromium',
            'id': None,
            'comments': ''
        })

  def testGetMergeIssueDetailsFromNonDuplicateIssue(self):
    # Valid non-duplicate issue in cache, so merge_details should contain
    # the issue itself and its id.
    layered_cache.SetExternal(self.commit_cache_key, 'chromium:12345')
    merge_details = update_bug_with_results.GetMergeIssueDetails(
        self.commit_cache_key)
    self.assertEqual(
        merge_details, {
            'issue': _BUGS['12345'],
            'projectId': 'chromium',
            'id': '12345',
            'comments': ''
        })

  def testUpdateMergeIssueNoMergeIssue(self):
    # If merge_details has no bug to merge into, add bug_id to the cache.
    self.merge_details['issue']['id'] = None
    self.merge_details['id'] = None
    update_bug_with_results.UpdateMergeIssue(
        commit_cache_key=self.commit_cache_key,
        merge_details=self.merge_details,
        bug_id=54321,
        project='chromium')
    merge_issue_key = layered_cache.GetExternal(self.commit_cache_key)
    self.assertEqual(merge_issue_key, 'chromium:54321')

  def testUpdateMergeIssueDuplicateMergeIssue(self):

    # If bug 1 is marked as duplicate, you do not merge bug 2 into it.
    self.merge_details['issue']['status'] = 'Duplicate'
    update_bug_with_results.UpdateMergeIssue(
        commit_cache_key=self.commit_cache_key,
        merge_details=self.merge_details,
        bug_id=54321,
        project='chromium')
    merge_issue_key = layered_cache.GetExternal(self.commit_cache_key)
    self.assertIsNone(merge_issue_key)

  def testUpdateMergeIssueValidMergeIssue(self):
    # Add bug 1 to the cache.
    layered_cache.SetExternal(self.commit_cache_key, 'chromium:12345')

    # UpdateMergeIssue gets called with bug 2.
    update_bug_with_results.UpdateMergeIssue(
        commit_cache_key=self.commit_cache_key,
        merge_details=self.merge_details,
        bug_id=54321,
        project='chromium')
    merge_issue_key = layered_cache.GetExternal(self.commit_cache_key)
    self.assertEqual(merge_issue_key, 'chromium:12345')

    # The anomaly from bug 2 should've moved to bug 1.
    anomalies1 = anomaly.Anomaly.query(
        anomaly.Anomaly.bug_id == 12345,
        anomaly.Anomaly.project_id == 'chromium').fetch(keys_only=True)
    self.assertCountEqual(anomalies1, [self.a1, self.a2])

    # And bug 2 should have zero anomalies.
    anomalies2 = anomaly.Anomaly.query(
        anomaly.Anomaly.bug_id == 54321,
        anomaly.Anomaly.project_id == 'chromium').fetch(keys_only=True)
    self.assertCountEqual(anomalies2, [])

  def testMapAnomaliesToMergeIntoBug(self):

    # Map anomalies to base(dest_bug_id) bug.
    update_bug_with_results._MapAnomaliesToMergeIntoBug(
        dest_issue=update_bug_with_results.IssueInfo('chromium', 12345),
        source_issue=update_bug_with_results.IssueInfo('chromium', 54321))

    self.assertEqual(self.a1.get().bug_id, 12345)
    self.assertEqual(self.a2.get().bug_id, 12345)

    anomalies1 = anomaly.Anomaly.query(
        anomaly.Anomaly.bug_id == 12345,
        anomaly.Anomaly.project_id == 'chromium').fetch(keys_only=True)
    self.assertCountEqual(anomalies1, [self.a1, self.a2])

    anomalies2 = anomaly.Anomaly.query(
        anomaly.Anomaly.bug_id == 54321,
        anomaly.Anomaly.project_id == 'chromium').fetch(keys_only=True)
    self.assertCountEqual(anomalies2, [])


if __name__ == '__main__':
  unittest.main()
