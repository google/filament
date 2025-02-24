# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""An approximation of the E-Divisive change detection algorithm.

This module implements the constituent functions and components for a change
detection module for time-series data. It derives heavily from the paper [0] on
E-Divisive using hierarchical significance testing and the Euclidean
distance-based divergence estimator.

[0]: https://arxiv.org/abs/1306.4933
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import itertools
import logging
import random

from dashboard.common import math_utils

# TODO(dberris): Remove this dependency if/when we are able to depend on SciPy
# instead.
from dashboard.pinpoint.models.compare import compare as pinpoint_compare

# This number controls the maximum number of iterations we perform when doing
# permutation testing to identify potential change-points hidden in the
# sub-clustering of values. The higher the number, the more CPU time we're
# likely to spend finding these potential hidden change-points.
_PERMUTATION_TESTING_ITERATIONS = 199

# This is the threshold percentage that permutation testing must meet for us to
# consider the sub-range that might contain a potential change-point.
_MIN_SIGNIFICANCE = 0.95

# The subsampling length is the maximum length we're letting the permutation
# testing use to find potential rearrangements of the underlying data.
_MAX_SUBSAMPLING_LENGTH = 20

# Extend the change range based on the estimate value in the range of tolerance.
_CHANGE_RANGE_TOLERANCE = 0.90


class Error(Exception):
  pass


class InsufficientData(Error):
  pass


def Cluster(sequence, partition_point):
  """Return a tuple (left, right) where partition_point is part of right."""
  return (sequence[:partition_point], sequence[partition_point:])


def Midpoint(sequence):
  """Return an index in the sequence representing the midpoint."""
  return (len(sequence) - 1) // 2


def ClusterAndCompare(sequence, partition_point):
  """Returns the comparison result and the clusters at the partition point."""
  # Detect a difference between the two clusters
  cluster_a, cluster_b = Cluster(sequence, partition_point)
  if len(cluster_a) > 2 and len(cluster_b) > 2:
    magnitude = float(math_utils.Iqr(cluster_a) + math_utils.Iqr(cluster_b)) / 2
  else:
    magnitude = 1
  return (pinpoint_compare.Compare(cluster_a, cluster_b,
                                   (len(cluster_a) + len(cluster_b)) // 2,
                                   'performance',
                                   magnitude), cluster_a, cluster_b)


