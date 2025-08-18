// Copyright 2017 The Dawn & Tint Authors
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

#include <limits>
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class BindGroupValidationTest : public ValidationTest {
  public:
    wgpu::Texture CreateTexture(wgpu::TextureUsage usage,
                                wgpu::TextureFormat format,
                                uint32_t layerCount) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = {kWidth, kHeight, layerCount};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = usage;
        descriptor.format = format;

        return device.CreateTexture(&descriptor);
    }

    void SetUp() override {
        ValidationTest::SetUp();

        // Create objects to use as resources inside test bind groups.
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::Uniform;
            mUBO = device.CreateBuffer(&descriptor);
        }
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 1024;
            descriptor.usage = wgpu::BufferUsage::Storage;
            mSSBO = device.CreateBuffer(&descriptor);
        }
        { mSampler = device.CreateSampler(); }
        {
            mSampledTexture =
                CreateTexture(wgpu::TextureUsage::TextureBinding, kDefaultTextureFormat, 1);
            mSampledTextureView = mSampledTexture.CreateView();

            wgpu::ExternalTextureDescriptor externalTextureDesc =
                CreateDefaultExternalTextureDescriptor();
            externalTextureDesc.plane0 = mSampledTextureView;
            mExternalTexture = device.CreateExternalTexture(&externalTextureDesc);
            mExternalTextureBindingEntry.externalTexture = mExternalTexture;
        }
    }

  protected:
    wgpu::ExternalTextureDescriptor CreateDefaultExternalTextureDescriptor() {
        wgpu::ExternalTextureDescriptor desc;
        desc.yuvToRgbConversionMatrix = mPlaceholderConstantArray.data();
        desc.gamutConversionMatrix = mPlaceholderConstantArray.data();
        desc.srcTransferFunctionParameters = mPlaceholderConstantArray.data();
        desc.dstTransferFunctionParameters = mPlaceholderConstantArray.data();
        desc.cropSize = {kWidth, kHeight};
        desc.apparentSize = {kWidth, kHeight};
        return desc;
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }

    void DoTextureSampleTypeTest(bool success,
                                 wgpu::TextureFormat format,
                                 wgpu::TextureSampleType sampleType,
                                 wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::BindGroupLayout layout =
            utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Fragment, sampleType}});

        wgpu::TextureDescriptor descriptor;
        descriptor.size = {4, 4, 1};
        descriptor.usage = wgpu::TextureUsage::TextureBinding;
        descriptor.format = format;

        wgpu::TextureViewDescriptor viewDescriptor;
        viewDescriptor.aspect = aspect;
        wgpu::TextureView view = device.CreateTexture(&descriptor).CreateView(&viewDescriptor);

        if (success) {
            utils::MakeBindGroup(device, layout, {{0, view}});
        } else {
            ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));
        }
    }

    wgpu::Buffer mUBO;
    wgpu::Buffer mSSBO;
    wgpu::Sampler mSampler;
    wgpu::Texture mSampledTexture;
    wgpu::TextureView mSampledTextureView;
    wgpu::ExternalTextureBindingEntry mExternalTextureBindingEntry;

    static constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

  private:
    uint32_t kWidth = 16;
    uint32_t kHeight = 16;
    wgpu::ExternalTexture mExternalTexture;
    std::array<float, 12> mPlaceholderConstantArray;
};

// Test the validation of BindGroupDescriptor::nextInChain
TEST_F(BindGroupValidationTest, NextInChainNullptr) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 0;
    descriptor.entries = nullptr;

    // Control case: check that nextInChain = nullptr is valid
    descriptor.nextInChain = nullptr;
    device.CreateBindGroup(&descriptor);

    // Check that nextInChain != nullptr is an error.
    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType(0u);
    descriptor.nextInChain = &chainedDescriptor;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
}

// Check constraints on entryCount
TEST_F(BindGroupValidationTest, EntryCountMismatch) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

    // Control case: check that a descriptor with one binding is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that entryCount != layout.entryCount fails.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {}));
}

// Check constraints on BindGroupEntry::binding
TEST_F(BindGroupValidationTest, WrongBindings) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

    // Control case: check that a descriptor with a binding matching the layout's is ok
    utils::MakeBindGroup(device, layout, {{0, mSampler}});

    // Check that binding must be present in the layout
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{1, mSampler}}));
}

// Check that the same binding cannot be set twice
TEST_F(BindGroupValidationTest, BindingSetTwice) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                 {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

    // Control case: check that different bindings work
    utils::MakeBindGroup(device, layout, {{0, mSampler}, {1, mSampler}});

    // Check that setting the same binding twice is invalid
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mSampler}, {0, mSampler}}));
}

// Check that a sampler binding must contain exactly one sampler
TEST_F(BindGroupValidationTest, SamplerBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

    wgpu::BindGroupEntry binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the sampler works
    binding.sampler = mSampler;
    device.CreateBindGroup(&descriptor);

    // Setting the texture view as well is an error
    binding.textureView = mSampledTextureView;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;

    // Setting the buffer as well is an error
    binding.buffer = mUBO;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.buffer = nullptr;

    // Setting the external texture view as well is an error
    binding.nextInChain = &mExternalTextureBindingEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.nextInChain = nullptr;

    // Setting the sampler to an error sampler is an error.
    {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.minFilter = static_cast<wgpu::FilterMode>(0xFFFFFFFF);

        wgpu::Sampler errorSampler;
        ASSERT_DEVICE_ERROR(errorSampler = device.CreateSampler(&samplerDesc));

        binding.sampler = errorSampler;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.sampler = nullptr;
    }
}

// Check that a texture binding must contain exactly a texture view
TEST_F(BindGroupValidationTest, TextureBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    wgpu::BindGroupEntry binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the texture view works
    binding.textureView = mSampledTextureView;
    device.CreateBindGroup(&descriptor);

    // Setting the sampler as well is an error
    binding.sampler = mSampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.sampler = nullptr;

    // Setting the buffer as well is an error
    binding.buffer = mUBO;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.buffer = nullptr;

    // Setting the external texture view as well is an error
    binding.nextInChain = &mExternalTextureBindingEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.nextInChain = nullptr;

    // Setting the texture view to an error texture view is an error.
    {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.format = kDefaultTextureFormat;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 0;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1000;

        wgpu::TextureView errorView;
        ASSERT_DEVICE_ERROR(errorView = mSampledTexture.CreateView(&viewDesc));

        binding.textureView = errorView;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.textureView = nullptr;
    }
}

// Check that a buffer binding must contain exactly a buffer
TEST_F(BindGroupValidationTest, BufferBindingType) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    wgpu::BindGroupEntry binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 1024;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting just the buffer works
    binding.buffer = mUBO;
    device.CreateBindGroup(&descriptor);

    // Setting the texture view as well is an error
    binding.textureView = mSampledTextureView;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;

    // Setting the sampler as well is an error
    binding.sampler = mSampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.sampler = nullptr;

    // Setting the external texture view as well is an error
    binding.nextInChain = &mExternalTextureBindingEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.nextInChain = nullptr;

    // Setting the buffer to an error buffer is an error.
    {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = 1024;
        bufferDesc.usage = static_cast<wgpu::BufferUsage>(0xFFFFFFFF);

        wgpu::Buffer errorBuffer;
        ASSERT_DEVICE_ERROR(errorBuffer = device.CreateBuffer(&bufferDesc));

        binding.buffer = errorBuffer;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.buffer = nullptr;
    }
}

// Check that an external texture binding must contain either exactly an external texture or a
// texture view.
TEST_F(BindGroupValidationTest, ExternalTextureBindingType) {
    // Create an external texture
    wgpu::Texture texture =
        CreateTexture(wgpu::TextureUsage::TextureBinding, kDefaultTextureFormat, 1);
    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group layout for a single external texture
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});

    wgpu::BindGroupEntry binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    wgpu::ExternalTextureBindingEntry externalBindingEntry;
    externalBindingEntry.externalTexture = externalTexture;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    for (bool bindTextureViewOnly : {false, true}) {
        binding.textureView = nullptr;
        binding.nextInChain = nullptr;

        if (bindTextureViewOnly) {
            // Control case: setting just the texture view works
            binding.textureView = mSampledTextureView;
            device.CreateBindGroup(&descriptor);
        } else {
            // Control case: setting just the external texture works
            binding.nextInChain = &externalBindingEntry;
            device.CreateBindGroup(&descriptor);
        }

        // Setting the sampler as well is an error
        binding.sampler = mSampler;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.sampler = nullptr;

        // Setting the buffer as well is an error
        binding.buffer = mUBO;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        binding.buffer = nullptr;

        // Setting the external texture to an error external texture is an error.
        {
            wgpu::Texture errorTexture =
                CreateTexture(wgpu::TextureUsage::TextureBinding, wgpu::TextureFormat::R8Unorm, 1);
            wgpu::ExternalTextureDescriptor errorExternalDescriptor =
                CreateDefaultExternalTextureDescriptor();
            errorExternalDescriptor.plane0 = errorTexture.CreateView();

            wgpu::ExternalTexture errorExternalTexture;
            ASSERT_DEVICE_ERROR(errorExternalTexture =
                                    device.CreateExternalTexture(&errorExternalDescriptor));

            wgpu::ExternalTextureBindingEntry errorExternalBindingEntry;
            errorExternalBindingEntry.externalTexture = errorExternalTexture;
            binding.nextInChain = &errorExternalBindingEntry;
            ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
            binding.nextInChain = nullptr;
        }

        if (!bindTextureViewOnly) {
            // Setting the texture view as well is an error
            binding.nextInChain = &externalBindingEntry;
            binding.textureView = mSampledTextureView;
            ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
            binding.nextInChain = nullptr;
            binding.textureView = nullptr;

            // Setting an external texture with another external texture chained is an error.
            wgpu::ExternalTexture externalTexture2 = device.CreateExternalTexture(&externalDesc);
            wgpu::ExternalTextureBindingEntry externalBindingEntry2;
            externalBindingEntry2.externalTexture = externalTexture2;
            externalBindingEntry.nextInChain = &externalBindingEntry2;

            ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        }

        // Chaining a struct that isn't an external texture binding entry is an error.
        {
            wgpu::ExternalTextureBindingLayout externalBindingLayout;
            binding.nextInChain = &externalBindingLayout;
            ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
        }
    }
}

// Check that a texture binding must have the correct usage
TEST_F(BindGroupValidationTest, TextureUsage) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // Control case: setting a sampleable texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make an render attachment texture and try to set it for a SampledTexture binding
    {
        wgpu::Texture outputTexture =
            CreateTexture(wgpu::TextureUsage::RenderAttachment, wgpu::TextureFormat::RGBA8Unorm, 1);
        wgpu::TextureView outputTextureView = outputTexture.CreateView();
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, outputTextureView}}));
    }

    // Make a sampled/render attachment texture and a view without sampling and attempt to bind it
    {
        wgpu::Texture outputTexture =
            CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
                          wgpu::TextureFormat::RGBA8Unorm, 1);
        wgpu::TextureViewDescriptor viewDescriptor;
        viewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        viewDescriptor.baseMipLevel = 0;
        viewDescriptor.mipLevelCount = 1;
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::TextureView outputTextureView = outputTexture.CreateView(&viewDescriptor);
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, outputTextureView}}));
    }
}

// Check that a storage texture binding must have the correct usage
TEST_F(BindGroupValidationTest, StorageTextureUsage) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly,
                  wgpu::TextureFormat::RGBA8Uint}});

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.sampleCount = 1;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::StorageBinding;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::TextureView view = device.CreateTexture(&descriptor).CreateView();

    // Control case: setting a storage texture view works.
    utils::MakeBindGroup(device, layout, {{0, view}});

    // Sampled texture is invalid with storage buffer binding
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    view = device.CreateTexture(&descriptor).CreateView();
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));

    // Multisampled texture is invalid with storage buffer binding
    // Regression case for crbug.com/dawn/614 where this hit an DAWN_ASSERT.
    descriptor.sampleCount = 4;
    descriptor.usage |= wgpu::TextureUsage::RenderAttachment;
    view = device.CreateTexture(&descriptor).CreateView();
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));
}

// Check that a texture must have the correct sample type
TEST_F(BindGroupValidationTest, TextureSampleType) {
    // Test that RGBA8Unorm is only compatible with float/unfilterable-float.
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureSampleType::Sint);

    // Test that float32 formats are only compatible with unfilterable-float (without the
    // float32-filterable feature enabled).
    for (const auto f32Format : {wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RG32Float,
                                 wgpu::TextureFormat::RGBA32Float}) {
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Float);
        DoTextureSampleTypeTest(true, f32Format, wgpu::TextureSampleType::UnfilterableFloat);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Depth);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Uint);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Sint);
    }

    // Test that Depth16Unorm is only compatible with depth/unfilterable-float.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth16Unorm,
                            wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth16Unorm,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth16Unorm,
                            wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth16Unorm,
                            wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth16Unorm,
                            wgpu::TextureSampleType::Sint);

    // Test that Depth24Plus is only compatible with depth/unfilterable-float.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24Plus,
                            wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth24Plus,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth24Plus, wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24Plus, wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24Plus, wgpu::TextureSampleType::Sint);

    // Test that Depth24PlusStencil8 with depth aspect is only compatible with
    // depth/unfilterable-float.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Float, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::UnfilterableFloat,
                            wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Depth, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Uint, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Sint, wgpu::TextureAspect::DepthOnly);

    // Test that Depth24PlusStencil8 with stencil aspect is only compatible with uint.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Float, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::UnfilterableFloat,
                            wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Depth, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Uint, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth24PlusStencil8,
                            wgpu::TextureSampleType::Sint, wgpu::TextureAspect::StencilOnly);

    // Test that Depth32Float is only compatible with depth/unfilterable-float.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32Float,
                            wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth32Float,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth32Float,
                            wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32Float,
                            wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32Float,
                            wgpu::TextureSampleType::Sint);

    // Test that Depth32FloatStencil8 with depth aspect is only compatible with
    // depth/unfilterable-float.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Float, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::UnfilterableFloat,
                            wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Depth, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Uint, wgpu::TextureAspect::DepthOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Sint, wgpu::TextureAspect::DepthOnly);

    // Test that Depth32FloatStencil8 with stencil aspect is only compatible with uint.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Float, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::UnfilterableFloat,
                            wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Depth, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Uint, wgpu::TextureAspect::StencilOnly);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::Depth32FloatStencil8,
                            wgpu::TextureSampleType::Sint, wgpu::TextureAspect::StencilOnly);

    // Test that RG8Uint is only compatible with uint.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RG8Uint, wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RG8Uint,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RG8Uint, wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::RG8Uint, wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::RG8Uint, wgpu::TextureSampleType::Sint);

    // Test that R16Sint is only compatible with sint.
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::R16Sint, wgpu::TextureSampleType::Float);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::R16Sint,
                            wgpu::TextureSampleType::UnfilterableFloat);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::R16Sint, wgpu::TextureSampleType::Depth);
    DoTextureSampleTypeTest(false, wgpu::TextureFormat::R16Sint, wgpu::TextureSampleType::Uint);
    DoTextureSampleTypeTest(true, wgpu::TextureFormat::R16Sint, wgpu::TextureSampleType::Sint);
}

