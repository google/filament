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

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static unsigned int kRTSize = 64;
constexpr wgpu::TextureFormat kDefaultFormat = wgpu::TextureFormat::RGBA8Unorm;
constexpr uint32_t kBytesPerTexel = 4;

class TextureViewTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews})) {
            requiredFeatures.push_back(wgpu::FeatureName::FlexibleTextureViews);
        }
        return requiredFeatures;
    }

    bool HasFlexibleTextureViews() {
        return !IsCompatibilityMode() ||
               SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews});
    }

    wgpu::Texture Create2DTexture(uint32_t width,
                                  uint32_t height,
                                  uint32_t arrayLayerCount,
                                  uint32_t mipLevelCount,
                                  wgpu::TextureUsage usage,
                                  wgpu::TextureViewDimension textureBindingViewDimension =
                                      wgpu::TextureViewDimension::e2DArray) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depthOrArrayLayers = arrayLayerCount;
        descriptor.sampleCount = 1;
        descriptor.format = kDefaultFormat;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;

        // Only set the textureBindingViewDimension in compat mode. It's not needed
        // nor used in non-compat.
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (!HasFlexibleTextureViews()) {
            textureBindingViewDimensionDesc.textureBindingViewDimension =
                textureBindingViewDimension;
            descriptor.nextInChain = &textureBindingViewDimensionDesc;
        }

        return device.CreateTexture(&descriptor);
    }

    wgpu::Texture Create3DTexture(wgpu::Extent3D size,
                                  uint32_t mipLevelCount,
                                  wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e3D;
        descriptor.size = size;
        descriptor.sampleCount = 1;
        descriptor.format = kDefaultFormat;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }
};

wgpu::ShaderModule CreateDefaultVertexShaderModule(wgpu::Device device) {
    return utils::CreateShaderModule(device, R"(
            struct VertexOut {
                @location(0) texCoord : vec2f,
                @builtin(position) position : vec4f,
            }

            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                var output : VertexOut;
                var pos = array(
                                            vec2f(-2., -2.),
                                            vec2f(-2.,  2.),
                                            vec2f( 2., -2.),
                                            vec2f(-2.,  2.),
                                            vec2f( 2., -2.),
                                            vec2f( 2.,  2.));
                var texCoord = array(
                                                 vec2f(0., 0.),
                                                 vec2f(0., 1.),
                                                 vec2f(1., 0.),
                                                 vec2f(0., 1.),
                                                 vec2f(1., 0.),
                                                 vec2f(1., 1.));
                output.position = vec4f(pos[VertexIndex], 0., 1.);
                output.texCoord = texCoord[VertexIndex];
                return output;
            }
        )");
}

class TextureViewSamplingTest : public TextureViewTestBase {
  protected:
    // Generates an arbitrary pixel value per-layer-per-level, used for the "actual" uploaded
    // textures and the "expected" results.
    static int GenerateTestPixelValue(uint32_t layer, uint32_t level) {
        return static_cast<int>(level * 10) + static_cast<int>(layer + 1);
    }

    void SetUp() override {
        DawnTest::SetUp();

        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::SamplerDescriptor samplerDescriptor = {};
        // (Off-topic) spot-test for defaulting of these six fields.
        samplerDescriptor.minFilter = wgpu::FilterMode::Undefined;
        samplerDescriptor.magFilter = wgpu::FilterMode::Undefined;
        samplerDescriptor.mipmapFilter = wgpu::MipmapFilterMode::Undefined;
        samplerDescriptor.addressModeU = wgpu::AddressMode::Undefined;
        samplerDescriptor.addressModeV = wgpu::AddressMode::Undefined;
        samplerDescriptor.addressModeW = wgpu::AddressMode::Undefined;
        mSampler = device.CreateSampler(&samplerDescriptor);

        mVSModule = CreateDefaultVertexShaderModule(device);
    }

