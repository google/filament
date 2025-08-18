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

#include "WebGPUPipelineLayoutCache.h"

#include "WebGPUConstants.h"

#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstring>

namespace filament::backend {

WebGPUPipelineLayoutCache::WebGPUPipelineLayoutCache(wgpu::Device const& device)
    : mDevice{ device } {}

wgpu::PipelineLayout const& WebGPUPipelineLayoutCache::getOrCreatePipelineLayout(
        PipelineLayoutRequest const& request) {
    PipelineLayoutKey key{};
    populateKey(request, key);
    if (auto iterator{ mPipelineLayouts.find(key) }; iterator != mPipelineLayouts.end()) {
        PipelineLayoutCacheEntry& entry{ iterator.value() };
        entry.lastUsedFrameCount = mFrameCount;
        return entry.layout;
    }
    const wgpu::PipelineLayout layout{ createPipelineLayout(request) };
    mPipelineLayouts.emplace(key, PipelineLayoutCacheEntry{
                                      .layout = layout,
                                      .lastUsedFrameCount = mFrameCount,
                                  });
    return mPipelineLayouts[key].layout;
}

void WebGPUPipelineLayoutCache::onFrameEnd() {
    ++mFrameCount;
    removeExpiredPipelineLayouts();
}

void WebGPUPipelineLayoutCache::populateKey(PipelineLayoutRequest const& request,
        PipelineLayoutKey& outKey) {
    outKey.bindGroupLayoutCount = static_cast<uint8_t>(request.bindGroupLayoutCount);
    for (size_t bindGroupIndex{ 0 }; bindGroupIndex < request.bindGroupLayoutCount;
            ++bindGroupIndex) {
        outKey.bindGroupLayoutHandles[bindGroupIndex] =
                request.bindGroupLayouts[bindGroupIndex]
                        ? request.bindGroupLayouts[bindGroupIndex].Get()
                        : nullptr;
    }
}

wgpu::PipelineLayout WebGPUPipelineLayoutCache::createPipelineLayout(
        PipelineLayoutRequest const& request) {
    const wgpu::PipelineLayoutDescriptor descriptor{
        .label = wgpu::StringView(request.label.c_str_safe()),
        .bindGroupLayoutCount = request.bindGroupLayoutCount,
        .bindGroupLayouts = request.bindGroupLayouts.data(),
        // TODO investigate immediateDataRangeByteSize
    };
    const wgpu::PipelineLayout layout{ mDevice.CreatePipelineLayout(&descriptor) };
    FILAMENT_CHECK_POSTCONDITION(layout)
            << "Failed to create pipeline layout " << descriptor.label << ".";
    return layout;
}

bool WebGPUPipelineLayoutCache::PipelineLayoutKeyEqual::operator()(PipelineLayoutKey const& key1,
        PipelineLayoutKey const& key2) const {
    // Compare the raw bytes of the keys for equality.
    return 0 == memcmp(reinterpret_cast<void const*>(&key1), reinterpret_cast<void const*>(&key2),
                        sizeof(key1));
}

void WebGPUPipelineLayoutCache::removeExpiredPipelineLayouts() {
    using Iterator = decltype(mPipelineLayouts)::const_iterator;
    for (Iterator iterator{ mPipelineLayouts.begin() }; iterator != mPipelineLayouts.end();) {
        PipelineLayoutCacheEntry const& entry{ iterator.value() };
        if (mFrameCount > (entry.lastUsedFrameCount +
                                  FILAMENT_WEBGPU_PIPELINE_LAYOUT_EXPIRATION_IN_FRAME_COUNT)) {
            // The pipeline layout has not been used recently, so we can remove it from the cache.
            iterator = mPipelineLayouts.erase(iterator);
        } else {
            ++iterator;
        }
    }
}

} // namespace filament::backend
