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

"""Test for the base encoder.  Also serves as a base class for the
chart-type-specific tests."""

from graphy import common
from graphy import graphy_test
from graphy import formatters
from graphy.backends.google_chart_api import encoders
from graphy.backends.google_chart_api import util


class TestEncoder(encoders.BaseChartEncoder):
  """Simple implementation of BaseChartEncoder for testing common behavior."""
  def _GetType(self, chart):
    return {'chart_type': 'TEST_TYPE'}

  def _GetDependentAxis(self, chart):
    return chart.left


class TestChart(common.BaseChart):
  """Simple implementation of BaseChart for testing common behavior."""

  def __init__(self, points=None):
    super(TestChart, self).__init__()
    if points is not None:
      self.AddData(points)

  def AddData(self, points, color=None, label=None):
    style = common._BasicStyle(color)
    series = common.DataSeries(points, style=style, label=label)
    self.data.append(series)
    return series


class BaseChartTest(graphy_test.GraphyTest):
  """Base class for all chart-specific tests"""

  def ExpectAxes(self, labels, positions):
    """Helper to test that the chart axis spec matches the expected values."""
    self.assertEqual(self.Param('chxl'), labels)
    self.assertEqual(self.Param('chxp'), positions)

  def GetChart(self, *args, **kwargs):
    """Get a chart object.  Other classes can override to change the
    type of chart being tested.
    """
    chart = TestChart(*args, **kwargs)
    chart.display = TestEncoder(chart)
    return chart

  def AddToChart(self, chart, points, color=None, label=None):
    """Add data to the chart.

    Chart is assumed to be of the same type as returned by self.GetChart().
    """
    return chart.AddData(points, color=color, label=label)

  def setUp(self):
    self.chart = self.GetChart()

  def testImgAndUrlUseSameUrl(self):
    """Check that Img() and Url() return the same URL."""
    self.assertIn(self.chart.display.Url(500, 100, use_html_entities=True),
                  self.chart.display.Img(500, 100))

  def testImgUsesHtmlEntitiesInUrl(self):
    img_tag = self.chart.display.Img(500, 100)
    self.assertNotIn('&ch', img_tag)
    self.assertIn('&amp;ch', img_tag)

  def testParamsAreStrings(self):
    """Test that params are all converted to strings."""
    self.chart.display.extra_params['test'] = 32
    self.assertEqual(self.Param('test'),  '32')

  def testExtraParamsOverideDefaults(self):
    self.assertNotEqual(self.Param('cht'), 'test')  # Sanity check.
    self.chart.display.extra_params['cht'] = 'test'
    self.assertEqual(self.Param('cht'), 'test')

  def testExtraParamsCanUseLongNames(self):
    self.chart.display.extra_params['color'] = 'XYZ'
    self.assertEqual(self.Param('chco'),  'XYZ')

  def testExtraParamsCanUseNewNames(self):
    """Make sure future Google Chart API features can be accessed immediately
    through extra_params.  (Double-checks that the long-to-short name
    conversion doesn't mess up the ability to use new features).
    """
    self.chart.display.extra_params['fancy_new_feature'] = 'shiny'
    self.assertEqual(self.Param('fancy_new_feature'),  'shiny')

  def testEmptyParamsDropped(self):
    """Check that empty parameters don't end up in the URL."""
    self.assertEqual(self.Param('chxt'), '')
    self.assertNotIn('chxt', self.chart.display.Url(0, 0))

  def testSizes(self):
    self.assertIn('89x102', self.chart.display.Url(89, 102))

    img = self.chart.display.Img(89, 102)
    self.assertIn('chs=89x102', img)
    self.assertIn('width="89"', img)
    self.assertIn('height="102"', img)

  def testChartType(self):
    self.assertEqual(self.Param('cht'), 'TEST_TYPE')

  def testChartSizeConvertedToInt(self):
    url = self.chart.display.Url(100.1, 200.2)
    self.assertIn('100x200', url)

  def testUrlBase(self):
    def assertStartsWith(actual_text, expected_start):
      message = "[%s] didn't start with [%s]" % (actual_text, expected_start)
      self.assert_(actual_text.startswith(expected_start), message)

    assertStartsWith(self.chart.display.Url(0, 0),
                     'http://chart.apis.google.com/chart')

    url_base = 'http://example.com/charts'
    self.chart.display.url_base = url_base
    assertStartsWith(self.chart.display.Url(0, 0), url_base)

  def testEnhancedEncoder(self):
    self.chart.display.enhanced_encoding = True
    self.assertEqual(self.Param('chd'), 'e:')

  def testUrlsEscaped(self):
    self.AddToChart(self.chart, [1, 2, 3])
    url = self.chart.display.Url(500, 100)
    self.assertNotIn('chd=s:', url)
    self.assertIn('chd=s%3A', url)

  def testUrls_DefaultIsWithoutHtmlEntities(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [1, 2, 3], label='Ciao&"Mario>Luigi"')
    url_default = self.chart.display.Url(500, 100)
    url_forced = self.chart.display.Url(500, 100, use_html_entities=False)
    self.assertEqual(url_forced, url_default)

  def testUrls_HtmlEntities(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [1, 2, 3], label='Ciao&"Mario>Luigi"')
    url = self.chart.display.Url(500, 100, use_html_entities=True)
    self.assertNotIn('&ch', url)
    self.assertIn('&amp;ch', url)
    self.assertIn('%7CCiao%26%22Mario%3ELuigi%22', url)

  def testUrls_NoEscapeWithHtmlEntities(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [1, 2, 3], label='Ciao&"Mario>Luigi"')
    self.chart.display.escape_url = False
    url = self.chart.display.Url(500, 100, use_html_entities=True)
    self.assertNotIn('&ch', url)
    self.assertIn('&amp;ch', url)
    self.assertIn('Ciao&amp;&quot;Mario&gt;Luigi&quot;', url)

  def testUrls_NoHtmlEntities(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [1, 2, 3], label='Ciao&"Mario>Luigi"')
    url = self.chart.display.Url(500, 100, use_html_entities=False)
    self.assertIn('&ch', url)
    self.assertNotIn('&amp;ch', url)
    self.assertIn('%7CCiao%26%22Mario%3ELuigi%22', url)

  def testCanRemoveDefaultFormatters(self):
    self.assertEqual(3, len(self.chart.formatters))
    # I don't know why you'd want to remove the default formatters like this.
    # It is just a proof that  we can manipulate the default formatters
    # through their aliases.
    self.chart.formatters.remove(self.chart.auto_color)
    self.chart.formatters.remove(self.chart.auto_legend)
    self.chart.formatters.remove(self.chart.auto_scale)
    self.assertEqual(0, len(self.chart.formatters))

  def testFormattersWorkOnCopy(self):
    """Make sure formatters can't modify the user's chart."""
    self.AddToChart(self.chart, [1])
    # By making sure our point is at the upper boundry, we make sure that both
    # line, pie, & bar charts encode it as a '9' in the simple encoding.
    self.chart.left.max = 1
    self.chart.left.min = 0
    # Sanity checks before adding a formatter.
    self.assertEqual(self.Param('chd'), 's:9')
    self.assertEqual(len(self.chart.data), 1)

    def MaliciousFormatter(chart):
      chart.data.pop() # Modify a mutable chart attribute
    self.chart.AddFormatter(MaliciousFormatter)

    self.assertEqual(self.Param('chd'), 's:', "Formatter wasn't used.")
    self.assertEqual(len(self.chart.data), 1,
                     "Formatter was able to modify original chart.")

    self.chart.formatters.remove(MaliciousFormatter)
    self.assertEqual(self.Param('chd'), 's:9',
                     "Chart changed even after removing the formatter")


