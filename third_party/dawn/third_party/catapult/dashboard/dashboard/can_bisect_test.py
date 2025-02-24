# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard import can_bisect
from dashboard.common import namespaced_stored_object
from dashboard.common import testing_common


class CanBisectTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    namespaced_stored_object.Set(
        can_bisect.BISECT_BOT_MAP_KEY,
        {'SupportedDomain': ['perf_bot', 'bisect_bot']})

  def testIsValidTestForBisect_BisectableTests_ReturnsTrue(self):
    self.assertEqual(
        can_bisect.IsValidTestForBisect(
            'SupportedDomain/mac/blink_perf.parser/simple-url'), True)

  def testIsValidTestForBisect_Supported_ReturnsTrue(self):
    self.assertTrue(can_bisect.IsValidTestForBisect('SupportedDomain/b/t/foo'))

  def testIsValidTestForBisect_V8_IsSupported(self):
    self.assertTrue(
        can_bisect.IsValidTestForBisect(
            'SupportedDomain/Pixel2/v8/JSTests/Array/Total'))

  def testIsValidTestForBisect_RefTest_ReturnsFalse(self):
    self.assertFalse(can_bisect.IsValidTestForBisect('SupportedDomain/b/t/ref'))

  def testIsValidTestForBisect_UnsupportedDomain_ReturnsFalse(self):
    self.assertFalse(can_bisect.IsValidTestForBisect('X/b/t/foo'))

  def testDomainNameIsExcludedForTriageBisects_NoDomains_ReturnsFalse(self):
    self.assertFalse(can_bisect.DomainIsExcludedFromTriageBisects('foo'))

  def testDomainNameIsExcludedForTriageBisects_NoMatch_ReturnsFalse(self):
    namespaced_stored_object.Set(can_bisect.FILE_BUG_BISECT_DENYLIST_KEY,
                                 {'bar': []})
    self.assertFalse(can_bisect.DomainIsExcludedFromTriageBisects('foo'))

  def testDomainNameIsExcludedForTriageBisects_Match_ReturnsTrue(self):
    namespaced_stored_object.Set(can_bisect.FILE_BUG_BISECT_DENYLIST_KEY,
                                 {'foo': []})
    self.assertTrue(can_bisect.DomainIsExcludedFromTriageBisects('foo'))


if __name__ == '__main__':
  unittest.main()
