#!/usr/bin/env python
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Calculates significance thresholds for functional comparisons."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import itertools
import math

from scipy import stats

# The approximate false negative rate.
P_VALUE = 0.01


def main():
  # failure_rate is the size of differences we are trying to detect
  # with a P_VALUE false negative rate. Smaller differences need
  # larger samples to have the same false negative rate.
  for failure_rate in (.1, .2, .3, .4, .5, .6, .7, .8, .9, 1):
    # Calculate the threshold for every sample_size, stopping
    # when the threshold is lower than P_VALUE.
    thresholds = []
    for sample_size in itertools.count(1):
      threshold = _Threshold(P_VALUE, failure_rate, sample_size)
      thresholds.append(threshold)
      if threshold < P_VALUE:
        break

    _PrintThresholds(failure_rate, thresholds)


def _Threshold(p_value, failure_rate, sample_size):
  """Calculates a significance threshold.

  Arguments:
    p_value: The approximate target false negative rate.
    failure_rate: The size of differences we are trying to detect with a p_value
        false negative rate. If we want to detect that a test went from 0%
        failing to 40% failing, this is 0.4.
    sample_size: The number of values in each sample. The bigger the samples,
        the more statistical power we have, and the lower the threshold.

  Returns:
    The significance threshold.
  """
  # Use the binomial distribution to find the sample of pass/fails where the
  # given sample or more extreme samples have p_value probability of occurring.
  failure_count = int(stats.binom(sample_size, failure_rate).ppf(p_value))
  a = [0] * sample_size
  b = [0] * (sample_size - failure_count) + [1] * failure_count
  try:
    return stats.mannwhitneyu(a, b, alternative='two-sided').pvalue
  except ValueError:
    return 1.0


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
