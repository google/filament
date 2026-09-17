/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "MockDriver.h"

#include "details/UboManager.h"

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>
#include <private/backend/Driver.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>

namespace {

using namespace filament;
using namespace backend;

using ::testing::_;
using ::testing::Return;

class FenceManagerTest : public ::testing::Test {
protected:
    static constexpr size_t MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
    static constexpr size_t COMMAND_BUFFERS_SIZE = 3 * MIN_COMMAND_BUFFERS_SIZE;

    FenceManagerTest()
            : mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE, COMMAND_BUFFERS_SIZE,
                      /*mPaused=*/false),
              mCommandStream(mMockDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream) {}

    MockDriver mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    UboManager::FenceManager mFenceManager;
};

TEST_F(FenceManagerTest, TrackFrame) {
    mFenceManager.track(mDriverApi);
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 1);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);
}

TEST_F(FenceManagerTest, ReclaimWithNoFences) {
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).Times(0);
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);
}

TEST_F(FenceManagerTest, ReclaimWhenFenceNotSignaled) {
    mFenceManager.track(mDriverApi);
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);
}

TEST_F(FenceManagerTest, ReclaimWhenFenceSignaled) {
    mFenceManager.track(mDriverApi);
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 1);

    EXPECT_CALL(mMockDriver, getFenceStatus(_)).Times(0);
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 1);
}

TEST_F(FenceManagerTest, ReclaimMultipleFencesPartial) {
    mFenceManager.track(mDriverApi);
    mFenceManager.track(mDriverApi);
    mFenceManager.track(mDriverApi);
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(3)))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(2)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    // A newer completed fence covers older submissions without querying them.
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1))).Times(0);
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 2);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 3);

    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(3)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 3);
}

TEST_F(FenceManagerTest, ErrorDoesNotCompleteFrame) {
    mFenceManager.track(mDriverApi);
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::ERROR));
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);
}

TEST_F(FenceManagerTest, Reset) {
    mFenceManager.track(mDriverApi);
    mFenceManager.track(mDriverApi);
    mFenceManager.reset(mDriverApi);
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).Times(0);
    mFenceManager.reclaimCompletedResources(mDriverApi);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 0);
    mFenceManager.track(mDriverApi);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 1);
}

} // anonymous namespace
