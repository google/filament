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
#include <initializer_list>
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
    static constexpr size_t DEFAULT_MAX_UNIFORM_BUFFER_SIZE = 64 * 1024;
    static constexpr float BUFFER_SIZE_GROWTH_MULTIPLIER =
            UboManager::BUFFER_SIZE_GROWTH_MULTIPLIER;

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

        // Reallocation clamps to this limit; gmock's default of 0 would make every buffer empty.
        ON_CALL(mMockDriver, getMaxUniformBufferSize())
                .WillByDefault(Return(DEFAULT_MAX_UNIFORM_BUFFER_SIZE));
    }

    FMaterialInstance* createInstance() {
        auto* mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
        mTestInstances.push_back(mi);
        return mi;
    }

    size_t retiredCount() const { return mUboManager.mRetiredAllocations.size(); }

    // beginFrame() retires a prefix of mRetiredAllocations, which requires it to be sorted.
    bool retiredSerialsSorted() const {
        const std::vector<UboManager::RetiredAllocation>& retired =
                mUboManager.mRetiredAllocations;
        return std::is_sorted(retired.begin(), retired.end(),
                [](const UboManager::RetiredAllocation& lhs,
                        const UboManager::RetiredAllocation& rhs) {
                    return lhs.serial < rhs.serial;
                });
    }

    UboManager::FenceManager::Serial oldestRetiredSerial() const {
        return mUboManager.mRetiredAllocations.front().serial;
    }

    UboManager::FenceManager::Serial newestRetiredSerial() const {
        return mUboManager.mRetiredAllocations.back().serial;
    }

    // Bypasses deferRetirement() to simulate a corrupted mRetiredAllocations.
    void pushRetiredUnchecked(AllocationId id, UboManager::FenceManager::Serial serial) {
        mUboManager.mRetiredAllocations.push_back({ id, serial });
    }

    void clearRetired() { mUboManager.mRetiredAllocations.clear(); }

    void deferRetirement(AllocationId id) { mUboManager.deferRetirement(id); }

    // Runs beginFrame()/finishBeginFrame() (but not endFrame()) with `dirty` instances orphaned,
    // then marks every managed instance clean so that later frames only orphan what they dirty.
    void beginFrameDirtying(std::initializer_list<FMaterialInstance*> dirty) {
        for (FMaterialInstance* mi : dirty) {
            mi->getUniformBuffer().invalidate();
        }
        mUboManager.beginFrame(mDriverApi);
        for (FMaterialInstance* mi : mManagedInstances) {
            mi->getUniformBuffer().clean();
        }
        mUboManager.finishBeginFrame(mDriverApi);
    }

    void TearDown() override {
        for (FMaterialInstance* mi : mTestInstances) {
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
    EXPECT_TRUE(mFenceManager.isPending());
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
    for (FMaterialInstance* mi : instances) {
        EXPECT_TRUE(contains(mManagedInstances, mi));
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
    }

    // The new buffer is mapped for writes, just like the non-reallocating path.
    EXPECT_NE(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);

    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, ManagingPendingInstanceTwiceAsserts) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_EQ(mPendingInstances.size(), 1);
    EXPECT_DEBUG_DEATH(mUboManager.manageMaterialInstance(mi1), "failed assertion");
}