    void InitTexture(uint32_t arrayLayerCount,
                     uint32_t mipLevelCount,
                     wgpu::TextureViewDimension textureBindingViewDimension =
                         wgpu::TextureViewDimension::e2DArray) {
        DAWN_ASSERT(arrayLayerCount > 0 && mipLevelCount > 0);

        const uint32_t textureWidthLevel0 = 1 << mipLevelCount;
        const uint32_t textureHeightLevel0 = 1 << mipLevelCount;
        constexpr wgpu::TextureUsage kUsage =
            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        mTexture = Create2DTexture(textureWidthLevel0, textureHeightLevel0, arrayLayerCount,
                                   mipLevelCount, kUsage, textureBindingViewDimension);

        mDefaultTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        mDefaultTextureViewDescriptor.format = kDefaultFormat;
        mDefaultTextureViewDescriptor.baseMipLevel = 0;
        mDefaultTextureViewDescriptor.mipLevelCount = mipLevelCount;
        mDefaultTextureViewDescriptor.baseArrayLayer = 0;
        mDefaultTextureViewDescriptor.arrayLayerCount = arrayLayerCount;
        mDefaultTextureViewDescriptor.usage = kUsage;

        // Create a texture with pixel = (0, 0, 0, level * 10 + layer + 1) at level `level` and
        // layer `layer`.
        static_assert((kTextureBytesPerRowAlignment % sizeof(utils::RGBA8)) == 0,
                      "Texture bytes per row alignment must be a multiple of sizeof(RGBA8).");
        constexpr uint32_t kPixelsPerRowPitch = kTextureBytesPerRowAlignment / sizeof(utils::RGBA8);
        ASSERT_LE(textureWidthLevel0, kPixelsPerRowPitch);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        for (uint32_t layer = 0; layer < arrayLayerCount; ++layer) {
            for (uint32_t level = 0; level < mipLevelCount; ++level) {
                const uint32_t texWidth = textureWidthLevel0 >> level;
                const uint32_t texHeight = textureHeightLevel0 >> level;

                const int pixelValue = GenerateTestPixelValue(layer, level);

                constexpr uint32_t kPaddedTexWidth = kPixelsPerRowPitch;
                std::vector<utils::RGBA8> data(kPaddedTexWidth * texHeight,
                                               utils::RGBA8(0, 0, 0, pixelValue));
                wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(
                    device, data.data(), data.size() * sizeof(utils::RGBA8),
                    wgpu::BufferUsage::CopySrc);
                wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
                    stagingBuffer, 0, kTextureBytesPerRowAlignment);
                wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                    utils::CreateTexelCopyTextureInfo(mTexture, level, {0, 0, layer});
                wgpu::Extent3D copySize = {texWidth, texHeight, 1};
                encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);
            }
        }
        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);
    }

    void Verify(const wgpu::TextureView& textureView, const char* fragmentShader, int expected) {
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader);

        utils::ComboRenderPipelineDescriptor textureDescriptor;
        textureDescriptor.vertex.module = mVSModule;
        textureDescriptor.cFragment.module = fsModule;
        textureDescriptor.cTargets[0].format = mRenderPass.colorFormat;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&textureDescriptor);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, mSampler}, {1, textureView}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectedPixel(0, 0, 0, expected);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel, mRenderPass.color, mRenderPass.width - 1,
                              mRenderPass.height - 1);
        // TODO(jiawei.shao@intel.com): add tests for 3D textures once Dawn supports 3D textures
    }

    void Texture2DViewTest(uint32_t textureArrayLayers,
                           uint32_t textureMipLevels,
                           uint32_t textureViewBaseLayer,
                           uint32_t textureViewBaseMipLevel) {
        DAWN_ASSERT(textureViewBaseLayer < textureArrayLayers);
        DAWN_ASSERT(textureViewBaseMipLevel < textureMipLevels);

        InitTexture(textureArrayLayers, textureMipLevels, wgpu::TextureViewDimension::e2D);

        wgpu::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2D;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.arrayLayerCount = 1;
        descriptor.baseMipLevel = textureViewBaseMipLevel;
        descriptor.mipLevelCount = 1;
        wgpu::TextureView textureView = mTexture.CreateView(&descriptor);

        const char* fragmentShader = R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, texCoord);
            }
        )";

        const int expected = GenerateTestPixelValue(textureViewBaseLayer, textureViewBaseMipLevel);
        Verify(textureView, fragmentShader, expected);
    }

    void Texture2DArrayViewTest(uint32_t textureArrayLayers,
                                uint32_t textureMipLevels,
                                uint32_t textureViewBaseLayer,
                                uint32_t textureViewBaseMipLevel) {
        DAWN_ASSERT(textureViewBaseLayer < textureArrayLayers);
        DAWN_ASSERT(textureViewBaseMipLevel < textureMipLevels);

        // We always set the layer count of the texture view to be 3 to match the fragment shader in
        // this test.
        constexpr uint32_t kTextureViewLayerCount = 3;
        DAWN_ASSERT(textureArrayLayers >= textureViewBaseLayer + kTextureViewLayerCount);

        InitTexture(textureArrayLayers, textureMipLevels, wgpu::TextureViewDimension::e2DArray);

        wgpu::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.arrayLayerCount = kTextureViewLayerCount;
        descriptor.baseMipLevel = textureViewBaseMipLevel;
        descriptor.mipLevelCount = 1;
        wgpu::TextureView textureView = mTexture.CreateView(&descriptor);

        const char* fragmentShader = R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d_array<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, texCoord, 0) +
                       textureSample(texture0, sampler0, texCoord, 1) +
                       textureSample(texture0, sampler0, texCoord, 2);
            }
        )";

        int expected = 0;
        for (int i = 0; i < static_cast<int>(kTextureViewLayerCount); ++i) {
            expected += GenerateTestPixelValue(textureViewBaseLayer + i, textureViewBaseMipLevel);
        }
        Verify(textureView, fragmentShader, expected);
    }

    template <typename T>
    std::string CreateFragmentShaderForCubeMapFace(T layer, bool isCubeMapArray) {
        // Reference: https://en.wikipedia.org/wiki/Cube_mapping
        const std::array<std::string, 6> kCoordsToCubeMapFace = {{
            " 1.,  tc, -sc",  // Positive X
            "-1.,  tc,  sc",  // Negative X
            " sc,  1., -tc",  // Positive Y
            " sc, -1.,  tc",  // Negative Y
            " sc,  tc,  1.",  // Positive Z
            "-sc,  tc, -1.",  // Negative Z
        }};

        const std::string textureType = isCubeMapArray ? "texture_cube_array" : "texture_cube";
        const T cubeMapArrayIndex = layer / 6;
        // We clamp here to avoid invalid negative indices for signed type T.
        const std::string coordToCubeMapFace = kCoordsToCubeMapFace[std::max(T(0), (layer % 6))];

        std::ostringstream stream;
        stream << R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : )"
               << textureType << R"(<f32>;
            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                var sc : f32 = 2.0 * texCoord.x - 1.0;
                var tc : f32 = 2.0 * texCoord.y - 1.0;
                return textureSample(texture0, sampler0, vec3f()"
               << coordToCubeMapFace << ")";

        if (isCubeMapArray) {
            stream << ", " << cubeMapArrayIndex;
        }

        stream << R"();
            })";

        return stream.str();
    }

    void TextureCubeMapTest(uint32_t textureArrayLayers,
                            uint32_t textureViewBaseLayer,
                            uint32_t textureViewLayerCount,
                            bool isCubeMapArray) {
        // Cube map arrays are unsupported in Compatbility mode.
        DAWN_TEST_UNSUPPORTED_IF(isCubeMapArray && IsCompatibilityMode());

        ASSERT_TRUE((textureViewLayerCount == 6) ||
                    (isCubeMapArray && textureViewLayerCount % 6 == 0));
        wgpu::TextureViewDimension dimension = (isCubeMapArray)
                                                   ? wgpu::TextureViewDimension::CubeArray
                                                   : wgpu::TextureViewDimension::Cube;

        constexpr uint32_t kMipLevels = 1u;
        InitTexture(textureArrayLayers, kMipLevels, dimension);

        wgpu::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = dimension;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.arrayLayerCount = textureViewLayerCount;

        wgpu::TextureView cubeMapTextureView = mTexture.CreateView(&descriptor);

        // Check the data in the every face of the cube map (array) texture view.
        for (uint32_t layer = 0; layer < textureViewLayerCount; ++layer) {
            const std::string& fragmentShader =
                CreateFragmentShaderForCubeMapFace(layer, isCubeMapArray);

            int expected = GenerateTestPixelValue(textureViewBaseLayer + layer, 0);
            Verify(cubeMapTextureView, fragmentShader.c_str(), expected);
        }
    }

    wgpu::Sampler mSampler;
    wgpu::Texture mTexture;
    wgpu::TextureViewDescriptor mDefaultTextureViewDescriptor;
    wgpu::ShaderModule mVSModule;
    utils::BasicRenderPass mRenderPass;
};

