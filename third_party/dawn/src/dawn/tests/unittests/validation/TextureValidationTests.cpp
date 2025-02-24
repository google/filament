// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr wgpu::TextureFormat kNonRenderableColorFormats[] = {
    wgpu::TextureFormat::RG11B10Ufloat, wgpu::TextureFormat::RGB9E5Ufloat,
    wgpu::TextureFormat::R8Snorm,       wgpu::TextureFormat::RG8Snorm,
    wgpu::TextureFormat::RGBA8Snorm,
};

wgpu::TextureDimension kDimensions[] = {
    wgpu::TextureDimension::e1D,
    wgpu::TextureDimension::e3D,
};

class TextureValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        queue = device.GetQueue();
    }

    wgpu::TextureDescriptor CreateDefaultTextureDescriptor() {
        wgpu::TextureDescriptor descriptor;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = kDefaultDepth;
        descriptor.mipLevelCount = kDefaultMipLevels;
        descriptor.sampleCount = kDefaultSampleCount;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = kDefaultTextureFormat;
        descriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        return descriptor;
    }

    wgpu::Queue queue;

  private:
    // Choose the LCM of all current compressed texture format texel dimensions as the
    // dimensions of the default texture.
    static constexpr uint32_t kWidth = 120;
    static constexpr uint32_t kHeight = 120;
    static constexpr uint32_t kDefaultDepth = 1;
    static constexpr uint32_t kDefaultMipLevels = 1;
    static constexpr uint32_t kDefaultSampleCount = 1;

    static constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::RGBA8Unorm;
};

// Test the validation of non-zero texture usage
TEST_F(TextureValidationTest, UsageNonZero) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();

    // Descriptor with proper usage is allowed
    {
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;

        device.CreateTexture(&descriptor);
    }

    // It is an error to create a texture with zero usage
    {
        descriptor.usage = wgpu::TextureUsage::None;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test the validation of sample count
TEST_F(TextureValidationTest, SampleCount) {
    wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

    // sampleCount == 1 is allowed.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 1;

        device.CreateTexture(&descriptor);
    }

    // sampleCount == 4 is allowed.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;

        device.CreateTexture(&descriptor);
    }

    // It is an error to create a texture with an invalid sampleCount.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 3;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to create a multisampled texture with mipLevelCount > 1.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.mipLevelCount = 2;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to create a multisampled 1D or 3D texture.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;

        descriptor.size.height = 1;
        descriptor.dimension = wgpu::TextureDimension::e1D;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.dimension = wgpu::TextureDimension::e3D;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to create a multisample texture when the format cannot support
    // multisample.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;

        for (wgpu::TextureFormat format : utils::kFormatsInCoreSpec) {
            descriptor.format = format;
            if (utils::TextureFormatSupportsMultisampling(device, format, UseCompatibilityMode())) {
                device.CreateTexture(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
            }
        }
    }

    // Currently we do not support multisampled 2D textures with depth > 1.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.size.depthOrArrayLayers = 2;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to set TextureUsage::StorageBinding when sampleCount > 1.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.usage |= wgpu::TextureUsage::StorageBinding;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to create a texture without TextureUsage::RenderAttachment usage when
    // sampleCount > 1.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.usage = wgpu::TextureUsage::TextureBinding;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test the validation of the mip level count