TEST_F(UboManagerTest, ManagingManagedInstanceAgainAsserts) {
    FMaterialInstance* mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);
    beginFrameDirtying({});
    ASSERT_TRUE(contains(mManagedInstances, mi1));
    EXPECT_DEBUG_DEATH(mUboManager.manageMaterialInstance(mi1), "mManagedInstances");
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
    // One instance is updated every frame while four stay stable, with the GPU never catching up.
    // Tracking cost must scale with the number of changes, not the number of allocations:
    // one fence per frame, one retired entry per orphaned slot, and stable slots are untouched.
    ON_CALL(mMockDriver, getFenceStatus(_)).WillByDefault(Return(FenceStatus::TIMEOUT_EXPIRED));
    FMaterialInstance* dynamic = createInstance();
    mUboManager.manageMaterialInstance(dynamic);
    std::vector<FMaterialInstance*> stable;
    for (size_t i = 0; i < 4; ++i) {
        FMaterialInstance* mi = createInstance();
        stable.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    // Frame 1: Initial allocations.
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi);
    std::vector<AllocationId> stableIds;
    for (FMaterialInstance* mi : stable) {
        stableIds.push_back(mi->getAllocationId());
    }
    AllocationId previousDynamicId = dynamic->getAllocationId();
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_EQ(mMockDriver.createdFences.size(), 1);

    // Frames 2-4: Only the dynamic instance is dirtied.
    for (size_t frame = 2; frame <= 4; ++frame) {
        beginFrameDirtying({ dynamic });
        mUboManager.endFrame(mDriverApi);

        // The dirty instance is orphaned onto a new slot; only its old slot is retired.
        EXPECT_NE(dynamic->getAllocationId(), previousDynamicId);
        previousDynamicId = dynamic->getAllocationId();
        EXPECT_EQ(retiredCount(), frame - 1);

        // Stable instances keep their slots and add no tracking state.
        for (size_t i = 0; i < stable.size(); ++i) {
            EXPECT_EQ(stable[i]->getAllocationId(), stableIds[i]);
        }

        // A single fence covers the whole frame regardless of how many allocations it reads.
        EXPECT_EQ(mMockDriver.createdFences.size(), frame);
    }
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 4);
}

TEST_F(UboManagerTest, FrameWithoutUboSlotsInUseCreatesNoFence) {
    mUboManager.beginFrame(mDriverApi);
    // An instance managed after beginFrame() stays pending and owns no slot until the next
    // beginFrame(), so nothing in this frame reads the UBO and endFrame() may skip the fence.
    FMaterialInstance* late = createInstance();
    mUboManager.manageMaterialInstance(late);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
    EXPECT_EQ(late->getAllocationId(), BufferAllocator::UNALLOCATED);
    EXPECT_TRUE(mMockDriver.createdFences.empty());
    EXPECT_FALSE(mFenceManager.isPending());
}

TEST_F(UboManagerTest, DestroyDuringFrameWaitsForCurrentFence) {
    // Frame 1: Allocate a slot for mi, then unmanage it before endFrame.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    beginFrameDirtying({});
    const AllocationId oldId = mi->getAllocationId();
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
    beginFrameDirtying({});
    EXPECT_NE(replacement->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 1);
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Once the Frame 1 fence signals, oldId is reclaimed and reused.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(fence))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* reused = createInstance();
    mUboManager.manageMaterialInstance(reused);
    beginFrameDirtying({});
    EXPECT_EQ(reused->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 0);
}

TEST_F(UboManagerTest, DestroyAfterFenceSignaledRetiresImmediately) {
    // Frame 1: Allocate a slot for mi and fence the frame.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    beginFrameDirtying({});
    const AllocationId oldId = mi->getAllocationId();
    mUboManager.endFrame(mDriverApi);
    ASSERT_EQ(mMockDriver.createdFences.size(), 1);

    // Destroy mi between frames; oldId goes into mFreedAllocations.
    mUboManager.unmanageMaterialInstance(mi);

    // Frame 2: Frame 1's fence has signaled, so nothing is in flight when beginFrame() drains
    // mFreedAllocations. deferRetirement() must take the immediate-retire branch: no entry is
    // queued, and oldId is free for the new instance allocated in the same beginFrame().
    // (If it were queued instead, the prefix scan has already run and oldId would stay locked.)
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* replacement = createInstance();
    mUboManager.manageMaterialInstance(replacement);
    beginFrameDirtying({});
    EXPECT_FALSE(mFenceManager.isPending());
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_EQ(replacement->getAllocationId(), oldId);
}

