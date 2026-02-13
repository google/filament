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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "details/UboManager.h"
#include "private/backend/CommandBufferQueue.h"

#include "private/backend/CommandStream.h"
#include "private/backend/Driver.h"

#include <unordered_set>

namespace {

using namespace filament;
using namespace backend;

using ::testing::_;
using ::testing::Return;

static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

class FenceManagerTest : public ::testing::Test {
protected:
    FenceManagerTest()
            : mCommandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE, CONFIG_COMMAND_BUFFERS_SIZE,
                      /*mPaused=*/false),
              mCommandStream(mMockDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream) {}

    MockDriver mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    UboManager::FenceManager mFenceManager;
};

TEST_F(FenceManagerTest, TrackEmptySet) {
    mFenceManager.track(mDriverApi, {});
    EXPECT_EQ(mMockDriver.nextFakeHandle, 1); // No fence allocation
}

TEST_F(FenceManagerTest, TrackNonEmptySet) {
    mFenceManager.track(mDriverApi, { 1, 2, 3 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);
}

TEST_F(FenceManagerTest, ReclaimWithNoFences) {
    std::vector<UboManager::FenceManager::AllocationId> reclaimedIds;
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.push_back(id); });

    EXPECT_TRUE(reclaimedIds.empty());
}

TEST_F(FenceManagerTest, ReclaimWhenFenceNotSignaled) {
    mFenceManager.track(mDriverApi, { 10 });

    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));

    std::unordered_set<UboManager::FenceManager::AllocationId> reclaimedIds;
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.insert(id); });

    EXPECT_TRUE(reclaimedIds.empty());
}

TEST_F(FenceManagerTest, ReclaimWhenFenceSignaled) {
    mFenceManager.track(mDriverApi, { 10, 20 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);

    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));

    std::unordered_set<UboManager::FenceManager::AllocationId> reclaimedIds;
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.insert(id); });

    // Verify that the correct IDs were reclaimed.
    ASSERT_EQ(reclaimedIds.size(), 2);
    EXPECT_TRUE(reclaimedIds.contains(10));
    EXPECT_TRUE(reclaimedIds.contains(20));

    // Verify that the fence is no longer tracked by calling reclaim again.
    reclaimedIds.clear();
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).Times(0); // No fences left to check.
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.insert(id); });
    EXPECT_TRUE(reclaimedIds.empty());
}

TEST_F(FenceManagerTest, ReclaimMultipleFencesPartial) {
    // Frame 1
    mFenceManager.track(mDriverApi, { 1 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);

    // Frame 2
    mFenceManager.track(mDriverApi, { 2 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 3);

    // Frame 3
    mFenceManager.track(mDriverApi, { 3 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 4);

    // Now, reclaim. Assume fence 1 and 2 are done, but 3 is not.
    // The implementation iterates from newest to oldest.
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(3)))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(2)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    EXPECT_CALL(mMockDriver, getFenceStatus(Handle<HwFence>(1)))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));

    std::vector<UboManager::FenceManager::AllocationId> reclaimedIds;
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.push_back(id); });

    // Verify that resources from the first two frames were reclaimed.
    ASSERT_EQ(reclaimedIds.size(), 2);
    EXPECT_EQ(reclaimedIds[0], 1);
    EXPECT_EQ(reclaimedIds[1], 2);
}

TEST_F(FenceManagerTest, Reset) {
    mFenceManager.track(mDriverApi, { 10 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);

    mFenceManager.track(mDriverApi, { 20 });
    EXPECT_EQ(mMockDriver.nextFakeHandle, 3);

    mFenceManager.reset(mDriverApi);

    // After reset, reclaiming should do nothing.
    std::vector<UboManager::FenceManager::AllocationId> reclaimedIds;
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).Times(0);
    mFenceManager.reclaimCompletedResources(mDriverApi,
            [&reclaimedIds](BufferAllocator::AllocationId id) { reclaimedIds.push_back(id); });
    EXPECT_TRUE(reclaimedIds.empty());
}

} // namespace anonymous