// Test drawing a rect with a 2D array texture.
TEST_P(TextureViewSamplingTest, Default2DArrayTexture) {
    // TODO(cwallez@chromium.org) understand what the issue is
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    constexpr uint32_t kLayers = 3;
    constexpr uint32_t kMipLevels = 1;
    InitTexture(kLayers, kMipLevels);

    wgpu::TextureViewDescriptor descriptor;
    // (Off-topic) spot-test for defaulting of .aspect.
    descriptor.aspect = wgpu::TextureAspect::Undefined;
    descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
    wgpu::TextureView textureView = mTexture.CreateView(&descriptor);

    const char* fragmentShader = R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d_array<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, texCoord, 0) +
                       textureSample(texture0, sampler0, texCoord, 1) +
                       textureSample(texture0, sampler0, texCoord, 2);
            }
        )";

    const int expected =
        GenerateTestPixelValue(0, 0) + GenerateTestPixelValue(1, 0) + GenerateTestPixelValue(2, 0);
    Verify(textureView, fragmentShader, expected);
}

// Test sampling a 2D array with negative signed array index.
TEST_P(TextureViewSamplingTest, 2DArrayTextureSignedNegativeIndex) {
    // TODO(cwallez@chromium.org) understand what the issue is
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());
    constexpr int32_t kIntentionalInvalidNegativeIndex = -1;
    constexpr uint32_t kLayers = 3;
    for (int32_t array_idx = kIntentionalInvalidNegativeIndex;
         array_idx < static_cast<int32_t>(kLayers); array_idx++) {
        constexpr uint32_t kMipLevels = 1;
        InitTexture(kLayers, kMipLevels);

        wgpu::TextureViewDescriptor descriptor;
        // (Off-topic) spot-test for defaulting of .aspect.
        descriptor.aspect = wgpu::TextureAspect::Undefined;
        descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
        wgpu::TextureView textureView = mTexture.CreateView(&descriptor);

        std::ostringstream fragmentShader;
        fragmentShader << R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d_array<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                let array_idx : i32 =  )"
                       << array_idx << R"(;
                return textureSample(texture0, sampler0, texCoord, array_idx);
            }
        )";

        const int expected =
            GenerateTestPixelValue(std::clamp(array_idx, 0, static_cast<int32_t>(kLayers - 1)), 0);
        Verify(textureView, fragmentShader.str().c_str(), expected);
    }
}

