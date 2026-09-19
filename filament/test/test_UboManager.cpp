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

#include "filament_test_resources.h"
#include "MockDriver.h"

#include "details/MaterialInstance.h"
#include "details/UboManager.h"

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>
#include <private/backend/Driver.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace {
using namespace filament;
using namespace backend;

using ::testing::_;
using ::testing::Return;

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;

class UboTestMockDriver : public MockDriver {
public:
    Handle<HwFence> createFenceS() noexcept override {
        Handle<HwFence> fence = MockDriver::createFenceS();
        createdFences.push_back(fence);
        return fence;
    }

    std::vector<Handle<HwFence>> createdFences;
};

} // anonymous namespace

class UboManagerTest : public ::testing::Test {
public:
    template<typename T>
    static bool contains(const std::vector<T>& v, const T& item) {
        return std::find(v.begin(), v.end(), item) != v.end();
    }

protected:
    static constexpr size_t MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
    static constexpr size_t COMMAND_BUFFERS_SIZE = 3 * MIN_COMMAND_BUFFERS_SIZE;
    static constexpr allocation_size_t DEFAULT_SLOT_SIZE = 64;
    static constexpr allocation_size_t DEFAULT_TOTAL_SIZE = 1024;

    UboManagerTest()
            : mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE, COMMAND_BUFFERS_SIZE, false),
              mCommandStream(mMockDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream),
              // The constructor will call reallocate, which calls createBufferObject.
              // MockDriver's default ...S() implementation returns an incrementing handle.
              // So, the first handle will be 1.
              mUboManager(mDriverApi, DEFAULT_SLOT_SIZE, DEFAULT_TOTAL_SIZE),
              mPendingInstances(mUboManager.mPendingInstances),
              mManagedInstances(mUboManager.mManagedInstances),
              mUbHandle(mUboManager.mUbHandle),
              mAllocator(mUboManager.mAllocator),
              mFenceManager(mUboManager.mFenceManager) {
        mEngine = Engine::Builder()
                .feature("material.enable_material_instance_uniform_batching", true)
                .backend(Backend::NOOP)
                .build();
        assert_invariant(mEngine);

        mMaterial = Material::Builder()
                .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA,
                        FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE)
                .build(*mEngine);
    }

    FMaterialInstance* createInstance() {
        auto* mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
        mTestInstances.push_back(mi);
        return mi;
    }

    size_t retiredCount() const { return mUboManager.mRetiredAllocations.size(); }

    void reallocate(allocation_size_t size) {
        mUboManager.reallocate(mDriverApi, size);
        mUboManager.allocateAllInstances();
    }

    void TearDown() override {
        for (auto* mi : mTestInstances) {
            mUboManager.unmanageMaterialInstance(mi);
            mEngine->destroy(mi);
        }
        mUboManager.terminate(mDriverApi);
        mEngine->destroy(mMaterial);
        Engine::destroy(&mEngine);
    }

    // The engine is only for creating materials/material instances, we're not using the UboManager
    // inside for testing.
    Engine* mEngine = nullptr;
    UboTestMockDriver mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    UboManager mUboManager;
    Material const* mMaterial;
    std::vector<FMaterialInstance*> mTestInstances;
    std::vector<FMaterialInstance*>& mPendingInstances;
    std::vector<FMaterialInstance*>& mManagedInstances;
    Handle<HwBufferObject>& mUbHandle;
    BufferAllocator& mAllocator;
    UboManager::FenceManager& mFenceManager;
};

TEST_F(UboManagerTest, InitialState) {
    EXPECT_EQ(mUboManager.getTotalSize(), DEFAULT_TOTAL_SIZE);
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);
    EXPECT_NE(mUbHandle.getId(), HandleBase::nullid);
}

TEST_F(UboManagerTest, BeginFrameWithoutReallocate) {
    const allocation_size_t originalBufferSize = mUboManager.getTotalSize();
    FMaterialInstance* mi1 = createInstance();
    EXPECT_EQ(mi1->getAllocationId(), BufferAllocator::UNALLOCATED);
    ASSERT_TRUE(mi1->isUsingUboBatching());

    // The mi1 should be put in the pending list.
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_TRUE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));

    mUboManager.beginFrame(mDriverApi);

    // The mi1 should be moved to managed list after beginFrame.
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_TRUE(contains(mManagedInstances, mi1));
    // Should have allocation after beginFrame.
    EXPECT_TRUE(BufferAllocator::isValid(mi1->getAllocationId()));

    // Reallocation is not triggered under this case.
    EXPECT_EQ(mUboManager.getTotalSize(), originalBufferSize);
    EXPECT_NE(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);

    mUboManager.finishBeginFrame(mDriverApi);
    EXPECT_EQ(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);

    mUboManager.endFrame(mDriverApi);
    EXPECT_GT(mFenceManager.getSubmittedSerial(), mFenceManager.getCompletedSerial());
}

