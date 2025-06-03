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

#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>
#include <private/backend/BackendUtils.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

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
            case ElementType::USHORT4: return VertexFormat::Unorm16x4;
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

wgpu::StringView getUserTextureLabel(filament::backend::SamplerType target) {
    // TODO will be helpful to get more useful info than this
    using filament::backend::SamplerType;
    switch (target) {
        case SamplerType::SAMPLER_2D:
            return "a_2D_user_texture";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "a_2D_array_user_texture";
        case SamplerType::SAMPLER_CUBEMAP:
            return "a_cube_map_user_texture";
        case SamplerType::SAMPLER_EXTERNAL:
            return "an_external_user_texture";
        case SamplerType::SAMPLER_3D:
            return "a_3D_user_texture";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "a_cube_map_array_user_texture";
    }
}

wgpu::StringView getUserTextureViewLabel(filament::backend::SamplerType target) {
    // TODO will be helpful to get more useful info than this
    using filament::backend::SamplerType;
    switch (target) {
        case SamplerType::SAMPLER_2D:
            return "a_2D_user_texture_view";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "a_2D_array_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP:
            return "a_cube_map_user_texture_view";
        case SamplerType::SAMPLER_EXTERNAL:
            return "an_external_user_texture_view";
        case SamplerType::SAMPLER_3D:
            return "a_3D_user_texture_view";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "a_cube_map_array_user_texture_view";
    }
}

}// namespace

namespace filament::backend {

void WGPUBufferBase::createBuffer(const wgpu::Device& device, wgpu::BufferUsage usage,
        uint32_t size, const char* label) {
    // Write size must be divisible by 4. If the whole buffer is written to as is common, so must
    // the buffer size.
    size += (4 - (size % 4)) % 4;
    wgpu::BufferDescriptor descriptor{ .label = label,
        .usage = usage,
        .size = size,
        .mappedAtCreation = false };
    buffer = device.CreateBuffer(&descriptor);
}

void WGPUBufferBase::updateGPUBuffer(BufferDescriptor& bufferDescriptor, uint32_t byteOffset,
        wgpu::Queue queue) {
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.buffer)
            << "copyIntoBuffer called with a null buffer";
    FILAMENT_CHECK_PRECONDITION(bufferDescriptor.size + byteOffset <= buffer.GetSize())
            << "Attempting to copy " << bufferDescriptor.size << " bytes into a buffer of size "
            << buffer.GetSize() << " at offset " << byteOffset;
    FILAMENT_CHECK_PRECONDITION(byteOffset % 4 == 0)
            << "Byte offset must be a multiple of 4 but is " << byteOffset;

    // TODO: All buffer objects are created with CopyDst usage.
    // This may have some performance implications. That should be investigated later.
    assert_invariant(buffer.GetUsage() & wgpu::BufferUsage::CopyDst);

    size_t remainder = bufferDescriptor.size % 4;

    // WriteBuffer is an async call. But cpu buffer data is already written to the staging
    // buffer on return from the WriteBuffer.
    auto legalSize = bufferDescriptor.size - remainder;
    queue.WriteBuffer(buffer, byteOffset, bufferDescriptor.buffer, legalSize);
    if (remainder != 0) {
        const uint8_t* remainderStart =
                static_cast<const uint8_t*>(bufferDescriptor.buffer) + legalSize;
        memcpy(mRemainderChunk.data(), remainderStart, remainder);
        // Pad the remainder with zeros to ensure deterministic behavior, though GPU shouldn't
        // access this
        std::memset(mRemainderChunk.data() + remainder, 0, 4 - remainder);

        queue.WriteBuffer(buffer, byteOffset + legalSize, &mRemainderChunk, 4);
    }
}