// Test sampling cube array with negative signed array index.
TEST_P(TextureViewSamplingTest, CubeArrayTextureSignedNegativeIndex) {
    // TODO(cwallez@chromium.org) understand what the issue is
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());
    // Cube map arrays are unsupported in Compatbility mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());
    constexpr bool kIsCubeMapArray = true;
    constexpr uint32_t kTextureViewLayerCount = 12;
    constexpr uint32_t kTextureArrayLayers = kTextureViewLayerCount;

    ASSERT_TRUE(kTextureViewLayerCount % 6 == 0);
    wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::CubeArray;
    constexpr uint32_t kMipLevels = 1u;
    InitTexture(kTextureArrayLayers, kMipLevels, dimension);

    wgpu::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
    descriptor.dimension = dimension;
    descriptor.arrayLayerCount = kTextureViewLayerCount;
    wgpu::TextureView cubeMapTextureView = mTexture.CreateView(&descriptor);

    // Check the data in the every face of the cube map (array) texture view.
    constexpr int kIntentionalInvalidNegativeIndex = -kTextureViewLayerCount;
    for (int layer = kIntentionalInvalidNegativeIndex;
         layer < static_cast<int>(kTextureArrayLayers); ++layer) {
        const std::string& fragmentShader =
            CreateFragmentShaderForCubeMapFace(layer, kIsCubeMapArray);
        int expected = GenerateTestPixelValue(std::max(0, layer), 0);
        Verify(cubeMapTextureView, fragmentShader.c_str(), expected);
    }
}

// Test sampling from a 2D texture view created on a 2D array texture.
TEST_P(TextureViewSamplingTest, Texture2DViewOn2DArrayTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());
    Texture2DViewTest(6, 1, 4, 0);
}

// Test sampling from a 2D array texture view created on a 2D array texture.
TEST_P(TextureViewSamplingTest, Texture2DArrayViewOn2DArrayTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());
    Texture2DArrayViewTest(6, 1, 2, 0);
}

// Test sampling from a 2D array texture view created on a 2D texture with one layer.
// Regression test for crbug.com/dawn/1309.
TEST_P(TextureViewSamplingTest, Texture2DArrayViewOnSingleLayer2DTexture) {
    InitTexture(1 /* array layer count */, 1 /* mip level count */);

    wgpu::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
    descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
    descriptor.baseArrayLayer = 0;
    descriptor.arrayLayerCount = 1;
    descriptor.baseMipLevel = 0;
    descriptor.mipLevelCount = 1;
    // (Off-topic) spot-test for defaulting of .aspect.
    descriptor.aspect = wgpu::TextureAspect::Undefined;
    wgpu::TextureView textureView = mTexture.CreateView(&descriptor);

    const char* fragmentShader = R"(
        @group(0) @binding(0) var sampler0 : sampler;
        @group(0) @binding(1) var texture0 : texture_2d_array<f32>;

        @fragment
        fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
            return textureSample(texture0, sampler0, texCoord, 0);
        }
    )";

    int expected = GenerateTestPixelValue(0, 0);
    Verify(textureView, fragmentShader, expected);
}

// Test sampling from a 2D texture view created on a mipmap level of a 2D texture.
TEST_P(TextureViewSamplingTest, Texture2DViewOnOneLevelOf2DTexture) {
    Texture2DViewTest(1, 6, 0, 4);
}

// Test sampling from a 2D texture view created on a mipmap level of a 2D array texture layer.
TEST_P(TextureViewSamplingTest, Texture2DViewOnOneLevelOf2DArrayTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());
    Texture2DViewTest(6, 6, 3, 4);
}

// Test sampling from a 2D array texture view created on a mipmap level of a 2D array texture.
TEST_P(TextureViewSamplingTest, Texture2DArrayViewOnOneLevelOf2DArrayTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());
    Texture2DArrayViewTest(6, 6, 2, 4);
}

// Test that an RGBA8 texture may be interpreted as RGBA8UnormSrgb and sampled from.
// More extensive color value checks and format tests are left for the CTS.
TEST_P(TextureViewSamplingTest, SRGBReinterpretation) {
    // View format reinterpretation is unsupported in Compatibility mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {2, 2, 1};
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.viewFormats = &viewDesc.format;
    textureDesc.viewFormatCount = 1;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::TexelCopyTextureInfo dst = {};
    dst.texture = texture;
    std::array<utils::RGBA8, 4> rgbaTextureData = {
        utils::RGBA8(180, 0, 0, 255),
        utils::RGBA8(0, 84, 0, 127),
        utils::RGBA8(0, 0, 62, 100),
        utils::RGBA8(62, 180, 84, 90),
    };

    wgpu::TexelCopyBufferLayout dataLayout = {};
    dataLayout.bytesPerRow = textureDesc.size.width * sizeof(utils::RGBA8);

    queue.WriteTexture(&dst, rgbaTextureData.data(), rgbaTextureData.size() * sizeof(utils::RGBA8),
                       &dataLayout, &textureDesc.size);

    wgpu::TextureView textureView = texture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                                        vec2f(-1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f( 1.0,  1.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var texture : texture_2d<f32>;

        @fragment
        fn main(@builtin(position) coord: vec4f) -> @location(0) vec4f {
            return textureLoad(texture, vec2i(coord.xy), 0);
        }
    )");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, textureDesc.size.width, textureDesc.size.height, wgpu::TextureFormat::RGBA8Unorm);
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, textureView}});

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_BETWEEN(        //
        utils::RGBA8(116, 0, 0, 255),  //
        utils::RGBA8(117, 0, 0, 255), renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 23, 0, 127),  //
        utils::RGBA8(0, 24, 0, 127), renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 0, 12, 100),  //
        utils::RGBA8(0, 0, 13, 100), renderPass.color, 0, 1);
    EXPECT_PIXEL_RGBA8_BETWEEN(         //
        utils::RGBA8(12, 116, 23, 90),  //
        utils::RGBA8(13, 117, 24, 90), renderPass.color, 1, 1);
}

