# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
import json
import unittest
import uuid

from dashboard.api import api_auth
from dashboard.api import timeseries2
from dashboard.common import testing_common
from dashboard.models import anomaly
from dashboard.models import graph_data
from dashboard.models import histogram
from dashboard.models.subscription import Subscription
from tracing.value.diagnostics import reserved_infos

_TEST_HISTOGRAM_DATA = {
    'binBoundaries': [1, [1, 1000, 20]],
    'diagnostics': {
        'buildbot': 'e9c2891d-2b04-413f-8cf4-099827e67626',
        'revisions': '25f0a111-9bb4-4cea-b0c1-af2609623160',
        'telemetry': '0bc1021b-8107-4db7-bc8c-49d7cf53c5ae'
    },
    'name': 'foo',
    'unit': 'count'
}

flask_app = Flask(__name__)


@flask_app.route('/api/timeseries2', methods=['POST'])
def TimeSeries2Post():
  return timeseries2.TimeSeries2Post()


class Timeseries2Test(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetUpFlaskApp(flask_app)
    self.SetCurrentClientIdOAuth(api_auth.OAUTH_CLIENT_ID_ALLOWLIST[0])
    self.SetCurrentUserOAuth(None)

  def _MockData(self,
                path='master/bot/suite/measure/case',
                internal_only=False):
    test = graph_data.TestMetadata(
        has_rows=True,
        id=path,
        improvement_direction=anomaly.DOWN,
        internal_only=internal_only,
        units='units')
    test.UpdateSheriff()
    test.put()

    for i in range(1, 21, 2):
      graph_data.Row(
          error=(i / 2.0),
          id=i,
          parent=test.key,
          r_i2=(i * 2),
          timestamp=datetime.datetime.utcfromtimestamp(i),
          value=float(i)).put()
      histogram.Histogram(
          data=_TEST_HISTOGRAM_DATA,
          id=str(uuid.uuid4()),
          internal_only=internal_only,
          revision=i,
          test=test.key).put()

    anomaly.Anomaly(
        end_revision=11,
        internal_only=internal_only,
        is_improvement=False,
        median_after_anomaly=6,
        median_before_anomaly=4,
        subscriptions=[
            Subscription(
                name='Taylor',
                notification_email=testing_common.INTERNAL_USER.email(),
            )
        ],
        subscription_names=['Taylor'],
        start_revision=10,
        test=test.key).put()

    histogram.SparseDiagnostic(
        data={
            'type': 'GenericSet',
            'guid': str(uuid.uuid4()),
            'values': [1]
        },
        end_revision=11,
        id=str(uuid.uuid4()),
        internal_only=internal_only,
        name=reserved_infos.DEVICE_IDS.name,
        start_revision=1,
        test=test.key).put()

    histogram.SparseDiagnostic(
        data={
            'type': 'GenericSet',
            'guid': str(uuid.uuid4()),
            'values': [2]
        },
        end_revision=None,
        id=str(uuid.uuid4()),
        internal_only=internal_only,
        name=reserved_infos.DEVICE_IDS.name,
        start_revision=11,
        test=test.key).put()

  def _Post(self, **params):
    return json.loads(self.Post('/api/timeseries2', params).body)

  def testNotFound(self):
    self._MockData()
    params = dict(
        test_suite='not a thing',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,revisions,avg,std,alert,diagnostics,histogram')
    self.Post('/api/timeseries2', params, status=404)

  def testInternalData_AnonymousUser(self):
    self._MockData(internal_only=True)
    params = dict(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,revisions,avg,std,alert,diagnostics,histogram')
    self.Post('/api/timeseries2', params, status=404)

  def testCollateAllColumns(self):
    self._MockData()
    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,revisions,avg,std,alert,diagnostics,histogram')
    self.assertEqual('units', response['units'])
    self.assertEqual('down', response['improvement_direction'])
    self.assertEqual(10, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(7, len(datum))
      ri = 1 + (2 * i)
      self.assertEqual(ri, datum[0])
      self.assertEqual(2 * ri, datum[1]['r_i2'])
      self.assertEqual(ri, datum[2])
      self.assertEqual(0.5 + i, datum[3])
      if i == 5:
        self.assertEqual('case', datum[4]['descriptor']['testCase'])
      else:
        self.assertEqual(None, datum[4])
      if i in [0, 5]:
        self.assertEqual('deviceIds', list(datum[5].keys())[0])
      else:
        self.assertEqual(None, datum[5])
      self.assertEqual(_TEST_HISTOGRAM_DATA['name'], datum[6]['name'])

  def testRevisionRange(self):
    self._MockData(internal_only=False)
    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        min_revision=5,
        max_revision=15,
        columns='revision,revisions,avg,std,alert,diagnostics,histogram')
    self.assertEqual(6, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(7, len(datum))
      self.assertEqual(5 + (2 * i), datum[0])

  def testTimestampRange(self):
    self._MockData(internal_only=False)
    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        min_timestamp=datetime.datetime.utcfromtimestamp(5).isoformat(),
        max_timestamp=datetime.datetime.utcfromtimestamp(15).isoformat(),
        columns='timestamp,revision,revisions,avg,alert,diagnostics')
    self.assertEqual(6, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(6, len(datum))
      self.assertEqual(5 + (2 * i), datum[1])

  def testAnnotations(self):
    test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/case',
        improvement_direction=anomaly.DOWN,
        units='units')
    test.UpdateSheriff()
    test.put()
    graph_data.Row(
        parent=test.key, id=1, value=42.0,
        a_trace_uri='http://example.com').put()

    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,annotations')
    self.assertEqual(1, len(response['data']))
    self.assertEqual(1, response['data'][0][0])
    self.assertEqual('http://example.com',
                     response['data'][0][1]['a_trace_uri'])

  def testMixOldStyleRowsWithNewStyleRows(self):
    old_count_test = graph_data.TestMetadata(
        has_rows=True, id='master/bot/suite/measure_count/case', units='count')
    old_count_test.UpdateSheriff()
    old_count_test.put()

    old_avg_test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure_avg/case',
        improvement_direction=anomaly.DOWN,
        units='units')
    old_avg_test.UpdateSheriff()
    old_avg_test.put()

    old_std_test = graph_data.TestMetadata(
        has_rows=True, id='master/bot/suite/measure_std/case', units='units')
    old_std_test.UpdateSheriff()
    old_std_test.put()

    for i in range(1, 21, 2):
      graph_data.Row(parent=old_avg_test.key, id=i, value=float(i)).put()
      graph_data.Row(parent=old_std_test.key, id=i, value=(i / 2.0)).put()
      graph_data.Row(parent=old_count_test.key, id=i, value=10).put()

    new_test = graph_data.TestMetadata(
        has_rows=True,
        id='master/bot/suite/measure/case',
        improvement_direction=anomaly.DOWN,
        units='units')
    new_test.UpdateSheriff()
    new_test.put()
    for i in range(21, 41, 2):
      graph_data.Row(
          d_count=10,
          error=(i / 2.0),
          id=i,
          parent=new_test.key,
          value=float(i)).put()

    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,avg,std,count')
    self.assertEqual('units', response['units'])
    self.assertEqual('down', response['improvement_direction'])
    self.assertEqual(20, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(4, len(datum))
      ri = 1 + (2 * i)
      self.assertEqual(ri, datum[0])
      self.assertEqual(ri, datum[1])
      self.assertEqual(ri / 2.0, datum[2])
      self.assertEqual(10, datum[3])

  def testProjection(self):
    self._MockData(internal_only=False)
    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,avg,timestamp')
    self.assertEqual(10, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(3, len(datum))
      ri = 1 + (2 * i)
      self.assertEqual(ri, datum[0])
      self.assertEqual(ri, datum[1])
      self.assertEqual(
          datetime.datetime.utcfromtimestamp(ri).isoformat(), datum[2])

  def testHistogramsOnly(self):
    self._MockData()
    response = self._Post(
        test_suite='suite',
        measurement='measure',
        bot='master:bot',
        test_case='case',
        build_type='test',
        columns='revision,histogram')
    self.assertEqual('units', response['units'])
    self.assertEqual('down', response['improvement_direction'])
    self.assertEqual(10, len(response['data']))
    for i, datum in enumerate(response['data']):
      self.assertEqual(2, len(datum))
      self.assertEqual(1 + (2 * i), datum[0])
      self.assertEqual(_TEST_HISTOGRAM_DATA['name'], datum[1]['name'])


if __name__ == '__main__':
  unittest.main()
