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

"""Code related to line charts."""

import copy
import warnings

from graphy import common

class LineStyle(object):

  """Represents the style for a line on a line chart.  Also provides some
  convenient presets.

  Object attributes (Passed directly to the Google Chart API. Check there for
  details):
    width: Width of the line
    on:    Length of a line segment (for dashed/dotted lines)
    off:   Length of a break (for dashed/dotted lines)
    color: Color of the line.  A hex string, like 'ff0000' for red.  Optional,
           AutoColor will fill this in for you automatically if empty.

  Some common styles, such as LineStyle.dashed, are available:
    LineStyle.solid()
    LineStyle.dashed()
    LineStyle.dotted()
    LineStyle.thick_solid()
    LineStyle.thick_dashed()
    LineStyle.thick_dotted()
  """

  # Widths
  THIN = 1
  THICK = 2

  # Patterns
  # ((on, off) tuples, as passed to LineChart.AddLine)
  SOLID = (1, 0)
  DASHED = (8, 4)
  DOTTED = (2, 4)

  def __init__(self, width, on, off, color=None):
    """Construct a LineStyle.  See class docstring for details on args."""
    self.width = width
    self.on = on
    self.off = off
    self.color = color

  @classmethod
  def solid(cls):
    return LineStyle(1, 1, 0)
    
  @classmethod
  def dashed(cls):
    return LineStyle(1, 8, 4)
   
  @classmethod
  def dotted(cls):
    return LineStyle(1, 2, 4)
      
  @classmethod
  def thick_solid(cls):
    return LineStyle(2, 1, 0)   

  @classmethod
  def thick_dashed(cls):
    return LineStyle(2, 8, 4)   

  @classmethod
  def thick_dotted(cls):
    return LineStyle(2, 2, 4)  


class LineChart(common.BaseChart):

  """Represents a line chart."""

  def __init__(self, points=None):
    super(LineChart, self).__init__()
    if points is not None:
      self.AddLine(points)

  def AddLine(self, points, label=None, color=None,
              pattern=LineStyle.SOLID, width=LineStyle.THIN, markers=None):
    """Add a new line to the chart.

    This is a convenience method which constructs the DataSeries and appends it
    for you.  It returns the new series.

      points:  List of equally-spaced y-values for the line
      label:   Name of the line (used for the legend)
      color:   Hex string, like 'ff0000' for red
      pattern: Tuple for (length of segment, length of gap).  i.e.
               LineStyle.DASHED
      width:   Width of the line (i.e. LineStyle.THIN)
      markers: List of Marker objects to attach to this line (see DataSeries
               for more info)
    """
    if color is not None and isinstance(color[0], common.Marker):
      warnings.warn('Your code may be broken! '
                    'You passed a list of Markers instead of a color. The '
                    'old argument order (markers before color) is deprecated.',
                    DeprecationWarning, stacklevel=2)
    style = LineStyle(width, pattern[0], pattern[1], color=color)
    series = common.DataSeries(points, label=label, style=style,
                               markers=markers)
    self.data.append(series)
    return series

  def AddSeries(self, points, color=None, style=LineStyle.solid, markers=None,
                label=None):
    """DEPRECATED"""
    warnings.warn('LineChart.AddSeries is deprecated.  Call AddLine instead. ',
                  DeprecationWarning, stacklevel=2)    
    return self.AddLine(points, color=color, width=style.width,
                        pattern=(style.on, style.off), markers=markers,
                        label=label)


class Sparkline(LineChart):
  """Represent a sparkline.  These behave like LineCharts,
  mostly, but come without axes.
  """
