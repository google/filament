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

"""Code common to all chart types."""

import copy
import warnings

from graphy import formatters
from graphy import util


class Marker(object):

  """Represents an abstract marker, without position.  You can attach these to
  a DataSeries.

  Object attributes:
    shape: One of the shape codes (Marker.arrow, Marker.diamond, etc.)
    color: color (as hex string, f.ex. '0000ff' for blue)
    size:  size of the marker
  """
  # TODO: Write an example using markers.

  # Shapes:
  arrow = 'a'
  cross = 'c'
  diamond = 'd'
  circle = 'o'
  square = 's'
  x = 'x'

  # Note: The Google Chart API also knows some other markers ('v', 'V', 'r',
  # 'b') that I think would fit better into a grid API.
  # TODO: Make such a grid API

  def __init__(self, shape, color, size):
    """Construct a Marker.  See class docstring for details on args."""
    # TODO: Shapes 'r' and 'b' would be much easier to use if they had a
    # special-purpose API (instead of trying to fake it with markers)
    self.shape = shape
    self.color = color
    self.size = size


class _BasicStyle(object):
  """Basic style object.  Used internally."""

  def __init__(self, color):
    self.color = color


class DataSeries(object):

  """Represents one data series for a chart (both data & presentation
  information).

  Object attributes:
    points:  List of numbers representing y-values (x-values are not specified
             because the Google Chart API expects even x-value spacing).
    label:   String with the series' label in the legend.  The chart will only
             have a legend if at least one series has a label.  If some series
             do not have a label then they will have an empty description in
             the legend.  This is currently a limitation in the Google Chart
             API.
    style:   A chart-type-specific style object.  (LineStyle for LineChart,
             BarsStyle for BarChart, etc.)
    markers: List of (x, m) tuples where m is a Marker object and x is the
             x-axis value to place it at.

             The "fill" markers ('r' & 'b') are a little weird because they
             aren't a point on a line.  For these, you can fake it by
             passing slightly weird data (I'd like a better API for them at
             some point):
               For 'b', you attach the marker to the starting series, and set x
               to the index of the ending line.  Size is ignored, I think.

               For 'r', you can attach to any line, specify the starting
               y-value for x and the ending y-value for size.  Y, in this case,
               is becase 0.0 (bottom) and 1.0 (top).
    color:   DEPRECATED
  """

  # TODO: Should we require the points list to be non-empty ?
  # TODO: Do markers belong here?  They are really only used for LineCharts
  def __init__(self, points, label=None, style=None, markers=None, color=None):
    """Construct a DataSeries.  See class docstring for details on args."""
    if label is not None and util._IsColor(label):
      warnings.warn('Your code may be broken! Label is a hex triplet.  Maybe '
                    'it is a color? The old argument order (color & style '
                    'before label) is deprecated.', DeprecationWarning,
                    stacklevel=2)
    if color is not None:
      warnings.warn('Passing color is deprecated.  Pass a style object '
                    'instead.', DeprecationWarning, stacklevel=2)
      # Attempt to fix it for them.  If they also passed a style, honor it.
      if style is None:
        style = _BasicStyle(color)
    if style is not None and isinstance(style, basestring):
      warnings.warn('Your code is broken! Style is a string, not an object. '
                    'Maybe you are passing a color?  Passing color is '
                    'deprecated; pass a style object instead.',
                    DeprecationWarning, stacklevel=2)
    if style is None:
      style = _BasicStyle(None)
    self.data = points
    self.style = style
    self.markers = markers or []
    self.label = label

  def _GetColor(self):
    warnings.warn('DataSeries.color is deprecated, use '
                  'DataSeries.style.color instead.', DeprecationWarning,
                  stacklevel=2)
    return self.style.color

  def _SetColor(self, color):
    warnings.warn('DataSeries.color is deprecated, use '
                  'DataSeries.style.color instead.', DeprecationWarning,
                  stacklevel=2)
    self.style.color = color
  
  color = property(_GetColor, _SetColor)    
    
  def _GetStyle(self):
    return self._style;
  
  def _SetStyle(self, style):
    if style is not None and callable(style):
      warnings.warn('Your code may be broken ! LineStyle.solid and similar '
                    'are no longer constants, but class methods that '
                    'create LineStyle instances. Change your code to call '
                    'LineStyle.solid() instead of passing it as a value.',
                    DeprecationWarning, stacklevel=2)
      self._style = style()
    else:  
      self._style = style   
  
  style = property(_GetStyle, _SetStyle)    


