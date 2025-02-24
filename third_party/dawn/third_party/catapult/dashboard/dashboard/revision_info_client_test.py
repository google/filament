# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard import add_histograms_queue
from dashboard import add_histograms_queue_test
from dashboard import revision_info_client
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from google.appengine.ext import ndb
from tracing.value.diagnostics import reserved_infos


class RevisionInfoClient(testing_common.TestCase):

  def _SetRevisionConfig(self):
    namespaced_stored_object.Set(
        revision_info_client.REVISION_INFO_KEY,
        {
            'r_commit_pos': {
                'name': 'Chromium Commit Position',
                'url': 'http://test-results.appspot.com/revision_range'
                       '?start={{R1}}&end={{R2}}&n={{n}}',
            },
        },
    )

  def _CreateRows(self):
    test_path = 'Chromium/win7/suite/metric'
    test_key = utils.TestKey(test_path)
    stat_names_to_test_keys = {
        'avg': utils.TestKey('Chromium/win7/suite/metric_avg'),
        'std': utils.TestKey('Chromium/win7/suite/metric_std'),
        'count': utils.TestKey('Chromium/win7/suite/metric_count'),
        'max': utils.TestKey('Chromium/win7/suite/metric_max'),
        'min': utils.TestKey('Chromium/win7/suite/metric_min'),
        'sum': utils.TestKey('Chromium/win7/suite/metric_sum')
    }
    histograms = add_histograms_queue_test.TEST_HISTOGRAM
    histograms['diagnostics'][
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name]['values'] = [99]
    ndb.put_multi(
        add_histograms_queue.CreateRowEntities(histograms, test_key,
                                               stat_names_to_test_keys, 99))
    histograms['diagnostics'][
        reserved_infos.CHROMIUM_COMMIT_POSITIONS.name]['values'] = [200]
    ndb.put_multi(
        add_histograms_queue.CreateRowEntities(
            add_histograms_queue_test.TEST_HISTOGRAM, test_key,
            stat_names_to_test_keys, 200))

  def testGetRangeRevisionInfo(self):
    self._SetRevisionConfig()
    self._CreateRows()
    info = revision_info_client.GetRangeRevisionInfo(
        utils.TestKey('Chromium/win7/suite/metric'), 100, 200)
    self.assertEqual(info, [{
        'url': 'http://test-results.appspot.com/revision_range'
               '?start=99&end=200&n=1000',
        'name': 'Chromium Commit Position'
    }])

  def testGetRangeRevisionInfo_Suffixed(self):
    self._SetRevisionConfig()
    self._CreateRows()
    info = revision_info_client.GetRangeRevisionInfo(
        utils.TestKey('Chromium/win7/suite/metric_avg'), 100, 200)
    self.assertEqual(info, [{
        'url': 'http://test-results.appspot.com/revision_range'
               '?start=99&end=200&n=1000',
        'name': 'Chromium Commit Position'
    }])

  def testGetRangeRevisionInfo_NotFound(self):
    self._SetRevisionConfig()
    info = revision_info_client.GetRangeRevisionInfo(
        utils.TestKey('Chromium/win7/suite/metric'), 100, 200)
    self.assertEqual(info, [])