WGPUVertexBufferInfo::WGPUVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
        AttributeArray const& attributes, wgpu::Limits const& deviceLimits)
    : HwVertexBufferInfo(bufferCount, attributeCount) {
    FILAMENT_CHECK_PRECONDITION(attributeCount <= MAX_VERTEX_ATTRIBUTE_COUNT &&
                                attributeCount <= deviceLimits.maxVertexAttributes)
            << "The number of vertex attributes requested (" << attributeCount
            << ") exceeds Filament's MAX_VERTEX_ATTRIBUTE_COUNT limit ("
            << MAX_VERTEX_ATTRIBUTE_COUNT << ") and/or the device's limit ("
            << deviceLimits.maxVertexAttributes << ")";
    mVertexAttributes.reserve(attributeCount);
    if (attributeCount == 0) {
        mVertexBufferLayouts.resize(0);
        mWebGPUSlotBindingInfos.resize(0);
        return; // should not be possible, but being defensive. nothing to do otherwise
    }
    // populate mWebGPUSlotBindingInfos and attribute info (slot + attribute) by going through each
    // attribute...
    constexpr uint32_t IMPOSSIBLE_SLOT_INDEX = MAX_VERTEX_BUFFER_COUNT;
    struct AttributeInfo final {
        uint8_t slot = IMPOSSIBLE_SLOT_INDEX;
        wgpu::VertexAttribute attribute = {};
        AttributeInfo()
            : slot(IMPOSSIBLE_SLOT_INDEX),
              attribute({}) {}
        AttributeInfo(uint8_t slot, wgpu::VertexAttribute attribute)
            : slot(slot),
              attribute(attribute) {}
    };
    std::array<AttributeInfo, MAX_VERTEX_ATTRIBUTE_COUNT> attributeInfos{};
    uint8_t currentWebGPUSlotIndex = 0;
    uint8_t currentAttributeIndex = 0;
    mVertexBufferLayouts.reserve(MAX_VERTEX_BUFFER_COUNT);
    mWebGPUSlotBindingInfos.reserve(mVertexBufferLayouts.capacity());
    for (uint32_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
        const auto& attribute = attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            // We ignore "unused" attributes, ones not associated with a buffer. If a shader
            // references such an attribute we have a bug one way or another. Either the API/CPU
            // will produce an error (best case scenario), where an attribute is referenced in the
            // shader but not provided by the backend API (CPU-side), or the shader would be getting
            // junk/undefined data from the GPU, since we do not have a valid buffer of data to
            // provide to the shader/GPU.
            continue;
        }
        bool const isInteger = attribute.flags & Attribute::FLAG_INTEGER_TARGET;
        bool const isNormalized = attribute.flags & Attribute::FLAG_NORMALIZED;
        wgpu::VertexFormat const vertexFormat =
                getVertexFormat(attribute.type, isNormalized, isInteger);
        uint8_t existingSlot = IMPOSSIBLE_SLOT_INDEX;
        for (uint32_t slot = 0; slot < currentWebGPUSlotIndex; slot++) {
            WebGPUSlotBindingInfo const& info = mWebGPUSlotBindingInfos[slot];
            // We consider attributes to be in the same slot/layout only if they belong to the
            // same buffer and are interleaved; they cannot belong to separate partitions in the
            // same buffer, for example.
            // For the attributes to be interleaved, the stride must match (among other things).
            // The attribute offset being less than the slot's/layout's buffer offset indicates
            // that it is in a separate partition before this slot/layout, thus not part of it.
            // The difference from the attribute's offset and this slot's/layout's buffer offset
            // being greater than the stride indicates it is in a separate partition after this
            // slot/layout, thus not part of it.
            if (info.sourceBufferIndex == attribute.buffer && info.stride == attribute.stride &&
                    attribute.offset >= info.bufferOffset &&
                    ((attribute.offset - info.bufferOffset) < attribute.stride)) {
                existingSlot = slot;
                break;
            }
        }
        if (existingSlot == IMPOSSIBLE_SLOT_INDEX) {
            // New combination, use a new WebGPU slot
            FILAMENT_CHECK_PRECONDITION(currentWebGPUSlotIndex < MAX_VERTEX_BUFFER_COUNT &&
                                        currentWebGPUSlotIndex < deviceLimits.maxVertexBuffers)
                    << "Number of vertex buffer layouts must not exceed MAX_VERTEX_BUFFER_COUNT ("
                    << MAX_VERTEX_BUFFER_COUNT << ") or the device limit ("
                    << deviceLimits.maxVertexBuffers << ")";
            existingSlot = currentWebGPUSlotIndex++;
            mWebGPUSlotBindingInfos.push_back({
                .sourceBufferIndex = attribute.buffer,
                .bufferOffset = attribute.offset,
                .stride = attribute.stride
            });
            // we need to have a vertex buffer layout for each slot
            mVertexBufferLayouts.push_back({
                .stepMode = wgpu::VertexStepMode::Vertex,
                .arrayStride = attribute.stride
                // we do not know attributeCount or attributes at this time. We get those
                // in a subsequent pass over the attributeInfos we collect in this loop.
            });
        }
        attributeInfos[currentAttributeIndex++] = AttributeInfo(existingSlot,
            {
                .format = vertexFormat,
                .offset = attribute.offset - mWebGPUSlotBindingInfos[existingSlot].bufferOffset,
                .shaderLocation = attributeIndex
            }
        );
    }
    FILAMENT_CHECK_POSTCONDITION(currentAttributeIndex == attributeCount)
            << "Using " << currentAttributeIndex << " attributes, but " << attributeCount
            << " where indicated.";
    mVertexBufferLayouts.shrink_to_fit();
    mWebGPUSlotBindingInfos.shrink_to_fit();
    // sort attribute infos by increasing slot (by increasing offset within each slot).
    // We do this to ensure that attributes for the same slot/layout are contiguous in
    // the vector, so the vertex buffer layout associated with these contiguous attributes
    // can directly reference them in the mVertexAttributes vector below.
    std::sort(attributeInfos.data(), attributeInfos.data() + attributeCount,
            [](AttributeInfo const& first, AttributeInfo const& second) {
                if (first.slot < second.slot) {
                    return true;
                }
                if (first.slot == second.slot) {
                    if (first.attribute.offset < second.attribute.offset) {
                        return true;
                    }
                }
                return false;
            });
    // populate mVertexAttributes and update mVertexBufferLayouts to reference the correct
    // attributes in it (which will be contiguous in memory as ensured by the sorting above)...
    for (uint32_t attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex) {
        AttributeInfo const& info = attributeInfos[attributeIndex];
        mVertexAttributes.push_back(info.attribute);
        if (mVertexBufferLayouts[info.slot].attributeCount == 0) {
            mVertexBufferLayouts[info.slot].attributes = &mVertexAttributes[attributeIndex];
        }
        mVertexBufferLayouts[info.slot].attributeCount++;
    }
}

