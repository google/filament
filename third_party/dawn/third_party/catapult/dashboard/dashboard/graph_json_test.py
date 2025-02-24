# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import json
import six
import unittest
import webtest

from google.appengine.ext import ndb

from dashboard import graph_json
from dashboard import list_tests
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data


flask_app = Flask(__name__)


@flask_app.route('/graph_json', methods=['POST'])
def GraphJsonPost():
  return graph_json.GraphJsonPost()


class GraphJsonTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    self.PatchDatastoreHooksRequest()

  def _AddTestColumns(self, start_rev=15000, end_rev=16500, step=3):
    """Adds a bunch of test data to the mock datastore.

    In particular, add Rows with revisions in the given range (but skipping
    some numbers, so the revisions are non-contiguous) under the dromaeo/dom
    test for winXP, win7, mac.

    Args:
      start_rev: Starting revision number.
      end_rev: Ending revision number.
      step: Difference between adjacent revisions.
    """
    master = graph_data.Master(id='ChromiumGPU')
    master.put()
    bots = []
    rows = []
    for name in ['winXP', 'win7', 'mac']:
      bot = graph_data.Bot(id=name, parent=master.key)
      bot.put()
      bots.append(bot)
      test = graph_data.TestMetadata(id='ChromiumGPU/%s/dromaeo' % name)
      test.UpdateSheriff()
      test.put()
      for sub_name in ['dom', 'jslib']:
        sub_test = graph_data.TestMetadata(
            id='%s/%s' % (test.key.id(), sub_name),
            improvement_direction=anomaly.UP,
            has_rows=True)
        sub_test.UpdateSheriff()
        sub_test.put()
        test_container_key = utils.GetTestContainerKey(sub_test)
        for i in range(start_rev, end_rev, step):
          # Add Rows for one bot with revision numbers that aren't lined up
          # with the other bots.
          rev = i + 1 if name == 'mac' else i
          row = graph_data.Row(
              parent=test_container_key,
              id=rev,
              value=float(i * 2),
              r_webkit=int(i * 0.25),
              a_str='some_string',
              buildnumber=i - start_rev,
              a_tracing_uri='http://trace/%d' % i)
          rows.append(row)
    ndb.put_multi(rows)

  def _AddLongTestColumns(self, start_rev=15000, end_rev=16500, step=3):
    """Adds test data with long nested sub test to the mock datastore.

    Args:
      start_rev: Starting revision number.
      end_rev: Ending revision number.
      step: Difference between adjacent revisions.
    """
    master = graph_data.Master(id='master')
    master.put()
    bot = graph_data.Bot(id='bot', parent=master.key)
    bot.put()
    test = graph_data.TestMetadata(id='master/bot/suite')
    test.UpdateSheriff()
    test.put()

    rows = []
    path = 'master/bot/suite'
    for sub_name in ['sub1', 'sub2', 'sub3', 'sub4', 'sub5']:
      path = '%s/%s' % (path, sub_name)
      test = graph_data.TestMetadata(
          id=path, improvement_direction=anomaly.UP, has_rows=True)
      test.UpdateSheriff()
      test.put()
      test_container_key = utils.GetTestContainerKey(test.key)
      for i in range(start_rev, end_rev, step):
        row = graph_data.Row(
            parent=test_container_key,
            id=i,
            value=float(i * 2),
            r_webkit=int(i * 0.25),
            a_str='some_string',
            buildnumber=i - start_rev,
            a_tracing_uri='http://trace/%d' % i)
        rows.append(row)
    ndb.put_multi(rows)

  def _GetSeriesIndex(self, flot, test_path):
    series = flot['annotations']['series']
    for index in series:
      if series[index]['path'] == test_path:
        return index
    return None

  def CheckFlotJson(self,
                    json_str,
                    num_rows,
                    num_cols,
                    start_rev,
                    end_rev,
                    step=3):
    """Checks whether a JSON string output by GetGraphJson is correct.

    It's assumed that data should match data that might be added by the
    _AddTestColumns method above.

    In general, the Flot JSON should at contain a dict with the key 'data'
    which is mapped to a list of dicts (which represent data series), each of
    which also has a key called 'data', which is mapped to a list of 2-element
    lists (which represent points). For example:

        {data: {data: [[1, 10], [2, 20]]}, {data: [[3, 30], [4, 40]]}}

    Args:
      json_str: The JSON string to check.
      num_rows: The expected number of points in each trace.
      num_cols: The expected number of trace lines.
      start_rev: Starting revision number.
      end_rev: End revision number.
      step: Expected difference between adjacent revision numbers.
    """
    try:
      flot = json.loads(json_str)
    except ValueError:
      self.fail('GetGraphJson returned invalid JSON')

    data = flot.get('data')
    if not data:
      self.fail('No flot data generated by GetGraphJson')
    self.assertEqual(num_cols, len(data))
    for key in data:
      col = data[key]
      if not col.get('data'):
        self.fail('No flot columns generated by GetGraphJson')
      self.assertEqual(num_rows, len(col['data']))
      for index, rev in enumerate(range(start_rev, end_rev, step)):
        self.assertEqual(rev, col['data'][index][0])
        self.assertEqual(rev * 2, col['data'][index][1])

  def testPost_ValidRequest(self):
    self._AddTestColumns(start_rev=15700, end_rev=16000, step=1)
    graphs = {
        'test_path_dict': {
            'ChromiumGPU/winXP/dromaeo/dom': [],
            'ChromiumGPU/winXP/dromaeo/jslib': [],
        }
    }
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    self.CheckFlotJson(flot_json_str, 150, 2, 15850, 16000, step=1)
    self.assertEqual('*', response.headers.get('Access-Control-Allow-Origin'))

  def testPost_NanFiltered(self):
    self._AddTestColumns(start_rev=15700, end_rev=16000, step=1)

    test_key = utils.OldStyleTestKey('ChromiumGPU/win7/dromaeo/jslib')
    row_key = utils.GetRowKey(test_key, 15900)
    row = row_key.get()
    row.value = float('nan')
    row.put()

    graphs = {'test_path_dict': {'ChromiumGPU/win7/dromaeo/jslib': [],}}
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    rows = json.loads(flot_json_str)['data']['0']['data']
    self.assertEqual(149, len(rows))

  def testPost_InvalidRequest_ReportsError(self):
    self.testapp.post('/graph_json', {}, status=500)
    self.testapp.post('/graph_json', {'graphs': ''}, status=500)
    self.testapp.post('/graph_json', {'graphs': '{}'}, status=500)

  def testPost_LongTestPathWithSelected(self):
    self._AddLongTestColumns(start_rev=15700, end_rev=16000, step=1)
    graphs = {
        'test_path_dict': {
            'master/bot/suite/sub1/sub2/sub3/sub4/sub5': ['sub5']
        },
        'is_selected': True
    }
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    self.CheckFlotJson(flot_json_str, 150, 1, 15850, 16000, step=1)

  def testPost_LongTestPathWithUnSelected(self):
    self._AddLongTestColumns(start_rev=15700, end_rev=16000, step=1)
    graphs = {
        'test_path_dict': {
            'master/bot/suite/sub1/sub2/sub3/sub4': ['sub4']
        }
    }
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    self.CheckFlotJson(flot_json_str, 150, 1, 15850, 16000, step=1)

  def testPost_LongTestPathWithUnSelectedAndNoSubTest_NoGraphData(self):
    self._AddLongTestColumns(start_rev=15700, end_rev=16000, step=1)
    graphs = {
        'test_path_dict': {
            'master/bot/suite/sub1/sub2/sub3/sub4/sub5': ['sub5']
        },
    }
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    flot = json.loads(flot_json_str)
    self.assertEqual(0, len(flot['data']))

  def testRequest_NoSubTest_ShowsSummaryTests(self):
    """Tests the post method of the request handler."""
    self._AddTestColumns(start_rev=15700, end_rev=16000, step=1)
    graphs = {'test_path_dict': {'ChromiumGPU/winXP/dromaeo': [],}}
    # If the request is valid, a valid response will be returned.
    response = self.testapp.post('/graph_json', {'graphs': json.dumps(graphs)})
    flot_json_str = response.body
    self.CheckFlotJson(flot_json_str, 150, 2, 15850, 16000, step=1)

  def testGetGraphJsonNoArgs(self):
    self._AddTestColumns(start_rev=16047)
    flot_json_str = graph_json.GetGraphJson(
        {'ChromiumGPU/win7/dromaeo/dom': []})
    self.CheckFlotJson(flot_json_str, 150, 1, 16050, 16500)

  def testGetGraphJsonRevisionStart(self):
    self._AddTestColumns(end_rev=15500)
    flot_json_str = graph_json.GetGraphJson(
        {'ChromiumGPU/win7/dromaeo/dom': []}, rev=15000)
    self.CheckFlotJson(flot_json_str, 76, 1, 15000, 15228)

  def testGetGraphJsonRevisionMiddle(self):
    self._AddTestColumns(end_rev=15600)
    flot_json_str = graph_json.GetGraphJson(
        {'ChromiumGPU/win7/dromaeo/dom': []}, rev=15300)
    self.CheckFlotJson(flot_json_str, 151, 1, 15075, 15525)

  def testGetGraphJsonNumPoints(self):
    self._AddTestColumns(end_rev=15500)
    flot_json_str = graph_json.GetGraphJson(
        {'ChromiumGPU/win7/dromaeo/dom': []}, rev=15300, num_points=8)
    self.CheckFlotJson(flot_json_str, 9, 1, 15288, 15315)

  def testGetGraphJsonStartEndRev(self):
    self._AddTestColumns(start_rev=15991)
    flot_json_str = graph_json.GetGraphJson(
        {'ChromiumGPU/win7/dromaeo/dom': []}, start_rev=16000, end_rev=16030)
    self.CheckFlotJson(flot_json_str, 11, 1, 16000, 16030)

  def testGetGraphJsonMultipleBots(self):
    self._AddTestColumns(start_rev=16047)
    flot_json_str = graph_json.GetGraphJson({
        'ChromiumGPU/win7/dromaeo/dom': [],
        'ChromiumGPU/winXP/dromaeo/dom': [],
    })
    self.CheckFlotJson(flot_json_str, 150, 2, 16050, 16500)

  def testGetGraphJsonMultipleTests(self):
    self._AddTestColumns(start_rev=16047)
    flot_json_str = graph_json.GetGraphJson({
        'ChromiumGPU/win7/dromaeo/dom': [],
        'ChromiumGPU/win7/dromaeo/jslib': [],
    })
    self.CheckFlotJson(flot_json_str, 150, 2, 16050, 16500)

  def testGetGraphJsonError(self):
    self._AddTestColumns(start_rev=15000, end_rev=15015)

    rows = graph_data.Row.query(graph_data.Row.parent_test == ndb.Key(
        'TestMetadata', 'ChromiumGPU/win7/dromaeo/dom'))
    for row in rows:
      row.error = 1 + ((row.revision - 15000) * 0.25)
    ndb.put_multi(rows)
    flot_json_str = graph_json.GetGraphJson({
        'ChromiumGPU/win7/dromaeo/dom': [],
    })
    flot = json.loads(flot_json_str)
    self.assertEqual(1, len(list(flot['error_bars'].keys())))
    rev = 0
    for col_dom, col_top, col_bottom in zip(flot['data']['0']['data'],
                                            flot['error_bars']['0'][1]['data'],
                                            flot['error_bars']['0'][0]['data']):
      error = 1 + (rev * 0.25)
      self.assertEqual(rev + 15000, col_top[0])
      self.assertEqual(col_dom[1] + error, col_top[1])
      self.assertEqual(rev + 15000, col_bottom[0])
      self.assertEqual(col_dom[1] - error, col_bottom[1])
      rev += 3

  def testGetGraphJsonSkewedRevisions(self):
    self._AddTestColumns(end_rev=15500)
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        rev=15000,
        num_points=8)
    flot = json.loads(json_str)
    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    if not data:
      self.fail('No flot data generated by GetGraphJson')
    self.assertEqual(2, len(data))
    self.assertEqual([[15000, 30000.0], [15003, 30006.0], [15006, 30012.0]],
                     data[win7_index].get('data'))
    self.assertEqual([[15001, 30000.0], [15004, 30006.0]],
                     data[mac_index].get('data'))

  def testGetGraphJson_ClampsRevisions(self):
    self._AddTestColumns(end_rev=15500)
    # No revision specified, clamps to the last 9 rows.
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        num_points=8)
    flot = json.loads(json_str)

    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    # Two columns
    self.assertEqual(2, len(data))
    # Clamped from 15487-15499 by 3 steps.  First row doesn't contain 15487.
    self.assertEqual([[15489, 30978.0], [15492, 30984.0], [15495, 30990.0],
                      [15498, 30996.0]], data[win7_index].get('data'))
    self.assertEqual([[15487, 30972.0], [15490, 30978.0], [15493, 30984.0],
                      [15496, 30990.0], [15499, 30996.0]],
                     data[mac_index].get('data'))

    # Revision 100 (way before data starts) specified, clamp to the first 8 rows
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        rev=100,
        num_points=8)
    flot = json.loads(json_str)
    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    # Two columns
    self.assertEqual(2, len(data))
    # 15000-15012.
    self.assertEqual([[15000, 30000.0], [15003, 30006.0], [15006, 30012.0],
                      [15009, 30018.0]], data[win7_index].get('data'))
    self.assertEqual([[15001, 30000.0], [15004, 30006.0], [15007, 30012.0],
                      [15010, 30018.0]], data[mac_index].get('data'))

    # Revision 15530 (valid) specified, clamp 4 rows before/after
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        rev=15030,
        num_points=8)
    flot = json.loads(json_str)
    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    # Two columns
    self.assertEqual(2, len(data))
    # 15524-15536.
    self.assertEqual([[15024, 30048.0], [15027, 30054.0], [15030, 30060.0],
                      [15033, 30066.0], [15036, 30072.0]],
                     data[win7_index].get('data'))
    self.assertEqual([[15025, 30048.0], [15028, 30054.0], [15031, 30060.0],
                      [15034, 30066.0]], data[mac_index].get('data'))

    # Revision 15498 specified, clamp 4 rows before and after is cut off
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        rev=15498,
        num_points=7)
    flot = json.loads(json_str)
    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    # Two columns
    self.assertEqual(2, len(data))

    # 15493-15499.
    self.assertEqual([[15495, 30990.0], [15498, 30996.0]],
                     data[win7_index].get('data'))
    self.assertEqual([[15493, 30984.0], [15496, 30990.0], [15499, 30996.0]],
                     data[mac_index].get('data'))

    # Revision 15001 specified, before is cut off and clamp 4 rows after
    json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
        },
        rev=15001,
        num_points=8)
    flot = json.loads(json_str)
    data = flot.get('data')
    win7_index = self._GetSeriesIndex(flot, 'ChromiumGPU/win7/dromaeo/dom')
    mac_index = self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom')

    # Two columns
    self.assertEqual(2, len(data))

    # 15493-15499.
    self.assertEqual([[15000, 30000.0], [15003, 30006.0], [15006, 30012.0]],
                     data[win7_index].get('data'))
    self.assertEqual([[15001, 30000.0], [15004, 30006.0], [15007, 30012.0]],
                     data[mac_index].get('data'))

  def testGetGraphJson_GraphJsonAnnotations(self):
    self._AddTestColumns(end_rev=15500)
    flot_json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
        }, rev=15000, num_points=8)
    flot = json.loads(flot_json_str)
    annotations = flot['annotations']
    self.assertEqual(5, len(flot['data']['0']['data']))
    for i, _ in enumerate(flot['data']['0']['data']):
      rev = flot['data']['0']['data'][i][0]
      self.assertEqual(
          int(int(rev) * 0.25), annotations['0'][str(i)]['r_webkit'])

  def testGetGraphJson_WithAnomalies_ReturnsCorrectAnomalyAnnotations(self):
    self._AddTestColumns()

    anomaly1 = anomaly.Anomaly(
        start_revision=14999,
        end_revision=15000,
        test=utils.TestKey('ChromiumGPU/win7/dromaeo/dom'),
        median_before_anomaly=100,
        median_after_anomaly=200)
    anomaly1.SetIsImprovement()
    key1 = anomaly1.put()

    anomaly2 = anomaly.Anomaly(
        start_revision=15004,
        end_revision=15006,
        test=utils.TestKey('ChromiumGPU/win7/dromaeo/dom'),
        median_before_anomaly=200,
        median_after_anomaly=100,
        bug_id=12345)
    anomaly2.SetIsImprovement()
    key2 = anomaly2.put()

    old_style_test_key = ndb.Key('Master', 'ChromiumGPU', 'Bot', 'win7', 'Test',
                                 'dromaeo', 'Test', 'dom')
    anomaly3 = anomaly.Anomaly(
        start_revision=15008,
        end_revision=15009,
        test=old_style_test_key,
        median_before_anomaly=100,
        median_after_anomaly=200)
    key3 = anomaly3.put()

    test = utils.TestKey('ChromiumGPU/win7/dromaeo/dom').get()
    test.description = 'About this test'
    test.units = 'ms'
    test.buildername = 'Windows 7 (1)'
    test.UpdateSheriff()
    test.put()

    flot_json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
        }, rev=15000, num_points=8)

    flot = json.loads(flot_json_str)
    annotations = flot['annotations']
    self.assertEqual(5, len(annotations['0']))

    # Verify key fields of the annotation dictionary for the first anomaly.
    anomaly_one_annotation = annotations['0']['0']['g_anomaly']
    self.assertEqual(14999, anomaly_one_annotation['start_revision'])
    self.assertEqual(15000, anomaly_one_annotation['end_revision'])
    self.assertEqual('100.0%', anomaly_one_annotation['percent_changed'])
    self.assertIsNone(anomaly_one_annotation['bug_id'])
    self.assertEqual(
        six.ensure_str(key1.urlsafe()), anomaly_one_annotation['key'])
    self.assertTrue(anomaly_one_annotation['improvement'])

    # Verify key fields of the annotation dictionary for the second anomaly.
    anomaly_two_annotation = annotations['0']['2']['g_anomaly']
    self.assertEqual(15004, anomaly_two_annotation['start_revision'])
    self.assertEqual(15006, anomaly_two_annotation['end_revision'])
    self.assertEqual('50.0%', anomaly_two_annotation['percent_changed'])
    self.assertEqual(12345, anomaly_two_annotation['bug_id'])
    self.assertEqual(
        six.ensure_str(key2.urlsafe()), anomaly_two_annotation['key'])
    self.assertFalse(anomaly_two_annotation['improvement'])

    # Verify the key for the third anomaly.
    anomaly_three_annotation = annotations['0']['3']['g_anomaly']
    self.assertEqual(
        six.ensure_str(key3.urlsafe()), anomaly_three_annotation['key'])

    # Verify the tracing link annotations
    self.assertEqual('http://trace/15000',
                     annotations['0']['0']['a_tracing_uri'])
    self.assertEqual('http://trace/15012',
                     annotations['0']['4']['a_tracing_uri'])

    # Verify the series annotations.
    self.assertEqual(
        {
            '0': {
                'name': 'dom',
                'path': 'ChromiumGPU/win7/dromaeo/dom',
                'units': 'ms',
                'better': 'Higher',
                'description': 'About this test',
                'can_bisect': True,
            }
        }, annotations['series'])

  def testGetGraphJson_SomeDataDeprecated_OmitsDeprecatedData(self):
    self._AddTestColumns(start_rev=15000, end_rev=15050)
    dom = utils.TestKey('ChromiumGPU/win7/dromaeo/dom').get()
    dom.deprecated = True
    dom.put()
    jslib = utils.TestKey('ChromiumGPU/win7/dromaeo/jslib').get()
    jslib.deprecated = True
    jslib.put()

    flot_json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/dom': [],
            'ChromiumGPU/win7/dromaeo/jslib': [],
            'ChromiumGPU/mac/dromaeo/dom': [],
            'ChromiumGPU/mac/dromaeo/jslib': [],
        },
        rev=15000,
        num_points=8)
    flot = json.loads(flot_json_str)
    # The win7 tests are deprecated and the mac tests are not. So only the mac
    # tests should be returned.
    self.assertEqual(2, len(flot['data']))
    self.assertEqual(2, len(flot['annotations']['series']))
    self.assertIsNotNone(
        self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/dom'))
    self.assertIsNotNone(
        self._GetSeriesIndex(flot, 'ChromiumGPU/mac/dromaeo/jslib'))

  def testGetGraphJson_WithSelectedTrace(self):
    self._AddTestColumns(start_rev=15000, end_rev=15050)
    rows = graph_data.Row.query(
        graph_data.Row.parent_test == utils.OldStyleTestKey(
            'ChromiumGPU/win7/dromaeo/jslib')).fetch()
    for row in rows:
      row.error = 1 + ((row.revision - 15000) * 0.25)
    ndb.put_multi(rows)

    flot_json_str = graph_json.GetGraphJson(
        {
            'ChromiumGPU/win7/dromaeo/jslib': ['jslib'],
        },
        rev=15000,
        num_points=8,
        is_selected=True)
    flot = json.loads(flot_json_str)

    self.assertEqual(1, len(flot['data']))
    self.assertEqual(5, len(flot['data']['0']['data']))
    self.assertEqual(1, len(flot['annotations']['series']))
    self.assertEqual(5, len(list(flot['annotations'].get('0').keys())))
    self.assertEqual(5, len(flot['error_bars']['0'][0]['data']))
    self.assertEqual(5, len(flot['error_bars']['0'][1]['data']))

  def testGetGraphJson_UnSelectedTrace(self):
    self._AddTestColumns(start_rev=15000, end_rev=15050)
    test_key = ndb.Key('TestMetadata', 'ChromiumGPU/win7/dromaeo/jslib')
    rows = graph_data.Row.query(graph_data.Row.parent_test == test_key).fetch()
    for row in rows:
      row.error = 1 + ((row.revision - 15000) * 0.25)
    ndb.put_multi(rows)

    # Insert sub tests to jslib.
    rows = []
    start_rev = 15000
    end_rev = 15050
    for name in ['sub_test_a', 'sub_test_b']:
      sub_test = graph_data.TestMetadata(
          id='%s/%s' % (test_key.id(), name),
          improvement_direction=anomaly.UP,
          has_rows=True)
      sub_test.UpdateSheriff()
      sub_test.put()

      sub_test_container_key = utils.GetTestContainerKey(sub_test)
      for i in range(start_rev, end_rev, 3):
        # Add Rows for one bot with revision numbers that aren't lined up
        # with the other bots.
        row = graph_data.Row(
            parent=sub_test_container_key,
            id=i,
            value=float(i * 2),
            r_webkit=int(i * 0.25),
            a_str='some_string',
            buildnumber=i - start_rev,
            a_tracing_uri='http://trace/%d' % i)
        rows.append(row)
    ndb.put_multi(rows)

    paths = list_tests.GetTestsForTestPathDict(
        {
            'ChromiumGPU/win7/dromaeo/jslib': ['jslib'],
        }, False)['tests']
    flot_json_str = graph_json.GetGraphJson(
        paths, rev=15000, num_points=8, is_selected=False)
    flot = json.loads(flot_json_str)

    sub_test_a_index = self._GetSeriesIndex(
        flot, 'ChromiumGPU/win7/dromaeo/jslib/sub_test_a')
    sub_test_b_index = self._GetSeriesIndex(
        flot, 'ChromiumGPU/win7/dromaeo/jslib/sub_test_b')

    self.assertEqual(2, len(flot['data']))
    self.assertEqual(5, len(flot['data'][sub_test_a_index]['data']))
    self.assertEqual(2, len(flot['annotations']['series']))
    self.assertEqual(
        5, len(list(flot['annotations'].get(sub_test_a_index).keys())))
    self.assertEqual(
        5, len(list(flot['annotations'].get(sub_test_b_index).keys())))

  def testGetGraphJson_ManyUnselected_ReturnsNothing(self):
    testing_common.AddTests(['M'], ['b'],
                            {'suite': {str(i): {} for i in range(100)}})
    test_paths = ['M/b/suite/%s' % i for i in range(100)]
    for p in test_paths:
      testing_common.AddRows(p, [1])
    path_list = list_tests.GetTestsForTestPathDict({p: [] for p in test_paths},
                                                   False)['tests']
    response = graph_json.GetGraphJson(path_list, is_selected=False)
    self.assertEqual({
        'data': {},
        'annotations': {},
        'error_bars': {}
    }, json.loads(response))


class GraphJsonParseRequestArgumentsTest(testing_common.TestCase):

  def testParseRequestArguments(self):
    # The numerical arguments get converted to integers, and the
    # unspecified arguments get set to None.
    params = {
        'test_path_dict': {
            'Master/b1/scrolling/frame_times/about.com': [],
            'Master/b2/scrolling/frame_times/about.com': [],
            'Master/linux/dromaeo.domcoremodify/dom': [],
        }
    }
    params.update(rev='12345', num_points='123')
    expected = {
        'test_paths': [
            'Master/b1/scrolling/frame_times/about.com',
            'Master/b2/scrolling/frame_times/about.com',
            'Master/linux/dromaeo.domcoremodify/dom'
        ],
        'rev': 12345,
        'num_points': 123,
        'start_rev': None,
        'end_rev': None,
        'is_selected': None,
    }
    actual = graph_json._ParseRequestArguments(json.dumps(params))
    self.assertCountEqual(expected, actual)

  def testParseRequestArguments_TestPathListSpecified(self):
    params = {
        'test_path_dict': {
            'Master/b1/scrolling/frame_times/about.com': [],
            'Master/b2/scrolling/frame_times/about.com': [],
            'Master/linux/dromaeo.domcoremodify/dom': [],
        }
    }
    params.update(
        test_path_dict=None,
        test_path_list=[
            'Master/b1/scrolling/frame_times/about.com',
            'Master/b2/scrolling/frame_times/about.com',
            'Master/linux/dromaeo.domcoremodify/dom'
        ])

    expected = {
        'test_paths': [
            'Master/b1/scrolling/frame_times/about.com',
            'Master/b2/scrolling/frame_times/about.com',
            'Master/linux/dromaeo.domcoremodify/dom'
        ],
        'rev': None,
        'num_points': 150,
        'start_rev': None,
        'end_rev': None,
        'is_selected': None,
    }
    actual = graph_json._ParseRequestArguments(json.dumps(params))
    self.assertCountEqual(expected, actual)

  def testParseRequestArguments_OnlyTestPathDictSpecified(self):
    # No revision or number of points is specified, so they're set to None.
    params = {
        'test_path_dict': {
            'Master/b1/scrolling/frame_times/about.com': [],
            'Master/b2/scrolling/frame_times/about.com': [],
            'Master/linux/dromaeo.domcoremodify/dom': [],
        }
    }
    expected = {
        'test_paths': [
            'Master/b1/scrolling/frame_times/about.com',
            'Master/b2/scrolling/frame_times/about.com',
            'Master/linux/dromaeo.domcoremodify/dom',
        ],
        'rev': None,
        'num_points': graph_json._DEFAULT_NUM_POINTS,
        'start_rev': None,
        'end_rev': None,
        'is_selected': None,
    }
    actual = graph_json._ParseRequestArguments(json.dumps(params))
    self.assertCountEqual(expected, actual)

  def testParseRequestArguments_NegativeRevision(self):
    # Negative revision is invalid; it's the same as no revision.
    params = {
        'test_path_dict': {
            'Master/b1/scrolling/frame_times/about.com': [],
            'Master/b2/scrolling/frame_times/about.com': [],
            'Master/linux/dromaeo.domcoremodify/dom': [],
        }
    }
    params.update(rev='-1')
    expected = {
        'test_paths': [
            'Master/b1/scrolling/frame_times/about.com',
            'Master/b2/scrolling/frame_times/about.com',
            'Master/linux/dromaeo.domcoremodify/dom',
        ],
        'rev': None,
        'num_points': graph_json._DEFAULT_NUM_POINTS,
        'start_rev': None,
        'end_rev': None,
        'is_selected': None,
    }
    actual = graph_json._ParseRequestArguments(json.dumps(params))
    self.assertCountEqual(expected, actual)


class GraphJsonHelperFunctionTest(testing_common.TestCase):

  def testPointInfoDict_StdioUriMarkdown(self):
    testing_common.AddTests(['Master'], ['b'], {'my_suite': {}})
    test = utils.TestKey('Master/b/my_suite').get()
    test.buildername = 'MyBuilder'
    test_container_key = utils.GetTestContainerKey(test)
    row = graph_data.Row(id=345, buildnumber=456, parent=test_container_key)
    # Test buildbot format
    row.a_stdio_uri = ('[Buildbot stdio]('
                       'http://build.chromium.org/p/my.master.id/'
                       'builders/MyBuilder%20%281%29/builds/456/steps/'
                       'my_suite/logs/stdio)')
    point_info = graph_json._PointInfoDict(row, {})

    # Test non-buildbot format
    row.a_stdio_uri = '[Buildbot stdio](http://unkonwn/type)'
    point_info = graph_json._PointInfoDict(row, {})
    self.assertEqual(row.a_stdio_uri, point_info['a_stdio_uri'])
    self.assertIsNone(point_info.get('a_buildbot_status_page'))

  def testPointInfoDict_BuildUri_NoBuildbotUri(self):
    testing_common.AddTests(['Master'], ['b'], {'my_suite': {}})
    test = utils.TestKey('Master/b/my_suite').get()
    test.buildername = 'MyBuilder'
    test_container_key = utils.GetTestContainerKey(test)
    row = graph_data.Row(id=345, buildnumber=456, parent=test_container_key)
    row.a_build_uri = ('[Build](http://foo/bar)')
    point_info = graph_json._PointInfoDict(row, {})
    self.assertIsNone(point_info.get('a_buildbot_status_page'))
    self.assertEqual(row.a_build_uri, point_info['a_build_uri'])

  def testPointInfoDict_RowHasNoTracingUri_ResultHasNoTracingUri(self):
    testing_common.AddTests(['Master'], ['b'], {'my_suite': {}})
    rows = testing_common.AddRows('Master/b/my_suite', [345])
    # This row has no a_tracing_uri property, so there should be no
    # trace annotation returned by _PointInfoDict.
    point_info = graph_json._PointInfoDict(rows[0], {})
    self.assertFalse(hasattr(rows[0], 'a_tracing_uri'))
    self.assertNotIn('a_tracing_uri', point_info)


if __name__ == '__main__':
  unittest.main()