// Test sampling from a cube map texture view that covers a whole 2D array texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapOnWholeTexture) {
    constexpr uint32_t kTotalLayers = 6;
    TextureCubeMapTest(kTotalLayers, 0, kTotalLayers, false);
}

// Test sampling from a cube map texture view that covers a sub part of a 2D array texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapViewOnPartOfTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());
    // TODO(dawn:1935): Total layers have to be at least 12 on Intel D3D11 Gen12.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntelGen12());

    TextureCubeMapTest(10, 2, 6, false);
}

// Test sampling from a cube map texture view that covers the last layer of a 2D array texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapViewCoveringLastLayer) {
    DAWN_TEST_UNSUPPORTED_IF(!HasFlexibleTextureViews());

    // TODO(dawn:1812): the test fails with DXGI_ERROR_DEVICE_HUNG on Intel D3D11 driver.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntel());

    constexpr uint32_t kTotalLayers = 10;
    constexpr uint32_t kBaseLayer = 4;
    TextureCubeMapTest(kTotalLayers, kBaseLayer, kTotalLayers - kBaseLayer, false);
}

// Test sampling from a cube map texture array view that covers a whole 2D array texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapArrayOnWholeTexture) {
    constexpr uint32_t kTotalLayers = 12;
    TextureCubeMapTest(kTotalLayers, 0, kTotalLayers, true);
}

// Test sampling from a cube map texture array view that covers a sub part of a 2D array texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapArrayViewOnPartOfTexture) {
    TextureCubeMapTest(20, 3, 12, true);
}

// Test sampling from a cube map texture array view that covers the last layer of a 2D array
// texture.
TEST_P(TextureViewSamplingTest, TextureCubeMapArrayViewCoveringLastLayer) {
    constexpr uint32_t kTotalLayers = 20;
    constexpr uint32_t kBaseLayer = 8;
    TextureCubeMapTest(kTotalLayers, kBaseLayer, kTotalLayers - kBaseLayer, true);
}

// Test sampling from a cube map array texture view that only has a single cube map.
TEST_P(TextureViewSamplingTest, TextureCubeMapArrayViewSingleCubeMap) {
    TextureCubeMapTest(20, 7, 6, true);
}

class TextureViewRenderingTest : public TextureViewTestBase {
  protected:
    void TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension dimension,
                                           uint32_t layerCount,
                                           uint32_t levelCount,
                                           uint32_t textureViewBaseLayer,
                                           uint32_t textureViewBaseLevel,
                                           uint32_t textureWidthLevel0,
                                           uint32_t textureHeightLevel0) {
        DAWN_ASSERT(dimension == wgpu::TextureViewDimension::e2D ||
                    dimension == wgpu::TextureViewDimension::e2DArray);
        ASSERT_LT(textureViewBaseLayer, layerCount);
        ASSERT_LT(textureViewBaseLevel, levelCount);

        constexpr wgpu::TextureUsage kUsage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture =
            Create2DTexture(textureWidthLevel0, textureHeightLevel0, layerCount, levelCount, kUsage,
                            layerCount > 1 ? wgpu::TextureViewDimension::e2DArray
                                           : wgpu::TextureViewDimension::e2D);

        wgpu::TextureViewDescriptor descriptor;
        descriptor.format = kDefaultFormat;
        descriptor.dimension = dimension;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.arrayLayerCount = 1;
        descriptor.baseMipLevel = textureViewBaseLevel;
        descriptor.mipLevelCount = 1;
        wgpu::TextureView textureView = texture.CreateView(&descriptor);

        wgpu::ShaderModule vsModule = CreateDefaultVertexShaderModule(device);

        // Clear textureView with Red(255, 0, 0, 255) and render Green(0, 255, 0, 255) into it
        utils::ComboRenderPassDescriptor renderPassInfo({textureView});
        renderPassInfo.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        const char* oneColorFragmentShader = R"(
            @fragment fn main(@location(0) texCoord : vec2f) ->
                @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            }
        )";
        wgpu::ShaderModule oneColorFsModule =
            utils::CreateShaderModule(device, oneColorFragmentShader);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = oneColorFsModule;
        pipelineDescriptor.cTargets[0].format = kDefaultFormat;

        wgpu::RenderPipeline oneColorPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassInfo);
            pass.SetPipeline(oneColorPipeline);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check if the right pixels (Green) have been written into the right part of the texture.
        uint32_t textureViewWidth = std::max(1u, textureWidthLevel0 >> textureViewBaseLevel);
        uint32_t textureViewHeight = std::max(1u, textureHeightLevel0 >> textureViewBaseLevel);
        uint32_t bytesPerRow =
            Align(kBytesPerTexel * textureWidthLevel0, kTextureBytesPerRowAlignment);
        uint32_t expectedDataSize =
            bytesPerRow / kBytesPerTexel * (textureWidthLevel0 - 1) + textureHeightLevel0;
        constexpr utils::RGBA8 kExpectedPixel(0, 255, 0, 255);
        std::vector<utils::RGBA8> expected(expectedDataSize, kExpectedPixel);
        EXPECT_TEXTURE_EQ(expected.data(), texture, {0, 0, textureViewBaseLayer},
                          {textureViewWidth, textureViewHeight}, textureViewBaseLevel);
    }
};

