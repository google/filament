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

#include "details/Engine.h"
#include "details/FramePacer.h"

#include <filament/Engine.h>
#include <filament/FramePacer.h>
#include <filament/FramePipelineEstimator.h>
#include <filament/Renderer.h>

#include <gtest/gtest.h>

#include <chrono>
#include <vector>

using namespace filament;

class FramePacerTest : public ::testing::Test {
protected:
    FramePacerTest() {
        mEngine = Engine::create(Engine::Backend::NOOP);
    }

    void TearDown() override {
        Engine::destroy(&mEngine);
    }

    Engine* mEngine = nullptr;
    using Duration = FramePacer::duration_t;
    using TimePoint = FramePacer::time_point_t;
};

// Test Suite 1: Verify default build configuration and baseline pacing
TEST_F(FramePacerTest, DefaultConfiguration) {
    FramePacer* framePacer = FramePacer::Builder().build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    // Simulate a 60Hz vsync tick
    FramePacer::VsyncTick tick;
    tick.baseTime = TimePoint(std::chrono::steady_clock::now());
    tick.vsyncPeriod = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // First frame cycle should be approved for rendering
    EXPECT_EQ(framePacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);

    // Expected presentation time should be baseTime + (16.666ms * 2 latencyFrames)
    TimePoint expectedPresent = tick.baseTime + (tick.vsyncPeriod * 2);
    // Allow slight floating point conversion variance
    Duration diff = std::chrono::abs(framePacer->getExpectedPresentationTime() - expectedPresent);

    EXPECT_LT(diff, std::chrono::nanoseconds(1000)); // Within 1 microsecond

    mEngine->destroy(framePacer);
}

// Test Suite 2: Verify custom target frame rate and CPU gating (e.g. 30 FPS pacing on 60Hz hardware)
TEST_F(FramePacerTest, TargetFrameRatePacing) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(30.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Frame 0 at T=0
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    // Frame 1 at T=16.66ms (next 60Hz tick). Because we target 30 FPS (33.33ms step), this is too early!
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + period60Hz;
    tick1.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick1), FramePacer::FrameStatus::SKIPPED_SPURIOUS); // Correctly skipped!

    // Frame 2 at T=33.33ms. This meets our 30 FPS gate threshold!
    FramePacer::VsyncTick tick2;
    tick2.baseTime = start + (period60Hz * 2);
    tick2.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick2), FramePacer::FrameStatus::ACCEPTED); // Approved!

    mEngine->destroy(framePacer);
}

// Test Suite 3: Verify hardware candidate timeline closest matching
TEST_F(FramePacerTest, TimelineMatching) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Our projected presentation is start + (16.666ms * 2) = start + 33.333ms.
    // Simulate candidate timelines returned from native platform choreographer:
    std::vector<FramePacer::HardwareTimeline> candidates = {
        { start + std::chrono::milliseconds(16), start + std::chrono::milliseconds(12) },
        { start + std::chrono::milliseconds(34), start + std::chrono::milliseconds(28) }, // Closest to 33.333ms!
        { start + std::chrono::milliseconds(50), start + std::chrono::milliseconds(45) }
    };

    FramePacer::VsyncTick tick;
    tick.baseTime = start;
    tick.vsyncPeriod = period60Hz;
    tick.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());

    EXPECT_EQ(framePacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);

    // The frame pacer must pick exactly Timeline 1 (start + 34ms)
    EXPECT_EQ(framePacer->getExpectedPresentationTime(), candidates[1].expectedPresentationTime);

    // Verify applyPresentationTime successfully applies to a Renderer
    Renderer* renderer = mEngine->createRenderer();
    ASSERT_NE(renderer, nullptr);
    framePacer->applyPresentationTime(renderer);

    mEngine->destroy(renderer);
    mEngine->destroy(framePacer);
}

TEST_F(FramePacerTest, TimelineMatching_FilterOutPastDeadlines) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Projected presentation is start + 33.3ms.
    // Candidates:
    // Timeline 0: expected 33.3ms, deadline 25ms (in the past relative to schedule time of start + 29ms)
    // Timeline 1: expected 50ms, deadline 41ms   (in the future!)
    // Timeline 2: expected 66.6ms, deadline 58ms
    std::vector<FramePacer::HardwareTimeline> candidates = {
        { start + std::chrono::milliseconds(33), start + std::chrono::milliseconds(25) },
        { start + std::chrono::milliseconds(50), start + std::chrono::milliseconds(41) },
        { start + std::chrono::milliseconds(66), start + std::chrono::milliseconds(58) }
    };

    FramePacer::VsyncTick tick;
    tick.baseTime = start;
    tick.vsyncPeriod = period60Hz;
    // Simulate we woke up 29ms after vsync, meaning the 25ms deadline of Timeline 0 has passed!
    tick.frameScheduleTime = start + std::chrono::milliseconds(29);
    tick.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());

    EXPECT_EQ(framePacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);

    // The frame pacer must reject Timeline 0 (even though it is closest to 33.3ms) because its deadline (25ms) has passed.
    // It must pick Timeline 1 (start + 50ms) which has a deadline in the future (41ms > 29ms).
    EXPECT_EQ(framePacer->getExpectedPresentationTime(), candidates[1].expectedPresentationTime);

    mEngine->destroy(framePacer);
}

// Test Suite 4: Verify dynamic hardware display refresh rate changes (e.g. 60Hz switching to 120Hz)
TEST_F(FramePacerTest, DynamicDisplayRefreshRateChange) {
    // A targetFrameRate of 0.0f means Track native hardware refresh
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(0.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));

    // Cycle 1: Hardware is running at 60Hz
    FramePacer::VsyncTick tick60;
    tick60.baseTime = start;
    tick60.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick60), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expected60 = start + (period60Hz * 2);
    Duration diff60 = std::chrono::abs(framePacer->getExpectedPresentationTime() - expected60);
    EXPECT_LT(diff60, std::chrono::nanoseconds(1000));

    // Cycle 2: Display dynamically switches to 120Hz!
    TimePoint nextStart = start + period60Hz;
    FramePacer::VsyncTick tick120;
    tick120.baseTime = nextStart;
    tick120.vsyncPeriod = period120Hz; // Active period becomes 8.33ms
    EXPECT_EQ(framePacer->setupFrame(tick120), FramePacer::FrameStatus::ACCEPTED);

    // Expected presentation automatically tracks the new 120Hz period (8.33ms * 4)
    TimePoint expected120 = nextStart + (period120Hz * 4);
    Duration diff120 = std::chrono::abs(framePacer->getExpectedPresentationTime() - expected120);
    EXPECT_LT(diff120, std::chrono::nanoseconds(1000));

    mEngine->destroy(framePacer);
}

// Test Suite 5: Verify custom pipeline depth / latencyFrames (Single Buffering vs Triple Buffering)
TEST_F(FramePacerTest, CustomLatencyFrames) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Single Buffering (latencyFrames = 1)
    FramePacer* pacerSingle = FramePacer::Builder()
            .latencyFrames(1)
            .build(*mEngine);
    ASSERT_NE(pacerSingle, nullptr);

    FramePacer::VsyncTick tickSingle;
    tickSingle.baseTime = start;
    tickSingle.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacerSingle->setupFrame(tickSingle), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expectedSingle = start + period60Hz;
    EXPECT_LT(std::chrono::abs(pacerSingle->getExpectedPresentationTime() - expectedSingle), std::chrono::nanoseconds(1000));
    mEngine->destroy(pacerSingle);

    // Triple Buffering (latencyFrames = 3)
    FramePacer* pacerTriple = FramePacer::Builder()
            .latencyFrames(3)
            .build(*mEngine);
    ASSERT_NE(pacerTriple, nullptr);

    FramePacer::VsyncTick tickTriple;
    tickTriple.baseTime = start;
    tickTriple.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacerTriple->setupFrame(tickTriple), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expectedTriple = start + (period60Hz * 3);
    EXPECT_LT(std::chrono::abs(pacerTriple->getExpectedPresentationTime() - expectedTriple), std::chrono::nanoseconds(1000));
    mEngine->destroy(pacerTriple);
}

// Test Suite 6: Verify complex combinations (Triple buffered 30 FPS pacing on 120Hz display with Candidate Timelines)
TEST_F(FramePacerTest, ComplexHardwareCombinations) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(30.0f)
            .latency(std::chrono::milliseconds(100))
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));

    // For 30 FPS target (33.333ms step) and latencyFrames(3), our projected deadline is +100ms.
    // Simulate candidate timelines returned from display hardware:
    std::vector<FramePacer::HardwareTimeline> candidates = {
        { start + period120Hz, start + period120Hz - std::chrono::milliseconds(5) }, // Realistic timeline 0
        { start + std::chrono::milliseconds(50), start + std::chrono::milliseconds(45) },
        { start + std::chrono::milliseconds(101), start + std::chrono::milliseconds(95) }, // Closest match to 100ms!
        { start + std::chrono::milliseconds(150), start + std::chrono::milliseconds(142) }
    };

    FramePacer::VsyncTick tick;
    tick.baseTime = start;
    tick.vsyncPeriod = period120Hz;
    tick.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());

    EXPECT_EQ(framePacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(framePacer->getExpectedPresentationTime(), candidates[2].expectedPresentationTime);

    mEngine->destroy(framePacer);
}

// Test Suite 7: Verify robust anomaly rejection recovery during a massive CPU or OS stall
TEST_F(FramePacerTest, AnomalyRejectionRecovery) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Frame 0 cycles normally
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    // Frame 1 arrives after a massive 500ms OS stall! Anomaly rejection must kick in and approve it instantly.
    TimePoint stallTime = start + std::chrono::milliseconds(500);
    FramePacer::VsyncTick tickStall;
    tickStall.baseTime = stallTime;
    tickStall.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tickStall), FramePacer::FrameStatus::ACCEPTED); // Recovered and approved!

    TimePoint expectedAfterStall = stallTime + (period60Hz * 2);
    EXPECT_LT(std::chrono::abs(framePacer->getExpectedPresentationTime() - expectedAfterStall), std::chrono::nanoseconds(1000));

    mEngine->destroy(framePacer);
}