TEST_F(TextureValidationTest, MipLevelCount) {
    wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();
    defaultDescriptor.usage = wgpu::TextureUsage::TextureBinding;

    // mipLevelCount == 1 is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 1;

        device.CreateTexture(&descriptor);
    }

    // mipLevelCount == 0 is an error
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 0;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Full mip chains are allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        // Mip level sizes: 32, 16, 8, 4, 2, 1
        descriptor.mipLevelCount = 6;

        device.CreateTexture(&descriptor);
    }

    // Test non-power-of-two width
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        // Mip level width: 31, 15, 7, 3, 1
        descriptor.size.width = 31;
        descriptor.size.height = 4;

        // Full mip chains on non-power-of-two width are allowed
        descriptor.mipLevelCount = 5;
        device.CreateTexture(&descriptor);

        // Too big mip chains on non-power-of-two width are disallowed
        descriptor.mipLevelCount = 6;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Test non-power-of-two height
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 4;
        // Mip level height: 31, 15, 7, 3, 1
        descriptor.size.height = 31;

        // Full mip chains on non-power-of-two height are allowed
        descriptor.mipLevelCount = 5;
        device.CreateTexture(&descriptor);

        // Too big mip chains on non-power-of-two height are disallowed
        descriptor.mipLevelCount = 6;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Undefined shift check if miplevel is bigger than the integer bit width.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 100;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Non square mip map halves the resolution until a 1x1 dimension
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 8;
        // Mip maps: 32 * 8, 16 * 4, 8 * 2, 4 * 1, 2 * 1, 1 * 1
        descriptor.mipLevelCount = 6;

        device.CreateTexture(&descriptor);
    }

    // Non square mip map for a 3D textures
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 8;
        descriptor.size.depthOrArrayLayers = 64;
        descriptor.dimension = wgpu::TextureDimension::e3D;
        // Non square mip map halves width, height and depth until a 1x1x1 dimension for a 3D
        // texture. So there are 7 mipmaps at most: 32 * 8 * 64, 16 * 4 * 32, 8 * 2 * 16,
        // 4 * 1 * 8, 2 * 1 * 4, 1 * 1 * 2, 1 * 1 * 1.
        descriptor.mipLevelCount = 7;
        device.CreateTexture(&descriptor);
    }

    // Non square mip map for 2D textures with depth > 1
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 8;
        descriptor.size.depthOrArrayLayers = 64;
        // Non square mip map halves width and height until a 1x1 dimension for a 2D texture,
        // even its depth > 1. So there are 6 mipmaps at most: 32 * 8, 16 * 4, 8 * 2, 4 * 1, 2 *
        // 1, 1 * 1.
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.mipLevelCount = 7;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        descriptor.mipLevelCount = 6;
        device.CreateTexture(&descriptor);
    }

    // Mip level equal to the maximum for a 2D texture is allowed
    {
        uint32_t maxTextureDimension2D = GetSupportedLimits().limits.maxTextureDimension2D;
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = maxTextureDimension2D;
        descriptor.size.height = maxTextureDimension2D;
        descriptor.mipLevelCount = Log2(maxTextureDimension2D) + 1u;

        device.CreateTexture(&descriptor);
    }

    // Mip level exceeding the maximum for a 2D texture not allowed
    {
        uint32_t maxTextureDimension2D = GetSupportedLimits().limits.maxTextureDimension2D;
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = maxTextureDimension2D;
        descriptor.size.height = maxTextureDimension2D;
        descriptor.mipLevelCount = Log2(maxTextureDimension2D) + 2u;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // 1D textures can only have a single mip level.
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.dimension = wgpu::TextureDimension::e1D;
        descriptor.size.width = 32;
        descriptor.size.height = 1;

        // Having a single mip level is allowed.
        descriptor.mipLevelCount = 1;
        device.CreateTexture(&descriptor);

        // Having more than 1 is an error.
        descriptor.mipLevelCount = 2;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test the validation of array layer count
TEST_F(TextureValidationTest, ArrayLayerCount) {
    wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();
    wgpu::Limits supportedLimits = GetSupportedLimits().limits;

    // Array layer count exceeding maxTextureArrayLayers is not allowed for 2D texture
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;

        descriptor.size.depthOrArrayLayers = supportedLimits.maxTextureArrayLayers + 1u;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Array layer count less than maxTextureArrayLayers is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.depthOrArrayLayers = supportedLimits.maxTextureArrayLayers >> 1;
        device.CreateTexture(&descriptor);
    }

    // Array layer count equal to maxTextureArrayLayers is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.depthOrArrayLayers = supportedLimits.maxTextureArrayLayers;
        device.CreateTexture(&descriptor);
    }
}

