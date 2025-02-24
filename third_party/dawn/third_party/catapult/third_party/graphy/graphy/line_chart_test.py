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

"""Tests for line_chart.py."""

import warnings

from graphy import common
from graphy import line_chart
from graphy import graphy_test


# TODO: All the different charts are expected to support a similar API (like
# having a display object, having a list of data series, axes, etc.).  Add some
# tests that run against all the charts to make sure they conform to the API.

class LineChartTest(graphy_test.GraphyTest):

  def tearDown(self):
    warnings.resetwarnings()

  # TODO: remove once AddSeries is deleted
  def testAddSeries(self):
    warnings.filterwarnings('ignore')
    chart = line_chart.LineChart()
    chart.AddSeries(points=[1, 2, 3], style=line_chart.LineStyle.solid(),
                    markers='markers', label='label')
    series = chart.data[0]
    self.assertEqual(series.data, [1, 2, 3])
    self.assertEqual(series.style.width, line_chart.LineStyle.solid().width)
    self.assertEqual(series.style.on, line_chart.LineStyle.solid().on)
    self.assertEqual(series.style.off, line_chart.LineStyle.solid().off)
    self.assertEqual(series.markers, 'markers')
    self.assertEqual(series.label, 'label')

  # TODO: remove once the deprecation warning is removed
  def testAddLineArgumentOrder(self):
    x = common.Marker(common.Marker.x, '0000ff', 5)

    # Deprecated approach
    chart = line_chart.LineChart()
    warnings.filterwarnings("error")
    self.assertRaises(DeprecationWarning, chart.AddLine, [1, 2, 3],
      'label', [x], 'color')

    # New order
    chart = line_chart.LineChart()
    chart.AddLine([1, 2, 3], 'label', 'color', markers=[x])
    self.assertEqual('label', chart.data[0].label)
    self.assertEqual([x], chart.data[0].markers)
    self.assertEqual('color', chart.data[0].style.color)
       

class LineStyleTest(graphy_test.GraphyTest):

  def tearDown(self):
    warnings.resetwarnings()

  def testPresets(self):
    """Test selected traits from the preset line styles."""
    self.assertEqual(0, line_chart.LineStyle.solid().off)
    self.assert_(line_chart.LineStyle.dashed().off > 0)
    self.assert_(line_chart.LineStyle.solid().width <
                 line_chart.LineStyle.thick_solid().width)
    
  def testLineStyleByValueGivesWarning(self):
    """Using LineStyle.foo as a value should throw a deprecation warning"""
    warnings.filterwarnings('error')    
    self.assertRaises(DeprecationWarning, common.DataSeries, [],
                      style=line_chart.LineStyle.solid)
    series = common.DataSeries([])
    def _TestAssignment():
      series.style = line_chart.LineStyle.solid
    self.assertRaises(DeprecationWarning, _TestAssignment)
    warnings.filterwarnings('ignore')
    series.style = line_chart.LineStyle.solid
    warnings.resetwarnings()
    self.assertEqual(1, series.style.width)    
    self.assertEqual(1, series.style.on)
    self.assertEqual(0, series.style.off)
    

if __name__ == '__main__':
  graphy_test.main()
