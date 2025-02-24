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

#include <array>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class TextureViewValidationTest : public ValidationTest {};

constexpr uint32_t kWidth = 32u;
constexpr uint32_t kHeight = 32u;
constexpr uint32_t kDepth = 6u;
constexpr uint32_t kDefaultMipLevels = 6u;

constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

wgpu::Texture Create2DArrayTexture(wgpu::Device& device,
                                   uint32_t arrayLayerCount,
                                   uint32_t width = kWidth,
                                   uint32_t height = kHeight,
                                   uint32_t mipLevelCount = kDefaultMipLevels,
                                   uint32_t sampleCount = 1) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = arrayLayerCount;
    descriptor.sampleCount = sampleCount;
    descriptor.format = kDefaultTextureFormat;
    descriptor.mipLevelCount = mipLevelCount;
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    return device.CreateTexture(&descriptor);
}

wgpu::Texture Create3DTexture(wgpu::Device& device) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size = {kWidth, kHeight, kDepth};
    descriptor.sampleCount = 1;
    descriptor.format = kDefaultTextureFormat;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    return device.CreateTexture(&descriptor);
}

wgpu::Texture Create1DTexture(wgpu::Device& device) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e1D;
    descriptor.size = {kWidth, 1, 1};
    descriptor.format = kDefaultTextureFormat;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    return device.CreateTexture(&descriptor);
}

wgpu::Texture CreateDepthStencilTexture(wgpu::Device& device, wgpu::TextureFormat format) {
    wgpu::TextureDescriptor descriptor = {};
    descriptor.size = {kWidth, kHeight, kDepth};
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    descriptor.mipLevelCount = kDefaultMipLevels;
    descriptor.format = format;
    return device.CreateTexture(&descriptor);
}

wgpu::TextureViewDescriptor CreateDefaultViewDescriptor(wgpu::TextureViewDimension dimension) {
    wgpu::TextureViewDescriptor descriptor;
    descriptor.format = kDefaultTextureFormat;
    descriptor.dimension = dimension;
    descriptor.baseMipLevel = 0;
    if (dimension != wgpu::TextureViewDimension::e1D) {
        descriptor.mipLevelCount = kDefaultMipLevels;
    }
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = 1;
    return descriptor;
}

// Test creating texture view on a 2D non-array texture
TEST_F(TextureViewValidationTest, CreateTextureViewOnTexture2D) {
    wgpu::Texture texture = Create2DArrayTexture(device, 1);

    wgpu::TextureViewDescriptor base2DTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);

    // It is an error to create a view with zero 'arrayLayerCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.arrayLayerCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with zero 'mipLevelCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.mipLevelCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a 2D texture view on a 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
    }

    // It is an error to view a layer past the end of the texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a 1-layer 2D array texture view on a 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a 3D texture view on a 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e3D;
        descriptor.arrayLayerCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // baseMipLevel == k && mipLevelCount == WGPU_MIP_LEVEL_COUNT_UNDEFINED means to use levels
    // k..end.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;

        descriptor.baseMipLevel = 0;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = 1;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = kDefaultMipLevels - 1;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = kDefaultMipLevels;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to make the mip level out of range.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = kDefaultMipLevels + 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = 1;
        descriptor.mipLevelCount = kDefaultMipLevels;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = kDefaultMipLevels - 1;
        descriptor.mipLevelCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = kDefaultMipLevels;
        descriptor.mipLevelCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

