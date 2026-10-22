// Copyright 2017 Google Inc. All rights reserved.
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

#include "port/gtest.h"

#include "examples/fuzzer_test.h"

using testing::Test;

namespace {

class ExpatExampleTest : public FuzzerTest {};

TEST_F(ExpatExampleTest, Fuzz) {
  EXPECT_EQ(0, RunFuzzer("expat_example", 500, 10000));
  EXPECT_GT(CountFilesInDir(), 50);
}

}  // namespace
