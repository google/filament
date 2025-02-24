# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import argparse
import json
import os
import sys

from ggplot import *
import pandas


def _ConvertToSimplifiedFormat(values_list):
  data = {}
  for trace_name, result_data in values_list:
    result_values = result_data['pairs']['values']

    values = {}
    for current_value in result_values:
      grouping_key = current_value['name']

      if current_value['type'] == 'numeric':
        numeric_value = current_value['numeric']
        if numeric_value['type'] == 'scalar':
          values[grouping_key] = [current_value['numeric']['value']]
        elif numeric_value['type'] == 'numeric':
          # Let's just skip histograms for now.
          histogram_values = []
          for bin in numeric_value['centralBins']:
            histo_mid = (bin['max'] - bin['min']) * 0.5
            histogram_values += [histo_mid for _ in xrange(bin['count'])]
          values[grouping_key] = histogram_values

    data[trace_name] = values
  return data


def _ReadValuesOutput(file_name, metric_names):
  with open(file_name, 'r') as f:
    results_list = f.read()

    try:
      # Try metrics format first, which is a dict of results.
      results_together = json.loads(results_list).iteritems()
    except ValueError:
      # Try flume pipeline output format, which is a 1 result per line.
      r = [json.loads(s) for s in results_list.splitlines()]
      results_together = dict([(str(i), r[i]) for i in xrange(len(r))])

    simplified_data = _ConvertToSimplifiedFormat(results_together)

    return _ConvertValuesToPD(simplified_data, metric_names)


def _ConvertValuesToPD(all_values_data, metric_names):
  pd_dict = {}

  for trace_name, values in all_values_data.iteritems():
    for metric_name, metric_value in values.iteritems():
      if not metric_name in metric_names:
        continue
      if not metric_name in pd_dict:
        pd_dict[metric_name] = []
      pd_dict[metric_name].extend(metric_value)
  return pandas.DataFrame(pd_dict)


def _DoGGPlot(graph_type, data, x, y):
  if graph_type == 'histogram':
    print ggplot(aes(x=x), data=data) + geom_histogram()
  elif graph_type == 'point':
    print (ggplot(aes(x=x, y=y), data=data) + geom_point() +
        stat_smooth(color='black', se=True))


def Main():
  # Parse options.
  parser = argparse.ArgumentParser()
  parser.add_argument('--x', help='X-Axis parameter.')
  parser.add_argument('--y', help='X-Axis parameter.')
  parser.add_argument('--graph-type',
                      choices=['histogram', 'point'],
                      help='Type of graph.')
  parser.add_argument('--source',
                      help='Path to file containing results of metrics run.')
  args = parser.parse_args()

  if not args.source:
    parser.error('Source file not specified. Use --source to specify '
                 'path to source file.')

  if not args.graph_type:
    parser.error('Graph type not specified. Use --graph-type.')

  metric_names = []
  if args.x:
    metric_names.append(args.x)
  if args.y:
    metric_names.append(args.y)

  data = _ReadValuesOutput(os.path.abspath(args.source), metric_names)

  _DoGGPlot(args.graph_type, data, args.x, args.y)
