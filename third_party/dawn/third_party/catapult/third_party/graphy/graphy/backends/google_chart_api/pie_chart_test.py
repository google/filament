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

import warnings

from graphy import graphy_test
from graphy import pie_chart
from graphy.backends import google_chart_api
from graphy.backends.google_chart_api import base_encoder_test


# Extend BaseChartTest so that we pick up & repeat all the line tests which
# Pie Charts should continue to satisfy
class PieChartTest(base_encoder_test.BaseChartTest):

  def tearDown(self):
    warnings.resetwarnings()
    super(PieChartTest, self).tearDown()

  def GetChart(self, *args, **kwargs):
    return google_chart_api.PieChart(*args, **kwargs)

  def AddToChart(self, chart, points, color=None, label=None):
    return chart.AddSegment(points[0], color=color, label=label)

  def testCanRemoveDefaultFormatters(self):
    # Override this test, as pie charts don't have default formatters.
    pass

  def testChartType(self):
    self.chart.display.is3d = False
    self.assertEqual(self.Param('cht'), 'p')
    self.chart.display.is3d = True
    self.assertEqual(self.Param('cht'), 'p3')

  def testEmptyChart(self):
    self.assertEqual(self.Param('chd'), 's:')
    self.assertEqual(self.Param('chco'), '')
    self.assertEqual(self.Param('chl'), '')

  def testChartCreation(self):
    self.chart = self.GetChart([1,2,3], ['Mouse', 'Cat', 'Dog'])
    self.assertEqual(self.Param('chd'), 's:Up9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog')
    self.assertEqual(self.Param('cht'), 'p')
    # TODO: Get 'None' labels to work and test them

  def testAddSegment(self):
    self.chart = self.GetChart([1,2,3], ['Mouse', 'Cat', 'Dog'])
    self.chart.AddSegment(4, label='Horse')
    self.assertEqual(self.Param('chd'), 's:Pfu9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog|Horse')
    
  # TODO: Remove this when AddSegments is removed   
  def testAddMultipleSegments(self):
    warnings.filterwarnings('ignore')
    self.chart.AddSegments([1,2,3],
                           ['Mouse', 'Cat', 'Dog'],
                           ['ff0000', '00ff00', '0000ff'])
    self.assertEqual(self.Param('chd'), 's:Up9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog')
    self.assertEqual(self.Param('chco'), 'ff0000,00ff00,0000ff')
    # skip two colors    
    self.chart.AddSegments([4,5,6], ['Horse', 'Moose', 'Elephant'], ['cccccc'])
    self.assertEqual(self.Param('chd'), 's:KUfpz9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog|Horse|Moose|Elephant')
    self.assertEqual(self.Param('chco'), 'ff0000,00ff00,0000ff,cccccc')    

  def testMultiplePies(self):
    self.chart.AddPie([1,2,3],
                      ['Mouse', 'Cat', 'Dog'],
                      ['ff0000', '00ff00', '0000ff'])    
    self.assertEqual(self.Param('chd'), 's:Up9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog')
    self.assertEqual(self.Param('chco'), 'ff0000,00ff00,0000ff')
    self.assertEqual(self.Param('cht'), 'p')
    # skip two colors
    self.chart.AddPie([4,5,6], ['Horse', 'Moose', 'Elephant'], ['cccccc'])
    self.assertEqual(self.Param('chd'), 's:KUf,pz9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog|Horse|Moose|Elephant')
    self.assertEqual(self.Param('chco'), 'ff0000,00ff00,0000ff,cccccc')
    self.assertEqual(self.Param('cht'), 'pc')

  def testMultiplePiesNo3d(self):
    chart = self.GetChart([1,2,3], ['Mouse', 'Cat', 'Dog'])
    chart.AddPie([4,5,6], ['Horse', 'Moose', 'Elephant'])
    chart.display.is3d = True
    warnings.filterwarnings('error')
    self.assertRaises(RuntimeWarning, chart.display.Url, 320, 240)

  def testAddSegmentByIndex(self):
    self.chart = self.GetChart([1,2,3], ['Mouse', 'Cat', 'Dog'])
    self.chart.AddSegment(4, 'Horse', pie_index=0)
    self.assertEqual(self.Param('chd'), 's:Pfu9')
    self.assertEqual(self.Param('chl'), 'Mouse|Cat|Dog|Horse')
    self.chart.AddPie([4,5], ['Apple', 'Orange'], [])
    self.chart.AddSegment(6, 'Watermelon', pie_index=1)
    self.assertEqual(self.Param('chd'), 's:KUfp,pz9')

  def testSetColors(self):
    self.assertEqual(self.Param('chco'), '')
    self.chart.AddSegment(1, label='Mouse')
    self.chart.AddSegment(5, label='Moose')
    self.chart.SetColors('000033', '0000ff')
    self.assertEqual(self.Param('chco'), '000033,0000ff')
    self.chart.AddSegment(6, label='Elephant')
    self.assertEqual(self.Param('chco'), '000033,0000ff')

  def testHugeSegmentSizes(self):
    self.chart = self.GetChart([1000000000000000L,3000000000000000L],
                               ['Big', 'Uber'])
    self.assertEqual(self.Param('chd'), 's:U9')
    self.chart.display.enhanced_encoding = True
    self.assertEqual(self.Param('chd'), 'e:VV..')

  def testSetSegmentSize(self):
    segment1 = self.chart.AddSegment(1)
    segment2 = self.chart.AddSegment(2)
    self.assertEqual(self.Param('chd'), 's:f9')
    segment2.size = 3
    self.assertEquals(segment1.size, 1)
    self.assertEquals(segment2.size, 3)
    self.assertEqual(self.Param('chd'), 's:U9')

  def testChartAngle(self):
    self.assertTrue('chp' not in self.chart.display._Params(self.chart))
    self.chart.display.angle = 3.1415
    self.assertEqual(self.Param('chp'), '3.1415')
    self.chart.display.angle = 0
    self.assertTrue('chp' not in self.chart.display._Params(self.chart))


if __name__ == '__main__':
  graphy_test.main()
