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

#include <utility>

namespace {
constexpr wgpu::BufferUsage getBufferObjectUsage(
        filament::backend::BufferObjectBinding bindingType) noexcept {
    switch (bindingType) {
        case filament::backend::BufferObjectBinding::VERTEX:
            return wgpu::BufferUsage::Vertex;
        case filament::backend::BufferObjectBinding::UNIFORM:
            return wgpu::BufferUsage::Uniform;
        case filament::backend::BufferObjectBinding::SHADER_STORAGE:
            return wgpu::BufferUsage::Storage;
    }
}

wgpu::Buffer createBuffer(wgpu::Device const& device, wgpu::BufferUsage usage, uint32_t size,
        char const* label) {
    wgpu::BufferDescriptor descriptor{ .label = label,
        .usage = usage,
        .size = size,
        .mappedAtCreation = false };
    return device.CreateBuffer(&descriptor);
}

wgpu::VertexFormat getVertexFormat(filament::backend::ElementType type, bool normalized, bool integer) {
    using ElementType = filament::backend::ElementType;
    using VertexFormat = wgpu::VertexFormat;
    if (normalized) {
        switch (type) {
            // Single Component Types
            case ElementType::BYTE: return VertexFormat::Snorm8;
            case ElementType::UBYTE: return VertexFormat::Unorm8;
            case ElementType::SHORT: return VertexFormat::Snorm16;
            case ElementType::USHORT: return VertexFormat::Unorm16;
            // Two Component Types
            case ElementType::BYTE2: return VertexFormat::Snorm8x2;
            case ElementType::UBYTE2: return VertexFormat::Unorm8x2;
            case ElementType::SHORT2: return VertexFormat::Snorm16x2;
            case ElementType::USHORT2: return VertexFormat::Unorm16x2;
            // Three Component Types
            // There is no vertex format type for 3 byte data in webgpu. Use
            // 4 byte signed normalized type and ignore the last byte.
            // TODO: This is to be verified.
            case ElementType::BYTE3: return VertexFormat::Snorm8x4;    // NOT MINSPEC
            case ElementType::UBYTE3: return VertexFormat::Unorm8x4;   // NOT MINSPEC
            case ElementType::SHORT3: return VertexFormat::Snorm16x4;  // NOT MINSPEC
            case ElementType::USHORT3: return VertexFormat::Unorm16x4; // NOT MINSPEC
            // Four Component Types
            case ElementType::BYTE4: return VertexFormat::Snorm8x4;
            case ElementType::UBYTE4: return VertexFormat::Unorm8x4;
            case ElementType::SHORT4: return VertexFormat::Snorm16x4;
            case ElementType::USHORT4: return VertexFormat::Unorm8x4;
            default:
                FILAMENT_CHECK_POSTCONDITION(false) << "Normalized format does not exist.";
                return VertexFormat::Float32x3;
        }
    }
    switch (type) {
        // Single Component Types
        // There is no direct alternative for SSCALED in webgpu. Convert them to Float32 directly.
        // This will result in increased memory on the cpu side.
        // TODO: Is Float16 acceptable instead with some potential accuracy errors?
        case ElementType::BYTE: return integer ? VertexFormat::Sint8 : VertexFormat::Float32;
        case ElementType::UBYTE: return integer ? VertexFormat::Uint8 : VertexFormat::Float32;
        case ElementType::SHORT: return integer ? VertexFormat::Sint16 : VertexFormat::Float32;
        case ElementType::USHORT: return integer ? VertexFormat::Uint16 : VertexFormat::Float32;
        case ElementType::HALF: return VertexFormat::Float16;
        case ElementType::INT: return VertexFormat::Sint32;
        case ElementType::UINT: return VertexFormat::Uint32;
        case ElementType::FLOAT: return VertexFormat::Float32;
        // Two Component Types
        case ElementType::BYTE2: return integer ? VertexFormat::Sint8x2 : VertexFormat::Float32x2;
        case ElementType::UBYTE2: return integer ? VertexFormat::Uint8x2 : VertexFormat::Float32x2;
        case ElementType::SHORT2: return integer ? VertexFormat::Sint16x2 : VertexFormat::Float32x2;
        case ElementType::USHORT2: return integer ? VertexFormat::Uint16x2 : VertexFormat::Float32x2;
        case ElementType::HALF2: return VertexFormat::Float16x2;
        case ElementType::FLOAT2: return VertexFormat::Float32x2;
        // Three Component Types
        case ElementType::BYTE3: return VertexFormat::Sint8x4;    // NOT MINSPEC
        case ElementType::UBYTE3: return VertexFormat::Uint8x4;   // NOT MINSPEC
        case ElementType::SHORT3: return VertexFormat::Sint16x4;  // NOT MINSPEC
        case ElementType::USHORT3: return VertexFormat::Uint16x4; // NOT MINSPEC
        case ElementType::HALF3: return VertexFormat::Float16x4;  // NOT MINSPEC
        case ElementType::FLOAT3: return VertexFormat::Float32x3;
        // Four Component Types
        case ElementType::BYTE4: return integer ? VertexFormat::Sint8x4 : VertexFormat::Float32x4;
        case ElementType::UBYTE4: return integer ? VertexFormat::Uint8x4 : VertexFormat::Float32x4;
        case ElementType::SHORT4: return integer ? VertexFormat::Sint16x4 : VertexFormat::Float32x4;
        case ElementType::USHORT4: return integer ? VertexFormat::Uint16x4 : VertexFormat::Float32x4;
        case ElementType::HALF4: return VertexFormat::Float16x4;
        case ElementType::FLOAT4: return VertexFormat::Float32x4;
    }
}

}// namespace

