// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class TexelBufferValidationTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateTexelBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor desc;
        desc.size = size;
        desc.usage = usage;
        return device.CreateBuffer(&desc);
    }

    struct TexelBufferLayoutDescriptor {
        wgpu::TexelBufferBindingLayout layout;
        wgpu::BindGroupLayoutEntry entry;
        wgpu::BindGroupLayoutDescriptor desc;

        TexelBufferLayoutDescriptor(wgpu::TexelBufferAccess access,
                                    wgpu::ShaderStage visibility,
                                    wgpu::TextureFormat format,
                                    uint32_t bindingArraySize = 0) {
            layout = {};
            layout.access = access;
            layout.format = format;

            entry = {};
            entry.binding = 0;
            entry.visibility = visibility;
            entry.bindingArraySize = bindingArraySize;
            entry.nextInChain = &layout;

            desc = {};
            desc.entryCount = 1;
            desc.entries = &entry;
        }
    };
};

// Creation succeeds when the feature is enabled.
TEST_F(TexelBufferValidationTest, CreationSuccess) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    device.CreateBuffer(&desc);
}

// Mappable usages require BufferMapExtendedUsages when combined with TexelBuffer.
TEST_F(TexelBufferValidationTest, MappableUsageRequiresExtendedFeature) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapRead;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapWrite;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));
}

// bindingArraySize > 1 is invalid.
TEST_F(TexelBufferValidationTest, BindingArraySize) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadOnly,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::RGBA8Uint,
                                       2);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Vertex visibility requires read-only access.
TEST_F(TexelBufferValidationTest, VertexVisibilityRequiresReadOnly) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadWrite,
                                       wgpu::ShaderStage::Vertex, wgpu::TextureFormat::RGBA8Uint);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Access must not be undefined.
TEST_F(TexelBufferValidationTest, UndefinedLayoutAccess) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::Undefined,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::RGBA8Uint);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Format must not be undefined.
TEST_F(TexelBufferValidationTest, UndefinedLayoutFormat) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadOnly,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::Undefined);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// BindingInitializationHelper should chain a TexelBufferBindingEntry when initialized
// with a TexelBufferView.
TEST_F(TexelBufferValidationTest, BindingHelperChainsTexelBufferBindingEntry) {
    constexpr uint64_t kSize = 4 * 4;
    wgpu::Buffer buffer = CreateTexelBuffer(kSize, wgpu::BufferUsage::TexelBuffer);

    wgpu::TexelBufferViewDescriptor viewDesc;
    viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
    viewDesc.offset = 0;
    viewDesc.size = kSize;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    utils::BindingInitializationHelper helper(0, view);
    wgpu::BindGroupEntry entry = helper.GetAsBinding();

    EXPECT_EQ(entry.buffer, nullptr);
    EXPECT_EQ(entry.textureView, nullptr);

    ASSERT_NE(entry.nextInChain, nullptr);
    EXPECT_EQ(entry.nextInChain->sType, wgpu::SType::TexelBufferBindingEntry);

    const auto* texelEntry =
        reinterpret_cast<const wgpu::TexelBufferBindingEntry*>(entry.nextInChain);
    EXPECT_EQ(texelEntry->texelBufferView.Get(), view.Get());
}

// Creating a bind group without chaining a TexelBufferBindingEntry fails.
TEST_F(TexelBufferValidationTest, BindGroupMissingTexelBufferBindingEntry) {
    constexpr uint64_t kSize = 4 * 4;
    wgpu::Buffer buffer = CreateTexelBuffer(kSize, wgpu::BufferUsage::TexelBuffer);

    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadOnly,
                                       wgpu::ShaderStage::Compute, wgpu::TextureFormat::R32Uint);
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&helper.desc);

    // Attempt to bind the buffer directly without the required TexelBufferBindingEntry chain.
    wgpu::BindGroupEntry entry = {};
    entry.binding = 0;
    entry.buffer = buffer;
    entry.offset = 0;
    entry.size = kSize;

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &entry;

    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

