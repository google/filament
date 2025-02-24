#!/usr/bin/env python
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Calculates significance thresholds for performance comparisons."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import math
import multiprocessing
import sys

import numpy
from scipy import stats

# The approximate false negative rate.
P_VALUE = 0.01

# The number of simulations to run. 1 million will give pretty
# stable numbers, but will take about 12 hours to run on 4 cores.
N = 1000000

# As the distances increase, the sample sizes we need decreases. It's
# costly to run too many extra repeats, but we need to know in advance how
# many repeats we want to run, so prepopulate a table with some estimated
# sample sizes. We can estimate these values by running this script and
# seeing how many samples are needed for the threshold to cross P_VALUE.
DISTANCES_AND_SAMPLE_SIZES = (
    (0.3, 360),
    (0.4, 240),
    (0.5, 120),
    (0.6, 90),
    (0.7, 70),
    (0.8, 50),
    (0.9, 40),
    (1.0, 35),
    (1.1, 25),
    (1.2, 25),
    (1.3, 20),
    (1.4, 20),
    (1.5, 15),
    (1.6, 15),
    (1.7, 15),
    (1.8, 15),
    (1.9, 10),
    (2.0, 10),
    (2.1, 10),
    (2.2, 10),
    (2.3, 10),
    (2.4, 10),
    (2.5, 10),
    (2.6, 10),
    (2.7, 10),
    (2.8, 10),
    (2.9, 10),
    (3.0, 10),
    (3.1, 10),
    (3.2, 10),
    (3.3, 10),
    (3.4, 10),
    (3.5, 10),
    (3.6, 10),
    (3.7, 10),
    (3.8, 10),
    (3.9, 10),
    (4.0, 10),
)


def main():
  # Distances are expressed in multiples of IQR. We need to convert them to
  # standard deviations.
  stddevs_per_iqr = stats.norm.ppf(0.75) - stats.norm.ppf(0.25)
  percentile = (1 - P_VALUE) * 100
  pool = multiprocessing.Pool()

  for distance_iqr, max_sample_size in DISTANCES_AND_SAMPLE_SIZES:
    # Assume two distributions that are distance_iqr apart. Simulate the
    # comparison between them N times, and take the P_VALUE percentile of the
    # results. P_VALUE simulations must then fall above that percentile,
    # so that percentile is the high threshold.
    thresholds = []
    arguments = distance_iqr * stddevs_per_iqr, max_sample_size
    all_p_values = pool.map(_PValues, [arguments] * N)
    for p_values in zip(*all_p_values):
      thresholds.append(numpy.percentile(p_values, percentile))

    _PrintThresholds(distance_iqr, thresholds)
    sys.stdout.flush()


def _PValues(arguments):
  """Performs a simulation of a comparison and returns the p-values.

  Starts with two normal distributions with a predetermined distance.
  Randomly pulls values from that distribution and calculates the
  running p-value as the samples grow in size, up to max_sample_size.

  Arguments:
    distance_stddev: The distance between the means of the two normal
        distributions, in multiples of the standard deviation.
    max_sample_size: The number of values to pull per sample.

  Returns:
    A list of p-values, from N=1 to N=max_sample_size.
  """
  distance_stddev, max_sample_size = arguments

  a = []
  b = []
  p_values = []
  for _ in range(max_sample_size):
    a.append(stats.norm.rvs())
    b.append(stats.norm.rvs(distance_stddev))
    p_values.append(stats.mannwhitneyu(a, b, alternative='two-sided').pvalue)
  return p_values


def _PrintThresholds(distance, thresholds):
  """Groups values into lines of 10 so they fit in the 80-character limit."""
  print('# ' + '%.1f' % distance)
  for i in range(0, len(thresholds), 10):
    threshold_line = thresholds[i:i + 10]
    print(', '.join(_Format(threshold) for threshold in threshold_line) + ',')


def _Format(number):
  """Makes all numbers a consistent length."""
  if number == 1:
    return '1.000'
  return ('%.4f' % (math.ceil(number * 10000) / 10000)).lstrip('0')


if __name__ == '__main__':
  main()
