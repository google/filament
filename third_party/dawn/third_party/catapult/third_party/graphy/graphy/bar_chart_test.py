#!/usr/bin/python2.4
#
# Copyright 2008 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tests for bar_chart.py."""

import warnings

from graphy import common
from graphy import bar_chart
from graphy import graphy_test
from graphy.backends import google_chart_api


class BarChartTest(graphy_test.GraphyTest):

  def setUp(self):
    self.chart = google_chart_api.BarChart()

  def tearDown(self):
    warnings.resetwarnings()

  # TODO: remove once the deprecation warning is removed
  def testBarStyleStillExists(self):
    warnings.filterwarnings('ignore')
    x = bar_chart.BarStyle(None, None, None)

  # TODO: remove once the deprecation warning is removed
  def testAddBarArgumentOrder(self):
    # Deprecated approach
    chart = bar_chart.BarChart()
    warnings.filterwarnings('error')
    self.assertRaises(DeprecationWarning, chart.AddBars, [1, 2, 3],
      '0000FF', 'label')

    # New order
    chart = bar_chart.BarChart()
    chart.AddBars([1, 2, 3], 'label', '0000FF')
    self.assertEqual('label', chart.data[0].label)
    self.assertEqual('0000FF', chart.data[0].style.color)

  def testGetDependentIndependentAxes(self):
    c = self.chart
    c.vertical = True
    self.assertEqual([c.left, c.right], c.GetDependentAxes())
    self.assertEqual([c.top, c.bottom], c.GetIndependentAxes())
    c.vertical = False
    self.assertEqual([c.top, c.bottom], c.GetDependentAxes())
    self.assertEqual([c.left, c.right], c.GetIndependentAxes())

    right2 = c.AddAxis(common.AxisPosition.RIGHT, common.Axis())
    bottom2 = c.AddAxis(common.AxisPosition.BOTTOM, common.Axis())

    c.vertical = True
    self.assertEqual([c.left, c.right, right2], c.GetDependentAxes())
    self.assertEqual([c.top, c.bottom, bottom2], c.GetIndependentAxes())
    c.vertical = False
    self.assertEqual([c.top, c.bottom, bottom2], c.GetDependentAxes())
    self.assertEqual([c.left, c.right, right2], c.GetIndependentAxes())

  def testDependentIndependentAxis(self):
    self.chart.vertical = True
    self.assertTrue(self.chart.left is self.chart.GetDependentAxis())
    self.assertTrue(self.chart.bottom is self.chart.GetIndependentAxis())
    self.chart.vertical = False
    self.assertTrue(self.chart.bottom, self.chart.GetDependentAxis())
    self.assertTrue(self.chart.left, self.chart.GetIndependentAxis())


if __name__ == '__main__':
  graphy_test.main()
