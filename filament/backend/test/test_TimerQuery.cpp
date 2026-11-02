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

// A backend out of timer queries may hand back a null handle. The Vulkan backend used to cap
// outstanding queries at 32 and segfaulted in createTimerQueryS on the 33rd, so 40 requests is
// past that old limit with room to spare. Vulkan no longer has a cap, but any backend that does
// must hand back a null handle rather than crash.
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

// Exercises repeated timer query reuse across completed frames (which uses host-side reset via
// VK_EXT_host_query_reset when supported) and verifies that beginning a new measurement clears
// readiness until the new command buffer completes.
TEST_F(BackendTest, TimerQueryReuseAcrossFrames) {
    auto& api = getDriverApi();
    if (!api.isFrameTimeSupported()) {
        GTEST_SKIP() << "Timer queries are not supported on this backend.";
    }

    auto swapChain = addCleanup(createSwapChain());
    auto defaultRt = addCleanup(api.createDefaultRenderTarget());
    Handle<HwTimerQuery> q = api.createTimerQuery();
    ASSERT_TRUE(bool(q));

    uint64_t elapsed = 0;
    EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

    constexpr uint32_t kFrameCount = 6;
    for (uint32_t frame = 1; frame <= kFrameCount; ++frame) {
        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, frame);
        api.beginTimerQuery(q);

        if (matchesEnvironment(Backend::VULKAN)) {
            // Once beginTimerQuery executes on the backend, a previously completed result must no
            // longer be reported while the new measurement is still recording.
            executeCommands();
            EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);
        }

        RenderPassParams params = getClearColorDepthRenderPass();
        params.viewport = getFullViewport();
        api.beginRenderPass(defaultRt, params);
        api.endRenderPass();

        api.endTimerQuery(q);
        api.commit(swapChain);
        api.endFrame(frame);

        api.finish();
        api.tick();
        executeCommands();
        getDriver().purge();

        TimerQueryResult const r = api.getTimerQueryValue(q, &elapsed);
        if (matchesEnvironment(Backend::VULKAN)) {
            // Completed Vulkan queries must have availability bits set; drivers with coarse
            // timestamps (begin == end) report ERROR rather than NOT_READY.
            EXPECT_NE(r, TimerQueryResult::NOT_READY);
            // Querying a second time before the next beginTimerQuery must remain idempotent.
            uint64_t elapsedAgain = 0;
            EXPECT_EQ(api.getTimerQueryValue(q, &elapsedAgain), r);
        }
    }

    api.destroyTimerQuery(q);
    flushAndWait();
}

// Exercises back-to-back query reuse and destroy/recreate slot reuse before GPU work completes
// (on Vulkan, each begin binds a fresh pair of queries while the previous pair waits for its
// command buffers to complete) followed by post-completion reuse (covering the reclamation of
// retired pairs, with host-side vkResetQueryPoolEXT when supported).
TEST_F(BackendTest, TimerQueryInFlightAndSlotReuse) {
    auto& api = getDriverApi();
    if (!api.isFrameTimeSupported()) {
        GTEST_SKIP() << "Timer queries are not supported on this backend.";
    }

    Handle<HwTimerQuery> q1 = api.createTimerQuery();
    ASSERT_TRUE(bool(q1));

    // Reuse the same query handle twice in the same command buffer, and again across a flush
    // without waiting for GPU completion.
    api.beginTimerQuery(q1);
    api.endTimerQuery(q1);
    api.beginTimerQuery(q1);
    api.endTimerQuery(q1);
    api.flush();
    api.beginTimerQuery(q1);
    api.endTimerQuery(q1);

    // Destroy q1 while its command buffers are still in flight, then allocate q2 (which reclaims
    // the freed pool slot) and record another query before waiting.
    api.destroyTimerQuery(q1);
    executeCommands();

    Handle<HwTimerQuery> q2 = api.createTimerQuery();
    ASSERT_TRUE(bool(q2));
    uint64_t elapsed = 0;
    EXPECT_EQ(api.getTimerQueryValue(q2, &elapsed), TimerQueryResult::NOT_READY);

    api.beginTimerQuery(q2);
    api.endTimerQuery(q2);
    flushAndWait();

    // Now that the slot is idle on the GPU, reuse q2 once more to exercise host-side reset.
    api.beginTimerQuery(q2);
    api.endTimerQuery(q2);
    flushAndWait();

    if (matchesEnvironment(Backend::VULKAN)) {
        EXPECT_NE(api.getTimerQueryValue(q2, &elapsed), TimerQueryResult::NOT_READY);
    }

    api.destroyTimerQuery(q2);
    flushAndWait();
}