class AxisPosition(object):
  """Represents all the available axis positions.

  The available positions are as follows:
    AxisPosition.TOP
    AxisPosition.BOTTOM
    AxisPosition.LEFT
    AxisPosition.RIGHT
  """
  LEFT = 'y'
  RIGHT = 'r'
  BOTTOM = 'x'
  TOP = 't'


class Axis(object):

  """Represents one axis.

  Object setings:
    min:    Minimum value for the bottom or left end of the axis
    max:    Max value.
    labels: List of labels to show along the axis.
    label_positions: List of positions to show the labels at.  Uses the scale
                     set by min & max, so if you set min = 0 and max = 10, then
                     label positions [0, 5, 10] would be at the bottom,
                     middle, and top of the axis, respectively.
    grid_spacing:  Amount of space between gridlines (in min/max scale).
                   A value of 0 disables gridlines.
    label_gridlines: If True, draw a line extending from each label
                   on the axis all the way across the chart.
  """

  def __init__(self, axis_min=None, axis_max=None):
    """Construct a new Axis.

    Args:
      axis_min: smallest value on the axis
      axis_max: largest value on the axis
    """
    self.min = axis_min
    self.max = axis_max
    self.labels = []
    self.label_positions = []
    self.grid_spacing = 0
    self.label_gridlines = False

# TODO: Add other chart types.  Order of preference:
# - scatter plots
# - us/world maps

