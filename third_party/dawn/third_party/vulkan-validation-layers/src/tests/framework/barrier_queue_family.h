/*
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"

class BarrierQueueFamilyBase {
  public:
    struct QueueFamilyObjs {
        uint32_t index;
        // We would use std::unique_ptr, but this triggers a compiler error on older compilers
        vkt::Queue *queue = nullptr;
        vkt::CommandPool *command_pool = nullptr;
        vkt::CommandBuffer *command_buffer = nullptr;
        vkt::CommandBuffer *command_buffer2 = nullptr;
        ~QueueFamilyObjs();
        void Init(vkt::Device *device, uint32_t qf_index, VkQueue qf_queue, VkCommandPoolCreateFlags cp_flags);
    };

    struct Context {
        VkLayerTest *layer_test;
        uint32_t default_index;
        std::unordered_map<uint32_t, QueueFamilyObjs> queue_families;
        Context(VkLayerTest *test, const std::vector<uint32_t> &queue_family_indices);
        void Reset();
    };

    BarrierQueueFamilyBase(Context *context) : context_(context) {}

    QueueFamilyObjs *GetQueueFamilyInfo(Context *context, uint32_t qfi);

    enum Modifier {
        NONE,
        DOUBLE_RECORD,
        DOUBLE_COMMAND_BUFFER,
    };

    static const uint32_t kInvalidQueueFamily = vvl::kU32Max;
    Context *context_;
    vkt::Image image_;
    vkt::Buffer buffer_;
};

class BarrierQueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    BarrierQueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(std::vector<uint32_t> *families, bool image_memory = true, bool buffer_memory = true);

    void operator()(const std::string &img_err, const std::string &buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, uint32_t queue_family_index = kInvalidQueueFamily,
                    Modifier mod = Modifier::NONE);

    void operator()(uint32_t src = VK_QUEUE_FAMILY_IGNORED, uint32_t dst = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE) {
        (*this)("", "", src, dst, queue_family_index, mod);
    }

    VkImageMemoryBarrier image_barrier_;
    VkBufferMemoryBarrier buffer_barrier_;
};

// TODO - Only works with extensions enabled, not using Vulkan1.3 (uses KHR functions)
class Barrier2QueueFamilyTestHelper : public BarrierQueueFamilyBase {
  public:
    Barrier2QueueFamilyTestHelper(Context *context) : BarrierQueueFamilyBase(context) {}
    // Init with queue families non-null for CONCURRENT sharing mode (which requires them)
    void Init(bool image_memory = true, bool buffer_memory = true);

    void operator()(const std::string &img_err, const std::string &buf_err = "", uint32_t src = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t dst = VK_QUEUE_FAMILY_IGNORED, uint32_t queue_family_index = kInvalidQueueFamily,
                    Modifier mod = Modifier::NONE);

    void operator()(uint32_t src = VK_QUEUE_FAMILY_IGNORED, uint32_t dst = VK_QUEUE_FAMILY_IGNORED,
                    uint32_t queue_family_index = kInvalidQueueFamily, Modifier mod = Modifier::NONE) {
        (*this)("", "", src, dst, queue_family_index, mod);
    }

    VkImageMemoryBarrier2 image_barrier_;
    VkBufferMemoryBarrier2 buffer_barrier_;
};

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::Queue *queue, vkt::CommandBuffer &cb, VkPipelineStageFlags src_stages,
                              VkPipelineStageFlags dst_stages, const VkBufferMemoryBarrier *buf_barrier,
                              const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransferOp(ErrorMonitor *monitor, vkt::Queue *queue, vkt::CommandBuffer &cb,
                              const VkBufferMemoryBarrier2 *buf_barrier, const VkImageMemoryBarrier2 *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::Queue *queue_from, vkt::CommandBuffer &cb_from, vkt::Queue *queue_to,
                            vkt::CommandBuffer &cb_to, VkPipelineStageFlags src_stages, VkPipelineStageFlags dst_stages,
                            const VkBufferMemoryBarrier *buf_barrier, const VkImageMemoryBarrier *img_barrier);

void ValidOwnershipTransfer(ErrorMonitor *monitor, vkt::Queue *queue_from, vkt::CommandBuffer &cb_from, vkt::Queue *queue_to,
                            vkt::CommandBuffer &cb_to, const VkBufferMemoryBarrier2 *buf_barrier,
                            const VkImageMemoryBarrier2 *img_barrier);