// The format of a bound texel buffer view must match the layout.
TEST_F(TexelBufferValidationTest, ViewFormatMustMatchLayout) {
    wgpu::BufferDescriptor desc;
    desc.size = 256;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
    viewDesc.offset = 0;
    viewDesc.size = 256;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadOnly;
    layout.format = wgpu::TextureFormat::R32Uint;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layout}});

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Read-write texel buffer bindings require the buffer to also have STORAGE usage.
TEST_F(TexelBufferValidationTest, ReadWriteBindingRequiresStorageUsage) {
    wgpu::BufferDescriptor desc;
    desc.size = 256;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    viewDesc.offset = 0;
    viewDesc.size = 4;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadWrite;
    layout.format = wgpu::TextureFormat::R32Uint;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layout}});

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Binding a TexelBuffer to a texture binding slot fails.
TEST_F(TexelBufferValidationTest, TexelBufferCannotBindToTextureSlot) {
    wgpu::BindGroupLayoutEntry textureEntry = {};
    textureEntry.binding = 0;
    textureEntry.visibility = wgpu::ShaderStage::Compute;
    textureEntry.texture.sampleType = wgpu::TextureSampleType::Float;
    textureEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &textureEntry;
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    wgpu::Buffer buffer = CreateTexelBuffer(4, wgpu::BufferUsage::TexelBuffer);
    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingEntry texelEntry = {};
    texelEntry.texelBufferView = view;

    wgpu::BindGroupEntry bgEntry = {};
    bgEntry.binding = 0;
    bgEntry.nextInChain = &texelEntry;

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;

    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

// Check that TexelBufferBindingEntry cannot be chained with something else.
TEST_F(TexelBufferValidationTest, AdditionalChain) {
    // Make the BGL using a texel buffer.
    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadWrite;
    layout.format = wgpu::TextureFormat::R32Uint;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layout}});

    // Make the texel buffer
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 256;
    bufferDesc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    viewDesc.offset = 0;
    viewDesc.size = 4;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    // Make the texel buffer binding.
    wgpu::TexelBufferBindingEntry texelEntry = {};
    texelEntry.texelBufferView = view;

    wgpu::BindGroupEntry bgEntry = {};
    bgEntry.binding = 0;
    bgEntry.nextInChain = &texelEntry;

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;

    // Success case, having only a texel buffer chained is valid.
    device.CreateBindGroup(&bgDesc);

    // Error case, an external texture is also chained.
    wgpu::ExternalTexture externalTexture;
    {
        wgpu::TextureDescriptor plane0Desc;
        plane0Desc.size = {1, 1};
        plane0Desc.format = wgpu::TextureFormat::RGBA8Unorm;
        plane0Desc.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture plane0 = device.CreateTexture(&plane0Desc);

        std::array<float, 12> placeholderConstants;

        wgpu::ExternalTextureDescriptor desc;
        desc.yuvToRgbConversionMatrix = placeholderConstants.data();
        desc.gamutConversionMatrix = placeholderConstants.data();
        desc.srcTransferFunctionParameters = placeholderConstants.data();
        desc.dstTransferFunctionParameters = placeholderConstants.data();
        desc.cropSize = {1, 1};
        desc.apparentSize = {1, 1};
        desc.plane0 = plane0.CreateView();
        externalTexture = device.CreateExternalTexture(&desc);
    }

    wgpu::ExternalTextureBindingEntry externalTextureEntry;
    externalTextureEntry.externalTexture = externalTexture;

    texelEntry.nextInChain = &externalTextureEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

class TexelBufferValidationWithExtendedMapTest : public TexelBufferValidationTest {
  protected:
    void SetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        TexelBufferValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(!adapter.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages));
    }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::BufferMapExtendedUsages};
    }
};

// When BufferMapExtendedUsages is enabled, MapRead and MapWrite can be combined
// with TexelBuffer usage.
TEST_F(TexelBufferValidationWithExtendedMapTest, MappableUsageWithFeatureEnabled) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapRead;
    device.CreateBuffer(&desc);

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapWrite;
    device.CreateBuffer(&desc);
}

class TexelBufferFeatureDisabledTest : public ValidationTest {
  protected:
    void SetUp() override {
        std::vector<const char*> features = {"texel_buffers"};
        wgpu::DawnWGSLBlocklist blocklist;
        blocklist.blocklistedFeatureCount = features.size();
        blocklist.blocklistedFeatures = features.data();

        wgpu::DawnTogglesDescriptor togglesDesc;
        togglesDesc.nextInChain = &blocklist;

        wgpu::InstanceDescriptor nativeDesc;
        nativeDesc.nextInChain = &togglesDesc;

        wgpu::DawnWireWGSLControl wgslControl;
        wgslControl.nextInChain = &blocklist;
        wgslControl.enableExperimental = false;
        wgslControl.enableUnsafe = false;
        wgslControl.enableTesting = true;

        wgpu::InstanceDescriptor wireDesc;
        wireDesc.nextInChain = &wgslControl;

        ValidationTest::SetUp(&nativeDesc, &wireDesc);
    }
};

// Texel buffer view offset must be a multiple of kTexelBufferOffsetAlignment (256).
TEST_F(TexelBufferValidationTest, ViewOffsetAlignment) {
    constexpr uint64_t kBufferSize = 1024;
    wgpu::Buffer buffer = CreateTexelBuffer(kBufferSize, wgpu::BufferUsage::TexelBuffer);

    wgpu::TexelBufferViewDescriptor viewDesc;
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    viewDesc.size = 4;

    // offset=0 is valid.
    viewDesc.offset = 0;
    buffer.CreateTexelView(&viewDesc);

    // offset=256 is valid.
    viewDesc.offset = 256;
    buffer.CreateTexelView(&viewDesc);

    // offset=12 is invalid (not a multiple of 256).
    viewDesc.offset = 12;
    ASSERT_DEVICE_ERROR(buffer.CreateTexelView(&viewDesc));

    // offset=128 is invalid (not a multiple of 256).
    viewDesc.offset = 128;
    ASSERT_DEVICE_ERROR(buffer.CreateTexelView(&viewDesc));
}

// Creating a buffer with texel buffer bit without enabling the feature fails.
TEST_F(TexelBufferFeatureDisabledTest, BufferRequiresFeature) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));
}

// Creating a texel buffer layout without enabling the feature fails.
TEST_F(TexelBufferFeatureDisabledTest, LayoutRequiresFeature) {
    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadOnly;
    layout.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::BindGroupLayoutEntry entry = {};
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.nextInChain = &layout;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &entry;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

}  // anonymous namespace
}  // namespace dawn