// Test Suite 8: Verify runtime mid-flight dynamic reconfiguration via configure(...)
TEST_F(FramePacerTest, RuntimeMidFlightReconfiguration) {
    FramePacer* framePacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(framePacer, nullptr);

    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Cycle 1 at 60 FPS, 2 latency frames
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start;
    tick1.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);
    TimePoint expected60 = start + (period60Hz * 2);
    EXPECT_LT(std::chrono::abs(framePacer->getExpectedPresentationTime() - expected60), std::chrono::nanoseconds(1000));

    // Live Reconfigure: App switches to 30 FPS power-saving mode, 1 latency frame
    FramePacer::Configuration newConfig;
    newConfig.targetFrameRate = 30.0f;
    newConfig.latency         = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / 30.0));
    framePacer->configure(newConfig);

    // Cycle 2: Execute next cycle and verify it adapts to the new 30 FPS pacing target and 1 latency frame depth
    Duration period30Fps = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 30.0f));
    TimePoint start2 = start + std::chrono::milliseconds(200);
    FramePacer::VsyncTick tick2;
    tick2.baseTime = start2;
    tick2.vsyncPeriod = period60Hz;
    EXPECT_EQ(framePacer->setupFrame(tick2), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expected30 = start2 + (period30Fps * 1);
    EXPECT_LT(std::chrono::abs(framePacer->getExpectedPresentationTime() - expected30), std::chrono::nanoseconds(1000));

    mEngine->destroy(framePacer);
}

// Test Suite 9: Verify robust pacing when requested frame rate is not an achievable integer fraction (e.g. 45 FPS)
TEST_F(FramePacerTest, NonIntegerRatioPacing) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz  = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));
    Duration step45Fps   = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 45.0f));

    // Case 1: 60Hz Display, 45 FPS Requested
    // 60 / 45 = 4 / 3 -> Out of 4 Vsyncs, we must render exactly 3 frames and skip 1 Vsync!
    FramePacer* pacer60 = FramePacer::Builder()
            .targetFrameRate(45.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(step45Fps * 2))
            .build(*mEngine);
    ASSERT_NE(pacer60, nullptr);

    // Vsync 0 (0.0ms) -> Approved for ideal frame 0 (0.0ms)
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer60->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_LT(std::chrono::abs(pacer60->getExpectedPresentationTime() - (start + step45Fps * 2)), std::chrono::nanoseconds(1000));

    // Vsync 1 (16.66ms) -> Approved for ideal frame 1 (22.22ms)
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + period60Hz;
    tick1.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer60->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_LT(std::chrono::abs(pacer60->getExpectedPresentationTime() - (start + step45Fps * 3)), std::chrono::nanoseconds(1000));

    // Vsync 2 (33.33ms) -> Too far from ideal frame 2 (44.44ms), as Vsync 3 (50ms) is closer! Must be skipped!
    FramePacer::VsyncTick tick2;
    tick2.baseTime = start + (period60Hz * 2);
    tick2.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer60->setupFrame(tick2), FramePacer::FrameStatus::SKIPPED_SPURIOUS); // Extraneous Choreographer call correctly skipped!

    // Vsync 3 (50.00ms) -> Approved for ideal frame 2 (44.44ms)
    FramePacer::VsyncTick tick3;
    tick3.baseTime = start + (period60Hz * 3);
    tick3.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer60->setupFrame(tick3), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_LT(std::chrono::abs(pacer60->getExpectedPresentationTime() - (start + step45Fps * 4)), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer60);

    // Case 2: 120Hz Display, 45 FPS Requested
    // Proves we achieve significantly tighter presentation alignment on higher refresh hardware!
    FramePacer* pacer120 = FramePacer::Builder()
            .targetFrameRate(45.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(step45Fps * 2))
            .build(*mEngine);
    ASSERT_NE(pacer120, nullptr);

    // Vsync 0 (0.0ms) -> Approved for ideal frame 0 (0.0ms)
    FramePacer::VsyncTick tick120_0;
    tick120_0.baseTime = start;
    tick120_0.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer120->setupFrame(tick120_0), FramePacer::FrameStatus::ACCEPTED);

    // Vsync 1 (8.33ms) and Vsync 2 (16.66ms) -> Extraneous calls correctly skipped!
    FramePacer::VsyncTick tick120_1;
    tick120_1.baseTime = start + period120Hz;
    tick120_1.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer120->setupFrame(tick120_1), FramePacer::FrameStatus::SKIPPED_SPURIOUS);

    FramePacer::VsyncTick tick120_2;
    tick120_2.baseTime = start + (period120Hz * 2);
    tick120_2.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer120->setupFrame(tick120_2), FramePacer::FrameStatus::SKIPPED_SPURIOUS);

    // Vsync 3 (25.00ms) -> Approved for ideal frame 1 (22.22ms). Notice 25ms is much closer to 22.22ms than 16.66ms was!
    FramePacer::VsyncTick tick120_3;
    tick120_3.baseTime = start + (period120Hz * 3);
    tick120_3.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer120->setupFrame(tick120_3), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_LT(std::chrono::abs(pacer120->getExpectedPresentationTime() - (start + step45Fps * 3)), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer120);
}

// Test Suite 10: Verify 3% fuzzy cadence synchronization and rate inspection getters
TEST_F(FramePacerTest, FuzzyCadenceSynchronization) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    // Simulate standard NTSC broadcast display running at exactly 59.94 Hz (16.683333 ms period)
    Duration period59_94Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 59.94f));

    // Case 1: Application requests exactly 60.0 FPS on a 59.94 Hz display
    FramePacer* pacer60 = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer60, nullptr);

    FramePacer::VsyncTick tick60;
    tick60.baseTime = start;
    tick60.vsyncPeriod = period59_94Hz;
    EXPECT_EQ(pacer60->setupFrame(tick60), FramePacer::FrameStatus::ACCEPTED);

    // Must perfectly snap to the 59.94 Hz hardware refresh rate
    EXPECT_NEAR(pacer60->getSelectedFrameRate(), 59.94f, 0.05f);
    EXPECT_TRUE(pacer60->isExactFrameRateAchieved());
    mEngine->destroy(pacer60);

    // Case 2: Application requests exactly 30.0 FPS on a 59.94 Hz display
    FramePacer* pacer30 = FramePacer::Builder()
            .targetFrameRate(30.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer30, nullptr);

    FramePacer::VsyncTick tick30;
    tick30.baseTime = start;
    tick30.vsyncPeriod = period59_94Hz;
    EXPECT_EQ(pacer30->setupFrame(tick30), FramePacer::FrameStatus::ACCEPTED);

    // Must perfectly snap to exactly 1/2 of the hardware refresh rate (29.97 FPS)
    EXPECT_NEAR(pacer30->getSelectedFrameRate(), 29.97f, 0.05f);
    EXPECT_TRUE(pacer30->isExactFrameRateAchieved());
    mEngine->destroy(pacer30);

    // Case 3: Application requests 45.0 FPS on a 59.94 Hz display (Non-integer ratio)
    FramePacer* pacer45 = FramePacer::Builder()
            .targetFrameRate(45.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer45, nullptr);

    FramePacer::VsyncTick tick45;
    tick45.baseTime = start;
    tick45.vsyncPeriod = period59_94Hz;
    EXPECT_EQ(pacer45->setupFrame(tick45), FramePacer::FrameStatus::ACCEPTED);

    // Must not snap! Must remain exactly 45.0 FPS and report exact not achieved
    EXPECT_NEAR(pacer45->getSelectedFrameRate(), 45.0f, 0.05f);
    EXPECT_FALSE(pacer45->isExactFrameRateAchieved());
    mEngine->destroy(pacer45);
}

// Test Suite 11: Verify capping when requested frame rate exceeds display hardware capability (e.g. 90 FPS on 60Hz)
TEST_F(FramePacerTest, HardwareCapabilityCapping) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Application requests exactly 90.0 FPS, but physical hardware can only achieve 60 Hz
    FramePacer* pacer90 = FramePacer::Builder()
            .targetFrameRate(90.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer90, nullptr);

    FramePacer::VsyncTick tick90;
    tick90.baseTime = start;
    tick90.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer90->setupFrame(tick90), FramePacer::FrameStatus::ACCEPTED);

    // Must perfectly clamp the active target rate to the display's maximum capability (60 FPS)
    EXPECT_NEAR(pacer90->getSelectedFrameRate(), 60.0f, 0.05f);
    EXPECT_TRUE(pacer90->isExactFrameRateAchieved());
    mEngine->destroy(pacer90);
}

// Test Suite 12: Verify robust tracking of out-of-phase asynchronous display refresh rate mode switches
TEST_F(FramePacerTest, AsynchronousDisplayModeSwitch) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz  = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));

    // 1. App requests 120 FPS
    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(120.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(period120Hz * 2))
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // 2. At the time Choreographer is called, display is still physically at 60 Hz.
    // Pacer must successfully cap to 60 FPS.
    FramePacer::VsyncTick tick60;
    tick60.baseTime = start;
    tick60.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tick60), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_NEAR(pacer->getSelectedFrameRate(), 60.0f, 0.05f);

    // 3. At some time later, display asynchronously switches to 120 Hz!
    // This switch happens at an out-of-phase timestamp (e.g. +22.5 ms, not falling on a 60Hz multiple).
    TimePoint outOfPhaseSwitch = start + std::chrono::microseconds(22500);

    FramePacer::VsyncTick tick120;
    tick120.baseTime = outOfPhaseSwitch;
    tick120.vsyncPeriod = period120Hz; // Active refresh period becomes 120Hz
    EXPECT_EQ(pacer->setupFrame(tick120), FramePacer::FrameStatus::ACCEPTED); // Must successfully approve and re-anchor!

    // 4. Verify it adapts instantly to the new 120 FPS requested pacing target and computes expected deadlines precisely
    EXPECT_NEAR(pacer->getSelectedFrameRate(), 120.0f, 0.05f);
    TimePoint expected120 = outOfPhaseSwitch + (period120Hz * 2);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected120), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 13: Exhaustively verify FramePacer on generic platforms/devices that do not support hardware timelines at all
