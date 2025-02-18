// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/QueueVk.h"

#include <limits>
#include <optional>
#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/vulkan/CommandBufferVk.h"
#include "dawn/native/vulkan/CommandRecordingContextVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/SharedFenceVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UniqueVkHandle.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

namespace {

// Destroys command pool/buffer.
// TODO(dawn:1601) Revisit this and potentially bake into pool/buffer objects instead.
void DestroyCommandPoolAndBuffer(const VulkanFunctions& fn,
                                 VkDevice device,
                                 const CommandPoolAndBuffer& commands) {
    // The VkCommandBuffer memory should be wholly owned by the pool and freed when it is
    // destroyed, but that's not the case in some drivers and they leak memory. So we call
    // FreeCommandBuffers before DestroyCommandPool to be safe.
    // TODO(enga): Only do this on a known list of bad drivers.
    if (commands.pool != VK_NULL_HANDLE) {
        if (commands.commandBuffer != VK_NULL_HANDLE) {
            fn.FreeCommandBuffers(device, commands.pool, 1, &commands.commandBuffer);
        }
        fn.DestroyCommandPool(device, commands.pool, nullptr);
    }
}

}  // anonymous namespace

// static
ResultOrError<Ref<Queue>> Queue::Create(Device* device,
                                        const QueueDescriptor* descriptor,
                                        uint32_t family) {
    Ref<Queue> queue = AcquireRef(new Queue(device, descriptor, family));
    DAWN_TRY(queue->Initialize());
    return queue;
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor, uint32_t family)
    : QueueBase(device, descriptor), mQueueFamily(family) {}

Queue::~Queue() {}

MaybeError Queue::Initialize() {
    Device* device = ToBackend(GetDevice());
    device->fn.GetDeviceQueue(device->GetVkDevice(), mQueueFamily, 0, &mQueue);

    DAWN_TRY(PrepareRecordingContext());

    SetLabelImpl();
    return {};
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferVk::RecordCommands");
    CommandRecordingContext* recordingContext = GetPendingRecordingContext();
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->RecordCommands(recordingContext));
    }
    TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferVk::RecordCommands");

    DAWN_TRY(SubmitPendingCommands());

    return {};
}

void Queue::SetLabelImpl() {
    Device* device = ToBackend(GetDevice());
    // TODO(crbug.com/dawn/1344): When we start using multiple queues this needs to be adjusted
    // so it doesn't always change the default queue's label.
    SetDebugName(device, VK_OBJECT_TYPE_QUEUE, mQueue, "Dawn_Queue", GetLabel());
}

bool Queue::HasPendingCommands() const {
    return mRecordingContext.needsSubmit;
}

VkQueue Queue::GetVkQueue() const {
    return mQueue;
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    Device* device = ToBackend(GetDevice());
    return mFencesInFlight.Use([&](auto fencesInFlight) -> ResultOrError<ExecutionSerial> {
        ExecutionSerial fenceSerial(0);
        while (!fencesInFlight->empty()) {
            VkFence fence = fencesInFlight->front().first;
            ExecutionSerial tentativeSerial = fencesInFlight->front().second;
            VkResult result = VkResult::WrapUnsafe(INJECT_ERROR_OR_RUN(
                device->fn.GetFenceStatus(device->GetVkDevice(), fence), VK_ERROR_DEVICE_LOST));

            // Fence are added in order, so we can stop searching as soon
            // as we see one that's not ready.
            if (result == VK_NOT_READY) {
                return fenceSerial;
            } else {
                DAWN_TRY(CheckVkSuccess(::VkResult(result), "GetFenceStatus"));
            }

            // Update fenceSerial since fence is ready.
            fenceSerial = tentativeSerial;

            mUnusedFences->push_back(fence);

            DAWN_ASSERT(fenceSerial > GetCompletedCommandSerial());
            fencesInFlight->pop_front();
        }
        return fenceSerial;
    });
}

void Queue::ForceEventualFlushOfCommands() {
    mRecordingContext.needsSubmit |= mRecordingContext.used;
}