// Test the validation of 1D texture size
TEST_F(TextureValidationTest, 1DTextureSize) {
    wgpu::Limits supportedLimits = GetSupportedLimits().limits;

    wgpu::TextureDescriptor defaultDescriptor;
    defaultDescriptor.size = {4, 1, 1};
    defaultDescriptor.dimension = wgpu::TextureDimension::e1D;
    defaultDescriptor.usage = wgpu::TextureUsage::CopySrc;
    defaultDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;

    // Width must be in [1, kMaxTextureDimension1D]
    {
        wgpu::TextureDescriptor desc = defaultDescriptor;
        desc.size.width = 0;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
        desc.size.width = 1;
        device.CreateTexture(&desc);

        desc.size.width = supportedLimits.maxTextureDimension1D;
        device.CreateTexture(&desc);
        desc.size.width = supportedLimits.maxTextureDimension1D + 1u;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
    }

    // Height must be 1
    {
        wgpu::TextureDescriptor desc = defaultDescriptor;
        desc.size.height = 2;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));

        desc.size.height = 0;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
    }

    // DepthOrArrayLayers must be 1
    {
        wgpu::TextureDescriptor desc = defaultDescriptor;
        desc.size.depthOrArrayLayers = 2;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));

        desc.size.depthOrArrayLayers = 0;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
    }
}

// Test the validation of 2D texture size
TEST_F(TextureValidationTest, 2DTextureSize) {
    wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();
    wgpu::Limits supportedLimits = GetSupportedLimits().limits;

    // Out-of-bound texture dimension is not allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = supportedLimits.maxTextureDimension2D + 1u;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size.width = 1;
        descriptor.size.height = supportedLimits.maxTextureDimension2D + 1u;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Zero-sized texture is not allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size = {0, 1, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, 0, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, 1, 0};
        // 2D texture with depth=0 is not allowed
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Texture size less than max dimension is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = supportedLimits.maxTextureDimension2D >> 1;
        descriptor.size.height = supportedLimits.maxTextureDimension2D >> 1;
        device.CreateTexture(&descriptor);
    }

    // Texture size equal to max dimension is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = supportedLimits.maxTextureDimension2D;
        descriptor.size.height = supportedLimits.maxTextureDimension2D;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        device.CreateTexture(&descriptor);
    }
}

// Test the validation of 3D texture size
TEST_F(TextureValidationTest, 3DTextureSize) {
    wgpu::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();
    defaultDescriptor.dimension = wgpu::TextureDimension::e3D;
    defaultDescriptor.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Limits supportedLimits = GetSupportedLimits().limits;

    // Out-of-bound texture dimension is not allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;

        descriptor.size = {supportedLimits.maxTextureDimension3D + 1u, 1, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, supportedLimits.maxTextureDimension3D + 1u, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, 1, supportedLimits.maxTextureDimension3D + 1u};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Zero-sized texture is not allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;

        descriptor.size = {0, 1, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, 0, 1};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

        descriptor.size = {1, 1, 0};
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Texture size less than max dimension is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;

        descriptor.size = {supportedLimits.maxTextureDimension3D >> 1,
                           supportedLimits.maxTextureDimension3D >> 1,
                           supportedLimits.maxTextureDimension3D >> 1};
        device.CreateTexture(&descriptor);
    }

    // Texture size equal to max dimension is allowed
    {
        wgpu::TextureDescriptor descriptor = defaultDescriptor;

        descriptor.size = {supportedLimits.maxTextureDimension3D,
                           supportedLimits.maxTextureDimension3D,
                           supportedLimits.maxTextureDimension3D};
        device.CreateTexture(&descriptor);
    }
}

// Test that depth/stencil formats are invalid for 1D and 3D texture
TEST_F(TextureValidationTest, DepthStencilFormatsFor1DAnd3D) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();

    wgpu::TextureFormat depthStencilFormats[] = {
        wgpu::TextureFormat::Stencil8,     wgpu::TextureFormat::Depth16Unorm,
        wgpu::TextureFormat::Depth24Plus,  wgpu::TextureFormat::Depth24PlusStencil8,
        wgpu::TextureFormat::Depth32Float,
    };

    for (wgpu::TextureDimension dimension : kDimensions) {
        for (wgpu::TextureFormat format : depthStencilFormats) {
            descriptor.format = format;
            descriptor.dimension = dimension;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }
}