class XYChartTest(BaseChartTest):
  """Base class for charts that display lines or points in 2d.

  Pretty much anything but the pie chart.
  """

  def testImgAndUrlUseSameUrl(self):
    """Check that Img() and Url() return the same URL."""
    super(XYChartTest, self).testImgAndUrlUseSameUrl()
    self.AddToChart(self.chart, range(0, 100))
    self.assertIn(self.chart.display.Url(500, 100, use_html_entities=True),
                  self.chart.display.Img(500, 100))
    self.chart = self.GetChart([-1, 0, 1])
    self.assertIn(self.chart.display.Url(500, 100, use_html_entities=True),
                  self.chart.display.Img(500, 100))

   # TODO: Once the deprecated AddSeries is removed, revisit
   # whether we need this test.
  def testAddSeries(self):
    self.chart.auto_scale.buffer = 0  # Buffer causes trouble for testing.
    self.assertEqual(self.Param('chd'), 's:')
    self.AddToChart(self.chart, (1, 2, 3))
    self.assertEqual(self.Param('chd'), 's:Af9')
    self.AddToChart(self.chart, (4, 5, 6))
    self.assertEqual(self.Param('chd'), 's:AMY,lx9')

   # TODO: Once the deprecated AddSeries is removed, revisit
   # whether we need this test.
  def testAddSeriesReturnsValue(self):
    points = (1, 2, 3)
    series = self.AddToChart(self.chart, points, '#000000')
    self.assertTrue(series is not None)
    self.assertEqual(series.data, points)
    self.assertEqual(series.style.color, '#000000')

  def testFlatSeries(self):
    """Make sure we handle scaling of a flat data series correctly (there are
    div by zero issues).
    """
    self.AddToChart(self.chart, [5, 5, 5])
    self.assertEqual(self.Param('chd'), 's:AAA')
    self.chart.left.min = 0
    self.chart.left.max = 5
    self.assertEqual(self.Param('chd'), 's:999')
    self.chart.left.min = 5
    self.chart.left.max = 15
    self.assertEqual(self.Param('chd'), 's:AAA')

  def testEmptyPointsStillCreatesSeries(self):
    """If we pass an empty list for points, we expect to get an empty data
    series, not nothing.  This way we can add data points later."""
    chart = self.GetChart()
    self.assertEqual(0, len(chart.data))
    data = []
    chart = self.GetChart(data)
    self.assertEqual(1, len(chart.data))
    self.assertEqual(0, len(chart.data[0].data))
    # This is the use case we are trying to serve: adding points later.
    data.append(0)
    self.assertEqual(1, len(chart.data[0].data))

  def testEmptySeriesDroppedFromParams(self):
    """By the time we make parameters, we don't want empty series to be
    included because it will mess up the indexes of other things like colors
    and makers.  They should be dropped instead."""
    self.chart.auto_scale.buffer = 0
    # Check just an empty series.
    self.AddToChart(self.chart, [], color='eeeeee')
    self.assertEqual(self.Param('chd'), 's:')
    # Now check when there are some real series in there too.
    self.AddToChart(self.chart, [1], color='111111')
    self.AddToChart(self.chart, [], color='FFFFFF')
    self.AddToChart(self.chart, [2], color='222222')
    self.assertEqual(self.Param('chd'), 's:A,9')
    self.assertEqual(self.Param('chco'), '111111,222222')

  def testDataSeriesCorrectlyConverted(self):
    # To avoid problems caused by floating-point errors, the input in this test
    # is carefully chosen to avoid 0.5 boundries (1.5, 2.5, 3.5, ...).
    chart = self.GetChart()
    chart.auto_scale.buffer = 0   # The buffer makes testing difficult.
    self.assertEqual(self.Param('chd', chart), 's:')
    chart = self.GetChart(range(0, 10))
    chart.auto_scale.buffer = 0
    self.assertEqual(self.Param('chd', chart), 's:AHOUbipv29')
    chart = self.GetChart(range(-10, 0))
    chart.auto_scale.buffer = 0
    self.assertEqual(self.Param('chd', chart), 's:AHOUbipv29')
    chart = self.GetChart((-1.1, 0.0, 1.1, 2.2))
    chart.auto_scale.buffer = 0
    self.assertEqual(self.Param('chd', chart), 's:AUp9')

  def testSeriesColors(self):
    self.AddToChart(self.chart, [1, 2, 3], '000000')
    self.AddToChart(self.chart, [4, 5, 6], 'FFFFFF')
    self.assertEqual(self.Param('chco'), '000000,FFFFFF')

  def testSeriesCaption_NoCaptions(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6])
    self.assertRaises(KeyError, self.Param, 'chdl')

  def testSeriesCaption_SomeCaptions(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6], label='Label')
    self.AddToChart(self.chart, [7, 8, 9])
    self.assertEqual(self.Param('chdl'), '|Label|')

  def testThatZeroIsPreservedInCaptions(self):
    """Test that a 0 caption becomes '0' and not ''.
    (This makes sure that the logic to rewrite a label of None to '' doesn't
    also accidentally rewrite 0 to '').
    """
    self.AddToChart(self.chart, [], label=0)
    self.AddToChart(self.chart, [], label=1)
    self.assertEqual(self.Param('chdl'), '0|1')

  def testSeriesCaption_AllCaptions(self):
    self.AddToChart(self.chart, [1, 2, 3], label='Its')
    self.AddToChart(self.chart, [4, 5, 6], label='Me')
    self.AddToChart(self.chart, [7, 8, 9], label='Mario')
    self.assertEqual(self.Param('chdl'), 'Its|Me|Mario')

  def testDefaultColorsApplied(self):
    self.AddToChart(self.chart, [1, 2, 3])
    self.AddToChart(self.chart, [4, 5, 6])
    self.assertEqual(self.Param('chco'), '0000ff,ff0000')

  def testShowingAxes(self):
    self.assertEqual(self.Param('chxt'), '')
    self.chart.left.min = 3
    self.chart.left.max = 5
    self.assertEqual(self.Param('chxt'), '')
    self.chart.left.labels = ['a']
    self.assertEqual(self.Param('chxt'), 'y')
    self.chart.right.labels = ['a']
    self.assertEqual(self.Param('chxt'), 'y,r')
    self.chart.left.labels = []  # Set back to the original state.
    self.assertEqual(self.Param('chxt'), 'r')

  def testAxisRanges(self):
    self.chart.left.labels = ['a']
    self.chart.bottom.labels = ['a']
    self.assertEqual(self.Param('chxr'), '')
    self.chart.left.min = -5
    self.chart.left.max = 10
    self.assertEqual(self.Param('chxr'), '0,-5,10')
    self.chart.bottom.min = 0.5
    self.chart.bottom.max = 0.75
    self.assertEqual(self.Param('chxr'), '0,-5,10|1,0.5,0.75')

  def testAxisLabels(self):
    self.ExpectAxes('', '')
    self.chart.left.labels = [10, 20, 30]
    self.ExpectAxes('0:|10|20|30', '')
    self.chart.left.label_positions = [0, 50, 100]
    self.ExpectAxes('0:|10|20|30', '0,0,50,100')
    self.chart.right.labels = ['cow', 'horse', 'monkey']
    self.chart.right.label_positions = [3.7, 10, -22.9]
    self.ExpectAxes('0:|10|20|30|1:|cow|horse|monkey',
                    '0,0,50,100|1,3.7,10,-22.9')

  def testGridBottomAxis(self):
    self.chart.bottom.min = 0
    self.chart.bottom.max = 20
    self.chart.bottom.grid_spacing = 10
    self.assertEqual(self.Param('chg'), '50,0,1,0')
    self.chart.bottom.grid_spacing = 2
    self.assertEqual(self.Param('chg'), '10,0,1,0')

  def testGridFloatingPoint(self):
    """Test that you can get decimal grid values in chg."""
    self.chart.bottom.min = 0
    self.chart.bottom.max = 8
    self.chart.bottom.grid_spacing = 1
    self.assertEqual(self.Param('chg'), '12.5,0,1,0')
    self.chart.bottom.max = 3
    self.assertEqual(self.Param('chg'), '33.3,0,1,0')

  def testGridLeftAxis(self):
    self.chart.auto_scale.buffer = 0
    self.AddToChart(self.chart, (0, 20))
    self.chart.left.grid_spacing = 5
    self.assertEqual(self.Param('chg'), '0,25,1,0')

  def testLabelGridBottomAxis(self):
    self.AddToChart(self.chart, [0, 20, 40])
    self.chart.bottom.label_gridlines = True
    self.chart.bottom.labels = ['Apple', 'Banana', 'Coconut']
    self.chart.bottom.label_positions = [1.5, 5, 8.5]
    self.chart.display._width = 320
    self.chart.display._height = 240
    self.assertEqual(self.Param('chxtc'), '0,-320')

  def testLabelGridLeftAxis(self):
    self.AddToChart(self.chart, [0, 20, 40])
    self.chart.left.label_gridlines = True
    self.chart.left.labels = ['Few', 'Some', 'Lots']
    self.chart.left.label_positions = [5, 20, 35]
    self.chart.display._width = 320
    self.chart.display._height = 240
    self.assertEqual(self.Param('chxtc'), '0,-320')

  def testLabelGridBothAxes(self):
    self.AddToChart(self.chart, [0, 20, 40])
    self.chart.left.label_gridlines = True
    self.chart.left.labels = ['Few', 'Some', 'Lots']
    self.chart.left.label_positions = [5, 20, 35]
    self.chart.bottom.label_gridlines = True
    self.chart.bottom.labels = ['Apple', 'Banana', 'Coconut']
    self.chart.bottom.label_positions = [1.5, 5, 8.5]
    self.chart.display._width = 320
    self.chart.display._height = 240
    self.assertEqual(self.Param('chxtc'), '0,-320|1,-320')

  def testDefaultDataScalingNotPersistant(self):
    """The auto-scaling shouldn't permanantly set the scale."""
    self.chart.auto_scale.buffer = 0  # Buffer just makes the math tricky here.
    # This data should scale to the simple encoding's min/middle/max values
    # (A, f, 9).
    self.AddToChart(self.chart, [1, 2, 3])
    self.assertEqual(self.Param('chd'), 's:Af9')
    # Different data that maintains the same relative spacing *should* scale
    # to the same min/middle/max.
    self.chart.data[0].data = [10, 20, 30]
    self.assertEqual(self.Param('chd'), 's:Af9')

  def FakeScale(self, data, old_min, old_max, new_min, new_max):
    self.min = old_min
    self.max = old_max
    return data

  def testDefaultDataScaling(self):
    """If you don't set min/max, it should use the data's min/max."""
    orig_scale = util.ScaleData
    util.ScaleData = self.FakeScale
    try:
      self.AddToChart(self.chart, [2, 3, 5, 7, 11])
      self.chart.auto_scale.buffer = 0
      # This causes scaling to happen & calls FakeScale.
      self.chart.display.Url(0, 0)
      self.assertEqual(2, self.min)
      self.assertEqual(11, self.max)
    finally:
      util.ScaleData = orig_scale

  def testDefaultDataScalingAvoidsCropping(self):
    """The default scaling should give a little buffer to avoid cropping."""
    orig_scale = util.ScaleData
    util.ScaleData = self.FakeScale
    try:
      self.AddToChart(self.chart, [1, 6])
      # This causes scaling to happen & calls FakeScale.
      self.chart.display.Url(0, 0)
      buffer = 5 * self.chart.auto_scale.buffer
      self.assertEqual(1 - buffer, self.min)
      self.assertEqual(6 + buffer, self.max)
    finally:
      util.ScaleData = orig_scale

  def testExplicitDataScaling(self):
    """If you set min/max, data should be scaled to this."""
    orig_scale = util.ScaleData
    util.ScaleData = self.FakeScale
    try:
      self.AddToChart(self.chart, [2, 3, 5, 7, 11])
      self.chart.left.min = -7
      self.chart.left.max = 49
      # This causes scaling to happen & calls FakeScale.
      self.chart.display.Url(0, 0)
      self.assertEqual(-7, self.min)
      self.assertEqual(49, self.max)
    finally:
      util.ScaleData = orig_scale

  def testImplicitMinValue(self):
    """min values should be filled in if they are not set explicitly."""
    orig_scale = util.ScaleData
    util.ScaleData = self.FakeScale
    try:
      self.AddToChart(self.chart, [0, 10])
      self.chart.auto_scale.buffer = 0
      self.chart.display.Url(0, 0)  # This causes a call to FakeScale.
      self.assertEqual(0, self.min)
      self.chart.left.min = -5
      self.chart.display.Url(0, 0)  # This causes a call to FakeScale.
      self.assertEqual(-5, self.min)
    finally:
      util.ScaleData = orig_scale

  def testImplicitMaxValue(self):
    """max values should be filled in if they are not set explicitly."""
    orig_scale = util.ScaleData
    util.ScaleData = self.FakeScale
    try:
      self.AddToChart(self.chart, [0, 10])
      self.chart.auto_scale.buffer = 0
      self.chart.display.Url(0, 0)  # This causes a call to FakeScale.
      self.assertEqual(10, self.max)
      self.chart.left.max = 15
      self.chart.display.Url(0, 0)  # This causes a call to FakeScale.
      self.assertEqual(15, self.max)
    finally:
      util.ScaleData = orig_scale

  def testNoneCanAppearInData(self):
    """None should be a valid value in a data series.  (It means "no data at
    this point")
    """
    # Buffer makes comparison difficult because min/max aren't A & 9
    self.chart.auto_scale.buffer = 0
    self.AddToChart(self.chart, [1, None, 3])
    self.assertEqual(self.Param('chd'), 's:A_9')

  def testResolveLabelCollision(self):
    self.chart.auto_scale.buffer = 0
    self.AddToChart(self.chart, [500, 1000])
    self.AddToChart(self.chart, [100, 999])
    self.AddToChart(self.chart, [200, 900])
    self.AddToChart(self.chart, [200, -99])
    self.AddToChart(self.chart, [100, -100])
    self.chart.right.max = 1000
    self.chart.right.min = -100
    self.chart.right.labels = [1000, 999, 900, 0, -99, -100]
    self.chart.right.label_positions =  self.chart.right.labels
    separation = formatters.LabelSeparator(right=40)
    self.chart.AddFormatter(separation)
    self.assertEqual(self.Param('chxp'), '0,1000,960,900,0,-60,-100')

    # Try to force a greater spacing than possible
    separation.right = 300
    self.assertEqual(self.Param('chxp'), '0,1000,780,560,340,120,-100')

    # Cluster some values around the lower and upper threshold to verify
    # that order is preserved.
    self.chart.right.labels = [1000, 901, 900, 899, 10, 1, -50, -100]
    self.chart.right.label_positions =  self.chart.right.labels
    separation.right = 100
    self.assertEqual(self.Param('chxp'), '0,1000,900,800,700,200,100,0,-100')
    self.assertEqual(self.Param('chxl'), '0:|1000|901|900|899|10|1|-50|-100')

    # Try to adjust a single label
    self.chart.right.labels = [1000]
    self.chart.right.label_positions =  self.chart.right.labels
    self.assertEqual(self.Param('chxp'), '0,1000')
    self.assertEqual(self.Param('chxl'), '0:|1000')

  def testAdjustSingleLabelDoesNothing(self):
    """Make sure adjusting doesn't bork the single-label case."""
    self.AddToChart(self.chart, (5, 6, 7))
    self.chart.left.labels = ['Cutoff']
    self.chart.left.label_positions = [3]
    def CheckExpectations():
      self.assertEqual(self.Param('chxl'), '0:|Cutoff')
      self.assertEqual(self.Param('chxp'), '0,3')
    CheckExpectations() # Check without adjustment
    self.chart.AddFormatter(formatters.LabelSeparator(right=15))
    CheckExpectations() # Make sure adjustment hasn't changed anything


if __name__ == '__main__':
  graphy_test.main()
