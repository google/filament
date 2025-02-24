# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
from flask import Flask
import json
import unittest
import webtest

from dashboard import graph_revisions
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data

flask_app = Flask(__name__)


@flask_app.route('/graph_revisions', methods=['POST'])
def GraphRevisionsPost():
  return graph_revisions.GraphRevisionsPost()


class GraphRevisionsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    self.PatchDatastoreHooksRequest()

  def _AddMockData(self):
    """Adds mock data to the datastore, not updating stored_object."""
    master_key = graph_data.Master(id='ChromiumPerf').put()
    for bot_name in ['win7', 'mac']:
      graph_data.Bot(id=bot_name, parent=master_key).put()
      t = graph_data.TestMetadata(id='ChromiumPerf/%s/dromaeo' % bot_name)
      t.UpdateSheriff()
      t.put()

      subtest_key = graph_data.TestMetadata(
          id='ChromiumPerf/%s/dromaeo/dom' % bot_name, has_rows=True)
      subtest_key.UpdateSheriff()
      subtest_key.put()

      test_container_key = utils.GetTestContainerKey(subtest_key)
      for rev in range(15000, 16000, 5):
        row = graph_data.Row(
            parent=test_container_key, id=rev, value=float(rev * 2.5))
        row.timestamp = datetime.datetime(2013, 8, 1)
        row.put()

  def testPost_ReturnsAndCachesCorrectRevisions(self):
    self._AddMockData()
    response = self.testapp.post('/graph_revisions',
                                 {'test_path': 'ChromiumPerf/win7/dromaeo/dom'})

    cached_rows = stored_object.Get(
        'externally_visible__num_revisions_ChromiumPerf/win7/dromaeo/dom')
    for index, row in enumerate(json.loads(response.body)):
      expected_rev = 15000 + (index * 5)
      expected_value = int(expected_rev) * 2.5
      expected_timestamp = utils.TimestampMilliseconds(
          datetime.datetime(2013, 8, 1))
      self.assertEqual([expected_rev, expected_value, expected_timestamp], row)
      self.assertEqual([expected_rev, expected_value, expected_timestamp],
                       cached_rows[index])

  def testPost_CacheSet_ReturnsCachedRevisions(self):
    stored_object.Set(
        'externally_visible__num_revisions_ChromiumPerf/win7/dromaeo/dom',
        [[1, 2, 3]])
    response = self.testapp.post('/graph_revisions',
                                 {'test_path': 'ChromiumPerf/win7/dromaeo/dom'})
    self.assertEqual([[1, 2, 3]], json.loads(response.body))

  def testPost_CacheSet_NanBecomesNone(self):
    stored_object.Set(
        'externally_visible__num_revisions_ChromiumPerf/win7/dromaeo/dom',
        [[1, 2, 3], [4, float('nan'), 6]])
    response = self.testapp.post('/graph_revisions',
                                 {'test_path': 'ChromiumPerf/win7/dromaeo/dom'})
    self.assertEqual([[1, 2, 3], [4, None, 6]], json.loads(response.body))

  def testAddRowsToCache(self):
    self._AddMockData()
    rows = []

    stored_object.Set(
        'externally_visible__num_revisions_ChromiumPerf/win7/dromaeo/dom',
        [[10, 2, 3], [15, 4, 5], [100, 6, 7]])

    test_key = utils.TestKey('ChromiumPerf/win7/dromaeo/dom')
    test_container_key = utils.GetTestContainerKey(test_key)
    ts1 = datetime.datetime(2013, 1, 1)
    row1 = graph_data.Row(
        parent=test_container_key, id=1, value=9, timestamp=ts1)
    rows.append(row1)
    ts2 = datetime.datetime(2013, 1, 2)
    row2 = graph_data.Row(
        parent=test_container_key, id=12, value=90, timestamp=ts2)
    rows.append(row2)
    ts3 = datetime.datetime(2013, 1, 3)
    row3 = graph_data.Row(
        parent=test_container_key, id=102, value=99, timestamp=ts3)
    rows.append(row3)
    graph_revisions.AddRowsToCache(rows)

    self.assertEqual(
        [[1, 9, utils.TimestampMilliseconds(ts1)], [10, 2, 3],
         [12, 90, utils.TimestampMilliseconds(ts2)], [15, 4, 5], [100, 6, 7],
         [102, 99, utils.TimestampMilliseconds(ts3)]],
        stored_object.Get('externally_visible__num_revisions_'
                          'ChromiumPerf/win7/dromaeo/dom'))


if __name__ == '__main__':
  unittest.main()