// Test creating texture view on a 2D array texture
TEST_F(TextureViewValidationTest, CreateTextureViewOnTexture2DArray) {
    constexpr uint32_t kDefaultArrayLayers = 6;

    wgpu::Texture texture = Create2DArrayTexture(device, kDefaultArrayLayers);

    wgpu::TextureViewDescriptor base2DArrayTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2DArray);

    // It is an error to create a view with zero 'arrayLayerCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        descriptor.arrayLayerCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with zero 'mipLevelCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        descriptor.mipLevelCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a 2D texture view on a 2D array texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
    }

    // It is OK to create a 2D array texture view on a 2D array texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.arrayLayerCount = kDefaultArrayLayers;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a 3D texture view on a 2D array texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e3D;
        descriptor.arrayLayerCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a 1D texture view on a 2D array texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e1D;
        descriptor.arrayLayerCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // baseArrayLayer == k && arrayLayerCount == wgpu::kArrayLayerCountUndefined means to use
    // layers k..end.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.arrayLayerCount = wgpu::kArrayLayerCountUndefined;

        descriptor.baseArrayLayer = 0;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = 1;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = kDefaultArrayLayers - 1;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = kDefaultArrayLayers;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error for the array layer range of the view to exceed that of the texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = kDefaultArrayLayers + 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseArrayLayer = 1;
        descriptor.arrayLayerCount = kDefaultArrayLayers;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseArrayLayer = kDefaultArrayLayers - 1;
        descriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseArrayLayer = kDefaultArrayLayers;
        descriptor.arrayLayerCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

// Test creating texture view on a 3D texture
TEST_F(TextureViewValidationTest, CreateTextureViewOnTexture3D) {
    wgpu::Texture texture = Create3DTexture(device);

    wgpu::TextureViewDescriptor base3DTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e3D);

    // It is an error to create a view with zero 'arrayLayerCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        descriptor.arrayLayerCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with zero 'mipLevelCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        descriptor.mipLevelCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a 3D texture view on a 3D texture.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a 1D/2D/2DArray/Cube/CubeArray texture view on a 3D texture.
    {
        wgpu::TextureViewDimension invalidDimensions[] = {
            wgpu::TextureViewDimension::e1D,       wgpu::TextureViewDimension::e2D,
            wgpu::TextureViewDimension::e2DArray,  wgpu::TextureViewDimension::Cube,
            wgpu::TextureViewDimension::CubeArray,
        };
        for (wgpu::TextureViewDimension dimension : invalidDimensions) {
            wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
            descriptor.dimension = dimension;
            if (dimension == wgpu::TextureViewDimension::Cube ||
                dimension == wgpu::TextureViewDimension::CubeArray) {
                descriptor.arrayLayerCount = 6;
            }
            ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        }
    }

    // baseMipLevel == k && mipLevelCount == WGPU_MIP_LEVEL_COUNT_UNDEFINED means to use levels
    // k..end.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        descriptor.mipLevelCount = WGPU_MIP_LEVEL_COUNT_UNDEFINED;

        descriptor.baseMipLevel = 0;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = 1;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = kDefaultMipLevels - 1;
        texture.CreateView(&descriptor);
        descriptor.baseMipLevel = kDefaultMipLevels;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to make the mip level out of range.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        descriptor.baseMipLevel = 0;
        descriptor.mipLevelCount = kDefaultMipLevels + 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = 1;
        descriptor.mipLevelCount = kDefaultMipLevels;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = kDefaultMipLevels - 1;
        descriptor.mipLevelCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseMipLevel = kDefaultMipLevels;
        descriptor.mipLevelCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // baseArrayLayer == k && arrayLayerCount == wgpu::kArrayLayerCountUndefined means to use
    // layers k..end. But baseArrayLayer must be 0, and arrayLayerCount must be 1 at most for 3D
    // texture view.
    {
        wgpu::TextureViewDescriptor descriptor = base3DTextureViewDescriptor;
        descriptor.arrayLayerCount = wgpu::kArrayLayerCountUndefined;
        descriptor.baseArrayLayer = 0;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));

        descriptor.baseArrayLayer = 0;
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.arrayLayerCount = kDepth;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

