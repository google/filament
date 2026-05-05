// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_
#define SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_

#include <atomic>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/ExecutionQueue.h"
#include "dawn/native/IntegerTypes.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

// Types supported by the FencedDeleter must provide:
//   Type name, Destroy function, whether the type is deleted with the device or instance.
//
// Types are listed in order of deletion.
//  - Buffers and images must be deleted before memories because it is invalid to free memory
// that still have resources bound to it.
//  - Vulkan swapchains must be destroyed before their corresponding VkSurface
#define FENCED_DELETER_TYPES(X)                                        \
    X(VkBufferView, DestroyBufferView, device)                         \
    X(VkBuffer, DestroyBuffer, device)                                 \
    X(VkImage, DestroyImage, device)                                   \
    X(VkDeviceMemory, FreeMemory, device)                              \
    X(VkPipelineLayout, DestroyPipelineLayout, device)                 \
    X(VkRenderPass, DestroyRenderPass, device)                         \
    X(VkFence, DestroyFence, device)                                   \
    X(VkFramebuffer, DestroyFramebuffer, device)                       \
    X(VkImageView, DestroyImageView, device)                           \
    X(VkPipeline, DestroyPipeline, device)                             \
    X(VkSwapchainKHR, DestroySwapchainKHR, device)                     \
    X(VkSurfaceKHR, DestroySurfaceKHR, instance)                       \
    X(VkSemaphore, DestroySemaphore, device)                           \
    X(VkDescriptorPool, DestroyDescriptorPool, device)                 \
    X(VkQueryPool, DestroyQueryPool, device)                           \
    X(VkSamplerYcbcrConversion, DestroySamplerYcbcrConversion, device) \
    X(VkSampler, DestroySampler, device)

class Device;

class FencedDeleter final : public ExecutionQueueBase::SerialProcessor {
  public:
    explicit FencedDeleter(Device* device);
    ~FencedDeleter() override;

#define X(Type, ...) void DeleteWhenUnused(Type handle);
    FENCED_DELETER_TYPES(X)
#undef X

    // SerialProcessor API.
    void UpdateCompletedSerialTo(ExecutionSerial completedSerial) override;
    void AssumeCommandsComplete() override;

    // Returns the last serial that an object is pending deletion after or
    // kBeginningOfGPUTime if no objects are pending deletion.
    ExecutionSerial GetLastPendingDeletionSerial();
    // Returns the serial used for deleting the resources.
    ExecutionSerial GetCurrentDeletionSerial();

  private:
    raw_ptr<Device> mDevice = nullptr;

    struct PendingDeletions {
#define X(Type, ...) SerialQueue<ExecutionSerial, Type> m##Type;
        FENCED_DELETER_TYPES(X)
#undef X
        bool mAssumeCompleted = false;
    };
    MutexProtected<PendingDeletions> mPendingDeletions;

    std::atomic<ExecutionSerial> mLastTaskSerial = kBeginningOfGPUTime;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_
