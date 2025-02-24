# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from six.moves import http_client
import json
from unittest import mock
import unittest

from apiclient import errors

from dashboard.common import testing_common
from dashboard.services import issue_tracker_service


def PythonVersionsDecorator():

  def Decorator(func):
    return mock.patch('services.issue_tracker_service.discovery.build',
                      mock.MagicMock())(
                          func)

  return Decorator


@PythonVersionsDecorator()
class IssueTrackerServiceTest(testing_common.TestCase):

  def testAddBugComment_Basic(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertTrue(service.AddBugComment(12345, 'The comment'))
    self.assertEqual(1, service._MakeCommentRequest.call_count)
    service._MakeCommentRequest.assert_called_with(
        12345, {
            'updates': {},
            'content': 'The comment'
        },
        project='chromium',
        send_email=True)

  def testAddBugComment_Basic_EmptyProject(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertTrue(service.AddBugComment(12345, 'The comment', project=''))
    self.assertEqual(1, service._MakeCommentRequest.call_count)
    service._MakeCommentRequest.assert_called_with(
        12345, {
            'updates': {},
            'content': 'The comment'
        },
        project='chromium',
        send_email=True)

  def testAddBugComment_Basic_ProjectIsNone(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertTrue(service.AddBugComment(12345, 'The comment', project=None))
    self.assertEqual(1, service._MakeCommentRequest.call_count)
    service._MakeCommentRequest.assert_called_with(
        12345, {
            'updates': {},
            'content': 'The comment'
        },
        project='chromium',
        send_email=True)

  def testAddBugComment_WithNoBug_ReturnsFalse(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertFalse(service.AddBugComment(None, 'Some comment'))
    self.assertFalse(service.AddBugComment(-1, 'Some comment'))

  def testAddBugComment_WithOptionalParameters(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertTrue(
        service.AddBugComment(
            12345,
            'Some other comment',
            status='Fixed',
            labels=['Foo'],
            cc_list=['someone@chromium.org']))
    self.assertEqual(1, service._MakeCommentRequest.call_count)
    service._MakeCommentRequest.assert_called_with(
        12345, {
            'updates': {
                'status': 'Fixed',
                'cc': ['someone@chromium.org'],
                'labels': ['Foo'],
            },
            'content': 'Some other comment'
        },
        project='chromium',
        send_email=True)

  def testAddBugComment_MergeBug(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCommentRequest = mock.Mock()
    self.assertTrue(service.AddBugComment(12345, 'Dupe', merge_issue=54321))
    self.assertEqual(1, service._MakeCommentRequest.call_count)
    service._MakeCommentRequest.assert_called_with(
        12345, {
            'updates': {
                'status': 'Duplicate',
                'mergedInto': 'chromium:54321',
            },
            'content': 'Dupe'
        },
        project='chromium',
        send_email=True)

  @mock.patch('logging.error')
  def testAddBugComment_Error(self, mock_logging_error):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value=None)
    self.assertFalse(service.AddBugComment(12345, 'My bug comment'))
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(1, mock_logging_error.call_count)

  def testNewBug_Success_NewBugReturnsId(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value={'id': 333})
    response = service.NewBug('Bug title', 'body', owner='someone@chromium.org')
    bug_id = response['bug_id']
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(333, bug_id)

  def testNewBug_Success_SupportNonChromium(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value={
        'id': 333,
        'projectId': 'non-chromium'
    })
    response = service.NewBug(
        'Bug title',
        'body',
        owner='someone@example.com',
        project='non-chromium')
    bug_id = response['bug_id']
    project_id = response['project_id']
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(333, bug_id)
    self.assertEqual('non-chromium', project_id)

  def testNewBug_Success_ProjectIsEmpty(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value={
        'id': 333,
        'projectId': 'chromium'
    })
    response = service.NewBug(
        'Bug title', 'body', owner='someone@example.com', project='')
    bug_id = response['bug_id']
    project_id = response['project_id']
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(333, bug_id)
    self.assertEqual('chromium', project_id)

  def testNewBug_Success_ProjectIsNone(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value={
        'id': 333,
        'projectId': 'chromium'
    })
    response = service.NewBug(
        'Bug title', 'body', owner='someone@example.com', project=None)
    bug_id = response['bug_id']
    project_id = response['project_id']
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(333, bug_id)
    self.assertEqual('chromium', project_id)

  def testNewBug_Failure_HTTPException(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(
        side_effect=http_client.HTTPException('reason'))
    response = service.NewBug('Bug title', 'body', owner='someone@chromium.org')
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertIn('error', response)

  def testNewBug_Failure_NewBugReturnsError(self):
    service = issue_tracker_service.IssueTrackerService()
    service._ExecuteRequest = mock.Mock(return_value={})
    response = service.NewBug('Bug title', 'body', owner='someone@chromium.org')
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertTrue('error' in response)

  def testNewBug_HttpError_NewBugReturnsError(self):
    service = issue_tracker_service.IssueTrackerService()
    error_content = {
        'error': {
            'message': 'The user does not exist: test@chromium.org',
            'code': 404
        }
    }
    service._ExecuteRequest = mock.Mock(
        side_effect=errors.HttpError(
            mock.Mock(return_value={'status': 404}),
            json.dumps(error_content).encode('utf-8')))
    response = service.NewBug('Bug title', 'body', owner='someone@chromium.org')
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertTrue('error' in response)

  def testNewBug_UsesExpectedParams(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCreateRequest = mock.Mock()
    service.NewBug(
        'Bug title',
        'body',
        owner='someone@chromium.org',
        cc=['somebody@chromium.org', 'nobody@chromium.org'])
    service._MakeCreateRequest.assert_called_with(
        {
            'title': 'Bug title',
            'summary': 'Bug title',
            'description': 'body',
            'labels': [],
            'components': [],
            'status': 'Assigned',
            'owner': {
                'name': 'someone@chromium.org'
            },
            'cc': mock.ANY,
            'projectId': 'chromium',
        }, 'chromium')
    self.assertCountEqual([
        {
            'name': 'somebody@chromium.org'
        },
        {
            'name': 'nobody@chromium.org'
        },
    ], service._MakeCreateRequest.call_args[0][0].get('cc'))

  def testNewBug_UsesExpectedParamsSansOwner(self):
    service = issue_tracker_service.IssueTrackerService()
    service._MakeCreateRequest = mock.Mock()
    service.NewBug(
        'Bug title',
        'body',
        cc=['somebody@chromium.org', 'nobody@chromium.org'])
    service._MakeCreateRequest.assert_called_with(
        {
            'title': 'Bug title',
            'summary': 'Bug title',
            'description': 'body',
            'labels': [],
            'components': [],
            'status': 'Unconfirmed',
            'cc': mock.ANY,
            'projectId': 'chromium',
        }, 'chromium')
    self.assertCountEqual([
        {
            'name': 'somebody@chromium.org'
        },
        {
            'name': 'nobody@chromium.org'
        },
    ], service._MakeCreateRequest.call_args[0][0].get('cc'))

  def testMakeCommentRequest_UserCantOwn_RetryMakeCommentRequest(self):
    service = issue_tracker_service.IssueTrackerService()
    error_content = {
        'error': {
            'message': 'Issue owner must be a project member',
            'code': 400
        }
    }
    service._ExecuteRequest = mock.Mock(
        side_effect=errors.HttpError(
            mock.Mock(return_value={'status': 404}),
            json.dumps(error_content).encode('utf-8')))
    service.AddBugComment(12345, 'The comment', owner=['test@chromium.org'])
    self.assertEqual(2, service._ExecuteRequest.call_count)

  def testMakeCommentRequest_UserDoesNotExist_RetryMakeCommentRequest(self):
    service = issue_tracker_service.IssueTrackerService()
    error_content = {
        'error': {
            'message': 'The user does not exist: test@chromium.org',
            'code': 404
        }
    }
    service._ExecuteRequest = mock.Mock(
        side_effect=errors.HttpError(
            mock.Mock(return_value={'status': 404}),
            json.dumps(error_content).encode('utf-8')))
    service.AddBugComment(
        12345,
        'The comment',
        cc_list=['test@chromium.org'],
        owner=['test@chromium.org'])
    self.assertEqual(2, service._ExecuteRequest.call_count)

  def testMakeCommentRequest_IssueDeleted_ReturnsTrue(self):
    service = issue_tracker_service.IssueTrackerService()
    error_content = {
        'error': {
            'message': 'User is not allowed to view this issue 12345',
            'code': 403
        }
    }
    service._ExecuteRequest = mock.Mock(
        side_effect=errors.HttpError(
            mock.Mock(return_value={'status': 403}),
            json.dumps(error_content).encode('utf-8')))
    comment_posted = service.AddBugComment(
        12345, 'The comment', owner='test@chromium.org')
    self.assertEqual(1, service._ExecuteRequest.call_count)
    self.assertEqual(True, comment_posted)


if __name__ == '__main__':
  unittest.main()
