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

#include "../src/BufferStuffingDetector.h"

#include <filament/Renderer.h>

#include <gtest/gtest.h>

#include <vector>

using namespace filament;

class BufferStuffingDetectorTest : public ::testing::Test {
protected:
    BufferStuffingDetector detector;
    std::vector<Renderer::FrameInfo> history;
    uint32_t currentFrameId = 100;
};

// Verify that if displayPresent < 0 or expectedPresentLatency < 0, it skips that frame.
TEST_F(BufferStuffingDetectorTest, PendingTelemetryRejection) {
    history.resize(1);
    history[0].frameId = 99;
    history[0].displayPresent = -1;
    history[0].expectedPresentLatency = 16666666;
    history[0].displayPresentInterval = 16666666;
    
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));

    history[0].displayPresent = 1000;
    history[0].expectedPresentLatency = -1;
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));
}

// Verify healthy frames do not trigger skips
TEST_F(BufferStuffingDetectorTest, HealthyPacingNoStuffing) {
    history.resize(1);
    auto& info = history[0];
    info.frameId = 99;
    info.vsync = 1000000;
    info.expectedPresentLatency = 16666666;
    info.displayPresentInterval = 16666666;
    info.displayPresent = info.vsync + info.expectedPresentLatency; // Latency is exactly expected

    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));
}

// Verify a delayed frame that just misses by 1 frame does not trigger skip (needs to be >= max allowed + 1 interval)
// maxAllowed = expectedPresentLatency + interval = 33.3ms
// presentLatency = displayPresent - vsync.
// skip if (presentLatency - maxAllowed >= interval) => presentLatency >= expectedPresentLatency + 2 * interval
TEST_F(BufferStuffingDetectorTest, SlightlyDelayedNoStuffing) {
    history.resize(1);
    auto& info = history[0];
    info.frameId = 99;
    info.vsync = 1000000;
    info.expectedPresentLatency = 16666666;
    info.displayPresentInterval = 16666666;
    // Delayed by expected + 1 interval (latency = 33.3ms)
    info.displayPresent = info.vsync + info.expectedPresentLatency + info.displayPresentInterval;

    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));
}

// Verify that when the latency exceeds the threshold, it skips the frame
TEST_F(BufferStuffingDetectorTest, TriggersSkipOnHighLatency) {
    history.resize(1);
    auto& info = history[0];
    info.frameId = 99;
    info.vsync = 1000000;
    info.expectedPresentLatency = 16666666;
    info.displayPresentInterval = 16666666;
    
    // Delayed by expected + 2 intervals (latency = 50ms)
    // presentLatency - maximumLatencyAllowed = 50ms - 33.3ms = 16.6ms >= interval
    info.displayPresent = info.vsync + info.expectedPresentLatency + 2 * info.displayPresentInterval;

    EXPECT_TRUE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));
}

// Verify that older frames prior to mLastFrameId are ignored
TEST_F(BufferStuffingDetectorTest, IgnoresPriorFrames) {
    history.resize(1);
    auto& info = history[0];
    info.frameId = 99;
    info.vsync = 1000000;
    info.expectedPresentLatency = 16666666;
    info.displayPresentInterval = 16666666;
    info.displayPresent = info.vsync + info.expectedPresentLatency + 2 * info.displayPresentInterval; // High latency

    // Set last skipped frame to 99
    detector.setLastFrameId(99);

    // Because frameId (99) - mLastFrameId (99) <= 0, it continues and ignores it.
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));
}