// Test creating texture view on a 1D texture
TEST_F(TextureViewValidationTest, CreateTextureViewOnTexture1D) {
    wgpu::Texture texture = Create1DTexture(device);

    wgpu::TextureViewDescriptor base1DTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e1D);

    // It is an error to create a view with zero 'arrayLayerCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base1DTextureViewDescriptor;
        descriptor.arrayLayerCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with zero 'mipLevelCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base1DTextureViewDescriptor;
        descriptor.mipLevelCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a 1D texture view on a 1D texture.
    {
        wgpu::TextureViewDescriptor descriptor = base1DTextureViewDescriptor;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a 2D/2DArray/Cube/CubeArray/3D texture view on a 1D texture.
    {
        wgpu::TextureViewDimension invalidDimensions[] = {
            wgpu::TextureViewDimension::e2D,  wgpu::TextureViewDimension::e2DArray,
            wgpu::TextureViewDimension::Cube, wgpu::TextureViewDimension::CubeArray,
            wgpu::TextureViewDimension::e3D,
        };
        for (wgpu::TextureViewDimension dimension : invalidDimensions) {
            wgpu::TextureViewDescriptor descriptor = base1DTextureViewDescriptor;
            descriptor.dimension = dimension;
            ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        }
    }

    // No tests for setting mip levels / array layer ranges because 1D textures can only have
    // a single mip and layer.
}

// Test creating texture view on a multisampled 2D texture
TEST_F(TextureViewValidationTest, CreateTextureViewOnMultisampledTexture2D) {
    wgpu::Texture texture = Create2DArrayTexture(device, /* arrayLayerCount */ 1, kWidth, kHeight,
                                                 /* mipLevelCount */ 1, /* sampleCount */ 4);

    // It is OK to create a 2D texture view on a multisampled 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = {};
        texture.CreateView(&descriptor);
    }

    // It is an error to create a 1-layer 2D array texture view on a multisampled 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = {};
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        descriptor.arrayLayerCount = 1;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a 1D texture view on a multisampled 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = {};
        descriptor.dimension = wgpu::TextureViewDimension::e1D;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a 3D texture view on a multisampled 2D texture.
    {
        wgpu::TextureViewDescriptor descriptor = {};
        descriptor.dimension = wgpu::TextureViewDimension::e3D;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

// Using the "none" ("default") values validates the same as explicitly
// specifying the values they're supposed to default to.
// Variant for a 2D texture with more than 1 array layer.
TEST_F(TextureViewValidationTest, TextureViewDescriptorDefaults2DArray) {
    constexpr uint32_t kDefaultArrayLayers = 8;
    wgpu::Texture texture = Create2DArrayTexture(device, kDefaultArrayLayers);

    { texture.CreateView(); }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.format = wgpu::TextureFormat::Undefined;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::R8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Undefined;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        texture.CreateView(&descriptor);
        // Setting view dimension to 2D, its arrayLayer will default to 1. And view creation
        // will success.
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        texture.CreateView(&descriptor);
        // Setting view dimension to Cube, its arrayLayer will default to 6.
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = 2;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = 3;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        // Setting view dimension to CubeArray, its arrayLayer will default to
        // size.depthOrArrayLayers (kDefaultArrayLayers) - baseArrayLayer.
        descriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        descriptor.baseArrayLayer = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.baseArrayLayer = 2;
        texture.CreateView(&descriptor);
        descriptor.baseArrayLayer = 3;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;

        // Setting array layers to > 1 with an explicit dimensionality of 2D will
        // causes an error.
        descriptor.arrayLayerCount = kDefaultArrayLayers;
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        // Setting view dimension to Undefined will result in a dimension of 2DArray because the
        // underlying texture has > 1 array layers.
        descriptor.dimension = wgpu::TextureViewDimension::Undefined;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        texture.CreateView(&descriptor);

        descriptor.mipLevelCount = kDefaultMipLevels;
        texture.CreateView(&descriptor);
    }
}

