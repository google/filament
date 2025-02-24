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

"""Tests for pie_chart.py."""

import warnings

from graphy import pie_chart
from graphy import graphy_test


class SegmentTest(graphy_test.GraphyTest):

  def setUp(self):
    warnings.resetwarnings()

  # TODO: remove once the deprecation warning is removed
  def testSegmentOrder(self):
    # Deprecated approach
    warnings.filterwarnings('error')
    self.assertRaises(DeprecationWarning, pie_chart.Segment, 1,
      '0000FF', 'label')

    # New order
    s = pie_chart.Segment(1, 'label', '0000FF')
    self.assertEqual('label', s.label)
    self.assertEqual('0000FF', s.color)


class PieChartTest(graphy_test.GraphyTest):

  def tearDown(self):
    warnings.resetwarnings()

  def testNegativeSegmentSizes(self):
    self.assertRaises(AssertionError, pie_chart.PieChart,
                      [-5, 10], ['Negative', 'Positive'])
    chart = pie_chart.PieChart()
    self.assertRaises(AssertionError, pie_chart.Segment, -5, 'Dummy', '0000ff')
    segment = chart.AddSegment(10, label='Dummy', color='0000ff')
    self.assertRaises(AssertionError, segment._SetSize, -5)

  # TODO: remove once the deprecation warning is removed
  def testAddSegmentOrder(self):
    chart = pie_chart.PieChart()
    # Deprecated approach
    warnings.filterwarnings('error')
    self.assertRaises(DeprecationWarning, chart.AddSegment, 1,
      '0000FF', 'label')

    # New order
    chart.AddSegment(1, 'label', '0000FF')
    self.assertEqual('label', chart.data[0][0].label)
    self.assertEqual('0000FF', chart.data[0][0].color)

  # TODO: remove once the deprecation warning is removed
  def testAddSegmentsOrder(self):
    chart = pie_chart.PieChart()
    # Deprecated approach
    warnings.filterwarnings('error')
    self.assertRaises(DeprecationWarning, chart.AddSegments, [1],
      ['0000FF'], ['label'])
    # New order
    warnings.filterwarnings('ignore')
    chart.AddSegments([1], ['label'], ['0000FF'])
    self.assertEqual('label', chart.data[0][0].label)
    self.assertEqual('0000FF', chart.data[0][0].color)

  def testAddPie(self):
    chart = pie_chart.PieChart()
    i = chart.AddPie([1], ['A'], ['ff0000'])
    self.assertEqual(i, 0)
    self.assertEqual(len(chart.data), 1)
    self.assertEqual(len(chart.data[0]), 1)
    self.assertEqual(chart.data[0][0].size, 1)
    i = chart.AddPie([2], ['B'], ['0000ff'])
    self.assertEqual(i, 1)
    self.assertEqual(len(chart.data), 2)
    self.assertEqual(len(chart.data[0]), 1)
    self.assertEqual(chart.data[0][0].size, 1)
    self.assertEqual(len(chart.data[1]), 1)
    self.assertEqual(chart.data[1][0].size, 2)

  def testAddSegmentToPie(self):
    chart = pie_chart.PieChart()
    chart.AddPie([1], ['A'], ['ff0000'])
    chart.AddPie([2], ['B'], ['0000ff'])
    chart.AddSegment([10], ['AA'])
    self.assertEqual(len(chart.data[0]), 2)
    self.assertEqual(len(chart.data[1]), 1)
    chart.AddSegment([20], ['BB'], pie_index=1)
    self.assertEqual(len(chart.data[0]), 2)
    self.assertEqual(len(chart.data[1]), 2)


if __name__ == '__main__':
  graphy_test.main()
