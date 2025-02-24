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

"""This module contains various formatters which can help format a chart
object.  To use these, add them to your chart's list of formatters.  For
example:
  chart.formatters.append(InlineLegend)
  chart.formatters.append(LabelSeparator(right=8))

Feel free to write your own formatter.  Formatters are just callables that
modify the chart in some (hopefully useful) way.  For example, the AutoColor
formatter makes sure each DataSeries has a color applied to it.  The formatter
should take the chart to format as its only argument.

(The formatters work on a deepcopy of the user's chart, so modifications
shouldn't leak back into the user's original chart)
"""

def AutoLegend(chart):
  """Automatically fill out the legend based on series labels.  This will only
  fill out the legend if is at least one series with a label.
  """
  chart._show_legend = False
  labels = []
  for series in chart.data:
    if series.label is None:
      labels.append('')
    else:
      labels.append(series.label)
      chart._show_legend = True
  if chart._show_legend:
    chart._legend_labels = labels


class AutoColor(object):
  """Automatically add colors to any series without colors.

  Object attributes:
    colors: The list of colors (hex strings) to cycle through.  You can modify
            this list if you don't like the default colors.
  """
  def __init__(self):
    # TODO: Add a few more default colors.
    # TODO: Add a default styles too, so if you don't specify color or
    # style, you get a unique set of colors & styles for your data.
    self.colors = ['0000ff', 'ff0000', '00dd00', '000000']

  def __call__(self, chart):
    index = -1
    for series in chart.data:
      if series.style.color is None:
        index += 1
        if index >= len(self.colors):
          index = 0
        series.style.color = self.colors[index]


class AutoScale(object):
  """If you don't set min/max on the dependent axes, this fills them in
  automatically by calculating min/max dynamically from the data.

  You can set just min or just max and this formatter will fill in the other
  value for you automatically.  For example, if you only set min then this will
  set max automatically, but leave min untouched.

  Charts can have multiple dependent axes (chart.left & chart.right, for
  example.)  If you set min/max on some axes but not others, then this formatter
  copies your min/max to the un-set axes.  For example, if you set up min/max on
  only the right axis then your values will be automatically copied to the left
  axis.  (if you use different min/max values for different axes, the
  precendence is undefined.  So don't do that.)
  """

  def __init__(self, buffer=0.05):
    """Create a new AutoScale formatter.

    Args:
      buffer: percentage of extra space to allocate around the chart's axes.
    """
    self.buffer = buffer

  def __call__(self, chart):
    """Format the chart by setting the min/max values on its dependent axis."""
    if not chart.data:
      return # Nothing to do.
    min_value, max_value = chart.GetMinMaxValues()
    if None in (min_value, max_value):
      return  # No data.  Nothing to do.

    # Honor user's choice, if they've picked min/max.
    for axis in chart.GetDependentAxes():
      if axis.min is not None:
        min_value = axis.min
      if axis.max is not None:
        max_value = axis.max

    buffer = (max_value - min_value) * self.buffer  # Stay away from edge.

    for axis in chart.GetDependentAxes():
      if axis.min is None:
        axis.min = min_value - buffer
      if axis.max is None:
        axis.max = max_value + buffer


class LabelSeparator(object):

  """Adjust the label positions to avoid having them overlap.  This happens for
  any axis with minimum_label_spacing set.
  """

  def __init__(self, left=None, right=None, bottom=None):
    self.left = left
    self.right = right
    self.bottom = bottom

  def __call__(self, chart):
    self.AdjustLabels(chart.left, self.left)
    self.AdjustLabels(chart.right, self.right)
    self.AdjustLabels(chart.bottom, self.bottom)

  def AdjustLabels(self, axis, minimum_label_spacing):
    if minimum_label_spacing is None:
      return
    if len(axis.labels) <= 1:  # Nothing to adjust
      return
    if axis.max is not None and axis.min is not None:
      # Find the spacing required to fit all labels evenly.
      # Don't try to push them farther apart than that.
      maximum_possible_spacing = (axis.max - axis.min) / (len(axis.labels) - 1)
      if minimum_label_spacing > maximum_possible_spacing:
        minimum_label_spacing = maximum_possible_spacing

    labels = [list(x) for x in zip(axis.label_positions, axis.labels)]
    labels = sorted(labels, reverse=True)

    # First pass from the top, moving colliding labels downward
    for i in range(1, len(labels)):
      if labels[i - 1][0] - labels[i][0] < minimum_label_spacing:
        new_position = labels[i - 1][0] - minimum_label_spacing
        if axis.min is not None and new_position < axis.min:
          new_position = axis.min
        labels[i][0] = new_position

    # Second pass from the bottom, moving colliding labels upward
    for i in range(len(labels) - 2, -1, -1):
      if labels[i][0] - labels[i + 1][0] < minimum_label_spacing:
        new_position = labels[i + 1][0] + minimum_label_spacing
        if axis.max is not None and new_position > axis.max:
          new_position = axis.max
        labels[i][0] = new_position

    # Separate positions and labels
    label_positions, labels = zip(*labels)
    axis.labels = labels
    axis.label_positions = label_positions


def InlineLegend(chart):
  """Provide a legend for line charts by attaching labels to the right
  end of each line.  Supresses the regular legend.
  """
  show = False
  labels = []
  label_positions = []
  for series in chart.data:
    if series.label is None:
      labels.append('')
    else:
      labels.append(series.label)
      show = True
    label_positions.append(series.data[-1])

  if show:
    chart.right.min = chart.left.min
    chart.right.max = chart.left.max
    chart.right.labels = labels
    chart.right.label_positions = label_positions
    chart._show_legend = False  # Supress the regular legend.
