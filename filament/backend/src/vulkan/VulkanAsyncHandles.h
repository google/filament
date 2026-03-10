/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANASYNCHANDLES_H
#define TNT_FILAMENT_BACKEND_VULKANASYNCHANDLES_H

#include <bluevk/BlueVK.h>

#include "DriverBase.h"
#include "backend/DriverEnums.h"
#include "backend/Platform.h"

#include "vulkan/memory/Resource.h"
#include "vulkan/utils/StaticVector.h"

#include <backend/Program.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace filament::backend {

using PushConstantNameArray = utils::FixedCapacityVector<char const*>;
using PushConstantNameByStage = std::array<PushConstantNameArray, Program::SHADER_TYPE_COUNT>;

struct PushConstantDescription {
    explicit PushConstantDescription(backend::Program const& program);

    VkPushConstantRange const* getVkRanges() const noexcept { return mRanges; }
    uint32_t getVkRangeCount() const noexcept { return mRangeCount; }
    void write(VkCommandBuffer cmdbuf, VkPipelineLayout layout, backend::ShaderStage stage,
            uint8_t index, backend::PushConstantVariant const& value);

private:
    static constexpr uint32_t ENTRY_SIZE = sizeof(uint32_t);

    struct ConstantDescription {
        utils::FixedCapacityVector<backend::ConstantType> types;
        uint32_t offset = 0;
    };

    // Describes the constants in each shader stage.
    ConstantDescription mDescriptions[Program::SHADER_TYPE_COUNT];
    VkPushConstantRange mRanges[Program::SHADER_TYPE_COUNT];
    uint32_t mRangeCount;
};

struct VulkanProgram : public HwProgram, fvkmemory::ThreadSafeResource {
    using BindingList = fvkutils::StaticVector<uint16_t, MAX_SAMPLER_COUNT>;

    VulkanProgram(VkDevice device, Program const& builder) noexcept;
    ~VulkanProgram();

    /**
     * Cancels any parallel compilation jobs that have not yet run for this
     * program.
     */
    inline void cancelParallelCompilation() {
        mParallelCompilationCanceled.store(true, std::memory_order_release);
    }

    /**
     * Writes out any queued push constants using the provided VkPipelineLayout.
     *
     * @param layout The layout that is to be used along with these push constants,
     *               in the next draw call.
     */
    void flushPushConstants(VkPipelineLayout layout);

    inline VkShaderModule getVertexShader() const {
        return mInfo->shaders[0];
    }

    inline VkShaderModule getFragmentShader() const { return mInfo->shaders[1]; }

    inline uint32_t getPushConstantRangeCount() const {
        return mInfo->pushConstantDescription.getVkRangeCount();
    }

    inline VkPushConstantRange const* getPushConstantRanges() const {
        return mInfo->pushConstantDescription.getVkRanges();
    }

    /**
     * Returns true if parallel compilation is canceled, false if not. Parallel
     * compilation will be canceled if this program is destroyed before relevant
     * pipelines are created.
     *
     * @return true if parallel compilation should run for this program, false if not
     */
    inline bool isParallelCompilationCanceled() const {
        return mParallelCompilationCanceled.load(std::memory_order_acquire);
    }

    inline void writePushConstant(VkCommandBuffer cmdbuf, VkPipelineLayout layout,
            backend::ShaderStage stage, uint8_t index, backend::PushConstantVariant const& value) {
        // It's possible that we don't have the layout yet. When external samplers are used, bindPipeline()
        // in VulkanDriver returns early, without binding a layout. If that happens, the layout is not
        // set until draw time. Any push constants that are written during that time should be saved for
        // later, and flushed when the layout is set.
        if (layout != VK_NULL_HANDLE) {
            mInfo->pushConstantDescription.write(cmdbuf, layout, stage, index, value);
        } else {
            mQueuedPushConstants.push_back({cmdbuf, stage, index, value});
        }
    }

    // TODO: handle compute shaders.
    // The expected order of shaders - from frontend to backend - is vertex, fragment, compute.
    static constexpr uint8_t const MAX_SHADER_MODULES = 2;

private:
    struct PipelineInfo {
        explicit PipelineInfo(backend::Program const& program) noexcept
            : pushConstantDescription(program)
            {}

        VkShaderModule shaders[MAX_SHADER_MODULES] = { VK_NULL_HANDLE };
        PushConstantDescription pushConstantDescription;
    };

    struct PushConstantInfo {
        VkCommandBuffer cmdbuf;
        backend::ShaderStage stage;
        uint8_t index;
        backend::PushConstantVariant value;
    };