namespace filament::backend {

WGPUVertexBufferInfo::WGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
        AttributeArray const& attributes)
    : HwVertexBufferInfo(bufferCount, attributeCount),
      mVertexBufferLayout(bufferCount),
      mAttributes(bufferCount) {
    assert_invariant(attributeCount > 0);
    assert_invariant(bufferCount > 0);
    for (uint32_t attribIndex = 0; attribIndex < attributes.size(); attribIndex++) {
        Attribute const& attrib = attributes[attribIndex];
        // Ignore the attributes which are not bind to vertex buffers.
        if (attrib.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }

        assert_invariant(attrib.buffer < bufferCount);
        bool const isInteger = attrib.flags & Attribute::FLAG_INTEGER_TARGET;
        bool const isNormalized = attrib.flags & Attribute::FLAG_NORMALIZED;
        wgpu::VertexFormat vertexFormat = getVertexFormat(attrib.type, isNormalized, isInteger);

        // Attributes are sequential per buffer
        mAttributes[attrib.buffer].push_back({
            .format = vertexFormat,
            .offset = attrib.offset,
            .shaderLocation = static_cast<uint32_t>(mAttributes[attrib.buffer].size()),
        });

        mVertexBufferLayout[attrib.buffer].stepMode = wgpu::VertexStepMode::Vertex;
        if (mVertexBufferLayout[attrib.buffer].arrayStride == 0) {
            mVertexBufferLayout[attrib.buffer].arrayStride = attrib.stride;
        } else {
            assert_invariant(mVertexBufferLayout[attrib.buffer].arrayStride == attrib.stride);
        }
    }

    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        mVertexBufferLayout[bufferIndex].attributeCount = mAttributes[bufferIndex].size();
        mVertexBufferLayout[bufferIndex].attributes = mAttributes[bufferIndex].data();
    }
}

WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const& device, uint8_t elementSize,
        uint32_t indexCount)
    : buffer(createBuffer(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
              elementSize * indexCount, "index_buffer")),
      indexFormat(elementSize == 2 ? wgpu::IndexFormat::Uint16 : wgpu::IndexFormat::Uint32) {}


WGPUVertexBuffer::WGPUVertexBuffer(wgpu::Device const& device, uint32_t vertexCount,
        uint32_t bufferCount, Handle<HwVertexBufferInfo> vbih)
    : HwVertexBuffer(vertexCount),
      vbih(vbih),
      buffers(bufferCount) {}

WGPUBufferObject::WGPUBufferObject(wgpu::Device const& device, BufferObjectBinding bindingType,
        uint32_t byteCount)
    : HwBufferObject(byteCount),
      buffer(createBuffer(device, wgpu::BufferUsage::CopyDst | getBufferObjectUsage(bindingType),
              byteCount, "buffer_object")),
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

    // TODO: layoutDescriptor has a "Label". Ideally we can get info on what this layout is for
    // debugging. For now, hack an incrementing value.
    static int layoutNum = 0;

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

    wgpu::BindGroupLayoutDescriptor layoutDescriptor{
        // TODO: layoutDescriptor has a "Label". Ideally we can get info on what this layout is for
        // debugging. For now, hack an incrementing value.
        .label{ "layout_" + std::to_string(++layoutNum) },
        .entryCount = wEntries.size(),
        .entries = wEntries.data()
    };
    // TODO Do we need to defer this until we have more info on textures and samplers??
    mLayoutSize = wEntries.size();
    mLayout = device.CreateBindGroupLayout(&layoutDescriptor);
}

WebGPUDescriptorSetLayout::~WebGPUDescriptorSetLayout() {}

WebGPUDescriptorSet::WebGPUDescriptorSet(const wgpu::BindGroupLayout& layout, uint layoutSize)
    : mLayout(layout),
      entries(layoutSize, wgpu::BindGroupEntry{}) {
    // Establish the size of entries based on the layout. This should be reliable and efficient.
}
WebGPUDescriptorSet::~WebGPUDescriptorSet() {}

wgpu::BindGroup WebGPUDescriptorSet::lockAndReturn(const wgpu::Device& device) {
    if (mBindGroup) {
        return mBindGroup;
    }
    // TODO label? Should we just copy layout label?
    wgpu::BindGroupDescriptor desc{ .layout = mLayout,
        .entryCount = entries.size(),
        .entries = entries.data() };
    mBindGroup = device.CreateBindGroup(&desc);
    return mBindGroup;
}

void WebGPUDescriptorSet::addEntry(uint index, wgpu::BindGroupEntry&& entry) {
    if (mBindGroup) {
        // We will keep getting hits from future updates, but shouldn't do anything
        // Filament guarantees this won't change after things have locked.
        return;
    }
    // TODO: Putting some level of trust that Filament is not going to reuse indexes or go past the
    // layout index for efficiency. Add guards if wrong.
    entries[index] = std::move(entry);
}
}// namespace filament::backend