WGPUIndexBuffer::WGPUIndexBuffer(wgpu::Device const& device, uint8_t elementSize,
        uint32_t indexCount)
    : indexFormat(elementSize == 2 ? wgpu::IndexFormat::Uint16 : wgpu::IndexFormat::Uint32) {
    createBuffer(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
            elementSize * indexCount, "index_buffer");
}


WGPUVertexBuffer::WGPUVertexBuffer(wgpu::Device const& device, uint32_t vertexCount,
        uint32_t bufferCount, Handle<HwVertexBufferInfo> vbih)
    : HwVertexBuffer(vertexCount),
      vbih(vbih),
      buffers(bufferCount) {}

WGPUBufferObject::WGPUBufferObject(wgpu::Device const& device, BufferObjectBinding bindingType,
        uint32_t byteCount)
    : HwBufferObject(byteCount) {
    createBuffer(device, wgpu::BufferUsage::CopyDst | getBufferObjectUsage(bindingType), byteCount,
            "buffer_object");
}

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

    unsigned int samplerCount =
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
                // TODO: Set once we have the filtering values
                if (isDepthDescriptor(fEntry.type)) {
                    samplerEntry.sampler.type = wgpu::SamplerBindingType::Comparison;
                } else if (isIntDescriptor(fEntry.type)) {
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
        if (isDepthDescriptor(fEntry.type))
        {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Depth;
        }
        else if (isFloatDescriptor(fEntry.type))
        {
            // TODO: Set once we have the filtering values
            wEntry.texture.sampleType = wgpu::TextureSampleType::Float;
        }
        else if (isIntDescriptor(fEntry.type))
        {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Sint;
        }
        else if (isUnsignedIntDescriptor(fEntry.type))
        {
            wEntry.texture.sampleType = wgpu::TextureSampleType::Uint;
        }

        if (is3dTypeDescriptor(fEntry.type))
        {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e3D;
        }
        else if (is2dTypeDescriptor(fEntry.type))
        {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        }
        else if (is2dArrayTypeDescriptor(fEntry.type))
        {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        }
        else if (isCubeTypeDescriptor(fEntry.type))
        {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        }
        else if (isCubeArrayTypeDescriptor(fEntry.type))
        {
            wEntry.texture.viewDimension = wgpu::TextureViewDimension::CubeArray;
        }
        // fEntry.count is unused currently
    }
    std::string label =  "layout_" + baseLabel + std::to_string(++layoutNum) ;
    wgpu::BindGroupLayoutDescriptor layoutDescriptor{
        .label{label.c_str()}, // Use .c_str() if label needs to be const char*
        .entryCount = wEntries.size(),
        .entries = wEntries.data()
    };
    mLayout = device.CreateBindGroupLayout(&layoutDescriptor);
}

WebGPUDescriptorSetLayout::~WebGPUDescriptorSetLayout() {}

WebGPUDescriptorSet::WebGPUDescriptorSet(wgpu::BindGroupLayout const& layout,
        std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const& bindGroupEntries)
    : mLayout(layout),
      mEntriesWithDynamicOffsetsCount(std::count_if(bindGroupEntries.begin(),
              bindGroupEntries.end(), [](auto const& entry) { return entry.hasDynamicOffset; })) {

    mEntries.resize(bindGroupEntries.size());
    for (size_t i = 0; i < bindGroupEntries.size(); ++i) {
        mEntries[i].binding = bindGroupEntries[i].binding;
    }
    // Establish the size of entries based on the layout. This should be reliable and efficient.
    assert_invariant(INVALID_INDEX > mEntryIndexByBinding.size());
    for (size_t i = 0; i < mEntryIndexByBinding.size(); i++) {
        mEntryIndexByBinding[i] = INVALID_INDEX;
    }
    for (size_t index = 0; index < mEntries.size(); index++) {
        wgpu::BindGroupEntry const& entry = mEntries[index];
        assert_invariant(entry.binding < mEntryIndexByBinding.size());
        mEntryIndexByBinding[entry.binding] = static_cast<uint8_t>(index);
    }
}

WebGPUDescriptorSet::~WebGPUDescriptorSet() {
    mBindGroup = nullptr;
    mLayout = nullptr;
}

wgpu::BindGroup WebGPUDescriptorSet::lockAndReturn(const wgpu::Device& device) {
    if (mBindGroup) {
        return mBindGroup;
    }
    // TODO label? Should we just copy layout label?
    wgpu::BindGroupDescriptor desc{
        .layout = mLayout,
        .entryCount = mEntries.size(),
        .entries = mEntries.data()
    };
    mBindGroup = device.CreateBindGroup(&desc);
    FILAMENT_CHECK_POSTCONDITION(mBindGroup) << "Failed to create bind group?";
    // once we have created the bind group itself we should no longer need any other state
    mLayout = nullptr;
    mEntries.clear();
    mEntries.shrink_to_fit();
    return mBindGroup;
}

