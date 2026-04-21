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

#include "VulkanAsyncHandles.h"

#include "VulkanConstants.h"

#include "vulkan/utils/Spirv.h"

#include <backend/DriverEnums.h>

#include <utils/debug.h>

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <chrono>

using namespace bluevk;

namespace filament::backend {

namespace {

inline VkShaderStageFlags getVkStage(backend::ShaderStage stage) {
    switch(stage) {
        case backend::ShaderStage::VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case backend::ShaderStage::FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case backend::ShaderStage::COMPUTE:
            PANIC_POSTCONDITION("Unsupported stage");
    }
}

} // namespace

PushConstantDescription::PushConstantDescription(backend::Program const& program) {
    mRangeCount = 0;
    uint32_t offset = 0;

    // The range is laid out so that the vertex constants are defined as the first set of bytes,
    // followed by fragment and compute. This means we need to keep track of the offset for each
    // stage. We do the bookeeping in mDescriptions.
    for (auto stage: { ShaderStage::VERTEX, ShaderStage::FRAGMENT, ShaderStage::COMPUTE }) {
        auto const& constants = program.getPushConstants(stage);
        if (constants.empty()) {
            continue;
        }

        auto& description = mDescriptions[(uint8_t) stage];
        // We store the type of the constant for type-checking when writing.
        description.types.reserve(constants.size());
        std::for_each(constants.cbegin(), constants.cend(),
                [&description](Program::PushConstant t) { description.types.push_back(t.type); });

        uint32_t const constantsSize = (uint32_t) constants.size() * ENTRY_SIZE;
        mRanges[mRangeCount++] = {
            .stageFlags = getVkStage(stage),
            .offset = offset,
            .size = constantsSize,
        };
        description.offset = offset;
        offset += constantsSize;
    }
}

void PushConstantDescription::write(VkCommandBuffer cmdbuf, VkPipelineLayout layout,
        backend::ShaderStage stage, uint8_t index, backend::PushConstantVariant const& value) {

    uint32_t binaryValue = 0;
    auto const& description = mDescriptions[(uint8_t) stage];
    UTILS_UNUSED_IN_RELEASE auto const& types = description.types;
    uint32_t const offset = description.offset;

    if (std::holds_alternative<bool>(value)) {
        assert_invariant(types[index] == ConstantType::BOOL);
        bool const bval = std::get<bool>(value);
        binaryValue = static_cast<uint32_t const>(bval ? VK_TRUE : VK_FALSE);
    } else if (std::holds_alternative<float>(value)) {
        assert_invariant(types[index] == ConstantType::FLOAT);
        float const fval = std::get<float>(value);
        binaryValue = *reinterpret_cast<uint32_t const*>(&fval);
    } else {
        assert_invariant(types[index] == ConstantType::INT);
        int const ival = std::get<int>(value);
        binaryValue = *reinterpret_cast<uint32_t const*>(&ival);
    }

    vkCmdPushConstants(cmdbuf, layout, getVkStage(stage), offset + index * ENTRY_SIZE, ENTRY_SIZE,
            &binaryValue);
}

VulkanProgram::VulkanProgram(VkDevice device, Program const& builder) noexcept
    : HwProgram(builder.getName()),
      mInfo(new(std::nothrow) PipelineInfo(builder)),
      mDevice(device) {

    Program::ShaderSource const& blobs = builder.getShadersSource();
    auto& modules = mInfo->shaders;
    auto const& specializationConstants = builder.getSpecializationConstants();
    std::vector<uint32_t> shader;

    static_assert(static_cast<ShaderStage>(0) == ShaderStage::VERTEX &&
            static_cast<ShaderStage>(1) == ShaderStage::FRAGMENT &&
            MAX_SHADER_MODULES == 2);

    for (size_t i = 0; i < MAX_SHADER_MODULES; i++) {
        Program::ShaderBlob const& blob = blobs[i];

        uint32_t* data = (uint32_t*) blob.data();
        size_t dataSize = blob.size();

        if (!specializationConstants.empty()) {
            fvkutils::workaroundSpecConstant(blob, specializationConstants, shader);
            data = (uint32_t*) shader.data();
            dataSize = shader.size() * 4;
        }

        VkShaderModule& module = modules[i];
        VkShaderModuleCreateInfo moduleInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = dataSize,
            .pCode = data,
        };
        VkResult result = vkCreateShaderModule(mDevice, &moduleInfo, VKALLOC, &module);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "Unable to create shader module."
                << " error=" << static_cast<int32_t>(result);

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
        utils::CString name{ builder.getName().c_str(), builder.getName().size() };
        switch (static_cast<ShaderStage>(i)) {
            case ShaderStage::VERTEX:
                name += "_vs";
                break;
            case ShaderStage::FRAGMENT:
                name += "_fs";
                break;
            default:
                PANIC_POSTCONDITION("Unexpected stage");
                break;
        }
        VulkanDriver::DebugUtils::setName(VK_OBJECT_TYPE_SHADER_MODULE,
                reinterpret_cast<uint64_t>(module), name.c_str());
#endif
    }

#if FVK_ENABLED(FVK_DEBUG_SHADER_MODULE)
    FVK_LOGD << "Created VulkanProgram " << builder << ", shaders = (" << modules[0]
             << ", " << modules[1] << ")";
#endif
}

