// Copyright 2021 The Dawn & Tint Authors
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
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ExternalTextureTest : public ValidationTest {
  public:
    wgpu::TextureDescriptor CreateTextureDescriptor(
        wgpu::TextureFormat format = kDefaultTextureFormat) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = kDefaultDepth;
        descriptor.mipLevelCount = kDefaultMipLevels;
        descriptor.sampleCount = kDefaultSampleCount;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = format;
        descriptor.usage = kDefaultUsage;
        return descriptor;
    }

    wgpu::ExternalTexture CreateDefaultExternalTexture() {
        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = defaultTexture.CreateView();
        return device.CreateExternalTexture(&externalDesc);
    }

    void SubmitExternalTextureInDefaultRenderPass(wgpu::ExternalTexture externalTexture,
                                                  bool success) {
        // Create a bind group that contains the external texture.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

        // Create another texture to use as a color attachment.
        wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
        wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
        wgpu::TextureView renderView = renderTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetBindGroup(0, bindGroup);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();

        if (success) {
            queue.Submit(1, &commands);
        } else {
            ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        }
    }

    void SubmitExternalTextureInDefaultRenderBundle(wgpu::ExternalTexture externalTexture,
                                                    bool success) {
        // Create a bind group that contains the external texture.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

        // Create another texture to use as a color attachment.
        wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
        wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
        wgpu::TextureView renderView = renderTexture.CreateView();

        // Create a RenderBundle using the bindgroup and doing nothing else.
        wgpu::RenderBundleEncoderDescriptor rbDesc;
        rbDesc.colorFormatCount = 1;
        rbDesc.colorFormats = &renderTextureDescriptor.format;

        wgpu::RenderBundleEncoder rbEncoder = device.CreateRenderBundleEncoder(&rbDesc);
        rbEncoder.SetBindGroup(0, bindGroup);
        wgpu::RenderBundle bundle = rbEncoder.Finish();

        // Use that render bundle in the render pass.
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &bundle);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();

        if (success) {
            queue.Submit(1, &commands);
        } else {
            ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        }
    }

    void SubmitExternalTextureInDefaultComputePass(wgpu::ExternalTexture externalTexture,
                                                   bool success) {
        // Create a bind group that contains the external texture.
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

        wgpu::ComputePassDescriptor computePass;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePass);
        pass.SetBindGroup(0, bindGroup);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();

        if (success) {
            queue.Submit(1, &commands);
        } else {
            ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        }
    }

  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        queue = device.GetQueue();

        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        defaultTexture = device.CreateTexture(&textureDescriptor);
    }

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

    static constexpr uint32_t kWidth = 32;
    static constexpr uint32_t kHeight = 32;
    static constexpr uint32_t kDefaultDepth = 1;
    static constexpr uint32_t kDefaultMipLevels = 1;
    static constexpr uint32_t kDefaultSampleCount = 1;
    static constexpr wgpu::TextureUsage kDefaultUsage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    static constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::RGBA8Unorm;
    static constexpr wgpu::TextureFormat kBiplanarPlane0Format = wgpu::TextureFormat::R8Unorm;
    static constexpr wgpu::TextureFormat kBiplanarPlane1Format = wgpu::TextureFormat::RG8Unorm;

    std::array<float, 12> mPlaceholderConstantArray;

    wgpu::Queue queue;
    wgpu::Texture defaultTexture;
};

