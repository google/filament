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

"""Backend which can generate charts using the Google Chart API."""

from graphy import line_chart
from graphy import bar_chart
from graphy import pie_chart
from graphy.backends.google_chart_api import encoders

def _GetChartFactory(chart_class, display_class):
  """Create a factory method for instantiating charts with displays.

  Returns a method which, when called, will create & return a chart with
  chart.display already populated.
  """
  def Inner(*args, **kwargs):
    chart = chart_class(*args, **kwargs)
    chart.display = display_class(chart)
    return chart
  return Inner

# These helper methods make it easy to get chart objects with display
# objects already setup.  For example, this:
#   chart = google_chart_api.LineChart()
# is equivalent to:
#   chart = line_chart.LineChart()
#   chart.display = google_chart_api.encoders.LineChartEncoder(chart)
#
# (If there's some chart type for which a helper method isn't available, you
# can always just instantiate the correct encoder manually, like in the 2nd
# example above).
# TODO: fix these so they have nice docs in ipython (give them __doc__)
LineChart = _GetChartFactory(line_chart.LineChart, encoders.LineChartEncoder)
Sparkline = _GetChartFactory(line_chart.Sparkline, encoders.SparklineEncoder)
BarChart  = _GetChartFactory(bar_chart.BarChart, encoders.BarChartEncoder)
PieChart  = _GetChartFactory(pie_chart.PieChart, encoders.PieChartEncoder)