// Using the "none" ("default") values validates the same as explicitly
// specifying the values they're supposed to default to.
// Variant for a 2D texture with only 1 array layer.
TEST_F(TextureViewValidationTest, TextureViewDescriptorDefaults2DNonArray) {
    constexpr uint32_t kDefaultArrayLayers = 1;
    wgpu::Texture texture = Create2DArrayTexture(device, kDefaultArrayLayers);

    { texture.CreateView(); }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.format = wgpu::TextureFormat::Undefined;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::R8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Undefined;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        texture.CreateView(&descriptor);
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.arrayLayerCount = wgpu::kArrayLayerCountUndefined;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.mipLevelCount = kDefaultMipLevels;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = kDefaultArrayLayers;
        texture.CreateView(&descriptor);
    }
}

// Using the "none" ("default") values validates the same as explicitly
// specifying the values they're supposed to default to.
// Variant for a 3D texture.
TEST_F(TextureViewValidationTest, TextureViewDescriptorDefaults3D) {
    wgpu::Texture texture = Create3DTexture(device);

    { texture.CreateView(); }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.format = wgpu::TextureFormat::Undefined;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&descriptor);
        descriptor.format = wgpu::TextureFormat::R8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Undefined;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e3D;
        texture.CreateView(&descriptor);
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.arrayLayerCount = wgpu::kArrayLayerCountUndefined;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = 1;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
    {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.mipLevelCount = kDefaultMipLevels;
        texture.CreateView(&descriptor);
        descriptor.arrayLayerCount = kDepth;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

// Regression test for crbug.com/1314049. Format default depends on the aspect.
// Test that computing the default does not crash if the aspect is invalid.
TEST_F(TextureViewValidationTest, TextureViewDescriptorDefaultsInvalidAspect) {
    wgpu::Texture texture =
        CreateDepthStencilTexture(device, wgpu::TextureFormat::Depth24PlusStencil8);

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.aspect = static_cast<wgpu::TextureAspect>(-1);

    // Validation should catch the invalid aspect.
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc),
                        testing::HasSubstr("is invalid for WGPUTextureAspect"));
}

// Test creating cube map texture view
TEST_F(TextureViewValidationTest, CreateCubeMapTextureView) {
    constexpr uint32_t kDefaultArrayLayers = 16;

    wgpu::Texture texture = Create2DArrayTexture(device, kDefaultArrayLayers);

    wgpu::TextureViewDescriptor base2DArrayTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2DArray);

    // It is an error to create a view with zero 'arrayLayerCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        descriptor.arrayLayerCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with zero 'mipLevelCount'.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        descriptor.mipLevelCount = 0;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a cube map texture view with arrayLayerCount == 6.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        descriptor.arrayLayerCount = 6;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a cube map texture view with arrayLayerCount != 6.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        descriptor.arrayLayerCount = 3;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is OK to create a cube map array texture view with arrayLayerCount % 6 == 0.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        descriptor.arrayLayerCount = 12;
        texture.CreateView(&descriptor);
    }

    // It is an error to create a cube map array texture view with arrayLayerCount % 6 != 0.
    {
        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        descriptor.arrayLayerCount = 11;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a cube map texture view with width != height.
    {
        wgpu::Texture nonSquareTexture = Create2DArrayTexture(device, 18, 32, 16, 5);

        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::Cube;
        descriptor.arrayLayerCount = 6;
        ASSERT_DEVICE_ERROR(nonSquareTexture.CreateView(&descriptor));
    }

    // It is an error to create a cube map array texture view with width != height.
    {
        wgpu::Texture nonSquareTexture = Create2DArrayTexture(device, 18, 32, 16, 5);

        wgpu::TextureViewDescriptor descriptor = base2DArrayTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::CubeArray;
        descriptor.arrayLayerCount = 12;
        ASSERT_DEVICE_ERROR(nonSquareTexture.CreateView(&descriptor));
    }
}

