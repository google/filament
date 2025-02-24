# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import unittest
import six
import webtest

from dashboard import main
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly

flask_app = Flask(__name__)


@flask_app.route('/')
def MainHandlerGet():
  return main.MainHandlerGet()


class MainTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)

  def testGet_PageIsShown(self):
    response = self.testapp.get('/')
    self.assertIn(b'<html>', response.body)

  def testGetColorClass(self):
    self.assertEqual('over-50', main._GetColorClass(95))
    self.assertEqual('over-40', main._GetColorClass(45))
    self.assertEqual('over-30', main._GetColorClass(31))
    self.assertEqual('over-20', main._GetColorClass(30))
    self.assertEqual('over-10', main._GetColorClass(12.0))
    self.assertEqual('under-10', main._GetColorClass(0.1))

  def testAnomalyInfoDicts(self):
    testing_common.AddTests(['M'], ['b'], {'t': {'foo': {}}})
    foo_key = utils.TestKey('M/b/t/foo')
    foo_anomaly = anomaly.Anomaly(
        start_revision=14999,
        end_revision=15000,
        test=foo_key,
        bug_id=12345,
        median_before_anomaly=100,
        median_after_anomaly=200)
    anomaly_key = foo_anomaly.put()
    self.assertEqual([{
        'master': 'M',
        'bot': 'b',
        'testsuite': 't',
        'test': 'foo',
        'bug_id': 12345,
        'start_revision': 14999,
        'end_revision': 15000,
        'key': anomaly_key.urlsafe(),
        'dashboard_link':
            ('https://chromeperf.appspot.com'
             '/group_report?keys=%s' % six.ensure_str(anomaly_key.urlsafe())),
        'percent_changed': '100.0%',
        'color_class': 'over-50',
        'improvement': False,
    }], main._AnomalyInfoDicts([foo_anomaly], {foo_key: foo_key.get()}))

  def testAnomalyInfoDicts_MissingTest_AnomalySkipped(self):
    testing_common.AddTests(['M'], ['b'], {'t': {'foo': {}}})
    foo_key = utils.TestKey('M/b/t/foo')
    foo_anomaly = anomaly.Anomaly(
        start_revision=14999,
        end_revision=15000,
        test=foo_key,
        bug_id=12345,
        median_before_anomaly=100,
        median_after_anomaly=200)
    foo_anomaly.put()
    self.assertEqual([], main._AnomalyInfoDicts([foo_anomaly], {}))


if __name__ == '__main__':
  unittest.main()
