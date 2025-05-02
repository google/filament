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
            .shaderLocation = attribIndex,
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

    static int layoutNum = 0;

    uint samplerCount =
            std::count_if(layout.bindings.begin(), layout.bindings.end(), [](auto& fEntry) {
                return fEntry.type == DescriptorType::SAMPLER ||
                       fEntry.type == DescriptorType::SAMPLER_EXTERNAL;
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
            case DescriptorType::SAMPLER_EXTERNAL:
            case DescriptorType::SAMPLER: {
                auto& samplerEntry = wEntries.emplace_back();
                auto& samplerEntryInfo = mBindGroupEntries.emplace_back();
                samplerEntry.binding = fEntry.binding * 2 + 1;
                samplerEntryInfo.binding = samplerEntry.binding;
                samplerEntryInfo.type = WebGPUDescriptorSetLayout::BindGroupEntryType::SAMPLER;
                samplerEntry.visibility = wEntry.visibility;
                // We are simply hoping that undefined and defaults suffices here.
                samplerEntry.sampler.type = wgpu::SamplerBindingType::NonFiltering; // Example default
                wEntry.texture.sampleType = wgpu::TextureSampleType::Float;      // Example default
                entryInfo.type = WebGPUDescriptorSetLayout::BindGroupEntryType::TEXTURE_VIEW;
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
                PANIC_POSTCONDITION("Input Attachment is not supported");
                break;
            }
            case DescriptorType::SHADER_STORAGE_BUFFER: {
                PANIC_POSTCONDITION("Shader storage is not supported");
                break;
            }
        }
        // fEntry.count is unused currently
    }
    std::string label =  "layout_"+ layout.label + std::to_string(++layoutNum) ;
    wgpu::BindGroupLayoutDescriptor layoutDescriptor{
        .label{label.c_str()}, // Use .c_str() if label needs to be const char*
        .entryCount = wEntries.size(),
        .entries = wEntries.data()
    };
    printf("===R Creating layout %i with %lu entries\n", layoutNum, wEntries.size());
    mLayout = device.CreateBindGroupLayout(&layoutDescriptor);
}

WebGPUDescriptorSetLayout::~WebGPUDescriptorSetLayout() {}

wgpu::Buffer WebGPUDescriptorSet::sDummyUniformBuffer = nullptr;
wgpu::Texture WebGPUDescriptorSet::sDummyTexture = nullptr;
wgpu::TextureView WebGPUDescriptorSet::sDummyTextureView = nullptr;
wgpu::Sampler WebGPUDescriptorSet::sDummySampler = nullptr;

void WebGPUDescriptorSet::initializeDummyResourcesIfNotAlready(wgpu::Device const& device,
        wgpu::TextureFormat aColorFormat) {
    if (!sDummyUniformBuffer) {
        wgpu::BufferDescriptor bufferDescriptor{
            .label = "dummy_uniform_not_to_be_used",
            .usage = wgpu::BufferUsage::Uniform,
            .size = 4
        };
        sDummyUniformBuffer = device.CreateBuffer(&bufferDescriptor);
        FILAMENT_CHECK_POSTCONDITION(sDummyUniformBuffer)
                << "Failed to create dummy uniform buffer?";
    }
    if (!sDummyTexture || !sDummyTextureView) {
        wgpu::TextureDescriptor textureDescriptor{
            .label = "dummy_texture_not_to_be_used",
            .usage = wgpu::TextureUsage::TextureBinding,
            .dimension = wgpu::TextureDimension::e2D,
            .size = wgpu::Extent3D{ .width = 4, .height = 4, .depthOrArrayLayers = 1 },
            .format = aColorFormat,
        };
        if (!sDummyTexture) {
            sDummyTexture = device.CreateTexture(&textureDescriptor);
            FILAMENT_CHECK_POSTCONDITION(sDummyUniformBuffer) << "Failed to create dummy texture?";
        }
        if (!sDummyTextureView) {
            wgpu::TextureViewDescriptor textureViewDescriptor{
                .label = "dummy_texture_view_not_to_be_used"
            };
            sDummyTextureView = sDummyTexture.CreateView(&textureViewDescriptor);
            FILAMENT_CHECK_POSTCONDITION(sDummyUniformBuffer)
                    << "Failed to create dummy texture view?";
        }
    }
    if (!sDummySampler) {
        wgpu::SamplerDescriptor samplerDescriptor{
            .label = "dummy_sampler_not_to_be_used"
        };
        sDummySampler = device.CreateSampler(&samplerDescriptor);
        FILAMENT_CHECK_POSTCONDITION(sDummyUniformBuffer) << "Failed to create dummy sampler?";
    }
}