TEST_F(FramePacerTest, GenericPlatformNoTimelines) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration step30Fps  = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 30.0f));

    // Sub-test 1: 60Hz Baseline Pacing (No Timelines)
    FramePacer* pacer60 = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer60, nullptr);

    FramePacer::VsyncTick tick0;
    tick0.baseTime    = start;
    tick0.vsyncPeriod = period60Hz;
    // Explicitly empty timelines (simulating generic desktop, WebGL, iOS, or older Android API 26-32)
    tick0.timelines   = {};

    EXPECT_EQ(pacer60->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_NEAR(pacer60->getSelectedFrameRate(), 60.0f, 0.05f);
    // When timelines are empty, expected presentation must be perfectly projected from ideal clock + latency
    TimePoint expected60 = start + (period60Hz * 2);
    EXPECT_LT(std::chrono::abs(pacer60->getExpectedPresentationTime() - expected60), std::chrono::nanoseconds(1000));
    mEngine->destroy(pacer60);

    // Sub-test 2: 30 FPS Custom CPU Skipping (No Timelines)
    FramePacer* pacer30 = FramePacer::Builder()
            .targetFrameRate(30.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(step30Fps * 2))
            .build(*mEngine);
    ASSERT_NE(pacer30, nullptr);

    // Cycle 1: Frame 0 approved
    FramePacer::VsyncTick tick30_0;
    tick30_0.baseTime    = start;
    tick30_0.vsyncPeriod = period60Hz;
    tick30_0.timelines   = {};
    EXPECT_EQ(pacer30->setupFrame(tick30_0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_NEAR(pacer30->getSelectedFrameRate(), 30.0f, 0.05f);

    // Cycle 2: Vsync arrives +16.66ms later -> Must be correctly skipped to throttle CPU thread
    FramePacer::VsyncTick tick30_1;
    tick30_1.baseTime    = start + period60Hz;
    tick30_1.vsyncPeriod = period60Hz;
    tick30_1.timelines   = {};
    EXPECT_EQ(pacer30->setupFrame(tick30_1), FramePacer::FrameStatus::SKIPPED_SPURIOUS);

    // Cycle 3: Vsync arrives +33.33ms later -> Approved for rendering ideal frame 1!
    FramePacer::VsyncTick tick30_2;
    tick30_2.baseTime    = start + (period60Hz * 2);
    tick30_2.vsyncPeriod = period60Hz;
    tick30_2.timelines   = {};
    EXPECT_EQ(pacer30->setupFrame(tick30_2), FramePacer::FrameStatus::ACCEPTED);
    TimePoint expected30 = start + (step30Fps * 2) + (step30Fps * 1); // Render Time (33.33ms) + 2 latency frames (66.66ms) = 100ms
    EXPECT_LT(std::chrono::abs(pacer30->getExpectedPresentationTime() - expected30), std::chrono::nanoseconds(1000));
    mEngine->destroy(pacer30);

    // Sub-test 3: Anomaly Recovery during an OS stall on non-timelined devices
    FramePacer* pacerStall = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacerStall, nullptr);

    // Pre-stall normal frame
    FramePacer::VsyncTick tickPreStall;
    tickPreStall.baseTime    = start;
    tickPreStall.vsyncPeriod = period60Hz;
    tickPreStall.timelines   = {};
    EXPECT_EQ(pacerStall->setupFrame(tickPreStall), FramePacer::FrameStatus::ACCEPTED);

    // Stall arriving 600ms later
    TimePoint postStallTime = start + std::chrono::milliseconds(600);
    FramePacer::VsyncTick tickPostStall;
    tickPostStall.baseTime    = postStallTime;
    tickPostStall.vsyncPeriod = period60Hz;
    tickPostStall.timelines   = {};
    EXPECT_EQ(pacerStall->setupFrame(tickPostStall), FramePacer::FrameStatus::ACCEPTED); // Automatically recovers and re-anchors!

    TimePoint expectedPostStall = postStallTime + (period60Hz * 2);
    EXPECT_LT(std::chrono::abs(pacerStall->getExpectedPresentationTime() - expectedPostStall), std::chrono::nanoseconds(1000));
    mEngine->destroy(pacerStall);
}

// Test Suite 14: Verify robust pacing and presentation predictions under extreme OS Vsync telemetry jitter
TEST_F(FramePacerTest, VsyncTelemetryJitterResilience) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Simulate 4 consecutive Vsync pulses with substantial phase noise / interrupt jitter (+1.5ms, -1.2ms, etc.)
    std::vector<Duration> jitterOffsets = {
        std::chrono::microseconds(0),
        std::chrono::microseconds(1500),  // +1.5 ms delayed interrupt
        std::chrono::microseconds(-1200), // -1.2 ms early interrupt
        std::chrono::microseconds(800)
    };

    for (size_t i = 0; i < jitterOffsets.size(); ++i) {
        FramePacer::VsyncTick tick;
        tick.baseTime    = start + (period60Hz * i) + jitterOffsets[i];
        tick.vsyncPeriod = period60Hz;
        EXPECT_EQ(pacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);
        // Verify expected presentation absorbs the jitter to maintain uniform 60Hz pacing projections
        TimePoint expected = start + (period60Hz * i) + (period60Hz * 2);
        EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected), std::chrono::milliseconds(2));
    }

    mEngine->destroy(pacer);
}

// Test Suite 15: Verify flawless tracking across Variable Refresh Rate (VRR) continuous sweeps (e.g. FreeSync / ProMotion)
TEST_F(FramePacerTest, VariableRefreshRateContinuousSweep) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(0.0f) // Track display panel dynamically
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Sweep across 120Hz -> 90Hz -> 60Hz -> 48Hz
    std::vector<float> vrrFrequencies = { 120.0f, 90.0f, 60.0f, 48.0f };
    std::vector<int> expectedMultipliers = { 4, 3, 2, 2 };
    TimePoint currentStart = start;

    for (size_t i = 0; i < vrrFrequencies.size(); ++i) {
        float freq = vrrFrequencies[i];
        Duration period = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / freq));
        FramePacer::VsyncTick tick;
        tick.baseTime    = currentStart;
        tick.vsyncPeriod = period;
        EXPECT_EQ(pacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);
        EXPECT_NEAR(pacer->getSelectedFrameRate(), freq, 0.1f);

        TimePoint expected = currentStart + std::chrono::nanoseconds(33333333);
        EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected), std::chrono::nanoseconds(1000));
        currentStart += period;
    }

    mEngine->destroy(pacer);
}

// Test Suite 16: Verify immediate graceful recovery during non-monotonic backwards time jumps (e.g. NTP time sync)
TEST_F(FramePacerTest, NonMonotonicBackwardsTimeJump) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Normal Frame 0
    FramePacer::VsyncTick tick0;
    tick0.baseTime    = start;
    tick0.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    // Frame 1 arrives but the OS steady clock has jumped backwards by 500 ms!
    TimePoint timeJumpBack = start - std::chrono::milliseconds(500);
    FramePacer::VsyncTick tickJump;
    tickJump.baseTime    = timeJumpBack;
    tickJump.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tickJump), FramePacer::FrameStatus::ACCEPTED); // Anomaly rejection must instantly approve and snap!

    TimePoint expectedAfterJump = timeJumpBack + (period60Hz * 2);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedAfterJump), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 17: Stress test rapid mid-flight thermal/thermal throttling profile switches via configure(...)
TEST_F(FramePacerTest, RapidThermalThrottlingStressTest) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(120.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Vsync 0: 120 FPS
    FramePacer::VsyncTick tick0;
    tick0.baseTime    = start;
    tick0.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    // Live Switch 1: Thermal throttle to 60 FPS
    FramePacer::Configuration config60;
    config60.targetFrameRate = 60.0f;
    config60.latency         = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(2.0 / 60.0));
    pacer->configure(config60);

    // Vsync 1 (+8.33ms) -> Must be approved because it matches the next scheduled 120Hz phase anchor before the switch
    FramePacer::VsyncTick tick1;
    tick1.baseTime    = start + period120Hz;
    tick1.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);

    // Vsync 2 (+16.66ms) -> Must be correctly skipped to maintain the new 60 FPS throttled cadence!
    FramePacer::VsyncTick tick2;
    tick2.baseTime    = start + (period120Hz * 2);
    tick2.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer->setupFrame(tick2), FramePacer::FrameStatus::SKIPPED_SPURIOUS);

    // Vsync 3 (+25.00ms) -> Approved for 60 FPS
    FramePacer::VsyncTick tick3;
    tick3.baseTime    = start + (period120Hz * 3);
    tick3.vsyncPeriod = period120Hz;
    EXPECT_EQ(pacer->setupFrame(tick3), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_NEAR(pacer->getSelectedFrameRate(), 60.0f, 0.1f);

    mEngine->destroy(pacer);
}

// Test Suite 18: Verify FramePacer operates correctly under extreme Cloud Gaming or VR pipeline depths (latencyFrames = 8)
TEST_F(FramePacerTest, ExtremeCloudGamingPipelineDepth) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(8) // 8 frames of latency!
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    FramePacer::VsyncTick tick;
    tick.baseTime    = start;
    tick.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tick), FramePacer::FrameStatus::ACCEPTED);

    // Target deadline must project exactly 8 latency frames into the future
    TimePoint expected = start + (period60Hz * 8);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 19: Verify dynamic Anomaly Rejection thresholding on ultra-low target frame rates (e.g. 5 FPS / 200ms step)
TEST_F(FramePacerTest, UltraLowFrameRateAnomalyRejection) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration step5Fps   = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 5.0f)); // 200ms step

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(5.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(step5Fps * 2))
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Frame 0: T0
    FramePacer::VsyncTick tick0;
    tick0.baseTime    = start;
    tick0.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    // Now suppose the app experiences a 150ms asset loading or shader compilation stall.
    // The next Vsync arrives at start + 200ms (next scheduled 5 FPS cycle) + 150ms stall = start + 350ms.
    // With a hardcoded 100ms anomaly check, this 150ms delay would exceed 100ms, force an anomalous clock anchor reset,
    // and completely break our scheduled phase looper deadlines.
    // With our new dynamic threshold (max(100ms, targetStep * 2) = 400ms), 150ms is well within 400ms!
    // It should be successfully approved without anomalous resetting!
    TimePoint timeDelayed = start + step5Fps + std::chrono::milliseconds(150);
    FramePacer::VsyncTick tickDelayed;
    tickDelayed.baseTime    = timeDelayed;
    tickDelayed.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer->setupFrame(tickDelayed), FramePacer::FrameStatus::ACCEPTED);

    // Because it did not reset the anchor to timeDelayed, the projected deadline must remain perfectly anchored
    // to our unshifted, true scheduled phase (start + 200ms + 400ms) = start + 600ms!
    // (If it anomalously reset, the deadline would have become timeDelayed + 400ms = start + 750ms).
    TimePoint expectedTruePhase = start + (step5Fps * 3); // 600ms relative deadline for ideal Frame 1
    // We allow up to mHardwarePeriod variance since expected presentation matches the nearest candidate Vsync
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedTruePhase), period60Hz * 2);

    mEngine->destroy(pacer);
}