class BindGroupValidationTest_Float32Filterable : public BindGroupValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Float32Filterable};
    }
};

// Checks that float32 texture formats have the correct sample type when float32-filterable feature
// enabled.
TEST_F(BindGroupValidationTest_Float32Filterable, TextureSampleType) {
    // With the feature enabled, float32 formats should be compatible with both float and
    // unfilterable-float.
    for (const auto f32Format : {wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RG32Float,
                                 wgpu::TextureFormat::RGBA32Float}) {
        DoTextureSampleTypeTest(true, f32Format, wgpu::TextureSampleType::Float);
        DoTextureSampleTypeTest(true, f32Format, wgpu::TextureSampleType::UnfilterableFloat);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Depth);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Uint);
        DoTextureSampleTypeTest(false, f32Format, wgpu::TextureSampleType::Sint);
    }
}

// Test which depth-stencil formats are allowed to be sampled (all).
TEST_F(BindGroupValidationTest, SamplingDepthStencilTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});

    wgpu::TextureDescriptor desc;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::TextureBinding;

    // Depth32Float is allowed to be sampled.
    {
        desc.format = wgpu::TextureFormat::Depth32Float;
        wgpu::Texture texture = device.CreateTexture(&desc);

        utils::MakeBindGroup(device, layout, {{0, texture.CreateView()}});
    }

    // Depth24Plus is allowed to be sampled.
    {
        desc.format = wgpu::TextureFormat::Depth24Plus;
        wgpu::Texture texture = device.CreateTexture(&desc);

        utils::MakeBindGroup(device, layout, {{0, texture.CreateView()}});
    }

    // Depth24PlusStencil8 is allowed to be sampled, if the depth or stencil aspect is selected.
    {
        desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        wgpu::Texture texture = device.CreateTexture(&desc);
        wgpu::TextureViewDescriptor viewDesc = {};

        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&viewDesc)}});

        layout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});

        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;
        utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&viewDesc)}});
    }
}

// Check that a texture must have the correct dimension
TEST_F(BindGroupValidationTest, TextureDimension) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // Control case: setting a 2D texture view works.
    utils::MakeBindGroup(device, layout, {{0, mSampledTextureView}});

    // Make a 2DArray texture and try to set it to a 2D binding.
    wgpu::Texture arrayTexture =
        CreateTexture(wgpu::TextureUsage::TextureBinding, wgpu::TextureFormat::RGBA8Uint, 2);
    wgpu::TextureView arrayTextureView = arrayTexture.CreateView();

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, arrayTextureView}}));
}

// Check that a storage texture binding must have a texture view with a mipLevelCount of 1
TEST_F(BindGroupValidationTest, StorageTextureViewLayerCount) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly,
                  wgpu::TextureFormat::RGBA8Uint}});

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size = {16, 16, 1};
    descriptor.sampleCount = 1;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::StorageBinding;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::Texture textureNoMip = device.CreateTexture(&descriptor);

    descriptor.mipLevelCount = 3;
    wgpu::Texture textureMip = device.CreateTexture(&descriptor);

    // Control case: setting a storage texture view on a texture with only one mip level works
    {
        wgpu::TextureView view = textureNoMip.CreateView();
        utils::MakeBindGroup(device, layout, {{0, view}});
    }

    // Setting a storage texture view with mipLevelCount=1 on a texture of multiple mip levels is
    // valid
    {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.aspect = wgpu::TextureAspect::All;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;

        // Setting texture view with lod 0 is valid
        wgpu::TextureView view = textureMip.CreateView(&viewDesc);
        utils::MakeBindGroup(device, layout, {{0, view}});

        // Setting texture view with other lod is also valid
        viewDesc.baseMipLevel = 2;
        view = textureMip.CreateView(&viewDesc);
        utils::MakeBindGroup(device, layout, {{0, view}});
    }

    // Texture view with mipLevelCount > 1 is invalid
    {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.aspect = wgpu::TextureAspect::All;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 2;

        // Setting texture view with lod 0 and 1 is invalid
        wgpu::TextureView view = textureMip.CreateView(&viewDesc);
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));

        // Setting texture view with lod 1 and 2 is invalid
        viewDesc.baseMipLevel = 1;
        view = textureMip.CreateView(&viewDesc);
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));
    }
}

// Check that a UBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageUBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    // Control case: using a buffer with the uniform usage works
    utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}});

    // Using a buffer without the uniform usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}}));
}

// Check that a SSBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageSSBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a buffer without the storage usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}}));
}

// Check that a readonly SSBO must have the correct usage
TEST_F(BindGroupValidationTest, BufferUsageReadonlySSBO) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a buffer without the storage usage fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 256}}));
}

// Check that a resolve buffer with internal storge usage cannot be used as SSBO
TEST_F(BindGroupValidationTest, BufferUsageQueryResolve) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    // Control case: using a buffer with the storage usage works
    utils::MakeBindGroup(device, layout, {{0, mSSBO, 0, 256}});

    // Using a resolve buffer with the internal storage usage fails
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 1024;
    descriptor.usage = wgpu::BufferUsage::QueryResolve;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256}}));
}

// Tests constraints on the buffer offset for bind groups.
TEST_F(BindGroupValidationTest, BufferOffsetAlignment) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                });

    // Check that offset 0 is valid
    utils::MakeBindGroup(device, layout, {{0, mUBO, 0, 512}});

    // Check that offset 256 (aligned) is valid
    utils::MakeBindGroup(device, layout, {{0, mUBO, 256, 256}});

    // Check cases where unaligned buffer offset is invalid
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 1, 256}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 128, 256}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, mUBO, 255, 256}}));
}

// Tests constraints on the texture for MultisampledTexture bindings
TEST_F(BindGroupValidationTest, MultisampledTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat,
                  wgpu::TextureViewDimension::e2D, true}});

    wgpu::BindGroupEntry binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;
    binding.offset = 0;
    binding.size = 0;

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Not setting anything fails
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));

    // Control case: setting a multisampled 2D texture works
    wgpu::TextureDescriptor textureDesc;
    textureDesc.sampleCount = 4;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.size = {1, 1, 1};
    wgpu::Texture msTexture = device.CreateTexture(&textureDesc);

    binding.textureView = msTexture.CreateView();
    device.CreateBindGroup(&descriptor);
    binding.textureView = nullptr;

    // Error case: setting a single sampled 2D texture is an error.
    binding.textureView = mSampledTextureView;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&descriptor));
    binding.textureView = nullptr;
}

// Tests dafault offset and size of bind group entry work as expected
TEST_F(BindGroupValidationTest, BufferBindingDefaultOffsetAndSize) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 768;  // 768 = 256 x 3
    bufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    bufferDesc.size = 260;
    wgpu::Buffer bufferSized260 = device.CreateBuffer(&bufferDesc);
    bufferDesc.size = 256;
    wgpu::Buffer bufferSized256 = device.CreateBuffer(&bufferDesc);

    // Create a layout requiring minimium buffer binding size of 260
    wgpu::BufferBindingLayout bufferBindingLayout;
    bufferBindingLayout.type = wgpu::BufferBindingType::Uniform;
    bufferBindingLayout.hasDynamicOffset = false;
    bufferBindingLayout.minBindingSize = 260;

    wgpu::BindGroupLayoutEntry bindGroupLayoutEntry;
    bindGroupLayoutEntry.binding = 0;
    bindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex;
    bindGroupLayoutEntry.buffer = bufferBindingLayout;

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor;
    bindGroupLayoutDescriptor.entryCount = 1;
    bindGroupLayoutDescriptor.entries = &bindGroupLayoutEntry;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

    // Default offset should be 0
    {
        wgpu::BindGroupEntry binding;
        binding.binding = 0;
        binding.sampler = nullptr;
        binding.textureView = nullptr;
        binding.buffer = buffer;
        // binding.offset omitted.
        binding.size = 768;

        wgpu::BindGroupDescriptor bgDesc;
        bgDesc.layout = layout;
        bgDesc.entryCount = 1;
        bgDesc.entries = &binding;

        // Offset 0 and size 768 should work.
        device.CreateBindGroup(&bgDesc);

        // Offset 0 and size 260 should work.
        binding.size = 260;
        device.CreateBindGroup(&bgDesc);

        // Offset 0 and size 256 go smaller than minBindingSize and validation error.
        binding.size = 256;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));

        // Offset 0 and size 769 should be OOB and validation error.
        binding.size = 769;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
    }

    // Default size should be whole size - offset
    {
        wgpu::BindGroupEntry binding;
        binding.binding = 0;
        binding.sampler = nullptr;
        binding.textureView = nullptr;
        binding.buffer = buffer;
        binding.offset = 0;
        // binding.size omitted

        wgpu::BindGroupDescriptor bgDesc;
        bgDesc.layout = layout;
        bgDesc.entryCount = 1;
        bgDesc.entries = &binding;

        // Offset 0 and default size = 768 should work.
        device.CreateBindGroup(&bgDesc);

        // Offset 256 and default size = 512 should work.
        binding.offset = 256;
        device.CreateBindGroup(&bgDesc);

        // Offset 512 and default size = 256 go smaller than minBindingSize and validation error.
        binding.offset = 512;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));

        // Offset 1024 should be OOB and validation error.
        binding.offset = 1024;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
    }

    // Both offset and size are default, should be offset = 0 and size = whole size
    {
        wgpu::BindGroupEntry binding;
        binding.binding = 0;
        binding.sampler = nullptr;
        binding.textureView = nullptr;
        binding.buffer = buffer;
        // binding.offset omitted
        // binding.size omitted

        wgpu::BindGroupDescriptor bgDesc;
        bgDesc.layout = layout;
        bgDesc.entryCount = 1;
        bgDesc.entries = &binding;

        // Offset 0 and default size = 768 should work.
        device.CreateBindGroup(&bgDesc);

        // Use a buffer with size 260, offset 0 and default size = 260 should work.
        binding.buffer = bufferSized260;
        device.CreateBindGroup(&bgDesc);

        // Use a buffer with size 256, offset 0 and default size = 256 go smaller than
        // minBindingSize and validation error.
        binding.buffer = bufferSized256;
        ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
    }
}

// Tests constraints to be sure the buffer binding fits in the buffer
TEST_F(BindGroupValidationTest, BufferBindingOOB) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                });

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 1024;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    // Success case, touching the start of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256}});

    // Success case, touching the end of the buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 3 * 256, 256}});

    // Error case, zero size is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 0}}));

    // Success case, touching the full buffer works
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, 1024}});
    utils::MakeBindGroup(device, layout, {{0, buffer, 0, wgpu::kWholeSize}});

    // Success case, whole size causes the rest of the buffer to be used but not beyond.
    utils::MakeBindGroup(device, layout, {{0, buffer, 256, wgpu::kWholeSize}});

    // Error case, offset is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 256 * 5, 0}}));

    // Error case, size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 0, 256 * 5}}));

    // Error case, offset+size is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, 1024, 256}}));

    // Error case, offset+size overflows to be 0
    ASSERT_DEVICE_ERROR(
        utils::MakeBindGroup(device, layout, {{0, buffer, 256, uint32_t(0) - uint32_t(256)}}));
}

// Tests constraints to be sure the uniform buffer binding isn't too large
TEST_F(BindGroupValidationTest, MaxUniformBufferBindingSize) {
    const auto& supportedLimits = GetSupportedLimits();

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 2 * supportedLimits.maxUniformBufferBindingSize;
    descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    // Success case, this is exactly the limit
    utils::MakeBindGroup(device, uniformLayout,
                         {{0, buffer, 0, supportedLimits.maxUniformBufferBindingSize}});

    wgpu::BindGroupLayout doubleUniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                 {1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    // Success case, individual bindings don't exceed the limit
    utils::MakeBindGroup(device, doubleUniformLayout,
                         {{0, buffer, 0, supportedLimits.maxUniformBufferBindingSize},
                          {1, buffer, supportedLimits.maxUniformBufferBindingSize,
                           supportedLimits.maxUniformBufferBindingSize}});

    // Error case, this is above the limit
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(
        device, uniformLayout, {{0, buffer, 0, supportedLimits.maxUniformBufferBindingSize + 1}}));

    // Making sure the constraint doesn't apply to storage buffers
    wgpu::BindGroupLayout readonlyStorageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    // Success case, storage buffer can still be created.
    utils::MakeBindGroup(device, readonlyStorageLayout,
                         {{0, buffer, 0, 2 * supportedLimits.maxUniformBufferBindingSize}});
    utils::MakeBindGroup(device, storageLayout,
                         {{0, buffer, 0, 2 * supportedLimits.maxUniformBufferBindingSize}});
}

