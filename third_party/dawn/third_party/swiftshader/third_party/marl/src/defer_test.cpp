// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marl/defer.h"

#include "marl_test.h"

TEST_F(WithoutBoundScheduler, Defer) {
  bool deferCalled = false;
  { defer(deferCalled = true); }
  ASSERT_TRUE(deferCalled);
}

TEST_F(WithoutBoundScheduler, DeferOrder) {
  int counter = 0;
  int a = 0, b = 0, c = 0;
  {
    defer(a = ++counter);
    defer(b = ++counter);
    defer(c = ++counter);
  }
  ASSERT_EQ(a, 3);
  ASSERT_EQ(b, 2);
  ASSERT_EQ(c, 1);
}