// Test rendering into a 2D texture view created on a mipmap level of a 2D texture.
TEST_P(TextureViewRenderingTest, Texture2DViewOnALevelOf2DTextureAsColorAttachment) {
    constexpr uint32_t kLayers = 1;
    constexpr uint32_t kMipLevels = 4;
    constexpr uint32_t kBaseLayer = 0;

    // Rendering into the first level
    {
        constexpr uint32_t kBaseLevel = 0;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }

    // Rendering into the last level
    {
        constexpr uint32_t kBaseLevel = kMipLevels - 1;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }
}

// Test rendering into a 2D texture view created on a mipmap level of a rectangular 2D texture.
TEST_P(TextureViewRenderingTest, Texture2DViewOnALevelOfRectangular2DTextureAsColorAttachment) {
    constexpr uint32_t kLayers = 1;
    constexpr uint32_t kMipLevels = 4;
    constexpr uint32_t kBaseLayer = 0;

    // Rendering into the first level
    {
        constexpr uint32_t kBaseLevel = 0;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels,
                                          1 << (kMipLevels - 2));
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << (kMipLevels - 2),
                                          1 << kMipLevels);
    }

    // Rendering into the last level
    {
        constexpr uint32_t kBaseLevel = kMipLevels - 1;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels,
                                          1 << (kMipLevels - 2));
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << (kMipLevels - 2),
                                          1 << kMipLevels);
    }
}

// Test rendering into a 2D texture view created on a layer of a 2D array texture.
TEST_P(TextureViewRenderingTest, Texture2DViewOnALayerOf2DArrayTextureAsColorAttachment) {
    // TODO(dawn:1812): the test fails with DXGI_ERROR_DEVICE_HUNG on Intel D3D11 driver.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntel());

    constexpr uint32_t kMipLevels = 1;
    constexpr uint32_t kBaseLevel = 0;
    constexpr uint32_t kLayers = 10;

    // Rendering into the first layer
    {
        constexpr uint32_t kBaseLayer = 0;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }

    // Rendering into the last layer
    {
        constexpr uint32_t kBaseLayer = kLayers - 1;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2D, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }
}

// Test rendering into a 1-layer 2D array texture view created on a mipmap level of a 2D texture.
TEST_P(TextureViewRenderingTest, Texture2DArrayViewOnALevelOf2DTextureAsColorAttachment) {
    constexpr uint32_t kLayers = 1;
    constexpr uint32_t kMipLevels = 4;
    constexpr uint32_t kBaseLayer = 0;

    // Rendering into the first level
    {
        constexpr uint32_t kBaseLevel = 0;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2DArray, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }

    // Rendering into the last level
    {
        constexpr uint32_t kBaseLevel = kMipLevels - 1;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2DArray, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }
}

// Test rendering into a 1-layer 2D array texture view created on a layer of a 2D array texture.
TEST_P(TextureViewRenderingTest, Texture2DArrayViewOnALayerOf2DArrayTextureAsColorAttachment) {
    // TODO(dawn:1812): the test fails with DXGI_ERROR_DEVICE_HUNG on Intel D3D11 driver.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntel());

    constexpr uint32_t kMipLevels = 1;
    constexpr uint32_t kBaseLevel = 0;
    constexpr uint32_t kLayers = 10;

    // Rendering into the first layer
    {
        constexpr uint32_t kBaseLayer = 0;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2DArray, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }

    // Rendering into the last layer
    {
        constexpr uint32_t kBaseLayer = kLayers - 1;
        TextureLayerAsColorAttachmentTest(wgpu::TextureViewDimension::e2DArray, kLayers, kMipLevels,
                                          kBaseLayer, kBaseLevel, 1 << kMipLevels, 1 << kMipLevels);
    }
}