MaybeError Queue::WaitForIdleForDestruction() {
    // Immediately tag the recording context as unused so we don't try to submit it in Tick.
    // Move the mRecordingContext.used to mUnusedCommands so it can be cleaned up in
    // ShutDownImpl
    if (mRecordingContext.used) {
        CommandPoolAndBuffer commands = {mRecordingContext.commandPool,
                                         mRecordingContext.commandBuffer};
        mUnusedCommands.push_back(commands);
        mRecordingContext = CommandRecordingContext();
    }

    Device* device = ToBackend(GetDevice());
    VkDevice vkDevice = device->GetVkDevice();

    // Ignore the result of QueueWaitIdle: it can return OOM which we can't really do anything
    // about, Device lost, which means workloads running on the GPU are no longer accessible
    // (so they are as good as waited on) or success.
    [[maybe_unused]] VkResult waitIdleResult =
        VkResult::WrapUnsafe(device->fn.QueueWaitIdle(mQueue));

    // Make sure all fences are complete by explicitly waiting on them all
    mFencesInFlight.Use([&](auto fencesInFlight) {
        while (!fencesInFlight->empty()) {
            VkFence fence = fencesInFlight->front().first;
            ExecutionSerial fenceSerial = fencesInFlight->front().second;
            DAWN_ASSERT(fenceSerial > GetCompletedCommandSerial());

            VkResult result = VkResult::WrapUnsafe(VK_TIMEOUT);
            do {
                // If WaitForIdleForDesctruction is called while we are Disconnected, it means that
                // the device lost came from the ErrorInjector and we need to wait without allowing
                // any more error to be injected. This is because the device lost was "fake" and
                // commands might still be running.
                if (GetDevice()->GetState() == Device::State::Disconnected) {
                    result = VkResult::WrapUnsafe(
                        device->fn.WaitForFences(vkDevice, 1, &*fence, true, UINT64_MAX));
                    continue;
                }

                result = VkResult::WrapUnsafe(INJECT_ERROR_OR_RUN(
                    device->fn.WaitForFences(vkDevice, 1, &*fence, true, UINT64_MAX),
                    VK_ERROR_DEVICE_LOST));
            } while (result == VK_TIMEOUT);
            // Ignore errors from vkWaitForFences: it can be either OOM which we can't do anything
            // about (and we need to keep going with the destruction of all fences), or device
            // loss, which means the workload on the GPU is no longer accessible and we can
            // safely destroy the fence.

            device->fn.DestroyFence(vkDevice, fence, nullptr);
            fencesInFlight->pop_front();
        }
    });
    return {};
}

CommandRecordingContext* Queue::GetPendingRecordingContext(SubmitMode submitMode) {
    DAWN_ASSERT(mRecordingContext.commandBuffer != VK_NULL_HANDLE);
    mRecordingContext.needsSubmit |= (submitMode == SubmitMode::Normal);
    mRecordingContext.used = true;
    return &mRecordingContext;
}

MaybeError Queue::PrepareRecordingContext() {
    DAWN_ASSERT(!mRecordingContext.needsSubmit);
    DAWN_ASSERT(mRecordingContext.commandBuffer == VK_NULL_HANDLE);
    DAWN_ASSERT(mRecordingContext.commandPool == VK_NULL_HANDLE);

    CommandPoolAndBuffer commands;
    DAWN_TRY_ASSIGN(commands, BeginVkCommandBuffer());

    mRecordingContext.commandBuffer = commands.commandBuffer;
    mRecordingContext.commandPool = commands.pool;
    mRecordingContext.commandBufferList.push_back(commands.commandBuffer);
    mRecordingContext.commandPoolList.push_back(commands.pool);

    return {};
}

// Splits the recording context, ending the current command buffer and beginning a new one.
// This should not be necessary in most cases, and is provided only to work around driver issues
// on some hardware.
MaybeError Queue::SplitRecordingContext(CommandRecordingContext* recordingContext) {
    DAWN_ASSERT(recordingContext->used);
    Device* device = ToBackend(GetDevice());

    DAWN_TRY(CheckVkSuccess(device->fn.EndCommandBuffer(recordingContext->commandBuffer),
                            "vkEndCommandBuffer"));

    CommandPoolAndBuffer commands;
    DAWN_TRY_ASSIGN(commands, BeginVkCommandBuffer());

    recordingContext->commandBuffer = commands.commandBuffer;
    recordingContext->commandPool = commands.pool;
    recordingContext->commandBufferList.push_back(commands.commandBuffer);
    recordingContext->commandPoolList.push_back(commands.pool);
    recordingContext->hasRecordedRenderPass = false;

    return {};
}

