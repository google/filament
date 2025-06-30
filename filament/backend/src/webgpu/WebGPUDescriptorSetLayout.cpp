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

#include "WebGPUDescriptorSetLayout.h"

#include <backend/DriverEnums.h>

#include <utils/BitmaskEnum.h>
#include <utils/CString.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

namespace filament::backend {

namespace {

// Convert Filament Shader Stage Flags bitmask to webgpu equivalent
[[nodiscard]] wgpu::ShaderStage filamentStageToWGPUStage(const ShaderStageFlags fFlags) {
    wgpu::ShaderStage retStages = wgpu::ShaderStage::None;
    if (any(ShaderStageFlags::VERTEX & fFlags)) {
        retStages |= wgpu::ShaderStage::Vertex;
    }
    if (any(ShaderStageFlags::FRAGMENT & fFlags)) {
        retStages |= wgpu::ShaderStage::Fragment;
    }
    if (any(ShaderStageFlags::COMPUTE & fFlags)) {
        retStages |= wgpu::ShaderStage::Compute;
    }
    return retStages;
}

} // namespace

WebGPUDescriptorSetLayout::WebGPUDescriptorSetLayout(DescriptorSetLayout const& layout,
        wgpu::Device const& device) {
    assert_invariant(device);

    std::string baseLabel;
    if (std::holds_alternative<utils::StaticString>(layout.label)) {
        const auto& temp = std::get_if<utils::StaticString>(&layout.label);
        baseLabel = temp->c_str();
    } else if (std::holds_alternative<utils::CString>(layout.label)) {
        const auto& temp = std::get_if<utils::CString>(&layout.label);
        baseLabel = temp->c_str();
    }

    // TODO: layoutDescriptor has a "Label". Ideally we can get info on what this layout is for
    // debugging. For now, hack an incrementing value.
    static int layoutNum = 0;

    const unsigned int samplerCount =
            std::count_if(layout.bindings.begin(), layout.bindings.end(), [](auto& fEntry) {
                return DescriptorSetLayoutBinding::isSampler(fEntry.type);
            });

    std::vector<wgpu::BindGroupLayoutEntry> wEntries;
    wEntries.reserve(layout.bindings.size() + samplerCount);
    mBindGroupEntries.reserve(wEntries.capacity());

    for (auto fEntry: layout.bindings) {
        auto& wEntry = wEntries.emplace_back();
        auto& entryInfo = mBindGroupEntries.emplace_back();
        wEntry.visibility = filamentStageToWGPUStage(fEntry.stageFlags);
        wEntry.binding = fEntry.binding * 2;
        entryInfo.binding = wEntry.binding;

        switch (fEntry.type) {
            case DescriptorType::SAMPLER_2D_FLOAT:
            case DescriptorType::SAMPLER_2D_INT:
            case DescriptorType::SAMPLER_2D_UINT:
            case DescriptorType::SAMPLER_2D_DEPTH:
            case DescriptorType::SAMPLER_2D_ARRAY_FLOAT:
            case DescriptorType::SAMPLER_2D_ARRAY_INT:
            case DescriptorType::SAMPLER_2D_ARRAY_UINT:
            case DescriptorType::SAMPLER_2D_ARRAY_DEPTH:
            case DescriptorType::SAMPLER_CUBE_FLOAT:
            case DescriptorType::SAMPLER_CUBE_INT:
            case DescriptorType::SAMPLER_CUBE_UINT:
            case DescriptorType::SAMPLER_CUBE_DEPTH:
            case DescriptorType::SAMPLER_CUBE_ARRAY_FLOAT:
            case DescriptorType::SAMPLER_CUBE_ARRAY_INT:
            case DescriptorType::SAMPLER_CUBE_ARRAY_UINT:
            case DescriptorType::SAMPLER_CUBE_ARRAY_DEPTH:
            case DescriptorType::SAMPLER_3D_FLOAT:
            case DescriptorType::SAMPLER_3D_INT:
            case DescriptorType::SAMPLER_3D_UINT:
            case DescriptorType::SAMPLER_2D_MS_FLOAT:
            case DescriptorType::SAMPLER_2D_MS_INT:
            case DescriptorType::SAMPLER_2D_MS_UINT:
            case DescriptorType::SAMPLER_2D_MS_ARRAY_FLOAT:
            case DescriptorType::SAMPLER_2D_MS_ARRAY_INT:
            case DescriptorType::SAMPLER_2D_MS_ARRAY_UINT: {
                auto& samplerEntry = wEntries.emplace_back();
                auto& samplerEntryInfo = mBindGroupEntries.emplace_back();
                samplerEntry.binding = fEntry.binding * 2 + 1;
                samplerEntryInfo.binding = samplerEntry.binding;
                samplerEntry.visibility = wEntry.visibility;
                wEntry.texture.multisampled = isMultiSampledTypeDescriptor(fEntry.type);
                if (isDepthDescriptor(fEntry.type)) {
                    samplerEntry.sampler.type = wgpu::SamplerBindingType::Comparison;
                } else if (isIntDescriptor(fEntry.type) ||
                        any(fEntry.flags & DescriptorFlags::UNFILTERABLE)) {
                    samplerEntry.sampler.type = wgpu::SamplerBindingType::NonFiltering;
                } else {
                    samplerEntry.sampler.type = wgpu::SamplerBindingType::Filtering;
                }
                break;
            }
            case DescriptorType::UNIFORM_BUFFER: {
                wEntry.buffer.hasDynamicOffset =
                        any(fEntry.flags & DescriptorFlags::DYNAMIC_OFFSET);
                entryInfo.hasDynamicOffset = wEntry.buffer.hasDynamicOffset;
                wEntry.buffer.type = wgpu::BufferBindingType::Uniform;
                // TODO: Ideally we fill minBindingSize
                break;
            }
            case DescriptorType::INPUT_ATTACHMENT: {
                PANIC_POSTCONDITION("Input Attachment is not supported");
                break;
            }
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                PANIC_POSTCONDITION("Shader storage is not supported");
                break;
            }
            case DescriptorType::SAMPLER_EXTERNAL: {
                PANIC_POSTCONDITION("External Sampler is not supported");
                break;
            }
        }
        if (isDepthDescriptor(fEntry.type)) {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Depth;
        } else if (isFloatDescriptor(fEntry.type)) {
            if (any(fEntry.flags & DescriptorFlags::UNFILTERABLE)) {
                wEntry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
            } else {
                wEntry.texture.sampleType = wgpu::TextureSampleType::Float;
            }
        } else if (isIntDescriptor(fEntry.type)) {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Sint;
        } else if (isUnsignedIntDescriptor(fEntry.type)) {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Uint;
        }

        if (is3dTypeDescriptor(fEntry.type)) {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e3D;
        } else if (is2dTypeDescriptor(fEntry.type)) {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        } else if (is2dArrayTypeDescriptor(fEntry.type)) {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        } else if (isCubeTypeDescriptor(fEntry.type)) {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        } else if (isCubeArrayTypeDescriptor(fEntry.type)) {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::CubeArray;
        }
        // fEntry.count is unused currently
    }
    std::string label =  "layout_" + baseLabel + std::to_string(++layoutNum) ;
    const wgpu::BindGroupLayoutDescriptor layoutDescriptor{
        .label{label.c_str()}, // Use .c_str() if label needs to be const char*
        .entryCount = wEntries.size(),
        .entries = wEntries.data()
    };
    mLayout = device.CreateBindGroupLayout(&layoutDescriptor);
    FILAMENT_CHECK_POSTCONDITION(mLayout)
            << "Failed to create bind group layout with label " << label;
}

} // namespace filament::backend
