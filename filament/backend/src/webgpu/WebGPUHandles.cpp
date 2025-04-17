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

#include "WebGPUHandles.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Program.h>

#include <utils/BitmaskEnum.h>
#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include <sstream>
#include <string> // for std::to_string

namespace {

wgpu::Buffer createIndexBuffer(wgpu::Device const& device, uint8_t elementSize, uint32_t indexCount) {
    wgpu::BufferDescriptor descriptor{ .label = "index_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
        .size = elementSize * indexCount,
        .mappedAtCreation = false };
    return device.CreateBuffer(&descriptor);
}

wgpu::ShaderModule createShaderModuleFromWgsl(wgpu::Device& device, const char* programName,
        std::string_view shaderType, utils::FixedCapacityVector<uint8_t> const& wgslSource) {
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = wgpu::StringView(reinterpret_cast<const char*>(wgslSource.data()));
    std::stringstream labelStream;
    labelStream << programName << "_" << shaderType << "_shader";
    wgpu::ShaderModuleDescriptor descriptor{
        .nextInChain = &wgslDescriptor,
        .label = labelStream.str().data()
    };
    return device.CreateShaderModule(&descriptor);
}

wgpu::ShaderModule createVertexShaderModule(const char* programName, wgpu::Device& device,
        utils::FixedCapacityVector<uint8_t> const& source) {
    if (UTILS_UNLIKELY(source.empty())) {
        return nullptr;// null handle
    }
    return createShaderModuleFromWgsl(device, programName, "vertex", source);
}

wgpu::ShaderModule createFragmentShaderModule(const char* programName, wgpu::Device& device,
        utils::FixedCapacityVector<uint8_t> const& source) {
    if (source.empty()) {
        return nullptr;// null handle
    }
    return createShaderModuleFromWgsl(device, programName, "fragment", source);
}

wgpu::ShaderModule createComputeShaderModule(const char* programName, wgpu::Device& device,
        utils::FixedCapacityVector<uint8_t> const& source) {
    if (source.empty()) {
        return nullptr;// null handle
    }
    return createShaderModuleFromWgsl(device, programName, "compute", source);
}

} // namespace

namespace filament::backend {

WGPUProgram::WGPUProgram(wgpu::Device& device, Program& program)
    : HwProgram(program.getName()),
      vertexShaderModule(createVertexShaderModule(name.c_str_safe(), device,
              program.getShadersSource()[static_cast<size_t>(ShaderStage::VERTEX)])),
      fragmentShaderModule(createFragmentShaderModule(name.c_str_safe(), device,
              program.getShadersSource()[static_cast<size_t>(ShaderStage::FRAGMENT)])),
      computeShaderModule(createComputeShaderModule(name.c_str_safe(), device,
              program.getShadersSource()[static_cast<size_t>(ShaderStage::COMPUTE)])) {}

WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const& device, uint8_t elementSize,
        uint32_t indexCount)
    : buffer(createIndexBuffer(device, elementSize, indexCount)) {}


WGPUVertexBuffer::WGPUVertexBuffer(wgpu::Device const &device, uint32_t vextexCount, uint32_t bufferCount,
                                   Handle<WGPUVertexBufferInfo> vbih)
        : HwVertexBuffer(vextexCount),
          vbih(vbih),
          buffers(bufferCount) {
    wgpu::BufferDescriptor descriptor {
            .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
            .size = vextexCount * bufferCount,
            .mappedAtCreation = false };

    for (uint32_t i = 0; i < bufferCount; ++i) {
        descriptor.label = ("vertex_buffer_" + std::to_string(i)).c_str();
        buffers[i] = device.CreateBuffer(&descriptor);
    }
}

// TODO: Empty function is a place holder for verxtex buffer updates and should be
// updated for that purpose.
void WGPUVertexBuffer::setBuffer(WGPUBufferObject* bufferObject, uint32_t index) {}

WGPUBufferObject::WGPUBufferObject(BufferObjectBinding bindingType, uint32_t byteCount)
    : HwBufferObject(byteCount),
      bufferObjectBinding(bindingType) {}

wgpu::ShaderStage WebGPUDescriptorSetLayout::filamentStageToWGPUStage(ShaderStageFlags fFlags) {
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

WebGPUDescriptorSetLayout::WebGPUDescriptorSetLayout(DescriptorSetLayout const& layout,
        wgpu::Device const& device) {
    assert_invariant(device);

//    // TODO: layoutDescriptor has a "Label". Ideally we can get info on what this layout is for
//    // debugging. For now, hack an incrementing value.
//    static int layoutNum = 0;

    uint samplerCount =
            std::count_if(layout.bindings.begin(), layout.bindings.end(), [](auto& fEntry) {
                return fEntry.type == DescriptorType::SAMPLER ||
                       fEntry.type == DescriptorType::SAMPLER_EXTERNAL;
            });


    std::vector<wgpu::BindGroupLayoutEntry> wEntries;
    wEntries.reserve(layout.bindings.size() + samplerCount);

    for (auto fEntry: layout.bindings) {
        auto& wEntry = wEntries.emplace_back();
        wEntry.visibility = filamentStageToWGPUStage(fEntry.stageFlags);
        wEntry.binding = fEntry.binding * 2;

        switch (fEntry.type) {
            // TODO Metal treats these the same. Is this fine?
            case DescriptorType::SAMPLER_EXTERNAL:
            case DescriptorType::SAMPLER: {
                // Sampler binding is 2n+1 due to split.
                auto& samplerEntry = wEntries.emplace_back();
                samplerEntry.binding = fEntry.binding * 2 + 1;
                samplerEntry.visibility = wEntry.visibility;
                // We are simply hoping that undefined and defaults suffices here.
                samplerEntry.sampler.type = wgpu::SamplerBindingType::Undefined;
                wEntry.texture.sampleType = wgpu::TextureSampleType::Undefined;
                break;
            }
            case DescriptorType::UNIFORM_BUFFER: {
                wEntry.buffer.hasDynamicOffset =
                        any(fEntry.flags & DescriptorFlags::DYNAMIC_OFFSET);
                wEntry.buffer.type = wgpu::BufferBindingType::Uniform;
                // TODO: Ideally we fill minBindingSize
                break;
            }

            case DescriptorType::INPUT_ATTACHMENT: {
                // TODO: support INPUT_ATTACHMENT. Metal does not currently.
                PANIC_POSTCONDITION("Input Attachment is not supported");
                break;
            }

            case DescriptorType::SHADER_STORAGE_BUFFER: {
                // TODO: Vulkan does not support this, can we?
                PANIC_POSTCONDITION("Shader storage is not supported");
                break;
            }
        }

        // Currently flags are only used to specify dynamic offset.

        // UNUSED
        // fEntry.count
    }

//    wgpu::BindGroupLayoutDescriptor layoutDescriptor{
//        // TODO: layoutDescriptor has a "Label". Ideally we can get info on what this layout is for
//        // debugging. For now, hack an incrementing value.
//        .label{ "layout_" + std::to_string(++layoutNum) },
//        .entryCount = wEntries.size(),
//        .entries = wEntries.data()
//    };
//    // TODO Do we need to defer this until we have more info on textures and samplers??
//    mLayout = device.CreateBindGroupLayout(&layoutDescriptor);
}
WebGPUDescriptorSetLayout::~WebGPUDescriptorSetLayout() {}
}// namespace filament::backend