ResultOrError<CommandPoolAndBuffer> Queue::BeginVkCommandBuffer() {
    Device* device = ToBackend(GetDevice());
    VkDevice vkDevice = device->GetVkDevice();

    CommandPoolAndBuffer commands;

    // First try to recycle unused command pools.
    if (!mUnusedCommands.empty()) {
        commands = mUnusedCommands.back();
        mUnusedCommands.pop_back();
        DAWN_TRY_WITH_CLEANUP(
            CheckVkSuccess(device->fn.ResetCommandPool(vkDevice, commands.pool, 0),
                           "vkResetCommandPool"),
            { DestroyCommandPoolAndBuffer(device->fn, vkDevice, commands); });
    } else {
        // Create a new command pool for our commands and allocate the command buffer.
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        createInfo.queueFamilyIndex = mQueueFamily;

        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateCommandPool(vkDevice, &createInfo, nullptr, &*commands.pool),
            "vkCreateCommandPool"));

        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = commands.pool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        DAWN_TRY_WITH_CLEANUP(CheckVkSuccess(device->fn.AllocateCommandBuffers(
                                                 vkDevice, &allocateInfo, &commands.commandBuffer),
                                             "vkAllocateCommandBuffers"),
                              { DestroyCommandPoolAndBuffer(device->fn, vkDevice, commands); });
    }

    // Start the recording of commands in the command buffer.
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    DAWN_TRY_WITH_CLEANUP(
        CheckVkSuccess(device->fn.BeginCommandBuffer(commands.commandBuffer, &beginInfo),
                       "vkBeginCommandBuffer"),
        { DestroyCommandPoolAndBuffer(device->fn, vkDevice, commands); });

    return commands;
}

void Queue::RecycleCompletedCommands(ExecutionSerial completedSerial) {
    for (auto& commands : mCommandsInFlight.IterateUpTo(completedSerial)) {
        mUnusedCommands.push_back(commands);
    }
    mCommandsInFlight.ClearUpTo(completedSerial);
}

