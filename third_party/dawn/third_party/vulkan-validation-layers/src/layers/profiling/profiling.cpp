/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(TRACY_ENABLE)

#include "profiling/profiling.h"

#include "containers/custom_containers.h"
#include "vulkan/generated/dispatch_functions.h"

#include <cstdlib>
#include <thread>

#if defined(VVL_TRACY_CPU_MEMORY)

void *operator new(std ::size_t size) {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

void *operator new[](std::size_t size) {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete[](void *ptr) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
};

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
    auto ptr = malloc(size);
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete[](void *ptr, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    free(ptr);
}

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))

#if defined(_WIN32)
#define vvl_aligned_malloc(size, alignment) _aligned_malloc(size, alignment)
#define vvl_aligned_free(ptr) _aligned_free(ptr)
#else
// TODO - Need to understand why on Linux sometimes calling aligned_alloc causes corruption
void *vvl_aligned_malloc(std::size_t size, std::size_t al) {
    void *mem = malloc(size + al + sizeof(void *));
    void **ptr = (void **)((uintptr_t)((uintptr_t)mem + al + sizeof(void *)) & ~(al - 1));
    ptr[-1] = mem;
    return ptr;
}

void vvl_aligned_free(void *ptr) { free(((void **)ptr)[-1]); }
#endif

void *operator new(std::size_t size, std::align_val_t al) noexcept(false) {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}
void *operator new[](std::size_t size, std::align_val_t al) noexcept(false) {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void *operator new(std::size_t size, std::align_val_t al, const std::nothrow_t &) noexcept {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}
void *operator new[](std::size_t size, std::align_val_t al, const std::nothrow_t &) noexcept {
    auto ptr = vvl_aligned_malloc(size, size_t(al));
    VVL_TracyAlloc(ptr, size);
    return ptr;
}

void operator delete(void *ptr, std::align_val_t al) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
void operator delete[](void *ptr, std::align_val_t al) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}

