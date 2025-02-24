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

#pragma once

#if defined(TRACY_ENABLE)
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#include "tracy/../client/TracyProfiler.hpp"

// Define CPU zones
#define VVL_ZoneScoped ZoneScoped
#define VVL_ZoneScopedN(name) ZoneScopedN(name)
#define VVL_TracyCZone(zone_name, active) TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name) TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark TracyCFrameMark

// Print messages
#define VVL_TracyMessage TracyMessage
#define VVL_TracyMessageL TracyMessageL
#define VVL_TracyPlot(name, value) TracyPlot(name, value)
#define VVL_TracyMessageStream(message)                \
    {                                                  \
        std::stringstream tracy_ss;                    \
        tracy_ss << message;                           \
        const std::string tracy_s = tracy_ss.str();    \
        TracyMessage(tracy_s.c_str(), tracy_s.size()); \
    }
#define VVL_TracyMessageMap(map, key_printer, value_printer)           \
    {                                                                  \
        static int tracy_map_log_i = 0;                                \
        std::string tracy_map_log_str = #map " ";                      \
        tracy_map_log_str += std::to_string(tracy_map_log_i++);        \
        tracy_map_log_str += " - size: ";                              \
        tracy_map_log_str += std::to_string(map.size());               \
        tracy_map_log_str += " - one pair: ";                          \
        for (const auto& [key, value] : map) {                         \
            std::string key_value_str = tracy_map_log_str;             \
            key_value_str += " | key: ";                               \
            key_value_str += key_printer(key);                         \
            key_value_str += " - value: ";                             \
            key_value_str += value_printer(value);                     \
            TracyMessage(key_value_str.c_str(), key_value_str.size()); \
        }                                                              \
    }

#else
#define VVL_ZoneScoped
#define VVL_ZoneScopedN(name)
#define VVL_TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark
#define VVL_TracyMessage
#define VVL_TracyMessageL
#define VVL_TracyPlot(name, value)
#define VVL_TracyMessageStream(message)
#define VVL_TracyMessageMap(map, key_printer, value_printer)
#endif

#if defined(VVL_TRACY_CPU_MEMORY)
#define VVL_TracyAlloc(ptr, size) TracySecureAlloc(ptr, size)
#define VVL_TracyFree(ptr) TracySecureFree(ptr)
#else
#define VVL_TracyAlloc(ptr, size)
#define VVL_TracyFree(ptr)
#endif

#if defined(VVL_TRACY_GPU)

#include <vulkan/vulkan.h>
#include "tracy/TracyVulkan.hpp"

#include <atomic>
#include <optional>

#define VVL_TracyVkZone(ctx, cmdbuf, name) TracyVkZone(ctx, cmdbuf, name)

void InitTracyVk(VkInstance instance, VkPhysicalDevice gpu, VkDevice device, PFN_vkGetInstanceProcAddr GetInstanceProcAddr,
                 PFN_vkGetDeviceProcAddr GetDeviceProcAddr, PFN_vkResetCommandBuffer ResetCommandBuffer,
                 PFN_vkBeginCommandBuffer BeginCommandBuffer, PFN_vkEndCommandBuffer EndCommandBuffer,
                 PFN_vkQueueSubmit QueueSubmit);

void CleanupTracyVk(VkDevice device);

TracyVkCtx& GetTracyVkCtx();

// One per queue
class TracyVkCollector {
  public:
    static void Create(VkDevice device, VkQueue queue, uint32_t queue_family_i);
    static void Destroy(TracyVkCollector& collector);

    static TracyVkCollector& GetTracyVkCollector(VkQueue queue);
    static void TrySubmitCollectCb(VkQueue queue);

    void Collect();
    std::optional<std::pair<VkCommandBuffer, VkFence>> TryGetCollectCb(VkQueue queue);

    static PFN_vkResetCommandBuffer ResetCommandBuffer;
    static PFN_vkBeginCommandBuffer BeginCommandBuffer;
    static PFN_vkEndCommandBuffer EndCommandBuffer;
    static PFN_vkQueueSubmit QueueSubmit;

    VkDevice device = VK_NULL_HANDLE;  // weak reference
    VkCommandPool cmd_pool = VK_NULL_HANDLE;
    VkCommandBuffer cmd_buf = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;  // weak reference
    VkFence fence = VK_NULL_HANDLE;

    bool should_abort = false;
    bool should_collect = false;
    std::mutex collect_mutex;
    std::condition_variable collect_cv;
    std::thread collect_thread;

    std::mutex collect_cb_mutex;
    bool collect_cb_ready = false;
};

void TracyVkZoneStart(tracy::VkCtx* ctx, const tracy::SourceLocationData* srcloc, VkCommandBuffer cmd_buf);
void TracyVkZoneEnd(VkCommandBuffer cmd_buf);

#define VVL_TracyVkNamedZoneStart(ctx, cmdbuf, name)                                                                               \
    static constexpr tracy::SourceLocationData TracyConcat(__tracy_gpu_source_location, TracyLine){name, TracyFunction, TracyFile, \
                                                                                                   (uint32_t)TracyLine, 0};        \
    TracyVkZoneStart(ctx, &TracyConcat(__tracy_gpu_source_location, TracyLine), cmdbuf);
#define VVL_TracyVkNamedZoneEnd(cmdbuf) TracyVkZoneEnd(cmdbuf);

#else

#define VVL_TracyVkZone(ctx, cmdbuf, name)
#define VVL_TracyVkNamedZoneStart(ctx, cmdbuf, name)
#define VVL_TracyVkNamedZoneEnd(cmdbuf)

#endif