TEST_F(UboManagerTest, RetirementWaitsForLastFrameUsingAllocation) {
    // Frame 1: Allocate a slot for mi.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    beginFrameDirtying({});
    const AllocationId oldId = mi->getAllocationId();
    mUboManager.endFrame(mDriverApi);
    const Handle<HwFence> firstFence = mMockDriver.createdFences[0];

    // Frame 2: Keep using mi in a second frame, then unmanage it after endFrame.
    EXPECT_CALL(mMockDriver, getFenceStatus(firstFence))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    beginFrameDirtying({});
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
    beginFrameDirtying({});
    EXPECT_NE(replacement->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 1);
    mUboManager.endFrame(mDriverApi);

    // Frame 4: Once lastFence signals, oldId is reclaimed and reused.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(lastFence))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* reused = createInstance();
    mUboManager.manageMaterialInstance(reused);
    beginFrameDirtying({});
    EXPECT_EQ(reused->getAllocationId(), oldId);
    EXPECT_EQ(retiredCount(), 0);
}

TEST_F(UboManagerTest, PartialFenceCompletionReclaimsOnlyCoveredOrphans) {
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    ON_CALL(mMockDriver, getFenceStatus(_)).WillByDefault(Return(FenceStatus::TIMEOUT_EXPIRED));

    // Frames 1-3: Dirty mi each frame while all fences remain unsignaled. Each frame after the
    // first orphans mi onto a fresh slot, so ids[0] and ids[1] are retired and ids[2] stays active.
    AllocationId ids[3];
    for (size_t frame = 0; frame < 3; ++frame) {
        beginFrameDirtying({ mi });
        ids[frame] = mi->getAllocationId();
        if (frame > 0) {
            EXPECT_NE(ids[frame], ids[frame - 1]);
        }
        mUboManager.endFrame(mDriverApi);
    }
    ASSERT_EQ(retiredCount(), 2);

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
    beginFrameDirtying({});
    EXPECT_EQ(firstReuse->getAllocationId(), ids[0]);
    EXPECT_EQ(retiredCount(), 1);
    mUboManager.endFrame(mDriverApi);

    // Frame 5: Frame 3's fence (createdFences[2]) signals, covering Frame 2 as well.
    // ids[1] is now reclaimed and reused, while mi keeps its active allocation (ids[2]).
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[3]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[2]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* secondReuse = createInstance();
    mUboManager.manageMaterialInstance(secondReuse);
    beginFrameDirtying({});
    EXPECT_EQ(secondReuse->getAllocationId(), ids[1]);
    EXPECT_EQ(mi->getAllocationId(), ids[2]);
    EXPECT_EQ(retiredCount(), 0);
}

TEST_F(UboManagerTest, CompletedFrameAllowsDirtyInstanceToUpdateInPlace) {
    // Frame 1: Allocate a slot for mi.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    beginFrameDirtying({});
    const AllocationId id = mi->getAllocationId();
    mUboManager.endFrame(mDriverApi);

    // Frame 2: Frame 1's fence has already signaled, so no GPU reads are pending.
    // Marking mi dirty should update its existing slot in-place without orphaning.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    beginFrameDirtying({ mi });
    EXPECT_EQ(mi->getAllocationId(), id);
    EXPECT_EQ(retiredCount(), 0);
}

TEST_F(UboManagerTest, NewInstanceIsNotOrphanedByOlderFrames) {
    // Frame 1: Submit a frame with the first instance.
    FMaterialInstance* first = createInstance();
    mUboManager.manageMaterialInstance(first);
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi);

    // Frame 2: While Frame 1 is still in flight on the GPU, manage a newly created (dirty)
    // instance. Because it was never used by Frame 1, it only needs initial allocation and must
    // not be orphaned in the same beginFrame pass.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    FMaterialInstance* second = createInstance();
    mUboManager.manageMaterialInstance(second);
    beginFrameDirtying({ second });
    EXPECT_TRUE(BufferAllocator::isValid(second->getAllocationId()));
    EXPECT_EQ(retiredCount(), 0);
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

