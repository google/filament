# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

import datetime
import json
import os
import sys
from pathlib import Path
import unittest

app_path = Path(__file__).parent.parent.parent
if str(app_path) not in sys.path:
  sys.path.insert(0, str(app_path))

from application import app
from application.perf_api import anomalies
from google.cloud import datastore

from unittest import mock


class QueryAnomaliesTest(unittest.TestCase):

  _test_email = "test@chromium.org"
  def setUp(self):
    self.client = app.Create().test_client()
    os.environ['DISABLE_METRICS'] = 'True'

  def testInvalidRequestJson(self):
    test_name = 'master/bot/test1/metric'
    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email
      response = self.client.post(
          '/anomalies/find_time',
          data='{"tests":["%s"],'  # Wrong request param
               '"start_time":"1234", "end_time":"1233" add invalid}'
               % test_name)
      self.assertEqual(400, response.status_code)
      self.assertEqual("Malformed Json", response.get_data(as_text=True))

  def testInvalidRequestParam(self):
    test_name = 'master/bot/test1/metric'
    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email
      response = self.client.post(
          '/anomalies/find_time',
          data='{"SearchTests":["%s"],'  # Wrong request param
               '"start_time":"Jan 1, 2024 at 4:28:08.609PM UTC-8"}'
               % test_name)
      self.assertEqual(400, response.status_code)
      self.assertTrue('tests' in response.get_data(as_text=True))

  @mock.patch('application.perf_api.datastore_client'
              '.DataStoreClient.QueryAnomaliesTimestamp')
  def testNoAnomaliesExist(self, query_mock):

    query_mock.return_value = []
    test_name = 'master/bot/test1/metric'
    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email
      response = self.client.post(
          '/anomalies/find_time',
          data='''{"tests":["%s"],
          "start_time":"Jan 1, 2024 at 4:28:08.609PM UTC-8",
          "end_time":"Jan 2, 2024 at 4:28:08.609PM UTC-8"}''' % test_name)
      data = json.loads(response.get_data(as_text=True))
      self.assertEqual({}, data["anomalies"], 'No anomalies expected')

  @mock.patch('application.perf_api.datastore_client'
              '.DataStoreClient.QueryAnomaliesTimestamp')
  def testNoAnomaliesFound(self, query_mock):
    test_name = 'master/bot/test1/metric'
    client = datastore.Client()
    def mock_query(tests, start_time, end_time):
      start = datetime.datetime(2024, 1, 1)
      end = datetime.datetime(2024, 1, 2)
      if test_name in tests and \
         start >= start_time and end <= end_time:
        test_key1 = client.key('TestMetadata', test_name)
        anomaly_key = client.key('Anomaly', '1111')
        test_anomaly = datastore.entity.Entity(anomaly_key)
        test_anomaly['start_revision'] = 1234
        test_anomaly['end_revision'] = 5678
        test_anomaly['test'] = test_key1
        return [test_anomaly]

      return []

    query_mock.side_effect = mock_query

    test_name_2 = 'some/other/test'

    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email
      # Search for a test for which anomaly does not exist
      response = self.client.post(
          '/anomalies/find_time',
          data='{"tests":["%s"], "start_time":"Jan 1, 2024 at 4:28:08.609PM",'
           '"end_time":"Jan 2, 2024 at 4:28:08.609PM"}'
               % test_name_2)
      data = json.loads(response.get_data(as_text=True))
      self.assertEqual({}, data["anomalies"], 'No anomalies expected')

      # Search for an existing test anomaly, but a different time
      response = self.client.post(
          '/anomalies/find_time',
          data='{"tests":["%s"], "start_time":"Jan 1, 2023 at 4:28:08.609PM",'
           '"end_time":"Jan 4, 2023 at 4:28:08.609PM"}'
               % test_name)
      data = json.loads(response.get_data(as_text=True))
      self.assertEqual({}, data["anomalies"], 'No anomalies expected')

  @mock.patch('application.perf_api.datastore_client'
              '.DataStoreClient.QueryAnomaliesTimestamp')
  def testAnomalyRequestBatching(self, query_mock):
    query_mock.return_value = []
    batch_size = anomalies.DATASTORE_TEST_BATCH_SIZE
    batch_count = 2
    test_count = batch_size*batch_count
    tests = []
    for i in range(test_count):
      tests.append('master/bot/benchmark/test_%i' % i)

    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email

      # Replace the single inverted comma with double to render the json
      test_str = str(tests).replace('\'', '"')
      request_data = '''{"tests":%s,
      "start_time":"Jan 1, 2024 at 4:28:08.609PM",
      "end_time":"Jan 2, 2024 at 4:28:08.609PM"}'''% test_str

      response = self.client.post(
          '/anomalies/find_time',
          data=request_data)
      data = json.loads(response.get_data(as_text=True))
      self.assertEqual({}, data["anomalies"], 'No anomalies expected')
      self.assertEqual(batch_count, query_mock.call_count,
                       'Datastore expected to be queried exactly %i times' %
                       batch_count)

  @mock.patch('application.perf_api.datastore_client'
              '.DataStoreClient.QueryAnomaliesTimestamp')
  def testAnomaliesFound(self, query_mock):
    test_name = 'master/bot/test1/metric'
    client = datastore.Client()

    def create_anomaly(key, start_rev, end_rev):
      test_anomaly = datastore.entity.Entity(key)
      test_anomaly['start_revision'] = start_rev
      test_anomaly['end_revision'] = end_rev
      test_anomaly['test'] = test_key1
      return test_anomaly

    start_rev = 1233
    end_rev = 1234
    start = datetime.datetime(2024, 1, 2)
    end = datetime.datetime(2024, 1, 2)
    test_key1 = client.key('TestMetadata', test_name)
    anomaly_key_1 = client.key('Anomaly', 1111)
    anomaly_key_2 = client.key('Anomaly', 2222)
    test_anomaly_1 = create_anomaly(anomaly_key_1, start_rev, end_rev)
    test_anomaly_2 = create_anomaly(anomaly_key_2, start_rev, end_rev)

    def mock_query(tests, start_time, end_time):
      if test_name in tests and \
         start >= start_time and end <= end_time:
        return [test_anomaly_1, test_anomaly_2]

      return []

    query_mock.side_effect = mock_query
    with mock.patch('application.perf_api.auth_helper.AuthorizeBearerToken') \
        as auth_mock:
      auth_mock.return_value = True, self._test_email
      response = self.client.post(
          '/anomalies/find_time',
          data='{"tests":["%s"], "start_time":"Jan 1, 2024 at 4:28:08.609PM",'
           '"end_time":"Jan 2, 2024 at 4:28:08.609PM"}'
               % test_name)
      data = response.get_data(as_text=True)
      response_data = json.loads(data)
      self.assertIsNotNone(response_data)
      anomaly_list = response_data["anomalies"][test_name]
      self.assertIsNotNone(anomaly_list, 'Anomaly list for test expected.')
      self.assertEqual(2, len(anomaly_list), 'Two anomalies expected in list')
      anomaly_data = anomaly_list[0]
      self.assertEqual(test_name, anomaly_data['test_path'])
      self.assertEqual(test_anomaly_1['start_revision'],
                       anomaly_data['start_revision'])
      self.assertEqual(test_anomaly_1['end_revision'],
                       anomaly_data['end_revision'])


if __name__ == '__main__':
  unittest.main()
