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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockDriver.h"
#include "details/MaterialInstance.h"
#include "details/UboManager.h"

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>
#include <private/backend/Driver.h>

#include "filament_test_resources.h"

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
    static constexpr BufferAllocator::allocation_size_t DEFAULT_SLOT_SIZE = 64;
    static constexpr BufferAllocator::allocation_size_t DEFAULT_TOTAL_SIZE = 1024;

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
              mAllocator(mUboManager.mAllocator) {
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
        auto mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
        mTestInstances.push_back(mi);
        return mi;
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
};

TEST_F(UboManagerTest, InitialState) {
    EXPECT_EQ(mUboManager.getTotalSize(), DEFAULT_TOTAL_SIZE);
    EXPECT_EQ(mMockDriver.nextFakeHandle, 2);
    EXPECT_NE(mUbHandle.getId(), HandleBase::nullid);
}

TEST_F(UboManagerTest, BeginFrameWithoutReallocate) {
    BufferAllocator::allocation_size_t originalBufferSize = mUboManager.getTotalSize();
    auto mi1 = createInstance();
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
    EXPECT_TRUE(mAllocator.isLockedByGpu(mi1->getAllocationId()));
}

TEST_F(UboManagerTest, BeginFrameWithReallocate) {
    const allocation_size_t originalBufferSize = mUboManager.getTotalSize();
    const Handle<HwBufferObject> originalBufferHandle = mUbHandle;

    // Create enough material instances to trigger a reallocation.
    constexpr size_t numInstances = (DEFAULT_TOTAL_SIZE / DEFAULT_SLOT_SIZE) + 1;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = createInstance();
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    // Before beginFrame, all instances should be pending.
    EXPECT_EQ(mPendingInstances.size(), numInstances);
    EXPECT_TRUE(mManagedInstances.empty());

    mUboManager.beginFrame(mDriverApi);

    // After beginFrame, reallocation should have occurred.
    EXPECT_NE(mUbHandle.getId(), originalBufferHandle.getId());
    EXPECT_GT(mUboManager.getTotalSize(), originalBufferSize);

    // All instances should now be managed and have valid allocations.
    EXPECT_TRUE(mPendingInstances.empty());
    EXPECT_EQ(mManagedInstances.size(), numInstances);
    for (const auto* mi: instances) {
        EXPECT_TRUE(contains(mManagedInstances, const_cast<FMaterialInstance*>(mi)));
        EXPECT_TRUE(BufferAllocator::isValid(mi->getAllocationId()));
    }

    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, RecycleSlot) {
    auto mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    // Frame 1: mi1 gets an allocation.
    mUboManager.beginFrame(mDriverApi);
    const AllocationId mi1AllocationId = mi1->getAllocationId();
    const allocation_size_t mi1AllocationOffset =
            mAllocator.getAllocationOffset(mi1AllocationId);
    EXPECT_TRUE(BufferAllocator::isValid(mi1AllocationId));
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Locks mi1's allocation.

    // Now, unmanage mi1. The slot should be retired but not yet released.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_TRUE(mAllocator.isLockedByGpu(mi1AllocationId));

    ASSERT_FALSE(mMockDriver.createdFences.empty());
    const auto fenceFrame1 = mMockDriver.createdFences[0];

    // Frame 2: The slot for mi1 is still locked by the GPU.
    // We expect getFenceStatus to be called for the fence from frame 1.
    // We'll mock it to return TIMEOUT_EXPIRED, so the resource is not reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1)).WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Now, we'll simulate that the fence from frame 1 has signaled.
    // The resource for mi1 should be reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1)).WillOnce(Return(FenceStatus::CONDITION_SATISFIED));

    auto mi2 = createInstance();
    mUboManager.manageMaterialInstance(mi2);

    mUboManager.beginFrame(mDriverApi);

    // mi2 should now have a valid allocation, and it should reuse the slot from mi1.
    EXPECT_TRUE(BufferAllocator::isValid(mi2->getAllocationId()));
    EXPECT_EQ(mAllocator.getAllocationOffset(mi2->getAllocationId()), mi1AllocationOffset);

    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, OrphanSlot) {
    auto mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    // Frame 1: mi1 gets an allocation.
    mUboManager.beginFrame(mDriverApi);
    const AllocationId alloc1 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc1));
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Locks alloc1.

    ASSERT_FALSE(mMockDriver.createdFences.empty());
    const auto fenceFrame1 = mMockDriver.createdFences[0];

    // Frame 2: Mark the instance as dirty and begin a new frame.
    // This should trigger orphaning.
    mi1->getUniformBuffer().invalidate();
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1)).WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);

    const AllocationId alloc2 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc2));
    EXPECT_NE(alloc1, alloc2); // Should have a new allocation.

    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Locks alloc2.

    ASSERT_GE(mMockDriver.createdFences.size(), 2);
    const auto fenceFrame2 = mMockDriver.createdFences[1];

    // Frame 3: The fence for alloc1 should now be signaled.
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame2))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED)); // For alloc2's fence
    EXPECT_CALL(mMockDriver, getFenceStatus(fenceFrame1))
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED)); // For alloc1's fence
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
}

TEST_F(UboManagerTest, DoubleManage) {
    auto mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_EQ(mPendingInstances.size(), 1);
    EXPECT_DEATH(mUboManager.manageMaterialInstance(mi1), "");
}

TEST_F(UboManagerTest, ManageAndUnmanageBeforeBeginFrame) {
    auto mi1 = createInstance();
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
    auto mi1 = createInstance();

    // Unmanaging an instance that was never managed should not cause any issues.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));
}

TEST_F(UboManagerTest, AllAllocationsLockedAfterEndFrame) {
    constexpr size_t numInstances = 5;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = createInstance();
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    for (const auto* mi: instances) {
        EXPECT_TRUE(mAllocator.isLockedByGpu(mi->getAllocationId()));
    }
}

TEST_F(UboManagerTest, AllAllocationsLockedAfterEndFrameWithInvalidIdInBetween) {
    constexpr size_t numInstances = 5;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = createInstance();
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);

    // It should rarely happen, but we want to make sure all other instances are locked properly.
    instances[2]->assignUboAllocation(mUbHandle, BufferAllocator::REALLOCATION_REQUIRED, 0);
    mUboManager.endFrame(mDriverApi);

    for (const auto* mi: instances) {
        if (BufferAllocator::isValid(mi->getAllocationId())) {
            EXPECT_TRUE(mAllocator.isLockedByGpu(mi->getAllocationId()));
        }
    }
}

TEST_F(UboManagerTest, UpdateSlot) {
    auto mi1 = createInstance();
    mUboManager.manageMaterialInstance(mi1);

    mUboManager.beginFrame(mDriverApi);
    EXPECT_NE(mUboManager.getMemoryMappedBufferHandle().getId(), HandleBase::nullid);

    char data[64] = {};
    BufferDescriptor desc(data, sizeof(data));
    mUboManager.updateSlot(mDriverApi, mi1->getAllocationId(), std::move(desc));

    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);
}
