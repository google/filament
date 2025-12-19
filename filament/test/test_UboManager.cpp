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

using ::testing::NiceMock;

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;
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

        mMaterial = Material::Builder()
                            .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA,
                                    FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE)
                            .build(*mEngine);
    }

    void TearDown() override {
        mEngine->destroy(mMaterial);
        Engine::destroy(&mEngine);
    }

    // The engine is only for creating materials/material instances, we're not using the UboManager
    // inside for testing.
    Engine* mEngine = nullptr;
    NiceMock<MockDriver> mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    UboManager mUboManager;
    Material const* mMaterial;
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
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
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

    // We're not using the UboManager inside mEngine, so we need to unmanage the material instance
    // by ourselves.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));

    mUboManager.terminate(mDriverApi);
    mEngine->destroy(mi1);
}

TEST_F(UboManagerTest, BeginFrameWithReallocate) {
    const allocation_size_t originalBufferSize = mUboManager.getTotalSize();
    const Handle<HwBufferObject> originalBufferHandle = mUbHandle;

    // Create enough material instances to trigger a reallocation.
    constexpr size_t numInstances = (DEFAULT_TOTAL_SIZE / DEFAULT_SLOT_SIZE) + 1;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
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
    mUboManager.terminate(mDriverApi);

    for (auto* mi: instances) {
        // We're not using the UboManager inside mEngine, so we need to unmanage the material instance
        // by ourselves.
        mUboManager.unmanageMaterialInstance(mi);
        mEngine->destroy(mi);
    }
}

TEST_F(UboManagerTest, RecycleSlot) {
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
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

    // Frame 2: The slot for mi1 is still locked by the GPU.
    // We expect getFenceStatus to be called for the fence from frame 1.
    // We'll mock it to return TIMEOUT_EXPIRED, so the resource is not reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    // Frame 3: Now, we'll simulate that the fence from frame 1 has signaled.
    // The resource for mi1 should be reclaimed.
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).WillOnce(Return(FenceStatus::CONDITION_SATISFIED));

    auto mi2 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
    mUboManager.manageMaterialInstance(mi2);

    mUboManager.beginFrame(mDriverApi);

    // mi2 should now have a valid allocation, and it should reuse the slot from mi1.
    EXPECT_TRUE(BufferAllocator::isValid(mi2->getAllocationId()));
    EXPECT_EQ(mAllocator.getAllocationOffset(mi2->getAllocationId()), mi1AllocationOffset);

    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.unmanageMaterialInstance(mi2);
    mUboManager.terminate(mDriverApi);

    mEngine->destroy(mi1);
    mEngine->destroy(mi2);
}

TEST_F(UboManagerTest, OrphanSlot) {
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
    mUboManager.manageMaterialInstance(mi1);

    // Frame 1: mi1 gets an allocation.
    mUboManager.beginFrame(mDriverApi);
    const AllocationId alloc1 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc1));
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Locks alloc1.

    // Frame 2: Mark the instance as dirty and begin a new frame.
    // This should trigger orphaning.
    mi1->getUniformBuffer().invalidate();
    EXPECT_CALL(mMockDriver, getFenceStatus(_)).WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED));
    mUboManager.beginFrame(mDriverApi);

    const AllocationId alloc2 = mi1->getAllocationId();
    EXPECT_TRUE(BufferAllocator::isValid(alloc2));
    EXPECT_NE(alloc1, alloc2); // Should have a new allocation.

    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi); // Locks alloc2.

    // Frame 3: The fence for alloc1 should now be signaled.
    EXPECT_CALL(mMockDriver, getFenceStatus(_))
            .WillOnce(Return(FenceStatus::TIMEOUT_EXPIRED))      // For alloc2's fence
            .WillOnce(Return(FenceStatus::CONDITION_SATISFIED)); // For alloc1's fence
    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.unmanageMaterialInstance(mi1);
    mUboManager.terminate(mDriverApi);

    mEngine->destroy(mi1);
}

TEST_F(UboManagerTest, DoubleManage) {
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_EQ(mPendingInstances.size(), 1);
    EXPECT_DEATH(mUboManager.manageMaterialInstance(mi1), "");

    mUboManager.terminate(mDriverApi);
    mEngine->destroy(mi1);
}

TEST_F(UboManagerTest, ManageAndUnmanageBeforeBeginFrame) {
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());
    mUboManager.manageMaterialInstance(mi1);
    EXPECT_TRUE(contains(mPendingInstances, mi1));

    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));

    // After beginFrame, the instance should not be in any list.
    mUboManager.beginFrame(mDriverApi);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));
    EXPECT_EQ(mi1->getAllocationId(), BufferAllocator::UNALLOCATED);

    mUboManager.terminate(mDriverApi);
    mEngine->destroy(mi1);
}

TEST_F(UboManagerTest, UnmanageUnmanaged) {
    auto mi1 = static_cast<FMaterialInstance*>(mMaterial->createInstance());

    // Unmanaging an instance that was never managed should not cause any issues.
    mUboManager.unmanageMaterialInstance(mi1);
    EXPECT_FALSE(contains(mPendingInstances, mi1));
    EXPECT_FALSE(contains(mManagedInstances, mi1));

    mUboManager.terminate(mDriverApi);
    mEngine->destroy(mi1);
}

TEST_F(UboManagerTest, AllAllocationsLockedAfterEndFrame) {
    constexpr size_t numInstances = 5;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
        instances.push_back(mi);
        mUboManager.manageMaterialInstance(mi);
    }

    mUboManager.beginFrame(mDriverApi);
    mUboManager.finishBeginFrame(mDriverApi);
    mUboManager.endFrame(mDriverApi);

    for (const auto* mi: instances) {
        EXPECT_TRUE(mAllocator.isLockedByGpu(mi->getAllocationId()));
    }

    mUboManager.terminate(mDriverApi);
    for (auto* mi: instances) {
        // We're not using the UboManager inside mEngine, so we need to unmanage the material instance
        // by ourselves.
        mUboManager.unmanageMaterialInstance(mi);
        mEngine->destroy(mi);
    }
}

TEST_F(UboManagerTest, AllAllocationsLockedAfterEndFrameWithInvalidIdInBetween) {
    constexpr size_t numInstances = 5;
    std::vector<FMaterialInstance*> instances;
    instances.reserve(numInstances);

    for (size_t i = 0; i < numInstances; ++i) {
        auto mi = static_cast<FMaterialInstance*>(mMaterial->createInstance());
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

    mUboManager.terminate(mDriverApi);
    for (auto* mi: instances) {
        // We're not using the UboManager inside mEngine, so we need to unmanage the material instance
        // by ourselves.
        mUboManager.unmanageMaterialInstance(mi);
        mEngine->destroy(mi);
    }
}

// TODO: Add more tests for the beginFrame flow
