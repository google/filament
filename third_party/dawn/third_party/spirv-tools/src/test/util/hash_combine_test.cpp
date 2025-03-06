// Copyright (c) 2022 The Khronos Group Inc.
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

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "source/util/hash_combine.h"

namespace spvtools {
namespace utils {
namespace {

using HashCombineTest = ::testing::Test;

TEST(HashCombineTest, Identity) { EXPECT_EQ(hash_combine(0), 0); }

TEST(HashCombineTest, Variadic) {
  // Expect manual and variadic template versions be the same.
  EXPECT_EQ(hash_combine(hash_combine(hash_combine(0, 1), 2), 3),
            hash_combine(0, 1, 2, 3));
}

TEST(HashCombineTest, Vector) {
  // Expect variadic and vector versions be the same.
  EXPECT_EQ(hash_combine(0, std::vector<uint32_t>({1, 2, 3})),
            hash_combine(0, 1, 2, 3));
}

}  // namespace
}  // namespace utils
}  // namespace spvtools