class BaseChart(object):
  """Base chart object with standard behavior for all other charts.

  Object attributes:
    data: List of DataSeries objects. Chart subtypes provide convenience
          functions (like AddLine, AddBars, AddSegment) to add more series
          later.
    left/right/bottom/top: Axis objects for the 4 different axes.
    formatters: A list of callables which will be used to format this chart for
                display.  TODO: Need better documentation for how these
                work.
    auto_scale, auto_color, auto_legend:
      These aliases let users access the default formatters without poking
      around in self.formatters.  If the user removes them from
      self.formatters then they will no longer be enabled, even though they'll
      still be accessible through the aliases.  Similarly, re-assigning the
      aliases has no effect on the contents of self.formatters.
    display: This variable is reserved for backends to populate with a display
             object.  The intention is that the display object would be used to
             render this chart.  The details of what gets put here depends on
             the specific backend you are using.
  """

  # Canonical ordering of position keys
  _POSITION_CODES = 'yrxt'

  # TODO: Add more inline args to __init__ (esp. labels).
  # TODO: Support multiple series in the constructor, if given.
  def __init__(self):
    """Construct a BaseChart object."""
    self.data = []

    self._axes = {}
    for code in self._POSITION_CODES:
      self._axes[code] = [Axis()]
    self._legend_labels = []  # AutoLegend fills this out
    self._show_legend = False  # AutoLegend fills this out

    # Aliases for default formatters
    self.auto_color = formatters.AutoColor()
    self.auto_scale = formatters.AutoScale()
    self.auto_legend = formatters.AutoLegend
    self.formatters = [self.auto_color, self.auto_scale, self.auto_legend]
    # display is used to convert the chart into something displayable (like a
    # url or img tag).
    self.display = None

  def AddFormatter(self, formatter):
    """Add a new formatter to the chart (convenience method)."""
    self.formatters.append(formatter)

  def AddSeries(self, points, color=None, style=None, markers=None,
                label=None):
    """DEPRECATED

    Add a new series of data to the chart; return the DataSeries object."""
    warnings.warn('AddSeries is deprecated.  Instead, call AddLine for '
                  'LineCharts, AddBars for BarCharts, AddSegment for '
                  'PieCharts ', DeprecationWarning, stacklevel=2)
    series = DataSeries(points, color=color, style=style, markers=markers,
                        label=label)
    self.data.append(series)
    return series

  def GetDependentAxes(self):
    """Return any dependent axes ('left' and 'right' by default for LineCharts,
    although bar charts would use 'bottom' and 'top').
    """
    return self._axes[AxisPosition.LEFT] + self._axes[AxisPosition.RIGHT]

  def GetIndependentAxes(self):
    """Return any independent axes (normally top & bottom, although horizontal
    bar charts use left & right by default).
    """
    return self._axes[AxisPosition.TOP] + self._axes[AxisPosition.BOTTOM]

  def GetDependentAxis(self):
    """Return this chart's main dependent axis (often 'left', but
    horizontal bar-charts use 'bottom').
    """
    return self.left

  def GetIndependentAxis(self):
    """Return this chart's main independent axis (often 'bottom', but
    horizontal bar-charts use 'left').
    """
    return self.bottom

  def _Clone(self):
    """Make a deep copy this chart.

    Formatters & display will be missing from the copy, due to limitations in
    deepcopy.
    """
    orig_values = {}
    # Things which deepcopy will likely choke on if it tries to copy.
    uncopyables = ['formatters', 'display', 'auto_color', 'auto_scale',
                   'auto_legend']
    for name in uncopyables:
      orig_values[name] = getattr(self, name)
      setattr(self, name, None)
    clone = copy.deepcopy(self)
    for name, orig_value in orig_values.iteritems():
      setattr(self, name, orig_value)
    return clone

  def GetFormattedChart(self):
    """Get a copy of the chart with formatting applied."""
    # Formatters need to mutate the chart, but we don't want to change it out
    # from under the user.  So, we work on a copy of the chart.
    scratchpad = self._Clone()
    for formatter in self.formatters:
      formatter(scratchpad)
    return scratchpad

  def GetMinMaxValues(self):
    """Get the largest & smallest values in this chart, returned as
    (min_value, max_value).  Takes into account complciations like stacked data
    series.

    For example, with non-stacked series, a chart with [1, 2, 3] and [4, 5, 6]
    would return (1, 6).  If the same chart was stacking the data series, it
    would return (5, 9).
    """
    MinPoint = lambda data: min(x for x in data if x is not None)
    MaxPoint = lambda data: max(x for x in data if x is not None)
    mins  = [MinPoint(series.data) for series in self.data if series.data]
    maxes = [MaxPoint(series.data) for series in self.data if series.data]
    if not mins or not maxes:
      return None, None # No data, just bail.
    return min(mins), max(maxes)

  def AddAxis(self, position, axis):
    """Add an axis to this chart in the given position.

    Args:
      position: an AxisPosition object specifying the axis's position
      axis: The axis to add, an Axis object
    Returns:
      the value of the axis parameter
    """
    self._axes.setdefault(position, []).append(axis)
    return axis

  def GetAxis(self, position):
    """Get or create the first available axis in the given position.

    This is a helper method for the left, right, top, and bottom properties.
    If the specified axis does not exist, it will be created.

    Args:
      position: the position to search for
    Returns:
      The first axis in the given position
    """
    # Not using setdefault here just in case, to avoid calling the Axis()
    # constructor needlessly
    if position in self._axes:
      return self._axes[position][0]
    else:
      axis = Axis()
      self._axes[position] = [axis]
      return axis

  def SetAxis(self, position, axis):
    """Set the first axis in the given position to the given value.

    This is a helper method for the left, right, top, and bottom properties.

    Args:
      position: an AxisPosition object specifying the axis's position
      axis: The axis to set, an Axis object
    Returns:
      the value of the axis parameter
    """
    self._axes.setdefault(position, [None])[0] = axis
    return axis

  def _GetAxes(self):
    """Return a generator of (position_code, Axis) tuples for this chart's axes.

    The axes will be sorted by position using the canonical ordering sequence,
    _POSITION_CODES.
    """
    for code in self._POSITION_CODES:
      for axis in self._axes.get(code, []):
        yield (code, axis)

  def _GetBottom(self):
    return self.GetAxis(AxisPosition.BOTTOM)

  def _SetBottom(self, value):
    self.SetAxis(AxisPosition.BOTTOM, value)

  bottom = property(_GetBottom, _SetBottom,
                    doc="""Get or set the bottom axis""")

  def _GetLeft(self):
    return self.GetAxis(AxisPosition.LEFT)

  def _SetLeft(self, value):
    self.SetAxis(AxisPosition.LEFT, value)

  left = property(_GetLeft, _SetLeft,
                  doc="""Get or set the left axis""")

  def _GetRight(self):
    return self.GetAxis(AxisPosition.RIGHT)

  def _SetRight(self, value):
    self.SetAxis(AxisPosition.RIGHT, value)

  right = property(_GetRight, _SetRight,
                   doc="""Get or set the right axis""")

  def _GetTop(self):
    return self.GetAxis(AxisPosition.TOP)

  def _SetTop(self, value):
    self.SetAxis(AxisPosition.TOP, value)

  top = property(_GetTop, _SetTop,
                 doc="""Get or set the top axis""")