// Mirrors FRenderer::endFrame, where SwapChain::commit runs before endTimerQuery and splits the
// starting and stopping timestamps across two command buffers without a semaphore link between
// them on non-headless swapchains.
TEST_F(BackendTest, TimerQuerySplitAcrossCommit) {
    auto& api = getDriverApi();
    if (!api.isFrameTimeSupported()) {
        GTEST_SKIP() << "Timer queries are not supported on this backend.";
    }

    auto swapChain = addCleanup(createSwapChain());
    auto defaultRt = addCleanup(api.createDefaultRenderTarget());
    Handle<HwTimerQuery> const q = api.createTimerQuery();
    ASSERT_TRUE(bool(q));

    uint64_t elapsed = 0;
    constexpr uint32_t kFrameCount = 4;
    for (uint32_t frame = 1; frame <= kFrameCount; ++frame) {
        api.makeCurrent(swapChain, swapChain);
        api.beginFrame(0, 0, frame);
        api.beginTimerQuery(q);

        RenderPassParams params = getClearColorDepthRenderPass();
        params.viewport = getFullViewport();
        api.beginRenderPass(defaultRt, params);
        api.endRenderPass();

        // Commit flushes the command buffer containing beginTimerQuery before endTimerQuery is
        // recorded. Even after that first command buffer completes, the query must stay NOT_READY.
        api.commit(swapChain);
        flushAndWait();
        EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

        api.endTimerQuery(q);
        api.endFrame(frame);

        api.finish();
        api.tick();
        executeCommands();
        getDriver().purge();

        if (matchesEnvironment(Backend::VULKAN)) {
            EXPECT_NE(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);
        }
    }

    api.destroyTimerQuery(q);
    flushAndWait();
}

TEST_F(BackendTest, TimerQueryUnpairedEnd) {
    if (!matchesEnvironment(Backend::VULKAN)) {
        GTEST_SKIP() << "Vulkan-specific unpaired endTimerQuery validation test.";
    }
    auto& api = getDriverApi();

    Handle<HwTimerQuery> const q = api.createTimerQuery();
    ASSERT_TRUE(bool(q));

    uint64_t elapsed = 0;
    // An endTimerQuery with no preceding beginTimerQuery must be ignored rather than marking an
    // unwritten starting query as completed.
    api.endTimerQuery(q);
    flushAndWait();
    EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

    // A duplicate endTimerQuery after a valid pair must not write to an un-reset query slot.
    api.beginTimerQuery(q);
    api.endTimerQuery(q);
    api.endTimerQuery(q);
    flushAndWait();
    EXPECT_NE(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

    api.destroyTimerQuery(q);
    flushAndWait();
}

// On Vulkan, re-beginning a timer retires its previous pair of queries until the command buffer
// that wrote to it completes. Re-beginning a timer more often than the pool can absorb within a
// single unflushed command buffer must report ERROR rather than reuse a pair that is still in
// flight, and the timer must recover once the GPU catches up.
TEST_F(BackendTest, TimerQueryPairExhaustionRecovers) {
    if (!matchesEnvironment(Backend::VULKAN)) {
        GTEST_SKIP() << "Vulkan-specific query pair exhaustion test.";
    }
    auto& api = getDriverApi();

    Handle<HwTimerQuery> const q = api.createTimerQuery();
    ASSERT_TRUE(bool(q));

    // The Vulkan backend holds 96 pairs, and none of the pairs retired here can be reclaimed
    // because they were all written by the command buffer that is still being recorded.
    constexpr size_t kBeyondPairs = 128;
    for (size_t i = 0; i < kBeyondPairs; ++i) {
        api.beginTimerQuery(q);
        api.endTimerQuery(q);
    }
    executeCommands();

    uint64_t elapsed = 0;
    EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::ERROR);

    // Completing the command buffer allows the retired pairs to be reclaimed on the next begin.
    flushAndWait();
    api.beginTimerQuery(q);
    executeCommands();
    EXPECT_EQ(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

    api.endTimerQuery(q);
    flushAndWait();
    EXPECT_NE(api.getTimerQueryValue(q, &elapsed), TimerQueryResult::NOT_READY);

    api.destroyTimerQuery(q);
    flushAndWait();
}

} // namespace test
