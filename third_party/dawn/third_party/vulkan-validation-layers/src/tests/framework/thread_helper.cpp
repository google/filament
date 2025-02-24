/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "thread_helper.h"

bool ThreadTimeoutHelper::WaitForThreads(int timeout_in_seconds) {
    std::unique_lock lock(mutex_);
    return cv_.wait_for(lock, std::chrono::seconds{timeout_in_seconds}, [this] {
        std::lock_guard lock_guard(active_thread_mutex_);
        return active_threads_ == 0;
    });
}

void ThreadTimeoutHelper::OnThreadDone() {
    bool last_worker = false;
    {
        std::lock_guard lock(active_thread_mutex_);
        active_threads_--;
        assert(active_threads_ >= 0);
        if (!active_threads_) {
            last_worker = true;
        }
    }
    if (last_worker) {
        cv_.notify_one();
    }
}

#if GTEST_IS_THREADSAFE
void AddToCommandBuffer(ThreadTestData *data) {
    for (int i = 0; i < 80000; i++) {
        vk::CmdSetEvent(data->commandBuffer, data->event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        if (*data->bailout) {
            break;
        }
    }
}

void UpdateDescriptor(ThreadTestData *data) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = data->buffer;
    buffer_info.offset = 0;
    buffer_info.range = 1;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = data->descriptorSet;
    descriptor_write.dstBinding = data->binding;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    for (int i = 0; i < 80000; i++) {
        vk::UpdateDescriptorSets(data->device, 1, &descriptor_write, 0, NULL);
        if (*data->bailout) {
            break;
        }
    }
}

#endif  // GTEST_IS_THREADSAFE

void ReleaseNullFence(ThreadTestData *data) {
    for (int i = 0; i < 40000; i++) {
        vk::DestroyFence(data->device, VK_NULL_HANDLE, NULL);
        if (*data->bailout) {
            break;
        }
    }
}
