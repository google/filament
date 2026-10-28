/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BackendTest.h"

using namespace filament;
using namespace filament::backend;

namespace test {

// A backend out of timer queries hands back a null handle. The Vulkan pool is a
// utils::bitset32, so the 33rd outstanding query has nowhere to go; 40 requests is past
// that with room to spare. Before the pool was checked this segfaulted in
// createTimerQueryS rather than losing a timing.
constexpr size_t kBeyondPool = 40;

TEST_F(BackendTest, TimerQueryPoolExhaustion) {
    auto& api = getDriverApi();

    Handle<HwTimerQuery> queries[kBeyondPool];
    for (auto& q : queries) {
        q = api.createTimerQuery();
    }

    // A null handle past the pool's capacity is the contract, not a failure.
    for (auto& q : queries) {
        api.beginTimerQuery(q);
        api.endTimerQuery(q);
    }
    flushAndWait();

    // An unbacked query reports ERROR -- "result won't be available". A live one is
    // NOT_READY or AVAILABLE. Neither may dereference a null handle.
    for (auto& q : queries) {
        uint64_t elapsed = 0;
        TimerQueryResult const r = api.getTimerQueryValue(q, &elapsed);
        if (!q) {
            EXPECT_EQ(r, TimerQueryResult::ERROR);
        }
    }

    for (auto& q : queries) {
        api.destroyTimerQuery(q);
    }
    flushAndWait();
}

} // namespace test