// Tests constraints to be sure the storage buffer binding isn't too large
TEST_F(BindGroupValidationTest, MaxStorageBufferBindingSize) {
    const auto& supportedLimits = GetSupportedLimits();

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 2 * supportedLimits.maxStorageBufferBindingSize;
    descriptor.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    // Success case, this is exactly the limit
    utils::MakeBindGroup(device, uniformLayout,
                         {{0, buffer, 0, supportedLimits.maxStorageBufferBindingSize}});

    // Success case, this is one less than the limit (check it is not an alignment constraint)
    utils::MakeBindGroup(device, uniformLayout,
                         {{0, buffer, 0, supportedLimits.maxStorageBufferBindingSize - 4}});

    wgpu::BindGroupLayout doubleUniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage},
                 {1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    // Success case, individual bindings don't exceed the limit
    utils::MakeBindGroup(device, doubleUniformLayout,
                         {{0, buffer, 0, supportedLimits.maxStorageBufferBindingSize},
                          {1, buffer, supportedLimits.maxStorageBufferBindingSize,
                           supportedLimits.maxStorageBufferBindingSize}});

    // Error case, this is above the limit
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(
        device, uniformLayout, {{0, buffer, 0, supportedLimits.maxStorageBufferBindingSize + 4}}));
}

// Test constraints to be sure the effective storage and read-only storage buffer binding size must
// be a multiple of 4.
TEST_F(BindGroupValidationTest, EffectiveStorageBufferBindingSize) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 262;
    descriptor.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr std::array<wgpu::BufferBindingType, 2> kStorageBufferBindingTypes = {
        {wgpu::BufferBindingType::Storage, wgpu::BufferBindingType::ReadOnlyStorage}};

    for (wgpu::BufferBindingType bufferBindingType : kStorageBufferBindingTypes) {
        wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, bufferBindingType}});

        // Error case, as the effective buffer binding size (262) isn't a multiple of 4.
        {
            constexpr uint32_t kOffset = 0;
            ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, kOffset}}));
        }

        // Error case, as the effective buffer binding size (6) isn't a multiple of 4.
        {
            constexpr uint32_t kOffset = 256;
            ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, buffer, kOffset}}));
        }

        // Error case, as the effective buffer binding size (2) isn't a multiple of 4.
        {
            constexpr uint32_t kOffset = 0;
            constexpr uint32_t kBindingSize = 2;
            ASSERT_DEVICE_ERROR(
                utils::MakeBindGroup(device, layout, {{0, buffer, kOffset, kBindingSize}}));
        }

        // Success case, as the effective buffer binding size (4) is a multiple of 4.
        {
            constexpr uint32_t kOffset = 0;
            constexpr uint32_t kBindingSize = 4;
            utils::MakeBindGroup(device, layout, {{0, buffer, kOffset, kBindingSize}});
        }
    }
}

// Test making that all the bindings for an BGL with bindingArraySize must be specified.
TEST_F(BindGroupValidationTest, AllArrayElementsMustBeSpecified) {
    // Create the BGL with three entries for the array.
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 1;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.bindingArraySize = 3;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entry;
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    // The texture used for the test.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture t = device.CreateTexture(&tDesc);

    // Success case, making a BindGroup with all three entries
    utils::MakeBindGroup(device, bgl,
                         {
                             {1, t.CreateView()},
                             {2, t.CreateView()},
                             {3, t.CreateView()},
                         });

    // Error case, one element is missing
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl,
                                             {
                                                 {1, t.CreateView()},
                                                 {3, t.CreateView()},
                                             }));
}

// Test what happens when the layout is an error.
TEST_F(BindGroupValidationTest, ErrorLayout) {
    wgpu::BindGroupLayout goodLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                });

    wgpu::BindGroupLayout errorLayout;
    ASSERT_DEVICE_ERROR(
        errorLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                        {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                    }));

    // Control case, creating with the good layout works
    utils::MakeBindGroup(device, goodLayout, {{0, mUBO, 0, 256}});

    // Creating with an error layout fails
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, errorLayout, {{0, mUBO, 0, 256}}));
}

class BindGroupLayoutValidationTest : public ValidationTest {
  public:
    wgpu::BindGroupLayout MakeBindGroupLayout(wgpu::BindGroupLayoutEntry* binding, uint32_t count) {
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = count;
        descriptor.entries = binding;
        return device.CreateBindGroupLayout(&descriptor);
    }

    void TestCreateBindGroupLayout(wgpu::BindGroupLayoutEntry* binding,
                                   uint32_t count,
                                   bool expected) {
        wgpu::BindGroupLayoutDescriptor descriptor;

        descriptor.entryCount = count;
        descriptor.entries = binding;

        if (!expected) {
            ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
        } else {
            device.CreateBindGroupLayout(&descriptor);
        }
    }

    void TestCreatePipelineLayout(wgpu::BindGroupLayout* bgl, uint32_t count, bool expected) {
        wgpu::PipelineLayoutDescriptor descriptor;

        descriptor.bindGroupLayoutCount = count;
        descriptor.bindGroupLayouts = bgl;

        if (!expected) {
            ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&descriptor));
        } else {
            device.CreatePipelineLayout(&descriptor);
        }
    }
};

// Tests setting storage buffer and readonly storage buffer bindings in vertex and fragment shader.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutStorageBindingsInVertexShader) {
    // Checks that storage buffer binding is not supported in vertex shader.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Storage}}));

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::ReadOnlyStorage}});

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage}});
}

// Tests setting that bind group layout bindings numbers may be very large.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutEntryMax) {
    // Check that up to kMaxBindingsPerBindGroup-1 is valid.
    utils::MakeBindGroupLayout(device, {{kMaxBindingsPerBindGroup - 1, wgpu::ShaderStage::Vertex,
                                         wgpu::BufferBindingType::Uniform}});

    // But after is an error.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device,
        {{kMaxBindingsPerBindGroup, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}}));
}

// This test verifies that the BindGroupLayout bindings are correctly validated, even if the
// binding ids are out-of-order.
TEST_F(BindGroupLayoutValidationTest, BindGroupEntry) {
    utils::MakeBindGroupLayout(device,
                               {
                                   {1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                                   {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                               });
}

// Check that dynamic = true is only allowed buffer bindings.
TEST_F(BindGroupLayoutValidationTest, DynamicAndTypeCompatibility) {
    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, true},
                });

    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true},
                });

    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                });
}

// Test that it is invalid to create a BGL with more than one binding type set.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutEntryTooManySet) {
    wgpu::BindGroupLayoutEntry entry = {};
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor),
                        testing::HasSubstr("had more than one of"));
}

// Test that it is invalid to create a BGL with none one of buffer,
// sampler, texture, storageTexture, or externalTexture set.
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutEntryNoneSet) {
    wgpu::BindGroupLayoutEntry entry = {};

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor),
                        testing::HasSubstr("had none of"));
}

// This test verifies that visibility of bindings in BindGroupLayout can be none
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutVisibilityNone) {
    utils::MakeBindGroupLayout(device,
                               {
                                   {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                               });

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    device.CreateBindGroupLayout(&descriptor);
}

// This test verifies that binding with none visibility in bind group layout can be supported in
// bind group
TEST_F(BindGroupLayoutValidationTest, BindGroupLayoutVisibilityNoneExpectsBindGroupEntry) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform},
                    {1, wgpu::ShaderStage::None, wgpu::BufferBindingType::Uniform},
                });
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    utils::MakeBindGroup(device, bgl, {{0, buffer}, {1, buffer}});

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, buffer}}));
}

#define BGLEntryType(...) \
    utils::BindingLayoutEntryInitializationHelper(0, wgpu::ShaderStage::Compute, __VA_ARGS__)

TEST_F(BindGroupLayoutValidationTest, PerStageLimits) {
    struct TestInfo {
        uint32_t maxCount;
        wgpu::BindGroupLayoutEntry entry;
        wgpu::BindGroupLayoutEntry otherEntry;
    };

    const auto& limits = GetSupportedLimits();

    std::array<TestInfo, 7> kTestInfos = {
        TestInfo{limits.maxSampledTexturesPerShaderStage,
                 BGLEntryType(wgpu::TextureSampleType::Float),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxSamplersPerShaderStage,
                 BGLEntryType(wgpu::SamplerBindingType::Filtering),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxSamplersPerShaderStage,
                 BGLEntryType(wgpu::SamplerBindingType::Comparison),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxStorageBuffersPerShaderStage,
                 BGLEntryType(wgpu::BufferBindingType::Storage),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{
            limits.maxStorageTexturesPerShaderStage,
            BGLEntryType(wgpu::StorageTextureAccess::WriteOnly, wgpu::TextureFormat::RGBA8Unorm),
            BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxUniformBuffersPerShaderStage,
                 BGLEntryType(wgpu::BufferBindingType::Uniform),
                 BGLEntryType(wgpu::TextureSampleType::Float)},
        // External textures use multiple bindings (3 sampled textures, 1 sampler, 1 uniform buffer)
        // that count towards the per stage binding limits. The number of external textures are
        // currently restricted by the maximum number of sampled textures.
        TestInfo{limits.maxSampledTexturesPerShaderStage / kSampledTexturesPerExternalTexture,
                 BGLEntryType(&utils::kExternalTextureBindingLayout),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)}};

    for (TestInfo info : kTestInfos) {
        wgpu::BindGroupLayout bgl[2];
        std::vector<utils::BindingLayoutEntryInitializationHelper> maxBindings;

        for (uint32_t i = 0; i < info.maxCount; ++i) {
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = i;
            maxBindings.push_back(entry);
        }

        // Creating with the maxes works.
        bgl[0] = MakeBindGroupLayout(maxBindings.data(), maxBindings.size());

        // Adding an extra binding of a different type works.
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.otherEntry;
            entry.binding = info.maxCount;
            bindings.push_back(entry);
            MakeBindGroupLayout(bindings.data(), bindings.size());
        }

        // Adding an extra binding of the maxed type in a different stage works
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = info.maxCount;
            entry.visibility = wgpu::ShaderStage::Fragment;
            bindings.push_back(entry);
            MakeBindGroupLayout(bindings.data(), bindings.size());
        }

        // Adding an extra binding of the maxed type and stage exceeds the per stage limit.
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = info.maxCount;
            bindings.push_back(entry);
            ASSERT_DEVICE_ERROR(MakeBindGroupLayout(bindings.data(), bindings.size()));
        }

        // Creating a pipeline layout from the valid BGL works.
        TestCreatePipelineLayout(bgl, 1, true);

        // Adding an extra binding of a different type in a different BGL works
        bgl[1] = utils::MakeBindGroupLayout(device, {info.otherEntry});
        TestCreatePipelineLayout(bgl, 2, true);

        {
            // Adding an extra binding of the maxed type in a different stage works
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.visibility = wgpu::ShaderStage::Fragment;
            bgl[1] = utils::MakeBindGroupLayout(device, {entry});
            TestCreatePipelineLayout(bgl, 2, true);
        }

        // Adding an extra binding of the maxed type in a different BGL exceeds the per stage limit.
        bgl[1] = utils::MakeBindGroupLayout(device, {info.entry});
        TestCreatePipelineLayout(bgl, 2, false);
    }
}

// External textures require multiple binding slots (3 sampled texture, 1 uniform buffer, 1
// sampler), so ensure that these count towards the limit when combined non-external texture
// bindings.
TEST_F(BindGroupLayoutValidationTest, PerStageLimitsWithExternalTexture) {
    struct TestInfo {
        uint32_t maxCount;
        uint32_t bindingsPerExternalTexture;
        wgpu::BindGroupLayoutEntry entry;
        wgpu::BindGroupLayoutEntry otherEntry;
    };

    const auto& limits = GetSupportedLimits();

    std::array<TestInfo, 3> kTestInfos = {
        TestInfo{limits.maxSampledTexturesPerShaderStage, kSampledTexturesPerExternalTexture,
                 BGLEntryType(wgpu::TextureSampleType::Float),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxSamplersPerShaderStage, kSamplersPerExternalTexture,
                 BGLEntryType(wgpu::SamplerBindingType::Filtering),
                 BGLEntryType(wgpu::BufferBindingType::Uniform)},
        TestInfo{limits.maxUniformBuffersPerShaderStage, kUniformsPerExternalTexture,
                 BGLEntryType(wgpu::BufferBindingType::Uniform),
                 BGLEntryType(wgpu::TextureSampleType::Float)},
    };

    for (TestInfo info : kTestInfos) {
        wgpu::BindGroupLayout bgl[2];
        std::vector<utils::BindingLayoutEntryInitializationHelper> maxBindings;

        // Create an external texture binding layout entry
        wgpu::BindGroupLayoutEntry firstEntry = BGLEntryType(&utils::kExternalTextureBindingLayout);
        firstEntry.binding = 0;
        maxBindings.push_back(firstEntry);

        // Create the other bindings such that we reach the max bindings per stage when including
        // the external texture.
        for (uint32_t i = 1; i <= info.maxCount - info.bindingsPerExternalTexture; ++i) {
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = i;
            maxBindings.push_back(entry);
        }

        // Ensure that creation without the external texture works.
        bgl[0] = MakeBindGroupLayout(maxBindings.data(), maxBindings.size());

        // Adding an extra binding of a different type works.
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.otherEntry;
            entry.binding = info.maxCount;
            bindings.push_back(entry);
            MakeBindGroupLayout(bindings.data(), bindings.size());
        }

        // Adding an extra binding of the maxed type in a different stage works
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = info.maxCount;
            entry.visibility = wgpu::ShaderStage::Fragment;
            bindings.push_back(entry);
            MakeBindGroupLayout(bindings.data(), bindings.size());
        }

        // Adding an extra binding of the maxed type and stage exceeds the per stage limit.
        {
            std::vector<utils::BindingLayoutEntryInitializationHelper> bindings = maxBindings;
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.binding = info.maxCount;
            bindings.push_back(entry);
            ASSERT_DEVICE_ERROR(MakeBindGroupLayout(bindings.data(), bindings.size()));
        }

        // Creating a pipeline layout from the valid BGL works.
        TestCreatePipelineLayout(bgl, 1, true);

        // Adding an extra binding of a different type in a different BGL works
        bgl[1] = utils::MakeBindGroupLayout(device, {info.otherEntry});
        TestCreatePipelineLayout(bgl, 2, true);

        {
            // Adding an extra binding of the maxed type in a different stage works
            wgpu::BindGroupLayoutEntry entry = info.entry;
            entry.visibility = wgpu::ShaderStage::Fragment;
            bgl[1] = utils::MakeBindGroupLayout(device, {entry});
            TestCreatePipelineLayout(bgl, 2, true);
        }

        // Adding an extra binding of the maxed type in a different BGL exceeds the per stage limit.
        bgl[1] = utils::MakeBindGroupLayout(device, {info.entry});
        TestCreatePipelineLayout(bgl, 2, false);
    }
}