TEST_F(UboManagerTest, BeginFrameWithReallocate) {
    const allocation_size_t originalBufferSize = mUboManager.getTotalSize();
    const Handle<HwBufferObject> originalBufferHandle = mUbHandle;

    // Create enough material instances to trigger a reallocation.
    constexpr size_t NUM_INSTANCES = (DEFAULT_TOTAL_SIZE / DEFAULT_SLOT_SIZE) + 1;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(NUM_INSTANCES);

    for (size_t i = 0; i < NUM_INSTANCES; ++i) {
        FMaterialInstance* mi = createInstance();
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    // Before beginFrame, all instances should be pending.
    EXPECT_EQ(mPendingInstances.size(), NUM_INSTANCES);
    EXPECT_TRUE(mManagedInstances.empty());

    mUboManager.beginFrame(mDriverApi);

    // After beginFrame, reallocation should have occurred.
    EXPECT_NE(mUbHandle.getId(), originalBufferHandle.getId());
    EXPECT_GT(mUboManager.getTotalSize(), originalBufferSize);

    // All instances should now be managed and have valid allocations.
    EXPECT_TRUE(mPendingInstances.empty());
    EXPECT_EQ(mManagedInstances.size(), NUM_INSTANCES);
    for (auto* mi : instances) {
        EXPECT_TRUE(contains(mManagedInstances, mi));
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
    }

    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, RecycleSlot) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    // Frame 1: mi1 gets an allocation.
    mUboManager.beginFrame(mDriverApi);
    const AllocationId mi1AllocationId = mi1->getAllocationId();
    const allocation_size_t mi1AllocationOffset =
            mAllocator.getAllocationOffset(mi1AllocationId);
    EXPECT_TRUE(BufferAllocator::isValid(mi1AllocationId));
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Fences reads of mi1.

    // Now, unmanage mi1. The slot should be retired but not yet released.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_EQ(mFenceManager.getCompletedSerial(), 0);

    ASSERT_FALSE(mMockDriver.createdFences.empty());
    const Handle<HwFence> fenceFrame1 = mMockDriver.createdFences[0];

    // Frame 2: The slot for mi1 is still locked by the GPU.
    // We expect getFenceStatus to be called for the fence from frame 1.
    // We'll mock it to return TIMEOUT_EXPIRED, so the resource is not reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Now, we'll simulate that the fence from frame 1 has signaled.
    // The resource for mi1 should be reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));

    FMaterialInstance* mi2 = createInstance();
    mUboManager.manageMaterialInstance(mi2);

    mUboManager.beginFrame(mDriverApi);

    // mi2 should now have a valid allocation, and it should reuse the slot from mi1.
    EXPECT_TRUE(BufferAllocator::isValid(mi2->getAllocationId()));
    EXPECT_EQ(mAllocator.getAllocationOffset(mi2->getAllocationId()), mi1AllocationOffset);

    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, OrphanSlot) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    // Frame 1: mi1 gets an allocation.
    mUboManager.beginFrame(mDriverApi);
    const AllocationId alloc1 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc1));
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Fences reads of alloc1.

    ASSERT_FALSE(mMockDriver.createdFences.empty());
    const Handle<HwFence> fenceFrame1 = mMockDriver.createdFences[0];

    // Frame 2: Mark the instance as dirty and begin a new frame.
    // This should trigger orphaning.
    mi1->getUniformBuffer().invalidate();
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);

    const AllocationId alloc2 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc2));
    EXPECT_NE(alloc1, alloc2); // Should have a new allocation.

    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Fences reads of alloc2.

    ASSERT_GE(mMockDriver.createdFences.size(), 2);
    const Handle<HwFence> fenceFrame2 = mMockDriver.createdFences[1];

    // Frame 3: The fence for alloc1 should now be signaled.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame2))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED)); // For alloc2's fence
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED)); // For alloc1's fence
    mUboManager.beginFrame(mDriverApi);
    // alloc1 is reusable even though the newer allocation is still in flight.
    EXPECT_EQ(mi1->getAllocationId(), alloc1);
    EXPECT_EQ(retiredCount(), 1);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, DoubleManage) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_EQ(mPendingInstances.size(), 1);
    EXPECT_DEATH(mUboManager.manageMaterialInstance(mi1), "");
}

