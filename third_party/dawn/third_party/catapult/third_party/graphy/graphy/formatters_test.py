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

"""Tests for the formatters."""

from graphy import common
from graphy import formatters
from graphy import graphy_test
from graphy.backends import google_chart_api


class InlineLegendTest(graphy_test.GraphyTest):

  def setUp(self):
    self.chart = google_chart_api.LineChart()
    self.chart.formatters.append(formatters.InlineLegend)
    self.chart.AddLine([1, 2, 3], label='A')
    self.chart.AddLine([4, 5, 6], label='B')
    self.chart.auto_scale.buffer = 0

  def testLabelsAdded(self):
    self.assertEqual(self.Param('chxl'), '0:|A|B')

  def testLabelPositionedCorrectly(self):
    self.assertEqual(self.Param('chxp'), '0,3,6')
    self.assertEqual(self.Param('chxr'), '0,1,6')

  def testRegularLegendSuppressed(self):
    self.assertRaises(KeyError, self.Param, 'chdl')


class AutoScaleTest(graphy_test.GraphyTest):

  def setUp(self):
    self.chart = google_chart_api.LineChart([1, 2, 3])
    self.auto_scale = formatters.AutoScale(buffer=0)

  def testNormalCase(self):
    self.auto_scale(self.chart)
    self.assertEqual(1, self.chart.left.min)
    self.assertEqual(3, self.chart.left.max)
    
  def testKeepsDataAwayFromEdgesByDefault(self):
    self.auto_scale = formatters.AutoScale()
    self.auto_scale(self.chart)
    self.assertTrue(1 > self.chart.left.min)
    self.assertTrue(3 < self.chart.left.max)

  def testDoNothingIfNoData(self):
    self.chart.data = []
    self.auto_scale(self.chart)
    self.assertEqual(None, self.chart.left.min)
    self.assertEqual(None, self.chart.left.max)
    self.chart.AddLine([])
    self.auto_scale(self.chart)
    self.assertEqual(None, self.chart.left.min)
    self.assertEqual(None, self.chart.left.max)

  def testKeepMinIfSet(self):
    self.chart.left.min = -10
    self.auto_scale(self.chart)
    self.assertEqual(-10, self.chart.left.min)
    self.assertEqual(3, self.chart.left.max)

  def testKeepMaxIfSet(self):
    self.chart.left.max = 9
    self.auto_scale(self.chart)
    self.assertEqual(1, self.chart.left.min)
    self.assertEqual(9, self.chart.left.max)

  def testOtherDependentAxesAreAlsoSet(self):
    self.chart.AddAxis(common.AxisPosition.LEFT, common.Axis())
    self.chart.AddAxis(common.AxisPosition.RIGHT, common.Axis())
    self.assertEqual(4, len(self.chart.GetDependentAxes()))
    self.auto_scale(self.chart)
    for axis in self.chart.GetDependentAxes():
      self.assertEqual(1, axis.min)
      self.assertEqual(3, axis.max)
  
  def testRightSetsLeft(self):
    """If user sets min/max on right but NOT left, they are copied to left.
    (Otherwise the data will be scaled differently from the right-axis labels,
    which is bad).
    """
    self.chart.right.min = 18
    self.chart.right.max = 19
    self.auto_scale(self.chart)
    self.assertEqual(18, self.chart.left.min)
    self.assertEqual(19, self.chart.left.max)


if __name__ == '__main__':
  graphy_test.main()
