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

from graphy import common
from graphy import graphy_test
from graphy import line_chart
from graphy.backends import google_chart_api
from graphy.backends.google_chart_api import base_encoder_test


# Extend XYChartTest so that we pick up & repeat all the basic tests which
# LineCharts should continue to satisfy
class LineChartTest(base_encoder_test.XYChartTest):

  def GetChart(self, *args, **kwargs):
    return google_chart_api.LineChart(*args, **kwargs)

  def AddToChart(self, chart, points, color=None, label=None):
    return chart.AddLine(points, color=color, label=label)

  def testChartType(self):
    self.assertEqual(self.Param('cht'), 'lc')

  def testMarkers(self):
    x = common.Marker('x', '0000FF', 5)
    o = common.Marker('o', '00FF00', 5)
    line = common.Marker('V', 'dddddd', 1)
    self.chart.AddLine([1, 2, 3], markers=[(1, x), (2, o), (3, x)])
    self.chart.AddLine([4, 5, 6], markers=[(x, line) for x in range(3)])
    x = 'x,0000FF,0,%s,5'
    o = 'o,00FF00,0,%s,5'
    V = 'V,dddddd,1,%s,1'
    actual = self.Param('chm')
    expected = [m % i for i, m in zip([1, 2, 3, 0, 1, 2], [x, o, x, V, V, V])]
    expected = '|'.join(expected)
    error_msg = '\n%s\n!=\n%s' % (actual, expected)
    self.assertEqual(actual, expected, error_msg)

  def testLinePatterns(self):
    self.chart.AddLine([1, 2, 3])
    self.chart.AddLine([4, 5, 6], pattern=line_chart.LineStyle.DASHED)
    self.assertEqual(self.Param('chls'), '1,1,0|1,8,4')

  def testMultipleAxisLabels(self):
    self.ExpectAxes('', '')

    left_axis = self.chart.AddAxis(common.AxisPosition.LEFT,
                                   common.Axis())
    left_axis.labels = [10, 20, 30]
    left_axis.label_positions = [0, 50, 100]
    self.ExpectAxes('0:|10|20|30', '0,0,50,100')

    bottom_axis = self.chart.AddAxis(common.AxisPosition.BOTTOM,
                                     common.Axis())
    bottom_axis.labels = ['A', 'B', 'c', 'd']
    bottom_axis.label_positions = [0, 33, 66, 100]
    sub_axis = self.chart.AddAxis(common.AxisPosition.BOTTOM,
                                  common.Axis())
    sub_axis.labels = ['CAPS', 'lower']
    sub_axis.label_positions = [0, 50]
    self.ExpectAxes('0:|10|20|30|1:|A|B|c|d|2:|CAPS|lower',
                    '0,0,50,100|1,0,33,66,100|2,0,50')

    self.chart.AddAxis(common.AxisPosition.RIGHT, left_axis)
    self.ExpectAxes('0:|10|20|30|1:|10|20|30|2:|A|B|c|d|3:|CAPS|lower',
                    '0,0,50,100|1,0,50,100|2,0,33,66,100|3,0,50')
    self.assertEqual(self.Param('chxt'), 'y,r,x,x')

  def testAxisProperties(self):
    self.ExpectAxes('', '')

    self.chart.top.labels = ['cow', 'horse', 'monkey']
    self.chart.top.label_positions = [3.7, 10, -22.9]
    self.ExpectAxes('0:|cow|horse|monkey', '0,3.7,10,-22.9')

    self.chart.left.labels = [10, 20, 30]
    self.chart.left.label_positions = [0, 50, 100]
    self.ExpectAxes('0:|10|20|30|1:|cow|horse|monkey',
                    '0,0,50,100|1,3.7,10,-22.9')
    self.assertEqual(self.Param('chxt'), 'y,t')

    sub_axis = self.chart.AddAxis(common.AxisPosition.BOTTOM,
                                  common.Axis())
    sub_axis.labels = ['CAPS', 'lower']
    sub_axis.label_positions = [0, 50]
    self.ExpectAxes('0:|10|20|30|1:|CAPS|lower|2:|cow|horse|monkey',
                    '0,0,50,100|1,0,50|2,3.7,10,-22.9')
    self.assertEqual(self.Param('chxt'), 'y,x,t')

    self.chart.bottom.labels = ['A', 'B', 'C']
    self.chart.bottom.label_positions = [0, 33, 66]
    self.ExpectAxes('0:|10|20|30|1:|A|B|C|2:|CAPS|lower|3:|cow|horse|monkey',
                    '0,0,50,100|1,0,33,66|2,0,50|3,3.7,10,-22.9')
    self.assertEqual(self.Param('chxt'), 'y,x,x,t')


# Extend LineChartTest so that we pick up & repeat all the line tests which
# Sparklines should continue to satisfy
class SparklineTest(LineChartTest):

  def GetChart(self, *args, **kwargs):
    return google_chart_api.Sparkline(*args, **kwargs)

  def testChartType(self):
    self.assertEqual(self.Param('cht'), 'lfi')


if __name__ == '__main__':
  graphy_test.main()