// Test that it is valid to destroy a texture
TEST_F(TextureValidationTest, DestroyTexture) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    texture.Destroy();
}

// Test that it's valid to destroy a destroyed texture
TEST_F(TextureValidationTest, DestroyDestroyedTexture) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    texture.Destroy();
    texture.Destroy();
}

// Test that it's invalid to submit a destroyed texture in a queue
// in the case of destroy, encode, submit
TEST_F(TextureValidationTest, DestroyEncodeSubmit) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({textureView});

    // Destroy the texture
    texture.Destroy();

    wgpu::CommandEncoder encoder_post_destroy = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder_post_destroy.BeginRenderPass(&renderPass);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder_post_destroy.Finish();

    // Submit should fail due to destroyed texture
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that it's invalid to submit a destroyed texture in a queue
// in the case of encode, destroy, submit
TEST_F(TextureValidationTest, EncodeDestroySubmit) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({textureView});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();

    // Destroy the texture
    texture.Destroy();

    // Submit should fail due to destroyed texture
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test it is an error to create an RenderAttachment texture with a non-renderable format.
TEST_F(TextureValidationTest, NonRenderableAndRenderAttachment) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    // Succeeds because RGBA8Unorm is renderable
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    device.CreateTexture(&descriptor);

    for (wgpu::TextureFormat format : kNonRenderableColorFormats) {
        // Fails because `format` is non-renderable
        descriptor.format = format;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test it is an error to create a Storage texture with any format that doesn't support
// TextureUsage::StorageBinding texture usages.
TEST_F(TextureValidationTest, TextureFormatNotSupportTextureUsageStorage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::StorageBinding;

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        descriptor.format = format;
        if (utils::TextureFormatSupportsStorageTexture(format, device, UseCompatibilityMode())) {
            device.CreateTexture(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }
}

// Test it is an error to create a RenderAttachment texture with the texture dimensions that
// doesn't support TextureUsage::RenderAttachment texture usages.
TEST_F(TextureValidationTest, TextureDimensionNotSupportRenderAttachment) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    constexpr std::array<wgpu::TextureDimension, 3> kTextureDimensions = {
        {wgpu::TextureDimension::e1D, wgpu::TextureDimension::e2D, wgpu::TextureDimension::e3D}};
    for (wgpu::TextureDimension dimension : kTextureDimensions) {
        descriptor.dimension = dimension;
        if (dimension == wgpu::TextureDimension::e1D) {
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        } else {
            device.CreateTexture(&descriptor);
        }
    }
}

// Test it is an error to create a texture with format "Undefined".
TEST_F(TextureValidationTest, TextureFormatUndefined) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    descriptor.format = wgpu::TextureFormat::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test that the creation of a texture with depth32float-stencil8 will fail when the feature
// Depth32FloatStencil8 is not enabled.
TEST_F(TextureValidationTest, UseD32S8FormatWithoutEnablingFeature) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    descriptor.format = wgpu::TextureFormat::Depth32FloatStencil8;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test that the creation of a texture with BC format will fail when the feature
// textureCompressionBC is not enabled.
TEST_F(TextureValidationTest, UseBCFormatWithoutEnablingFeature) {
    for (wgpu::TextureFormat format : utils::kBCFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test that the creation of a texture with ETC2 format will fail when the feature
// textureCompressionETC2 is not enabled.
TEST_F(TextureValidationTest, UseETC2FormatWithoutEnablingFeature) {
    for (wgpu::TextureFormat format : utils::kETC2Formats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test that the creation of a texture with ASTC format will fail when the feature
// textureCompressionASTC is not enabled.
TEST_F(TextureValidationTest, UseASTCFormatWithoutEnablingFeature) {
    for (wgpu::TextureFormat format : utils::kASTCFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

class D32S8TextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }
};

// Test that depth32float-stencil8 format is invalid for 3D texture
TEST_F(D32S8TextureFormatsValidationTests, DepthStencilFormatsFor3D) {
    wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();

    for (wgpu::TextureDimension dimension : kDimensions) {
        descriptor.format = wgpu::TextureFormat::Depth32FloatStencil8;
        descriptor.dimension = dimension;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

class CompressedTextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TextureCompressionBC, wgpu::FeatureName::TextureCompressionETC2,
                wgpu::FeatureName::TextureCompressionASTC};
    }

    wgpu::TextureDescriptor CreateDefaultTextureDescriptor() {
        wgpu::TextureDescriptor descriptor =
            TextureValidationTest::CreateDefaultTextureDescriptor();
        descriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                           wgpu::TextureUsage::TextureBinding;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        return descriptor;
    }

  private:
    // Choose the LCM of all current compressed texture format texel dimensions as the
    // dimensions of the default texture.
    static constexpr uint32_t kWidth = 120;
    static constexpr uint32_t kHeight = 120;
};

// Test that only CopySrc, CopyDst and Sampled are accepted as usage in compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, TextureUsage) {
    wgpu::TextureUsage invalidUsages[] = {
        wgpu::TextureUsage::RenderAttachment,
        wgpu::TextureUsage::StorageBinding,
    };
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        for (wgpu::TextureUsage usage : invalidUsages) {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.usage = usage;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }
    }
}

// Test that using various MipLevelCount is allowed for compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, MipLevelCount) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        for (uint32_t mipLevels : {1, 3, 6}) {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.mipLevelCount = mipLevels;
            device.CreateTexture(&descriptor);
        }
    }
}