// Test the format compatibility rules when creating a texture view.
TEST_F(TextureViewValidationTest, TextureViewFormatCompatibility) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size.width = 4;
    textureDesc.size.height = 4;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding;

    wgpu::TextureViewDescriptor viewDesc = {};

    // It is an error to create an sRGB texture view from an RGB texture, without viewFormats.
    {
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is an error to create an RGB texture view from an sRGB texture, without viewFormats.
    {
        textureDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        viewDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is an error to create a texture view with a depth-stencil format of an RGBA texture.
    {
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is an error to create a texture view with a depth format of a depth-stencil texture.
    {
        textureDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24Plus;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is invalid to create a texture view with a combined depth-stencil format if only
    // the depth aspect is selected.
    {
        textureDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is invalid to create a texture view with a combined depth-stencil format if only
    // the stencil aspect is selected.
    {
        textureDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.aspect = wgpu::TextureAspect::StencilOnly;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // It is valid to create a texture view with a depth format of a depth-stencil texture
    // if the depth only aspect is selected.
    {
        textureDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewDesc.format = wgpu::TextureFormat::Depth24Plus;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);
        texture.CreateView(&viewDesc);

        viewDesc = {};
    }

    // Prep for testing a single view format in viewFormats.
    wgpu::TextureFormat viewFormat;
    textureDesc.viewFormats = &viewFormat;
    textureDesc.viewFormatCount = 1;

    // An aspect format is not a valid view format of a depth-stencil texture.
    {
        textureDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        viewFormat = wgpu::TextureFormat::Depth24Plus;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
    }

    // Test that a RGBA texture can be viewed as both RGBA and RGBASrgb, but not BGRA or
    // BGRASrgb
    {
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewFormat = wgpu::TextureFormat::RGBA8UnormSrgb;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

        viewDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // Test that a BGRASrgb texture can be viewed as both BGRA and BGRASrgb, but not RGBA or
    // RGBASrgb
    {
        textureDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        viewFormat = wgpu::TextureFormat::BGRA8Unorm;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        viewDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

        viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // Test an RGBA format may be viewed as RGBA (same)
    {
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewFormat = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }

    // Test that duplicate, and multiple view formats are allowed.
    {
        std::array<wgpu::TextureFormat, 5> viewFormats = {
            wgpu::TextureFormat::RGBA8UnormSrgb, wgpu::TextureFormat::RGBA8Unorm,
            wgpu::TextureFormat::RGBA8Unorm,     wgpu::TextureFormat::RGBA8UnormSrgb,
            wgpu::TextureFormat::RGBA8Unorm,
        };
        textureDesc.viewFormats = viewFormats.data();
        textureDesc.viewFormatCount = viewFormats.size();

        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        texture.CreateView(&viewDesc);

        viewDesc.format = wgpu::TextureFormat::BGRA8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

        viewDesc.format = wgpu::TextureFormat::BGRA8UnormSrgb;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
    }
}

// Test that it's valid to create a texture view from a destroyed texture
TEST_F(TextureViewValidationTest, DestroyCreateTextureView) {
    wgpu::Texture texture = Create2DArrayTexture(device, 1);
    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);
    texture.Destroy();
    texture.CreateView(&descriptor);
}

// Test that the selected TextureAspects must exist in the texture format
TEST_F(TextureViewValidationTest, AspectMustExist) {
    wgpu::TextureDescriptor descriptor = {};
    descriptor.size = {1, 1, 1};
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    // Can select: All and DepthOnly from Depth32Float, but not StencilOnly
    {
        descriptor.format = wgpu::TextureFormat::Depth32Float;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TextureViewDescriptor viewDescriptor = {};
        viewDescriptor.aspect = wgpu::TextureAspect::All;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::DepthOnly;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::StencilOnly;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDescriptor));
    }

    // Can select: All, DepthOnly, and StencilOnly from Depth24PlusStencil8
    {
        descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TextureViewDescriptor viewDescriptor = {};
        viewDescriptor.aspect = wgpu::TextureAspect::All;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::DepthOnly;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::StencilOnly;
        texture.CreateView(&viewDescriptor);
    }

    // Can select: All from RGBA8Unorm
    {
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TextureViewDescriptor viewDescriptor = {};
        viewDescriptor.aspect = wgpu::TextureAspect::All;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::DepthOnly;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDescriptor));

        viewDescriptor.aspect = wgpu::TextureAspect::StencilOnly;
        ASSERT_DEVICE_ERROR(texture.CreateView(&viewDescriptor));
    }
}

// Test that CreateErrorView creates an invalid texture view but doesn't produce an error.
TEST_F(TextureViewValidationTest, CreateErrorView) {
    wgpu::Texture texture = Create2DArrayTexture(device, 1);
    wgpu::TextureViewDescriptor descriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);

    // Creating the error texture view doesn't produce an error.
    wgpu::TextureView view = texture.CreateErrorView(&descriptor);

    // Using the error texture view will throw an error.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, layout, {{0, view}}));
}

