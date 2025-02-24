# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""An experimental alternative to find_change_points.FindChangePoints.

The general approach that this function takes is similar to that
of skiaperf.com. For the implementation that this is based on, see:
http://goo.gl/yikYZY
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import math

from dashboard.common import math_utils


def FindStep(data_series, score_threshold=4.0):
  """Finds a point in the data series where there appears to be step.

  Args:
    data_series: A list of ordered (x, y) pairs.
    score_threshold: The minimum "regression size" score for any
        step to be considered sufficiently large.

  Returns:
    The x-value at which there appears to be a step, or None.
  """
  if len(data_series) < 2:
    return None
  x_values, y_values = list(zip(*data_series))
  step_index = _MinimizeDistanceFromStep(y_values)
  score = _RegressionSizeScore(y_values, step_index)
  if score > score_threshold:
    return x_values[step_index]
  return None


def Steppiness(values, step_index):
  """Returns a number between 0 and 1 that indicates how step-like |values| is.

  Args:
    values: A list of numbers. Should have non-zero variance, non-zero length.
    step_index: An index in values.

  Returns:
    A number between zero and one that indicates how similar the shape of the
    given series is to a step function.
  """
  normalized = _Normalize(values)
  return 1.0 - _DistanceFromStep(normalized, step_index)


def _Normalize(values):
  """Makes a series with the same shape but with variance = 1, mean = 0."""
  mean = math_utils.Mean(values)
  zeroed = [x - mean for x in values]
  stddev = math_utils.StandardDeviation(zeroed)
  return [math_utils.Divide(x, stddev) for x in zeroed]


def _MinimizeDistanceFromStep(values):
  """Returns the step index which minimizes difference from a step function."""
  indices = list(range(1, len(values)))
  return min(indices, key=lambda i: _DistanceFromStep(values, i))


def _RegressionSizeScore(values, step_index):
  """Returns the "regression size" score at the given point."""
  left, right = values[:step_index], values[step_index:]
  step_size = abs(math_utils.Mean(left) - math_utils.Mean(right))
  if not step_size:
    return 0.0
  distance = _DistanceFromStep(values, step_index)
  if not distance:
    return float('inf')
  return step_size / distance


def _DistanceFromStep(values, step_index):
  """Returns the root-mean-square deviation from a step function."""
  step_values = _StepFunctionValues(values, step_index)
  return _RootMeanSquareDeviation(values, step_values)


def _StepFunctionValues(values, step_index):
  """Makes values for a step function corresponding to |values|."""
  left, right = values[:step_index], values[step_index:]
  step_left = len(left) * [math_utils.Mean(left)]
  step_right = len(right) * [math_utils.Mean(right)]
  return step_left + step_right


def _RootMeanSquareDeviation(values1, values2):
  """Returns the root-mean-square deviation between two lists of values."""
  if len(values1) == 0:
    return 0.0
  return math.sqrt(_SumOfSquaredResiduals(values1, values2) / len(values1))


def _SumOfSquaredResiduals(values1, values2):
  """Returns the sum of the squared deviations between corresponding values."""
  return sum((v1 - v2)**2 for v1, v2 in zip(values1, values2))