void WebGPUDescriptorSet::addEntry(unsigned int index, wgpu::BindGroupEntry&& entry) {
    if (mBindGroup) {
        // We will keep getting hits from future updates, but shouldn't do anything
        // Filament guarantees this won't change after things have locked.
        return;
    }
    // TODO: Putting some level of trust that Filament is not going to reuse indexes or go past the
    // layout index for efficiency. Add guards if wrong.
    FILAMENT_CHECK_POSTCONDITION(index < mEntryIndexByBinding.size())
            << "impossible/invalid index for a descriptor/binding (our of range or >= "
               "MAX_DESCRIPTOR_COUNT) "
            << index;
    uint8_t entryIndex = mEntryIndexByBinding[index];
    FILAMENT_CHECK_POSTCONDITION(
            entryIndex != INVALID_INDEX && entryIndex < mEntries.size())
            << "Invalid binding " << index;
    entry.binding = index;
    mEntries[entryIndex] = std::move(entry);
}

size_t WebGPUDescriptorSet::countEntitiesWithDynamicOffsets() const {
    return mEntriesWithDynamicOffsetsCount;
}

WGPUTexture::WGPUTexture(SamplerType samplerTargetType, uint8_t levels, TextureFormat format,
        uint8_t samples, uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
        wgpu::Device const& device) noexcept {
    assert_invariant(
            samples == 1 ||
            samples == 4 &&
                    "An invalid number of samples were requested, as WGPU requires the sample "
                    "count to either be 1 (no multisampling) or 4, at least as of April 2025 of "
                    "the spec. See https://www.w3.org/TR/webgpu/#texture-creation or "
                    "https://gpuweb.github.io/gpuweb/#multisample-state");
    // First, the texture aspect, starting with the defaults/basic configuration
    mFormat = fToWGPUTextureFormat(format);
    mUsage = fToWGPUTextureUsage(usage);
    mAspect = fToWGPUTextureViewAspect(usage, format);
    mSamplerType = target;
    mBlockWidth = filament::backend::getBlockWidth(format);
    mBlockHeight = filament::backend::getBlockHeight(format);
    target = samplerTargetType;

    wgpu::TextureDescriptor textureDescriptor{
        .label = getUserTextureLabel(target),
        .usage = mUsage,
        .dimension = target == SamplerType::SAMPLER_3D ? wgpu::TextureDimension::e3D
                                                       : wgpu::TextureDimension::e2D,
        .size = { .width = width, .height = height, .depthOrArrayLayers = depth },
        .format = mFormat,
        .mipLevelCount = levels,
        .sampleCount = samples,
        // TODO Is this fine? Could do all-the-things, a naive mapping or get something from
        // Filament
        .viewFormatCount = 0,
        .viewFormats = nullptr,
    };
    // adjust for specific cases
    switch (target) {
        case SamplerType::SAMPLER_2D:
            mArrayLayerCount = 1;
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            textureDescriptor.size.depthOrArrayLayers = 6;
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_3D:
            mArrayLayerCount = 1;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            textureDescriptor.size.depthOrArrayLayers = depth * 6;
            mArrayLayerCount = textureDescriptor.size.depthOrArrayLayers;
            break;
    }
    assert_invariant(textureDescriptor.format != wgpu::TextureFormat::Undefined &&
                     "Could not find appropriate WebGPU format");
    mTexture = device.CreateTexture(&textureDescriptor);
    FILAMENT_CHECK_POSTCONDITION(mTexture)
            << "Failed to create texture for " << textureDescriptor.label;
    // Second, the texture view aspect
    mTexureView = makeTextureView(0, levels, 0, mArrayLayerCount, target);
}

WGPUTexture::WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept {
    mTexture = src->mTexture;
    mAspect = src->mAspect;
    mBlockWidth = src->mBlockWidth;
    mBlockHeight = src->mBlockHeight;
    mArrayLayerCount = src->mArrayLayerCount;
    mSamplerType = src->mSamplerType;

    mTexureView = makeTextureView(baseLevel, levelCount, 0, src->mArrayLayerCount, mSamplerType);
}