// Helper struct simulating a slow GPU pipeline
struct SlowGpuSimulation {
    struct GpuFrame {
        uint64_t frameId;
        FramePacer::time_point_t submitTime;
        FramePacer::time_point_t completeTime;
    };

    std::vector<GpuFrame> activeGpuQueue;
    std::vector<GpuFrame> finishedGpuFrames;
    FramePacer::duration_t gpuRenderDuration = std::chrono::milliseconds(25);
    uint32_t maxInFlight = 2; // Simulating shouldRenderFrame N-2 gate

    void updateSimulationClock(FramePacer::time_point_t currentVsync) {
        auto it = activeGpuQueue.begin();
        while (it != activeGpuQueue.end()) {
            if (it->completeTime <= currentVsync) {
                finishedGpuFrames.push_back(*it);
                it = activeGpuQueue.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool shouldRenderFrame() const {
        return activeGpuQueue.size() < maxInFlight;
    }

    void submitFrameToGpu(uint64_t frameId, FramePacer::time_point_t submitTime) {
        FramePacer::time_point_t startTime = submitTime;
        if (!activeGpuQueue.empty()) {
            startTime = std::max(startTime, activeGpuQueue.back().completeTime);
        }
        GpuFrame f;
        f.frameId      = frameId;
        f.submitTime   = submitTime;
        f.completeTime = startTime + gpuRenderDuration;
        activeGpuQueue.push_back(f);
    }
};

// Test Suite 20: Verify completely steady 25ms presentation intervals when GPU bound at 120Hz
TEST_F(FramePacerTest, GpuBoundSteadyState120Hz) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f)); // ~8.33ms

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(120.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(period120Hz * 2))
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    SlowGpuSimulation gpuSim;
    gpuSim.gpuRenderDuration = std::chrono::milliseconds(25);
    gpuSim.maxInFlight       = 2; // N-2 check

    std::vector<TimePoint> actualPresentations;
    uint64_t frameCounter = 0;

    // Run simulation loop for 15 Vsync cycles (0ms to 125ms)
    for (int vsyncIndex = 0; vsyncIndex <= 15; ++vsyncIndex) {
        TimePoint currentVsync = start + (period120Hz * vsyncIndex);
        gpuSim.updateSimulationClock(currentVsync);

        FramePacer::VsyncTick tick;
        tick.baseTime    = currentVsync;
        tick.frameScheduleTime = currentVsync;
        tick.vsyncPeriod = period120Hz;

        if (!gpuSim.shouldRenderFrame() || pacer->setupFrame(tick) != FramePacer::FrameStatus::ACCEPTED) {
            continue;
        }

        // Renders and submits frame
        actualPresentations.push_back(pacer->getExpectedPresentationTime());
        gpuSim.submitFrameToGpu(frameCounter++, currentVsync);
    }

    // In the steady state (from Frame 2 to Frame 3 onwards), presentations must occur exactly at 25ms physical intervals
    ASSERT_GE(actualPresentations.size(), 4);
    for (size_t i = 3; i < actualPresentations.size(); ++i) {
        Duration interval = actualPresentations[i] - actualPresentations[i - 1];
        // Must be exactly 3 Vsync periods (25ms)
        EXPECT_LT(std::chrono::abs(interval - (period120Hz * 3)), std::chrono::nanoseconds(1000));
    }

    mEngine->destroy(pacer);
}

// Test Suite 21: Verify alternating 16.67ms -> 33.33ms presentation intervals when GPU bound at 60Hz
TEST_F(FramePacerTest, GpuBoundSteadyState60Hz) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f)); // ~16.67ms

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    SlowGpuSimulation gpuSim;
    gpuSim.gpuRenderDuration = std::chrono::milliseconds(25);
    gpuSim.maxInFlight       = 2; // N-2 check

    std::vector<TimePoint> actualPresentations;
    uint64_t frameCounter = 0;

    // Run simulation loop for 10 Vsync cycles (0ms to 166ms)
    for (int vsyncIndex = 0; vsyncIndex <= 10; ++vsyncIndex) {
        TimePoint currentVsync = start + (period60Hz * vsyncIndex);
        gpuSim.updateSimulationClock(currentVsync);

        FramePacer::VsyncTick tick;
        tick.baseTime    = currentVsync;
        tick.frameScheduleTime = currentVsync;
        tick.vsyncPeriod = period60Hz;

        if (!gpuSim.shouldRenderFrame() || pacer->setupFrame(tick) != FramePacer::FrameStatus::ACCEPTED) {
            continue;
        }

        // Renders and submits frame
        actualPresentations.push_back(pacer->getExpectedPresentationTime());
        gpuSim.submitFrameToGpu(frameCounter++, currentVsync);
    }

    // In the composite steady state, CPU submission intervals alternate:
    // Frame 0 (0ms), Frame 1 (16.6ms), Frame 2 (33.3ms), Frame 3 (50.0ms), Frame 4 (83.3ms)
    // Projected presentations exactly reflect this submission structure advanced by 2 Vsyncs:
    ASSERT_GE(actualPresentations.size(), 5);
    // Frame 1 to Frame 2: 16.67ms
    EXPECT_LT(std::chrono::abs((actualPresentations[2] - actualPresentations[1]) - period60Hz), std::chrono::nanoseconds(1000));
    // Frame 2 to Frame 3: 16.67ms
    EXPECT_LT(std::chrono::abs((actualPresentations[3] - actualPresentations[2]) - period60Hz), std::chrono::nanoseconds(1000));
    // Frame 3 to Frame 4: 33.33ms (since GPU completes Frame 1 at 50ms, CPU submits Frame 3 at 50ms, then Frame 4 waits until 83.3ms)
    EXPECT_LT(std::chrono::abs((actualPresentations[4] - actualPresentations[3]) - (period60Hz * 2)), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 22: Verify that re-anchoring mNextIdealRenderTime prevents open-loop phase drift and skipped frames
TEST_F(FramePacerTest, OpenLoopPhaseDriftPrevention) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration nominalPeriod = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    // Physical hardware VSYNC interrupt arrives slightly faster/slower than nominal period
    Duration actualHardwarePulse = nominalPeriod - std::chrono::microseconds(150);

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latencyFrames(2)
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    int skippedCount = 0;

    // Run simulation loop for 200 consecutive VSYNC pulses
    for (int i = 0; i <= 200; ++i) {
        TimePoint currentVsync = start + (actualHardwarePulse * i);

        FramePacer::VsyncTick tick;
        tick.baseTime    = currentVsync;
        tick.vsyncPeriod = nominalPeriod;

        if (pacer->setupFrame(tick) != FramePacer::FrameStatus::ACCEPTED) {
            skippedCount++;
        }
    }

    // In an open-loop implementation, phase drift eventually accumulates and causes skipped VSYNCs.
    // In a re-anchored implementation, rendering at native refresh rate must never drop/skip a VSYNC.
    EXPECT_EQ(skippedCount, 0);

    mEngine->destroy(pacer);
}

// Test Suite 23: Verify probabilistic pipeline throughput and latency recommendation model via FramePipelineEstimator
TEST_F(FramePacerTest, FramePipelineEstimatorModel) {
    std::vector<Renderer::FrameInfo> history;
    history.resize(3);

    // Frame 0
    history[0].vsync      = 1000000;
    history[0].beginFrame = 2000000;
    history[0].endFrame   = 9000000; // Main: 8 ms
    history[0].backendBeginFrame = 2000000;
    history[0].backendEndFrame   = 9000000; // Backend: 7 ms
    history[0].gpuFrameDuration  = 9000000; // GPU: 9 ms

    // Frame 1
    history[1].vsync      = 10000000;
    history[1].beginFrame = 11000000;
    history[1].endFrame   = 20000000; // Main: 10 ms
    history[1].backendBeginFrame = 11000000;
    history[1].backendEndFrame   = 19000000; // Backend: 8 ms
    history[1].gpuFrameDuration  = 12000000; // GPU: 12 ms

    // Frame 2
    history[2].vsync      = 20000000;
    history[2].beginFrame = 21000000;
    history[2].endFrame   = 32000000; // Main: 12 ms
    history[2].backendBeginFrame = 21000000;
    history[2].backendEndFrame   = 30000000; // Backend: 9 ms
    history[2].gpuFrameDuration  = 15000000; // GPU: 15 ms

    // For Main: 8, 10, 12 ms -> mean = 10 ms (10,000,000 ns), stdDev = 2 ms (2,000,000 ns)
    // For Backend: 7, 8, 9 ms -> mean = 8 ms (8,000,000 ns), stdDev = 1 ms (1,000,000 ns)
    // For GPU: 9, 12, 15 ms -> mean = 12 ms (12,000,000 ns), stdDev = 3 ms (3,000,000 ns)

    // At P95 (Z = 1.645):
    // EffMain    = 10ms + 1.645 * 2ms = 13.29 ms
    // EffBackend = 8ms + 1.645 * 1ms  = 9.645 ms
    // EffGpu     = 12ms + 1.645 * 3ms = 16.935 ms

    // IdealFrameTime = max(13.29, 9.645, 16.935) = 16.935 ms (16,935,000 ns)
    // TotalTransitTime = 13.29 + 9.645 + 16.935 = 39.87 ms (39,870,000 ns)
    // IdealLatencyFrames = ceil(39.87 / 16.935) = ceil(2.354) = 3 frames

    auto estimation = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P95);

    EXPECT_NEAR(estimation.idealFrameDuration.count(), 16935000, 1000);
    EXPECT_EQ(estimation.idealLatencyFrames, 3);
    EXPECT_NEAR(estimation.safeDelayDuration.count(), 10130000, 1000);
}

