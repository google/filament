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

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/IntegerTypes.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

class Device;

class FencedDeleter {
  public:
    explicit FencedDeleter(Device* device);
    ~FencedDeleter();

    void DeleteWhenUnused(VkBuffer buffer);
    void DeleteWhenUnused(VkDescriptorPool pool);
    void DeleteWhenUnused(VkDeviceMemory memory);
    void DeleteWhenUnused(VkFence fence);
    void DeleteWhenUnused(VkFramebuffer framebuffer);
    void DeleteWhenUnused(VkImage image);
    void DeleteWhenUnused(VkImageView view);
    void DeleteWhenUnused(VkPipelineLayout layout);
    void DeleteWhenUnused(VkRenderPass renderPass);
    void DeleteWhenUnused(VkPipeline pipeline);
    void DeleteWhenUnused(VkQueryPool querypool);
    void DeleteWhenUnused(VkSamplerYcbcrConversion samplerYcbcrConversion);
    void DeleteWhenUnused(VkSampler sampler);
    void DeleteWhenUnused(VkSemaphore semaphore);
    void DeleteWhenUnused(VkShaderModule module);
    void DeleteWhenUnused(VkSurfaceKHR surface);
    void DeleteWhenUnused(VkSwapchainKHR swapChain);

    // Returns the last serial that an object is pending deletion after or
    // kBeginningOfGPUTime if no objects are pending deletion.
    ExecutionSerial GetLastPendingDeletionSerial();
    // Returns the serial used for deleting the resources.
    ExecutionSerial GetCurrentDeletionSerial();

    void Tick(ExecutionSerial completedSerial);

  private:
    raw_ptr<Device> mDevice = nullptr;
    SerialQueue<ExecutionSerial, VkBuffer> mBuffersToDelete;
    SerialQueue<ExecutionSerial, VkDescriptorPool> mDescriptorPoolsToDelete;
    SerialQueue<ExecutionSerial, VkDeviceMemory> mMemoriesToDelete;
    SerialQueue<ExecutionSerial, VkFence> mFencesToDelete;
    SerialQueue<ExecutionSerial, VkFramebuffer> mFramebuffersToDelete;
    SerialQueue<ExecutionSerial, VkImage> mImagesToDelete;
    SerialQueue<ExecutionSerial, VkImageView> mImageViewsToDelete;
    SerialQueue<ExecutionSerial, VkPipeline> mPipelinesToDelete;
    SerialQueue<ExecutionSerial, VkPipelineLayout> mPipelineLayoutsToDelete;
    SerialQueue<ExecutionSerial, VkQueryPool> mQueryPoolsToDelete;
    SerialQueue<ExecutionSerial, VkRenderPass> mRenderPassesToDelete;
    SerialQueue<ExecutionSerial, VkSamplerYcbcrConversion> mSamplerYcbcrConversionsToDelete;
    SerialQueue<ExecutionSerial, VkSampler> mSamplersToDelete;
    SerialQueue<ExecutionSerial, VkSemaphore> mSemaphoresToDelete;
    SerialQueue<ExecutionSerial, VkShaderModule> mShaderModulesToDelete;
    SerialQueue<ExecutionSerial, VkSurfaceKHR> mSurfacesToDelete;
    SerialQueue<ExecutionSerial, VkSwapchainKHR> mSwapChainsToDelete;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_