VulkanProgram::~VulkanProgram() {
    for (auto shader: mInfo->shaders) {
        vkDestroyShaderModule(mDevice, shader, VKALLOC);
    }
    delete mInfo;
}

void VulkanProgram::flushPushConstants(VkPipelineLayout layout) {
    // At this point, we really ought to have a VkPipelineLayout.
    assert_invariant(layout != VK_NULL_HANDLE);
    for (const auto& c : mQueuedPushConstants) {
        mInfo->pushConstantDescription.write(c.cmdbuf, layout, c.stage, c.index, c.value);
    }
    mQueuedPushConstants.clear();
}

std::shared_ptr<VulkanCmdFence> VulkanCmdFence::completed() noexcept {
    auto cmdFence = std::make_shared<VulkanCmdFence>(VK_NULL_HANDLE);
    cmdFence->mStatus = VK_SUCCESS;
    return cmdFence;
}

FenceStatus VulkanCmdFence::wait(VkDevice device, uint64_t const timeout,
    std::chrono::steady_clock::time_point const until) {

    // this lock MUST be held for READ when calling vkWaitForFences()
    std::shared_lock rl(mLock);

    // If the vulkan fence has not been submitted yet, we need to wait for that before we
    // can use vkWaitForFences()
    if (mStatus == VK_INCOMPLETE) {
        bool const success = mCond.wait_until(rl, until, [this] {
            // Internally we use the VK_INCOMPLETE status to mean "not yet submitted".
            // When this fence gets submitted, its status changes to VK_NOT_READY.
            return mStatus != VK_INCOMPLETE || mCanceled;
        });
        if (!success) {
            // !success indicates a timeout or cancel
            return mCanceled ? FenceStatus::ERROR : FenceStatus::TIMEOUT_EXPIRED;
        }
    }

    // The fence could have already signaled, avoid calling into vkWaitForFences()
    if (mStatus == VK_SUCCESS) {
        return FenceStatus::CONDITION_SATISFIED;
    }

    // Or it could have been canceled, return immediately
    if (mCanceled) {
        return FenceStatus::ERROR;
    }

    // If we're here, we know that vkQueueSubmit has been called (because it sets the status
    // to VK_NOT_READY).
    // Now really wait for the fence while holding the shared_lock, this allows several
    // threads to call vkWaitForFences(), but will prevent vkResetFence from taking
    // place simultaneously. vkResetFence is only called once it knows the fence has signaled,
    // which guaranties that vkResetFence won't have to wait too long, just enough for
    // all the vkWaitForFences() to return.
    VkResult const status = vkWaitForFences(device, 1, &mFence, VK_TRUE, timeout);
    if (status == VK_TIMEOUT) {
        return FenceStatus::TIMEOUT_EXPIRED;
    }

    if (status == VK_SUCCESS) {
        rl.unlock();
        std::lock_guard const wl(mLock);
        mStatus = status;
        return FenceStatus::CONDITION_SATISFIED;
    }

    return FenceStatus::ERROR; // not supported
}

void VulkanCmdFence::resetFence(VkDevice device) {
    // This lock prevents vkResetFences() from being called simultaneously with vkWaitForFences(),
    // but by construction, when we're here, we know that the fence has signaled and
    // vkWaitForFences() will return shortly.
    std::lock_guard const l(mLock);
    assert_invariant(mStatus == VK_SUCCESS);
    vkResetFences(device, 1, &mFence);
}

} // namespace filament::backend