TEST_F(FramePacerTest, FramePipelineEstimator_EmptyHistory) {
    auto const estimation = FramePipelineEstimator::estimate({});

    EXPECT_NEAR(estimation.idealFrameDuration.count(), 16666666, 1000);
    EXPECT_NEAR(estimation.idealFrameRate, 60.0f, 0.1f);
    EXPECT_EQ(estimation.idealLatencyFrames, 2);
}

TEST_F(FramePacerTest, FramePipelineEstimator_GapsInHistory) {
    std::vector<Renderer::FrameInfo> history(3);

    // Frame 0 (frameId = 100)
    history[0].frameId           = 100;
    history[0].vsync             = 1000000;
    history[0].beginFrame        = 2000000;
    history[0].endFrame          = 9000000; // Main: 8 ms
    history[0].backendBeginFrame = 2000000;
    history[0].backendEndFrame   = 9000000; // Backend: 7 ms
    history[0].gpuFrameDuration  = 9000000; // GPU: 9 ms

    // Frame 1 (frameId = 102 - Frame 101 skipped)
    history[1].frameId           = 102;
    history[1].vsync             = 10000000;
    history[1].beginFrame        = 11000000;
    history[1].endFrame          = 20000000; // Main: 10 ms
    history[1].backendBeginFrame = 11000000;
    history[1].backendEndFrame   = 19000000; // Backend: 8 ms
    history[1].gpuFrameDuration  = 12000000; // GPU: 12 ms

    // Frame 2 (frameId = 105 - Frames 103, 104 skipped)
    history[2].frameId           = 105;
    history[2].vsync             = 20000000;
    history[2].beginFrame        = 21000000;
    history[2].endFrame          = 32000000; // Main: 12 ms
    history[2].backendBeginFrame = 21000000;
    history[2].backendEndFrame   = 30000000; // Backend: 9 ms
    history[2].gpuFrameDuration  = 15000000; // GPU: 15 ms

    auto estimation = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P95);

    // Results must match the contiguous case in FramePipelineEstimatorModel exactly
    EXPECT_NEAR(estimation.idealFrameDuration.count(), 16935000, 1000);
    EXPECT_EQ(estimation.idealLatencyFrames, 3);
    EXPECT_NEAR(estimation.safeDelayDuration.count(), 10130000, 1000);
}

TEST_F(FramePacerTest, FramePipelineEstimator_NoGpuTimings) {
    std::vector<Renderer::FrameInfo> history(3);

    // Frame 0
    history[0].vsync             = 1000000;
    history[0].beginFrame        = 2000000;
    history[0].endFrame          = 9000000; // Main: 8 ms
    history[0].backendBeginFrame = 2000000;
    history[0].backendEndFrame   = 9000000; // Backend: 7 ms
    history[0].gpuFrameDuration  = 0;       // No GPU timer

    // Frame 1
    history[1].vsync             = 10000000;
    history[1].beginFrame        = 11000000;
    history[1].endFrame          = 20000000; // Main: 10 ms
    history[1].backendBeginFrame = 11000000;
    history[1].backendEndFrame   = 19000000; // Backend: 8 ms
    history[1].gpuFrameDuration  = 0;

    // Frame 2
    history[2].vsync             = 20000000;
    history[2].beginFrame        = 21000000;
    history[2].endFrame          = 32000000; // Main: 12 ms
    history[2].backendBeginFrame = 21000000;
    history[2].backendEndFrame   = 30000000; // Backend: 9 ms
    history[2].gpuFrameDuration  = 0;

    auto const estimation = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P90);

    EXPECT_NEAR(estimation.idealFrameDuration.count(), 12564000, 1000);
    EXPECT_EQ(estimation.idealLatencyFrames, 2);
}

TEST_F(FramePacerTest, FramePipelineEstimator_Percentiles) {
    std::vector<Renderer::FrameInfo> history(3);

    // Frame 0
    history[0].vsync             = 1000000;
    history[0].beginFrame        = 2000000;
    history[0].endFrame          = 9000000; // Main: 8 ms
    history[0].backendBeginFrame = 2000000;
    history[0].backendEndFrame   = 9000000; // Backend: 7 ms
    history[0].gpuFrameDuration  = 9000000; // GPU: 9 ms

    // Frame 1
    history[1].vsync             = 10000000;
    history[1].beginFrame        = 11000000;
    history[1].endFrame          = 20000000; // Main: 10 ms
    history[1].backendBeginFrame = 11000000;
    history[1].backendEndFrame   = 19000000; // Backend: 8 ms
    history[1].gpuFrameDuration  = 12000000; // GPU: 12 ms

    // Frame 2
    history[2].vsync             = 20000000;
    history[2].beginFrame        = 21000000;
    history[2].endFrame          = 32000000; // Main: 12 ms
    history[2].backendBeginFrame = 21000000;
    history[2].backendEndFrame   = 30000000; // Backend: 9 ms
    history[2].gpuFrameDuration  = 15000000; // GPU: 15 ms

    auto const est50 = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P50);
    auto const est90 = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P90);
    auto const est95 = FramePipelineEstimator::estimate({history.data(), history.size()}, FramePipelineEstimator::TargetPercentile::P95);

    EXPECT_NEAR(est50.idealFrameDuration.count(), 12000000, 1000);

    EXPECT_GT(est90.idealFrameDuration, est50.idealFrameDuration);
    EXPECT_LT(est90.idealFrameDuration, est95.idealFrameDuration);

    EXPECT_NEAR(est90.idealFrameDuration.count(), 15846000, 1000);
    EXPECT_NEAR(est95.idealFrameDuration.count(), 16935000, 1000);
}

TEST_F(FramePacerTest, FramePipelineEstimator_SafeDelay) {
    std::vector<Renderer::FrameInfo> history(3);

    // Frame 0
    history[0].vsync             = 1000000;
    history[0].beginFrame        = 2000000;
    history[0].endFrame          = 8000000;  // Main: 7 ms
    history[0].backendBeginFrame = 2000000;
    history[0].backendEndFrame   = 7500000;  // Backend: 5.5 ms
    history[0].gpuFrameDuration  = 8000000;  // GPU: 8 ms

    // Frame 1
    history[1].vsync             = 10000000;
    history[1].beginFrame        = 11000000;
    history[1].endFrame          = 18000000; // Main: 8 ms
    history[1].backendBeginFrame = 11000000;
    history[1].backendEndFrame   = 17000000; // Backend: 6.0 ms
    history[1].gpuFrameDuration  = 10000000; // GPU: 10 ms

    // Frame 2
    history[2].vsync             = 20000000;
    history[2].beginFrame        = 21000000;
    history[2].endFrame          = 29000000; // Main: 9 ms
    history[2].backendBeginFrame = 21000000;
    history[2].backendEndFrame   = 27500000; // Backend: 6.5 ms
    history[2].gpuFrameDuration  = 12000000; // GPU: 12 ms

    // Calculations for P90 (Z = 1.282):
    // EffMain = 8ms + 1.282 * 1ms = 9.282 ms (9,282,000 ns)
    // EffBackend = 6ms + 1.282 * 0.5ms = 6.641 ms (6,641,000 ns)
    // EffGpu = 10ms + 1.282 * 2ms = 12.564 ms (12,564,000 ns)
    //
    // IdealFrameTime = max(9.282, 6.641, 12.564) = 12.564 ms (12,564,000 ns)
    // TotalTransit = 9.282 + 6.641 + 12.564 = 28.487 ms (28,487,000 ns)
    // IdealLatency = ceil(28.487 / 12.564) = 3 frames

    // Test 1: VSYNC = 16.666ms (60Hz)
    // Budget = 3 * 16.666666ms = 50ms (50,000,000 ns)
    // SafeDelay = 50,000,000 - 28,487,000 = 21,513,000 ns
    {
        auto const est = FramePipelineEstimator::estimate(
                {history.data(), history.size()},
                FramePipelineEstimator::TargetPercentile::P90,
                std::chrono::nanoseconds(16666666));

        EXPECT_EQ(est.idealLatencyFrames, 3);
        EXPECT_NEAR(est.safeDelayDuration.count(), 21513000, 5000);
    }

    // Test 2: VSYNC = 11.111ms (90Hz)
    // Budget = 3 * 11.111111ms = 33.333333ms (33,333,333 ns)
    // SafeDelay = 33,333,333 - 28,487,000 = 4,846,333 ns
    {
        auto const est = FramePipelineEstimator::estimate(
                {history.data(), history.size()},
                FramePipelineEstimator::TargetPercentile::P90,
                std::chrono::nanoseconds(11111111));

        EXPECT_EQ(est.idealLatencyFrames, 3);
        EXPECT_NEAR(est.safeDelayDuration.count(), 4846333, 5000);
    }

    // Test 3: VSYNC = 8.333ms (120Hz)
    // Budget = 3 * 8.333333ms = 25ms (25,000,000 ns)
    // SafeDelay = 25,000,000 - 28,487,000 = -3.487ms -> Clamped to 0
    {
        auto const est = FramePipelineEstimator::estimate(
                {history.data(), history.size()},
                FramePipelineEstimator::TargetPercentile::P90,
                std::chrono::nanoseconds(8333333));

        EXPECT_EQ(est.idealLatencyFrames, 3);
        EXPECT_EQ(est.safeDelayDuration.count(), 0);
    }
}

