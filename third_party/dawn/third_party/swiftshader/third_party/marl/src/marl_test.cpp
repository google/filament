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

#include "marl_test.h"

INSTANTIATE_TEST_SUITE_P(
    SchedulerParams,
    WithBoundScheduler,
    testing::Values(SchedulerParams{0},  // Single-threaded mode test
                    SchedulerParams{1},  // Single worker thread
                    SchedulerParams{2},  // 2 worker threads...
                    SchedulerParams{4},
                    SchedulerParams{8},
                    SchedulerParams{64}));

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