// Test that an RGBA8 texture may be interpreted as RGBA8UnormSrgb and rendered to.
// More extensive color value checks and format tests are left for the CTS.
TEST_P(TextureViewRenderingTest, SRGBReinterpretationRenderAttachment) {
    // View format reinterpretation is unsupported in Compatibility mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // Test will render into an SRGB view
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;

    // Make an RGBA8Unorm texture to back the SRGB view.
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {2, 2, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.viewFormats = &viewDesc.format;
    textureDesc.viewFormatCount = 1;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    // Make an RGBA8Unorm sampled texture as the source.
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture sampledTexture = device.CreateTexture(&textureDesc);

    // Initial non-SRGB data to upload to |sampledTexture|.
    std::array<utils::RGBA8, 4> rgbaTextureData = {
        utils::RGBA8(117, 0, 0, 255),
        utils::RGBA8(0, 23, 0, 127),
        utils::RGBA8(0, 0, 12, 100),
        utils::RGBA8(13, 117, 24, 90),
    };

    wgpu::TexelCopyTextureInfo dst = {};
    wgpu::TexelCopyBufferLayout dataLayout = {};

    // Upload |rgbaTextureData| into |sampledTexture|.
    dst.texture = sampledTexture;
    dataLayout.bytesPerRow = textureDesc.size.width * sizeof(utils::RGBA8);
    queue.WriteTexture(&dst, rgbaTextureData.data(), rgbaTextureData.size() * sizeof(utils::RGBA8),
                       &dataLayout, &textureDesc.size);

    // View both the attachment as SRGB.
    wgpu::TextureView textureView = texture.CreateView(&viewDesc);

    // Create a render pipeline to blit |sampledTexture| into |textureView|.
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                                        vec2f(-1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f( 1.0,  1.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var texture : texture_2d<f32>;

        @fragment
        fn main(@builtin(position) coord: vec4f) -> @location(0) vec4f {
            return textureLoad(texture, vec2i(coord.xy), 0);
        }
    )");
    pipelineDesc.cTargets[0].format = viewDesc.format;

    // Submit a render pass to perform the blit from |sampledTexture| to |textureView|.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampledTexture.CreateView()}});

        utils::ComboRenderPassDescriptor renderPassInfo({textureView});

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check the results. This is the sRGB encoding for the same non-SRGB colors
    // represented by |initialData|.
    EXPECT_PIXEL_RGBA8_BETWEEN(        //
        utils::RGBA8(180, 0, 0, 255),  //
        utils::RGBA8(181, 0, 0, 255), texture, 0, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 85, 0, 127),  //
        utils::RGBA8(0, 86, 0, 127), texture, 1, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 0, 61, 100),  //
        utils::RGBA8(0, 0, 62, 100), texture, 0, 1);
    EXPECT_PIXEL_RGBA8_BETWEEN(         //
        utils::RGBA8(64, 180, 86, 90),  //
        utils::RGBA8(15, 181, 87, 90), texture, 1, 1);
}