// Tests that texture view usage is validated for the texture view format and is compatible with the
// source texture usages
TEST_F(TextureViewValidationTest, Usage) {
    wgpu::TextureFormat viewFormats[] = {wgpu::TextureFormat::RGBA8Unorm,
                                         wgpu::TextureFormat::RGBA8UnormSrgb};

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.size.width = kWidth;
    textureDescriptor.size.height = kHeight;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding |
                              wgpu::TextureUsage::RenderAttachment |
                              wgpu::TextureUsage::StorageBinding;
    textureDescriptor.viewFormats = viewFormats;
    textureDescriptor.viewFormatCount = 2;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::TextureViewDescriptor base2DTextureViewDescriptor;
    base2DTextureViewDescriptor.format = kDefaultTextureFormat;
    base2DTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    base2DTextureViewDescriptor.baseMipLevel = 0;
    base2DTextureViewDescriptor.mipLevelCount = 1;
    base2DTextureViewDescriptor.baseArrayLayer = 0;
    base2DTextureViewDescriptor.arrayLayerCount = 1;

    // It is an error to request a usage outside of the source texture's usage
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.usage |= wgpu::TextureUsage::CopyDst;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // It is an error to create a view with RGBA8UnormSrgb and default usage which includes
    // StorageBinding
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.format = wgpu::TextureFormat::RGBA8UnormSrgb;

        // TODO(363903526): Change this to inherited usage when inherited and explicit usages are
        // validated the same way.
        descriptor.usage = textureDescriptor.usage;

        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }

    // A view can be created for RGBA8UnormSrgb with a compatible subset of usages
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.format = wgpu::TextureFormat::RGBA8UnormSrgb;
        descriptor.usage =
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
        texture.CreateView(&descriptor);
    }
}

class D32S8TextureViewValidationTests : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }
};

// Test that the selected TextureAspects must exist in the Depth32FloatStencil8 texture format
TEST_F(D32S8TextureViewValidationTests, AspectMustExist) {
    wgpu::Texture texture =
        CreateDepthStencilTexture(device, wgpu::TextureFormat::Depth32FloatStencil8);

    // Can select: All, DepthOnly, and StencilOnly from Depth32FloatStencil8
    {
        wgpu::TextureViewDescriptor viewDescriptor = {};
        viewDescriptor.aspect = wgpu::TextureAspect::All;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::DepthOnly;
        texture.CreateView(&viewDescriptor);

        viewDescriptor.aspect = wgpu::TextureAspect::StencilOnly;
        texture.CreateView(&viewDescriptor);
    }
}

// Test the format compatibility rules when creating a texture view.
TEST_F(D32S8TextureViewValidationTests, TextureViewFormatCompatibility) {
    wgpu::Texture texture =
        CreateDepthStencilTexture(device, wgpu::TextureFormat::Depth32FloatStencil8);

    wgpu::TextureViewDescriptor base2DTextureViewDescriptor =
        CreateDefaultViewDescriptor(wgpu::TextureViewDimension::e2D);

    // It is an error to create a texture view in color format on a depth-stencil texture.
    {
        wgpu::TextureViewDescriptor descriptor = base2DTextureViewDescriptor;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        ASSERT_DEVICE_ERROR(texture.CreateView(&descriptor));
    }
}

}  // anonymous namespace
}  // namespace dawn
