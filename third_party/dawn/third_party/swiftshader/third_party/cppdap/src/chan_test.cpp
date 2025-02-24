// Copyright 2019 Google LLC
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

#include "chan.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <thread>

TEST(ChanTest, PutTakeClose) {
  dap::Chan<int> chan;
  auto thread = std::thread([&] {
    chan.put(10);
    chan.put(20);
    chan.put(30);
    chan.close();
  });
  EXPECT_EQ(chan.take(), dap::optional<int>(10));
  EXPECT_EQ(chan.take(), dap::optional<int>(20));
  EXPECT_EQ(chan.take(), dap::optional<int>(30));
  EXPECT_EQ(chan.take(), dap::optional<int>());
  thread.join();
}