// Check that dynamic buffer numbers exceed maximum value in one bind group layout.
TEST_F(BindGroupLayoutValidationTest, DynamicBufferNumberLimit) {
    wgpu::BindGroupLayout bgl[2];
    std::vector<wgpu::BindGroupLayoutEntry> maxUniformDB;
    std::vector<wgpu::BindGroupLayoutEntry> maxStorageDB;
    std::vector<wgpu::BindGroupLayoutEntry> maxReadonlyStorageDB;

    const auto& limits = GetSupportedLimits();

    // In this test, we use all the same shader stage. Ensure that this does not exceed the
    // per-stage limit.
    DAWN_ASSERT(limits.maxDynamicUniformBuffersPerPipelineLayout <=
                limits.maxUniformBuffersPerShaderStage);
    DAWN_ASSERT(limits.maxDynamicStorageBuffersPerPipelineLayout <=
                limits.maxStorageBuffersPerShaderStage);

    for (uint32_t i = 0; i < limits.maxDynamicUniformBuffersPerPipelineLayout; ++i) {
        maxUniformDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            i, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, true));
    }

    for (uint32_t i = 0; i < limits.maxDynamicStorageBuffersPerPipelineLayout; ++i) {
        maxStorageDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            i, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true));
    }

    for (uint32_t i = 0; i < limits.maxDynamicStorageBuffersPerPipelineLayout; ++i) {
        maxReadonlyStorageDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            i, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true));
    }

    // Test creating with the maxes works
    {
        bgl[0] = MakeBindGroupLayout(maxUniformDB.data(), maxUniformDB.size());
        TestCreatePipelineLayout(bgl, 1, true);

        bgl[0] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());
        TestCreatePipelineLayout(bgl, 1, true);

        bgl[0] = MakeBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size());
        TestCreatePipelineLayout(bgl, 1, true);
    }

    // The following tests exceed the per-pipeline layout limits. We use the Fragment stage to
    // ensure we don't hit the per-stage limit.

    // Check dynamic uniform buffers exceed maximum in pipeline layout.
    {
        bgl[0] = MakeBindGroupLayout(maxUniformDB.data(), maxUniformDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true},
                    });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic storage buffers exceed maximum in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage, true},
                    });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic readonly storage buffers exceed maximum in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device,
            {
                {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage, true},
            });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic storage buffers + dynamic readonly storage buffers exceed maximum storage
    // buffers in pipeline layout
    {
        bgl[0] = MakeBindGroupLayout(maxStorageDB.data(), maxStorageDB.size());
        bgl[1] = utils::MakeBindGroupLayout(
            device,
            {
                {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::ReadOnlyStorage, true},
            });

        TestCreatePipelineLayout(bgl, 2, false);
    }

    // Check dynamic uniform buffers exceed maximum in bind group layout.
    {
        maxUniformDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            limits.maxDynamicUniformBuffersPerPipelineLayout, wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::Uniform, true));
        TestCreateBindGroupLayout(maxUniformDB.data(), maxUniformDB.size(), false);
    }

    // Check dynamic storage buffers exceed maximum in bind group layout.
    {
        maxStorageDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            limits.maxDynamicStorageBuffersPerPipelineLayout, wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::Storage, true));
        TestCreateBindGroupLayout(maxStorageDB.data(), maxStorageDB.size(), false);
    }

    // Check dynamic readonly storage buffers exceed maximum in bind group layout.
    {
        maxReadonlyStorageDB.push_back(utils::BindingLayoutEntryInitializationHelper(
            limits.maxDynamicStorageBuffersPerPipelineLayout, wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::ReadOnlyStorage, true));
        TestCreateBindGroupLayout(maxReadonlyStorageDB.data(), maxReadonlyStorageDB.size(), false);
    }
}

// Test that multisampled textures must be 2D sampled textures
TEST_F(BindGroupLayoutValidationTest, MultisampledTextureViewDimension) {
    // Multisampled 2D texture works.
    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::e2D, true},
                });

    // Multisampled 2D (defaulted) texture works.
    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::Undefined, true},
                });

    // Multisampled 2D array texture is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::e2DArray, true},
                }));

    // Multisampled cube texture is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::Cube, true},
                }));

    // Multisampled cube array texture is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::CubeArray, true},
                }));

    // Multisampled 3D texture is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::e3D, true},
                }));

    // Multisampled 1D texture is invalid.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::e1D, true},
                }));
}

// Test that multisampled texture bindings are valid
TEST_F(BindGroupLayoutValidationTest, MultisampledTextureSampleType) {
    // Multisampled float sample type is not supported.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float,
                     wgpu::TextureViewDimension::e2D, true},
                }));

    // Multisampled unfilterable float sample type works.
    utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat,
                     wgpu::TextureViewDimension::e2D, true},
                });

    // Multisampled uint sample type works.
    utils::MakeBindGroupLayout(device,
                               {
                                   {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Uint,
                                    wgpu::TextureViewDimension::e2D, true},
                               });

    // Multisampled sint sample type works.
    utils::MakeBindGroupLayout(device,
                               {
                                   {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Sint,
                                    wgpu::TextureViewDimension::e2D, true},
                               });

    // Multisampled depth sample type works.
    utils::MakeBindGroupLayout(device,
                               {
                                   {0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Depth,
                                    wgpu::TextureViewDimension::e2D, true},
                               });
}

// Tests that creating a bind group layout with a valid static sampler raises an error
// if the required feature is not enabled.
TEST_F(BindGroupLayoutValidationTest, StaticSamplerNotSupportedWithoutFeatureEnabled) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Control case for a valid use of bindingArraySize
TEST_F(BindGroupLayoutValidationTest, ArraySizeSuccess) {
    for (uint32_t bindingArraySize : {0, 1, 2}) {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.bindingArraySize = bindingArraySize;
        entry.texture.sampleType = wgpu::TextureSampleType::Float;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;
        device.CreateBindGroupLayout(&desc);
    }
}

class BindGroupLayoutArraySizeDisabledValidationTest : public BindGroupValidationTest {
  protected:
    std::vector<const char*> GetEnabledToggles() override {
        return {"disable_bind_group_layout_entry_array_size"};
    }
};

// Check that using bindingArraySize > 1 is disallowed by the toggle.
TEST_F(BindGroupLayoutArraySizeDisabledValidationTest, ArraySizeDisabled) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;

    entry.bindingArraySize = 0;
    device.CreateBindGroupLayout(&desc);

    entry.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    entry.bindingArraySize = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that using a binding_array statically in an entry point is disabled.
TEST_F(BindGroupLayoutArraySizeDisabledValidationTest, BindingArrayDisabled) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var textures : binding_array<texture_2d<f32>, 3>;
        @fragment fn fs() -> @location(0) u32 {
            let _ = textures[0];
            return 0;
        }
    )"));

    // Even an array of size 1 is an error.
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var textures : binding_array<texture_2d<f32>, 1>;
        @fragment fn fs() -> @location(0) u32 {
            let _ = textures[0];
            return 0;
        }
    )"));
}

// Check that using bindingArraySize > 1 is only allowed for sampled textures.
TEST_F(BindGroupLayoutValidationTest, ArraySizeAllowedBindingTypes) {
    // Sampled texture
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.texture.sampleType = wgpu::TextureSampleType::Float;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        // Success case
        entry.bindingArraySize = 1;
        device.CreateBindGroupLayout(&desc);
        // Success case
        entry.bindingArraySize = 2;
        device.CreateBindGroupLayout(&desc);
    }

    // Uniform buffer
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.buffer.type = wgpu::BufferBindingType::Uniform;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        // Success case
        entry.bindingArraySize = 1;
        device.CreateBindGroupLayout(&desc);
        // Error case
        entry.bindingArraySize = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Storage buffer
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.buffer.type = wgpu::BufferBindingType::Uniform;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        // Success case
        entry.bindingArraySize = 1;
        device.CreateBindGroupLayout(&desc);
        // Error case
        entry.bindingArraySize = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Storage texture
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.storageTexture.format = wgpu::TextureFormat::R32Uint;
        entry.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        // Success case
        entry.bindingArraySize = 1;
        device.CreateBindGroupLayout(&desc);
        // Error case
        entry.bindingArraySize = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Sampler
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.sampler.type = wgpu::SamplerBindingType::Filtering;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        // Success case
        entry.bindingArraySize = 1;
        device.CreateBindGroupLayout(&desc);
        // Error case
        entry.bindingArraySize = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }
}

// Check that binding + bindingArraySize must fit in maxBindingsPerBindGroup
TEST_F(BindGroupLayoutValidationTest, ArrayEndPastMaxBindingsPerBindGroup) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = kMaxBindingsPerBindGroup - 1;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;

    // Success case, bindingArraySize is 1 so the last used binding is maxBindingsPerBindGroup
    entry.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    // Error case, bindingArraySize is 2 so we go past maxBindingsPerBindGroup
    entry.bindingArraySize = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that binding+bindingArraySize overflowing is caught by some validation.
TEST_F(BindGroupLayoutValidationTest, ArraySizePlusBindingOverflow) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 3;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.bindingArraySize = std::numeric_limits<uint32_t>::max();
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 1;
    desc.entries = &entry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that bindingArraySize is taken into account when looking for duplicate bindings.
TEST_F(BindGroupLayoutValidationTest, ArraySizeDuplicateBindings) {
    const uint32_t kPlaceholderBinding = 42;  // will be replaced before each check.

    // Check single entry, then an array that overlaps it.
    {
        wgpu::BindGroupLayoutEntry entries[2] = {
            {
                .binding = kPlaceholderBinding,
                .visibility = wgpu::ShaderStage::Fragment,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
            {
                .binding = 1,
                .visibility = wgpu::ShaderStage::Fragment,
                .bindingArraySize = 3,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
        };
        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 2;
        desc.entries = entries;

        // Success cases, 0 and 4 are not in [1, 4)
        entries[0].binding = 0;
        device.CreateBindGroupLayout(&desc);
        entries[0].binding = 4;
        device.CreateBindGroupLayout(&desc);

        // Error cases, 1, 2, 3 are in [1, 4)
        entries[0].binding = 1;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 3;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Check an array, then a single entry that goes in it.
    {
        wgpu::BindGroupLayoutEntry entries[2] = {
            {
                .binding = 1,
                .visibility = wgpu::ShaderStage::Fragment,
                .bindingArraySize = 3,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
            {
                .binding = kPlaceholderBinding,
                .visibility = wgpu::ShaderStage::Fragment,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
        };
        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 2;
        desc.entries = entries;

        // Success cases, 0 and 4 are not in [1, 4)
        entries[1].binding = 0;
        device.CreateBindGroupLayout(&desc);
        entries[1].binding = 4;
        device.CreateBindGroupLayout(&desc);

        // Error cases, 1, 2, 3 are in [1, 4)
        entries[1].binding = 1;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[1].binding = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[1].binding = 3;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }

    // Check two arrays that overlap
    {
        wgpu::BindGroupLayoutEntry entries[2] = {
            {
                .binding = kPlaceholderBinding,
                .visibility = wgpu::ShaderStage::Fragment,
                .bindingArraySize = 3,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
            {
                .binding = 3,
                .visibility = wgpu::ShaderStage::Fragment,
                .bindingArraySize = 3,
                .texture = {.sampleType = wgpu::TextureSampleType::Float},
            },
        };
        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 2;
        desc.entries = entries;

        // Success cases, 0 and 6 make the arrays not overlap
        entries[0].binding = 0;
        device.CreateBindGroupLayout(&desc);
        entries[0].binding = 6;
        device.CreateBindGroupLayout(&desc);

        // Error cases, 1, 2, 3, 4, 5 make the arrays overlap
        entries[0].binding = 1;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 2;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 3;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 4;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
        entries[0].binding = 5;
        ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
    }
}

// Check that bindingArraySize counts towards the binding limits.
TEST_F(BindGroupLayoutValidationTest, ArraySizeCountsTowardsLimit) {
    wgpu::BindGroupLayoutEntry entries[2] = {
        {
            .binding = 1,
            .visibility = wgpu::ShaderStage::Fragment,
            .bindingArraySize = 0,
            .texture = {.sampleType = wgpu::TextureSampleType::Float},
        },
        {
            .binding = 0,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = {.sampleType = wgpu::TextureSampleType::Float},
        },
    };
    wgpu::BindGroupLayoutDescriptor desc;
    desc.entryCount = 2;
    desc.entries = entries;

    const auto& limits = GetSupportedLimits();

    // Success case: we are just at the limit with the bindingArraySize.
    entries[0].bindingArraySize = limits.maxSampledTexturesPerShaderStage - 1;
    device.CreateBindGroupLayout(&desc);

    // Error case: we are just above the limit with the bindingArraySize.
    entries[0].bindingArraySize = limits.maxSampledTexturesPerShaderStage;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Test that only bindingArraySize = 0/1 is allowed for external textures
TEST_F(BindGroupLayoutValidationTest, ExternalTextureWithArraySize) {
    wgpu::ExternalTextureBindingLayout externalTextureLayout = {};

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.nextInChain = &externalTextureLayout;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // Success case
    binding.bindingArraySize = 0;
    device.CreateBindGroupLayout(&desc);

    // Success case
    binding.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    // Error case
    binding.bindingArraySize = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

class BindGroupLayoutWithStaticSamplersValidationTest : public BindGroupLayoutValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::StaticSamplers};
    }
};

// Tests that creating a bind group layout with a valid static sampler succeeds if the
// required feature is enabled.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest, StaticSamplerSupportedWhenFeatureEnabled) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    device.CreateBindGroupLayout(&desc);
}

// Tests that creating a bind group layout with a static sampler that has an
// invalid sampler object fails.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest, StaticSamplerWithInvalidSamplerObject) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = static_cast<wgpu::FilterMode>(0xFFFFFFFF);

    wgpu::Sampler errorSampler;
    ASSERT_DEVICE_ERROR(errorSampler = device.CreateSampler(&samplerDesc));

    staticSamplerBinding.sampler = errorSampler;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Verifies that creation of a bind group with no entry for a static sampler
// succeeds.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest, CreateBindGroupWithStaticSamplerSupported) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 0;

    device.CreateBindGroup(&descriptor);
}

// Verifies that creation of a correctly-specified bind group for a layout that
// has a static sampler and a sampler succeeds.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest,
       CreateBindGroupWithStaticSamplerAndSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding0.nextInChain = &staticSamplerBinding;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    binding1.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    utils::MakeBindGroup(device, layout, {{1, device.CreateSampler(&samplerDesc)}});
}

// Verifies that creation of a correctly-specified bind group for a layout that
// has a sampler and a static sampler succeeds.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest,
       CreateBindGroupWithSamplerAndStaticSamplerSupported) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding1.nextInChain = &staticSamplerBinding;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    utils::MakeBindGroup(device, layout, {{0, device.CreateSampler(&samplerDesc)}});
}