wgpu::TextureUsage WGPUTexture::fToWGPUTextureUsage(TextureUsage const& fUsage) {
    wgpu::TextureUsage retUsage = wgpu::TextureUsage::None;

    // Basing this mapping off of VulkanTexture.cpp's getUsage func and suggestions from Gemini
    // TODO Validate assumptions, revisit if issues.
    if (any(TextureUsage::BLIT_SRC & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopySrc;
    }
    if (any((TextureUsage::BLIT_DST | TextureUsage::UPLOADABLE) & fUsage)) {
        retUsage |= wgpu::TextureUsage::CopyDst;
    }
    if (any(TextureUsage::SAMPLEABLE & fUsage)) {
        retUsage |= wgpu::TextureUsage::TextureBinding;
    }
    // WGPU Render attachment covers either color or stencil situation dependant
    // NOTE: Depth attachment isn't used this way in Vulkan but logically maps to WGPU docs. If
    // issues, investigate here.
    if (any((TextureUsage::COLOR_ATTACHMENT | TextureUsage::STENCIL_ATTACHMENT |
                    TextureUsage::DEPTH_ATTACHMENT) &
                fUsage)) {
        retUsage |= wgpu::TextureUsage::RenderAttachment;
    }

    // This is from Vulkan logic- if there are any issues try disabling this first, allows perf
    // benefit though
    const bool useTransientAttachment =
            // Usage consists of attachment flags only.
            none(fUsage & ~TextureUsage::ALL_ATTACHMENTS) &&
            // Usage contains at least one attachment flag.
            any(fUsage & TextureUsage::ALL_ATTACHMENTS) &&
            // Depth resolve cannot use transient attachment because it uses a custom shader.
            // TODO: see VulkanDriver::isDepthStencilResolveSupported() to know when to remove this
            // restriction.
            // Note that the custom shader does not resolve stencil. We do need to move to vk 1.2
            // and above to be able to support stencil resolve (along with depth).
            !(any(fUsage & TextureUsage::DEPTH_ATTACHMENT) && samples > 1);
    if (useTransientAttachment) {
        retUsage |= wgpu::TextureUsage::TransientAttachment;
    }
    // NOTE: Unused wgpu flags:
    //  StorageBinding
    //  StorageAttachment

    // NOTE: Unused Filament flags:
    //  SUBPASS_INPUT VK goes to input attachment which we don't support right now
    //  PROTECTED
    return retUsage;
}

wgpu::TextureFormat WGPUTexture::fToWGPUTextureFormat(TextureFormat const& fFormat) {
    switch (fFormat) {
        case filament::backend::TextureFormat::R8:
            return wgpu::TextureFormat::R8Unorm;
        case filament::backend::TextureFormat::R8_SNORM:
            return wgpu::TextureFormat::R8Snorm;
        case filament::backend::TextureFormat::R8UI:
            return wgpu::TextureFormat::R8Uint;
        case filament::backend::TextureFormat::R8I:
            return wgpu::TextureFormat::R8Sint;
        case filament::backend::TextureFormat::STENCIL8:
            return wgpu::TextureFormat::Stencil8;
        case filament::backend::TextureFormat::R16F:
            return wgpu::TextureFormat::R16Float;
        case filament::backend::TextureFormat::R16UI:
            return wgpu::TextureFormat::R16Uint;
        case filament::backend::TextureFormat::R16I:
            return wgpu::TextureFormat::R16Sint;
        case filament::backend::TextureFormat::RG8:
            return wgpu::TextureFormat::RG8Unorm;
        case filament::backend::TextureFormat::RG8_SNORM:
            return wgpu::TextureFormat::RG8Snorm;
        case filament::backend::TextureFormat::RG8UI:
            return wgpu::TextureFormat::RG8Uint;
        case filament::backend::TextureFormat::RG8I:
            return wgpu::TextureFormat::RG8Sint;
        case filament::backend::TextureFormat::R32F:
            return wgpu::TextureFormat::R32Float;
        case filament::backend::TextureFormat::R32UI:
            return wgpu::TextureFormat::R32Uint;
        case filament::backend::TextureFormat::R32I:
            return wgpu::TextureFormat::R32Sint;
        case filament::backend::TextureFormat::RG16F:
            return wgpu::TextureFormat::RG16Float;
        case filament::backend::TextureFormat::RG16UI:
            return wgpu::TextureFormat::RG16Uint;
        case filament::backend::TextureFormat::RG16I:
            return wgpu::TextureFormat::RG16Sint;
        case filament::backend::TextureFormat::RGBA8:
            return wgpu::TextureFormat::RGBA8Unorm;
        case filament::backend::TextureFormat::SRGB8_A8:
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGBA8_SNORM:
            return wgpu::TextureFormat::RGBA8Snorm;
        case filament::backend::TextureFormat::RGBA8UI:
            return wgpu::TextureFormat::RGBA8Uint;
        case filament::backend::TextureFormat::RGBA8I:
            return wgpu::TextureFormat::RGBA8Sint;
        case filament::backend::TextureFormat::DEPTH16:
            return wgpu::TextureFormat::Depth16Unorm;
        case filament::backend::TextureFormat::DEPTH24:
            return wgpu::TextureFormat::Depth24Plus;
        case filament::backend::TextureFormat::DEPTH32F:
            return wgpu::TextureFormat::Depth32Float;
        case filament::backend::TextureFormat::DEPTH24_STENCIL8:
            return wgpu::TextureFormat::Depth24PlusStencil8;
        case filament::backend::TextureFormat::DEPTH32F_STENCIL8:
            return wgpu::TextureFormat::Depth32FloatStencil8;
        case filament::backend::TextureFormat::RG32F:
            return wgpu::TextureFormat::RG32Float;
        case filament::backend::TextureFormat::RG32UI:
            return wgpu::TextureFormat::RG32Uint;
        case filament::backend::TextureFormat::RG32I:
            return wgpu::TextureFormat::RG32Sint;
        case filament::backend::TextureFormat::RGBA16F:
            return wgpu::TextureFormat::RGBA16Float;
        case filament::backend::TextureFormat::RGBA16UI:
            return wgpu::TextureFormat::RGBA16Uint;
        case filament::backend::TextureFormat::RGBA16I:
            return wgpu::TextureFormat::RGBA16Sint;
        case filament::backend::TextureFormat::RGBA32F:
            return wgpu::TextureFormat::RGBA32Float;
        case filament::backend::TextureFormat::RGBA32UI:
            return wgpu::TextureFormat::RGBA32Uint;
        case filament::backend::TextureFormat::RGBA32I:
            return wgpu::TextureFormat::RGBA32Sint;
        case filament::backend::TextureFormat::EAC_R11:
            return wgpu::TextureFormat::EACR11Unorm;
        case filament::backend::TextureFormat::EAC_R11_SIGNED:
            return wgpu::TextureFormat::EACR11Snorm;
        case filament::backend::TextureFormat::EAC_RG11:
            return wgpu::TextureFormat::EACRG11Unorm;
        case filament::backend::TextureFormat::EAC_RG11_SIGNED:
            return wgpu::TextureFormat::EACRG11Snorm;
        case filament::backend::TextureFormat::ETC2_RGB8:
            return wgpu::TextureFormat::ETC2RGB8Unorm;
        case filament::backend::TextureFormat::ETC2_SRGB8:
            return wgpu::TextureFormat::ETC2RGB8UnormSrgb;
        case filament::backend::TextureFormat::ETC2_RGB8_A1:
            return wgpu::TextureFormat::ETC2RGB8A1Unorm;
        case filament::backend::TextureFormat::ETC2_SRGB8_A1:
            return wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
        case filament::backend::TextureFormat::ETC2_EAC_RGBA8:
            return wgpu::TextureFormat::ETC2RGBA8Unorm;
        case filament::backend::TextureFormat::ETC2_EAC_SRGBA8:
            return wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_4x4:
            return wgpu::TextureFormat::ASTC4x4Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_4x4:
            return wgpu::TextureFormat::ASTC4x4UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_5x4:
            return wgpu::TextureFormat::ASTC5x4Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_5x4:
            return wgpu::TextureFormat::ASTC5x4UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_5x5:
            return wgpu::TextureFormat::ASTC5x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_5x5:
            return wgpu::TextureFormat::ASTC5x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_6x5:
            return wgpu::TextureFormat::ASTC6x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_6x5:
            return wgpu::TextureFormat::ASTC6x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_6x6:
            return wgpu::TextureFormat::ASTC6x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_6x6:
            return wgpu::TextureFormat::ASTC6x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x5:
            return wgpu::TextureFormat::ASTC8x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x5:
            return wgpu::TextureFormat::ASTC8x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x6:
            return wgpu::TextureFormat::ASTC8x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x6:
            return wgpu::TextureFormat::ASTC8x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_8x8:
            return wgpu::TextureFormat::ASTC8x8Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_8x8:
            return wgpu::TextureFormat::ASTC8x8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x5:
            return wgpu::TextureFormat::ASTC10x5Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x5:
            return wgpu::TextureFormat::ASTC10x5UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x6:
            return wgpu::TextureFormat::ASTC10x6Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x6:
            return wgpu::TextureFormat::ASTC10x6UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x8:
            return wgpu::TextureFormat::ASTC10x8Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x8:
            return wgpu::TextureFormat::ASTC10x8UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_10x10:
            return wgpu::TextureFormat::ASTC10x10Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_10x10:
            return wgpu::TextureFormat::ASTC10x10UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_12x10:
            return wgpu::TextureFormat::ASTC12x10Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_12x10:
            return wgpu::TextureFormat::ASTC12x10UnormSrgb;
        case filament::backend::TextureFormat::RGBA_ASTC_12x12:
            return wgpu::TextureFormat::ASTC12x12Unorm;
        case filament::backend::TextureFormat::SRGB8_ALPHA8_ASTC_12x12:
            return wgpu::TextureFormat::ASTC12x12UnormSrgb;
        case filament::backend::TextureFormat::RED_RGTC1:
            return wgpu::TextureFormat::BC4RUnorm;
        case filament::backend::TextureFormat::SIGNED_RED_RGTC1:
            return wgpu::TextureFormat::BC4RSnorm;
        case filament::backend::TextureFormat::RED_GREEN_RGTC2:
            return wgpu::TextureFormat::BC5RGUnorm;
        case filament::backend::TextureFormat::SIGNED_RED_GREEN_RGTC2:
            return wgpu::TextureFormat::BC5RGSnorm;
        case filament::backend::TextureFormat::RGB_BPTC_UNSIGNED_FLOAT:
            return wgpu::TextureFormat::BC6HRGBUfloat;
        case filament::backend::TextureFormat::RGB_BPTC_SIGNED_FLOAT:
            return wgpu::TextureFormat::BC6HRGBFloat;
        case filament::backend::TextureFormat::RGBA_BPTC_UNORM:
            return wgpu::TextureFormat::BC7RGBAUnorm;
        case filament::backend::TextureFormat::SRGB_ALPHA_BPTC_UNORM:
            return wgpu::TextureFormat::BC7RGBAUnormSrgb;
        case filament::backend::TextureFormat::RGB565:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and discard the alpha and lower precision.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB9_E5:
            return wgpu::TextureFormat::RGB9E5Ufloat;
        case filament::backend::TextureFormat::RGB5_A1:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGBA4:
            // No direct mapping in wgpu. Could potentially map to RGBA8Unorm
            // and handle the packing/unpacking in shaders.
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB8:
            // No direct sRGB equivalent in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Unorm;
        case filament::backend::TextureFormat::SRGB8:
            // No direct sRGB equivalent in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8UnormSrgb;
        case filament::backend::TextureFormat::RGB8_SNORM:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Snorm;
        case filament::backend::TextureFormat::RGB8UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Uint;
        case filament::backend::TextureFormat::RGB8I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA8Sint;
        case filament::backend::TextureFormat::R11F_G11F_B10F:
            return wgpu::TextureFormat::RG11B10Ufloat;
        case filament::backend::TextureFormat::UNUSED:
            return wgpu::TextureFormat::Undefined;
        case filament::backend::TextureFormat::RGB10_A2:
            return wgpu::TextureFormat::RGB10A2Unorm;
        case filament::backend::TextureFormat::RGB16F:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Float;
        case filament::backend::TextureFormat::RGB16UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Uint;
        case filament::backend::TextureFormat::RGB16I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA16Sint;
        case filament::backend::TextureFormat::RGB32F:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Float;
        case filament::backend::TextureFormat::RGB32UI:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Uint;
        case filament::backend::TextureFormat::RGB32I:
            // No direct mapping in wgpu without alpha.
            return wgpu::TextureFormat::RGBA32Sint;
        case filament::backend::TextureFormat::DXT1_RGB:
            return wgpu::TextureFormat::BC1RGBAUnorm;
        case filament::backend::TextureFormat::DXT1_RGBA:
            return wgpu::TextureFormat::BC1RGBAUnorm;
        case filament::backend::TextureFormat::DXT3_RGBA:
            return wgpu::TextureFormat::BC2RGBAUnorm;
        case filament::backend::TextureFormat::DXT5_RGBA:
            return wgpu::TextureFormat::BC3RGBAUnorm;
        case filament::backend::TextureFormat::DXT1_SRGB:
            return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT1_SRGBA:
            return wgpu::TextureFormat::BC1RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT3_SRGBA:
            return wgpu::TextureFormat::BC2RGBAUnormSrgb;
        case filament::backend::TextureFormat::DXT5_SRGBA:
            return wgpu::TextureFormat::BC3RGBAUnormSrgb;
    }
}

wgpu::TextureAspect WGPUTexture::fToWGPUTextureViewAspect(TextureUsage const& fUsage,
        TextureFormat const& fFormat) {

    const bool isDepth = any(fUsage & TextureUsage::DEPTH_ATTACHMENT);
    const bool isStencil = any(fUsage & TextureUsage::STENCIL_ATTACHMENT);
    const bool isColor = any(fUsage & TextureUsage::COLOR_ATTACHMENT);
    const bool isSample = any(fUsage & TextureUsage::SAMPLEABLE);

    if (isDepth && !isColor && !isStencil) {
        return wgpu::TextureAspect::DepthOnly;
    }

    if (isStencil && !isColor && !isDepth) {
        return wgpu::TextureAspect::StencilOnly;
    }

    if (fFormat == filament::backend::TextureFormat::DEPTH32F ||
            fFormat == filament::backend::TextureFormat::DEPTH24 ||
            fFormat == filament::backend::TextureFormat::DEPTH16) {
        return wgpu::TextureAspect::DepthOnly;
    }

    if (fFormat == filament::backend::TextureFormat::STENCIL8) {
        return wgpu::TextureAspect::StencilOnly;
    }

    if (fFormat == filament::backend::TextureFormat::DEPTH24_STENCIL8 ||
            fFormat == filament::backend::TextureFormat::DEPTH32F_STENCIL8) {
        if (isSample) {
            return wgpu::TextureAspect::DepthOnly;
        }
    }

    return wgpu::TextureAspect::All;
}

wgpu::TextureView WGPUTexture::makeTextureView(const uint8_t& baseLevel, const uint8_t& levelCount,
        const uint32_t& baseArrayLayer, const uint32_t& arrayLayerCount,
        SamplerType target) {

    wgpu::TextureViewDescriptor textureViewDescriptor{
        .label = getUserTextureViewLabel(target),
        .format = mFormat,
        .baseMipLevel = baseLevel,
        .mipLevelCount = levelCount,
        .baseArrayLayer = baseArrayLayer,
        .arrayLayerCount = arrayLayerCount,
        .aspect = mAspect,
        .usage = mUsage
    };

    switch (target) {
        case SamplerType::SAMPLER_2D:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
            break;
        case SamplerType::SAMPLER_2D_ARRAY:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2DArray;
            break;
        case SamplerType::SAMPLER_CUBEMAP:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
            break;
        case SamplerType::SAMPLER_EXTERNAL:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
            break;
        case SamplerType::SAMPLER_3D:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::e3D;
            break;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            textureViewDescriptor.dimension = wgpu::TextureViewDimension::CubeArray;
            break;
    }
    wgpu::TextureView textureView = mTexture.CreateView(&textureViewDescriptor);
    FILAMENT_CHECK_POSTCONDITION(textureView)
            << "Failed to create texture view " << textureViewDescriptor.label;
    return textureView;
}

WGPURenderTarget::WGPURenderTarget(uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount,
        const MRT& colorAttachmentsMRT,
        const Attachment& depthAttachmentInfo,
        const Attachment& stencilAttachmentInfo)
    : HwRenderTarget(width, height),
      mLayerCount(layerCount),
      defaultRenderTarget(false),
      samples(samples),
      mColorAttachments(colorAttachmentsMRT),
      mDepthAttachment(depthAttachmentInfo),
      mStencilAttachment(stencilAttachmentInfo) {
    // TODO Make this an array
    mColorAttachmentDescriptors.reserve(MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
}

wgpu::LoadOp WGPURenderTarget::getLoadOperation(RenderPassParams const& params,
                                                TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.clear & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear;
    }
    if (any(params.flags.discardStart & bufferToOperateOn)) {
        return wgpu::LoadOp::Clear; // Or wgpu::LoadOp::Undefined if clear is not desired on discard
    }
    return wgpu::LoadOp::Load;
}

wgpu::StoreOp WGPURenderTarget::getStoreOperation(RenderPassParams const& params,
                                                  TargetBufferFlags bufferToOperateOn) {
    if (any(params.flags.discardEnd & bufferToOperateOn)) {
        return wgpu::StoreOp::Discard;
    }
    return wgpu::StoreOp::Store;
}

void WGPURenderTarget::setUpRenderPassAttachments(wgpu::RenderPassDescriptor& descriptor,
        RenderPassParams const& params, wgpu::TextureView const& defaultColorTextureView,
        wgpu::TextureView const& defaultDepthStencilTextureView,
        wgpu::TextureView const* customColorTextureViews, uint32_t customColorTextureViewCount,
        wgpu::TextureView const& customDepthTextureView,
        wgpu::TextureView const& customStencilTextureView, wgpu::TextureFormat customDepthFormat,
        wgpu::TextureFormat customStencilFormat) {
    mColorAttachmentDescriptors.clear();
    mHasDepthStencilAttachment = false;

    if (defaultRenderTarget) {
        assert_invariant(defaultColorTextureView);
        mColorAttachmentDescriptors.push_back({ .view = defaultColorTextureView,
            .resolveTarget = nullptr,
            .loadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::COLOR0),
            .storeOp = WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::COLOR0),
            .clearValue = { params.clearColor.r, params.clearColor.g, params.clearColor.b,
                params.clearColor.a } });

        if (defaultDepthStencilTextureView) {
            mDepthStencilAttachmentDescriptor = {
                .view = defaultDepthStencilTextureView,
                .depthLoadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH),
                .depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH),
                .depthClearValue = static_cast<float>(params.clearDepth),
                .depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0,
                .stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL),
                .stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL),
                .stencilClearValue = params.clearStencil,
                .stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0,
            };
            mHasDepthStencilAttachment = true;
        }
    } else {// Custom Render Target
        for (uint32_t i = 0; i < customColorTextureViewCount; ++i) {
            if (customColorTextureViews[i]) {
                mColorAttachmentDescriptors.push_back({ .view = customColorTextureViews[i],
                    // .resolveTarget = nullptr; // TODO: MSAA resolve for custom RT
                    .loadOp = WGPURenderTarget::getLoadOperation(params, getTargetBufferFlagsAt(i)),
                    .storeOp =
                            WGPURenderTarget::getStoreOperation(params, getTargetBufferFlagsAt(i)),
                    .clearValue = { .r = params.clearColor.r,
                        .g = params.clearColor.g,
                        .b = params.clearColor.b,
                        .a = params.clearColor.a } });
            }
        }

        FILAMENT_CHECK_POSTCONDITION(!(customDepthTextureView && customStencilTextureView))
                << "WebGPU CANNOT support separate texture views for depth + stencil. depth + "
                   "stencil needs to be in one texture view";

        const bool hasStencil =
                customStencilTextureView ||
                (customDepthFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        const bool hasDepth =
                customDepthTextureView ||
                (customStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                        customDepthFormat == wgpu::TextureFormat::Depth32FloatStencil8);

        if (customDepthTextureView || customStencilTextureView) {
            assert_invariant((hasDepth || hasStencil) &&
                             "Depth or Texture view without a valid texture format");
            mDepthStencilAttachmentDescriptor = {};
            mDepthStencilAttachmentDescriptor.view =
                    customDepthTextureView ? customDepthTextureView : customStencilTextureView;

            if (hasDepth) {
                mDepthStencilAttachmentDescriptor.depthLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH);
                mDepthStencilAttachmentDescriptor.depthClearValue =
                        static_cast<float>(params.clearDepth);
                mDepthStencilAttachmentDescriptor.depthReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.depthLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.depthReadOnly = true;
            }

            if (hasStencil) {
                mDepthStencilAttachmentDescriptor.stencilLoadOp =
                        WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilStoreOp =
                        WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL);
                mDepthStencilAttachmentDescriptor.stencilClearValue = params.clearStencil;
                mDepthStencilAttachmentDescriptor.stencilReadOnly =
                        (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0;
            } else {
                mDepthStencilAttachmentDescriptor.stencilLoadOp = wgpu::LoadOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilStoreOp = wgpu::StoreOp::Undefined;
                mDepthStencilAttachmentDescriptor.stencilReadOnly = true;
            }
            mHasDepthStencilAttachment = true;
        }
    }

    descriptor.colorAttachmentCount = mColorAttachmentDescriptors.size();
    descriptor.colorAttachments = mColorAttachmentDescriptors.data();
    descriptor.depthStencilAttachment =
            mHasDepthStencilAttachment ? &mDepthStencilAttachmentDescriptor : nullptr;

    // descriptor.sampleCount was removed from the core spec. If your webgpu.h still has it,
    // and your Dawn version expects it, you might need to set it here based on this->samples.
    // e.g., descriptor.sampleCount = this->samples;
}

}// namespace filament::backend
