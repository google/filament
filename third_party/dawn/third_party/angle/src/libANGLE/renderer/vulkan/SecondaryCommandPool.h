//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SecondaryCommandPool:
//    A class for allocating Command Buffers for VulkanSecondaryCommandBuffer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDPOOL_H_
#define LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDPOOL_H_

#include "common/FixedQueue.h"
#include "common/SimpleMutex.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{
namespace vk
{
class ErrorContext;
class VulkanSecondaryCommandBuffer;

// VkCommandPool must be externally synchronized when its Command Buffers are: allocated, freed,
// reset, or recorded. This class ensures that Command Buffers are freed from the thread that
// recording commands (Context thread).
class SecondaryCommandPool final : angle::NonCopyable
{
  public:
    SecondaryCommandPool();
    ~SecondaryCommandPool();

    angle::Result init(ErrorContext *context,
                       uint32_t queueFamilyIndex,
                       ProtectionType protectionType);
    void destroy(VkDevice device);

    bool valid() const { return mCommandPool.valid(); }

    // Call only from the Context thread that owns the SecondaryCommandPool instance.
    angle::Result allocate(ErrorContext *context, VulkanSecondaryCommandBuffer *buffer);

    // Single threaded - use external synchronization.
    void collect(VulkanSecondaryCommandBuffer *buffer);

  private:
    static constexpr size_t kFixedQueueLimit = 100u;

    // Context thread access members.

    void freeCollectedBuffers(VkDevice device);

    // Use single pool for now.
    CommandPool mCommandPool;

    // Other thread access members.

    // Fast lock free queue for processing buffers while new may be added from the other thread.
    angle::FixedQueue<VkCommandBuffer> mCollectedBuffers;

    // Overflow vector to use in cases when FixedQueue is filled.
    std::vector<VkCommandBuffer> mCollectedBuffersOverflow;
    angle::SimpleMutex mOverflowMutex;
    std::atomic<bool> mHasOverflow;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDPOOL_H_