// Verifies that creating a bind group with an entry for a static sampler causes
// an error.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest,
       EntryForStaticSamplerInBindGroupCausesError) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    ASSERT_DEVICE_ERROR(
        utils::MakeBindGroup(device, layout, {{0, device.CreateSampler(&samplerDesc)}}));
}

// Verifies that creation of a bind group with the correct number of entries for a layout that has a
// sampler and a static sampler raises an error if the entry is specified at the
// index of the static sampler rather than that of the sampler.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest,
       CorrectNumberOfEntriesButEntryForStaticSamplerAtSecondIndexInBindGroupCausesError) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    binding0.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding1.nextInChain = &staticSamplerBinding;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    ASSERT_DEVICE_ERROR(
        utils::MakeBindGroup(device, layout, {{1, device.CreateSampler(&samplerDesc)}}));
}

// Verifies that creation of a bind group with the correct number of entries for a layout that has a
// static sampler and a sampler raises an error if the entry is specified at the
// index of the static sampler rather than that of the sampler.
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest,
       CorrectNumberOfEntriesButEntryForStaticSamplerInBindGroupCausesError) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;

    wgpu::BindGroupLayoutEntry binding0 = {};
    binding0.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding0.nextInChain = &staticSamplerBinding;
    entries.push_back(binding0);

    wgpu::BindGroupLayoutEntry binding1 = {};
    binding1.binding = 1;
    binding1.sampler.type = wgpu::SamplerBindingType::Filtering;
    entries.push_back(binding1);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 2;
    desc.entries = entries.data();

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    ASSERT_DEVICE_ERROR(
        utils::MakeBindGroup(device, layout, {{0, device.CreateSampler(&samplerDesc)}}));
}

// Test that only bindingArraySize = 0/1 is allowed for static samplers
TEST_F(BindGroupLayoutWithStaticSamplersValidationTest, StaticSamplerWithArraySize) {
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // Success case
    binding.bindingArraySize = 0;
    device.CreateBindGroupLayout(&desc);

    // Success case
    binding.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    // Error case
    binding.bindingArraySize = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

constexpr uint32_t kBindingSize = 8;

class SetBindGroupValidationTest : public ValidationTest {
  public:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::StaticSamplers};
    }

    void SetUp() override {
        ValidationTest::SetUp();

        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform, true},
                     {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform, false},
                     {2, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Storage, true},
                     {3, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::ReadOnlyStorage, true}});
        mMinUniformBufferOffsetAlignment = GetSupportedLimits().minUniformBufferOffsetAlignment;
        mBufferSize = 3 * mMinUniformBufferOffsetAlignment + 8;
    }

    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    wgpu::BindGroupLayout mBindGroupLayout;

    wgpu::RenderPipeline CreateRenderPipeline() {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                struct S {
                    value : vec2f
                }

                @group(0) @binding(0) var<uniform> uBufferDynamic : S;
                @group(0) @binding(1) var<uniform> uBuffer : S;
                @group(0) @binding(2) var<storage, read_write> sBufferDynamic : S;
                @group(0) @binding(3) var<storage, read> sReadonlyBufferDynamic : S;

                @fragment fn main() {
                })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);
        pipelineDescriptor.layout = pipelineLayout;
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline() {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
                struct S {
                    value : vec2f
                }

                @group(0) @binding(0) var<uniform> uBufferDynamic : S;
                @group(0) @binding(1) var<uniform> uBuffer : S;
                @group(0) @binding(2) var<storage, read_write> sBufferDynamic : S;
                @group(0) @binding(3) var<storage, read> sReadonlyBufferDynamic : S;

                @compute @workgroup_size(4, 4, 1) fn main() {
                })");

        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pipelineLayout;
        csDesc.compute.module = csModule;

        return device.CreateComputePipeline(&csDesc);
    }

    wgpu::BindGroup CreateAllTypesBindGroup(uint32_t bindingSize) {
        wgpu::Buffer uniformBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Uniform);
        wgpu::Buffer storageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
        wgpu::Buffer readonlyStorageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
        return utils::MakeBindGroup(device, mBindGroupLayout,
                                    {{0, uniformBuffer, 0, bindingSize},
                                     {1, uniformBuffer, 0, bindingSize},
                                     {2, storageBuffer, 0, bindingSize},
                                     {3, readonlyStorageBuffer, 0, bindingSize}});
    }

    void TestRenderPassBindGroup(wgpu::BindGroup bindGroup,
                                 uint32_t* offsets,
                                 uint32_t count,
                                 bool expectation) {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipeline();
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        if (bindGroup != nullptr) {
            renderPassEncoder.SetBindGroup(0, bindGroup, count, offsets);
        }
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

    void TestComputePassBindGroup(wgpu::BindGroup bindGroup,
                                  uint32_t* offsets,
                                  uint32_t count,
                                  bool expectation) {
        wgpu::ComputePipeline computePipeline = CreateComputePipeline();

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        if (bindGroup != nullptr) {
            computePassEncoder.SetBindGroup(0, bindGroup, count, offsets);
        }
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

  protected:
    uint32_t mMinUniformBufferOffsetAlignment;
    uint64_t mBufferSize;
};

// This is the test case that should work.
TEST_F(SetBindGroupValidationTest, Basic) {
    // Set up the bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    std::array<uint32_t, 3> offsets = {512, 256, 0};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, true);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, true);
}

// Draw/dispatch with a bind group missing is invalid
TEST_F(SetBindGroupValidationTest, MissingBindGroup) {
    TestRenderPassBindGroup(nullptr, nullptr, 0, false);
    TestComputePassBindGroup(nullptr, nullptr, 0, false);
}

// Unset the bind group required by current pipeline is invalid.
TEST_F(SetBindGroupValidationTest, UnsetBindGroupWhenNeeded) {
    // Set up the bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    std::array<uint32_t, 3> offsets = {512, 256, 0};

    // render pass case
    {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipeline();
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        // Unset the bind group which is required by the current pipeline.
        renderPassEncoder.SetBindGroup(0, nullptr);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // compute pass case
    {
        wgpu::ComputePipeline computePipeline = CreateComputePipeline();

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        // The pipeline in compute pass requires an appropriate bindgroup, which is missing.
        computePassEncoder.SetBindGroup(0, nullptr);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Regression test for the validation aspect caching not being invalidated when unsetting a
// bind group.
TEST_F(SetBindGroupValidationTest, UnsetInvalidatesBindGroupValidationCache) {
    // Set up the bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    std::array<uint32_t, 3> offsets = {512, 256, 0};

    // Render pass case
    {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipeline();
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup, 3, offsets.data());
        renderPassEncoder.Draw(3);
        // Unset the bindgroup after draw. The bindgroup might be cached by the previous draw.
        renderPassEncoder.SetBindGroup(0, nullptr);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Compute pass case
    {
        wgpu::ComputePipeline computePipeline = CreateComputePipeline();

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        computePassEncoder.SetBindGroup(0, bindGroup, 3, offsets.data());
        computePassEncoder.DispatchWorkgroups(1);
        // Unset the bindgroup after dispatch. The bindgroup might be cached by the previous
        // dispatch.
        computePassEncoder.SetBindGroup(0, nullptr);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Setting bind group after a draw / dispatch should re-verify the layout is compatible
TEST_F(SetBindGroupValidationTest, VerifyGroupIfChangedAfterAction) {
    // Set up the bind group
    wgpu::Buffer uniformBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
    wgpu::Buffer readonlyStorageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, mBindGroupLayout,
                                                     {{0, uniformBuffer, 0, kBindingSize},
                                                      {1, uniformBuffer, 0, kBindingSize},
                                                      {2, storageBuffer, 0, kBindingSize},
                                                      {3, readonlyStorageBuffer, 0, kBindingSize}});

    std::array<uint32_t, 3> offsets = {512, 256, 0};

    // Set up bind group that is incompatible
    wgpu::BindGroupLayout invalidLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});
    wgpu::BindGroup invalidGroup =
        utils::MakeBindGroup(device, invalidLayout, {{0, storageBuffer, 0, kBindingSize}});

    {
        wgpu::ComputePipeline computePipeline = CreateComputePipeline();
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);
        computePassEncoder.SetBindGroup(0, bindGroup, 3, offsets.data());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.SetBindGroup(0, invalidGroup, 0, nullptr);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
    {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipeline();
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup, 3, offsets.data());
        renderPassEncoder.Draw(3);
        renderPassEncoder.SetBindGroup(0, invalidGroup, 0, nullptr);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test cases that test dynamic offsets count mismatch with bind group layout.
TEST_F(SetBindGroupValidationTest, DynamicOffsetsMismatch) {
    // Set up bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    // Number of offsets mismatch.
    std::array<uint32_t, 4> mismatchOffsets = {768, 512, 256, 0};

    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 1, false);
    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 2, false);
    TestRenderPassBindGroup(bindGroup, mismatchOffsets.data(), 4, false);

    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 1, false);
    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 2, false);
    TestComputePassBindGroup(bindGroup, mismatchOffsets.data(), 4, false);
}

// Test cases that test dynamic offsets not aligned
TEST_F(SetBindGroupValidationTest, DynamicOffsetsNotAligned) {
    // Set up bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    // Dynamic offsets are not aligned.
    std::array<uint32_t, 3> notAlignedOffsets = {512, 128, 0};

    TestRenderPassBindGroup(bindGroup, notAlignedOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, notAlignedOffsets.data(), 3, false);
}

// Test cases that test dynamic uniform buffer out of bound situation.
TEST_F(SetBindGroupValidationTest, OffsetOutOfBoundDynamicUniformBuffer) {
    // Set up bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    // Dynamic offset + offset is larger than buffer size.
    std::array<uint32_t, 3> overFlowOffsets = {1024, 256, 0};

    TestRenderPassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);
}

// Test cases that test dynamic storage buffer out of bound situation.
TEST_F(SetBindGroupValidationTest, OffsetOutOfBoundDynamicStorageBuffer) {
    // Set up bind group.
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kBindingSize);

    // Dynamic offset + offset is larger than buffer size.
    std::array<uint32_t, 3> overFlowOffsets = {0, 256, 1024};

    TestRenderPassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, overFlowOffsets.data(), 3, false);
}

// Test cases that test dynamic uniform buffer out of bound situation because of binding size.
TEST_F(SetBindGroupValidationTest, BindingSizeOutOfBoundDynamicUniformBuffer) {
    // Set up bind group, but binding size is larger than (mBufferSize - DynamicOffset).
    constexpr uint32_t kLargeBindingSize = kBindingSize + 4u;
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kLargeBindingSize);

    // c + offset isn't larger than buffer size.
    // But with binding size, it will trigger OOB error.
    std::array<uint32_t, 3> offsets = {768, 256, 0};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, false);
}

// Test cases that test dynamic storage buffer out of bound situation because of binding size.
TEST_F(SetBindGroupValidationTest, BindingSizeOutOfBoundDynamicStorageBuffer) {
    constexpr uint32_t kLargeBindingSize = kBindingSize + 4u;
    wgpu::BindGroup bindGroup = CreateAllTypesBindGroup(kLargeBindingSize);

    // Dynamic offset + offset isn't larger than buffer size.
    // But with binding size, it will trigger OOB error.
    std::array<uint32_t, 3> offsets = {0, 256, 768};

    TestRenderPassBindGroup(bindGroup, offsets.data(), 3, false);

    TestComputePassBindGroup(bindGroup, offsets.data(), 3, false);
}