TEST_F(ExternalTextureTest, CreateExternalTextureValidation) {
    // Creating an external texture from a 2D, single-subresource texture should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        device.CreateExternalTexture(&externalDesc);
    }

    // Creating an external texture from a non-2D texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.dimension = wgpu::TextureDimension::e3D;
        textureDescriptor.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture from a texture with mip count > 1 should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.mipLevelCount = 2;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture from a texture without TextureUsage::TextureBinding should
    // fail.
    {
        wgpu::TextureDescriptor textureDescriptor = {};
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        textureDescriptor.size = {kWidth, kHeight, kDefaultDepth};
        textureDescriptor.format = kDefaultTextureFormat;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture with a non 4-component format should fail.
    {
        for (const auto& format : {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm}) {
            wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
            textureDescriptor.format = format;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = internalTexture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

    // Creating an external texture with a non float-filterable format should fail.
    {
        for (const auto& format : {wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Sint,
                                   wgpu::TextureFormat::RGBA32Uint, wgpu::TextureFormat::RGBA32Sint,
                                   wgpu::TextureFormat::RGBA32Float}) {
            wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
            textureDescriptor.format = format;
            wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = internalTexture.CreateView();
            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

    // Creating an external texture with an multisampled texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.sampleCount = 4;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture with an error texture view should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureViewDescriptor errorViewDescriptor;
        errorViewDescriptor.format = kDefaultTextureFormat;
        errorViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        errorViewDescriptor.mipLevelCount = 1;
        errorViewDescriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(wgpu::TextureView errorTextureView =
                                internalTexture.CreateView(&errorViewDescriptor));

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = errorTextureView;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

TEST_F(ExternalTextureTest, CreateExternalTextureConstantValueValidation) {
    DAWN_SKIP_TEST_IF(UsesWire());
    // Creating a single plane external texture without a YUV-to-RGB matrix should pass.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.yuvToRgbConversionMatrix = nullptr;
        device.CreateExternalTexture(&externalDesc);
    }

    // Creating a multiplanar external texture without a YUV-to-RGB matrix should fail.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0Format);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1Format);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();
        externalDesc.yuvToRgbConversionMatrix = nullptr;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture without a gamut conversion matrix should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.gamutConversionMatrix = nullptr;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture without source transfer function constants should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.srcTransferFunctionParameters = nullptr;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture without destination transfer function constants should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.dstTransferFunctionParameters = nullptr;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

// Test that external texture creation works as expected in multiplane scenarios.
TEST_F(ExternalTextureTest, CreateMultiplanarExternalTextureValidation) {
    // Creating an external texture from two 2D, single-subresource textures with a biplanar
    // format should succeed.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0Format);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1Format);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();

        device.CreateExternalTexture(&externalDesc);
    }

    // Creating a multiplanar external texture with an 1-component float-filterable format for
    // plane0 should succeed.
    {
        for (const auto& format : {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R16Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::TextureDescriptor plane1TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::RG8Unorm);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            device.CreateExternalTexture(&externalDesc);
        }
    }

    // Creating a multiplanar external texture with a 2-component float-filterable format for
    // plane1 should succeed.
    {
        for (const auto& format : {wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RG16Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::R8Unorm);
            wgpu::TextureDescriptor plane1TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            device.CreateExternalTexture(&externalDesc);
        }
    }

    // Creating a multiplanar external texture with an 1-component non float-filterable format for
    // plane0 should fail.
    {
        for (const auto& format : {wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::R8Sint,
                                   wgpu::TextureFormat::R16Uint, wgpu::TextureFormat::R16Sint,
                                   wgpu::TextureFormat::R32Uint, wgpu::TextureFormat::R32Sint,
                                   wgpu::TextureFormat::R32Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::TextureDescriptor plane1TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::RG8Unorm);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

    // Creating a multiplanar external texture with a 2-component non float-filterable format for
    // plane1 should fail.
    {
        for (const auto& format : {wgpu::TextureFormat::RG8Uint, wgpu::TextureFormat::RG8Sint,
                                   wgpu::TextureFormat::RG16Uint, wgpu::TextureFormat::RG16Sint,
                                   wgpu::TextureFormat::RG32Uint, wgpu::TextureFormat::RG32Sint,
                                   wgpu::TextureFormat::RG32Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::R8Unorm);
            wgpu::TextureDescriptor plane1TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

    // Creating a multiplanar external texture with a non 1-component format for
    // plane0 should fail.
    {
        for (const auto& format :
             {wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RGBA8Unorm,
              wgpu::TextureFormat::RG8Uint, wgpu::TextureFormat::RGBA8Uint,
              wgpu::TextureFormat::RG8Sint, wgpu::TextureFormat::RGBA8Sint,
              wgpu::TextureFormat::RG32Float, wgpu::TextureFormat::RGBA32Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::TextureDescriptor plane1TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::RG8Unorm);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }

    // Creating a multiplanar external texture with a non 2-component format for
    // plane1 should fail.
    {
        for (const auto& format :
             {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RGBA8Unorm,
              wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::RGBA8Uint,
              wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA32Float}) {
            wgpu::TextureDescriptor plane0TextureDescriptor =
                CreateTextureDescriptor(wgpu::TextureFormat::R8Unorm);
            wgpu::TextureDescriptor plane1TextureDescriptor = CreateTextureDescriptor(format);
            wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
            wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

            wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
            externalDesc.plane0 = texture0.CreateView();
            externalDesc.plane1 = texture1.CreateView();

            ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
        }
    }
}

// Test that refresh on an expired/active external texture.
TEST_F(ExternalTextureTest, RefreshExternalTexture) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();

    externalTexture.Refresh();
    externalTexture.Expire();
    externalTexture.Refresh();
}

// Test that refresh on a destroyed external texture results in an error.
TEST_F(ExternalTextureTest, RefreshDestroyedExternalTexture) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();

    // Refresh on destroyed external texture should result in an error.
    externalTexture.Destroy();
    ASSERT_DEVICE_ERROR(externalTexture.Refresh());
}

// Test that expire on a destroyed external texture results in an error.
TEST_F(ExternalTextureTest, ExpireDestroyedExternalTexture) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();

    externalTexture.Destroy();
    ASSERT_DEVICE_ERROR(externalTexture.Expire());
}

// Test that submitting a render pass that contains an active external texture.
TEST_F(ExternalTextureTest, SubmitActiveExternalTextureInRenderPass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    SubmitExternalTextureInDefaultRenderPass(externalTexture, true /* success = true */);
}

// Test that submitting a render pass that contains an expired external texture results in an error.
TEST_F(ExternalTextureTest, SubmitExpiredExternalTextureInRenderPass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Expire();
    SubmitExternalTextureInDefaultRenderPass(externalTexture, false /* success = false */);
}

// Test that submitting a render pass that contains an destroyed external texture results in an
// error.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTextureInRenderPass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Destroy();
    SubmitExternalTextureInDefaultRenderPass(externalTexture, false /* success = false */);
}

// Test that submitting a compute pass that contains an active external.
TEST_F(ExternalTextureTest, SubmitActiveExternalTextureInComputePass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    SubmitExternalTextureInDefaultComputePass(externalTexture, true /* success = true */);
}

// Test that submitting a compute pass that contains an expired external texture results in an
// error.
TEST_F(ExternalTextureTest, SubmitExpiredExternalTextureInComputePass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Expire();
    SubmitExternalTextureInDefaultComputePass(externalTexture, false /* success = false */);
}

// Test that submitting a compute pass that contains a destroyed external texture should success.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTextureInComputePass) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Destroy();
    SubmitExternalTextureInDefaultComputePass(externalTexture, false /* success = false */);
}

// Test that submitting a render bundle that contains an active external.
TEST_F(ExternalTextureTest, SubmitActiveExternalTextureInRenderBundle) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    SubmitExternalTextureInDefaultRenderBundle(externalTexture, true /* success = true */);
}

// Test that submitting a render bundle that contains an expired external texture results in an
// error.
TEST_F(ExternalTextureTest, SubmitExpiredExternalTextureInRenderBundle) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Expire();
    SubmitExternalTextureInDefaultRenderBundle(externalTexture, false /* success = false */);
}

// Test that submitting a render bundle that contains a destroyed external texture should success.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTextureInRenderBundle) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Destroy();
    SubmitExternalTextureInDefaultRenderBundle(externalTexture, false /* success = false */);
}

// Test that refresh an expired external texture and submit a compute pass with it.
TEST_F(ExternalTextureTest, RefreshExpiredExternalTexture) {
    wgpu::ExternalTexture externalTexture = CreateDefaultExternalTexture();
    externalTexture.Expire();

    // Submit with expired external texture results in error
    SubmitExternalTextureInDefaultComputePass(externalTexture, false /* success = false */);

    externalTexture.Refresh();
    // Refreshed external texture could be submitted.
    SubmitExternalTextureInDefaultComputePass(externalTexture, true /* success = true */);
}

// Test that submitting a render pass that contains a dereferenced external texture results in
// success
TEST_F(ExternalTextureTest, SubmitDereferencedExternalTextureInRenderPass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Dereferencing the external texture should not result in a use-after-free error.
    {
        externalTexture = nullptr;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
}

// Ensure that bind group validation catches mismatched entries for BGL external texture entries.
TEST_F(ExternalTextureTest, BindGroupDoesNotMatchLayout) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
    wgpu::TextureView textureView = texture.CreateView();

    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = textureView;
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Control case for external texture should succeed.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        utils::MakeBindGroup(device, bgl, {{0, externalTexture}});
    }

    // Control case for texture view should succeed.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        utils::MakeBindGroup(device, bgl, {{0, textureView}});
    }

    // Bind group creation should fail when an external texture is not present in the
    // corresponding slot of the bind group layout.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, externalTexture}}));
    }

    // Bind group creation should fail when a sampler is present in the
    // corresponding slot of the bind group layout.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, device.CreateSampler()}}));
    }
}

