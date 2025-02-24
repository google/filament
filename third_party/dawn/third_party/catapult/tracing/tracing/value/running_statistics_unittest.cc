// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tracing/tracing/value/running_statistics.h"

#include <limits>

#include "testing/gtest/include/gtest/gtest.h"

namespace catapult {

TEST(RunningStatisticsUnittest, GetsCountRight) {
  RunningStatistics stats;

  stats.Add(1);
  stats.Add(1);
  stats.Add(1);
  stats.Add(1);
  stats.Add(1);
  stats.Add(1);

  EXPECT_EQ(stats.count(), 6);
}

TEST(RunningStatisticsUnittest, ComputesMean) {
  RunningStatistics stats;

  stats.Add(1);
  stats.Add(2);
  stats.Add(3);
  stats.Add(4);

  EXPECT_EQ(stats.mean(), 2.5);
}

TEST(RunningStatisticsUnittest, MeanIsInfiniteIfInfiniteSampleAdded) {
  RunningStatistics stats;

  stats.Add(1);
  stats.Add(std::numeric_limits<float>::infinity());
  stats.Add(2);

  EXPECT_EQ(stats.mean(), std::numeric_limits<float>::infinity());
}

TEST(RunningStatisticsUnittest, ComputesMaxAndMin) {
  RunningStatistics stats;

  stats.Add(4);
  stats.Add(2);
  stats.Add(-18);
  stats.Add(10);

  EXPECT_EQ(stats.min(), -18);
  EXPECT_EQ(stats.max(), 10);
}

TEST(RunningStatisticsUnittest, ComputesMeanLogs) {
  RunningStatistics stats;

  stats.Add(100);
  stats.Add(200);
  stats.Add(300);
  stats.Add(400);

  ASSERT_TRUE(stats.meanlogs_valid());
  ASSERT_FLOAT_EQ(stats.meanlogs(), 5.399684);
}

TEST(RunningStatisticsUnittest, MeanlogsGoInvalidIfNegativeSampleAdded) {
  RunningStatistics stats;

  stats.Add(-1);

  ASSERT_FALSE(stats.meanlogs_valid());
}

TEST(RunningStatisticsUnittest, ComputesVariance) {
  RunningStatistics stats;

  stats.Add(0);
  stats.Add(1);
  stats.Add(2);
  stats.Add(3);

  ASSERT_FLOAT_EQ(stats.variance(), 1.6666666);
}

TEST(RunningStatisticsUnittest, VarianceIsZeroForOneSample) {
  RunningStatistics stats;

  stats.Add(17);

  EXPECT_EQ(stats.mean(), 17);
  EXPECT_EQ(stats.variance(), 0);
}

}  // namespace catapult