// Test that it is invalid to specify SampleCount>1 in compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, SampleCount) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        descriptor.sampleCount = 4;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test that it is allowed to create a 2D texture with depth>1 in compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, 2DArrayTexture) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        descriptor.size.depthOrArrayLayers = 6;
        device.CreateTexture(&descriptor);
    }
}

// Test that it is not allowed to create a 1D texture in compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, 1DTexture) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        // Unfortunately we can't use the block height here otherwise validation for the max
        // texture 1D size will trigger. We check the error message below to make sure the
        // correct code path is covered.
        descriptor.size.height = 1;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.dimension = wgpu::TextureDimension::e1D;
        ASSERT_DEVICE_ERROR(
            device.CreateTexture(&descriptor),
            testing::HasSubstr(
                "The dimension (TextureDimension::e1D) of a texture with a compressed format"));
    }
}

// Test that it is not allowed to create a 3D texture in compressed formats.
TEST_F(CompressedTextureFormatsValidationTests, 3DTexture) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
        descriptor.format = format;
        descriptor.size.depthOrArrayLayers = 4;
        descriptor.dimension = wgpu::TextureDimension::e3D;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test that it is invalid to use numbers for a texture's width/height that are not multiples
// of the compressed block sizes.
TEST_F(CompressedTextureFormatsValidationTests, TextureSize) {
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);

        // Test that the default size (120 x 120) is valid for all formats.
        {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            ASSERT_TRUE(descriptor.size.width % blockWidth == 0 &&
                        descriptor.size.height % blockHeight == 0);
            device.CreateTexture(&descriptor);
        }

        // Test that invalid width should cause an error. Note that if the block width of the
        // compression type is even, we test that alignment to half the width is not sufficient.
        {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.size.width =
                blockWidth % 2 == 0 ? blockWidth - (blockWidth / 2) : blockWidth - 1;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Test that invalid width should cause an error. Note that if the block height of the
        // compression type is even, we test that alignment to half the height is not
        // sufficient.
        {
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.size.height =
                blockHeight % 2 == 0 ? blockHeight - (blockHeight / 2) : blockHeight - 1;
            ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
        }

        // Test a working dimension based on some constant multipliers to the dimensions.
        {
            constexpr uint32_t kWidthMultiplier = 3;
            constexpr uint32_t kHeightMultiplier = 8;
            wgpu::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
            descriptor.format = format;
            descriptor.size.width = kWidthMultiplier * blockWidth;
            descriptor.size.height = kHeightMultiplier * blockHeight;
            device.CreateTexture(&descriptor);
        }
    }
}