// Ensure that bind group validation catches error external textures.
TEST_F(ExternalTextureTest, UseErrorExternalTextureInBindGroup) {
    // Control case should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        utils::MakeBindGroup(device, bgl, {{0, externalTexture}});
    }

    // Bind group creation should fail when an error external texture is present.
    {
        wgpu::ExternalTexture errorExternalTexture = device.CreateErrorExternalTexture();
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, errorExternalTexture}}));
    }
}

// Test create external texture with too large crop rect results in error.
TEST_F(ExternalTextureTest, CreateExternalTextureWithErrorCropOriginOrSize) {
    // Control case should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {texture.GetWidth(), texture.GetHeight()};
        device.CreateExternalTexture(&externalDesc);
    }

    // cropOrigin is OOB on x
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {1, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {texture.GetWidth(), texture.GetHeight()};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // cropOrigin is OOB on y
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 1};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {texture.GetWidth(), texture.GetHeight()};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // cropSize is OOB on width
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth() + 1, texture.GetHeight()};
        externalDesc.apparentSize = {texture.GetWidth(), texture.GetHeight()};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // cropSize is OOB on height
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight() + 1};
        externalDesc.apparentSize = {texture.GetWidth(), texture.GetHeight()};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

// Test create external texture with too large apparent size results in error.
TEST_F(ExternalTextureTest, CreateExternalTextureWithApparentSizeTooLarge) {
    wgpu::Limits limits;
    device.GetLimits(&limits);

    // Control case should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {limits.maxTextureDimension2D, limits.maxTextureDimension2D};
        device.CreateExternalTexture(&externalDesc);
    }

    // Error: apparentSize.width is too large
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {limits.maxTextureDimension2D + 1,
                                     limits.maxTextureDimension2D};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Error: apparentSize.height is too large
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture.CreateView();
        externalDesc.cropOrigin = {0, 0};
        externalDesc.cropSize = {texture.GetWidth(), texture.GetHeight()};
        externalDesc.apparentSize = {limits.maxTextureDimension2D,
                                     limits.maxTextureDimension2D + 1};
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