TEST_F(UboManagerTest, ManageAndUnmanageBeforeBeginFrame) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_TRUE(contains(mPendingInstances, mi1));

    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));

    // After beginFrame, the instance should not be in any list.
    mUboManager.beginFrame(mDriverApi);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));
    EXPECT_EQ(mi1->getAllocationId(), BufferAllocator::UNALLOCATED);
}

TEST_F(UboManagerTest, UnmanageUnmanaged) {
    FMaterialInstance* mi1 = createInstance();

    // Unmanaging an instance that was never managed should not cause any issues.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));
}

TEST_F(UboManagerTest, StableInstancesNeedNoPerAllocationFenceTracking) {
    for (size_t i = 0; i < 5; ++i) {
        FMaterialInstance* mi = createInstance();
        mUboManager.manageMaterialInstance(mi);
    }
    ON_CALL(mMockDriver, getFenceStatus(_)).WillByDefault(Return(FenceStatus::TIMEOUT_EXPIRED));
    for (size_t frame = 0; frame < 3; ++frame) {
        mUboManager.beginFrame(mDriverApi);
        for (auto* mi : mManagedInstances) {
            mi->getUniformBuffer().clean();
        }
        mUboManager.finishBeginFrame(mDriverApi);
        mUboManager.endFrame(mDriverApi);
        EXPECT_EQ(retiredCount(), 0);
        for (auto* mi : mManagedInstances) {
            EXPECT_FALSE(mAllocator.isLockedByGpu(mi->getAllocationId()));
        }
    }
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 3);
}

TEST_F(UboManagerTest, EmptyFrameDoesNotCreateFence) {
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
    EXPECT_TRUE(mMockDriver.createdFences.empty());
}