// Regression test for crbug.com/dawn/408 where dynamic offsets were applied in the wrong order.
// Dynamic offsets should be applied in increasing order of binding number.
TEST_F(SetBindGroupValidationTest, DynamicOffsetOrder) {
    // Note: The order of the binding numbers of the bind group and bind group layout are
    // intentionally different and not in increasing order.
    // This test uses both storage and uniform buffers to ensure buffer bindings are sorted first by
    // binding number before type.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {3, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, true},
                });

    // Create buffers which are 3x, 2x, and 1x the size of the minimum buffer offset, plus 4 bytes
    // to spare (to avoid zero-sized bindings). We will offset the bindings so they reach the very
    // end of the buffer. Any mismatch applying too-large of an offset to a smaller buffer will hit
    // the out-of-bounds condition during validation.
    wgpu::Buffer buffer3x =
        CreateBuffer(3 * mMinUniformBufferOffsetAlignment + 4, wgpu::BufferUsage::Storage);
    wgpu::Buffer buffer2x =
        CreateBuffer(2 * mMinUniformBufferOffsetAlignment + 4, wgpu::BufferUsage::Storage);
    wgpu::Buffer buffer1x =
        CreateBuffer(1 * mMinUniformBufferOffsetAlignment + 4, wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, buffer3x, 0, 4},
                                                         {3, buffer2x, 0, 4},
                                                         {2, buffer1x, 0, 4},
                                                     });

    std::array<uint32_t, 3> offsets;
    {
        // Base case works.
        offsets = {/* binding 0 */ 0,
                   /* binding 2 */ 0,
                   /* binding 3 */ 0};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.End();
        commandEncoder.Finish();
    }
    {
        // Offset the first binding to touch the end of the buffer. Should succeed.
        // Will fail if the offset is applied to the first or second bindings since their buffers
        // are too small.
        offsets = {/* binding 0 */ 3 * mMinUniformBufferOffsetAlignment,
                   /* binding 2 */ 0,
                   /* binding 3 */ 0};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.End();
        commandEncoder.Finish();
    }
    {
        // Offset the second binding to touch the end of the buffer. Should succeed.
        offsets = {/* binding 0 */ 0,
                   /* binding 2 */ 1 * mMinUniformBufferOffsetAlignment,
                   /* binding 3 */ 0};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.End();
        commandEncoder.Finish();
    }
    {
        // Offset the third binding to touch the end of the buffer. Should succeed.
        // Will fail if the offset is applied to the second binding since its buffer
        // is too small.
        offsets = {/* binding 0 */ 0,
                   /* binding 2 */ 0,
                   /* binding 3 */ 2 * mMinUniformBufferOffsetAlignment};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.End();
        commandEncoder.Finish();
    }
    {
        // Offset each binding to touch the end of their buffer. Should succeed.
        offsets = {/* binding 0 */ 3 * mMinUniformBufferOffsetAlignment,
                   /* binding 2 */ 1 * mMinUniformBufferOffsetAlignment,
                   /* binding 3 */ 2 * mMinUniformBufferOffsetAlignment};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
        computePassEncoder.End();
        commandEncoder.Finish();
    }
}

// Test that an error is produced (and no ASSERTs fired) when using an error bindgroup in
// SetBindGroup
TEST_F(SetBindGroupValidationTest, ErrorBindGroup) {
    // Bindgroup creation fails because not all bindings are specified.
    wgpu::BindGroup bindGroup;
    ASSERT_DEVICE_ERROR(bindGroup = utils::MakeBindGroup(device, mBindGroupLayout, {}));

    TestRenderPassBindGroup(bindGroup, nullptr, 0, false);

    TestComputePassBindGroup(bindGroup, nullptr, 0, false);
}

// Test validation of the bindgroup slot for OOB.
TEST_F(SetBindGroupValidationTest, BindGroupSlotBoundary) {
    wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
    wgpu::BindGroup emptyBindGroup = utils::MakeBindGroup(device, emptyBGL, {});

    PlaceholderRenderPass renderPass(device);

    auto TestIndex = [this, renderPass](wgpu::BindGroup bg, uint32_t i, bool valid) {
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass();
            cp.SetBindGroup(i, bg);
            cp.End();
            if (valid) {
                encoder.Finish();
            } else {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            }
        }

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder rp = encoder.BeginRenderPass(&renderPass);
            rp.SetBindGroup(i, bg);
            rp.End();
            if (valid) {
                encoder.Finish();
            } else {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            }
        }

        {
            utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
            renderBundleDesc.colorFormatCount = 1;
            renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
            wgpu::RenderBundleEncoder rb = device.CreateRenderBundleEncoder(&renderBundleDesc);
            rb.SetBindGroup(i, bg);
            if (valid) {
                rb.Finish();
            } else {
                ASSERT_DEVICE_ERROR(rb.Finish());
            }
        }
    };

    // Base
    TestIndex(emptyBindGroup, 0, true);
    // Set the last bind group slot is valid
    TestIndex(emptyBindGroup, kMaxBindGroups - 1, true);
    // Set pass the last bind group slot is invalid
    TestIndex(emptyBindGroup, kMaxBindGroups, false);

    // Unset the slot which is not set before is valid
    TestIndex(nullptr, 0, true);
    // Unset the last bind group slot is valid
    TestIndex(nullptr, kMaxBindGroups - 1, true);
    // Unset pass the last bind group slot is invalid
    TestIndex(nullptr, kMaxBindGroups, false);
}

// Test that dynamic offset count must be zero when unsetting a bindgroup.
TEST_F(SetBindGroupValidationTest, UnsetWithDynamicOffsetIsInvalid) {
    auto TestDynamicOffsetCount = [this](uint32_t count, uint32_t* offsets, bool valid) {
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass();
            cp.SetBindGroup(0, nullptr, count, offsets);
            cp.End();
            if (valid) {
                encoder.Finish();
            } else {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            }
        }

        {
            PlaceholderRenderPass renderPass(device);
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder rp = encoder.BeginRenderPass(&renderPass);
            rp.SetBindGroup(0, nullptr, count, offsets);
            rp.End();
            if (valid) {
                encoder.Finish();
            } else {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            }
        }

        {
            utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
            renderBundleDesc.colorFormatCount = 1;
            renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
            wgpu::RenderBundleEncoder rb = device.CreateRenderBundleEncoder(&renderBundleDesc);
            rb.SetBindGroup(0, nullptr, count, offsets);
            if (valid) {
                rb.Finish();
            } else {
                ASSERT_DEVICE_ERROR(rb.Finish());
            }
        }
    };

    std::array<uint32_t, 2> offsets = {256, 0};

    // When unsetting bindgroup, it is invalid if dynamicOffsetsCount > 0.
    TestDynamicOffsetCount(2, offsets.data(), false);

    // When unsetting bindgroup, it is valid if dynamicOffsets is non-null and dynamicOffsetsCount
    // is 0.
    TestDynamicOffsetCount(0, offsets.data(), true);
}

// Test that a pipeline with empty bindgroups layouts doesn't need empty bindgroups to be set.
TEST_F(SetBindGroupValidationTest, EmptyBindGroupsAreNotRequired) {
    wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
    wgpu::PipelineLayout pl =
        utils::MakePipelineLayout(device, {emptyBGL, emptyBGL, emptyBGL, emptyBGL});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroup emptyBindGroup = utils::MakeBindGroup(device, emptyBGL, {});

    // Setting 4 empty bindgroups works.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, emptyBindGroup);
        pass.SetBindGroup(1, emptyBindGroup);
        pass.SetBindGroup(2, emptyBindGroup);
        pass.SetBindGroup(3, emptyBindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }

    // Setting only the first three empty bindgroups also works.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, emptyBindGroup);
        pass.SetBindGroup(1, emptyBindGroup);
        pass.SetBindGroup(2, emptyBindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }

    // Mixedly setting and unsetting empty bindgroups also works.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(1, emptyBindGroup);
        pass.SetBindGroup(3, emptyBindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        encoder.Finish();
    }
}

// Test that a static sampler is valid to access from a shader module.
TEST_F(SetBindGroupValidationTest, StaticSamplerAccessedFromShader) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Compute;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::PipelineLayout pl = utils::MakePipelineLayout(device, {layout});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var s : sampler;
        @compute @workgroup_size(1) fn main() {
            _ = s;
        }
    )");
    device.CreateComputePipeline(&pipelineDesc);
}

// Test that a static sampler cannot be accessed from a shader module as a
// non-sampler variable type.
TEST_F(SetBindGroupValidationTest, StaticSamplerAccessedFromShaderAsNonSamplerTypeCausesError) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Compute;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = device.CreateSampler();
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&desc);

    wgpu::PipelineLayout pl = utils::MakePipelineLayout(device, {layout});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = textureDimensions(t);
        }
    )");
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&pipelineDesc));
}

class SetBindGroupPersistenceValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        mVsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        mBufferSize = 3 * GetSupportedLimits().minUniformBufferOffsetAlignment + 8;
    }

    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    // Generates bind group layouts and a pipeline from a 2D list of binding types.
    std::tuple<std::vector<wgpu::BindGroupLayout>, wgpu::RenderPipeline> SetUpLayoutsAndPipeline(
        std::vector<std::vector<wgpu::BufferBindingType>> layouts) {
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts(layouts.size());

        // Iterate through the desired bind group layouts.
        for (uint32_t l = 0; l < layouts.size(); ++l) {
            const auto& layout = layouts[l];
            std::vector<wgpu::BindGroupLayoutEntry> bindings(layout.size());

            // Iterate through binding types and populate a list of BindGroupLayoutEntrys.
            for (uint32_t b = 0; b < layout.size(); ++b) {
                bindings[b] = utils::BindingLayoutEntryInitializationHelper(
                    b, wgpu::ShaderStage::Fragment, layout[b]);
            }

            // Create the bind group layout.
            wgpu::BindGroupLayoutDescriptor bglDescriptor;
            bglDescriptor.entryCount = bindings.size();
            bglDescriptor.entries = bindings.data();
            bindGroupLayouts[l] = device.CreateBindGroupLayout(&bglDescriptor);
        }

        // Create a pipeline layout from the list of bind group layouts.
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bindGroupLayouts.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts.data();

        wgpu::PipelineLayout pipelineLayout =
            device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        std::stringstream ss;
        ss << "struct S { value : vec2f }";

        // Build a shader which has bindings that match the pipeline layout.
        for (uint32_t l = 0; l < layouts.size(); ++l) {
            const auto& layout = layouts[l];

            for (uint32_t b = 0; b < layout.size(); ++b) {
                wgpu::BufferBindingType binding = layout[b];
                ss << "@group(" << l << ") @binding(" << b << ") ";
                switch (binding) {
                    case wgpu::BufferBindingType::Storage:
                        ss << "var<storage, read_write> set" << l << "_binding" << b << " : S;";
                        break;
                    case wgpu::BufferBindingType::Uniform:
                        ss << "var<uniform> set" << l << "_binding" << b << " : S;";
                        break;
                    default:
                        DAWN_UNREACHABLE();
                }
            }
        }

        ss << "@fragment fn main() {}";

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, ss.str().c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = mVsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = pipelineLayout;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        return std::make_tuple(bindGroupLayouts, pipeline);
    }

  protected:
    uint32_t mBufferSize;

  private:
    wgpu::ShaderModule mVsModule;
};

// Test it is valid to set bind groups before setting the pipeline.
TEST_F(SetBindGroupPersistenceValidationTest, BindGroupBeforePipeline) {
    auto [bindGroupLayouts, pipeline] = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BufferBindingType::Uniform,
            wgpu::BufferBindingType::Uniform,
        }},
        {{
            wgpu::BufferBindingType::Storage,
            wgpu::BufferBindingType::Uniform,
        }},
    }});

    wgpu::Buffer uniformBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroup0 = utils::MakeBindGroup(
        device, bindGroupLayouts[0],
        {{0, uniformBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(
        device, bindGroupLayouts[1],
        {{0, storageBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    PlaceholderRenderPass renderPass(device);
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);

    renderPassEncoder.SetBindGroup(0, bindGroup0);
    renderPassEncoder.SetBindGroup(1, bindGroup1);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.Draw(3);

    renderPassEncoder.End();
    commandEncoder.Finish();
}

// Dawn does not have a concept of bind group inheritance though the backing APIs may.
// Test that it is valid to draw with bind groups that are not "inherited". They persist
// after a pipeline change.
TEST_F(SetBindGroupPersistenceValidationTest, NotVulkanInheritance) {
    auto [bindGroupLayoutsA, pipelineA] = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BufferBindingType::Uniform,
            wgpu::BufferBindingType::Storage,
        }},
        {{
            wgpu::BufferBindingType::Uniform,
            wgpu::BufferBindingType::Uniform,
        }},
    }});

    auto [bindGroupLayoutsB, pipelineB] = SetUpLayoutsAndPipeline({{
        {{
            wgpu::BufferBindingType::Storage,
            wgpu::BufferBindingType::Uniform,
        }},
        {{
            wgpu::BufferBindingType::Uniform,
            wgpu::BufferBindingType::Uniform,
        }},
    }});

    wgpu::Buffer uniformBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Uniform);
    wgpu::Buffer storageBuffer = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroupA0 = utils::MakeBindGroup(
        device, bindGroupLayoutsA[0],
        {{0, uniformBuffer, 0, kBindingSize}, {1, storageBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroupA1 = utils::MakeBindGroup(
        device, bindGroupLayoutsA[1],
        {{0, uniformBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    wgpu::BindGroup bindGroupB0 = utils::MakeBindGroup(
        device, bindGroupLayoutsB[0],
        {{0, storageBuffer, 0, kBindingSize}, {1, uniformBuffer, 0, kBindingSize}});

    PlaceholderRenderPass renderPass(device);
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);

    renderPassEncoder.SetPipeline(pipelineA);
    renderPassEncoder.SetBindGroup(0, bindGroupA0);
    renderPassEncoder.SetBindGroup(1, bindGroupA1);
    renderPassEncoder.Draw(3);

    renderPassEncoder.SetPipeline(pipelineB);
    renderPassEncoder.SetBindGroup(0, bindGroupB0);
    // This draw is valid.
    // Bind group 1 persists even though it is not "inherited".
    renderPassEncoder.Draw(3);

    renderPassEncoder.End();
    commandEncoder.Finish();
}

class BindGroupLayoutCompatibilityTest : public ValidationTest {
  public:
    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
    }

    wgpu::RenderPipeline CreateFSRenderPipeline(
        const char* fsShader,
        std::vector<wgpu::BindGroupLayout> bindGroupLayout) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fsShader);

        wgpu::PipelineLayoutDescriptor descriptor;
        descriptor.bindGroupLayoutCount = bindGroupLayout.size();
        descriptor.bindGroupLayouts = bindGroupLayout.data();
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        pipelineDescriptor.layout = pipelineLayout;
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(std::vector<wgpu::BindGroupLayout> bindGroupLayouts) {
        return CreateFSRenderPipeline(R"(
            struct S {
                value : vec2f
            }

            @group(0) @binding(0) var<storage, read_write> sBufferDynamic : S;
            @group(1) @binding(0) var<storage, read> sReadonlyBufferDynamic : S;

            @fragment fn main() {
                var val : vec2f = sBufferDynamic.value;
                val = sReadonlyBufferDynamic.value;
            })",
                                      std::move(bindGroupLayouts));
    }

    wgpu::ComputePipeline CreateComputePipeline(
        const char* shader,
        std::vector<wgpu::BindGroupLayout> bindGroupLayout) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shader);

        wgpu::PipelineLayoutDescriptor descriptor;
        descriptor.bindGroupLayoutCount = bindGroupLayout.size();
        descriptor.bindGroupLayouts = bindGroupLayout.data();
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pipelineLayout;
        csDesc.compute.module = csModule;

        return device.CreateComputePipeline(&csDesc);
    }

    wgpu::ComputePipeline CreateComputePipeline(
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts) {
        return CreateComputePipeline(R"(
            struct S {
                value : vec2f
            }

            @group(0) @binding(0) var<storage, read_write> sBufferDynamic : S;
            @group(1) @binding(0) var<storage, read> sReadonlyBufferDynamic : S;

            @compute @workgroup_size(4, 4, 1) fn main() {
                var val : vec2f = sBufferDynamic.value;
                val = sReadonlyBufferDynamic.value;
            })",
                                     std::move(bindGroupLayouts));
    }
};