void operator delete(void *ptr, std::align_val_t al, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
void operator delete[](void *ptr, std::align_val_t al, const std::nothrow_t &) noexcept {
    VVL_TracyFree(ptr);
    vvl_aligned_free(ptr);
}
#endif

#endif  // #if defined(VVL_TRACY_CPU_MEMORY)

#if TRACY_MANUAL_LIFETIME

#if defined(_WIN32)
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            tracy::StartupProfiler();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#else

__attribute__((constructor)) static void so_attach(void) { tracy::StartupProfiler(); }

#endif  // #if defined(_WIN32)

#endif  // #if TRACY_MANUAL_LIFETIME

#if defined(VVL_TRACY_GPU)

// To do things properly, should be a per device object
static std::array<TracyVkCtx, 8> tracy_vk_contexts;

TracyVkCtx &GetTracyVkCtx() {
    static std::atomic<uint32_t> context_counter = {0};
    uint32_t context_i = context_counter.fetch_add(1, std::memory_order_relaxed);
    context_i = context_i % (uint32_t)tracy_vk_contexts.size();
    assert(tracy_vk_contexts[context_i]);
    return tracy_vk_contexts[context_i];
}

static std::vector<std::unique_ptr<TracyVkCollector>> queue_to_collector_map;

PFN_vkResetCommandBuffer TracyVkCollector::ResetCommandBuffer = nullptr;
PFN_vkBeginCommandBuffer TracyVkCollector::BeginCommandBuffer = nullptr;
PFN_vkEndCommandBuffer TracyVkCollector::EndCommandBuffer = nullptr;
PFN_vkQueueSubmit TracyVkCollector::QueueSubmit = nullptr;

void TracyVkCollector::Create(VkDevice device, VkQueue queue, uint32_t queue_family_i) {
    if (std::find_if(queue_to_collector_map.begin(), queue_to_collector_map.end(),
                     [queue](const std::unique_ptr<TracyVkCollector> &collector) { return collector->queue == queue; }) !=
        queue_to_collector_map.end()) {
        return;
    }

    std::unique_ptr<TracyVkCollector> collector = std::make_unique<TracyVkCollector>();
    collector->device = device;
    collector->queue = queue;

    VkCommandPoolCreateInfo cmd_pool_ci = vku::InitStructHelper();
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_ci.queueFamilyIndex = queue_family_i;
    VkResult result = DispatchCreateCommandPool(device, &cmd_pool_ci, nullptr, &collector->cmd_pool);
    assert(result == VK_SUCCESS);

    VkCommandBufferAllocateInfo cmd_buf_ai = vku::InitStructHelper();
    cmd_buf_ai.commandPool = collector->cmd_pool;
    cmd_buf_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_ai.commandBufferCount = 1;

    result = DispatchAllocateCommandBuffers(device, &cmd_buf_ai, &collector->cmd_buf);
    assert(result == VK_SUCCESS);

    VkFenceCreateInfo fence_ci = vku::InitStructHelper();
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    result = DispatchCreateFence(device, &fence_ci, nullptr, &collector->fence);
    assert(result == VK_SUCCESS);

    (void)result;

    collector->collect_thread = std::thread([collector_ptr = collector.get()]() {
        tracy::SetThreadName("TracyVkCollector::Collect Worker");

        while (true) {
            std::unique_lock<std::mutex> collect_lock(collector_ptr->collect_mutex);
            collector_ptr->collect_cv.wait(
                collect_lock, [collector_ptr]() { return collector_ptr->should_collect || collector_ptr->should_abort; });
            if (collector_ptr->should_abort) {
                collect_lock.unlock();
                return;
            }
            collector_ptr->should_collect = false;
            collect_lock.unlock();

            // VVL_TracyMessageStream("[tracyvk] Collecting TracyVkCollector: device: " << collector_ptr->device
            //                                                                          << " queue: " << collector_ptr->queue);
            {
                VVL_ZoneScopedN("Wait for collect cmd buf fence");
                const VkResult wait_result =
                    DispatchWaitForFences(collector_ptr->device, 1, &collector_ptr->fence, VK_TRUE, 16'000'000);
                assert(wait_result == VK_SUCCESS || wait_result == VK_TIMEOUT);
                if (wait_result == VK_TIMEOUT) {
                    VVL_ZoneScopedN("Wait for collect cmd buf fence timeout - setting should_collect to true");
                    collect_lock.lock();
                    collector_ptr->should_collect = true;
                    collect_lock.unlock();
                    continue;
                }
            }
            {
                // Due to CPU calls to get query results happening in tracy_vk_ctx->Collect,
                // the collect command buffer has to be reset and prepared every time
                // ---
                VVL_ZoneScopedN("Preparing collect cmd buf");
                DispatchResetFences(collector_ptr->device, 1, &collector_ptr->fence);
                TracyVkCollector::ResetCommandBuffer(collector_ptr->cmd_buf, 0);

                VkCommandBufferBeginInfo cmd_buf_bi = vku::InitStructHelper();
                cmd_buf_bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                TracyVkCollector::BeginCommandBuffer(collector_ptr->cmd_buf, &cmd_buf_bi);
                for (TracyVkCtx &tracy_vk_ctx : tracy_vk_contexts) {
                    tracy_vk_ctx->Collect(collector_ptr->cmd_buf);
                }
                TracyVkCollector::EndCommandBuffer(collector_ptr->cmd_buf);

                // No need to guard cmd buffer creation with a mutex,
                // The VkFence is in charge of synchronizing accesses
                std::lock_guard<std::mutex> collect_cb_lock(collector_ptr->collect_cb_mutex);
                collector_ptr->collect_cb_ready = true;
            }
        }
    });

    queue_to_collector_map.emplace_back(std::move(collector));
    VVL_TracyMessageStream("[tracyvk] Added TracyVkCollector (now " << queue_to_collector_map.size()
                                                                    << " of them): device: " << device << " queue: " << queue
                                                                    << " queue family i:" << queue_family_i);
}

void TracyVkCollector::Destroy(TracyVkCollector &collector) {
    VVL_TracyMessageStream("[tracyvk] Destroying TracyVkCollector: device: " << collector.device << " queue: " << collector.queue);

    {
        std::lock_guard<std::mutex> collect_lock(collector.collect_mutex);
        collector.should_abort = true;
        collector.collect_cv.notify_one();
    }
    collector.collect_thread.join();

    if (collector.fence != VK_NULL_HANDLE) {
        DispatchDestroyFence(collector.device, collector.fence, nullptr);
        collector.fence = VK_NULL_HANDLE;
    }

    if (collector.cmd_buf != VK_NULL_HANDLE) {
        DispatchFreeCommandBuffers(collector.device, collector.cmd_pool, 1, &collector.cmd_buf);
        collector.cmd_buf = VK_NULL_HANDLE;
    }

    if (collector.cmd_pool != VK_NULL_HANDLE) {
        DispatchDestroyCommandPool(collector.device, collector.cmd_pool, nullptr);
        collector.cmd_pool = VK_NULL_HANDLE;
    }

    collector.device = VK_NULL_HANDLE;
    collector.queue = VK_NULL_HANDLE;
}

void TracyVkCollector::Collect() {
    VVL_ZoneScoped;
    std::lock_guard<std::mutex> collect_lock(collect_mutex);
    should_collect = true;
    collect_cv.notify_one();
}

std::optional<std::pair<VkCommandBuffer, VkFence>> TracyVkCollector::TryGetCollectCb(VkQueue queue) {
    if (this->queue != queue) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> collect_lock(collect_cb_mutex);
    if (collect_cb_ready) {
        collect_cb_ready = false;
        return std::make_pair(cmd_buf, fence);
    }
    return std::nullopt;
}

// Not thread safe for now, should be fine to profile GFXR traces
TracyVkCollector &TracyVkCollector::GetTracyVkCollector(VkQueue queue) {
    VVL_TracyMessageStream("[tracyvk] Getting TracyVkCollector for queue " << queue);
    for (const std::unique_ptr<TracyVkCollector> &collector : queue_to_collector_map) {
        if (collector->queue == queue) return *collector;
    }
    assert(false);
    return *queue_to_collector_map[0];
}

void TracyVkCollector::TrySubmitCollectCb(VkQueue queue) {
    VVL_ZoneScoped;
    TracyVkCollector &collector = GetTracyVkCollector(queue);

    std::optional<std::pair<VkCommandBuffer, VkFence>> collect_cb = collector.TryGetCollectCb(queue);
    if (collect_cb.has_value()) {
        VkSubmitInfo submit_info = vku::InitStructHelper();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &collect_cb->first;
        const VkResult tracy_collect_result = QueueSubmit(queue, 1, &submit_info, collect_cb->second);
        assert(tracy_collect_result);
        (void)tracy_collect_result;
    }
}

void InitTracyVk(VkInstance instance, VkPhysicalDevice gpu, VkDevice device, PFN_vkGetInstanceProcAddr GetInstanceProcAddr,
                 PFN_vkGetDeviceProcAddr GetDeviceProcAddr, PFN_vkResetCommandBuffer ResetCommandBuffer,
                 PFN_vkBeginCommandBuffer BeginCommandBuffer, PFN_vkEndCommandBuffer EndCommandBuffer,
                 PFN_vkQueueSubmit QueueSubmit) {
    TracyVkCollector::ResetCommandBuffer = ResetCommandBuffer;
    TracyVkCollector::BeginCommandBuffer = BeginCommandBuffer;
    TracyVkCollector::EndCommandBuffer = EndCommandBuffer;
    TracyVkCollector::QueueSubmit = QueueSubmit;

    for (TracyVkCtx &tracy_vk_ctx : tracy_vk_contexts) {
        tracy_vk_ctx = TracyVkContextHostCalibrated(instance, gpu, device, GetInstanceProcAddr, GetDeviceProcAddr);
        assert(tracy_vk_ctx);
    }
}

void CleanupTracyVk(VkDevice device) {
    for (size_t i = 0; i < queue_to_collector_map.size(); ++i) {
        if (queue_to_collector_map[i]->device == device) {
            TracyVkCollector::Destroy(*queue_to_collector_map[i]);
            queue_to_collector_map[i] = nullptr;
            std::swap(queue_to_collector_map[i], queue_to_collector_map[queue_to_collector_map.size() - 1]);
            queue_to_collector_map.resize(queue_to_collector_map.size() - 1);
        }
    }

    for (TracyVkCtx &tracy_vk_ctx : tracy_vk_contexts) {
        if (tracy_vk_ctx) {
            TracyVkDestroy(tracy_vk_ctx);
        }
    }
}

static vvl::concurrent_unordered_map<VkCommandBuffer, tracy::VkCtxScope *> &GetCbToCtxScopeMap() {
    static vvl::concurrent_unordered_map<VkCommandBuffer, tracy::VkCtxScope *> cb_to_ctx_scope_map;
    return cb_to_ctx_scope_map;
}

void TracyVkZoneStart(tracy::VkCtx *ctx, const tracy::SourceLocationData *srcloc, VkCommandBuffer cmd_buf) {
    VVL_ZoneScoped;
    auto ctx_scope = new tracy::VkCtxScope(ctx, srcloc, cmd_buf, true);
    GetCbToCtxScopeMap().insert(cmd_buf, ctx_scope);
}
void TracyVkZoneEnd(VkCommandBuffer cmd_buf) {
    VVL_ZoneScoped;
    auto iter = GetCbToCtxScopeMap().pop(cmd_buf);
    if (iter != GetCbToCtxScopeMap().end()) {
        assert(iter->second);
        delete iter->second;
    }
}

#endif

#endif  // #if defined(TRACY_ENABLE)