    PipelineInfo* mInfo;
    VkDevice mDevice = VK_NULL_HANDLE;
    std::atomic<bool> mParallelCompilationCanceled { false };
    std::vector<PushConstantInfo> mQueuedPushConstants;
};

// Wrapper to enable use of shared_ptr for implementing shared ownership of low-level Vulkan fences.
struct VulkanCmdFence {
    explicit VulkanCmdFence(VkFence fence) : mFence(fence) { }
    ~VulkanCmdFence() = default;

    // Creates a VulkanCmdFence with its status set to VK_SUCCESS. It holds
    // a null handle; it is assumed that any user of this object will avoid
    // using the fence handle directly if getStatus() returns VK_SUCCESS, as
    // in that case, it's likely the fence is being reused for other passes,
    // and is not in the expected state anyway.
    static std::shared_ptr<VulkanCmdFence> completed() noexcept;

    void setStatus(VkResult const value) {
        std::lock_guard const l(mLock);
        mStatus = value;
        mCond.notify_all();
    }

    VkResult getStatus() {
        std::shared_lock const l(mLock);
        return mStatus;
    }

    void resetFence(VkDevice device);

    FenceStatus wait(VkDevice device, uint64_t timeout,
        std::chrono::steady_clock::time_point until);

    void cancel() {
        std::lock_guard const l(mLock);
        mCanceled = true;
        mCond.notify_all();
    }

private:
    std::shared_mutex mLock; // NOLINT(*-include-cleaner)
    std::condition_variable_any mCond;
    bool mCanceled = false;
    // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence
    // gets submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually
    // finishes executing the command buffer, the status changes to VK_SUCCESS.
    VkResult mStatus{ VK_INCOMPLETE };
    VkFence mFence;
};

struct VulkanFence : public HwFence, fvkmemory::ThreadSafeResource {
    VulkanFence() {}

    void setFence(std::shared_ptr<VulkanCmdFence> fence) {
        std::lock_guard const l(lock);
        sharedFence = std::move(fence);
        cond.notify_all();
    }

    std::shared_ptr<VulkanCmdFence>& getSharedFence() {
        std::lock_guard const l(lock);
        return sharedFence;
    }

    std::pair<std::shared_ptr<VulkanCmdFence>, bool>
            wait(std::chrono::steady_clock::time_point const until) {
        // hold a reference so that our state doesn't disappear while we wait
        std::unique_lock l(lock);
        cond.wait_until(l, until, [this] {
            return bool(sharedFence) || canceled;
        });
        // here mSharedFence will be null if we timed out
        return { sharedFence, canceled };
    }

    void cancel() const {
        std::lock_guard const l(lock);
        if (sharedFence) {
            sharedFence->cancel();
        }
        canceled = true;
        cond.notify_all();
    }

private:
    mutable std::mutex lock;
    mutable std::condition_variable cond;
    mutable bool canceled = false;
    std::shared_ptr<VulkanCmdFence> sharedFence;
};

struct VulkanSync : fvkmemory::ThreadSafeResource, public HwSync {
    struct CallbackData {
        CallbackHandler* handler;
        Platform::SyncCallback cb;
        Platform::Sync* sync;
        void* userData;
    };

    VulkanSync() {}
    std::mutex lock;
    std::vector<std::unique_ptr<CallbackData>> conversionCallbacks;
};

struct VulkanTimerQuery : public HwTimerQuery, fvkmemory::ThreadSafeResource {
    VulkanTimerQuery(uint32_t startingIndex, uint32_t stoppingIndex)
        : mStartingQueryIndex(startingIndex),
          mStoppingQueryIndex(stoppingIndex) {}

    void setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept {
        std::lock_guard const lock(mFenceMutex);
        mFence = std::move(fence);
    }

    bool isCompleted() noexcept {
        std::lock_guard const lock(mFenceMutex);
        // QueryValue is a synchronous call and might occur before beginTimerQuery has written
        // anything into the command buffer, which is an error according to the validation layer
        // that ships in the Android NDK.  Even when AVAILABILITY_BIT is set, validation seems to
        // require that the timestamp has at least been written into a processed command buffer.

        // This fence indicates that the corresponding buffer has been completed.
        return mFence && mFence->getStatus() == VK_SUCCESS;
    }

    uint32_t getStartingQueryIndex() const { return mStartingQueryIndex; }

    uint32_t getStoppingQueryIndex() const {
        return mStoppingQueryIndex;
    }

private:
    uint32_t mStartingQueryIndex;
    uint32_t mStoppingQueryIndex;

    std::shared_ptr<VulkanCmdFence> mFence;
    utils::Mutex mFenceMutex;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANHASYNCANDLES_H