TEST_F(UboManagerTest, ReallocationDiscardsPendingRetirements) {
    // Frame 1: victim and dynamic get slots.
    FMaterialInstance* victim = createInstance();
    FMaterialInstance* dynamic = createInstance();
    mUboManager.manageMaterialInstance(victim);
    mUboManager.manageMaterialInstance(dynamic);
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi);

    // Frame 2 (Frame 1 still in flight): destroy victim and dirty dynamic, so beginFrame() queues
    // both of their old slots in mRetiredAllocations, via the mFreedAllocations drain and via
    // orphaning respectively. Enough new instances are added to exhaust the buffer in Pass 1, so
    // dynamic's orphan also takes the REALLOCATION_REQUIRED shortcut, and the UBO is reallocated.
    // Reallocation must drop the old generation's retirements: their ids now belong to live
    // instances in the new buffer.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.unmanageMaterialInstance(victim);
    constexpr size_t NUM_NEW = DEFAULT_TOTAL_SIZE / DEFAULT_SLOT_SIZE;
    for (size_t i = 0; i < NUM_NEW; ++i) {
        mUboManager.manageMaterialInstance(createInstance());
    }
    const allocation_size_t sizeBeforeReallocation = mUboManager.getTotalSize();
    beginFrameDirtying({ dynamic });
    ASSERT_GT(mUboManager.getTotalSize(), sizeBeforeReallocation);
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_FALSE(mFenceManager.isPending());
    std::vector<AllocationId> liveIds;
    for (FMaterialInstance* mi : mManagedInstances) {
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
        liveIds.push_back(mi->getAllocationId());
    }
    mUboManager.endFrame(mDriverApi);

    // Frame 3: The new generation's first fence signals. Had the stale entries survived, their ids
    // would be retired here and handed out again while still owned by live instances.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    FMaterialInstance* extra = createInstance();
    mUboManager.manageMaterialInstance(extra);
    beginFrameDirtying({});
    EXPECT_FALSE(contains(liveIds, extra->getAllocationId()));
    for (size_t i = 0; i < liveIds.size(); ++i) {
        EXPECT_EQ(mManagedInstances[i]->getAllocationId(), liveIds[i]);
    }
}