TEST_F(UboManagerTest, DestroyDuringFrameWaitsForCurrentFence) {
    // Frame 1: Allocate a slot for mi, then unmanage it before endFrame.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    mUboManager.beginFrame(mDriverApi);
    const AllocationId oldId = mi->getAllocationId();
    mUboManager.finishBeginFrame(mDriverApi);
    // No managed instances remain, but the current frame may already reference this allocation.
    mUboManager.unmanageMaterialInstance(mi);
    mUboManager.endFrame(mDriverApi);
    ASSERT_EQ(mMockDriver.createdFences.size(), 1);
    const Handle<HwFence> fence = mMockDriver.createdFences[0];

    // Frame 2: The Frame 1 fence has not signaled yet, so oldId stays retired and cannot be reused.
    EXPECT_CALL(mMockDriver, getFenceStatus(fence))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    FMaterialInstance* replacement = createInstance();
    mUboManager.manageMaterialInstance(replacement);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_NE(replacement->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 1);
    replacement->getUniformBuffer().clean();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Once the Frame 1 fence signals, oldId is reclaimed and reused.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(fence))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* reused = createInstance();
    mUboManager.manageMaterialInstance(reused);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(reused->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 0);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, RetirementWaitsForLastFrameUsingAllocation) {
    // Frame 1: Allocate a slot for mi.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    mUboManager.beginFrame(mDriverApi);
    const AllocationId oldId = mi->getAllocationId();
    mi->getUniformBuffer().clean();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
    const Handle<HwFence> firstFence = mMockDriver.createdFences[0];

    // Frame 2: Keep using mi in a second frame, then unmanage it after endFrame.
    EXPECT_CALL(mMockDriver, getFenceStatus(firstFence))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
    const Handle<HwFence> lastFence = mMockDriver.createdFences[1];
    mUboManager.unmanageMaterialInstance(mi);

    // Frame 3: Frame 1's fence has signaled, but Frame 2's fence (lastFence) is still pending.
    // The slot must wait for lastFence before it can be reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(lastFence))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(firstFence))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* replacement = createInstance();
    mUboManager.manageMaterialInstance(replacement);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_NE(replacement->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 1);
    replacement->getUniformBuffer().clean();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 4: Once lastFence signals, oldId is reclaimed and reused.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(lastFence))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* reused = createInstance();
    mUboManager.manageMaterialInstance(reused);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(reused->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 0);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, ReclaimOnlyCompletedRetirementBatches) {
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    ON_CALL(mMockDriver, getFenceStatus(_)).WillByDefault(Return(FenceStatus::TIMEOUT_EXPIRED));

    // Frames 1-3: Dirty mi each frame while all fences remain unsignaled,
    // which orphans and retires ids[0] and ids[1] while ids[2] stays active.
    AllocationId ids[3];
    for (size_t frame = 0; frame < 3; ++frame) {
        mi->getUniformBuffer().invalidate();
        mUboManager.beginFrame(mDriverApi);
        ids[frame] = mi->getAllocationId();
        mUboManager.finishBeginFrame(mDriverApi);
        mUboManager.endFrame(mDriverApi);
    }
    ASSERT_EQ(retiredCount(), 2);
    mi->getUniformBuffer().clean();

    // Frame 4: Only Frame 1's fence (createdFences[0]) has signaled.
    // Only ids[0] should be reclaimed and reused; ids[1] remains retired.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[2]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[1]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* firstReuse = createInstance();
    mUboManager.manageMaterialInstance(firstReuse);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(firstReuse->getAllocationId(), ids[0]);
    EXPECT_EQ(retiredCount(), 1);
    firstReuse->getUniformBuffer().clean();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 5: Frame 3's fence (createdFences[2]) signals, covering Frame 2 as well.
    // ids[1] is now reclaimed and reused, while mi keeps its active allocation (ids[2]).
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[3]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[2]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* secondReuse = createInstance();
    mUboManager.manageMaterialInstance(secondReuse);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(secondReuse->getAllocationId(), ids[1]);
    EXPECT_EQ(mi->getAllocationId(), ids[2]);
    EXPECT_EQ(retiredCount(), 0);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, CompletedFrameAllowsDirtyInstanceToUpdateInPlace) {
    // Frame 1: Allocate a slot for mi.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    mUboManager.beginFrame(mDriverApi);
    const AllocationId id = mi->getAllocationId();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 2: Frame 1's fence has already signaled, so no GPU reads are pending.
    // Marking mi dirty should update its existing slot in-place without orphaning.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    mi->getUniformBuffer().invalidate();
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(mi->getAllocationId(), id);
    EXPECT_EQ(retiredCount(), 0);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, NewInstanceIsNotOrphanedByOlderFrames) {
    // Frame 1: Submit a frame with the first instance.
    FMaterialInstance* first = createInstance();
    mUboManager.manageMaterialInstance(first);
    mUboManager.beginFrame(mDriverApi);
    first->getUniformBuffer().clean();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 2: While Frame 1 is still in flight on the GPU, manage a newly created instance.
    // Because the new instance was never used by Frame 1, it only needs initial allocation
    // and must not be orphaned in the same beginFrame pass.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    FMaterialInstance* second = createInstance();
    mUboManager.manageMaterialInstance(second);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_TRUE(BufferAllocator::isValid(second->getAllocationId()));
    EXPECT_EQ(retiredCount(), 0);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, ReallocationDiscardsOldRetirementsBeforeIdReuse) {
    // Frame 1: Allocate oldId for mi.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    mUboManager.beginFrame(mDriverApi);
    const AllocationId oldId = mi->getAllocationId();
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 2: Dirty mi while Frame 1 is still in flight, retiring oldId.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mi->getUniformBuffer().invalidate();
    mUboManager.beginFrame(mDriverApi);
    EXPECT_EQ(retiredCount(), 1);
    mUboManager.finishBeginFrame(mDriverApi);

    // Reallocating the UBO resets the allocator and reassigns oldId to mi.
    // Stale retirements from the previous buffer generation must be cleared so that
    // oldId in the new buffer is not mistakenly retired when the next fence signals.
    reallocate(DEFAULT_TOTAL_SIZE * 2);
    ASSERT_EQ(mi->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 0);
    mi->getUniformBuffer().clean();
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Verify mi retains oldId and a newly added instance receives a distinct slot.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* second = createInstance();
    mUboManager.manageMaterialInstance(second);
    mUboManager.beginFrame(mDriverApi);
    EXPECT_NE(second->getAllocationId(), oldId);
    EXPECT_EQ(mi->getAllocationId(), oldId);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, UpdateSlot) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    // Begin frame maps the UBO buffer for CPU writes.
    mUboManager.beginFrame(mDriverApi);
    EXPECT_NE(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);

    // Copy uniform data into mi1's allocated slot while the buffer is mapped.
    char data[64] = {};
    BufferDescriptor desc(data, sizeof(data));
    mUboManager.updateSlot(mDriverApi, mi1->getAllocationId(), std::move(desc));

    // Unmap the buffer and fence the frame.
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
}
