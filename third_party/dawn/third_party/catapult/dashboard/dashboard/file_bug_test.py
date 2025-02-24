# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=too-many-lines
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
import json

from unittest import mock
import sys
import six
import webtest

from dashboard import file_bug
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import bug_label_patterns
from dashboard.models import histogram
from dashboard.models.subscription import Subscription

from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

flask_app = Flask(__name__)


@flask_app.route('/file_bug', methods=['GET', 'POST'])
def FileBugHandlerGet():
  return file_bug.FileBugHandlerGet()


class FileBugTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetSheriffDomains(['chromium.org'])
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)
    self.SetCurrentUser('foo@chromium.org')
    self._issue_tracker_service = testing_common.FakeIssueTrackerService()

    perf_issue_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.PostIssue',
        self._issue_tracker_service.NewBug)
    perf_issue_post_patcher.start()
    self.addCleanup(perf_issue_post_patcher.stop)

    perf_comment_post_patcher = mock.patch(
        'dashboard.services.perf_issue_service_client.PostIssueComment',
        self._issue_tracker_service.AddBugComment)
    perf_comment_post_patcher.start()
    self.addCleanup(perf_comment_post_patcher.stop)

    self.testapp = webtest.TestApp(flask_app)

  def tearDown(self):
    super().tearDown()
    self.UnsetCurrentUser()

  def _AddSampleAlerts(self, master='ChromiumPerf', has_commit_positions=True):
    """Adds sample data and returns a dict of rev to anomaly key."""
    # Add sample sheriff, masters, bots, and tests.
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff'],
        bug_components=['Blink>Javascript'],
    )
    testing_common.AddTests(
        [master], ['linux'],
        {'scrolling': {
            'first_paint': {},
            'mean_frame_time': {},
        }})
    test_path1 = '%s/linux/scrolling/first_paint' % master
    test_path2 = '%s/linux/scrolling/mean_frame_time' % master
    test_key1 = utils.TestKey(test_path1)
    test_key2 = utils.TestKey(test_path2)
    anomaly_key1 = self._AddAnomaly(111995, 112005, test_key1, subscription)
    anomaly_key2 = self._AddAnomaly(112000, 112010, test_key2, subscription)
    anomaly_key3 = self._AddAnomaly(112015, 112015, test_key2, subscription)
    rows_1 = testing_common.AddRows(test_path1, [112005])
    rows_2 = testing_common.AddRows(test_path2, [112010])
    rows_2 = testing_common.AddRows(test_path2, [112015])
    if has_commit_positions:
      rows_1[0].r_commit_pos = 112005
      rows_2[0].r_commit_pos = 112010
    return (anomaly_key1, anomaly_key2, anomaly_key3)

  def _AddSampleClankAlerts(self):
    """Adds sample data and returns a dict of rev to anomaly key.

    The biggest difference here is that the start/end revs aren't chromium
    commit positions. This tests the _MilestoneLabel function to make sure
    it will update the end_revision if r_commit_pos is found.
    """
    # Add sample sheriff, masters, bots, and tests. Doesn't need to be Clank.
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff'],
        bug_components=['Blink>Javascript'],
    )
    testing_common.AddTests(
        ['ChromiumPerf'], ['linux'],
        {'scrolling': {
            'first_paint': {},
            'mean_frame_time': {},
        }})
    test_path1 = 'ChromiumPerf/linux/scrolling/first_paint'
    test_path2 = 'ChromiumPerf/linux/scrolling/mean_frame_time'
    test_key1 = utils.TestKey(test_path1)
    test_key2 = utils.TestKey(test_path2)
    anomaly_key1 = self._AddAnomaly(1476193324, 1476201840, test_key1,
                                    subscription)
    anomaly_key2 = self._AddAnomaly(1476193320, 1476201870, test_key2,
                                    subscription)
    anomaly_key3 = self._AddAnomaly(1476193390, 1476193390, test_key2,
                                    subscription)
    rows_1 = testing_common.AddRows(test_path1, [1476201840])
    rows_2 = testing_common.AddRows(test_path2, [1476201870])
    rows_3 = testing_common.AddRows(test_path2, [1476193390])
    # These will be the revisions used to determine label.
    rows_1[0].r_commit_pos = 112005
    rows_2[0].r_commit_pos = 112010
    rows_3[0].r_commit_pos = 112015
    return (anomaly_key1, anomaly_key2, anomaly_key3)

  def _AddAnomaly(self, start_rev, end_rev, test_key, subscription):
    return anomaly.Anomaly(
        start_revision=start_rev,
        end_revision=end_rev,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscription_names=[subscription.name],
        subscriptions=[subscription]).put()

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('google.appengine.api.app_identity.get_default_version_hostname',
              mock.MagicMock(return_value='chromeperf.appspot.com'))
  def testBisectDisabled(self):
    owner = ''
    cc = 'you@chromium.org'
    summary = 'test'
    description = 'Test test.'
    project_id = None
    labels = []
    components = []
    test_path = 'ChromiumPerf/linux/scrolling/first_paint'
    test_key = utils.TestKey(test_path)
    subscription = Subscription(name='Sheriff',)
    keys = [self._AddAnomaly(10, 20, test_key, subscription).urlsafe()]
    bisect = False
    result = file_bug.file_bug.FileBug(owner, cc, summary, description,
                                       project_id, labels, components, keys,
                                       bisect)
    self.assertNotIn('bisect_error', result)
    self.assertNotIn('jobId', result)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('google.appengine.api.app_identity.get_default_version_hostname',
              mock.MagicMock(return_value='chromeperf.appspot.com'))
  def testSupportsCCList(self):
    owner = ''
    cc = 'you@chromium.org,me@chromium.org,other@chromium.org,,'
    summary = 'test'
    description = 'Test test.'
    project_id = None
    labels = []
    components = []
    test_path = 'ChromiumPerf/linux/scrolling/first_paint'
    test_key = utils.TestKey(test_path)
    subscription = Subscription(name='Sheriff',)
    keys = [self._AddAnomaly(10, 20, test_key, subscription).urlsafe()]
    bisect = False
    result = file_bug.file_bug.FileBug(owner, cc, summary, description,
                                       project_id, labels, components, keys,
                                       bisect)
    self.assertNotIn('bisect_error', result)
    self.assertNotIn('jobId', result)

  def testGet_WithNoKeys_ShowsError(self):
    # When a request is made and no keys parameter is given,
    # an error message is shown in the reply.
    response = self.testapp.get('/file_bug?summary=s&description=d&finish=true')
    self.assertIn(b'<div class="error">', response.body)
    self.assertIn(b'No alerts specified', response.body)

  def testGet_WithNoFinish_ShowsForm(self):
    # When a GET request is sent with keys specified but the finish parameter
    # is not given, the response should contain a form for the sheriff to fill
    # in bug details (summary, description, etc).
    alert_keys = self._AddSampleAlerts()
    response = self.testapp.get('/file_bug?summary=s&description=d&keys=%s' %
                                six.ensure_str(alert_keys[0].urlsafe()))
    self.assertEqual(1, len(response.html('form')))
    expected = '<input name="cc" type="text" value="foo@chromium.org"/>'
    self.assertIn(expected, str(response.html('form')[0]))

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  def testInternalBugLabel(self):
    # If any of the alerts are marked as internal-only, which should happen
    # when the corresponding test is internal-only, then the create bug dialog
    # should suggest adding a Restrict-View-Google label.
    self.SetCurrentUser('internal@chromium.org')
    alert_keys = self._AddSampleAlerts()
    anomaly_entity = alert_keys[0].get()
    anomaly_entity.internal_only = True
    anomaly_entity.put()
    response = self.testapp.get('/file_bug?summary=s&description=d&keys=%s' %
                                six.ensure_str(alert_keys[0].urlsafe()))
    self.assertIn(b'Restrict-View-Google', response.body)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  def testGet_SetsBugLabelsComponents(self):
    self.SetCurrentUser('internal@chromium.org')
    alert_keys = self._AddSampleAlerts()
    bug_label_patterns.AddBugLabelPattern('label1-foo', '*/*/*/first_paint')
    bug_label_patterns.AddBugLabelPattern('Cr-Performance-Blink',
                                          '*/*/*/mean_frame_time')
    response = self.testapp.get(
        '/file_bug?summary=s&description=d&keys=%s,%s' % (six.ensure_str(
            alert_keys[0].urlsafe()), six.ensure_str(alert_keys[1].urlsafe())))
    self.assertIn(b'label1-foo', response.body)
    self.assertIn(b'Performance&gt;Blink', response.body)
    self.assertIn(b'Performance-Sheriff', response.body)
    self.assertIn(b'Blink&gt;Javascript', response.body)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('google.appengine.api.app_identity.get_default_version_hostname',
              mock.MagicMock(return_value='chromeperf.appspot.com'))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def _PostSampleBug(self,
                     has_commit_positions=True,
                     master='ChromiumPerf',
                     is_single_rev=False):
    if master == 'ClankInternal':
      alert_keys = self._AddSampleClankAlerts()
    else:
      alert_keys = self._AddSampleAlerts(master, has_commit_positions)
    if is_single_rev:
      alert_keys = alert_keys[2].urlsafe()
    else:
      alert_keys = '%s,%s' % (six.ensure_str(
          alert_keys[0].urlsafe()), six.ensure_str(alert_keys[1].urlsafe()))
    response = self.testapp.post('/file_bug', [
        ('keys', alert_keys),
        ('summary', 's'),
        ('description', 'd\n'),
        ('finish', 'true'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    return response

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def testGet_WithFinish_CreatesBug(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug()

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # The anomaly entities should be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision in [112005, 112010]:
        self.assertEqual(277761, anomaly_entity.bug_id)
      else:
        self.assertIsNone(anomaly_entity.bug_id)

    # Two HTTP requests are made when filing a bug; only test 2nd request.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertIn('https://chromeperf.appspot.com/group_report?bug_id=277761',
                  comment)
    self.assertIn('https://chromeperf.appspot.com/group_report?sid=', comment)
    self.assertIn('\n\n\nBot(s) for this bug\'s original alert(s):\n\nlinux',
                  comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def testGet_WithFinish_CreatesBug_WithDocs(self):
    diag_dict = generic_set.GenericSet([[u'Benchmark doc link',
                                         u'http://docs']])
    diag = histogram.SparseDiagnostic(
        data=diag_dict.AsDict(),
        start_revision=1,
        end_revision=sys.maxsize,
        name=reserved_infos.DOCUMENTATION_URLS.name,
        test=utils.TestKey('ChromiumPerf/linux/scrolling'))
    diag.put()

    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug()

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # The anomaly entities should be updated.
    for anomaly_entity in anomaly.Anomaly.query().fetch():
      if anomaly_entity.end_revision in [112005, 112010]:
        self.assertEqual(277761, anomaly_entity.bug_id)
      else:
        self.assertIsNone(anomaly_entity.bug_id)

    # Two HTTP requests are made when filing a bug; only test 2nd request.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertIn('https://chromeperf.appspot.com/group_report?bug_id=277761',
                  comment)
    self.assertIn('https://chromeperf.appspot.com/group_report?sid=', comment)
    self.assertIn('\n\n\nBot(s) for this bug\'s original alert(s):\n\nlinux',
                  comment)
    self.assertIn('scrolling - Benchmark doc link:', comment)
    self.assertIn('http://docs', comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(
      file_bug.file_bug.crrev_service, 'GetNumbering',
      mock.MagicMock(
          return_value={'git_sha': '852ba7672ce02911e9f8f2a22363283adc80940e'}))
  @mock.patch('dashboard.services.gitiles_service.CommitInfo',
              mock.MagicMock(return_value={
                  'author': {
                      'email': 'foo@bar.com'
                  },
                  'message': 'My first commit!'
              }))
  def testGet_WithFinish_CreatesBugSingleRevOwner(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True)

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Three HTTP requests are made when filing a bug with owner; test third
    # request for owner hame.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertIn(
        'Assigning to foo@bar.com because this is the only CL in range',
        comment)
    self.assertIn('My first commit', comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(
      file_bug.file_bug.crrev_service, 'GetNumbering',
      mock.MagicMock(
          return_value={'git_sha': '852ba7672ce02911e9f8f2a22363283adc80940e'}))
  @mock.patch('dashboard.services.gitiles_service.CommitInfo',
              mock.MagicMock(
                  return_value={
                      'author': {
                          'email': 'v8-ci-autoroll-builder@chops-service-'
                                   'accounts.iam.gserviceaccount.com'
                      },
                      'message': 'This is a roll\n\nTBR=sheriff@bar.com'
                  }))
  def testGet_WithFinish_CreatesBugSingleRevAutorollOwner(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True)

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Two HTTP requests are made when filing a bug; only test 2nd request.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertIn('Assigning to sheriff@bar.com', comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  def testGet_WithFinish_SingleRevOwner_Clank_Skips(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True, master='ClankInternal')

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Three HTTP requests are made when filing a bug with owner; test third
    # request for owner hame.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertNotIn(
        'Assigning to foo@bar.com because this is the only CL in range',
        comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  def testGet_WithFinish_SingleRevOwner_InvalidRepository_Skips(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True, master='FakeMaster')

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Three HTTP requests are made when filing a bug with owner; test third
    # request for owner hame.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertNotIn(
        'Assigning to foo@bar.com because this is the only CL in range',
        comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(
      file_bug.file_bug.crrev_service, 'GetNumbering',
      mock.MagicMock(
          return_value={'git_sha': '852ba7672ce02911e9f8f2a22363283adc80940e'}))
  @mock.patch('dashboard.services.gitiles_service.CommitInfo',
              mock.MagicMock(return_value={
                  'author': {
                      'email': 'foo@bar.com'
                  },
                  'message': 'My first commit!'
              }))
  def testGet_WithFinish_CreatesBugSingleRevDifferentMasterOwner(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True, master='Foo')

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Three HTTP requests are made when filing a bug with owner; test third
    # request for owner hame.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertNotIn(
        'Assigning to foo@bar.com because this is the only CL in range',
        comment)
    self.assertNotIn('My first commit', comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[]))
  @mock.patch.object(
      file_bug.file_bug.crrev_service, 'GetNumbering',
      mock.MagicMock(
          return_value={'git_sha': '852ba7672ce02911e9f8f2a22363283adc80940e'}))
  @mock.patch('dashboard.services.gitiles_service.CommitInfo',
              mock.MagicMock(return_value={
                  'author': {
                      'email': 'robot@chops-service-accounts'
                               '.iam.gserviceaccount.com'
                  },
                  'message': 'This is an autoroll\n\nTBR=foo@bar.com',
              }))
  def testGet_WithFinish_CreatesBugSingleRevAutorollSheriff(self):
    # When a POST request is sent with keys specified and with the finish
    # parameter given, an issue will be created using the issue tracker
    # API, and the anomalies will be updated, and a response page will
    # be sent which indicates success.
    namespaced_stored_object.Set(
        'repositories', {
            "chromium": {
                "repository_url":
                    "https://chromium.googlesource.com/chromium/src"
            }
        })
    self._issue_tracker_service._bug_id_counter = 277761
    response = self._PostSampleBug(is_single_rev=True)

    # The response page should have a bug number.
    self.assertIn(b'277761', response.body)

    # Three HTTP requests are made when filing a bug with owner; test third
    # request for owner hame.
    comment = self._issue_tracker_service.add_comment_kwargs['comment']
    self.assertIn(
        'Assigning to foo@bar.com because this is the only CL in range',
        comment)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[{
                         'versions': [{
                             'branch_base_position': '112000',
                             'current_version': '2.0'
                         }, {
                             'branch_base_position': '111990',
                             'current_version': '1.0'
                         }]
                     }]))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def testGet_WithFinish_LabelsBugWithMilestone(self):
    # Here, we expect the bug to have the following end revisions:
    # [112005, 112010] and the milestones are M-1 for rev 111990 and
    # M-2 for 11200. Hence the expected behavior is to label the bug
    # M-2 since 111995 (lowest possible revision introducing regression)
    # is less than 112010 (revision for M-2).
    self._PostSampleBug()
    self.assertIn('M-2', self._issue_tracker_service.new_bug_kwargs['labels'])

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[{
                         'versions': [{
                             'branch_base_position': '112000',
                             'current_version': '2.0'
                         }, {
                             'branch_base_position': '111990',
                             'current_version': '1.0'
                         }]
                     }]))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def testGet_WithFinish_LabelsBugWithNoMilestoneBecauseNoCommitPos(self):
    # Here, we expect to return no Milestone label because the alerts do not
    # contain r_commit_pos (and therefore aren't chromium). Assuming
    # testGet_WithFinish_LabelsBugWithMilestone passes, M-2
    # would be the label that it would get if the alert was Chromium.
    self._PostSampleBug(has_commit_positions=False)
    labels = self._issue_tracker_service.new_bug_kwargs['labels']
    self.assertEqual(0, len([x for x in labels if x.startswith(u'M-')]))

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch.object(file_bug.file_bug, '_GetAllCurrentVersionsFromOmahaProxy',
                     mock.MagicMock(return_value=[{
                         'versions': [{
                             'branch_base_position': '113000',
                             'current_version': '2.0'
                         }, {
                             'branch_base_position': '112000',
                             'current_version': '2.0'
                         }, {
                             'branch_base_position': '111990',
                             'current_version': '1.0'
                         }]
                     }]))
  @mock.patch.object(file_bug.file_bug.auto_bisect, 'StartNewBisectForBug',
                     mock.MagicMock(return_value={
                         'issue_id': 123,
                         'issue_url': 'foo.com'
                     }))
  def testGet_WithFinish_LabelsBugForClank(self):
    # Here, we expect to return M-2 even though the alert revisions aren't
    # even close to the branching points. We use r_commmit_pos to determine
    # which revision to check. There are 3 branching points to ensure we are
    # actually changing the revision that is checked to r_commit_pos instead
    # of just displaying the highest one (previous behavior).
    self._PostSampleBug(master='ClankInternal')
    self.assertIn('M-2', self._issue_tracker_service.new_bug_kwargs['labels'])

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch(
      'google.appengine.api.urlfetch.fetch',
      mock.MagicMock(return_value=testing_common.FakeResponseObject(200, '[]')))
  def testGet_WithFinish_SucceedsWithNoVersions(self):
    # Here, we test that we don't label the bug with an unexpected value when
    # there is no version information from omahaproxy (for whatever reason)
    self._PostSampleBug()
    labels = self._issue_tracker_service.new_bug_kwargs['labels']
    self.assertEqual(0, len([x for x in labels if x.startswith(u'M-')]))

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch(
      'google.appengine.api.urlfetch.fetch',
      mock.MagicMock(return_value=testing_common.FakeResponseObject(200, '[]')))
  def testGet_WithFinish_SucceedsWithComponents(self):
    # Here, we test that components are posted separately from labels.
    self._PostSampleBug()
    self.assertIn('Foo>Bar',
                  self._issue_tracker_service.new_bug_kwargs['components'])

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('google.appengine.api.urlfetch.fetch',
              mock.MagicMock(
                  return_value=testing_common.FakeResponseObject(
                      200,
                      json.dumps([{
                          'versions': [{
                              'branch_base_position': '0',
                              'current_version': '1.0'
                          }]
                      }]))))
  def testGet_WithFinish_SucceedsWithRevisionOutOfRange(self):
    # Here, we test that we label the bug with the highest milestone when the
    # revision introducing regression is beyond all milestones in the list.
    self._PostSampleBug()
    self.assertIn('M-1', self._issue_tracker_service.new_bug_kwargs['labels'])

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('google.appengine.api.urlfetch.fetch',
              mock.MagicMock(
                  return_value=testing_common.FakeResponseObject(
                      200,
                      json.dumps([{
                          'versions': [{
                              'branch_base_position': 'N/A',
                              'current_version': 'N/A'
                          }]
                      }]))))
  @mock.patch('logging.warning')
  def testGet_WithFinish_SucceedsWithNAAndLogsWarning(self, mock_warn):
    self._PostSampleBug()
    labels = self._issue_tracker_service.new_bug_kwargs['labels']
    self.assertEqual(0, len([x for x in labels if x.startswith(u'M-')]))
    self.assertEqual(1, mock_warn.call_count)

  def testGet_OwnersAreEmptyEvenWithOwnership(self):
    ownership_samples = [{
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'emails': ['alice@chromium.org']
    }, {
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
        'emails': ['bob@chromium.org']
    }]
    test_paths = [
        'ChromiumPerf/linux/scrolling/first_paint',
        'ChromiumPerf/linux/scrolling/mean_frame_time'
    ]
    test_keys = [utils.TestKey(test_path) for test_path in test_paths]
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    anomaly_1 = anomaly.Anomaly(
        start_revision=1476193324,
        end_revision=1476201840,
        test=test_keys[0],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[0]).put()
    anomaly_2 = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_keys[1],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[1]).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(
            anomaly_1.urlsafe()), six.ensure_str(anomaly_2.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(b'<input type="text" name="owner" value="">', response.body)
    response_changed_order = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(
            anomaly_2.urlsafe()), six.ensure_str(anomaly_1.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(b'<input type="text" name="owner" value="">',
                  response_changed_order.body)

  def testGet_OwnersNotFilledWhenNoOwnership(self):
    test_key = utils.TestKey('ChromiumPerf/linux/scrolling/first_paint')
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    anomaly_entity = anomaly.Anomaly(
        start_revision=1476193324,
        end_revision=1476201840,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
    ).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s' % (six.ensure_str(anomaly_entity.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(b'<input type="text" name="owner" value="">', response.body)

  def testGet_WithAllOwnershipComponents(self):
    ownership_samples = [{
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'component': 'Abc>Xyz'
    }, {
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
        'component': 'Def>123'
    }]
    test_paths = [
        'ChromiumPerf/linux/scrolling/first_paint',
        'ChromiumPerf/linux/scrolling/mean_frame_time'
    ]
    test_keys = [utils.TestKey(test_path) for test_path in test_paths]
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    anomaly_1 = anomaly.Anomaly(
        start_revision=1476193324,
        end_revision=1476201840,
        test=test_keys[0],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[0]).put()
    anomaly_2 = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_keys[1],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[1]).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s' % six.ensure_str((anomaly_1.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Xyz">',
        response.body)
    response_with_both_anomalies = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(
            anomaly_1.urlsafe()), six.ensure_str(anomaly_2.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Xyz">',
        response_with_both_anomalies.body)
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="Def&gt;123">',
        response_with_both_anomalies.body)

  def testGet_UsesOnlyMostRecentComponents(self):
    ownership_samples = [
        {
            'type': 'Ownership',
            'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
            'component': 'Abc>Def'
        },
        {
            'type': 'Ownership',
            'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
            'component': '123>456'
        },
    ]
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    test_key = utils.TestKey('ChromiumPerf/linux/scrolling/first_paint')
    now_datetime = datetime.datetime.now()
    older_alert = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[0],
        timestamp=now_datetime).put()
    newer_alert = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[1],
        timestamp=now_datetime + datetime.timedelta(10)).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(
            older_alert.urlsafe()), six.ensure_str(newer_alert.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertNotIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Def">',
        response.body)
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="123&gt;456">',
        response.body)
    response_inverted_order = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(
            newer_alert.urlsafe()), six.ensure_str(older_alert.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertNotIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Def">',
        response_inverted_order.body)
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="123&gt;456">',
        response_inverted_order.body)

  def testGet_ComponentsChosenPerTest(self):
    ownership_samples = [
        {
            'type': 'Ownership',
            'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
            'component': 'Abc>Def'
        },
        {
            'type': 'Ownership',
            'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
            'component': '123>456'
        },
    ]
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    test_paths = [
        'ChromiumPerf/linux/scrolling/first_paint',
        'ChromiumPerf/linux/scrolling/mean_frame_time'
    ]
    test_keys = [utils.TestKey(test_path) for test_path in test_paths]
    now_datetime = datetime.datetime.now()
    alert_test_key_0 = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_keys[0],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[0],
        timestamp=now_datetime).put()
    alert_test_key_1 = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_keys[1],
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[1],
        timestamp=now_datetime + datetime.timedelta(10)).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s,%s' % (six.ensure_str(alert_test_key_0.urlsafe()),
                            six.ensure_str(alert_test_key_1.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Def">',
        response.body)
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="123&gt;456">',
        response.body)

  def testGet_UsesFirstDefinedComponent(self):
    ownership_samples = [{
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
    }, {
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'component': ''
    }, {
        'type': 'Ownership',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'component': 'Abc>Def'
    }]
    now_datetime = datetime.datetime.now()
    test_key = utils.TestKey('ChromiumPerf/linux/scrolling/first_paint')
    subscription = Subscription(
        name='Sheriff',
        bug_labels=['Performance-Sheriff', 'Cr-Blink-Javascript'])
    alert_without_ownership = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        timestamp=now_datetime).put()
    alert_without_component = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[0],
        timestamp=now_datetime + datetime.timedelta(10)).put()
    alert_with_empty_component = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[1],
        timestamp=now_datetime + datetime.timedelta(20)).put()
    alert_with_component = anomaly.Anomaly(
        start_revision=1476193320,
        end_revision=1476201870,
        test=test_key,
        median_before_anomaly=100,
        median_after_anomaly=200,
        subscriptions=[subscription],
        subscription_names=[subscription.name],
        ownership=ownership_samples[2],
        timestamp=now_datetime + datetime.timedelta(30)).put()
    response = self.testapp.post('/file_bug', [
        ('keys', '%s,%s,%s,%s' %
         (six.ensure_str(alert_without_ownership.urlsafe()),
          six.ensure_str(alert_without_component.urlsafe()),
          six.ensure_str(alert_with_empty_component.urlsafe()),
          six.ensure_str(alert_with_component.urlsafe()))),
        ('summary', 's'),
        ('description', 'd\n'),
        ('label', 'one'),
        ('label', 'two'),
        ('component', 'Foo>Bar'),
    ])
    self.assertIn(
        b'<input type="checkbox" checked name="component" value="Abc&gt;Def">',
        response.body)