// Test Suite 24: Verify BufferStuffingDetector boundary conditions, filtering, stuffing detection, and post-skip recovery
TEST_F(FramePacerTest, BufferStuffingDetectorComprehensive) {
    BufferStuffingDetector detector;
    std::vector<Renderer::FrameInfo> history;
    uint32_t currentFrameId = 100;

    // 1. Verify empty history is ignored, and single healthy frame does not skip
    EXPECT_FALSE(detector.shouldSkipFrame({}, currentFrameId));
    history.resize(1);
    history[0].frameId = 99;
    history[0].vsync = 1000000;
    history[0].expectedPresentLatency = 16666666;
    history[0].displayPresentInterval = 16666666;
    history[0].displayPresent = history[0].vsync + history[0].expectedPresentLatency; // Healthy
    history[0].presentDeadline = 15000000;
    history[0].gpuFrameComplete = 10000000;
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, currentFrameId));

    // 2. Verify pending / in-flight frames are ignored
    history.resize(2);
    for (auto& info : history) {
        info.frameId = --currentFrameId;
        info.displayPresent = -1; // Pending
        info.presentDeadline = 20000000;
    }
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 100));

    // 3. Verify healthy frames (meeting presentation deadlines) do not trigger skips
    currentFrameId = 100;
    for (auto& info : history) {
        info.frameId = --currentFrameId;
        info.vsync = 1000000;
        info.expectedPresentLatency = 16666666;
        info.displayPresentInterval = 16666666;
        info.displayPresent = info.vsync + info.expectedPresentLatency; // On time
        info.presentDeadline = 15000000;
        info.gpuFrameComplete = 10000000; // Early
    }
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 100));

    // 4. Verify GPU/CPU bottleneck (late completion) does not trigger stuffing skip
    currentFrameId = 100;
    for (auto& info : history) {
        info.frameId = --currentFrameId;
        info.vsync = 1000000;
        info.expectedPresentLatency = 16666666;
        info.displayPresentInterval = 16666666;
        info.displayPresent = info.vsync + info.expectedPresentLatency + info.displayPresentInterval;
        info.presentDeadline = 15000000;
        info.gpuFrameComplete = 16000000; // Missed completion deadline (> presentDeadline)
    }
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 100));

    // 5. Verify isolated stuffing (N=1) triggers skip
    history[0].frameId = 99;
    history[0].vsync = 1000000;
    history[0].expectedPresentLatency = 16666666;
    history[0].displayPresentInterval = 16666666;
    history[0].displayPresent = history[0].vsync + history[0].expectedPresentLatency + 35000000; // Late
    history[0].presentDeadline = 15000000;
    history[0].gpuFrameComplete = 10000000;

    history[1].frameId = 98;
    history[1].displayPresent = history[1].vsync + history[1].expectedPresentLatency;
    history[1].presentDeadline = 15000000;
    history[1].gpuFrameComplete = 10000000;
    EXPECT_TRUE(detector.shouldSkipFrame({history.data(), history.size()}, 99));
    detector.setLastFrameId(99); // Record skip manually

    detector.setLastFrameId(0); // Reset detector state for next test case

    // 6. Verify consecutive stuffed frames trigger skip and subsequent recovery
    for (int i = 0; i < 2; i++) {
        history[i].frameId = 99 - i;
        history[i].vsync = 1000000;
        history[i].expectedPresentLatency = 16666666;
        history[i].displayPresentInterval = 16666666;
        history[i].displayPresent = history[i].vsync + history[i].expectedPresentLatency + 35000000; // Late
        history[i].presentDeadline = 15000000;
        history[i].gpuFrameComplete = 10000000; // Completed early (>= 0.5ms margin)
    }

    // Call 1 (Frame 100 attempt with last submitted frame ID 99): Should detect stuffing
    EXPECT_TRUE(detector.shouldSkipFrame({history.data(), history.size()}, 99));
    detector.setLastFrameId(99); // Record skip manually
    EXPECT_EQ(detector.getLastFrameId(), 99);

    // Call 2 (Frame 101 attempt with last submitted frame ID 99): Should evaluate to false (Recovery)
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 99));
}

// Test Suite 25: Verify that passing the last submitted frame ID (mLastSubmittedFrameId)
// correctly anchors mLastFrameId to the submitted frame rather than an unrecorded skipped frame
// so that subsequent checks do not evaluate stale history relative to a skipped frame ID.
TEST_F(FramePacerTest, BufferStuffingDetectorStaleHistoryRecovery) {
    BufferStuffingDetector detector;
    std::vector<Renderer::FrameInfo> history(2);

    history[0].frameId = 99;
    history[0].vsync = 1000000;
    history[0].expectedPresentLatency = 16666666;
    history[0].displayPresentInterval = 16666666;
    history[0].displayPresent = history[0].vsync + history[0].expectedPresentLatency + 35000000;
    history[0].presentDeadline = 15000000;
    history[0].gpuFrameComplete = 10000000;

    history[1].frameId = 98;
    history[1].vsync = 1000000;
    history[1].expectedPresentLatency = 16666666;
    history[1].displayPresentInterval = 16666666;
    history[1].displayPresent = history[1].vsync + history[1].expectedPresentLatency + 35000000;
    history[1].presentDeadline = 15000000;
    history[1].gpuFrameComplete = 10000000;

    // Call 1 on Frame 100 attempt (last submitted frame ID = 99) -> Detects stuffing
    EXPECT_TRUE(detector.shouldSkipFrame({history.data(), history.size()}, 99));
    detector.setLastFrameId(99); // Record skip manually
    EXPECT_EQ(detector.getLastFrameId(), 99);

    // Call 2 on Frame 101 attempt (where Frame 100 was skipped, so last submitted frame ID is still 99)
    // Must immediately recover and evaluate to false!
    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 99));
}

TEST_F(FramePacerTest, BufferStuffingDetector_HealthyNewestStuffedOlder) {
    BufferStuffingDetector detector;
    std::vector<Renderer::FrameInfo> history(3);

    // history[0]: Healthy frame (met presentation)
    history[0].frameId                = 99;
    history[0].vsync                  = 30000000;
    history[0].expectedPresentLatency = 16666666;
    history[0].displayPresentInterval = 16666666;
    history[0].displayPresent         = history[0].vsync + history[0].expectedPresentLatency;
    history[0].presentDeadline        = 45000000;
    history[0].gpuFrameComplete       = 40000000;

    // history[1]: Late frame (missed presentation)
    history[1].frameId                = 98;
    history[1].vsync                  = 20000000;
    history[1].expectedPresentLatency = 16666666;
    history[1].displayPresentInterval = 16666666;
    history[1].displayPresent         = history[1].vsync + history[1].expectedPresentLatency + 35000000;
    history[1].presentDeadline        = 35000000;
    history[1].gpuFrameComplete       = 30000000;

    // history[2]: Healthy frame (met presentation)
    history[2].frameId                = 97;
    history[2].vsync                  = 10000000;
    history[2].expectedPresentLatency = 16666666;
    history[2].displayPresentInterval = 16666666;
    history[2].displayPresent         = history[2].vsync + history[2].expectedPresentLatency;
    history[2].presentDeadline        = 25000000;
    history[2].gpuFrameComplete       = 20000000;

    EXPECT_FALSE(detector.shouldSkipFrame({history.data(), history.size()}, 100));
}

// Test Suite 26: Verify that reducing latencyFrames dynamically triggers the Monotonic Presentation Guard
// to skip non-monotonic presentation targets (scheduling <= previous presentation timestamp).
TEST_F(FramePacerTest, MonotonicPresentationGuard) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period100Hz = std::chrono::milliseconds(10); // 100Hz display (10ms)

    FramePacer* pacer = FramePacer::Builder()
            .targetFrameRate(100.0f)
            .latency(std::chrono::milliseconds(30)) // 3 frames of latency
            .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Frame 0 at T=0 with Latency 30ms -> Approved (Expected presentation T=30ms)
    FramePacer::VsyncTick tick0;
    tick0.baseTime    = start;
    tick0.vsyncPeriod = period100Hz;
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expected0 = start + std::chrono::milliseconds(30);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected0), std::chrono::nanoseconds(1000));

    // Reconfigure: User drops latency from 30ms down to 20ms
    FramePacer::Configuration newConfig;
    newConfig.targetFrameRate = 100.0f;
    newConfig.latency         = std::chrono::milliseconds(20);
    pacer->configure(newConfig);

    // Frame 1 at T=10ms with Latency 20ms -> Would target T=30ms (same as Frame 0).
    // Monotonic Presentation Guard must skip it!
    FramePacer::VsyncTick tick1;
    tick1.baseTime    = start + period100Hz;
    tick1.vsyncPeriod = period100Hz;
    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::SKIPPED_STALE); // Correctly skipped!

    // Frame 2 at T=20ms with Latency 20ms -> Targets T=40ms (> 30ms). Approved!
    FramePacer::VsyncTick tick2;
    tick2.baseTime    = start + (period100Hz * 2);
    tick2.vsyncPeriod = period100Hz;
    EXPECT_EQ(pacer->setupFrame(tick2), FramePacer::FrameStatus::ACCEPTED);

    TimePoint expected2 = start + std::chrono::milliseconds(40);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expected2), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 27: Buffer Stuffing (Relative Presentation Pacing) Absorbs Small Delays
TEST_F(FramePacerTest, BufferStuffingAbsorbsSmallDelay) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(3)
                        .build(*mEngine);

    // Steady State: Frames 0, 1
    for (int i = 0; i < 2; i++) {
        FramePacer::VsyncTick tick;
        tick.baseTime = start + (period60Hz * i);
        tick.frameScheduleTime = tick.baseTime;
        tick.vsyncPeriod = period60Hz;

        std::vector<FramePacer::HardwareTimeline> timelines;
        FramePacer::HardwareTimeline timeline;
        timeline.deadline = tick.baseTime + period60Hz;
        timeline.expectedPresentationTime = tick.baseTime + (period60Hz * 3);
        timelines.push_back(timeline);

        // Next timeline
        FramePacer::HardwareTimeline nextTimeline;
        nextTimeline.deadline = tick.baseTime + (period60Hz * 2);
        nextTimeline.expectedPresentationTime = tick.baseTime + (period60Hz * 4);
        timelines.push_back(nextTimeline);

        tick.timelines = {timelines.data(), timelines.size()};
        pacer->setupFrame(tick);
    }

    // Frame 3 is delayed slightly (CPU spike).
    // Wait! Since it's delayed, baseTime is the same, but it misses the deadline!
    FramePacer::VsyncTick lateTick;
    lateTick.baseTime = start + (period60Hz * 2); // Old vsync woke us up
    lateTick.frameScheduleTime = lateTick.baseTime + period60Hz + (period60Hz / 2); // 25ms delay!
    lateTick.vsyncPeriod = period60Hz;

    // The timelines generated relative to lateTick.baseTime
    std::vector<FramePacer::HardwareTimeline> lateTimelines;
    FramePacer::HardwareTimeline tl1;
    tl1.deadline = lateTick.baseTime + period60Hz; // deadline in the past!
    tl1.expectedPresentationTime = lateTick.baseTime + (period60Hz * 3);

    FramePacer::HardwareTimeline tl2;
    tl2.deadline = lateTick.baseTime + (period60Hz * 2); // deadline is in the future
    tl2.expectedPresentationTime = lateTick.baseTime + (period60Hz * 4);

    lateTimelines.push_back(tl1);
    lateTimelines.push_back(tl2);
    lateTick.timelines = {lateTimelines.data(), lateTimelines.size()};

    pacer->setupFrame(lateTick);

    // Because tl1's deadline is in the past, it gets filtered out by matchHardwareTimeline.
    // It should seamlessly pick tl2 (slip by 1 frame), avoiding a reset!
    TimePoint expectedPresent = lateTick.baseTime + (period60Hz * 4);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedPresent), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 28: Massive Delay Forces Buffer Underflow Reset
