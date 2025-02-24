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
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class InternalUsageValidationDisabledTest : public ValidationTest {};

// Test that using DawnTextureInternalUsageDescriptor is an error if DawnInternalUsages is not
// enabled
TEST_F(InternalUsageValidationDisabledTest, TextureDescriptorRequiresFeature) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    // Control case: Normal texture creation works
    device.CreateTexture(&textureDesc);

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;

    // Error with chained feature struct.
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));

    // Also does not work with various internal usages.
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));

    internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
}

// Test that using DawnEncoderInternalUsageDescriptor is an error if DawnInternalUsages is not
// enabled
TEST_F(InternalUsageValidationDisabledTest, CommandEncoderDescriptorRequiresFeature) {
    wgpu::CommandEncoderDescriptor encoderDesc = {};

    // Control case: Normal encoder creation works
    device.CreateCommandEncoder(&encoderDesc);

    wgpu::DawnEncoderInternalUsageDescriptor internalDesc = {};
    encoderDesc.nextInChain = &internalDesc;

    // Error with chained DawnEncoderInternalUsageDescriptor.
    ASSERT_DEVICE_ERROR(wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc));

    // Check that the encoder records that it is invalid, and not any other errors.
    encoder.InjectValidationError("injected error");
    ASSERT_DEVICE_ERROR(encoder.Finish(),
                        testing::HasSubstr("[Invalid CommandEncoder (unlabeled)]"));
}

class TextureInternalUsageValidationTest : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }
};

// Test that internal usages can be passed in a chained descriptor.
TEST_F(TextureInternalUsageValidationTest, Basic) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;

    // Internal usage: none
    device.CreateTexture(&textureDesc);

    // Internal usage is the same as the base usage.
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    device.CreateTexture(&textureDesc);

    // Internal usage adds to the base usage.
    internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
    device.CreateTexture(&textureDesc);
}

// Test that internal usages takes part in other validation that
// depends on the usage.
TEST_F(TextureInternalUsageValidationTest, UsageValidation) {
    {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.size = {1, 1};
        textureDesc.usage = wgpu::TextureUsage::CopySrc;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDesc.nextInChain = &internalDesc;

        // Internal usage adds an invalid usage.
        internalDesc.internalUsage = static_cast<wgpu::TextureUsage>(-1);
        ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
    }

    {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.size = {1, 1};
        textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.sampleCount = 4;

        // Control case: multisampled texture
        device.CreateTexture(&textureDesc);

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDesc.nextInChain = &internalDesc;

        // OK: internal usage adds nothing.
        device.CreateTexture(&textureDesc);

        // Internal usage adds storage usage which is invalid
        // with multisampling.
        internalDesc.internalUsage = wgpu::TextureUsage::StorageBinding;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
    }
}

// Test that internal usage does not add to the validated usage
// for command encoding
TEST_F(TextureInternalUsageValidationTest, DeprecatedCommandValidation) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    textureDesc.usage = wgpu::TextureUsage::CopyDst;
    wgpu::Texture dst = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    wgpu::Texture src = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::None;

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;

    wgpu::Texture srcInternal = device.CreateTexture(&textureDesc);

    // Control: src -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(src, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        encoder.Finish();
    }

    // Invalid: src internal -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcInternal, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that the internal usages aren't reflected with wgpu::Texture::GetUsage.
TEST_F(TextureInternalUsageValidationTest, InternalUsagesAreNotReflected) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    internalDesc.internalUsage = wgpu::TextureUsage::StorageBinding;

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.nextInChain = &internalDesc;

    wgpu::Texture texture = device.CreateTexture(&textureDesc);
    ASSERT_EQ(texture.GetUsage(), wgpu::TextureUsage::CopySrc);
}

// Test the validation of internal usages against command encoders with and without
// useInternalUsages.
TEST_F(TextureInternalUsageValidationTest, CommandValidation) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    textureDesc.usage = wgpu::TextureUsage::CopyDst;
    wgpu::Texture dst = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    wgpu::Texture src = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::None;

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;

    wgpu::Texture srcInternal = device.CreateTexture(&textureDesc);

    // Control: src -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(src, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        encoder.Finish();
    }

    // Invalid: src internal -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcInternal, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Invalid: src internal -> dst, with internal descriptor, but useInternalUsages set to false.
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcInternal, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoderDescriptor encoderDesc = {};
        wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc = {};
        encoderInternalDesc.useInternalUsages = false;
        encoderDesc.nextInChain = &encoderInternalDesc;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Control with internal copy: src -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(src, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoderDescriptor encoderDesc = {};
        wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc = {};
        encoderInternalDesc.useInternalUsages = true;
        encoderDesc.nextInChain = &encoderInternalDesc;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        encoder.Finish();
    }

    // Valid with internal copy: src internal -> dst
    {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcInternal, 0, {0, 0});
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoderDescriptor encoderDesc = {};
        wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc = {};
        encoderInternalDesc.useInternalUsages = true;
        encoderDesc.nextInChain = &encoderInternalDesc;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
        encoder.Finish();
    }
}

}  // anonymous namespace
}  // namespace dawn
