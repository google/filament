// Copyright (c) 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/util/index_range.h"

#include "gmock/gmock.h"

namespace spvtools {
namespace utils {
namespace {

using IndexRangeTest = ::testing::Test;

using ushort = unsigned short;

TEST(IndexRangeTest, Initialize_Default) {
  double sentinel_a = 0.0;
  double sentinel_b = 1.0;
  const IndexRange<double, unsigned, ushort> ir;

  EXPECT_EQ(ir.first(), unsigned(0));
  EXPECT_EQ(ir.count(), ushort(0));
  EXPECT_TRUE(ir.empty());

  auto span_null = ir.apply(nullptr);
  EXPECT_EQ(span_null.data(), nullptr);
  EXPECT_EQ(span_null.size(), 0);
  EXPECT_TRUE(span_null.empty());

  auto span_a = ir.apply(&sentinel_a);
  EXPECT_EQ(span_a.data(), &sentinel_a);
  EXPECT_EQ(span_a.size(), 0);
  EXPECT_TRUE(span_a.empty());

  auto span_b = ir.apply(&sentinel_b);
  EXPECT_EQ(span_b.data(), &sentinel_b);
  EXPECT_EQ(span_b.size(), 0);
  EXPECT_TRUE(span_b.empty());
}

TEST(IndexRangeTest, Initialize_NonEmpty) {
  const IndexRange<double, unsigned, ushort> ir(1, 2);

  EXPECT_EQ(ir.first(), unsigned(1));
  EXPECT_EQ(ir.count(), ushort(2));
  EXPECT_FALSE(ir.empty());

  auto span_null = ir.apply(nullptr);
  EXPECT_EQ(span_null.data(), nullptr);
  EXPECT_EQ(span_null.size(), 0);
  EXPECT_TRUE(span_null.empty());

  double arr[] = {0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0};

  auto span_a = ir.apply(arr);
  EXPECT_EQ(span_a.begin(), arr + 1);
  EXPECT_EQ(span_a.end(), arr + 3);
  EXPECT_FALSE(span_a.empty());

  auto span_b = ir.apply(arr + 3);
  EXPECT_EQ(span_b.begin(), arr + 4);
  EXPECT_EQ(span_b.end(), arr + 6);
  EXPECT_FALSE(span_a.empty());
}

}  // namespace
}  // namespace utils
}  // namespace spvtools