TEST_F(FramePacerTest, BufferUnderflowForcesLatencyReset) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(3)
                        .build(*mEngine);

    // Steady state frame
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.frameScheduleTime = start;
    tick0.vsyncPeriod = period60Hz;
    std::vector<FramePacer::HardwareTimeline> tls0;
    tls0.push_back({start + (period60Hz * 3), start + period60Hz});
    tick0.timelines = {tls0.data(), tls0.size()};
    pacer->setupFrame(tick0);

    // MASSIVE Delay (50ms)
    FramePacer::VsyncTick lateTick;
    lateTick.baseTime = start; // Same old vsync
    lateTick.frameScheduleTime = start + (period60Hz * 3) + (period60Hz / 2); // Now is ~58ms later
    lateTick.vsyncPeriod = period60Hz;

    // Timelines array generated from the old baseTime
    std::vector<FramePacer::HardwareTimeline> lateTimelines;
    lateTimelines.push_back({start + (period60Hz * 3), start + period60Hz});
    lateTimelines.push_back({start + (period60Hz * 4), start + (period60Hz * 2)});
    lateTimelines.push_back({start + (period60Hz * 5), start + (period60Hz * 3)});
    lateTick.timelines = {lateTimelines.data(), lateTimelines.size()};

    pacer->setupFrame(lateTick);

    // Because ALL deadlines are <= frameScheduleTime (start + 3.5 * period), it hits fallback.
    // This triggers the explicit Underflow Reset!
    // It clears buffer stuffing and resets to baseTime + 3*period = start + 3*period.
    TimePoint expectedPresent = start + (period60Hz * 3);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedPresent), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 29: Latency Config Change forces a reset
TEST_F(FramePacerTest, LatencyConfigChangeForcesReset) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder().targetFrameRate(60.0f).latencyFrames(1).build(*mEngine);

    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    pacer->setupFrame(tick0); // Warms up mLastTargetPresentationTime

    // Change config
    FramePacer::Configuration newConfig;
    newConfig.targetFrameRate = 60.0f;
    newConfig.latency         = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(3.0 / 60.0));
    pacer->configure(newConfig);

    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + period60Hz;
    tick1.vsyncPeriod = period60Hz;

    // MUST reset buffer stuffing and strictly use the new latency
    pacer->setupFrame(tick1);
    TimePoint expectedPresent = tick1.baseTime + (period60Hz * 3); // Latency 3
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedPresent), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 30: Hardware Refresh Rate Change is absorbed organically
TEST_F(FramePacerTest, HardwarePeriodChangeIsAbsorbed) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration period120Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 120.0f));

    FramePacer* pacer = FramePacer::Builder().targetFrameRate(30.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(period60Hz * 4))
            .build(*mEngine);

    // 60Hz tick
    FramePacer::VsyncTick tick60;
    tick60.baseTime = start;
    tick60.frameScheduleTime = tick60.baseTime;
    tick60.vsyncPeriod = period60Hz;
    std::vector<FramePacer::HardwareTimeline> tls60;
    tls60.push_back({start + (period60Hz * 2), start + period60Hz});
    tls60.push_back({start + (period60Hz * 4), start + (period60Hz * 3)});
    tls60.push_back({start + (period60Hz * 6), start + (period60Hz * 5)});
    tls60.push_back({start + (period60Hz * 8), start + (period60Hz * 7)});
    tls60.push_back({start + (period60Hz * 10), start + (period60Hz * 9)});
    tick60.timelines = {tls60.data(), tls60.size()};
    pacer->setupFrame(tick60);

    // 120Hz tick arrives!
    FramePacer::VsyncTick tick120;
    tick120.baseTime = start + period60Hz;
    tick120.frameScheduleTime = tick120.baseTime;
    tick120.vsyncPeriod = period120Hz;
    std::vector<FramePacer::HardwareTimeline> tls120;
    tls120.push_back({tick120.baseTime + (period120Hz * 2), tick120.baseTime + period120Hz});
    tls120.push_back({tick120.baseTime + (period120Hz * 4), tick120.baseTime + (period120Hz * 3)});
    tls120.push_back({tick120.baseTime + (period120Hz * 6), tick120.baseTime + (period120Hz * 5)});
    tls120.push_back({tick120.baseTime + (period120Hz * 8), tick120.baseTime + (period120Hz * 7)});
    tls120.push_back({tick120.baseTime + (period120Hz * 10), tick120.baseTime + (period120Hz * 9)});
    tick120.timelines = {tls120.data(), tls120.size()};

    // Buffer stuffing remains active, smoothly mapping to the 120Hz grid
    pacer->setupFrame(tick120);
    TimePoint expectedPresent = tick60.baseTime + (period60Hz * 2) + (period60Hz * 2); // Next 30fps target
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedPresent), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 31: Inexact Framerate Disables Buffer Stuffing to prevent Quantization Drag
TEST_F(FramePacerTest, InexactFramerateDisablesBufferStuffing) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));
    Duration period50Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 50.0f)); // 20ms

    FramePacer* pacer = FramePacer::Builder().targetFrameRate(50.0f)
            .latency(std::chrono::duration_cast<std::chrono::nanoseconds>(period50Hz * 2))
            .build(*mEngine);

    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    pacer->setupFrame(tick0);

    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + period60Hz;
    tick1.vsyncPeriod = period60Hz;
    pacer->setupFrame(tick1);

    // If it disabled Buffer Stuffing correctly, it uses Rigid Anchoring (mExpectedBaseTime).
    // The expected presentation should be an exact multiple of 20ms, ignoring the hardware snap!
    TimePoint expectedPresent = start + period50Hz + (period50Hz * 2);
    EXPECT_LT(std::chrono::abs(pacer->getExpectedPresentationTime() - expectedPresent), std::chrono::nanoseconds(1000));

    mEngine->destroy(pacer);
}

// Test Suite 32: Verify that time-based latency automatically rounds and scales to vsync periods
TEST_F(FramePacerTest, TimeBasedLatencyVsyncRounding) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // Case 1: 50ms latency at 60 FPS target rate on 60Hz display
    FramePacer* pacer60 = FramePacer::Builder()
            .targetFrameRate(60.0f)
            .latency(std::chrono::milliseconds(50))
            .build(*mEngine);
    ASSERT_NE(pacer60, nullptr);

    FramePacer::VsyncTick tick60;
    tick60.baseTime = start;
    tick60.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer60->setupFrame(tick60), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer60->getEffectiveLatency(), std::chrono::nanoseconds(50000000));
    mEngine->destroy(pacer60);

    // Case 2: 50ms latency at 15 FPS target rate on 60Hz display
    FramePacer* pacer15 = FramePacer::Builder()
            .targetFrameRate(15.0f)
            .latency(std::chrono::milliseconds(50))
            .build(*mEngine);
    ASSERT_NE(pacer15, nullptr);

    FramePacer::VsyncTick tick15;
    tick15.baseTime = start;
    tick15.vsyncPeriod = period60Hz;
    EXPECT_EQ(pacer15->setupFrame(tick15), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer15->getEffectiveLatency(), std::chrono::nanoseconds(50000000));
    mEngine->destroy(pacer15);

    // Case 3: Candidate matching with time-based latency
    FramePacer* pacerCandidate = FramePacer::Builder()
            .targetFrameRate(15.0f)
            .latency(std::chrono::milliseconds(50))
            .build(*mEngine);
    ASSERT_NE(pacerCandidate, nullptr);

    std::vector<FramePacer::HardwareTimeline> candidates = {
        { start + period60Hz,     start + period60Hz - std::chrono::milliseconds(5) },
        { start + period60Hz * 2, start + period60Hz * 2 - std::chrono::milliseconds(5) },
        { start + period60Hz * 3, start + period60Hz * 3 - std::chrono::milliseconds(5) }, // 50ms match
        { start + period60Hz * 4, start + period60Hz * 4 - std::chrono::milliseconds(5) }
    };

    FramePacer::VsyncTick tickCandidate;
    tickCandidate.baseTime = start;
    tickCandidate.vsyncPeriod = period60Hz;
    tickCandidate.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());

    EXPECT_EQ(pacerCandidate->setupFrame(tickCandidate), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacerCandidate->getExpectedPresentationTime(), candidates[2].expectedPresentationTime);
    EXPECT_EQ(pacerCandidate->getEffectiveLatency(), candidates[2].expectedPresentationTime - start);

    mEngine->destroy(pacerCandidate);
}

