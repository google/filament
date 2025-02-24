# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard import add_point_queue
from dashboard.common import testing_common
from dashboard.models import anomaly
from dashboard.models import graph_data


class GetOrCreateAncestorsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.SetCurrentUser('foo@bar.com', is_admin=True)

  def testGetOrCreateAncestors_GetsExistingEntities(self):
    master_key = graph_data.Master(id='ChromiumPerf', parent=None).put()
    graph_data.Bot(id='win7', parent=master_key).put()
    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo',)
    t.UpdateSheriff()
    t.put()

    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo/dom')
    t.UpdateSheriff()
    t.put()

    t = graph_data.TestMetadata(id='ChromiumPerf/win7/dromaeo/dom/modify')
    t.UpdateSheriff()
    t.put()

    actual_parent = add_point_queue.GetOrCreateAncestors(
        'ChromiumPerf', 'win7', 'dromaeo/dom/modify')
    self.assertEqual('ChromiumPerf/win7/dromaeo/dom/modify',
                     actual_parent.key.id())
    # No extra TestMetadata or Bot objects should have been added to the
    # database beyond the four that were put in before the _GetOrCreateAncestors
    # call.
    self.assertEqual(1, len(graph_data.Master.query().fetch()))
    self.assertEqual(1, len(graph_data.Bot.query().fetch()))
    self.assertEqual(3, len(graph_data.TestMetadata.query().fetch()))

  def testGetOrCreateAncestors_CreatesAllExpectedEntities(self):
    parent = add_point_queue.GetOrCreateAncestors('ChromiumPerf', 'win7',
                                                  'dromaeo/dom/modify')
    self.assertEqual('ChromiumPerf/win7/dromaeo/dom/modify', parent.key.id())
    # Check that all the Bot and TestMetadata entities were correctly added.
    created_masters = graph_data.Master.query().fetch()
    created_bots = graph_data.Bot.query().fetch()
    created_tests = graph_data.TestMetadata.query().fetch()
    self.assertEqual(1, len(created_masters))
    self.assertEqual(1, len(created_bots))
    self.assertEqual(3, len(created_tests))
    self.assertEqual('ChromiumPerf', created_masters[0].key.id())
    self.assertIsNone(created_masters[0].key.parent())
    self.assertEqual('win7', created_bots[0].key.id())
    self.assertEqual('ChromiumPerf', created_bots[0].key.parent().id())
    self.assertEqual('ChromiumPerf/win7/dromaeo', created_tests[0].key.id())
    self.assertIsNone(created_tests[0].parent_test)
    self.assertEqual('win7', created_tests[0].bot_name)
    self.assertEqual('dom', created_tests[1].test_part1_name)
    self.assertEqual('ChromiumPerf/win7/dromaeo',
                     created_tests[1].parent_test.id())
    self.assertIsNone(created_tests[1].bot)
    self.assertEqual('ChromiumPerf/win7/dromaeo/dom/modify',
                     created_tests[2].key.id())
    self.assertEqual('ChromiumPerf/win7/dromaeo/dom',
                     created_tests[2].parent_test.id())
    self.assertIsNone(created_tests[2].bot)

  def testGetOrCreateAncestors_RespectsImprovementDirectionForNewTest(self):
    test = add_point_queue.GetOrCreateAncestors(
        'M', 'b', 'suite/foo', units='bogus', improvement_direction=anomaly.UP)
    self.assertEqual(anomaly.UP, test.improvement_direction)

  def testGetOrCreateAncestors_UpdatesAllExpectedEntities(self):
    # pylint: disable=line-too-long
    t = graph_data.TestMetadata(
        id='WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests',
        internal_only=True)
    t.UpdateSheriff()
    t.put()

    t = graph_data.TestMetadata(
        id='WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests/render_frame_rate_fps',
        internal_only=True)
    t.UpdateSheriff()
    t.put()

    master_key = graph_data.Master(id='WebRTCPerf', parent=None).put()
    graph_data.Bot(
        id='android32-pixel5-android11', parent=master_key,
        internal_only=False).put()

    test_path = 'WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests/render_frame_rate_fps/foreman_cif_delay_50_0_plr_5_flexfec'
    test_path_parts = test_path.split('/')
    master = test_path_parts[0]
    bot = test_path_parts[1]
    full_test_name = '/'.join(test_path_parts[2:])
    internal_only = graph_data.Bot.GetInternalOnlySync(master, bot)

    parent = add_point_queue.GetOrCreateAncestors(
        master, bot, full_test_name, internal_only=internal_only)
    self.assertEqual(
        'WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests/render_frame_rate_fps/foreman_cif_delay_50_0_plr_5_flexfec',
        parent.key.id())
    # Check that all the Bot and TestMetadata entities were correctly added.
    created_masters = graph_data.Master.query().fetch()
    created_bots = graph_data.Bot.query().fetch()
    created_tests = graph_data.TestMetadata.query().fetch()
    self.assertEqual(1, len(created_masters))
    self.assertEqual(1, len(created_bots))
    self.assertEqual(3, len(created_tests))

    self.assertEqual('WebRTCPerf', created_masters[0].key.id())
    self.assertIsNone(created_masters[0].key.parent())

    self.assertEqual('android32-pixel5-android11', created_bots[0].key.id())
    self.assertEqual('WebRTCPerf', created_bots[0].key.parent().id())
    self.assertFalse(created_bots[0].internal_only)

    self.assertEqual('WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests',
                     created_tests[0].key.id())
    self.assertIsNone(created_tests[0].parent_test)
    self.assertEqual('android32-pixel5-android11', created_tests[0].bot_name)
    self.assertFalse(created_tests[0].internal_only)

    self.assertEqual('render_frame_rate_fps', created_tests[1].test_part1_name)
    self.assertEqual('WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests',
                     created_tests[1].parent_test.id())
    self.assertIsNone(created_tests[1].bot)
    self.assertFalse(created_tests[1].internal_only)

    self.assertEqual(
        'WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests/render_frame_rate_fps/foreman_cif_delay_50_0_plr_5_flexfec',
        created_tests[2].key.id())
    self.assertEqual(
        'WebRTCPerf/android32-pixel5-android11/webrtc_perf_tests/render_frame_rate_fps',
        created_tests[2].parent_test.id())
    self.assertIsNone(created_tests[2].bot)
    self.assertFalse(created_tests[2].internal_only)
    # pylint: enable=line-too-long

if __name__ == '__main__':
  unittest.main()