MaybeError Queue::SubmitPendingCommands() {
    if (!mRecordingContext.needsSubmit) {
        return {};
    }

    Device* device = ToBackend(GetDevice());

    if (!mRecordingContext.mappableBuffersForEagerTransition.empty()) {
        // Transition mappable buffers back to map usages with the submit.
        Buffer::TransitionMappableBuffersEagerly(
            device, &mRecordingContext, mRecordingContext.mappableBuffersForEagerTransition);
    }

    // Create an external semaphore for each external textures used in the pending submit.
    std::vector<UniqueVkHandle<VkSemaphore>> externalTextureSemaphores(
        mRecordingContext.specialSyncTextures.size());
    for (size_t i = 0; i < mRecordingContext.specialSyncTextures.size(); ++i) {
        VkSemaphore semaphore;
        DAWN_TRY_ASSIGN(semaphore,
                        device->GetExternalSemaphoreService()->CreateExportableSemaphore());
        externalTextureSemaphores[i] = {device, semaphore};
    }

    // Transition eagerly all used external textures for export.
    for (auto texture : mRecordingContext.specialSyncTextures) {
        texture->TransitionEagerlyForExport(&mRecordingContext);

        // TODO(330385376): Remove once ExternalImageDescriptorVk is removed.
        std::vector<VkSemaphore> waitRequirements = texture->AcquireWaitRequirements();
        mRecordingContext.waitSemaphores.insert(mRecordingContext.waitSemaphores.end(),
                                                waitRequirements.begin(), waitRequirements.end());

        SharedResourceMemoryContents* contents = texture->GetSharedResourceMemoryContents();
        if (contents != nullptr) {
            SharedTextureMemoryBase::PendingFenceList fences;
            contents->AcquirePendingFences(&fences);

            for (const auto& fence : fences) {
                // All semaphores are binary semaphores.
                DAWN_ASSERT(fence.signaledValue == 1u);
                ExternalSemaphoreHandle semaphoreHandle =
                    ToBackend(fence.object)->GetHandle().Get();

                VkSemaphore semaphore;
                DAWN_TRY_ASSIGN(semaphore, device->GetExternalSemaphoreService()->ImportSemaphore(
                                               semaphoreHandle));
                mRecordingContext.waitSemaphores.push_back(semaphore);
            }
        }
    }

    DAWN_TRY(CheckVkSuccess(device->fn.EndCommandBuffer(mRecordingContext.commandBuffer),
                            "vkEndCommandBuffer"));

    std::vector<VkPipelineStageFlags> dstStageMasks(mRecordingContext.waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    for (auto& externalTextureSemaphore : externalTextureSemaphores) {
        mRecordingContext.signalSemaphores.push_back(externalTextureSemaphore.Get());
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(mRecordingContext.waitSemaphores.size());
    submitInfo.pWaitSemaphores = AsVkArray(mRecordingContext.waitSemaphores.data());
    submitInfo.pWaitDstStageMask = dstStageMasks.data();
    submitInfo.commandBufferCount = mRecordingContext.commandBufferList.size();
    submitInfo.pCommandBuffers = mRecordingContext.commandBufferList.data();
    submitInfo.signalSemaphoreCount = mRecordingContext.signalSemaphores.size();
    submitInfo.pSignalSemaphores = AsVkArray(mRecordingContext.signalSemaphores.data());

    VkFence fence = VK_NULL_HANDLE;
    DAWN_TRY_ASSIGN(fence, GetUnusedFence());

    TRACE_EVENT_BEGIN0(device->GetPlatform(), Recording, "vkQueueSubmit");
    DAWN_TRY_WITH_CLEANUP(
        CheckVkSuccess(device->fn.QueueSubmit(mQueue, 1, &submitInfo, fence), "vkQueueSubmit"), {
            // If submitting to the queue fails, move the fence back into the unused fence
            // list, as if it were never acquired. Not doing so would leak the fence since
            // it would be neither in the unused list nor in the in-flight list.
            mUnusedFences->push_back(fence);
        });
    TRACE_EVENT_END0(device->GetPlatform(), Recording, "vkQueueSubmit");

    // Enqueue the semaphores before incrementing the serial, so that they can be deleted as
    // soon as the current submission is finished.
    for (VkSemaphore semaphore : mRecordingContext.waitSemaphores) {
        device->GetFencedDeleter()->DeleteWhenUnused(semaphore);
    }
    IncrementLastSubmittedCommandSerial();
    ExecutionSerial lastSubmittedSerial = GetLastSubmittedCommandSerial();
    mFencesInFlight->emplace_back(fence, lastSubmittedSerial);

    for (size_t i = 0; i < mRecordingContext.commandBufferList.size(); ++i) {
        CommandPoolAndBuffer submittedCommands = {mRecordingContext.commandPoolList[i],
                                                  mRecordingContext.commandBufferList[i]};
        mCommandsInFlight.Enqueue(submittedCommands, lastSubmittedSerial);
    }

    auto externalTextureSemaphoreIter = externalTextureSemaphores.begin();
    for (auto texture : mRecordingContext.specialSyncTextures) {
        // Export the signal semaphore.
        ExternalSemaphoreHandle semaphoreHandle;
        DAWN_TRY_ASSIGN(semaphoreHandle, device->GetExternalSemaphoreService()->ExportSemaphore(
                                             externalTextureSemaphoreIter->Get()));
        ++externalTextureSemaphoreIter;

        // Update all external textures, eagerly transitioned in the submit, with the exported
        // handles.
        texture->UpdateExternalSemaphoreHandle(semaphoreHandle);
    }
    DAWN_ASSERT(externalTextureSemaphoreIter == externalTextureSemaphores.end());

    mRecordingContext = CommandRecordingContext();
    DAWN_TRY(PrepareRecordingContext());

    return {};
}

ResultOrError<VkFence> Queue::GetUnusedFence() {
    Device* device = ToBackend(GetDevice());
    VkDevice vkDevice = device->GetVkDevice();

    auto result =
        mUnusedFences.Use([&](auto unusedFences) -> std::optional<ResultOrError<VkFence>> {
            if (!unusedFences->empty()) {
                VkFence fence = unusedFences->back();
                DAWN_ASSERT(fence != VK_NULL_HANDLE);
                DAWN_TRY(
                    CheckVkSuccess(device->fn.ResetFences(vkDevice, 1, &*fence), "vkResetFences"));

                unusedFences->pop_back();
                return fence;
            }
            return std::nullopt;
        });
    if (result) {
        return std::move(*result);
    }

    VkFenceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VkFence fence = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(device->fn.CreateFence(vkDevice, &createInfo, nullptr, &*fence),
                            "vkCreateFence"));

    return fence;
}

void Queue::DestroyImpl() {
    Device* device = ToBackend(GetDevice());
    VkDevice vkDevice = device->GetVkDevice();

    // Immediately tag the recording context as unused so we don't try to submit it in Tick.
    mRecordingContext.needsSubmit = false;
    if (mRecordingContext.commandPool != VK_NULL_HANDLE) {
        DestroyCommandPoolAndBuffer(
            device->fn, vkDevice, {mRecordingContext.commandPool, mRecordingContext.commandBuffer});
    }

    for (VkSemaphore semaphore : mRecordingContext.waitSemaphores) {
        device->fn.DestroySemaphore(vkDevice, semaphore, nullptr);
    }
    mRecordingContext.waitSemaphores.clear();
    mRecordingContext.signalSemaphores.clear();

    // Some commands might still be marked as in-flight if we shut down because of a device
    // loss. Recycle them as unused so that we free them below.
    RecycleCompletedCommands(kMaxExecutionSerial);
    DAWN_ASSERT(mCommandsInFlight.Empty());

    for (const CommandPoolAndBuffer& commands : mUnusedCommands) {
        DestroyCommandPoolAndBuffer(device->fn, vkDevice, commands);
    }
    mUnusedCommands.clear();

    // Some fences might still be marked as in-flight if we shut down because of a device loss.
    // Delete them since at this point all commands are complete.
    mFencesInFlight.Use([&](auto fencesInFlight) {
        while (!fencesInFlight->empty()) {
            device->fn.DestroyFence(vkDevice, *fencesInFlight->front().first, nullptr);
            fencesInFlight->pop_front();
        }
    });

    mUnusedFences.Use([&](auto unusedFences) {
        for (VkFence fence : *unusedFences) {
            device->fn.DestroyFence(vkDevice, fence, nullptr);
        }
        unusedFences->clear();
    });

    QueueBase::DestroyImpl();
}

ResultOrError<bool> Queue::WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout) {
    Device* device = ToBackend(GetDevice());
    VkDevice vkDevice = device->GetVkDevice();
    // If the client has passed a finite timeout, the function will eventually return due to
    // either (1) the fences being signaled, (2) the timeout being reached, or (3) the device
    // being lost. If the client has passed an infinite timeout, this function might hang forever
    // if the fences are never signaled (which matches the semantics that the client has
    // specified).
    // TODO(crbug.com/344798087): Handle the issue of timeouts in a more general way further up the
    // stack.
    while (1) {
        VkResult waitResult = mFencesInFlight.Use([&](auto fencesInFlight) {
            // Search from for the first fence >= serial.
            VkFence waitFence = VK_NULL_HANDLE;
            for (auto it = fencesInFlight->begin(); it != fencesInFlight->end(); ++it) {
                if (it->second >= serial) {
                    waitFence = it->first;
                    break;
                }
            }
            if (waitFence == VK_NULL_HANDLE) {
                // Fence not found. This serial must have already completed.
                // Return a VK_SUCCESS status.
                DAWN_ASSERT(serial <= GetCompletedCommandSerial());
                return VkResult::WrapUnsafe(VK_SUCCESS);
            }
            // Wait for the fence.
            return VkResult::WrapUnsafe(
                INJECT_ERROR_OR_RUN(device->fn.WaitForFences(vkDevice, 1, &*waitFence, true,
                                                             static_cast<uint64_t>(timeout)),
                                    VK_ERROR_DEVICE_LOST));
        });
        if (waitResult == VK_TIMEOUT) {
            // There is evidence that `VK_TIMEOUT` can get returned even when the
            // client has specified an infinite timeout (e.g., due to signals). Retry
            // waiting on the fence in this case in order to satisfy the semantics
            // that the function should return only when either (a) the fences are
            // signaled or (b) the passed-in timeout is reached. Note that this can
            // result in this function busy-looping forever in this case, but the
            // client has explicitly requested this behavior by passing in an infinite
            // timeout.
            // TODO(crbug.com/344798087): Handle the issue of timeouts in a more general way further
            // up the stack.
            if (static_cast<uint64_t>(timeout) == std::numeric_limits<uint64_t>::max()) {
                continue;
            }
            return false;
        }
        DAWN_TRY(CheckVkSuccess(::VkResult(waitResult), "vkWaitForFences"));
        return true;
    }
}

}  // namespace dawn::native::vulkan