class RG11B10UfloatTextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::RG11B10UfloatRenderable};
    }
};

// Test that RG11B10Ufloat format is valid as render attachment and also it allows
// multisampling if "rg11b10ufloat-renderable" feature is enabled
TEST_F(RG11B10UfloatTextureFormatsValidationTests, RenderableFeature) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    descriptor.format = wgpu::TextureFormat::RG11B10Ufloat;
    descriptor.sampleCount = 4;
    device.CreateTexture(&descriptor);
}

class BGRA8UnormTextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::BGRA8UnormStorage};
    }
};

// Test that BGRA8Unorm format is valid as storage texture if 'bgra8unorm-storage' is enabled.
TEST_F(BGRA8UnormTextureFormatsValidationTests, StorageFeature) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::StorageBinding;

    descriptor.format = wgpu::TextureFormat::BGRA8Unorm;
    device.CreateTexture(&descriptor);
}

class Unorm16TextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Unorm16TextureFormats};
    }
};

// Test that Norm16 formats are valid as renderable and sample-able texture if
// 'norm16-texture-formats' is enabled.
TEST_F(Unorm16TextureFormatsValidationTests, RenderAndSample) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    descriptor.format = wgpu::TextureFormat::R16Unorm;
    device.CreateTexture(&descriptor);

    descriptor.format = wgpu::TextureFormat::RG16Unorm;
    device.CreateTexture(&descriptor);
}

class Snorm16TextureFormatsValidationTests : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Snorm16TextureFormats};
    }
};

// Test that Norm16 formats are valid as renderable and sample-able texture if
// 'norm16-texture-formats' is enabled.
TEST_F(Snorm16TextureFormatsValidationTests, RenderAndSample) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    descriptor.format = wgpu::TextureFormat::R16Snorm;
    device.CreateTexture(&descriptor);

    descriptor.format = wgpu::TextureFormat::RG16Snorm;
    device.CreateTexture(&descriptor);

    descriptor.format = wgpu::TextureFormat::RGBA16Snorm;
    device.CreateTexture(&descriptor);
}

// Test that the Norm16 formats are not available even for just TextureBinding when the optional
// feature is not specified.
TEST_F(TextureValidationTest, Norm16NotAvailableWithoutExtension) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    descriptor.format = wgpu::TextureFormat::R16Unorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

    descriptor.format = wgpu::TextureFormat::RG16Unorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

    descriptor.format = wgpu::TextureFormat::RGBA16Unorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

    descriptor.format = wgpu::TextureFormat::R16Snorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

    descriptor.format = wgpu::TextureFormat::RG16Snorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));

    descriptor.format = wgpu::TextureFormat::RGBA16Snorm;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

static void CheckTextureMatchesDescriptor(const wgpu::Texture& tex,
                                          const wgpu::TextureDescriptor& desc) {
    EXPECT_EQ(desc.size.width, tex.GetWidth());
    EXPECT_EQ(desc.size.height, tex.GetHeight());
    EXPECT_EQ(desc.size.depthOrArrayLayers, tex.GetDepthOrArrayLayers());
    EXPECT_EQ(desc.mipLevelCount, tex.GetMipLevelCount());
    EXPECT_EQ(desc.sampleCount, tex.GetSampleCount());
    EXPECT_EQ(desc.dimension, tex.GetDimension());
    EXPECT_EQ(desc.usage, tex.GetUsage());
    EXPECT_EQ(desc.format, tex.GetFormat());
}