TEST_F(UboManagerTest, OrphanThatDoesNotFitTriggersReallocation) {
    // Frame 1: Exactly fill the buffer.
    const allocation_size_t instanceSize =
            mAllocator.alignUp(createInstance()->getUniformBuffer().getSize());
    ASSERT_EQ(DEFAULT_TOTAL_SIZE % instanceSize, 0u);
    const size_t numInstances = DEFAULT_TOTAL_SIZE / instanceSize;
    std::vector<FMaterialInstance*> instances;
    for (size_t i = 0; i < numInstances; ++i) {
        FMaterialInstance* mi = createInstance();
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi);
    ASSERT_EQ(mUboManager.getTotalSize(), DEFAULT_TOTAL_SIZE);
    const Handle<HwBufferObject> originalBufferHandle = mUbHandle;

    // Frame 2 (Frame 1 still in flight): dirty one instance. Pass 1 has nothing to do, but the
    // orphan in Pass 2 needs a second slot and the buffer is full, so beginFrame() must reallocate.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences[0]))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    instances[0]->getUniformBuffer().invalidate();
    mUboManager.beginFrame(mDriverApi);
    EXPECT_GT(mUboManager.getTotalSize(), DEFAULT_TOTAL_SIZE);
    EXPECT_NE(mUbHandle.getId(), originalBufferHandle.getId());
    EXPECT_NE(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_FALSE(mFenceManager.isPending());
    for (FMaterialInstance* mi : instances) {
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
        // Every instance moved to the new buffer and must be rewritten, even if it was clean.
        EXPECT_TRUE(mi->getUniformBuffer().isDirty());
    }
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, ReallocationSizeGrowsByMultiplier) {
    // One more instance than fits. The first numFitting get a slot in Pass 1; the last one fails.
    const allocation_size_t instanceSize =
            mAllocator.alignUp(createInstance()->getUniformBuffer().getSize());
    const size_t numFitting = DEFAULT_TOTAL_SIZE / instanceSize;
    for (size_t i = 0; i < numFitting + 1; ++i) {
        mUboManager.manageMaterialInstance(createInstance());
    }
    beginFrameDirtying({});

    // calculateRequiredSize() reserves two slots for the instance that failed allocation.
    const allocation_size_t expected = mAllocator.alignUp(
            (numFitting * instanceSize + 2 * instanceSize) * BUFFER_SIZE_GROWTH_MULTIPLIER);
    ASSERT_LT(expected, DEFAULT_MAX_UNIFORM_BUFFER_SIZE);
    EXPECT_EQ(mUboManager.getTotalSize(), expected);
}

TEST_F(UboManagerTest, ReallocationSizeIsClampedToMaxUniformBufferSize) {
    const allocation_size_t instanceSize =
            mAllocator.alignUp(createInstance()->getUniformBuffer().getSize());
    const size_t numInstances = DEFAULT_TOTAL_SIZE / instanceSize + 1;
    for (size_t i = 0; i < numInstances; ++i) {
        mUboManager.manageMaterialInstance(createInstance());
    }

    // The limit holds every instance, but is below the size the growth policy asks for.
    const allocation_size_t limit = numInstances * instanceSize;
    const allocation_size_t unclamped = mAllocator.alignUp(
            (numInstances + 1) * instanceSize * BUFFER_SIZE_GROWTH_MULTIPLIER);
    ASSERT_LT(limit, unclamped);
    EXPECT_CALL(mMockDriver, getMaxUniformBufferSize()).WillOnce(Return(limit));

    beginFrameDirtying({});
    EXPECT_EQ(mUboManager.getTotalSize(), limit);
    for (FMaterialInstance* mi : mManagedInstances) {
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
    }
}

TEST_F(UboManagerTest, ReallocationClampRoundsUnalignedLimitDown) {
    const allocation_size_t instanceSize =
            mAllocator.alignUp(createInstance()->getUniformBuffer().getSize());
    const size_t numInstances = DEFAULT_TOTAL_SIZE / instanceSize + 1;
    for (size_t i = 0; i < numInstances; ++i) {
        mUboManager.manageMaterialInstance(createInstance());
    }

    // A device limit that is not a multiple of the slot size must be rounded down to one.
    const allocation_size_t alignedLimit = numInstances * instanceSize;
    EXPECT_CALL(mMockDriver, getMaxUniformBufferSize())
            .WillOnce(Return(alignedLimit + DEFAULT_SLOT_SIZE / 2));

    beginFrameDirtying({});
    EXPECT_EQ(mUboManager.getTotalSize(), alignedLimit);
    for (FMaterialInstance* mi : mManagedInstances) {
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
    }
}

TEST_F(UboManagerTest, RetiredAllocationsStaySortedBySerial) {
    // Keep every fence pending so nothing is reclaimed and all retirements accumulate.
    ON_CALL(mMockDriver, getFenceStatus(_)).WillByDefault(Return(FenceStatus::TIMEOUT_EXPIRED));

    FMaterialInstance* a = createInstance();
    FMaterialInstance* b = createInstance();
    FMaterialInstance* c = createInstance();
    FMaterialInstance* d = createInstance();
    for (FMaterialInstance* mi : { a, b, c, d }) {
        mUboManager.manageMaterialInstance(mi);
    }

    // Frame 1: Initial allocations only.
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi); // serial 1
    EXPECT_EQ(retiredCount(), 0);

    // Frame 2: Orphan a, then destroy b mid-frame (deferred through mFreedAllocations).
    beginFrameDirtying({ a });
    mUboManager.unmanageMaterialInstance(b);
    mUboManager.endFrame(mDriverApi); // serial 2
    EXPECT_EQ(retiredCount(), 1);
    EXPECT_TRUE(retiredSerialsSorted());

    // Frame 3: b's deferred retirement and a's orphan both land at serial 2.
    // c is unmanaged after endFrame.
    beginFrameDirtying({ a });
    mUboManager.endFrame(mDriverApi); // serial 3
    mUboManager.unmanageMaterialInstance(c);
    EXPECT_EQ(retiredCount(), 3);
    EXPECT_TRUE(retiredSerialsSorted());

    // Frame 4: c's deferred retirement plus two orphans at serial 3.
    beginFrameDirtying({ a, d });
    mUboManager.endFrame(mDriverApi); // serial 4
    ASSERT_EQ(retiredCount(), 6);
    EXPECT_TRUE(retiredSerialsSorted());
    EXPECT_LT(oldestRetiredSerial(), newestRetiredSerial());

    // Frame 5: 2 live + 6 retired slots are held, so 9 new instances force a reallocation, which
    // restarts the serial counter. a is also orphaned before the reallocation happens, so the
    // stale entries (old serials) must be discarded rather than mixed with new ones.
    constexpr size_t NUM_NEW = (DEFAULT_TOTAL_SIZE / DEFAULT_SLOT_SIZE) / 2 + 1;
    std::vector<FMaterialInstance*> newInstances;
    newInstances.reserve(NUM_NEW);
    for (size_t i = 0; i < NUM_NEW; ++i) {
        FMaterialInstance* mi = createInstance();
        newInstances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }
    const allocation_size_t sizeBeforeReallocation = mUboManager.getTotalSize();
    beginFrameDirtying({ a });
    ASSERT_GT(mUboManager.getTotalSize(), sizeBeforeReallocation);
    EXPECT_EQ(retiredCount(), 0);
    EXPECT_EQ(mFenceManager.getSubmittedSerial(), 0);
    mUboManager.endFrame(mDriverApi); // serial 1 (new generation)

    // Frame 6: Retirements restart from the low serials of the new generation.
    mUboManager.unmanageMaterialInstance(newInstances[0]);
    beginFrameDirtying({ a });
    mUboManager.endFrame(mDriverApi); // serial 2
    EXPECT_EQ(retiredCount(), 2);
    EXPECT_TRUE(retiredSerialsSorted());

    // Frame 7: One more orphan at a later serial.
    beginFrameDirtying({ d });
    mUboManager.endFrame(mDriverApi); // serial 3
    ASSERT_EQ(retiredCount(), 3);
    EXPECT_TRUE(retiredSerialsSorted());
    EXPECT_LT(oldestRetiredSerial(), newestRetiredSerial());

    // Frame 8: The newest fence signals, covering all earlier submissions. Because the list is
    // sorted, the prefix scan in beginFrame() must drain every retired allocation.
    EXPECT_CALL(mMockDriver, getFenceStatus(mMockDriver.createdFences.back()))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED));
    beginFrameDirtying({});
    EXPECT_EQ(retiredCount(), 0);
}

TEST_F(UboManagerTest, OutOfOrderRetirementAsserts) {
    // Submit a frame so the GPU is considered busy and deferRetirement() queues the id.
    FMaterialInstance* mi = createInstance();
    mUboManager.manageMaterialInstance(mi);
    beginFrameDirtying({});
    mUboManager.endFrame(mDriverApi);
    ASSERT_TRUE(mFenceManager.isPending());

    // Simulate a corrupted list whose tail is newer than the serial deferRetirement() will stamp.
    pushRetiredUnchecked(mi->getAllocationId(), mFenceManager.getSubmittedSerial() + 1);
    EXPECT_DEBUG_DEATH(deferRetirement(mi->getAllocationId()),
            "failed assertion.*serial <= serial");
    clearRetired();
}