// Test that submitting an external texture with a plane that is not submittable results in error.
TEST_F(ExternalTextureTest, SubmitExternalTextureWithDestroyedPlane) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Destroying the plane0 backed texture should result in an error.
    {
        texture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Ensure that bind group validation catches invalid texture views.
TEST_F(ExternalTextureTest, TextureViewValidation) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});

    // A texture view from a 2D, single-subresource texture should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureView textureView = texture.CreateView();
        utils::MakeBindGroup(device, bgl, {{0, textureView}});
    }

    // A texture view from a non-2D texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.dimension = wgpu::TextureDimension::e3D;
        textureDescriptor.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureView textureView = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, textureView}}));
    }

    // A texture view from a texture with mip count > 1 should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.mipLevelCount = 2;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureView textureView = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, textureView}}));
    }

    // A texture view from a texture without TextureUsage::TextureBinding should
    // fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureView textureView = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, textureView}}));
    }

    // A texture view from a multisampled texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.sampleCount = 4;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureView textureView = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, textureView}}));
    }
}

// Ensure that bind group validation catches invalid texture views for non supported context
// formats.
TEST_F(ExternalTextureTest, TextureViewValidationForNonSupportedContextFormats) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});

    for (wgpu::TextureFormat format : utils::kFormatsInCoreSpec) {
        if (!utils::IsRenderableFormat(device, format)) {
            continue;
        }
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor(format);
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);
        if (utils::IsSupportedContextFormat(format)) {
            utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}});
        } else {
            ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}}));
        }
    }
}

// Test that submitting a texture view with a destroyed texture results in error.
TEST_F(ExternalTextureTest, SubmitTextureViewWithDestroyedTexture) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::TextureView textureView = texture.CreateView();

    // Create a bind group that contains the texture view.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, textureView}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Destroying the texture should result in an error.
    {
        texture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

class ExternalTextureNorm16Test : public ExternalTextureTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Unorm16TextureFormats};
    }

    static constexpr wgpu::TextureFormat kBiplanarPlane0FormatNorm16 =
        wgpu::TextureFormat::R16Unorm;
    static constexpr wgpu::TextureFormat kBiplanarPlane1FormatNorm16 =
        wgpu::TextureFormat::RG16Unorm;
};