std::vector<wgpu::BindGroupEntry> WebGPUDescriptorSet::createDummyEntriesSortedByBinding(
        std::vector<filament::backend::WebGPUDescriptorSetLayout::BindGroupEntryInfo> const&
                bindGroupEntries) {
    assert_invariant(WebGPUDescriptorSet::sDummyUniformBuffer &&
                     "Dummy uniform buffer must have been created before "
                     "creating dummy bind group entries.");
    assert_invariant(
            WebGPUDescriptorSet::sDummyTexture &&
            "Dummy texture must have been created before creating dummy bind group entries.");
    assert_invariant(
            WebGPUDescriptorSet::sDummyTextureView &&
            "Dummy texture view must have been created before creating dummy bind group entries.");
    assert_invariant(
            WebGPUDescriptorSet::sDummySampler &&
            "Dummy sampler must have been created before creating dummy bind group entries.");
    using filament::backend::WebGPUDescriptorSetLayout;
    std::vector<wgpu::BindGroupEntry> entries;
    entries.reserve(bindGroupEntries.size());
    for (auto const& entryInfo: bindGroupEntries) {
        auto& entry = entries.emplace_back();
        entry.binding = entryInfo.binding;
        switch (entryInfo.type) {
            case WebGPUDescriptorSetLayout::BindGroupEntryType::UNIFORM_BUFFER:
                entry.buffer = WebGPUDescriptorSet::sDummyUniformBuffer;
                break;
            case WebGPUDescriptorSetLayout::BindGroupEntryType::TEXTURE_VIEW:
                entry.textureView = WebGPUDescriptorSet::sDummyTextureView;
                break;
            case WebGPUDescriptorSetLayout::BindGroupEntryType::SAMPLER:
                entry.sampler = WebGPUDescriptorSet::sDummySampler;
                break;
        }
    }
    std::sort(entries.begin(), entries.end(),
            [](wgpu::BindGroupEntry const& a, wgpu::BindGroupEntry const& b) {
                return a.binding < b.binding;
            });
    return entries;
}

