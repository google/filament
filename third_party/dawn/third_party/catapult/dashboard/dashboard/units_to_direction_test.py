# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from dashboard import units_to_direction
from dashboard.common import testing_common
from dashboard.models import anomaly


class UnitsToDirectionTest(testing_common.TestCase):

  def testUpdateFromJson_SetsImprovementDirections(self):
    units_to_direction.UpdateFromJson({
        'description': 'this is ignored',
        'ms': {
            'improvement_direction': 'down'
        },
        'score': {
            'improvement_direction': 'up'
        },
    })
    self.assertEqual(anomaly.DOWN,
                     units_to_direction.GetImprovementDirection('ms'))
    self.assertEqual(anomaly.UP,
                     units_to_direction.GetImprovementDirection('score'))
    self.assertEqual(
        anomaly.UNKNOWN,
        units_to_direction.GetImprovementDirection('does-not-exist'))

  def testUpdateFromJson_UnknownUnit_Added(self):
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'down'
        },
    })
    self.assertEqual(anomaly.UNKNOWN,
                     units_to_direction.GetImprovementDirection('runs/s'))
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'down'
        },
        'runs/s': {
            'improvement_direction': 'up'
        },
    })
    self.assertEqual(anomaly.UP,
                     units_to_direction.GetImprovementDirection('runs/s'))

  def testUpdateFromJson_ExistingUnitNotInNewList_RemovesUnit(self):
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'down'
        },
        'score': {
            'improvement_direction': 'up'
        },
    })
    self.assertEqual(anomaly.UP,
                     units_to_direction.GetImprovementDirection('score'))
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'down'
        },
    })
    self.assertEqual(anomaly.UNKNOWN,
                     units_to_direction.GetImprovementDirection('score'))

  def testUpdateFromJson_ExistingUnit_ChangesDirection(self):
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'down'
        },
    })
    self.assertEqual(anomaly.DOWN,
                     units_to_direction.GetImprovementDirection('ms'))
    units_to_direction.UpdateFromJson({
        'ms': {
            'improvement_direction': 'up'
        },
    })
    self.assertEqual(anomaly.UP,
                     units_to_direction.GetImprovementDirection('ms'))


if __name__ == '__main__':
  unittest.main()