// Test that the texture creation parameters are correctly reflected for succesfully created
// textures.
TEST_F(TextureValidationTest, CreationParameterReflectionForValidTextures) {
    // Test reflection on two succesfully created but different textures.
    {
        wgpu::TextureDescriptor desc;
        desc.size = {3, 2, 1};
        desc.mipLevelCount = 1;
        desc.sampleCount = 4;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture tex = device.CreateTexture(&desc);

        CheckTextureMatchesDescriptor(tex, desc);
    }
    {
        wgpu::TextureDescriptor desc;
        desc.size = {47, 32, 19};
        desc.mipLevelCount = 3;
        desc.sampleCount = 1;
        desc.dimension = wgpu::TextureDimension::e3D;
        desc.usage = wgpu::TextureUsage::TextureBinding;
        desc.format = wgpu::TextureFormat::R32Float;
        wgpu::Texture tex = device.CreateTexture(&desc);

        CheckTextureMatchesDescriptor(tex, desc);
    }
}

// Test that the texture creation parameters are correctly reflected for error textures.
TEST_F(TextureValidationTest, CreationParameterReflectionForErrorTextures) {
    // Fill a descriptor with a bunch of garbage values.
    wgpu::TextureDescriptor desc;
    desc.size = {0, 0xFFFF, 1};
    desc.mipLevelCount = 0;
    desc.sampleCount = 42;
    desc.dimension = static_cast<wgpu::TextureDimension>(0xFF00);
    desc.usage = static_cast<wgpu::TextureUsage>(0xFFFF);
    desc.format = static_cast<wgpu::TextureFormat>(0xFFF0);

    // Error! Because the texture width is 0.
    wgpu::Texture tex;
    ASSERT_DEVICE_ERROR(tex = device.CreateTexture(&desc));

    CheckTextureMatchesDescriptor(tex, desc);
}

// Test that CreateErrorTexture creates an invalid texture but doesn't produce an error.
TEST_F(TextureValidationTest, CreateErrorTexture) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment;

    // Check that the descriptor is valid.
    device.CreateTexture(&desc);

    // Creating the error texture doesn't produce a validation error.
    wgpu::Texture tex = device.CreateErrorTexture(&desc);

    // Using the texture, for example to create a view, is an error.
    ASSERT_DEVICE_ERROR(tex.CreateView());
}

// Test that the texture creation parameters are correctly reflected for textures created via
// CreateErrorTexture
TEST_F(TextureValidationTest, CreationParameterReflectionForCreateErrorTexture) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment;

    wgpu::Texture tex = device.CreateErrorTexture(&desc);
    CheckTextureMatchesDescriptor(tex, desc);
}

// A tiny test that Device::ValidateTextureDescriptor works, under the assumption that all the
// texture validation logic is implemented through it (so there is no need to re-test every possible
// failure case).
TEST_F(TextureValidationTest, APIValidateTextureDescriptor) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment;
    // Spot-test for defaulting of .dimension.
    desc.dimension = wgpu::TextureDimension::Undefined;

    device.ValidateTextureDescriptor(&desc);

    desc.size.width = 0;
    ASSERT_DEVICE_ERROR(device.ValidateTextureDescriptor(&desc));
}

// Tests that specification of the transient attachment on an unsupported device
// causes an error.
TEST_F(TextureValidationTest, TransientAttachmentOnUnsupportedDevice) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

class TransientAttachmentValidationTest : public TextureValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TransientAttachments};
    }
};

// Tests that specification of the transient attachment with supported usage on
// a supported device does not raise a validation error.
TEST_F(TransientAttachmentValidationTest, Success) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;

    device.CreateTexture(&desc);
}

// Tests that specification of the transient attachment without specification of
// the render attachment causes an error.
TEST_F(TransientAttachmentValidationTest, NoRenderAttachment) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::TransientAttachment;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

// Tests that specification of the transient attachment with flags beyond just
// render attachment causes an error.
TEST_F(TransientAttachmentValidationTest, FlagsBeyondRenderAttachment) {
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.size = {1, 1, 1};
    desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment |
                 wgpu::TextureUsage::CopySrc;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

}  // anonymous namespace
}  // namespace dawn
