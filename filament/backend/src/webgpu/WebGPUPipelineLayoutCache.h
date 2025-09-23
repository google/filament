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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUPIPELINELAYOUTCACHE_H
#define TNT_FILAMENT_BACKEND_WEBGPUPIPELINELAYOUTCACHE_H

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/Hash.h>

#include <tsl/robin_map.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <type_traits>

namespace filament::backend {

/**
 * A cache for WebGPU pipeline layouts.
 * This class is responsible for creating and caching wgpu::PipelineLayout objects to avoid
 * expensive pipeline layout creation at runtime.
 */
class WebGPUPipelineLayoutCache final {
public:
    struct PipelineLayoutRequest final {
        utils::CString const& label;
        std::array<wgpu::BindGroupLayout, MAX_DESCRIPTOR_SET_COUNT> const& bindGroupLayouts;
        size_t bindGroupLayoutCount;
    };

    explicit WebGPUPipelineLayoutCache(wgpu::Device const&);
    WebGPUPipelineLayoutCache(WebGPUPipelineLayoutCache const&) = delete;
    WebGPUPipelineLayoutCache(WebGPUPipelineLayoutCache const&&) = delete;
    WebGPUPipelineLayoutCache& operator=(WebGPUPipelineLayoutCache const&) = delete;
    WebGPUPipelineLayoutCache& operator=(WebGPUPipelineLayoutCache const&&) = delete;

    /**
     * Retrieves a pipeline layout from the cache or creates a new one if it doesn't exist.
     * @return A constant reference to the cached or newly created pipeline layout.
     */
    [[nodiscard]] wgpu::PipelineLayout const& getOrCreatePipelineLayout(
            PipelineLayoutRequest const&);

    /**
     * Should be called at the end of each frame to perform cache maintenance.
     */
    void onFrameEnd();

private:
    /**
     * Key designed for efficient hashing and uniquely identifying all the parameters for
     * creating a pipeline layout.
     * The efficient hashing requires a small memory footprint
     * (using the smallest representations of enums, just handle instances instead of wrapper class
     *  instances, single bytes for booleans etc.), trivial copying and comparison (byte by byte),
     *  and a word-aligned structure with a size in bytes as a multiple of 4 (for murmer hash).
     */
    struct PipelineLayoutKey final {
        WGPUBindGroupLayout bindGroupLayoutHandles[MAX_DESCRIPTOR_SET_COUNT]{ nullptr }; // 32 :0
        uint8_t bindGroupLayoutCount{ 0 };                                               // 1  :32
        uint8_t padding[7]{ 0 };                                                         // 7  :33
    };
    static_assert(sizeof(PipelineLayoutKey) == 40,
            "PipelineLayoutKey must not have implicit padding.");
    static_assert(std::is_trivially_copyable<PipelineLayoutKey>::value,
            "PipelineLayoutKey must be a trivially copyable POD for fast hashing.");

    struct PipelineLayoutKeyEqual {
        bool operator()(PipelineLayoutKey const&, PipelineLayoutKey const&) const;
    };

    struct PipelineLayoutCacheEntry final {
        wgpu::PipelineLayout layout{ nullptr };
        uint64_t lastUsedFrameCount{ 0 };
    };

    static void populateKey(PipelineLayoutRequest const&, PipelineLayoutKey& outKey);

    [[nodiscard]] wgpu::PipelineLayout createPipelineLayout(PipelineLayoutRequest const&);

    void removeExpiredPipelineLayouts();

    wgpu::Device mDevice;
    tsl::robin_map<PipelineLayoutKey, PipelineLayoutCacheEntry,
            utils::hash::MurmurHashFn<PipelineLayoutKey>, PipelineLayoutKeyEqual>
            mPipelineLayouts{};
    uint64_t mFrameCount{ 0 };
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUPIPELINELAYOUTCACHE_H
