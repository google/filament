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

wgpu::Buffer createIndexBuffer(wgpu::Device const& device, uint8_t elementSize, uint32_t indexCount) {
    wgpu::BufferDescriptor descriptor{ .label = "index_buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
        .size = elementSize * indexCount,
        .mappedAtCreation = false };
    return device.CreateBuffer(&descriptor);
}

wgpu::VertexFormat getVertexFormat(filament::backend::ElementType type, bool normalized, bool integer) {
    using ElementType = filament::backend::ElementType;
    // using VertexFormat = wgpu::VertexFormat;
    if (normalized) {
        switch (type) {
            // Single Component Types
            case ElementType::BYTE: return wgpu::VertexFormat::Snorm8;
            case ElementType::UBYTE: return wgpu::VertexFormat::Unorm8;
            case ElementType::SHORT: return wgpu::VertexFormat::Snorm16;
            case ElementType::USHORT: return wgpu::VertexFormat::Unorm16;
            // Two Component Types
            case ElementType::BYTE2: return wgpu::VertexFormat::Snorm8x2;
            case ElementType::UBYTE2: return wgpu::VertexFormat::Unorm8x2;
            case ElementType::SHORT2: return wgpu::VertexFormat::Snorm16x2;
            case ElementType::USHORT2: return wgpu::VertexFormat::Unorm16x2;
            // Three Component Types
            // There is no vertex format type for 3 byte data in webgpu. Use
            // 4 byte signed normalized type and ignore the last byte.
            // TODO: This is to be verified.
            case ElementType::BYTE3: return wgpu::VertexFormat::Snorm8x4;      // NOT MINSPEC
            case ElementType::UBYTE3: return wgpu::VertexFormat::Unorm8x4;     // NOT MINSPEC
            case ElementType::SHORT3: return wgpu::VertexFormat::Snorm16x4;  // NOT MINSPEC
            case ElementType::USHORT3: return wgpu::VertexFormat::Unorm16x4; // NOT MINSPEC
            // Four Component Types
            case ElementType::BYTE4: return wgpu::VertexFormat::Snorm8x4;
            case ElementType::UBYTE4: return wgpu::VertexFormat::Unorm8x4;
            case ElementType::SHORT4: return wgpu::VertexFormat::Snorm16x4;
            case ElementType::USHORT4: return wgpu::VertexFormat::Unorm8x4;
            default:
                FILAMENT_CHECK_POSTCONDITION(false) << "Normalized format does not exist.";
                return wgpu::VertexFormat::Float32x3;
        }
    }
    switch (type) {
        // Single Component Types
        // There is no direct alternative for SSCALED in webgpu. Convert them to Float32 directly.
        // This will result in increased memory on the cpu side.
        // TODO: Is Float16 acceptable instead with some potential accuracy errors?
        case ElementType::BYTE: return integer ? wgpu::VertexFormat::Sint8 : wgpu::VertexFormat::Float32;
        case ElementType::UBYTE: return integer ? wgpu::VertexFormat::Uint8 : wgpu::VertexFormat::Float32;
        case ElementType::SHORT: return integer ? wgpu::VertexFormat::Sint16 : wgpu::VertexFormat::Float32;
        case ElementType::USHORT: return integer ? wgpu::VertexFormat::Uint16 : wgpu::VertexFormat::Float32;
        case ElementType::HALF: return wgpu::VertexFormat::Float16;
        case ElementType::INT: return wgpu::VertexFormat::Sint32;
        case ElementType::UINT: return wgpu::VertexFormat::Uint32;
        case ElementType::FLOAT: return wgpu::VertexFormat::Float32;
        // Two Component Types
        case ElementType::BYTE2: return integer ? wgpu::VertexFormat::Sint8x2 : wgpu::VertexFormat::Float32x2;
        case ElementType::UBYTE2: return integer ? wgpu::VertexFormat::Uint8x2 : wgpu::VertexFormat::Float32x2;
        case ElementType::SHORT2: return integer ? wgpu::VertexFormat::Sint16x2 : wgpu::VertexFormat::Float32x2;
        case ElementType::USHORT2: return integer ? wgpu::VertexFormat::Uint16x2 : wgpu::VertexFormat::Float32x2;
        case ElementType::HALF2: return wgpu::VertexFormat::Float16x2;
        case ElementType::FLOAT2: return wgpu::VertexFormat::Float32x2;
        // Three Component Types
        case ElementType::BYTE3: return wgpu::VertexFormat::Sint8x4;      // NOT MINSPEC
        case ElementType::UBYTE3: return wgpu::VertexFormat::Uint8x4;     // NOT MINSPEC
        case ElementType::SHORT3: return wgpu::VertexFormat::Sint16x4;  // NOT MINSPEC
        case ElementType::USHORT3: return wgpu::VertexFormat::Uint16x4; // NOT MINSPEC
        case ElementType::HALF3: return wgpu::VertexFormat::Float16x4; // NOT MINSPEC
        case ElementType::FLOAT3: return wgpu::VertexFormat::Float32x3;
        // Four Component Types
        case ElementType::BYTE4: return integer ? wgpu::VertexFormat::Sint8x4 : wgpu::VertexFormat::Float32x4;
        case ElementType::UBYTE4: return integer ? wgpu::VertexFormat::Uint8x4 : wgpu::VertexFormat::Float32x4;
        case ElementType::SHORT4: return integer ? wgpu::VertexFormat::Sint16x4 : wgpu::VertexFormat::Float32x4;
        case ElementType::USHORT4: return integer ? wgpu::VertexFormat::Uint16x4 : wgpu::VertexFormat::Float32x4;
        case ElementType::HALF4: return wgpu::VertexFormat::Float16x4;
        case ElementType::FLOAT4: return wgpu::VertexFormat::Float32x4;
    }
    FILAMENT_CHECK_POSTCONDITION(false) << "Vertex format should always be defined.";
    return wgpu::VertexFormat::Float32x3;
}

} // namespace

namespace filament::backend {

WGPUVertexBufferInfo::WGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
        AttributeArray const& attributes)
    : HwVertexBufferInfo(bufferCount, attributeCount),
      mVertexBufferLayout(bufferCount),
      mAttributes(bufferCount) {
    for (uint32_t attribIndex = 0; attribIndex < attributes.size(); attribIndex++) {
        Attribute attrib = attributes[attribIndex];
        // Ignore the attributes which are not bind to vertex buffers.
        if (attrib.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }

        assert_invariant(attrib.buffer < bufferCount);
        bool const isInteger = attrib.flags & Attribute::FLAG_INTEGER_TARGET;
        bool const isNormalized = attrib.flags & Attribute::FLAG_NORMALIZED;
        wgpu::VertexFormat vertexFormat = getVertexFormat(attrib.type, isNormalized, isInteger);

        mAttributes[attrib.buffer].push_back({
            .format = vertexFormat,
            .offset = attrib.offset,
            .shaderLocation = attribIndex,
        });
        mVertexBufferLayout[attrib.buffer] = {
            .arrayStride = attrib.stride,
        };
    }

    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        mVertexBufferLayout[bufferIndex] = {
            .attributeCount = mAttributes[bufferIndex].size(),
            .attributes = mAttributes[bufferIndex].data(),
        };
    }
}

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
        wgpu::Device const* device) {
    assert_invariant(device->Get());

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
    mLayout = device->CreateBindGroupLayout(&layoutDescriptor);
}
WebGPUDescriptorSetLayout::~WebGPUDescriptorSetLayout() {}
}// namespace filament::backend
