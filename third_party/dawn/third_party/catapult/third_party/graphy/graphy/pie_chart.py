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

"""Code for pie charts."""

import warnings

from graphy import common
from graphy import util


class Segment(common.DataSeries):
  """A single segment of the pie chart.

  Object attributes:
    size: relative size of the segment
    label: label of the segment (if any)
    color: color of the segment (if any)
  """
  def __init__(self, size, label=None, color=None):
    if label is not None and util._IsColor(label):
      warnings.warn('Your code may be broken! '
                    'Label looks like a hex triplet; it might be a color.  '
                    'The old argument order (color before label) is '
                    'deprecated.',
                    DeprecationWarning, stacklevel=2)
    style = common._BasicStyle(color)
    super(Segment, self).__init__([size], label=label, style=style)
    assert size >= 0

  def _GetSize(self):
    return self.data[0]

  def _SetSize(self, value):
    assert value >= 0
    self.data[0] = value

  size = property(_GetSize, _SetSize,
                  doc = """The relative size of this pie segment.""")

  # Since Segments are so simple, provide color for convenience.
  def _GetColor(self):
    return self.style.color

  def _SetColor(self, color):
    self.style.color = color

  color = property(_GetColor, _SetColor,
                   doc = """The color of this pie segment.""")


class PieChart(common.BaseChart):
  """Represents a pie chart.

  The pie chart consists of a single "pie" by default, but additional pies
  may be added using the AddPie method. The Google Chart API will display
  the pies as concentric circles, with pie #0 on the inside; other backends
  may display the pies differently.
  """

  def __init__(self, points=None, labels=None, colors=None):
    """Constructor for PieChart objects.

    Creates a pie chart with a single pie.

    Args:
      points: A list of data points for the pie chart;
              i.e., relative sizes of the pie segments
      labels: A list of labels for the pie segments.
              TODO: Allow the user to pass in None as one of
              the labels in order to skip that label.
      colors: A list of colors for the pie segments, as hex strings
              (f.ex. '0000ff' for blue). If there are less colors than pie
              segments, the Google Chart API will attempt to produce a smooth
              color transition between segments by spreading the colors across
              them.
    """
    super(PieChart, self).__init__()
    self.formatters = []
    self._colors = None
    if points:
      self.AddPie(points, labels, colors)

  def AddPie(self, points, labels=None, colors=None):
    """Add a whole pie to the chart.

    Args:
      points: A list of pie segment sizes
      labels: A list of labels for the pie segments
      colors: A list of colors for the segments. Missing colors will be chosen
          automatically.
    Return:
      The index of the newly added pie.
    """
    num_colors = len(colors or [])
    num_labels = len(labels or [])
    pie_index = len(self.data)
    self.data.append([])
    for i, pt in enumerate(points):
      label = None
      if i < num_labels:
        label = labels[i]
      color = None
      if i < num_colors:
        color = colors[i]
      self.AddSegment(pt, label=label, color=color, pie_index=pie_index)
    return pie_index

  def AddSegments(self, points, labels, colors):
    """DEPRECATED."""
    warnings.warn('PieChart.AddSegments is deprecated. Call AddPie instead. ',
                  DeprecationWarning, stacklevel=2)
    num_colors = len(colors or [])
    for i, pt in enumerate(points):
      assert pt >= 0
      label = labels[i]
      color = None
      if i < num_colors:
        color = colors[i]
      self.AddSegment(pt, label=label, color=color)

  def AddSegment(self, size, label=None, color=None, pie_index=0):
    """Add a pie segment to this chart, and return the segment.

    size: The size of the segment.
    label: The label for the segment.
    color: The color of the segment, or None to automatically choose the color.
    pie_index: The index of the pie that will receive the new segment.
      By default, the chart has one pie (pie #0); use the AddPie method to
      add more pies.
    """
    if isinstance(size, Segment):
      warnings.warn("AddSegment(segment) is deprecated.  Use AddSegment(size, "
                    "label, color) instead",  DeprecationWarning, stacklevel=2)
      segment = size
    else:
      segment = Segment(size, label=label, color=color)
    assert segment.size >= 0
    if pie_index == 0 and not self.data:
      # Create the default pie
      self.data.append([])
    assert (pie_index >= 0 and pie_index < len(self.data))
    self.data[pie_index].append(segment)
    return segment

  def AddSeries(self, points, color=None, style=None, markers=None, label=None):
    """DEPRECATED

    Add a new segment to the chart and return it.

    The segment must contain exactly one data point; all parameters
    other than color and label are ignored.
    """
    warnings.warn('PieChart.AddSeries is deprecated.  Call AddSegment or '
                  'AddSegments instead.', DeprecationWarning)
    return self.AddSegment(Segment(points[0], color=color, label=label))

  def SetColors(self, *colors):
    """Change the colors of this chart to the specified list of colors.

    Note that this will completely override the individual colors specified
    in the pie segments. Missing colors will be interpolated, so that the
    list of colors covers all segments in all the pies.
    """
    self._colors = colors