// Test that it is invalid to pass a writable storage buffer in the pipeline layout when the shader
// uses the binding as a readonly storage buffer.
TEST_F(BindGroupLayoutCompatibilityTest, RWStorageInBGLWithROStorageInShader) {
    // Set up the bind group layout.
    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});
    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});

    ASSERT_DEVICE_ERROR(CreateRenderPipeline({bgl0, bgl1}));

    ASSERT_DEVICE_ERROR(CreateComputePipeline({bgl0, bgl1}));
}

// Test that it is invalid to pass a readonly storage buffer in the pipeline layout when the shader
// uses the binding as a writable storage buffer.
TEST_F(BindGroupLayoutCompatibilityTest, ROStorageInBGLWithRWStorageInShader) {
    // Set up the bind group layout.
    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::ReadOnlyStorage}});

    ASSERT_DEVICE_ERROR(CreateRenderPipeline({bgl0, bgl1}));

    ASSERT_DEVICE_ERROR(CreateComputePipeline({bgl0, bgl1}));
}

TEST_F(BindGroupLayoutCompatibilityTest, TextureViewDimension) {
    constexpr char kTexture2DShaderFS[] = R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @fragment fn main() {
            _ = textureDimensions(myTexture);
        })";
    constexpr char kTexture2DShaderCS[] = R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = textureDimensions(myTexture);
        })";

    // Render: Test that 2D texture with 2D view dimension works
    CreateFSRenderPipeline(
        kTexture2DShaderFS,
        {utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                      wgpu::TextureViewDimension::e2D}})});

    // Render: Test that 2D texture with 2D array view dimension is invalid
    ASSERT_DEVICE_ERROR(CreateFSRenderPipeline(
        kTexture2DShaderFS,
        {utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                      wgpu::TextureViewDimension::e2DArray}})}));

    // Compute: Test that 2D texture with 2D view dimension works
    CreateComputePipeline(
        kTexture2DShaderCS,
        {utils::MakeBindGroupLayout(device,
                                    {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float,
                                      wgpu::TextureViewDimension::e2D}})});

    // Compute: Test that 2D texture with 2D array view dimension is invalid
    ASSERT_DEVICE_ERROR(CreateComputePipeline(
        kTexture2DShaderCS,
        {utils::MakeBindGroupLayout(device,
                                    {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float,
                                      wgpu::TextureViewDimension::e2DArray}})}));

    constexpr char kTexture2DArrayShaderFS[] = R"(
        @group(0) @binding(0) var myTexture : texture_2d_array<f32>;
        @fragment fn main() {
            _ = textureDimensions(myTexture);
        })";
    constexpr char kTexture2DArrayShaderCS[] = R"(
        @group(0) @binding(0) var myTexture : texture_2d_array<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = textureDimensions(myTexture);
        })";

    // Render: Test that 2D texture array with 2D array view dimension works
    CreateFSRenderPipeline(
        kTexture2DArrayShaderFS,
        {utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                      wgpu::TextureViewDimension::e2DArray}})});

    // Render: Test that 2D texture array with 2D view dimension is invalid
    ASSERT_DEVICE_ERROR(CreateFSRenderPipeline(
        kTexture2DArrayShaderFS,
        {utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                      wgpu::TextureViewDimension::e2D}})}));

    // Compute: Test that 2D texture array with 2D array view dimension works
    CreateComputePipeline(
        kTexture2DArrayShaderCS,
        {utils::MakeBindGroupLayout(device,
                                    {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float,
                                      wgpu::TextureViewDimension::e2DArray}})});

    // Compute: Test that 2D texture array with 2D view dimension is invalid
    ASSERT_DEVICE_ERROR(CreateComputePipeline(
        kTexture2DArrayShaderCS,
        {utils::MakeBindGroupLayout(device,
                                    {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float,
                                      wgpu::TextureViewDimension::e2D}})}));
}

// Test that a bgl with an external texture is compatible with texture_external in a shader and that
// an error is returned when the binding in the shader does not match.
TEST_F(BindGroupLayoutCompatibilityTest, ExternalTextureBindGroupLayoutCompatibility) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});

    // Test that an external texture binding works with a texture_external in the shader.
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var myExternalTexture: texture_external;
            @fragment fn main() {
                _ = myExternalTexture;
            })",
                           {bgl});

    // Test that an external texture binding doesn't work with a texture_2d<f32> in the shader.
    ASSERT_DEVICE_ERROR(CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var myTexture: texture_2d<f32>;
            @fragment fn main() {
                _ = myTexture;
            })",
                                               {bgl}));
}

// Test that a BGL is compatible with a pipeline if a binding's array size is at least as big as the
// shader's binding_array's size.
TEST_F(BindGroupLayoutCompatibilityTest, ArraySizeCompatibility) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entry;
    wgpu::BindGroupLayout bglNoArray = device.CreateBindGroupLayout(&bglDesc);

    entry.bindingArraySize = 1;
    wgpu::BindGroupLayout bglArray1 = device.CreateBindGroupLayout(&bglDesc);

    entry.bindingArraySize = 2;
    wgpu::BindGroupLayout bglArray2 = device.CreateBindGroupLayout(&bglDesc);

    entry.bindingArraySize = 3;
    wgpu::BindGroupLayout bglArray3 = device.CreateBindGroupLayout(&bglDesc);

    // Test that a BGL with bindingArraySize 2 is valid for binding_array<T, 2>
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : binding_array<texture_2d<f32>, 2>;
            @fragment fn main() {
                _ = t[0];
            })",
                           {bglArray2});
    // Test that a BGL with a bigger bindingArraySize is valid.
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : binding_array<texture_2d<f32>, 2>;
            @fragment fn main() {
                _ = t[0];
            })",
                           {bglArray3});
    // Test that a BGL with a smaller bindingArraySize is an error.
    ASSERT_DEVICE_ERROR(CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : binding_array<texture_2d<f32>, 2>;
            @fragment fn main() {
                _ = t[0];
            })",
                                               {bglArray1}));

    // Test that an array of size 1 BGL is valid for a single binding.
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : texture_2d<f32>;
            @fragment fn main() {
                _ = t;
            })",
                           {bglArray1});
    // Test that an array of size 2 BGL is valid for a single binding at the start of the array.
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : texture_2d<f32>;
            @fragment fn main() {
                _ = t;
            })",
                           {bglArray2});
    // Test that the single binding cannot be an element from the arrayed BGL except the first
    ASSERT_DEVICE_ERROR(CreateFSRenderPipeline(R"(
            @group(0) @binding(1) var t : texture_2d<f32>;
            @fragment fn main() {
                _ = t;
            })",
                                               {bglArray2}));
    // Test that a binding_array<T, 1> bindings can be fulfilled by a non-arrayed BGL.
    CreateFSRenderPipeline(R"(
            @group(0) @binding(0) var t : binding_array<texture_2d<f32>, 1>;
            @fragment fn main() {
                _ = t[0];
            })",
                           {bglNoArray});
}

class BindingsValidationTest : public BindGroupLayoutCompatibilityTest {
  public:
    void SetUp() override {
        BindGroupLayoutCompatibilityTest::SetUp();
        mBufferSize = 3 * GetSupportedLimits().minUniformBufferOffsetAlignment + 8;
    }

    void TestRenderPassBindings(const wgpu::BindGroup* bg,
                                uint32_t count,
                                wgpu::RenderPipeline pipeline,
                                bool expectation) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        PlaceholderRenderPass PlaceholderRenderPass(device);
        wgpu::RenderPassEncoder rp = encoder.BeginRenderPass(&PlaceholderRenderPass);
        for (uint32_t i = 0; i < count; ++i) {
            rp.SetBindGroup(i, bg[i]);
        }
        rp.SetPipeline(pipeline);
        rp.Draw(3);
        rp.End();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        } else {
            encoder.Finish();
        }
    }

    void TestComputePassBindings(const wgpu::BindGroup* bg,
                                 uint32_t count,
                                 wgpu::ComputePipeline pipeline,
                                 bool expectation) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder cp = encoder.BeginComputePass();
        for (uint32_t i = 0; i < count; ++i) {
            cp.SetBindGroup(i, bg[i]);
        }
        cp.SetPipeline(pipeline);
        cp.DispatchWorkgroups(1);
        cp.End();
        if (!expectation) {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        } else {
            encoder.Finish();
        }
    }

    uint32_t mBufferSize;
    static constexpr uint32_t kBindingNum = 3;
};

// Test that it is valid to set a pipeline layout with bindings unused by the pipeline.
TEST_F(BindingsValidationTest, PipelineLayoutWithMoreBindingsThanPipeline) {
    // Set up bind group layouts.
    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage},
                 {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::ReadOnlyStorage}});
    wgpu::BindGroupLayout bgl2 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});

    // pipelineLayout has unused binding set (bgl2) and unused entry in a binding set (bgl0).
    CreateRenderPipeline({bgl0, bgl1, bgl2});

    CreateComputePipeline({bgl0, bgl1, bgl2});
}

// Test that it is invalid to set a pipeline layout that doesn't have all necessary bindings
// required by the pipeline.
TEST_F(BindingsValidationTest, PipelineLayoutWithFewerBindingsThanPipeline) {
    // Set up bind group layout.
    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Storage}});

    // missing a binding set (bgl1) in pipeline layout
    {
        ASSERT_DEVICE_ERROR(CreateRenderPipeline({bgl0}));

        ASSERT_DEVICE_ERROR(CreateComputePipeline({bgl0}));
    }

    // bgl1 is not missing, but it is empty
    {
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(device, {});

        ASSERT_DEVICE_ERROR(CreateRenderPipeline({bgl0, bgl1}));

        ASSERT_DEVICE_ERROR(CreateComputePipeline({bgl0, bgl1}));
    }

    // bgl1 is neither missing nor empty, but it doesn't contain the necessary binding
    {
        wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
            device, {{1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      wgpu::BufferBindingType::Uniform}});

        ASSERT_DEVICE_ERROR(CreateRenderPipeline({bgl0, bgl1}));

        ASSERT_DEVICE_ERROR(CreateComputePipeline({bgl0, bgl1}));
    }
}

// Test that it is valid to set bind groups whose layout is not set in the pipeline layout.
// But it's invalid to set extra entry for a given bind group's layout if that layout is set in
// the pipeline layout.
TEST_F(BindingsValidationTest, BindGroupsWithMoreBindingsThanPipelineLayout) {
    // Set up bind group layouts, buffers, bind groups, pipeline layouts and pipelines.
    std::array<wgpu::BindGroupLayout, kBindingNum + 1> bgl;
    std::array<wgpu::BindGroup, kBindingNum + 1> bg;
    std::array<wgpu::Buffer, kBindingNum + 1> buffer;
    for (uint32_t i = 0; i < kBindingNum + 1; ++i) {
        bgl[i] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      i == 1 ? wgpu::BufferBindingType::ReadOnlyStorage
                             : wgpu::BufferBindingType::Storage}});
        buffer[i] = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
        bg[i] = utils::MakeBindGroup(device, bgl[i], {{0, buffer[i]}});
    }

    // Set 3 bindings (and 3 pipeline layouts) in pipeline.
    wgpu::RenderPipeline renderPipeline = CreateRenderPipeline({bgl[0], bgl[1], bgl[2]});
    wgpu::ComputePipeline computePipeline = CreateComputePipeline({bgl[0], bgl[1], bgl[2]});

    // Comprared to pipeline layout, there is an extra bind group (bg[3])
    TestRenderPassBindings(bg.data(), kBindingNum + 1, renderPipeline, true);

    TestComputePassBindings(bg.data(), kBindingNum + 1, computePipeline, true);

    // If a bind group has entry (like bgl1_1 below) unused by the pipeline layout, it is invalid.
    // Bind groups associated layout should exactly match bind group layout if that layout is
    // set in pipeline layout.
    bgl[1] = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::ReadOnlyStorage},
                 {1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Uniform}});
    buffer[1] = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::Uniform);
    bg[1] = utils::MakeBindGroup(device, bgl[1], {{0, buffer[1]}, {1, buffer[1]}});

    TestRenderPassBindings(bg.data(), kBindingNum, renderPipeline, false);

    TestComputePassBindings(bg.data(), kBindingNum, computePipeline, false);
}