// Test Suite 33: Verify that clock drift relative to true hardware period does not accumulate
// and cause timeline expected presentation time to drift into a later vsync interval (causing stutter).
// This test targets 45 FPS on a 60Hz display, which is an inexact frame rate (exactAchieved = false),
// and therefore relies on Rigid Anchoring.
TEST_F(FramePacerTest, PhaseDriftWithTimelineMatching) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());

    // Nominal period used by FramePacer configuration: 16.666ms (60.0 Hz display)
    Duration nominalPeriod = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    // True hardware oscillator period: 16.650264ms (slightly faster)
    Duration truePeriod = std::chrono::nanoseconds(16650264);

    // We target 45 FPS, which is inexact on 60Hz (22.22ms step vs 16.66ms hardware period)
    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(45.0f)
                        .latency(std::chrono::nanoseconds(33333333)) // 33.3ms latency
                        .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    int acceptedFrameCount = 0;

    // Run the looper for 2000 VSYNC ticks (approx 33 seconds)
    for (int i = 0; i < 2000; ++i) {
        TimePoint currentVsync = start + (truePeriod * i);

        FramePacer::VsyncTick tick;
        tick.baseTime = currentVsync;
        tick.vsyncPeriod = nominalPeriod;

        // Simulate 4 timeline candidates that track the true VSYNC grid (T+1, T+2, T+3, T+4 true Vsyncs)
        std::vector<FramePacer::HardwareTimeline> timelines = {
            {currentVsync + truePeriod, currentVsync + truePeriod - std::chrono::milliseconds(5)},
            {currentVsync + truePeriod * 2, currentVsync + truePeriod * 2 - std::chrono::milliseconds(5)},
            {currentVsync + truePeriod * 3, currentVsync + truePeriod * 3 - std::chrono::milliseconds(5)},
            {currentVsync + truePeriod * 4, currentVsync + truePeriod * 4 - std::chrono::milliseconds(5)}
        };
        tick.timelines = utils::Slice<const FramePacer::HardwareTimeline>(timelines.data(), timelines.size());

        FramePacer::FrameStatus status = pacer->setupFrame(tick);

        if (status == FramePacer::FrameStatus::SKIPPED_SPURIOUS) {
            // Skips must always occur at expected 4*k + 2 phase: 2, 6, 10, 14...
            // If the phase drifts, it will eventually skip at a different index (e.g. 4*k).
            EXPECT_EQ(i % 4, 2)
                    << "Cadence phase shifted at VSYNC " << i << "! Expected skip at phase 2, but skipped at phase " <<
                        (i % 4);
        }

        if (status == FramePacer::FrameStatus::ACCEPTED) {
            acceptedFrameCount++;

            TimePoint expectedPresent = pacer->getExpectedPresentationTime();
            Duration offset = (expectedPresent - start) % truePeriod;
            Duration error = std::min(offset, truePeriod - offset);

            EXPECT_LT(error, std::chrono::microseconds(10))
                    << "Drift accumulated at VSYNC " << i << " (accepted frame " << acceptedFrameCount
                    << ")! Expected presentation time " << expectedPresent.time_since_epoch().count()
                    << " is not aligned with true VSYNC grid. Error = " << error.count() << " ns.";
        }
    }

    mEngine->destroy(pacer);
}

// Test Suite 34: Verify Flow Control API (PacingStatus and resetPacing)
TEST_F(FramePacerTest, FlowControlPacingStatus) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(2)
                        .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);

    // Cycle 0: Start up. We hit ideal (Timeline 2).
    std::vector<FramePacer::HardwareTimeline> candidates = {
        {start + period60Hz, start + period60Hz - std::chrono::milliseconds(4)},
        {start + (period60Hz * 2), start + (period60Hz * 2) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 3), start + (period60Hz * 3) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    tick0.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);

    // Apply presentation time to advance mLastTargetPresentationTime
    Renderer* renderer = mEngine->createRenderer();
    pacer->applyPresentationTime(renderer);

    // Cycle 1: CPU Starves and drops a frame.
    // Base time jumps 2 periods, but relative pacing only advances 1 period.
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + (period60Hz * 2);
    tick1.vsyncPeriod = period60Hz;

    std::vector<FramePacer::HardwareTimeline> candidates1 = {
        {start + (period60Hz * 3), start + (period60Hz * 3) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 4), start + (period60Hz * 4) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 5), start + (period60Hz * 5) - std::chrono::milliseconds(4)}
    };
    tick1.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates1.data(), candidates1.size());

    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);

    // Pipeline is now starved (locked into an earlier timeline than ideal)
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::DISPLAY_STARVING);

    // Recover using resetPacing()
    pacer->resetPacing();

    // setupFrame again for the same cycle to see it rigidly anchor back to ideal (Timeline 2)
    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);

    mEngine->destroy(renderer);
    mEngine->destroy(pacer);
}

// Test Suite 35: Verify Queue Stuffing via setupExtraFrame
TEST_F(FramePacerTest, FlowControlStuffQueue) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(2)
                        .build(*mEngine);
    ASSERT_NE(pacer, nullptr);
    Renderer* renderer = mEngine->createRenderer();

    // Cycle 0: Start up. We hit ideal.
    std::vector<FramePacer::HardwareTimeline> candidates = {
        {start + period60Hz, start + period60Hz - std::chrono::milliseconds(4)},
        {start + (period60Hz * 2), start + (period60Hz * 2) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 3), start + (period60Hz * 3) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    tick0.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 2));
    pacer->applyPresentationTime(renderer);

    // Cycle 1: CPU Starves and drops a frame.
    // Base time jumps 2 periods, but relative pacing only advances 1 period.
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + (period60Hz * 2);
    tick1.vsyncPeriod = period60Hz;
    std::vector<FramePacer::HardwareTimeline> candidates1 = {
        {start + (period60Hz * 3), start + (period60Hz * 3) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 4), start + (period60Hz * 4) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 5), start + (period60Hz * 5) - std::chrono::milliseconds(4)}
    };
    tick1.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates1.data(), candidates1.size());

    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::DISPLAY_STARVING);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 3));
    pacer->applyPresentationTime(renderer);

    // Now we recover using the setupExtraFrame mechanism
    EXPECT_TRUE(pacer->setupExtraFrame());
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 4));
    // (In reality, we would apply presentation time and render a frame here)
    pacer->applyPresentationTime(renderer);

    // Cycle 2: Next real Vsync arrives (16.6ms later)
    FramePacer::VsyncTick tick2;
    tick2.baseTime = start + (period60Hz * 3);
    tick2.vsyncPeriod = period60Hz;
    std::vector<FramePacer::HardwareTimeline> candidates2 = {
        {start + (period60Hz * 4), start + (period60Hz * 4) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 5), start + (period60Hz * 5) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 6), start + (period60Hz * 6) - std::chrono::milliseconds(4)}
    };
    tick2.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates2.data(), candidates2.size());

    EXPECT_EQ(pacer->setupFrame(tick2), FramePacer::FrameStatus::ACCEPTED);

    // Everything should be fully recovered and STEADY
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 5));
    pacer->applyPresentationTime(renderer);

    mEngine->destroy(renderer);
    mEngine->destroy(pacer);
}

// Test Suite 36: Verify setupExtraFrame safeguard against overstuffing
TEST_F(FramePacerTest, FlowControlRefuseOverstuffing) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(1) // 1 frame latency
                        .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Cycle 0: Start up. We hit ideal (1 frame latency).
    std::vector<FramePacer::HardwareTimeline> candidates = {
        {start + period60Hz, start + period60Hz - std::chrono::milliseconds(4)},
        {start + (period60Hz * 2), start + (period60Hz * 2) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.vsyncPeriod = period60Hz;
    tick0.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates.data(), candidates.size());
    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);

    // We are perfectly on schedule. Calling setupExtraFrame should be rejected.
    EXPECT_FALSE(pacer->setupExtraFrame());

    mEngine->destroy(pacer);
}

// Test Suite 37: Verify instant clock recovery and exact alignment after a skipped vsync callback
TEST_F(FramePacerTest, SkippedVsyncCallbackInstantRecovery) {
    TimePoint start = TimePoint(std::chrono::steady_clock::now());
    Duration period60Hz = std::chrono::duration_cast<Duration>(std::chrono::duration<float>(1.0f / 60.0f));

    FramePacer* pacer = FramePacer::Builder()
                        .targetFrameRate(60.0f)
                        .latencyFrames(1)
                        .build(*mEngine);
    ASSERT_NE(pacer, nullptr);

    // Callback 0 arrives at t=0, the list of timelines is [16.6, 33.3, ...]. We select 16.6.
    std::vector<FramePacer::HardwareTimeline> candidates0 = {
        {start + period60Hz, start + period60Hz - std::chrono::milliseconds(4)},
        {start + (period60Hz * 2), start + (period60Hz * 2) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick0;
    tick0.baseTime = start;
    tick0.frameScheduleTime = start;
    tick0.vsyncPeriod = period60Hz;
    tick0.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates0.data(), candidates0.size());

    EXPECT_EQ(pacer->setupFrame(tick0), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + period60Hz);

    // Callback 1 arrives at t=33.3 (it was so delayed it skipped a whole vsync period of 16.6).
    // The list of timelines is [50.0, 66.6, ...].
    std::vector<FramePacer::HardwareTimeline> candidates1 = {
        {start + (period60Hz * 3), start + (period60Hz * 3) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 4), start + (period60Hz * 4) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick1;
    tick1.baseTime = start + (period60Hz * 2);
    tick1.frameScheduleTime = start + (period60Hz * 2);
    tick1.vsyncPeriod = period60Hz;
    tick1.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates1.data(), candidates1.size());

    EXPECT_EQ(pacer->setupFrame(tick1), FramePacer::FrameStatus::ACCEPTED);
    // Because of syncExpectedBaseTimeWithVsync + snapOffset, we instantly recover:
    // our presentation time matches the exact first entry in the timeline (50.0ms / 3 periods)
    // right on schedule at 1-frame latency (STEADY, not STUFFED nor STARVING).
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 3));

    // Callback 2 arrives at t=50.0 right on time.
    // Verify that we do not lag behind and smoothly pick up the very next entry (66.6ms / 4 periods).
    std::vector<FramePacer::HardwareTimeline> candidates2 = {
        {start + (period60Hz * 4), start + (period60Hz * 4) - std::chrono::milliseconds(4)},
        {start + (period60Hz * 5), start + (period60Hz * 5) - std::chrono::milliseconds(4)}
    };
    FramePacer::VsyncTick tick2;
    tick2.baseTime = start + (period60Hz * 3);
    tick2.frameScheduleTime = start + (period60Hz * 3);
    tick2.vsyncPeriod = period60Hz;
    tick2.timelines = utils::Slice<const FramePacer::HardwareTimeline>(candidates2.data(), candidates2.size());

    EXPECT_EQ(pacer->setupFrame(tick2), FramePacer::FrameStatus::ACCEPTED);
    EXPECT_EQ(pacer->getPacingStatus(), FramePacer::PacingStatus::STEADY);
    EXPECT_EQ(pacer->getExpectedPresentationTime(), start + (period60Hz * 4));

    mEngine->destroy(pacer);
}