WebGPUDescriptorSet::WebGPUDescriptorSet(const wgpu::BindGroupLayout& layout,
        std::vector<WebGPUDescriptorSetLayout::BindGroupEntryInfo> const& bindGroupEntries)
    : mLayout(layout),
      mEntriesSortedByBinding(createDummyEntriesSortedByBinding(bindGroupEntries)) {
    // Establish the size of entries based on the layout. This should be reliable and efficient.
    assert_invariant(INVALID_INDEX > mEntryIndexByBinding.size());
    for (size_t i = 0; i < mEntryIndexByBinding.size(); i++) {
        mEntryIndexByBinding[i] = INVALID_INDEX;
    }
    for (size_t index = 0; index < mEntriesSortedByBinding.size(); index++) {
        wgpu::BindGroupEntry const& entry = mEntriesSortedByBinding[index];
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
    for (const auto& entry : mEntriesSortedByBinding){
        if(entry.buffer != nullptr){
            printf("===R Entry %i is buffer\n", entry.binding);
        }
        else if(entry.sampler != nullptr){
            printf("===R Entry %i is sampler\n", entry.binding);
        }
        else if(entry.textureView != nullptr){
            printf("===R Entry %i is textureView\n", entry.binding);
        }
        else{
            printf("===R Entry %i is ERROR STATE\n", entry.binding);
        }
    }
    printf("===R Complete Group\n");
    // TODO label? Should we just copy layout label?
    wgpu::BindGroupDescriptor desc{
        .layout = mLayout,
        .entryCount = mEntriesSortedByBinding.size(),
        .entries = mEntriesSortedByBinding.data()
    };
    mBindGroup = device.CreateBindGroup(&desc);
    FILAMENT_CHECK_POSTCONDITION(mBindGroup) << "Failed to create bind group?";
    // once we have created the bind group itself we should no longer need any other state
    mLayout = nullptr;
    mEntriesSortedByBinding.clear();
    mEntriesSortedByBinding.shrink_to_fit();
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
    FILAMENT_CHECK_POSTCONDITION(index < mEntryIndexByBinding.size())
            << "impossible/invalid index for a descriptor/binding (our of range or >= "
               "MAX_DESCRIPTOR_COUNT) "
            << index;
    uint8_t entryIndex = mEntryIndexByBinding[index];
    FILAMENT_CHECK_POSTCONDITION(
            entryIndex != INVALID_INDEX && entryIndex < mEntriesSortedByBinding.size())
            << "Invalid binding " << index;
    entry.binding = index;
    mEntriesSortedByBinding[entryIndex] = std::move(entry);
}

// From createTextureR
WGPUTexture::WGPUTexture(SamplerType target, uint8_t levels, TextureFormat format, uint8_t samples,
        uint32_t width, uint32_t height, uint32_t depth, TextureUsage usage,
        wgpu::Device device) noexcept {

    // First the texture aspect
    wgpu::TextureDescriptor desc;

    switch (target) {
        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
        // Should be safe to assume external is 2d
        case SamplerType::SAMPLER_EXTERNAL: {
            desc.dimension = wgpu::TextureDimension::e2D;
            break;
        }
        case SamplerType::SAMPLER_3D: {
            desc.dimension = wgpu::TextureDimension::e3D;
            break;
        }
    }
    desc.size = { .width = width, .height = height, .depthOrArrayLayers = depth };
    desc.format = fToWGPUTextureFormat(format);
    assert_invariant(desc.format != wgpu::TextureFormat::Undefined);

    // WGPU requires this to be true. Filament should comply
    assert(samples == 1 || samples || 4);

    desc.sampleCount = samples;
    desc.usage = fToWGPUTextureUsage(usage);
    desc.mipLevelCount = levels;

    // TODO Is this fine? Could do all-the-things, a naive mapping or get something from Filament
    desc.viewFormats = nullptr;

    texture = device.CreateTexture(&desc);

    // TODO should a default levelCount be something other than 0? Sample count?
    texView = makeTextureView(0, 1);
}
// From createTextureViewR
WGPUTexture::WGPUTexture(WGPUTexture* src, uint8_t baseLevel, uint8_t levelCount) noexcept {
    texture = src->texture;
    texView = makeTextureView(baseLevel, levelCount);
}

wgpu::TextureUsage WGPUTexture::fToWGPUTextureUsage(const TextureUsage& fUsage) {
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
wgpu::TextureFormat WGPUTexture::fToWGPUTextureFormat(const TextureFormat& fUsage) {
    switch (fUsage) {
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

wgpu::TextureView WGPUTexture::makeTextureView(const uint8_t& baseLevel,
        const uint8_t& levelCount) {
    wgpu::TextureViewDescriptor desc;
    desc.baseMipLevel = baseLevel;
    desc.mipLevelCount = levelCount;

    // baseArrayLayer is required, making a guess
    desc.baseArrayLayer = 0;
    // Have not found an analouge to aspect in other drivers, but ALL should be unrestrictive.
    // TODO Can we make this better?
    desc.aspect = wgpu::TextureAspect::All;

    // The rest of the properties should be fine to leave as default, using the texture params.
    desc.label = "TODO";

    desc.format = wgpu::TextureFormat::Undefined;
    desc.dimension = wgpu::TextureViewDimension::Undefined;
    desc.usage = wgpu::TextureUsage::None;

    return texture.CreateView(&desc);
}

WGPURenderTarget::Attachment WGPURenderTarget::getDrawColorAttachment(size_t index) {
    assert_invariant( index < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
    auto result = color[index];
    if (index == 0 && defaultRenderTarget) {

    }

    return result;
}

wgpu::LoadOp WGPURenderTarget::getLoadOperation(RenderPassParams const& params,
                                             TargetBufferFlags buffer) {
    auto clearFlags = params.flags.clear;
    auto discardStartFlags = params.flags.discardStart;
    if (any(clearFlags & buffer)) {
        return wgpu::LoadOp::Clear;
    } else if (any(discardStartFlags & buffer)) {
        return wgpu::LoadOp::Clear;
    }
    return wgpu::LoadOp::Load;
}

wgpu::StoreOp WGPURenderTarget::getStoreOperation(RenderPassParams const& params,
                                               TargetBufferFlags buffer) {
    const auto discardEndFlags = params.flags.discardEnd;
    if (any(discardEndFlags & buffer)) {
        return wgpu::StoreOp::Discard;
    }
    return wgpu::StoreOp::Store;
}
void WGPURenderTarget::setUpRenderPassAttachments(wgpu::RenderPassDescriptor* descriptor,
            wgpu::TextureView const& textureView, const RenderPassParams& params) {
    // auto discardFlags = params.flags.discardEnd;
    // (void) discardFlags;
    // std::vector<wgpu::RenderPassColorAttachment> colorAttachments;
    colorAttachments.clear();
    for (size_t i = 0; i < 1/*MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT*/; i++) {
        // auto attachment = getDrawColorAttachment(i);
        // if (attachment) {
            wgpu::RenderPassColorAttachment colorAttachment;
            colorAttachment.view = textureView;
            colorAttachment.loadOp  = getLoadOperation(params, getTargetBufferFlagsAt(i));
            colorAttachment.storeOp = getStoreOperation(params, getTargetBufferFlagsAt(i));
            colorAttachment.clearValue = { params.clearColor.r, params.clearColor.g, params.clearColor.b, params.clearColor.a };
            colorAttachments.emplace_back(colorAttachment);
        // }
    }
    descriptor->colorAttachments = colorAttachments.data();
    descriptor->colorAttachmentCount = colorAttachments.size();
    descriptor->depthStencilAttachment = nullptr;
    descriptor->timestampWrites = nullptr;

}


}// namespace filament::backend