// Test that it is invalid to set bind groups that don't have all necessary bindings required
// by the pipeline layout. Note that both pipeline layout and bind group have enough bindings for
// pipeline in the following test.
TEST_F(BindingsValidationTest, BindGroupsWithFewerBindingsThanPipelineLayout) {
    // Set up bind group layouts, buffers, bind groups, pipeline layouts and pipelines.
    std::array<wgpu::BindGroupLayout, kBindingNum> bgl;
    std::array<wgpu::BindGroup, kBindingNum> bg;
    std::array<wgpu::Buffer, kBindingNum> buffer;
    for (uint32_t i = 0; i < kBindingNum; ++i) {
        bgl[i] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                      i == 1 ? wgpu::BufferBindingType::ReadOnlyStorage
                             : wgpu::BufferBindingType::Storage}});
        buffer[i] = CreateBuffer(mBufferSize, wgpu::BufferUsage::Storage);
        bg[i] = utils::MakeBindGroup(device, bgl[i], {{0, buffer[i]}});
    }

    wgpu::RenderPipeline renderPipeline = CreateRenderPipeline({bgl[0], bgl[1], bgl[2]});
    wgpu::ComputePipeline computePipeline = CreateComputePipeline({bgl[0], bgl[1], bgl[2]});

    // Compared to pipeline layout, a binding set (bgl2) related bind group is missing
    TestRenderPassBindings(bg.data(), kBindingNum - 1, renderPipeline, false);

    TestComputePassBindings(bg.data(), kBindingNum - 1, computePipeline, false);

    // bgl[2] related bind group is not missing, but its bind group is empty
    bgl[2] = utils::MakeBindGroupLayout(device, {});
    bg[2] = utils::MakeBindGroup(device, bgl[2], {});

    TestRenderPassBindings(bg.data(), kBindingNum, renderPipeline, false);

    TestComputePassBindings(bg.data(), kBindingNum, computePipeline, false);

    // bgl[2] related bind group is neither missing nor empty, but it doesn't contain the necessary
    // binding
    bgl[2] = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment,
                  wgpu::BufferBindingType::Uniform}});
    buffer[2] = CreateBuffer(mBufferSize, wgpu::BufferUsage::Uniform);
    bg[2] = utils::MakeBindGroup(device, bgl[2], {{1, buffer[2]}});

    TestRenderPassBindings(bg.data(), kBindingNum, renderPipeline, false);

    TestComputePassBindings(bg.data(), kBindingNum, computePipeline, false);
}

class SamplerTypeBindingTest : public ValidationTest {
  protected:
    wgpu::RenderPipeline CreateFragmentPipeline(wgpu::BindGroupLayout* bindGroupLayout,
                                                const char* fragmentSource) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f();
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentSource);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        wgpu::PipelineLayout pipelineLayout =
            utils::MakeBasicPipelineLayout(device, bindGroupLayout);
        pipelineDescriptor.layout = pipelineLayout;
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }
};

// Test that the use of sampler and comparison_sampler in the shader must match the bind group
// layout.
TEST_F(SamplerTypeBindingTest, ShaderAndBGLMatches) {
    // Test that a filtering sampler binding works with normal sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @fragment fn main() {
                _ = mySampler;
            })");
    }

    // Test that a non-filtering sampler binding works with normal sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @fragment fn main() {
                _ = mySampler;
            })");
    }

    // Test that comparison sampler binding works with comparison sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler_comparison;
            @fragment fn main() {
                _ = mySampler;
            })");
    }

    // Test that filtering sampler binding does not work with comparison sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler_comparison;
            @fragment fn main() {
                _ = mySampler;
            })"));
    }

    // Test that non-filtering sampler binding does not work with comparison sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler_comparison;
            @fragment fn main() {
                _ = mySampler;
            })"));
    }

    // Test that comparison sampler binding does not work with normal sampler in the shader.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @fragment fn main() {
                _ = mySampler;
            })"));
    }

    // Test that a filtering sampler can be used to sample a float texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<f32>;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }

    // Test that a non-filtering sampler can be used to sample a float texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<f32>;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }

    // Test that a filtering sampler can not be used to sample a depth texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_depth_2d;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })"));
    }

    // Test that a non-filtering sampler can be used to sample a depth texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_depth_2d;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }

    // Test that a comparison sampler can be used to sample a depth texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler_comparison;
            @group(0) @binding(1) var myTexture: texture_depth_2d;
            @fragment fn main() {
                _ = textureSampleCompare(myTexture, mySampler, vec2f(0.0, 0.0), 0.0);
            })");
    }

    // Test that a filtering sampler cannot be used to sample an unfilterable-float texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<f32>;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })"));
    }

    // Test that a non-filtering sampler can be used to sample an unfilterable-float texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<f32>;
            @fragment fn main() {
                _ = textureSample(myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }

    // Test that a filtering sampler can not be used to sample an sint texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Sint}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<i32>;
            @fragment fn main() {
                _ = textureGather(0, myTexture, mySampler, vec2f(0.0, 0.0));
            })"));
    }

    // Test that a non-filtering sampler can be used to sample an sint texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Sint}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<i32>;
            @fragment fn main() {
                _ = textureGather(0, myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }

    // Test that a filtering sampler can not be used to sample an uint texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});

        ASSERT_DEVICE_ERROR(CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<u32>;
            @fragment fn main() {
                _ = textureGather(0, myTexture, mySampler, vec2f(0.0, 0.0));
            })"));
    }

    // Test that a non-filtering sampler can be used to sample an uint texture.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                     {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});

        CreateFragmentPipeline(&bindGroupLayout, R"(
            @group(0) @binding(0) var mySampler: sampler;
            @group(0) @binding(1) var myTexture: texture_2d<u32>;
            @fragment fn main() {
                _ = textureGather(0, myTexture, mySampler, vec2f(0.0, 0.0));
            })");
    }
}

TEST_F(SamplerTypeBindingTest, SamplerAndBindGroupMatches) {
    // Test that sampler binding works with normal sampler.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

        utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler()}});
    }

    // Test that comparison sampler binding works with sampler w/ compare function.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison}});

        wgpu::SamplerDescriptor desc = {};
        desc.compare = wgpu::CompareFunction::Never;
        utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}});
    }

    // Test that sampler binding does not work with sampler w/ compare function.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

        wgpu::SamplerDescriptor desc;
        desc.compare = wgpu::CompareFunction::Never;
        ASSERT_DEVICE_ERROR(
            utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}}));
    }

    // Test that comparison sampler binding does not work with normal sampler.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Comparison}});

        wgpu::SamplerDescriptor desc = {};
        ASSERT_DEVICE_ERROR(
            utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}}));
    }

    // Test that filtering sampler binding works with a filtering or non-filtering sampler.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

        // Test each filter member
        {
            wgpu::SamplerDescriptor desc;
            desc.minFilter = wgpu::FilterMode::Linear;
            utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}});
        }
        {
            wgpu::SamplerDescriptor desc;
            desc.magFilter = wgpu::FilterMode::Linear;
            utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}});
        }
        {
            wgpu::SamplerDescriptor desc;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}});
        }

        // Test non-filtering sampler
        utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler()}});
    }

    // Test that non-filtering sampler binding does not work with a filtering sampler.
    {
        wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering}});

        // Test each filter member
        {
            wgpu::SamplerDescriptor desc;
            desc.minFilter = wgpu::FilterMode::Linear;
            ASSERT_DEVICE_ERROR(
                utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}}));
        }
        {
            wgpu::SamplerDescriptor desc;
            desc.magFilter = wgpu::FilterMode::Linear;
            ASSERT_DEVICE_ERROR(
                utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}}));
        }
        {
            wgpu::SamplerDescriptor desc;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            ASSERT_DEVICE_ERROR(
                utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler(&desc)}}));
        }

        // Test non-filtering sampler
        utils::MakeBindGroup(device, bindGroupLayout, {{0, device.CreateSampler()}});
    }
}

class PipelineLayoutValidationTest : public ValidationTest {};

// Test creating pipeline layout with null bind group layout works when unsafe APIs are allowed.
TEST_F(PipelineLayoutValidationTest, CreateWithNullBindGroupLayout) {
    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        std::vector<wgpu::BindGroupLayout> bgls(4);
        for (uint32_t i = 0; i < 4; ++i) {
            if (i == nullBGLIndex) {
                continue;
            }
            bgls[i] = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly,
                          wgpu::TextureFormat::R32Float}});
        }
        utils::MakePipelineLayout(device, bgls);
    }
}

// Test the pipeline layout with null bind group layout must match the corresponding binding in
// shader.
TEST_F(PipelineLayoutValidationTest, ShaderMatchesPipelineLayoutWithNullBindGroupLayout) {
    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        std::vector<wgpu::BindGroupLayout> bgls(4);
        for (uint32_t i = 0; i < 4; ++i) {
            if (i == nullBGLIndex) {
                continue;
            }
            bgls[i] = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        }
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, bgls);

        for (uint32_t missedGroupIndex = 0; missedGroupIndex < 4; ++missedGroupIndex) {
            std::ostringstream stream;
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != missedGroupIndex) {
                    stream << "@group(" << i << ") @binding(0) var<storage, read_write> outputData"
                           << i << " : u32;\n";
                }
            }
            stream << "@compute @workgroup_size(1, 1) fn main() {\n";
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != missedGroupIndex) {
                    stream << "outputData" << i << " = 1u;\n";
                }
            }
            stream << "};";
            wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
            computePipelineDescriptor.compute.module =
                utils::CreateShaderModule(device, stream.str());
            computePipelineDescriptor.layout = pipelineLayout;
            if (missedGroupIndex == nullBGLIndex) {
                device.CreateComputePipeline(&computePipelineDescriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&computePipelineDescriptor));
            }
        }
    }
}

// Test the null or empty bind group layout in a pipeline layout should be ignored when we check the
// compatibility between the pipeline layout and the corresponding bind group.
TEST_F(PipelineLayoutValidationTest, BindGroupSlotWithEmptyLayoutIsNotValidated) {
    std::array<wgpu::BindGroupLayout, 2> nullOrEmptyBindGroupLayouts = {
        nullptr, utils::MakeBindGroupLayout(device, {})};

    wgpu::BindGroupLayout nonEmptyBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
    wgpu::BufferDescriptor bufferDescForNonEmptyBGL = {};
    bufferDescForNonEmptyBGL.size = 4;
    bufferDescForNonEmptyBGL.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer bufferForNonEmptyBGL = device.CreateBuffer(&bufferDescForNonEmptyBGL);
    wgpu::BindGroup nonEmptyBindGroup =
        utils::MakeBindGroup(device, nonEmptyBGL, {{0, bufferForNonEmptyBGL}});

    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        for (wgpu::BindGroupLayout nullOrEmptyBindGroupLayout : nullOrEmptyBindGroupLayouts) {
            std::vector<wgpu::BindGroupLayout> bgls(4);
            std::vector<wgpu::BindGroup> bgs(4);

            // Create compute pipeline with null or empty bind group layout and the bind groups.
            // Note that the bind groups are all non-empty.
            for (uint32_t i = 0; i < 4; ++i) {
                if (i == nullBGLIndex) {
                    bgls[i] = nullOrEmptyBindGroupLayout;
                    bgs[i] = nonEmptyBindGroup;
                } else {
                    bgls[i] = utils::MakeBindGroupLayout(
                        device,
                        {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

                    wgpu::BufferDescriptor bufferDesc = {};
                    bufferDesc.size = 4;
                    bufferDesc.usage = wgpu::BufferUsage::Storage;
                    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
                    bgs[i] = utils::MakeBindGroup(device, bgls[i], {{0, buffer}});
                }
            }
            wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, bgls);

            std::ostringstream stream;
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != nullBGLIndex) {
                    stream << "@group(" << i << ") @binding(0) var<storage, read_write> outputData"
                           << i << " : u32;\n";
                }
            }
            stream << "@compute @workgroup_size(1, 1) fn main() {\n";
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != nullBGLIndex) {
                    stream << "outputData" << i << " = 1u;\n";
                }
            }
            stream << "};";
            wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
            computePipelineDescriptor.compute.module =
                utils::CreateShaderModule(device, stream.str());
            computePipelineDescriptor.layout = pipelineLayout;

            wgpu::ComputePipeline computePipeline =
                device.CreateComputePipeline(&computePipelineDescriptor);

            // Set pipeline and bind groups. The null or empty bind group layout in the pipeline
            // layout should be ignored in the check of the compatibility between pipeline layout
            // and the bind group when encoding `SetBindGroup()`.
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
            for (uint32_t i = 0; i < 4; ++i) {
                computePass.SetBindGroup(i, bgs[i]);
            }
            computePass.SetPipeline(computePipeline);
            computePass.DispatchWorkgroups(1);
            computePass.End();
            wgpu::CommandBuffer cmdbuf = encoder.Finish();
            device.GetQueue().Submit(1, &cmdbuf);
        }
    }
}

// Test the empty bind group layout returned by calling `getBindGroupLayout()` on a pipeline created
// with `auto` pipeline layout cannot be used to create other pipeline layouts.
TEST_F(PipelineLayoutValidationTest, ReuseEmptyBindGroupLayoutCreatedwithAutoPipelineLayout) {
    // The empty bind group layout comes from a pipeline created with an explicit pipeline layout.
    {
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {});
        wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
                @compute @workgroup_size(1, 1) fn main() {})");
        computePipelineDescriptor.layout = pipelineLayout;
        wgpu::ComputePipeline computePipeline =
            device.CreateComputePipeline(&computePipelineDescriptor);

        wgpu::BindGroupLayout emptyBindGroupLayout = computePipeline.GetBindGroupLayout(3);
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {{emptyBindGroupLayout}};
        utils::MakePipelineLayout(device, bindGroupLayouts);
    }

    // The empty bind group layout comes from a pipeline created with an 'auto' pipeline layout.
    {
        wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
                @compute @workgroup_size(1, 1) fn main() {})");
        wgpu::ComputePipeline computePipeline =
            device.CreateComputePipeline(&computePipelineDescriptor);

        wgpu::BindGroupLayout emptyBindGroupLayout = computePipeline.GetBindGroupLayout(3);
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {{emptyBindGroupLayout}};
        ASSERT_DEVICE_ERROR(utils::MakePipelineLayout(device, bindGroupLayouts));
    }
}

}  // anonymous namespace
}  // namespace dawn
