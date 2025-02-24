// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tracing/tracing/value/running_statistics.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace catapult {

void RunningStatistics::Add(double value) {
  count_++;

  max_ = std::max(max_, value);
  min_ = std::min(min_, value);
  sum_ += value;

  if (std::islessequal(value, 0.0)) {
    meanlogs_valid_ = false;
  } else if (meanlogs_valid_) {
    meanlogs_ += (std::log(std::abs(value)) - meanlogs_) / count_;
  }

  // The following uses Welford's algorithm for computing running mean and
  // variance. See http://www.johndcook.com/blog/standard_deviation.
  if (count_ == 1) {
    mean_ = value;
    variance_ = 0.0;
  } else {
    double old_mean = mean_;
    double old_variance = variance_;

    // Using the 2nd formula for updating the mean yields better precision but
    // it doesn't work for the case old_mean is Infinity. Hence we handle that
    // case separately.
    if (std::isinf(old_mean)) {
      mean_ = sum_ / count_;
    } else {
      mean_ = (old_mean + (value - old_mean) / count_);
    }

    variance_ = old_variance + ((value - old_mean) * (value - mean_));
  }
}

double RunningStatistics::meanlogs() const {
  assert(meanlogs_valid_);
  return meanlogs_;
}

double RunningStatistics::variance() const {
  if (count() == 0 || count() == 1) {
    return 0;
  }

  // This returns the variance of the samples after Bessel's correction has
  // been applied.
  return variance_ / (count() - 1);
}

}  // namespace catapult
