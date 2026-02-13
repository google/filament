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

#include "source/util/span.h"

#include "gmock/gmock.h"

namespace spvtools {
namespace utils {
namespace {

using SpanTest = ::testing::Test;

TEST(SpanTest, Initialize_Default) {
  const Span<int> s;

  EXPECT_EQ(s.begin(), nullptr);
  EXPECT_EQ(s.end(), nullptr);
  EXPECT_EQ(s.cbegin(), nullptr);
  EXPECT_EQ(s.cend(), nullptr);

  EXPECT_EQ(s.size(), std::size_t(0));
  EXPECT_EQ(s.size_bytes(), std::size_t(0));
  EXPECT_TRUE(s.empty());

  EXPECT_EQ(s.data(), nullptr);
}

TEST(SpanTest, Initialize_EmptySpan) {
  int ints[3];
  int* first = ints + 1;
  const Span<int> s(first, 0);

  EXPECT_EQ(s.begin(), first);
  EXPECT_EQ(s.end(), first);
  EXPECT_EQ(s.begin(), s.end());
  EXPECT_EQ(s.cbegin(), first);
  EXPECT_EQ(s.cend(), first);
  EXPECT_EQ(s.cbegin(), s.cend());

  EXPECT_EQ(s.size(), std::size_t(0));
  EXPECT_EQ(s.size_bytes(), std::size_t(0));
  EXPECT_TRUE(s.empty());

  EXPECT_EQ(s.data(), first);
}

TEST(SpanTest, Initialize_NonemptySpan) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  EXPECT_EQ(s.begin(), first);
  EXPECT_EQ(s.end(), first + 3);
  EXPECT_NE(s.begin(), s.end());
  EXPECT_EQ(s.cbegin(), first);
  EXPECT_EQ(s.cend(), first + 3);
  EXPECT_NE(s.cbegin(), s.cend());

  EXPECT_EQ(s.size(), std::size_t(3));
  EXPECT_EQ(s.size_bytes(), 3 * sizeof(int));
  EXPECT_FALSE(s.empty());

  EXPECT_EQ(&(s.front()), first);
  EXPECT_EQ(s.front(), 20);
  EXPECT_EQ(&(s.back()), first + 2);
  EXPECT_EQ(s.back(), 40);

  EXPECT_EQ(s.data(), first);
  EXPECT_EQ(&s[0], first);
  EXPECT_EQ(s[0], 20);
  EXPECT_EQ(&s[1], first + 1);
  EXPECT_EQ(s[1], 30);
  EXPECT_EQ(&s[2], first + 2);
  EXPECT_EQ(s[2], 40);
  EXPECT_EQ(&s[3], first + 3);
  EXPECT_EQ(&s[3], s.end());
}

TEST(SpanTest, Initialize_NonemptySpan_Iterator_PostIncrement) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  auto iter = s.begin();
  EXPECT_NE(iter, s.end());
  EXPECT_EQ(*iter++, 20);

  EXPECT_NE(iter, s.end());
  EXPECT_EQ(*iter++, 30);

  EXPECT_NE(iter, s.end());
  EXPECT_EQ(*iter++, 40);

  EXPECT_EQ(iter, s.end());
}

TEST(SpanTest, Initialize_NonemptySpan_Iterator_PreIncrement) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  auto iter = s.begin();
  EXPECT_EQ(*++iter, 30);

  EXPECT_NE(iter, s.end());
  EXPECT_EQ(*++iter, 40);

  EXPECT_EQ(++iter, s.end());
}

TEST(SpanTest, Initialize_NonemptySpan_Iterator_PostDecrement) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  auto iter = s.end();
  EXPECT_EQ(iter--, s.end());
  EXPECT_EQ(*iter--, 40);

  EXPECT_NE(iter, s.begin());
  EXPECT_EQ(*iter--, 30);

  EXPECT_EQ(iter, s.begin());
  EXPECT_EQ(*iter--, 20);

  EXPECT_NE(iter, s.begin());
}

TEST(SpanTest, Initialize_NonemptySpan_Iterator_PreDecrement) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  auto iter = s.end();
  EXPECT_EQ(*--iter, 40);

  EXPECT_NE(iter, s.begin());
  EXPECT_EQ(*--iter, 30);

  EXPECT_NE(iter, s.begin());
  EXPECT_EQ(*--iter, 20);

  EXPECT_EQ(iter, s.begin());
}

TEST(SpanTest, Subspan_FromNil) {
  const Span<int> snil(nullptr, 0);

  const auto s0 = snil.subspan(0);
  const auto s2 = snil.subspan(2);

  EXPECT_EQ(s0.begin(), nullptr);
  EXPECT_EQ(s0.end(), nullptr);
  EXPECT_EQ(s0.size(), 0u);
  EXPECT_TRUE(s0.empty());

  EXPECT_EQ(s2.begin(), nullptr);
  EXPECT_EQ(s2.end(), nullptr);
  EXPECT_EQ(s2.size(), 0u);
  EXPECT_TRUE(s2.empty());
}

TEST(SpanTest, Subspan_FromEmpty) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 0);

  const auto s0 = s.subspan(0);
  const auto s2 = s.subspan(2);

  EXPECT_EQ(s0.begin(), nullptr);
  EXPECT_EQ(s0.end(), nullptr);
  EXPECT_EQ(s0.size(), 0u);
  EXPECT_TRUE(s0.empty());

  EXPECT_EQ(s2.begin(), nullptr);
  EXPECT_EQ(s2.end(), nullptr);
  EXPECT_EQ(s2.size(), 0u);
  EXPECT_TRUE(s2.empty());
}

TEST(SpanTest, Subspan_FromNonEmpty) {
  int ints[10] = {0, 10, 20, 30, 40, 50, 60};
  int* first = ints + 2;
  const Span<int> s(first, 3);

  const auto s0 = s.subspan(0);
  const auto s2 = s.subspan(2);
  const auto s3 = s.subspan(3);
  const auto s4 = s.subspan(4);

  EXPECT_EQ(s0.begin(), s.begin());
  EXPECT_EQ(s0.end(), s.end());
  EXPECT_EQ(s0.size(), 3u);

  EXPECT_EQ(s2.begin(), s.begin() + 2);
  EXPECT_EQ(s2.end(), s.end());
  EXPECT_EQ(s2.size(), 1u);

  EXPECT_EQ(s3.begin(), nullptr);
  EXPECT_EQ(s3.end(), nullptr);
  EXPECT_EQ(s3.size(), 0u);

  EXPECT_EQ(s4.begin(), nullptr);
  EXPECT_EQ(s4.end(), nullptr);
  EXPECT_EQ(s4.size(), 0u);
}

}  // namespace
}  // namespace utils
}  // namespace spvtools
