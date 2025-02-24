# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.common import bot_configurations
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common


class ConfigTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()

    namespaced_stored_object.Set(
        bot_configurations.BOT_CONFIGURATIONS_KEY, {
            'chromium-rel-mac11-pro': {
                'alias': 'mac-11-perf'
            },
            'mac-11-perf': {
                'arg': 'value'
            },
        })

  def testGet(self):
    actual = bot_configurations.Get('mac-11-perf')
    expected = {'arg': 'value'}
    self.assertEqual(actual, expected)

  def testGetWithAlias(self):
    actual = bot_configurations.Get('chromium-rel-mac11-pro')
    expected = {'arg': 'value'}
    self.assertEqual(actual, expected)

  def testList(self):
    actual = bot_configurations.List()
    expected = ['mac-11-perf']
    self.assertEqual(actual, expected)

  def testAliases(self):
    namespaced_stored_object.Set(
        bot_configurations.BOT_CONFIGURATIONS_KEY, {
            'a-alias0': {
                'alias': 'a-canonical',
            },
            'a-alias1': {
                'alias': 'a-canonical',
            },
            'a-canonical': {},
            'b-canonical': {},
            'c-canonical': {},
            'c-alias0': {
                'alias': 'c-canonical',
            },
        })
    actual = bot_configurations.GetAliasesAsync('a-canonical').get_result()
    self.assertEqual({'a-canonical', 'a-alias0', 'a-alias1'}, actual)
    actual = bot_configurations.GetAliasesAsync('a-alias0').get_result()
    self.assertEqual({'a-canonical', 'a-alias0', 'a-alias1'}, actual)
    actual = bot_configurations.GetAliasesAsync('a-alias1').get_result()
    self.assertEqual({'a-canonical', 'a-alias0', 'a-alias1'}, actual)
    actual = bot_configurations.GetAliasesAsync('b-canonical').get_result()
    self.assertEqual({'b-canonical'}, actual)
    actual = bot_configurations.GetAliasesAsync('c-canonical').get_result()
    self.assertEqual({'c-canonical', 'c-alias0'}, actual)
    actual = bot_configurations.GetAliasesAsync('c-alias0').get_result()
    self.assertEqual({'c-canonical', 'c-alias0'}, actual)
    actual = bot_configurations.GetAliasesAsync('unknown').get_result()
    self.assertEqual({'unknown'}, actual)