def PermutationTest(sequence, change_point, rand=None):
  """Run permutation testing on a sequence.

  Determine whether there's a potential change point within the sequence,
  using randomised permutation testing.

  Arguments:
    - sequence: an iterable of values to perform permutation testing on.
    - change_point: the possible change point calculated by Estimator.
    - rand: an implementation of a pseudo-random generator (see random.Random))

  Returns significance of the change point we are testing.
  """
  if len(sequence) < 3:
    return 0.0

  if rand is None:
    rand = random.Random()

  segment_start = max(change_point - (_MAX_SUBSAMPLING_LENGTH // 2), 0)
  segment_end = min(change_point + (_MAX_SUBSAMPLING_LENGTH // 2),
                    len(sequence))
  segment = list(sequence[segment_start:segment_end])
  change_point_q = Estimator(segment, change_point - segment_start)

  # Significance is defined by how many change points in random permutations
  # are less significant than the one we choose. This works because the change
  # point should be less significant if we mixing the left and right part
  # seperated by the point.
  significance = 0
  for _ in range(_PERMUTATION_TESTING_ITERATIONS):
    rand.shuffle(segment)
    _, q, _ = ChangePointEstimator(segment)
    if q < change_point_q:
      significance += 1

  return float(significance) / (_PERMUTATION_TESTING_ITERATIONS + 1)


def Estimator(sequence, index):
  cluster_a, cluster_b = Cluster(sequence, index)
  if len(cluster_a) == 0 or len(cluster_b) == 0:
    return float('NaN')
  a_array = tuple(
      abs(a - b)**2 for a, b in itertools.combinations(cluster_a, 2))
  if not a_array:
    a_array = (0.,)
  b_array = tuple(
      abs(a - b)**2 for a, b in itertools.combinations(cluster_b, 2))
  if not b_array:
    b_array = (0.,)
  y = sum(abs(a - b)**2 for a, b in itertools.product(cluster_a, cluster_b))
  x_a = sum(a_array)
  x_b = sum(b_array)
  a_len = len(cluster_a)
  b_len = len(cluster_b)
  a_len_combinations = len(a_array)
  b_len_combinations = len(b_array)
  y_scaler = 2.0 / (a_len * b_len)
  a_estimate = (x_a / a_len_combinations)
  b_estimate = (x_b / b_len_combinations)
  e = (y_scaler * y) - a_estimate - b_estimate
  return (e * a_len * b_len) / (a_len + b_len)


def ChangePointEstimator(sequence):
  # This algorithm does the following:
  #   - For each element in the sequence:
  #     - Partition the sequence into two clusters (X[a], X[b])
  #     - Compute the intra-cluster distances squared (X[n])
  #     - Scale the intra-cluster distances by the number of intra-cluster
  #       pairs. (X'[n] = X[n] / combinations(|X[n]|, 2) )
  #     - Compute the inter-cluster distances squared (Y)
  #     - Scale the inter-cluster distances by the number of total pairs
  #       multiplied by 2 (Y' = (Y * 2) / |X[a]||X[b]|)
  #     - Sum up as: Y' - X'[a] - X'[b]
  #   - Return the index of the highest estimator.
  #
  # The computation is based on Euclidean distances between measurements
  # within and across clusters to show the likelihood that the values on
  # either side of a sequence is likely to show a divergence.
  #
  # This algorithm is O(N^2) to the size of the sequence.
  margin = 1
  max_estimate = None
  max_index = 0
  estimates = [
      Estimator(sequence, i)
      for i, _ in enumerate(sequence)
      if margin <= i < len(sequence)
  ]
  if not estimates:
    return (0, 0, False)
  for index, estimate in enumerate(estimates):
    if max_estimate is None or estimate > max_estimate:
      max_estimate = estimate
      max_index = index
  return (max_index + margin, max_estimate, True)


def ExtendChangePointRange(change_point, sequence):
  max_estimate = Estimator(sequence, change_point)

  left, right = 1, len(sequence) - 1

  for index in range(change_point, 0, -1):
    if Estimator(sequence, index) < _CHANGE_RANGE_TOLERANCE * max_estimate:
      left = index + 1
      break

  for index in range(change_point, len(sequence) - 1):
    if Estimator(sequence, index) < _CHANGE_RANGE_TOLERANCE * max_estimate:
      right = index - 1
      break

  return (left, right)


def ClusterAndFindSplit(values, rand=None):
  """Finds a list of indices where we can detect significant changes.

  This algorithm looks for the point at which clusterings of the "left" and
  "right" datapoints show a significant difference. We understand that this
  algorithm is working on potentially already-aggregated data (means, etc.) and
  it would work better if we had access to all the underlying data points, but
  for now we can do our best with the points we have access to.

  In the E-Divisive paper, this is a two-step process: first estimate potential
  change points, then test whether the clusters partitioned by the proposed
  change point internally has potentially hidden change-points through random
  permutation testing. Because the current implementation only returns a single
  change-point, we do the change point estimation through bisection, and use the
  permutation testing to identify whether we should continue the bisection, not
  to find all potential change points.

  Arguments:
    - values: a sequence of values in time series order.
    - rand: a callable which produces a value used for subsequence permutation
      testing.

  Returns:
    - A list of indices into values where we can detect potential split points.

  Raises:
    - InsufficientData when the algorithm cannot find potential change points
      with statistical significance testing.
  """

  logging.debug('Starting change point detection.')
  length = len(values)
  if length <= 3:
    raise InsufficientData(
        'Sequence is not larger than minimum length (%s <= %s)' % (length, 3))
  candidate_indices = set()
  exploration_queue = [(0, length)]
  while exploration_queue:
    # Find the most likely change point in the whole range, only excluding the
    # first and last elements. We're doing this because we want to still be able
    # to pick a candidate within the margins (excluding the ends) if we have
    # enough confidence that it is a change point.
    start, end = exploration_queue.pop(0)
    logging.debug('Exploring range seq[%s:%s]', start, end)
    segment = values[start:end]

    partition_point, _, _ = ChangePointEstimator(segment)
    probability = PermutationTest(segment, partition_point, rand)
    logging.debug(
        'Permutation testing change point %d at seq[%s:%s]: %s;'
        ' probability = %.4f', partition_point, start, end,
        probability >= _MIN_SIGNIFICANCE, probability)
    if probability < _MIN_SIGNIFICANCE:
      continue
    lower, upper = ExtendChangePointRange(partition_point, segment)
    if lower != partition_point or upper != partition_point:
      logging.debug('Extending change range from %d to %d-%d.',
                    partition_point, lower, upper)
    candidate_indices.add(
        (start + partition_point, (start + lower, start + upper)))

    exploration_queue.append((start, start + partition_point))
    exploration_queue.append((start + partition_point, end))

  if not candidate_indices:
    raise InsufficientData('Not enough data to suggest a change point.')
  return list(sorted(candidate_indices))