// Test that norm16 external texture creation works as expected in multiplane scenarios.
TEST_F(ExternalTextureNorm16Test, CreateMultiplanarExternalTextureValidation) {
    // Creating an external texture from two 2D, single-subresource textures with a biplanar
    // format should succeed.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0FormatNorm16);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1FormatNorm16);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();

        device.CreateExternalTexture(&externalDesc);
    }
}

class ExternalTextureUnorm16Test : public ExternalTextureTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Unorm16FormatsForExternalTexture,
                wgpu::FeatureName::MultiPlanarFormatP210,
                wgpu::FeatureName::MultiPlanarFormatExtendedUsages};
    }
};

// Test that with Unorm16FormatsForExternalTexture it is possible to create an ExternalTexture from
// a YUV HDR biplanar texture that has plane formats that are R/RG16Unorm. Normally this should not
// be allowed because the formats are not filterable-float, but the Unorm16FormatsForExternalTexture
// feature bypasses this check.
TEST_F(ExternalTextureUnorm16Test, CreateMultiPlanarTextureFromHDRYUV) {
    // Create the YUV HDR texture. Direct creation of the texture requires
    // MultiPlanarFormatExtendedUsages and the use of the YUV HDR format requires
    // MultiPlanarFormatP210.
    wgpu::TextureDescriptor yuvDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {kWidth, kHeight},
        .format = wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,
    };
    wgpu::Texture yuv = device.CreateTexture(&yuvDesc);

    // Create plane views with formats that are not normally allowed, but can be used only when
    // retrieving the planes.
    wgpu::TextureViewDescriptor plane0Desc = {
        .format = wgpu::TextureFormat::R16Unorm,
        .aspect = wgpu::TextureAspect::Plane0Only,
    };
    wgpu::TextureView plane0 = yuv.CreateView(&plane0Desc);

    wgpu::TextureViewDescriptor plane1Desc = {
        .format = wgpu::TextureFormat::RG16Unorm,
        .aspect = wgpu::TextureAspect::Plane1Only,
    };
    wgpu::TextureView plane1 = yuv.CreateView(&plane1Desc);

    // The ExternalTexture creation validation will allow these unfilterable-float formats to be
    // used.
    wgpu::ExternalTextureDescriptor externalDesc = CreateDefaultExternalTextureDescriptor();
    externalDesc.plane0 = plane0;
    externalDesc.plane1 = plane1;
    device.CreateExternalTexture(&externalDesc);
}

class ExternalTextureUnorm16Test_FeatureDisabled : public ExternalTextureTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::MultiPlanarFormatP210,
                wgpu::FeatureName::MultiPlanarFormatExtendedUsages};
    }
};

// Check that without Unorm16FormatsForExternalTexture it's not possible to create the views from an
// HDR YUV texture.
TEST_F(ExternalTextureUnorm16Test_FeatureDisabled, CannotCreateViewsFromHDRYUV) {
    // Create the YUV HDR texture. Direct creation of the texture requires
    // MultiPlanarFormatExtendedUsages and the use of the YUV HDR format requires
    // MultiPlanarFormatP210.
    wgpu::TextureDescriptor yuvDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {kWidth, kHeight},
        .format = wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,
    };
    wgpu::Texture yuv = device.CreateTexture(&yuvDesc);

    // Create plane views with formats that are not normally allowed, but can be used only when
    // retrieving the planes.
    wgpu::TextureViewDescriptor plane0Desc = {
        .format = wgpu::TextureFormat::R16Unorm,
        .aspect = wgpu::TextureAspect::Plane0Only,
    };
    ASSERT_DEVICE_ERROR(yuv.CreateView(&plane0Desc));

    wgpu::TextureViewDescriptor plane1Desc = {
        .format = wgpu::TextureFormat::RG16Unorm,
        .aspect = wgpu::TextureAspect::Plane1Only,
    };
    ASSERT_DEVICE_ERROR(yuv.CreateView(&plane1Desc));
}

}  // anonymous namespace
}  // namespace dawn
