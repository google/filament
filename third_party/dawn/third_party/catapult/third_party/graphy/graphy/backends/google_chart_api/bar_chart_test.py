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

"""Unittest for Graphy and Google Chart API backend."""

import math

from graphy import graphy_test
from graphy import bar_chart
from graphy.backends import google_chart_api
from graphy.backends.google_chart_api import base_encoder_test


# Extend XYChartTest so that we pick up & repeat all the basic tests which
# BarCharts should continue to satisfy
class BarChartTest(base_encoder_test.XYChartTest):

  def GetChart(self, *args, **kwargs):
    return google_chart_api.BarChart(*args, **kwargs)

  def AddToChart(self, chart, points, color=None, label=None):
    return chart.AddBars(points, color=color, label=label)

  def testChartType(self):
    def Check(vertical, stacked, expected_type):
      self.chart.vertical = vertical
      self.chart.stacked = stacked
      self.assertEqual(self.Param('cht'), expected_type)
    Check(vertical=True,  stacked=True,  expected_type='bvs')
    Check(vertical=True,  stacked=False, expected_type='bvg')
    Check(vertical=False, stacked=True,  expected_type='bhs')
    Check(vertical=False, stacked=False, expected_type='bhg')

  def testSingleBarCase(self):
    """Test that we can handle a bar chart with only a single bar."""
    self.AddToChart(self.chart, [1])
    self.assertEqual(self.Param('chd'), 's:A')

  def testHorizontalScaling(self):
    """Test the scaling works correctly on horizontal bar charts (which have
    min/max on a different axis than other charts).
    """
    self.AddToChart(self.chart, [3])
    self.chart.vertical = False
    self.chart.bottom.min = 0
    self.chart.bottom.max = 3
    self.assertEqual(self.Param('chd'), 's:9')  # 9 is far right edge.
    self.chart.bottom.max = 6
    self.assertEqual(self.Param('chd'), 's:f')  # f is right in the middle.

  def testZeroPoint(self):
    self.AddToChart(self.chart, [-5, 0, 5])
    self.assertEqual(self.Param('chp'), str(.5))    # Auto scaling.
    self.chart.left.min = 0
    self.chart.left.max = 5
    self.assertRaises(KeyError, self.Param, 'chp')  # No negative values.
    self.chart.left.min = -5
    self.assertEqual(self.Param('chp'), str(.5))    # Explicit scaling.
    self.chart.left.max = 15
    self.assertEqual(self.Param('chp'), str(.25))   # Different zero point.
    self.chart.left.max = -1
    self.assertEqual(self.Param('chp'), str(1))     # Both negative values.

  def testLabelsInCorrectOrder(self):
    """Test that we reverse labels for horizontal bar charts
    (Otherwise they are backwards from what you would expect)
    """
    self.chart.left.labels = [1, 2, 3]
    self.chart.vertical = True
    self.assertEqual(self.Param('chxl'), '0:|1|2|3')
    self.chart.vertical = False
    self.assertEqual(self.Param('chxl'), '0:|3|2|1')

  def testLabelRangeDefaultsToDataScale(self):
    """Test that if you don't set axis ranges, they default to the data
    scale.
    """
    self.chart.auto_scale.buffer = 0  # Buffer causes trouble for testing.
    self.AddToChart(self.chart, [1, 5])
    self.chart.left.labels = (1, 5)
    self.chart.left.labels_positions = (1, 5)
    self.assertEqual(self.Param('chxr'), '0,1,5')

  def testCanOverrideChbh(self):
    self.chart.style = bar_chart.BarChartStyle(10, 3, 6)
    self.AddToChart(self.chart, [1, 2, 3])
    self.assertEqual(self.Param('chbh'), '10,3,6')
    self.chart.display.extra_params['chbh'] = '5,5,2'
    self.assertEqual(self.Param('chbh'), '5,5,2')

  def testDefaultBarChartStyle(self):
    self.assertNotIn('chbh', self.chart.display._Params(self.chart))
    self.chart.style = bar_chart.BarChartStyle(None, None, None)
    self.assertNotIn('chbh', self.chart.display._Params(self.chart))
    self.chart.style = bar_chart.BarChartStyle(10, 3, 6)
    self.assertNotIn('chbh', self.chart.display._Params(self.chart))
    self.AddToChart(self.chart, [1, 2, 3])
    self.assertEqual(self.Param('chbh'), '10,3,6')
    self.chart.style = bar_chart.BarChartStyle(10)
    self.assertEqual(self.Param('chbh'), '10,4,8')

  def testAutoBarSizing(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6])
    self.chart.style = bar_chart.BarChartStyle(None, 3, 6)
    self.chart.display._width = 100
    self.chart.display._height = 1000
    self.chart.stacked = False
    self.assertEqual(self.Param('chbh'), 'a,3,6')
    self.chart.stacked = True
    self.assertEqual(self.Param('chbh'), 'a,3')
    self.chart.vertical = False
    self.chart.stacked = False
    self.assertEqual(self.Param('chbh'), 'a,3,6')
    self.chart.stacked = True
    self.assertEqual(self.Param('chbh'), 'a,3')
    self.chart.display._height = 1
    self.assertEqual(self.Param('chbh'), 'a,3')

  def testAutoBarSpacing(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6])
    self.chart.style = bar_chart.BarChartStyle(10, 1, None)
    self.assertEqual(self.Param('chbh'), '10,1,2')
    self.chart.style = bar_chart.BarChartStyle(10, None, 2)
    self.assertEqual(self.Param('chbh'), '10,1,2')
    self.chart.style = bar_chart.BarChartStyle(10, None, 1)
    self.assertEqual(self.Param('chbh'), '10,0,1')

  def testFractionalAutoBarSpacing(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6])
    self.chart.style = bar_chart.BarChartStyle(10, 0.1, None,
        use_fractional_gap_spacing=True)
    self.assertEqual(self.Param('chbh'), '10,1,2')
    self.chart.style = bar_chart.BarChartStyle(10, None, 0.2,
        use_fractional_gap_spacing=True)
    self.assertEqual(self.Param('chbh'), '10,1,2')
    self.chart.style = bar_chart.BarChartStyle(10, None, 0.1,
        use_fractional_gap_spacing=True)
    self.assertEqual(self.Param('chbh'), '10,0,1')
    self.chart.style = bar_chart.BarChartStyle(None, 0.1, 0.2,
        use_fractional_gap_spacing=True)
    self.assertEqual(self.Param('chbh'), 'r,0.1,0.2')
    self.chart.style = bar_chart.BarChartStyle(None, 0.1, None,
        use_fractional_gap_spacing=True)
    self.assertEqual(self.Param('chbh'), 'r,0.1,0.2')

  def testStackedDataScaling(self):
    self.AddToChart(self.chart, [10, 20, 30])
    self.AddToChart(self.chart, [-5, -10, -15])
    self.chart.stacked = True
    self.assertEqual(self.Param('chd'), 's:iu6,PJD')
    self.chart.stacked = False
    self.assertEqual(self.Param('chd'), 's:iu6,PJD')

    self.chart = self.GetChart()
    self.chart.stacked = True
    self.AddToChart(self.chart, [10, 20, 30])
    self.AddToChart(self.chart, [5, -10, 15])
    self.assertEqual(self.Param('chd'), 's:Xhr,SDc')
    self.AddToChart(self.chart, [-15, -10, -45])
    self.assertEqual(self.Param('chd'), 's:lrx,iYo,VYD')
    # TODO: Figure out how to deal with missing data points, test them

  def testNegativeBars(self):
    self.chart.stacked = True
    self.AddToChart(self.chart, [-10,-20,-30])
    self.assertEqual(self.Param('chd'), 's:oVD')
    self.AddToChart(self.chart, [-1,-2,-3])
    self.assertEqual(self.Param('chd'), 's:pZI,531')
    self.chart.stacked = False
    self.assertEqual(self.Param('chd'), 's:pWD,642')


if __name__ == '__main__':
  graphy_test.main()
