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

"""Utility functions for working with the Google Chart API.

Not intended for end users, use the methods in __init__ instead."""

import cgi
import string
import urllib


# TODO: Find a better representation
LONG_NAMES = dict(
  client_id='chc',
  size='chs',
  chart_type='cht',
  axis_type='chxt',
  axis_label='chxl',
  axis_position='chxp',
  axis_range='chxr',
  axis_style='chxs',
  data='chd',
  label='chl',
  y_label='chly',
  data_label='chld',
  data_series_label='chdl',
  color='chco',
  extra='chp',
  right_label='chlr',
  label_position='chlp',
  y_label_position='chlyp',
  right_label_position='chlrp',
  grid='chg',
  axis='chx',
  # This undocumented parameter specifies the length of the tick marks for an
  # axis. Negative values will extend tick marks into the main graph area.
  axis_tick_marks='chxtc',
  line_style='chls',
  marker='chm',
  fill='chf',
  bar_size='chbh',
  bar_height='chbh',
  label_color='chlc',
  signature='sig',
  output_format='chof',
  title='chtt',
  title_style='chts',
  callback='callback',
  )

""" Used for parameters which involve joining multiple values."""
JOIN_DELIMS = dict(
  data=',',
  color=',',
  line_style='|',
  marker='|',
  axis_type=',',
  axis_range='|',
  axis_label='|',
  axis_position='|',
  axis_tick_marks='|',
  data_series_label='|',
  label='|',
  bar_size=',',
  bar_height=',',
)


class SimpleDataEncoder:

  """Encode data using simple encoding.  Out-of-range data will
  be dropped (encoded as '_').
  """

  def __init__(self):
    self.prefix = 's:'
    self.code = string.ascii_uppercase + string.ascii_lowercase + string.digits
    self.min = 0
    self.max = len(self.code) - 1

  def Encode(self, data):
    return ''.join(self._EncodeItem(i) for i in data)

  def _EncodeItem(self, x):
    if x is None:
      return '_'
    x = int(round(x))
    if x < self.min or x > self.max:
      return '_'
    return self.code[int(x)]


class EnhancedDataEncoder:

  """Encode data using enhanced encoding.  Out-of-range data will
  be dropped (encoded as '_').
  """

  def __init__(self):
    self.prefix = 'e:'
    chars = string.ascii_uppercase + string.ascii_lowercase + string.digits \
            + '-.'
    self.code = [x + y for x in chars for y in chars]
    self.min = 0
    self.max = len(self.code) - 1

  def Encode(self, data):
    return ''.join(self._EncodeItem(i) for i in data)

  def _EncodeItem(self, x):
    if x is None:
      return '__'
    x = int(round(x))
    if x < self.min or x > self.max:
      return '__'
    return self.code[int(x)]


def EncodeUrl(base, params, escape_url, use_html_entities):
  """Escape params, combine and append them to base to generate a full URL."""
  real_params = []
  for key, value in params.iteritems():
    if escape_url:
      value = urllib.quote(value)
    if value:
      real_params.append('%s=%s' % (key, value))
  if real_params:
    url = '%s?%s' % (base, '&'.join(real_params))
  else:
    url = base
  if use_html_entities:
    url = cgi.escape(url, quote=True)
  return url


def ShortenParameterNames(params):
  """Shorten long parameter names (like size) to short names (like chs)."""
  out = {}
  for name, value in params.iteritems():
    short_name = LONG_NAMES.get(name, name)
    if short_name in out:
      # params can't have duplicate keys, so the caller  must have specified
      # a parameter using both long & short names, like
      # {'size': '300x400', 'chs': '800x900'}.  We don't know which to use.
      raise KeyError('Both long and short version of parameter %s (%s) '
        'found.  It is unclear which one to use.' % (name, short_name))
    out[short_name] = value
  return out


def StrJoin(delim, data):
  """String-ize & join data."""
  return delim.join(str(x) for x in data)


def JoinLists(**args):
  """Take a dictionary of {long_name:values}, and join the values.

    For each long_name, join the values into a string according to
    JOIN_DELIMS.  If values is empty or None, replace with an empty string.

    Returns:
      A dictionary {long_name:joined_value} entries.
  """
  out = {}
  for key, val in args.items():
    if val:
      out[key] = StrJoin(JOIN_DELIMS[key], val)
    else:
      out[key] = ''
  return out


def EncodeData(chart, series, y_min, y_max, encoder):
  """Format the given data series in plain or extended format.

  Use the chart's encoder to determine the format. The formatted data will
  be scaled to fit within the range of values supported by the chosen
  encoding.

  Args:
    chart: The chart.
    series: A list of the the data series to format; each list element is
           a list of data points.
    y_min: Minimum data value. May be None if y_max is also None
    y_max: Maximum data value. May be None if y_min is also None
  Returns:
    A dictionary with one key, 'data', whose value is the fully encoded series.
  """
  assert (y_min is None) == (y_max is None)
  if y_min is not None:
    def _ScaleAndEncode(series):
      series = ScaleData(series, y_min, y_max, encoder.min, encoder.max)
      return encoder.Encode(series)
    encoded_series = [_ScaleAndEncode(s) for s in series]
  else:
    encoded_series = [encoder.Encode(s) for s in series]
  result = JoinLists(**{'data': encoded_series})
  result['data'] = encoder.prefix + result['data']
  return result


def ScaleData(data, old_min, old_max, new_min, new_max):
  """Scale the input data so that the range old_min-old_max maps to
  new_min-new_max.
  """
  def ScalePoint(x):
    if x is None:
      return None
    return scale * x + translate

  if old_min == old_max:
    scale = 1
  else:
    scale = (new_max - new_min) / float(old_max - old_min)
  translate = new_min - scale * old_min
  return map(ScalePoint, data)
