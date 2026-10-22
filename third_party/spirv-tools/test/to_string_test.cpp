// Copyright (c) 2024 Google LLC
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

#include "source/to_string.h"

#include "gmock/gmock.h"

namespace {

TEST(ToString, Uint32) {
  EXPECT_EQ(spvtools::to_string(0u), "0");
  EXPECT_EQ(spvtools::to_string(1u), "1");
  EXPECT_EQ(spvtools::to_string(1234567890u), "1234567890");
  EXPECT_EQ(spvtools::to_string(0xffffffffu), "4294967295");
}

}  // namespace