// Test that an RGBA8 texture may be interpreted as RGBA8UnormSrgb and resolved to.
// More extensive color value checks and format tests are left for the CTS.
// This test samples the RGBA8Unorm texture into an RGBA8Unorm multisample texture viewed as SRGB,
// and resolves it into an RGBA8Unorm texture, viewed as SRGB.
TEST_P(TextureViewRenderingTest, SRGBReinterpretionResolveAttachment) {
    // View format reinterpretation is unsupported in Compatibility mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // Test will resolve into an SRGB view
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;

    // Make an RGBA8Unorm texture to back the SRGB view.
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {2, 2, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.viewFormats = &viewDesc.format;
    textureDesc.viewFormatCount = 1;
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture resolveTexture = device.CreateTexture(&textureDesc);

    // Make an RGBA8Unorm multisampled texture for the color attachment.
    textureDesc.sampleCount = 4;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture multisampledTexture = device.CreateTexture(&textureDesc);

    // Make an RGBA8Unorm sampled texture as the source.
    textureDesc.sampleCount = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture sampledTexture = device.CreateTexture(&textureDesc);

    // Initial non-SRGB data to upload to |sampledTexture|.
    std::array<utils::RGBA8, 4> rgbaTextureData = {
        utils::RGBA8(117, 0, 0, 255),
        utils::RGBA8(0, 23, 0, 127),
        utils::RGBA8(0, 0, 12, 100),
        utils::RGBA8(13, 117, 24, 90),
    };

    wgpu::TexelCopyTextureInfo dst = {};
    wgpu::TexelCopyBufferLayout dataLayout = {};

    // Upload |rgbaTextureData| into |sampledTexture|.
    dst.texture = sampledTexture;
    dataLayout.bytesPerRow = textureDesc.size.width * sizeof(utils::RGBA8);
    queue.WriteTexture(&dst, rgbaTextureData.data(), rgbaTextureData.size() * sizeof(utils::RGBA8),
                       &dataLayout, &textureDesc.size);

    // View both the multisampled texture and the resolve texture as SRGB.
    wgpu::TextureView resolveView = resolveTexture.CreateView(&viewDesc);
    wgpu::TextureView multisampledTextureView = multisampledTexture.CreateView(&viewDesc);

    // Create a render pipeline to blit |sampledTexture| into |multisampledTextureView|.
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                                        vec2f(-1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f(-1.0,  1.0),
                                        vec2f( 1.0, -1.0),
                                        vec2f( 1.0,  1.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var texture : texture_2d<f32>;

        @fragment
        fn main(@builtin(position) coord: vec4f) -> @location(0) vec4f {
            return textureLoad(texture, vec2i(coord.xy), 0);
        }
    )");
    pipelineDesc.cTargets[0].format = viewDesc.format;
    pipelineDesc.multisample.count = 4;

    // Submit a render pass to perform the blit from |sampledTexture| to |multisampledTextureView|.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampledTexture.CreateView()}});

        utils::ComboRenderPassDescriptor renderPassInfo({multisampledTextureView});
        renderPassInfo.cColorAttachments[0].resolveTarget = resolveView;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check the results. This is the sRGB encoding for the same non-SRGB colors
    // represented by |initialData|.
    EXPECT_PIXEL_RGBA8_BETWEEN(        //
        utils::RGBA8(180, 0, 0, 255),  //
        utils::RGBA8(181, 0, 0, 255), resolveTexture, 0, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 85, 0, 127),  //
        utils::RGBA8(0, 86, 0, 127), resolveTexture, 1, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(       //
        utils::RGBA8(0, 0, 61, 100),  //
        utils::RGBA8(0, 0, 62, 100), resolveTexture, 0, 1);
    EXPECT_PIXEL_RGBA8_BETWEEN(         //
        utils::RGBA8(64, 180, 86, 90),  //
        utils::RGBA8(15, 181, 87, 90), resolveTexture, 1, 1);
}

DAWN_INSTANTIATE_TEST(TextureViewSamplingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(TextureViewRenderingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class TextureViewTest : public DawnTest {};

// This is a regression test for crbug.com/dawn/399 where creating a texture view with only copy
// usage would cause the Vulkan validation layers to warn
TEST_P(TextureViewTest, OnlyCopySrcDst) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {4, 4, 1};
    descriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView view = texture.CreateView();
}

// Test that a texture view can be created from a destroyed texture without
// backend errors.
TEST_P(TextureViewTest, DestroyedTexture) {
    wgpu::TextureDescriptor descriptor;
    descriptor.size = {4, 4, 2};
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture texture = device.CreateTexture(&descriptor);
    texture.Destroy();

    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.baseArrayLayer = 1;
    viewDesc.arrayLayerCount = 1;
    wgpu::TextureView view = texture.CreateView(&viewDesc);
}

DAWN_INSTANTIATE_TEST(TextureViewTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class TextureView3DTest : public TextureViewTestBase {};

// Test that 3D textures and 3D texture views can be created successfully
TEST_P(TextureView3DTest, BasicTest) {
    wgpu::Texture texture = Create3DTexture({4, 4, 4}, 3, wgpu::TextureUsage::TextureBinding);
    wgpu::TextureView view = texture.CreateView();
}

DAWN_INSTANTIATE_TEST(TextureView3DTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class TextureView1DTest : public DawnTest {};

// Test that it is possible to create a 1D texture view and sample from it.
TEST_P(TextureView1DTest, Sampling) {
    // Create a 1D texture and fill it with some data.
    wgpu::TextureDescriptor texDesc;
    texDesc.dimension = wgpu::TextureDimension::e1D;
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    texDesc.size = {4, 1, 1};
    wgpu::Texture tex = device.CreateTexture(&texDesc);

    std::array<utils::RGBA8, 4> data = {utils::RGBA8::kGreen, utils::RGBA8::kRed,
                                        utils::RGBA8::kBlue, utils::RGBA8::kWhite};
    wgpu::TexelCopyTextureInfo target = utils::CreateTexelCopyTextureInfo(tex, 0, {});
    wgpu::TexelCopyBufferLayout layout =
        utils::CreateTexelCopyBufferLayout(0, wgpu::kCopyStrideUndefined);
    queue.WriteTexture(&target, &data, sizeof(data), &layout, &texDesc.size);

    // Create a pipeline that will sample from the 1D texture and output to an attachment.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex
        fn vs(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec4f( 0.,  2., 0., 1.),
                vec4f(-3., -1., 0., 1.),
                vec4f( 3., -1., 0., 1.));
            return pos[VertexIndex];
        }

        @group(0) @binding(0) var tex : texture_1d<f32>;
        @group(0) @binding(1) var samp : sampler;
        @fragment
        fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f {
            return textureSample(tex, samp, pos.x / 4.0);
        }
    )");
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Do the sample + rendering.
    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {{0, tex.CreateView()}, {1, device.CreateSampler()}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 4, 1);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bg);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check texels got sampled correctly.
    EXPECT_PIXEL_RGBA8_EQ(data[0], rp.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(data[1], rp.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(data[2], rp.color, 2, 0);
    EXPECT_PIXEL_RGBA8_EQ(data[3], rp.color, 3, 0);
}

DAWN_INSTANTIATE_TEST(TextureView1DTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn
