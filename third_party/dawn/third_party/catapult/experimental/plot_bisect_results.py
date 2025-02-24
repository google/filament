#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script to plot the results of a bisect run."""

from __future__ import absolute_import
from __future__ import print_function
import argparse
import json
import math
import re
import six.moves.urllib.request
import six.moves.urllib.error
import six.moves.urllib.parse

from matplotlib import cm  # pylint: disable=import-error
from matplotlib import pyplot  # pylint: disable=import-error
import numpy  # pylint: disable=import-error
from six.moves import zip


_PLOT_WIDTH_INCHES = 8
_PLOT_HEIGHT_INCHES = 6
_PERCENTILES = (0, 0.05, 0.25, 0.5, 0.75, 0.95, 1)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('bisect_url_or_debug_info_file',
                      help='The Buildbot URL of a bisect run, or a file '
                      'containing the output from the Debug Info step.')
  parser.add_argument('output', nargs='?', help='File path to save a PNG to.')
  args = parser.parse_args()

  url = (args.bisect_url_or_debug_info_file +
         '/steps/Debug%20Info/logs/Debug%20Info/text')
  try:
    f = six.moves.urllib.request.urlopen(url)
  except ValueError:  # Not a valid URL.
    f = open(args.bisect_url_or_debug_info_file, 'r')

  results = []
  for line in f.readlines():
    regex = (r'(?:(?:[a-z0-9-]+@)?[a-z0-9]+,)*'
             r'(?:[a-z0-9-]+@)?(?P<commit>[a-z0-9]+)\s*'
             r'(?P<values>\[(?:-?[0-9.]+, )*-?[0-9.]*\])')
    match = re.match(regex, line)
    if not match:
      continue

    commit = match.group('commit')
    values = json.loads(match.group('values'))
    if not values:
      continue

    print(commit, values)
    results.append((commit, values))

  _SavePlots(results, args.output)


def _SavePlots(results, file_path=None):
  """Saves histograms and empirial distribution plots showing the diff.

  Args:
    file_path: The location to save the plots go.
  """
  figsize = (_PLOT_WIDTH_INCHES * 2, _PLOT_HEIGHT_INCHES)
  _, (axis0, axis1) = pyplot.subplots(nrows=1, ncols=2, figsize=figsize)

  _DrawHistogram(axis0, results)
  _DrawEmpiricalCdf(axis1, results)

  if file_path:
    pyplot.savefig(file_path)
  pyplot.show()
  pyplot.close()


def _DrawHistogram(axis, results):
  values_per_commit = [values for _, values in results]

  # Calculate bounds and bins.
  combined_values = sum(values_per_commit, [])
  lower_bound = min(combined_values)
  upper_bound = max(combined_values)
  if lower_bound == upper_bound:
    lower_bound -= 0.5
    upper_bound += 0.5
  bins = numpy.linspace(lower_bound, upper_bound,
                        math.log(len(combined_values)) * 4)

  # Histograms.
  colors = cm.rainbow(numpy.linspace(  # pylint: disable=no-member
      1, 0, len(results) + 1))
  for (commit, values), color in zip(results, colors):
    axis.hist(values, bins, alpha=0.5, normed=True, histtype='stepfilled',
              label='%s (n=%d)' % (commit, len(values)), color=color)

  # Vertical lines denoting the medians.
  medians = tuple(numpy.percentile(values, 50) for values in values_per_commit)
  axis.set_xticks(medians, minor=True)
  axis.grid(which='minor', axis='x', linestyle='--')

  # Axis labels and legend.
  #axis.set_xlabel(step.metric_name)
  axis.set_ylabel('Relative probability')
  axis.legend(loc='upper right')


def _DrawEmpiricalCdf(axis, results):
  colors = cm.rainbow(numpy.linspace(  # pylint: disable=no-member
      1, 0, len(results) + 1))
  for (commit, values), color in zip(results, colors):
    # Empirical distribution function.
    levels = numpy.linspace(0, 1, len(values) + 1)
    axis.step(sorted(values) + [max(values)], levels,
              label='%s (n=%d)' % (commit, len(values)), color=color)

    # Dots denoting the percentiles.
    axis.plot(numpy.percentile(values, tuple(p * 100 for p in _PERCENTILES)),
              _PERCENTILES, '.', color=color)

  axis.set_yticks(_PERCENTILES)

  # Vertical lines denoting the medians.
  values_per_commit = [values for _, values in results]
  medians = tuple(numpy.percentile(values, 50) for values in values_per_commit)
  axis.set_xticks(medians, minor=True)
  axis.grid(which='minor', axis='x', linestyle='--')

  # Axis labels and legend.
  #axis.set_xlabel(step.metric_name)
  axis.set_ylabel('Cumulative probability')
  axis.legend(loc='lower right')


if __name__ == '__main__':
  main()
