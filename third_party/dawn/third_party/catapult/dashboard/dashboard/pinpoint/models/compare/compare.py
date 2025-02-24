# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging

from dashboard.pinpoint.models.compare import kolmogorov_smirnov
from dashboard.pinpoint.models.compare import mann_whitney_u
from dashboard.pinpoint.models.compare import thresholds

DIFFERENT = 'different'
PENDING = 'pending'
SAME = 'same'
UNKNOWN = 'unknown'

_MIN_HIGH_THRESHOLDS_FUNCTIONAL = 0.10
_MIN_LOW_THRESHOLDS_FUNCTIONAL = 0.05


class ComparisonResults(
    collections.namedtuple(
        'ComparisonResults',
        ('result', 'p_value', 'low_threshold', 'high_threshold'))):
  __slots__ = ()


# TODO(https://crbug.com/1051710): Make this return all the values useful in
# decision making (and display).
def Compare(values_a,
            values_b,
            attempt_count,
            mode,
            magnitude,
            benchmark_arguments=None,
            job_id=None):
  """Decide whether two samples are the same, different, or unknown.

  Arguments:
    values_a: A list of sortable values. They don't need to be numeric.
    values_b: A list of sortable values. They don't need to be numeric.
    attempt_count: The average number of attempts made.
    mode: 'functional' or 'performance'. We use different significance
      thresholds for each type.
    magnitude: An estimate of the size of differences to look for. We need more
      values to find smaller differences. If mode is 'functional', this is the
      failure rate, a float between 0 and 1. If mode is 'performance', this is a
      multiple of the interquartile range (IQR).

  Returns:
    A tuple `ComparisonResults` which contains the following elements:
      * result: one of the following values:
          DIFFERENT: The samples are unlikely to come from the same
                     distribution, and are therefore likely different. Reject
                     the null hypothesis.
          SAME     : The samples are unlikely to come from distributions that
                     differ by the given magnitude. Cannot reject the null
                     hypothesis.
          UNKNOWN  : Not enough evidence to reject either hypothesis. We should
                     collect more data before making a final decision.
      * p_value: the consolidated p-value for the statistical tests used in the
                 implementation.
      * low_threshold: the `alpha` where if the p-value is lower means we can
                       reject the null hypothesis.
      * high_threshold: the `alpha` where if the p-value is lower means we need
                        more information to make a definitive judgement.
  """
  low_threshold = thresholds.LowThreshold()
  high_threshold = thresholds.HighThreshold(mode, magnitude, attempt_count)

  if not (len(values_a) > 0 and len(values_b) > 0):
    # A sample has no values in it.
    return ComparisonResults(UNKNOWN, None, low_threshold, high_threshold)

  # MWU is bad at detecting changes in variance, and K-S is bad with discrete
  # distributions. So use both. We want low p-values for the below examples.
  #        a                     b               MWU(a, b)  KS(a, b)
  # [0]*20            [0]*15+[1]*5                0.0097     0.4973
  # range(10, 30)     range(10)+range(30, 40)     0.4946     0.0082
  kolmogorov_p_value = kolmogorov_smirnov.KolmogorovSmirnov(values_a, values_b)
  mann_p_value = mann_whitney_u.MannWhitneyU(values_a, values_b)
  p_value = min(kolmogorov_p_value, mann_p_value)
  logging.debug(
    'BisectDebug: kolmogorov_p_value: %s, mann_p_value: %s, actual_p_value: %s',
    kolmogorov_p_value,
    mann_p_value,
    p_value)

  new_low_threshold = max(low_threshold, _MIN_LOW_THRESHOLDS_FUNCTIONAL)
  new_high_threshold = max(high_threshold, _MIN_HIGH_THRESHOLDS_FUNCTIONAL)

  logging.debug(
    'BisectDebug: pv: %s, lt: %s, ht: %s, mg: %s, ac: %s, va: %s, vb: %s',
    p_value,
    low_threshold,
    high_threshold,
    magnitude,
    attempt_count,
    values_a,
    values_b)

  if p_value <= new_low_threshold:
    new_comparison_result = ComparisonResults(DIFFERENT, p_value,
                                              new_low_threshold,
                                              new_high_threshold)
  elif p_value > new_high_threshold:
    new_comparison_result = ComparisonResults(SAME, p_value, new_low_threshold,
                                                   new_high_threshold)
  else:
    new_comparison_result = ComparisonResults(UNKNOWN, p_value,
                                              new_low_threshold,
                                              new_high_threshold)
  logging.debug('BisectDebug: new_comparison_result: %s', new_comparison_result)

  if p_value <= low_threshold:
    # The p-value is less than the significance level. Reject the null
    # hypothesis.
    comparison_result = ComparisonResults(DIFFERENT, p_value, low_threshold,
                                          high_threshold)
  elif p_value <= high_threshold:
    # The p-value is not less than the significance level, but it's small
    # enough to be suspicious. We'd like to investigate more closely.
    comparison_result = ComparisonResults(UNKNOWN, p_value, low_threshold,
                                           high_threshold)
  else:
    # The p-value is quite large. We're not suspicious that the two samples might
    # come from different distributions, and we don't care to investigate more.
    comparison_result = ComparisonResults(SAME, p_value, low_threshold,
                                          high_threshold)
  logging.debug('BisectDebug: actual_comparison_result: %s', comparison_result)

  if comparison_result.result != new_comparison_result.result:
    if benchmark_arguments is not None:
      logging.debug(
          'BisectDebug: Found different comparison result, '
          'job_id: %s, benchmark: %s, chart: %s, story: %s, '
          'new result: %s, actual result: %s', job_id,
          benchmark_arguments.benchmark, benchmark_arguments.chart,
          benchmark_arguments.story, new_comparison_result, comparison_result)
      # Apply the new comparison result to all benchmarks
      logging.debug(
          'BisectDebug: Return new comparison result, '
          'job_id: %s, benchmark: %s, chart: %s, story: %s, '
          'new result: %s, old result: %s', job_id,
          benchmark_arguments.benchmark, benchmark_arguments.chart,
          benchmark_arguments.story, new_comparison_result,
          comparison_result)
      return new_comparison_result

    logging.debug(
        'BisectDebug: Found different comparison result, new result: %s, actual result: %s',
        new_comparison_result, comparison_result)

  return comparison_result
