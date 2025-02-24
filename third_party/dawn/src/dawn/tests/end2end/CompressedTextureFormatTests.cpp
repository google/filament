// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// The helper struct to configure the copies between buffers and textures.
struct CopyConfig {
    wgpu::TextureDescriptor textureDescriptor;
    wgpu::Extent3D copyExtent3D;
    wgpu::Origin3D copyOrigin3D = {0, 0, 0};
    uint32_t viewMipmapLevel = 0;
    uint32_t bufferOffset = 0;
    uint32_t bytesPerRowAlignment = kTextureBytesPerRowAlignment;
    uint32_t rowsPerImage = wgpu::kCopyStrideUndefined;
};

using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(CompressedTextureFormatTestParams, TextureFormat);

class CompressedTextureFormatTest : public DawnTestWithParams<CompressedTextureFormatTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        const wgpu::TextureFormat format = GetParam().mTextureFormat;
        if (utils::IsBCTextureFormat(format) &&
            SupportsFeatures({wgpu::FeatureName::TextureCompressionBC})) {
            mIsFormatSupported = true;
            return {wgpu::FeatureName::TextureCompressionBC};
        }
        if (utils::IsETC2TextureFormat(format) &&
            SupportsFeatures({wgpu::FeatureName::TextureCompressionETC2})) {
            mIsFormatSupported = true;
            return {wgpu::FeatureName::TextureCompressionETC2};
        }
        if (utils::IsASTCTextureFormat(format) &&
            SupportsFeatures({wgpu::FeatureName::TextureCompressionASTC})) {
            mIsFormatSupported = true;
            return {wgpu::FeatureName::TextureCompressionASTC};
        }
        return {};
    }

    bool IsFormatSupported() const { return mIsFormatSupported; }

    uint32_t BlockWidthInTexels() const {
        DAWN_ASSERT(IsFormatSupported());
        return utils::GetTextureFormatBlockWidth(GetParam().mTextureFormat);
    }
    uint32_t BlockHeightInTexels() const {
        DAWN_ASSERT(IsFormatSupported());
        return utils::GetTextureFormatBlockHeight(GetParam().mTextureFormat);
    }

    // Compute the upload data for the copyConfig.
    std::vector<uint8_t> UploadData(const CopyConfig& copyConfig) {
        uint32_t copyWidthInBlock = copyConfig.copyExtent3D.width / BlockWidthInTexels();
        uint32_t copyHeightInBlock = copyConfig.copyExtent3D.height / BlockHeightInTexels();
        uint32_t copyBytesPerRow = 0;
        if (copyConfig.bytesPerRowAlignment != 0) {
            copyBytesPerRow = copyConfig.bytesPerRowAlignment;
        } else {
            copyBytesPerRow = copyWidthInBlock *
                              utils::GetTexelBlockSizeInBytes(copyConfig.textureDescriptor.format);
        }
        uint32_t copyRowsPerImage = copyConfig.rowsPerImage;
        if (copyRowsPerImage == wgpu::kCopyStrideUndefined) {
            copyRowsPerImage = copyHeightInBlock;
        }
        uint32_t copyBytesPerImage = copyBytesPerRow * copyRowsPerImage;
        uint32_t uploadBufferSize = copyConfig.bufferOffset +
                                    copyBytesPerImage * copyConfig.copyExtent3D.depthOrArrayLayers;

        // Fill data with the pre-prepared one-block compressed texture data.
        std::vector<uint8_t> data(uploadBufferSize, 0);
        std::vector<uint8_t> oneBlockCompressedTextureData = GetOneBlockFormatTextureData();
        for (uint32_t layer = 0; layer < copyConfig.copyExtent3D.depthOrArrayLayers; ++layer) {
            for (uint32_t h = 0; h < copyHeightInBlock; ++h) {
                for (uint32_t w = 0; w < copyWidthInBlock; ++w) {
                    uint32_t uploadBufferOffset = copyConfig.bufferOffset +
                                                  copyBytesPerImage * layer + copyBytesPerRow * h +
                                                  oneBlockCompressedTextureData.size() * w;
                    std::memcpy(&data[uploadBufferOffset], oneBlockCompressedTextureData.data(),
                                oneBlockCompressedTextureData.size() * sizeof(uint8_t));
                }
            }
        }

        return data;
    }

    // Copy the compressed texture data into the destination texture as is specified in
    // copyConfig.
    void InitializeDataInCompressedTexture(wgpu::Texture compressedTexture,
                                           const CopyConfig& copyConfig) {
        DAWN_ASSERT(IsFormatSupported());

        std::vector<uint8_t> data = UploadData(copyConfig);

        // Copy texture data from a staging buffer to the destination texture.
        wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(device, data.data(), data.size(),
                                                                 wgpu::BufferUsage::CopySrc);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            stagingBuffer, copyConfig.bufferOffset, copyConfig.bytesPerRowAlignment,
            copyConfig.rowsPerImage);

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            compressedTexture, copyConfig.viewMipmapLevel, copyConfig.copyOrigin3D);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo,
                                    &copyConfig.copyExtent3D);
        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);
    }

    // Create the bind group that includes a texture and a sampler.
    wgpu::BindGroup CreateBindGroupForTest(wgpu::BindGroupLayout bindGroupLayout,
                                           wgpu::Texture compressedTexture,
                                           uint32_t baseArrayLayer = 0,
                                           uint32_t baseMipLevel = 0) {
        DAWN_ASSERT(IsFormatSupported());

        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.minFilter = wgpu::FilterMode::Nearest;
        samplerDesc.magFilter = wgpu::FilterMode::Nearest;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::TextureViewDescriptor textureViewDescriptor;
        textureViewDescriptor.format = GetParam().mTextureFormat;
        textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        textureViewDescriptor.baseMipLevel = baseMipLevel;
        textureViewDescriptor.baseArrayLayer = baseArrayLayer;
        textureViewDescriptor.arrayLayerCount = 1;
        textureViewDescriptor.mipLevelCount = 1;
        wgpu::TextureView textureView = compressedTexture.CreateView(&textureViewDescriptor);

        return utils::MakeBindGroup(device, bindGroupLayout, {{0, sampler}, {1, textureView}});
    }

    // Create a render pipeline for sampling from a texture 2d and rendering into the render target.
    wgpu::RenderPipeline CreateRenderPipelineForTestTex2D() {
        DAWN_ASSERT(IsFormatSupported());

        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            struct VertexOut {
                @location(0) texCoord : vec2 <f32>,
                @builtin(position) position : vec4f,
            }

            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                var pos = array(
                    vec2f(-3.0,  1.0),
                    vec2f( 3.0,  1.0),
                    vec2f( 0.0, -2.0)
                );
                var output : VertexOut;
                output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                output.texCoord = vec2f(output.position.x / 2.0, -output.position.y / 2.0) + vec2f(0.5, 0.5);
                return output;
            })");
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                return textureSample(texture0, sampler0, texCoord);
            })");
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        return device.CreateRenderPipeline(&renderPipelineDescriptor);
    }

    // Create a render pipeline for sampling from a texture cube and rendering into the render
    // target. Used for compatibility mode as cube texture cannot be bound as texture_2d_array.
    wgpu::RenderPipeline CreateRenderPipelineForTestCube() {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            struct VertexOut {
                @location(0) texCoord : vec2 <f32>,
                @builtin(position) position : vec4f,
            }

            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                var pos = array(
                    vec2f(-3.0,  1.0),
                    vec2f( 3.0,  1.0),
                    vec2f( 0.0, -2.0)
                );
                var output : VertexOut;
                output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                output.texCoord = vec2f(output.position.x / 2.0, -output.position.y / 2.0) + vec2f(0.5, 0.5);
                return output;
            })");
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture0 : texture_cube<f32>;
            @group(0) @binding(2) var<uniform> layer : u32;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                var st: vec2f = texCoord;
                st.y = 1. - st.y;
                st = st * 2. - 1.;
                var coords: vec3f;
                switch(layer) {
                    case 0u: { coords = vec3f(1., st.y, -st.x); } // Positive X
                    case 1u: { coords = vec3f(-1., st.y, st.x); } // Negative X
                    case 2u: { coords = vec3f(st.x, 1., -st.y); } // Positive Y
                    case 3u: { coords = vec3f(st.x, -1., st.y); } // Negative Y
                    case 4u: { coords = vec3f(st.x, st.y, 1.); }  // Positive Z
                    case 5u: { coords = vec3f(-st.x, st.y, -1.);} // Negative Z
                    default: { return vec4f(0.); } // Unreachable
                }
                return textureSample(texture0, sampler0, coords);
            })");
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        return device.CreateRenderPipeline(&renderPipelineDescriptor);
    }

    // Run the given render pipeline and bind group and verify the pixels in the render target.
    void VerifyCompressedTexturePixelValues(wgpu::RenderPipeline renderPipeline,
                                            wgpu::BindGroup bindGroup,
                                            const wgpu::Extent3D& renderTargetSize,
                                            const wgpu::Origin3D& expectedOrigin,
                                            const wgpu::Extent3D& expectedExtent,
                                            const std::vector<utils::RGBA8>& expected) {
        DAWN_ASSERT(IsFormatSupported());

        utils::BasicRenderPass renderPass =
            utils::CreateBasicRenderPass(device, renderTargetSize.width, renderTargetSize.height);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(renderPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Some mobile chipsets decode the compressed texture values with a lower precision, leading
        // to color channels that are off by one from the expected result. This check is given a
        // little bit of tolerance to account for it. See dawn:1562.
        EXPECT_TEXTURE_EQ(expected.data(), renderPass.color, {expectedOrigin.x, expectedOrigin.y},
                          {expectedExtent.width, expectedExtent.height},
                          /* level */ 0, /* aspect */ wgpu::TextureAspect::All,
                          /* bytesPerRow */ 0, /* Tolerance */ utils::RGBA8(1, 1, 1, 1));
    }

    // Run the tests that copies pre-prepared format data into a texture and verifies if we can
    // render correctly with the pixel values sampled from the texture.
    void TestCopyRegionIntoFormatTextures(const CopyConfig& config) {
        DAWN_ASSERT(IsFormatSupported());

        wgpu::Texture texture = CreateTextureWithCompressedData(config);

        VerifyTexture(config, texture);
    }

    void VerifyTexture(const CopyConfig& config, wgpu::Texture texture) {
        wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();

        wgpu::Extent3D virtualSizeAtLevel = GetVirtualSizeAtLevel(config);

        // The copy region may exceed the subresource size because of the required paddings, so we
        // should limit the size of the expectedData to make it match the real size of the render
        // target.
        wgpu::Extent3D noPaddingExtent3D = config.copyExtent3D;
        if (config.copyOrigin3D.x + config.copyExtent3D.width > virtualSizeAtLevel.width) {
            noPaddingExtent3D.width = virtualSizeAtLevel.width - config.copyOrigin3D.x;
        }
        if (config.copyOrigin3D.y + config.copyExtent3D.height > virtualSizeAtLevel.height) {
            noPaddingExtent3D.height = virtualSizeAtLevel.height - config.copyOrigin3D.y;
        }
        noPaddingExtent3D.depthOrArrayLayers = 1u;

        std::vector<utils::RGBA8> expectedData = GetExpectedData(noPaddingExtent3D);

        wgpu::Origin3D firstLayerCopyOrigin = {config.copyOrigin3D.x, config.copyOrigin3D.y, 0};
        for (uint32_t layer = config.copyOrigin3D.z;
             layer < config.copyOrigin3D.z + config.copyExtent3D.depthOrArrayLayers; ++layer) {
            wgpu::BindGroup bindGroup = CreateBindGroupForTest(
                renderPipeline.GetBindGroupLayout(0), texture, layer, config.viewMipmapLevel);
            VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, virtualSizeAtLevel,
                                               firstLayerCopyOrigin, noPaddingExtent3D,
                                               expectedData);
        }
    }

    // Create a texture and initialize it with the pre-prepared compressed texture data.
    wgpu::Texture CreateTextureWithCompressedData(CopyConfig config) {
        wgpu::Texture texture = device.CreateTexture(&config.textureDescriptor);
        InitializeDataInCompressedTexture(texture, config);
        return texture;
    }

    // Record a texture-to-texture copy command into command encoder without finishing the
    // encoding.
    void RecordTextureToTextureCopy(wgpu::CommandEncoder encoder,
                                    wgpu::Texture srcTexture,
                                    wgpu::Texture dstTexture,
                                    CopyConfig srcConfig,
                                    CopyConfig dstConfig) {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfoSrc = utils::CreateTexelCopyTextureInfo(
            srcTexture, srcConfig.viewMipmapLevel, srcConfig.copyOrigin3D);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfoDst = utils::CreateTexelCopyTextureInfo(
            dstTexture, dstConfig.viewMipmapLevel, dstConfig.copyOrigin3D);
        encoder.CopyTextureToTexture(&texelCopyTextureInfoSrc, &texelCopyTextureInfoDst,
                                     &dstConfig.copyExtent3D);
    }

    wgpu::Texture CreateTextureFromTexture(wgpu::Texture srcTexture,
                                           CopyConfig srcConfig,
                                           CopyConfig dstConfig) {
        wgpu::Texture dstTexture = device.CreateTexture(&dstConfig.textureDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        RecordTextureToTextureCopy(encoder, srcTexture, dstTexture, srcConfig, dstConfig);
        wgpu::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        return dstTexture;
    }

    // Return the pre-prepared one-block texture data.
    std::vector<uint8_t> GetOneBlockFormatTextureData() {
        switch (GetParam().mTextureFormat) {
            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. We specify the same compressed data in both sRGB and
            // non-sRGB tests, but the rendering result should be different because for sRGB
            // formats, the red, green, and blue components are converted from an sRGB color
            // space to a linear color space as part of filtering.
            case wgpu::TextureFormat::BC1RGBAUnorm:
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                return {0x0, 0xC0, 0x60, 0x6, 0x50, 0x50, 0x50, 0x50};
            case wgpu::TextureFormat::BC7RGBAUnorm:
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return {0x50, 0x18, 0xfc, 0xf, 0x0,  0x30, 0xe3, 0xe1,
                        0xe1, 0xe1, 0xc1, 0xf, 0xfc, 0xc0, 0xf,  0xfc};

            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. The pixels in the left side of the block all have an alpha
            // value equal to 0x88. We specify the same compressed data in both sRGB and
            // non-sRGB tests, but the rendering result should be different because for sRGB
            // formats, the red, green, and blue components are converted from an sRGB color
            // space to a linear color space as part of filtering, and any alpha component is
            // left unchanged.
            case wgpu::TextureFormat::BC2RGBAUnorm:
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                return {0x88, 0xFF, 0x88, 0xFF, 0x88, 0xFF, 0x88, 0xFF,
                        0x0,  0xC0, 0x60, 0x6,  0x50, 0x50, 0x50, 0x50};
            case wgpu::TextureFormat::BC3RGBAUnorm:
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                return {0x88, 0xFF, 0x40, 0x2, 0x24, 0x40, 0x2,  0x24,
                        0x0,  0xC0, 0x60, 0x6, 0x50, 0x50, 0x50, 0x50};

            // The expected data represents 4x4 pixel images with the left side red and the
            // right side black.
            case wgpu::TextureFormat::BC4RSnorm:
                return {0x7F, 0x0, 0x40, 0x2, 0x24, 0x40, 0x2, 0x24};
            case wgpu::TextureFormat::BC4RUnorm:
                return {0xFF, 0x0, 0x40, 0x2, 0x24, 0x40, 0x2, 0x24};

            // The expected data represents 4x4 pixel images with the left side red and the
            // right side green and was encoded with DirectXTex from Microsoft.
            case wgpu::TextureFormat::BC5RGSnorm:
                return {0x7f, 0x81, 0x40, 0x2,  0x24, 0x40, 0x2,  0x24,
                        0x7f, 0x81, 0x9,  0x90, 0x0,  0x9,  0x90, 0x0};
            case wgpu::TextureFormat::BC5RGUnorm:
                return {0xff, 0x0, 0x40, 0x2,  0x24, 0x40, 0x2,  0x24,
                        0xff, 0x0, 0x9,  0x90, 0x0,  0x9,  0x90, 0x0};
            case wgpu::TextureFormat::BC6HRGBFloat:
                return {0xe3, 0x1f, 0x0, 0x0,  0x0, 0xe0, 0x1f, 0x0,
                        0x0,  0xff, 0x0, 0xff, 0x0, 0xff, 0x0,  0xff};
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return {0xe3, 0x3d, 0x0, 0x0,  0x0, 0xe0, 0x3d, 0x0,
                        0x0,  0xff, 0x0, 0xff, 0x0, 0xff, 0x0,  0xff};

            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. We specify the same compressed data in both sRGB and
            // non-sRGB tests, but the rendering result should be different because for sRGB
            // formats, the red, green, and blue components are converted from an sRGB color
            // space to a linear color space as part of filtering.
            case wgpu::TextureFormat::ETC2RGB8Unorm:
            case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            case wgpu::TextureFormat::ETC2RGB8A1Unorm:
            case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
                return {0x4, 0xc0, 0xc0, 0x2, 0x0, 0xff, 0x0, 0x0};

            // The expected data represents 4x4 pixel images with the left side dark red and the
            // right side dark green. The pixels in the left side of the block all have an alpha
            // value equal to 0x88. We specify the same compressed data in both sRGB and
            // non-sRGB tests, but the rendering result should be different because for sRGB
            // formats, the red, green, and blue components are converted from an sRGB color
            // space to a linear color space as part of filtering, and any alpha component is
            // left unchanged.
            case wgpu::TextureFormat::ETC2RGBA8Unorm:
            case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
                return {0xc0, 0x78, 0x49, 0x24, 0x92, 0xff, 0xff, 0xff,
                        0x4,  0xc0, 0xc0, 0x2,  0x0,  0xff, 0x0,  0x0};

            // The expected data represents 4x4 pixel image with the left side red and the right
            // side black.
            case wgpu::TextureFormat::EACR11Unorm:
                return {0x84, 0x90, 0xff, 0xff, 0xff, 0x6d, 0xb6, 0xdb};
            case wgpu::TextureFormat::EACR11Snorm:
                return {0x2, 0x90, 0xff, 0xff, 0xff, 0x6d, 0xb6, 0xdb};

            // The expected data represents 4x4 pixel image with the left side red and the right
            // side green.
            case wgpu::TextureFormat::EACRG11Unorm:
                return {0x84, 0x90, 0xff, 0xff, 0xff, 0x6d, 0xb6, 0xdb,
                        0x84, 0x90, 0x6d, 0xb6, 0xdb, 0xff, 0xff, 0xff};
            case wgpu::TextureFormat::EACRG11Snorm:
                return {0x2, 0x90, 0xff, 0xff, 0xff, 0x6d, 0xb6, 0xdb,
                        0x2, 0x90, 0x6d, 0xb6, 0xdb, 0xff, 0xff, 0xff};

            // The expected data is a texel block of the corresponding size where the left width / 2
            // pixels are dark red with an alpha of 0x80 and the remaining (width - width / 2)
            // pixels are dark green. We specify the same compressed data in both sRGB and non-sRGB
            // tests, but the rendering result should be different because for sRGB formats, the
            // red, green, and blue components are converted from an sRGB color space to a linear
            // color space as part of filtering, and any alpha component is left unchanged.
            case wgpu::TextureFormat::ASTC4x4Unorm:
            case wgpu::TextureFormat::ASTC4x4UnormSrgb:
                return {0x13, 0x80, 0xe9, 0x1, 0x0, 0xe8, 0x1,  0x0,
                        0x0,  0xff, 0x1,  0x0, 0x0, 0x3f, 0xf0, 0x3};
            case wgpu::TextureFormat::ASTC5x4Unorm:
            case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            case wgpu::TextureFormat::ASTC5x5Unorm:
            case wgpu::TextureFormat::ASTC5x5UnormSrgb:
                return {0x83, 0x80, 0xe9, 0x1, 0x0,  0xe8, 0x1,  0x0,
                        0x0,  0xff, 0x1,  0x0, 0x80, 0x14, 0x90, 0x2};
            case wgpu::TextureFormat::ASTC6x5Unorm:
            case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            case wgpu::TextureFormat::ASTC6x6Unorm:
            case wgpu::TextureFormat::ASTC6x6UnormSrgb:
                return {0x2, 0x81, 0xe9, 0x1, 0x0, 0xe8, 0x1,  0x0,
                        0x0, 0xff, 0x1,  0x0, 0x0, 0x3f, 0xf0, 0x3};
            case wgpu::TextureFormat::ASTC8x5Unorm:
            case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            case wgpu::TextureFormat::ASTC8x6Unorm:
            case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            case wgpu::TextureFormat::ASTC8x8Unorm:
            case wgpu::TextureFormat::ASTC8x8UnormSrgb:
                return {0x6, 0x80, 0xe9, 0x1, 0x0,  0xe8, 0x1,  0x0,
                        0x0, 0xff, 0x1,  0x0, 0xff, 0x0,  0xff, 0x0};
            case wgpu::TextureFormat::ASTC10x5Unorm:
            case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            case wgpu::TextureFormat::ASTC10x6Unorm:
            case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            case wgpu::TextureFormat::ASTC10x8Unorm:
            case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            case wgpu::TextureFormat::ASTC10x10Unorm:
            case wgpu::TextureFormat::ASTC10x10UnormSrgb:
                return {0x6, 0x81, 0xe9, 0x1,  0x0, 0xe8, 0x1,  0x0,
                        0x0, 0xff, 0x1,  0xff, 0x3, 0xf0, 0x3f, 0x0};
            case wgpu::TextureFormat::ASTC12x10Unorm:
            case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            case wgpu::TextureFormat::ASTC12x12Unorm:
            case wgpu::TextureFormat::ASTC12x12UnormSrgb:
                return {0x4, 0x80, 0xe9, 0x1, 0x0, 0xe8, 0x1,  0x0,
                        0x0, 0xff, 0x1,  0x0, 0x0, 0x3f, 0xf0, 0x3};

            default:
                DAWN_UNREACHABLE();
                return {};
        }
    }

    // Return the texture data that is decoded from the result of GetOneBlockFormatTextureData
    // in RGBA8 formats. Since some compression methods may be lossy, we may use different colors
    // to test different formats.
    std::vector<utils::RGBA8> GetExpectedData(const wgpu::Extent3D& testRegion) {
        constexpr uint8_t kLeftAlpha = 0x88;
        constexpr uint8_t kRightAlpha = 0xFF;

        constexpr utils::RGBA8 kBCDarkRed(198, 0, 0, 255);
        constexpr utils::RGBA8 kBCDarkGreen(0, 207, 0, 255);
        constexpr utils::RGBA8 kBCDarkRedSRGB(144, 0, 0, 255);
        constexpr utils::RGBA8 kBCDarkGreenSRGB(0, 159, 0, 255);

        constexpr utils::RGBA8 kETC2DarkRed(204, 0, 0, 255);
        constexpr utils::RGBA8 kETC2DarkGreen(0, 204, 0, 255);
        constexpr utils::RGBA8 kETC2DarkRedSRGB(154, 0, 0, 255);
        constexpr utils::RGBA8 kETC2DarkGreenSRGB(0, 154, 0, 255);

        constexpr utils::RGBA8 kASTCDarkRed(244, 0, 0, 128);
        constexpr utils::RGBA8 kASTCDarkGreen(0, 244, 0, 255);
        constexpr utils::RGBA8 kASTCDarkRedSRGB(231, 0, 0, 128);
        constexpr utils::RGBA8 kASTCDarkGreenSRGB(0, 231, 0, 255);

        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::BC1RGBAUnorm:
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return FillExpectedData(testRegion, kBCDarkRed, kBCDarkGreen);

            case wgpu::TextureFormat::BC2RGBAUnorm:
            case wgpu::TextureFormat::BC3RGBAUnorm: {
                constexpr utils::RGBA8 kLeftColor = utils::RGBA8(kBCDarkRed.r, 0, 0, kLeftAlpha);
                constexpr utils::RGBA8 kRightColor =
                    utils::RGBA8(0, kBCDarkGreen.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return FillExpectedData(testRegion, kBCDarkRedSRGB, kBCDarkGreenSRGB);

            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            case wgpu::TextureFormat::BC3RGBAUnormSrgb: {
                constexpr utils::RGBA8 kLeftColor =
                    utils::RGBA8(kBCDarkRedSRGB.r, 0, 0, kLeftAlpha);
                constexpr utils::RGBA8 kRightColor =
                    utils::RGBA8(0, kBCDarkGreenSRGB.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case wgpu::TextureFormat::BC4RSnorm:
            case wgpu::TextureFormat::BC4RUnorm:
                return FillExpectedData(testRegion, utils::RGBA8::kRed, utils::RGBA8::kBlack);

            case wgpu::TextureFormat::BC5RGSnorm:
            case wgpu::TextureFormat::BC5RGUnorm:
            case wgpu::TextureFormat::BC6HRGBFloat:
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return FillExpectedData(testRegion, utils::RGBA8::kRed, utils::RGBA8::kGreen);

            case wgpu::TextureFormat::ETC2RGB8Unorm:
            case wgpu::TextureFormat::ETC2RGB8A1Unorm:
                return FillExpectedData(testRegion, kETC2DarkRed, kETC2DarkGreen);

            case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
                return FillExpectedData(testRegion, kETC2DarkRedSRGB, kETC2DarkGreenSRGB);

            case wgpu::TextureFormat::ETC2RGBA8Unorm: {
                constexpr utils::RGBA8 kLeftColor = utils::RGBA8(kETC2DarkRed.r, 0, 0, kLeftAlpha);
                constexpr utils::RGBA8 kRightColor =
                    utils::RGBA8(0, kETC2DarkGreen.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case wgpu::TextureFormat::ETC2RGBA8UnormSrgb: {
                constexpr utils::RGBA8 kLeftColor =
                    utils::RGBA8(kETC2DarkRedSRGB.r, 0, 0, kLeftAlpha);
                constexpr utils::RGBA8 kRightColor =
                    utils::RGBA8(0, kETC2DarkGreenSRGB.g, 0, kRightAlpha);
                return FillExpectedData(testRegion, kLeftColor, kRightColor);
            }

            case wgpu::TextureFormat::EACR11Unorm:
            case wgpu::TextureFormat::EACR11Snorm:
                return FillExpectedData(testRegion, utils::RGBA8::kRed, utils::RGBA8::kBlack);

            case wgpu::TextureFormat::EACRG11Unorm:
            case wgpu::TextureFormat::EACRG11Snorm:
                return FillExpectedData(testRegion, utils::RGBA8::kRed, utils::RGBA8::kGreen);

            case wgpu::TextureFormat::ASTC4x4Unorm:
            case wgpu::TextureFormat::ASTC5x4Unorm:
            case wgpu::TextureFormat::ASTC5x5Unorm:
            case wgpu::TextureFormat::ASTC6x5Unorm:
            case wgpu::TextureFormat::ASTC6x6Unorm:
            case wgpu::TextureFormat::ASTC8x5Unorm:
            case wgpu::TextureFormat::ASTC8x6Unorm:
            case wgpu::TextureFormat::ASTC8x8Unorm:
            case wgpu::TextureFormat::ASTC10x5Unorm:
            case wgpu::TextureFormat::ASTC10x6Unorm:
            case wgpu::TextureFormat::ASTC10x8Unorm:
            case wgpu::TextureFormat::ASTC10x10Unorm:
            case wgpu::TextureFormat::ASTC12x10Unorm:
            case wgpu::TextureFormat::ASTC12x12Unorm:
                return FillExpectedData(testRegion, kASTCDarkRed, kASTCDarkGreen);

            case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            case wgpu::TextureFormat::ASTC12x12UnormSrgb:
                return FillExpectedData(testRegion, kASTCDarkRedSRGB, kASTCDarkGreenSRGB);

            default:
                DAWN_UNREACHABLE();
                return {};
        }
    }

    std::vector<utils::RGBA8> FillExpectedData(const wgpu::Extent3D& testRegion,
                                               utils::RGBA8 leftColorInBlock,
                                               utils::RGBA8 rightColorInBlock) {
        DAWN_ASSERT(testRegion.depthOrArrayLayers == 1);

        std::vector<utils::RGBA8> expectedData(testRegion.width * testRegion.height,
                                               leftColorInBlock);
        for (uint32_t y = 0; y < testRegion.height; ++y) {
            for (uint32_t x = 0; x < testRegion.width; ++x) {
                if (x % BlockWidthInTexels() >= BlockWidthInTexels() / 2) {
                    expectedData[testRegion.width * y + x] = rightColorInBlock;
                }
            }
        }
        return expectedData;
    }

    // Returns a texture size given the number of texel blocks that should be tiled. For example,
    // if a texel block size is 5x4, then GetTextureSizeFromBlocks(2, 2) -> {10, 8, 1}.
    wgpu::Extent3D GetTextureSizeWithNumBlocks(uint32_t numBlockWidth,
                                               uint32_t numBlockHeight,
                                               uint32_t depthOrArrayLayers = 1) const {
        return {numBlockWidth * BlockWidthInTexels(), numBlockHeight * BlockHeightInTexels(),
                depthOrArrayLayers};
    }

    CopyConfig GetDefaultFullConfig(uint32_t depthOrArrayLayers = 1) const {
        DAWN_ASSERT(IsFormatSupported());

        CopyConfig config;
        config.textureDescriptor.format = GetParam().mTextureFormat;
        config.textureDescriptor.usage = kDefaultFormatTextureUsage;
        config.textureDescriptor.size = GetTextureSizeWithNumBlocks(
            kUnalignedBlockSize, kUnalignedBlockSize, depthOrArrayLayers);
        config.textureDescriptor.mipLevelCount = kMipmapLevelCount;
        config.viewMipmapLevel = kMipmapLevelCount - 1;

        const wgpu::Extent3D virtualSize = GetVirtualSizeAtLevel(config);
        DAWN_ASSERT(virtualSize.width % BlockWidthInTexels() != 0u);
        DAWN_ASSERT(virtualSize.height % BlockHeightInTexels() != 0u);

        return config;
    }

    CopyConfig GetDefaultSmallConfig(uint32_t depthOrArrayLayers = 1) const {
        DAWN_ASSERT(IsFormatSupported());

        CopyConfig config;
        config.textureDescriptor.format = GetParam().mTextureFormat;
        config.textureDescriptor.usage = kDefaultFormatTextureUsage;
        config.textureDescriptor.size = GetTextureSizeWithNumBlocks(2, 2, depthOrArrayLayers);
        return config;
    }

    CopyConfig GetDefaultSubresourceConfig(uint32_t depthOrArrayLayers = 1) const {
        DAWN_ASSERT(IsFormatSupported());

        CopyConfig config;
        config.textureDescriptor.format = GetParam().mTextureFormat;
        config.textureDescriptor.usage = kDefaultFormatTextureUsage;
        config.textureDescriptor.size =
            GetPhysicalSizeAtLevel(GetDefaultFullConfig(depthOrArrayLayers));
        config.viewMipmapLevel = config.textureDescriptor.mipLevelCount - 1;
        return config;
    }

    // Note: Compressed formats are only valid with 2D (array) textures.
    static wgpu::Extent3D GetVirtualSizeAtLevel(const CopyConfig& config) {
        return {config.textureDescriptor.size.width >> config.viewMipmapLevel,
                config.textureDescriptor.size.height >> config.viewMipmapLevel,
                config.textureDescriptor.size.depthOrArrayLayers};
    }

    wgpu::Extent3D GetPhysicalSizeAtLevel(const CopyConfig& config) const {
        wgpu::Extent3D sizeAtLevel = GetVirtualSizeAtLevel(config);
        sizeAtLevel.width = (sizeAtLevel.width + BlockWidthInTexels() - 1) / BlockWidthInTexels() *
                            BlockWidthInTexels();
        sizeAtLevel.height = (sizeAtLevel.height + BlockHeightInTexels() - 1) /
                             BlockHeightInTexels() * BlockHeightInTexels();
        return sizeAtLevel;
    }

    static constexpr wgpu::TextureUsage kDefaultFormatTextureUsage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;

    // We choose a prime that is greater than the current max texel dimension size as a multiplier
    // to compute test texture sizes so that we can be certain that its level 2 mipmap (x4)
    // cannot be a multiple of the dimension. This is useful for testing padding at the edges of
    // the mipmaps.
    static constexpr uint32_t kUnalignedBlockSize = 13;
    static constexpr uint32_t kMipmapLevelCount = 3;

    bool mIsFormatSupported = false;
};

// Test copying into the whole texture with 2x2 blocks and sampling from it.
TEST_P(CompressedTextureFormatTest, Basic) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.copyExtent3D = config.textureDescriptor.size;

    TestCopyRegionIntoFormatTextures(config);
}

// Test copying into the whole cube texture with 2x2 blocks and sampling from it.
// Made for compatibility mode.
TEST_P(CompressedTextureFormatTest, Cube) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());
    // TODO(crbug.com/dawn/2131): diagnose this failure on Win Angle D3D11
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    const wgpu::TextureFormat format = GetParam().mTextureFormat;

    // TODO(crbug.com/362762192): diagnose this failure on GLES with ASTC and ETC2 texture formats
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && (utils::IsASTCTextureFormat(format)) ||
                          utils::IsETC2TextureFormat(format));

    constexpr uint32_t kLayers = 6;
    CopyConfig config = GetDefaultSmallConfig(kLayers);
    config.copyExtent3D = config.textureDescriptor.size;
    config.bytesPerRowAlignment = Align(
        config.copyExtent3D.width / BlockWidthInTexels() * utils::GetTexelBlockSizeInBytes(format),
        kTextureBytesPerRowAlignment);
    config.rowsPerImage = kLayers;
    wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
    if (IsCompatibilityMode()) {
        textureBindingViewDimensionDesc.textureBindingViewDimension =
            wgpu::TextureViewDimension::Cube;
        config.textureDescriptor.nextInChain = &textureBindingViewDimensionDesc;

        wgpu::Texture texture = CreateTextureWithCompressedData(config);
        wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestCube();

        wgpu::Extent3D virtualSizeAtLevel = GetVirtualSizeAtLevel(config);

        // The copy region may exceed the subresource size because of the required paddings, so we
        // should limit the size of the expectedData to make it match the real size of the render
        // target.
        wgpu::Extent3D noPaddingExtent3D = config.copyExtent3D;
        if (config.copyOrigin3D.x + config.copyExtent3D.width > virtualSizeAtLevel.width) {
            noPaddingExtent3D.width = virtualSizeAtLevel.width - config.copyOrigin3D.x;
        }
        if (config.copyOrigin3D.y + config.copyExtent3D.height > virtualSizeAtLevel.height) {
            noPaddingExtent3D.height = virtualSizeAtLevel.height - config.copyOrigin3D.y;
        }
        noPaddingExtent3D.depthOrArrayLayers = 1u;

        std::vector<utils::RGBA8> expectedData = GetExpectedData(noPaddingExtent3D);

        wgpu::Origin3D firstLayerCopyOrigin = {config.copyOrigin3D.x, config.copyOrigin3D.y, 0};

        wgpu::SamplerDescriptor samplerDesc = {};
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::TextureViewDescriptor textureViewDescriptor = {};
        textureViewDescriptor.format = GetParam().mTextureFormat;
        textureViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
        textureViewDescriptor.baseMipLevel = config.viewMipmapLevel;
        textureViewDescriptor.mipLevelCount = 1;
        wgpu::TextureView textureView = texture.CreateView(&textureViewDescriptor);

        for (uint32_t layer = config.copyOrigin3D.z;
             layer < config.copyOrigin3D.z + config.copyExtent3D.depthOrArrayLayers; ++layer) {
            wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
                device, &layer, sizeof(uint32_t), wgpu::BufferUsage::Uniform);

            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                     {
                                         {0, sampler},
                                         {1, textureView},
                                         {2, uniformBuffer},
                                     });

            VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, virtualSizeAtLevel,
                                               firstLayerCopyOrigin, noPaddingExtent3D,
                                               expectedData);
        }
    } else {
        TestCopyRegionIntoFormatTextures(config);
    }
}

// Test copying into a sub-region of a texture works correctly.
TEST_P(CompressedTextureFormatTest, CopyIntoSubRegion) {
    // TODO(crbug.com/dawn/976): Failing on Linux Intel OpenGL drivers.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.copyOrigin3D = {BlockWidthInTexels(), BlockHeightInTexels(), 0};
    config.copyExtent3D = {BlockWidthInTexels(), BlockHeightInTexels(), 1};

    TestCopyRegionIntoFormatTextures(config);
}

// Test copying into the non-zero layer of a 2D array texture works correctly.
TEST_P(CompressedTextureFormatTest, CopyIntoNonZeroArrayLayer) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode cannot bind a 2d-array texture as 2d.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    constexpr uint32_t kArrayLayerCount = 3;

    CopyConfig config = GetDefaultSmallConfig(kArrayLayerCount);
    config.copyExtent3D = config.textureDescriptor.size;
    config.copyExtent3D.depthOrArrayLayers = 1;
    config.copyOrigin3D.z = kArrayLayerCount - 1;

    TestCopyRegionIntoFormatTextures(config);
}

// Test copying into a non-zero mipmap level of a texture.
TEST_P(CompressedTextureFormatTest, CopyBufferIntoNonZeroMipmapLevel) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // TODO(crbug.com/dawn/0000): diagnose this failure on QualComm OpenGL ES.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsOpenGLES() && IsQualcomm());

    CopyConfig config = GetDefaultFullConfig();
    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    config.copyExtent3D = GetPhysicalSizeAtLevel(config);

    TestCopyRegionIntoFormatTextures(config);
}

// Test texture-to-texture whole-size copies.
TEST_P(CompressedTextureFormatTest, CopyWholeTextureSubResourceIntoNonZeroMipmapLevel) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode does not support T2T copies of compressed textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/816): This consistently fails on with the 12th pixel being opaque
    // black instead of opaque red on Win10 FYI Release (NVIDIA GeForce GTX 1660).
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

    CopyConfig config = GetDefaultFullConfig();
    // Add the usage bit for both source and destination textures so that we don't need to
    // create two copy configs.
    config.textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                     wgpu::TextureUsage::TextureBinding;

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    const wgpu::Extent3D kVirtualSize = GetVirtualSizeAtLevel(config);
    config.copyExtent3D = GetPhysicalSizeAtLevel(config);

    wgpu::Texture textureSrc = CreateTextureWithCompressedData(config);

    // Create textureDst and copy from the content in textureSrc into it.
    wgpu::Texture textureDst = CreateTextureFromTexture(textureSrc, config, config);

    // Verify if we can use texture as sampled textures correctly.
    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();
    wgpu::BindGroup bindGroup =
        CreateBindGroupForTest(renderPipeline.GetBindGroupLayout(0), textureDst,
                               config.copyOrigin3D.z, config.viewMipmapLevel);

    std::vector<utils::RGBA8> expectedData = GetExpectedData(kVirtualSize);
    VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kVirtualSize, config.copyOrigin3D,
                                       kVirtualSize, expectedData);
}

// Test texture-to-texture partial copies where the physical size of the destination subresource is
// different from its virtual size.
TEST_P(CompressedTextureFormatTest, CopyIntoSubresourceWithPhysicalSizeNotEqualToVirtualSize) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode does not support T2T copies of compressed textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/817): add workaround on the T2T copies where Extent3D fits in one
    // subresource and does not fit in another one on OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL());

    CopyConfig srcConfig = GetDefaultSubresourceConfig();
    srcConfig.textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

    CopyConfig dstConfig = GetDefaultFullConfig();

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    const wgpu::Extent3D kDstVirtualSize = GetVirtualSizeAtLevel(dstConfig);
    const wgpu::Extent3D kDstPhysicalSize = GetPhysicalSizeAtLevel(dstConfig);
    srcConfig.copyExtent3D = dstConfig.copyExtent3D = kDstPhysicalSize;

    // Create textureSrc as the source texture and initialize it with pre-prepared compressed
    // data.
    wgpu::Texture textureSrc = CreateTextureWithCompressedData(srcConfig);
    wgpu::TexelCopyTextureInfo texelCopyTextureInfoSrc = utils::CreateTexelCopyTextureInfo(
        textureSrc, srcConfig.viewMipmapLevel, srcConfig.copyOrigin3D);

    // Create textureDst and copy from the content in textureSrc into it.
    wgpu::Texture textureDst = CreateTextureFromTexture(textureSrc, srcConfig, dstConfig);

    // Verify if we can use texture as sampled textures correctly.
    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();
    wgpu::BindGroup bindGroup =
        CreateBindGroupForTest(renderPipeline.GetBindGroupLayout(0), textureDst,
                               dstConfig.copyOrigin3D.z, dstConfig.viewMipmapLevel);

    std::vector<utils::RGBA8> expectedData = GetExpectedData(kDstVirtualSize);
    VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kDstVirtualSize,
                                       dstConfig.copyOrigin3D, kDstVirtualSize, expectedData);
}

// Test texture-to-texture partial copies where the physical size of the source subresource is
// different from its virtual size.
TEST_P(CompressedTextureFormatTest, CopyFromSubresourceWithPhysicalSizeNotEqualToVirtualSize) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode does not support T2T copies of compressed textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/817): add workaround on the T2T copies where Extent3D fits in one
    // subresource and does not fit in another one on OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL());

    CopyConfig srcConfig = GetDefaultFullConfig();
    srcConfig.textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

    CopyConfig dstConfig = GetDefaultSubresourceConfig();

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    const wgpu::Extent3D kSrcVirtualSize = GetVirtualSizeAtLevel(srcConfig);
    const wgpu::Extent3D kDstVirtualSize = GetVirtualSizeAtLevel(dstConfig);
    srcConfig.copyExtent3D = dstConfig.copyExtent3D = kDstVirtualSize;

    ASSERT_GT(srcConfig.copyOrigin3D.x + srcConfig.copyExtent3D.width, kSrcVirtualSize.width);
    ASSERT_GT(srcConfig.copyOrigin3D.y + srcConfig.copyExtent3D.height, kSrcVirtualSize.height);

    // Create textureSrc as the source texture and initialize it with pre-prepared compressed
    // data.
    wgpu::Texture textureSrc = CreateTextureWithCompressedData(srcConfig);

    // Create textureDst and copy from the content in textureSrc into it.
    wgpu::Texture textureDst = CreateTextureFromTexture(textureSrc, srcConfig, dstConfig);

    // Verify if we can use texture as sampled textures correctly.
    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();
    wgpu::BindGroup bindGroup =
        CreateBindGroupForTest(renderPipeline.GetBindGroupLayout(0), textureDst,
                               dstConfig.copyOrigin3D.z, dstConfig.viewMipmapLevel);

    std::vector<utils::RGBA8> expectedData = GetExpectedData(kDstVirtualSize);
    VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kDstVirtualSize,
                                       dstConfig.copyOrigin3D, kDstVirtualSize, expectedData);
}

// Test recording two texture-to-texture partial copies where the physical size of the source
// subresource is different from its virtual size into one command buffer.
TEST_P(CompressedTextureFormatTest, MultipleCopiesWithPhysicalSizeNotEqualToVirtualSize) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode does not support T2T copies of compressed textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/817): add workaround on the T2T copies where Extent3D fits in one
    // subresource and does not fit in another one on OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL());

    constexpr uint32_t kTotalCopyCount = 2;
    std::array<CopyConfig, kTotalCopyCount> srcConfigs;
    std::array<CopyConfig, kTotalCopyCount> dstConfigs;

    srcConfigs[0] = GetDefaultFullConfig();
    srcConfigs[0].textureDescriptor.usage =
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    dstConfigs[0] = GetDefaultSubresourceConfig();
    srcConfigs[0].copyExtent3D = dstConfigs[0].copyExtent3D = GetVirtualSizeAtLevel(dstConfigs[0]);

    srcConfigs[1] = GetDefaultSubresourceConfig();
    srcConfigs[1].textureDescriptor.usage =
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    dstConfigs[1] = GetDefaultFullConfig();
    srcConfigs[1].copyExtent3D = dstConfigs[1].copyExtent3D = GetVirtualSizeAtLevel(srcConfigs[1]);

    std::array<wgpu::Extent3D, kTotalCopyCount> dstVirtualSizes;
    for (uint32_t i = 0; i < kTotalCopyCount; ++i) {
        dstVirtualSizes[i] = GetVirtualSizeAtLevel(dstConfigs[i]);
    }

    std::array<wgpu::Texture, kTotalCopyCount> srcTextures;
    std::array<wgpu::Texture, kTotalCopyCount> dstTextures;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    for (uint32_t i = 0; i < kTotalCopyCount; ++i) {
        // Create srcTextures as the source textures and initialize them with pre-prepared
        // compressed data.
        srcTextures[i] = CreateTextureWithCompressedData(srcConfigs[i]);
        dstTextures[i] = device.CreateTexture(&dstConfigs[i].textureDescriptor);
        RecordTextureToTextureCopy(encoder, srcTextures[i], dstTextures[i], srcConfigs[i],
                                   dstConfigs[i]);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();

    for (uint32_t i = 0; i < kTotalCopyCount; ++i) {
        // Verify if we can use dstTextures as sampled textures correctly.
        wgpu::BindGroup bindGroup0 =
            CreateBindGroupForTest(renderPipeline.GetBindGroupLayout(0), dstTextures[i],
                                   dstConfigs[i].copyOrigin3D.z, dstConfigs[i].viewMipmapLevel);

        std::vector<utils::RGBA8> expectedData = GetExpectedData(dstVirtualSizes[i]);
        VerifyCompressedTexturePixelValues(renderPipeline, bindGroup0, dstVirtualSizes[i],
                                           dstConfigs[i].copyOrigin3D, dstVirtualSizes[i],
                                           expectedData);
    }
}

// A regression test for a bug for the toggle UseTemporaryBufferInCompressedTextureToTextureCopy on
// Vulkan backend: test texture-to-texture partial copies with multiple array layers where the
// physical size of the source subresource is different from its virtual size.
TEST_P(CompressedTextureFormatTest, CopyWithMultipleLayerAndPhysicalSizeNotEqualToVirtualSize) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode cannot bind a 2d-array texture as 2d.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/817): add workaround on the T2T copies where Extent3D fits in one
    // subresource and does not fit in another one on OpenGL.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL());

    constexpr uint32_t kArrayLayerCount = 5;

    CopyConfig srcConfig = GetDefaultFullConfig(kArrayLayerCount);
    srcConfig.textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

    CopyConfig dstConfig = GetDefaultSubresourceConfig(kArrayLayerCount);

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    const wgpu::Extent3D kSrcVirtualSize = GetVirtualSizeAtLevel(srcConfig);
    const wgpu::Extent3D kDstVirtualSize = GetVirtualSizeAtLevel(dstConfig);

    srcConfig.copyExtent3D = dstConfig.copyExtent3D = kDstVirtualSize;
    srcConfig.rowsPerImage = srcConfig.copyExtent3D.height / BlockHeightInTexels();
    ASSERT_GT(srcConfig.copyOrigin3D.x + srcConfig.copyExtent3D.width, kSrcVirtualSize.width);
    ASSERT_GT(srcConfig.copyOrigin3D.y + srcConfig.copyExtent3D.height, kSrcVirtualSize.height);

    const wgpu::TextureFormat format = GetParam().mTextureFormat;
    srcConfig.textureDescriptor.format = dstConfig.textureDescriptor.format = format;
    srcConfig.bytesPerRowAlignment = Align(srcConfig.copyExtent3D.width / BlockWidthInTexels() *
                                               utils::GetTexelBlockSizeInBytes(format),
                                           kTextureBytesPerRowAlignment);
    dstConfig.textureDescriptor.usage = kDefaultFormatTextureUsage;

    // Create textureSrc as the source texture and initialize it with pre-prepared compressed
    // data.
    wgpu::Texture textureSrc = CreateTextureWithCompressedData(srcConfig);

    // Create textureDst and copy from the content in textureSrc into it.
    wgpu::Texture textureDst = CreateTextureFromTexture(textureSrc, srcConfig, dstConfig);

    // We use the render pipeline to test if each layer can be correctly sampled with the
    // expected data.
    wgpu::RenderPipeline renderPipeline = CreateRenderPipelineForTestTex2D();

    const wgpu::Extent3D kExpectedDataRegionPerLayer = {kDstVirtualSize.width,
                                                        kDstVirtualSize.height, 1u};
    std::vector<utils::RGBA8> kExpectedDataPerLayer = GetExpectedData(kExpectedDataRegionPerLayer);
    const wgpu::Origin3D kCopyOriginPerLayer = {dstConfig.copyOrigin3D.x, dstConfig.copyOrigin3D.y,
                                                0};
    for (uint32_t copyLayer = 0; copyLayer < kArrayLayerCount; ++copyLayer) {
        wgpu::BindGroup bindGroup =
            CreateBindGroupForTest(renderPipeline.GetBindGroupLayout(0), textureDst,
                                   dstConfig.copyOrigin3D.z + copyLayer, dstConfig.viewMipmapLevel);

        VerifyCompressedTexturePixelValues(renderPipeline, bindGroup, kExpectedDataRegionPerLayer,
                                           kCopyOriginPerLayer, kExpectedDataRegionPerLayer,
                                           kExpectedDataPerLayer);
    }
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset and texture
// extent exactly fit the RowPitch.
TEST_P(CompressedTextureFormatTest, BufferOffsetAndExtentFitRowPitch) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.copyExtent3D = config.textureDescriptor.size;

    const wgpu::TextureFormat format = GetParam().mTextureFormat;
    const uint32_t blockCountPerRow = config.textureDescriptor.size.width / BlockWidthInTexels();
    const uint32_t blockSizeInBytes = utils::GetTexelBlockSizeInBytes(format);
    const uint32_t blockCountPerRowPitch = config.bytesPerRowAlignment / blockSizeInBytes;
    config.bufferOffset = (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes;

    TestCopyRegionIntoFormatTextures(config);
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset exceeds the
// slice pitch (slicePitch = bytesPerRow * (rowsPerImage / blockHeightInTexels)). On D3D12
// backend the texelOffset.y will be greater than 0 after calcuting the texelOffset in the function
// ComputeTexelOffsets().
TEST_P(CompressedTextureFormatTest, BufferOffsetExceedsSlicePitch) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.copyExtent3D = config.textureDescriptor.size;

    const wgpu::TextureFormat format = GetParam().mTextureFormat;
    const wgpu::Extent3D textureSizeLevel = config.textureDescriptor.size;
    const uint32_t blockCountPerRow = textureSizeLevel.width / BlockWidthInTexels();
    const uint32_t slicePitchInBytes =
        config.bytesPerRowAlignment * (textureSizeLevel.height / BlockHeightInTexels());
    const uint32_t blockSizeInBytes = utils::GetTexelBlockSizeInBytes(format);
    const uint32_t blockCountPerRowPitch = config.bytesPerRowAlignment / blockSizeInBytes;
    config.bufferOffset = (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes +
                          config.bytesPerRowAlignment + slicePitchInBytes;

    TestCopyRegionIntoFormatTextures(config);
}

// Test the special case of the B2T copies on the D3D12 backend that the buffer offset and texture
// extent exceed the RowPitch. On D3D12 backend two copies are required for this case.
TEST_P(CompressedTextureFormatTest, CopyWithBufferOffsetAndExtentExceedRowPitch) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    constexpr uint32_t kExceedRowBlockCount = 1;

    CopyConfig config = GetDefaultSmallConfig();
    config.copyExtent3D = config.textureDescriptor.size;

    const wgpu::TextureFormat format = GetParam().mTextureFormat;
    const uint32_t blockCountPerRow = config.textureDescriptor.size.width / BlockWidthInTexels();
    const uint32_t blockSizeInBytes = utils::GetTexelBlockSizeInBytes(format);
    const uint32_t blockCountPerRowPitch = config.bytesPerRowAlignment / blockSizeInBytes;
    config.bufferOffset =
        (blockCountPerRowPitch - blockCountPerRow + kExceedRowBlockCount) * blockSizeInBytes;

    TestCopyRegionIntoFormatTextures(config);
}

// Test the special case of the B2T copies on the D3D12 backend that the slicePitch is equal to the
// bytesPerRow. On D3D12 backend the texelOffset.z will be greater than 0 after calcuting the
// texelOffset in the function ComputeTexelOffsets().
TEST_P(CompressedTextureFormatTest, RowPitchEqualToSlicePitch) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.textureDescriptor.size = GetTextureSizeWithNumBlocks(2, 1);
    config.copyExtent3D = config.textureDescriptor.size;

    const wgpu::TextureFormat format = GetParam().mTextureFormat;
    const uint32_t blockCountPerRow = config.textureDescriptor.size.width / BlockWidthInTexels();
    const uint32_t slicePitchInBytes = config.bytesPerRowAlignment;
    const uint32_t blockSizeInBytes = utils::GetTexelBlockSizeInBytes(format);
    const uint32_t blockCountPerRowPitch = config.bytesPerRowAlignment / blockSizeInBytes;
    config.bufferOffset =
        (blockCountPerRowPitch - blockCountPerRow) * blockSizeInBytes + slicePitchInBytes;

    TestCopyRegionIntoFormatTextures(config);
}

// Test the workaround in the B2T copies when (bufferSize - bufferOffset < bytesPerImage *
// copyExtent.depthOrArrayLayers) on Metal backends. As copyExtent.depthOrArrayLayers can only be 1
// for compressed formats, on Metal backend we will use two copies to implement such copy.
TEST_P(CompressedTextureFormatTest, LargeImageHeight) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    CopyConfig config = GetDefaultSmallConfig();
    config.copyExtent3D = config.textureDescriptor.size;
    config.rowsPerImage = config.textureDescriptor.size.height * 2 / BlockHeightInTexels();

    TestCopyRegionIntoFormatTextures(config);
}

// Test the workaround in the B2T copies when (bufferSize - bufferOffset < bytesPerImage *
// copyExtent.depthOrArrayLayers) and copyExtent needs to be clamped.
TEST_P(CompressedTextureFormatTest, LargeImageHeightAndClampedCopyExtent) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // TODO(crbug.com/dawn/0000): diagnose this failure on QualComm OpenGL ES.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsOpenGLES() && IsQualcomm());

    CopyConfig config = GetDefaultFullConfig();

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies.
    const wgpu::Extent3D kPhysicalSize = GetPhysicalSizeAtLevel(config);
    config.copyExtent3D = kPhysicalSize;
    config.rowsPerImage = kPhysicalSize.height * 2 / BlockHeightInTexels();

    TestCopyRegionIntoFormatTextures(config);
}

// Test copying a whole 2D array texture with array layer count > 1 in one copy command works with
// compressed formats.
TEST_P(CompressedTextureFormatTest, CopyWhole2DArrayTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode does not support T2T copies of compressed textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    constexpr uint32_t kArrayLayerCount = 3;

    CopyConfig config = GetDefaultSmallConfig(kArrayLayerCount);
    config.rowsPerImage = 8;
    config.copyExtent3D = config.textureDescriptor.size;
    config.copyExtent3D.depthOrArrayLayers = kArrayLayerCount;

    TestCopyRegionIntoFormatTextures(config);
}

// Test copying a multiple 2D texture array layers in one copy command works.
TEST_P(CompressedTextureFormatTest, CopyMultiple2DArrayLayers) {
    DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

    // Compatibility mode cannot bind a 2d-array texture as 2d.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    constexpr uint32_t kArrayLayerCount = 3;

    CopyConfig config = GetDefaultSmallConfig(kArrayLayerCount);
    config.rowsPerImage = 8;

    constexpr uint32_t kCopyBaseArrayLayer = 1;
    constexpr uint32_t kCopyLayerCount = 2;
    config.copyOrigin3D = {0, 0, kCopyBaseArrayLayer};
    config.copyExtent3D = config.textureDescriptor.size;
    config.copyExtent3D.depthOrArrayLayers = kCopyLayerCount;

    TestCopyRegionIntoFormatTextures(config);
}

DAWN_INSTANTIATE_TEST_P(CompressedTextureFormatTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend(),
                         VulkanBackend({"use_temporary_buffer_in_texture_to_texture_copy"})},
                        std::vector<wgpu::TextureFormat>(utils::kCompressedFormats.begin(),
                                                         utils::kCompressedFormats.end()));

// Suite of regression tests that target specific compression types.
class CompressedTextureFormatSpecificTest : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsBCFormatSupported = SupportsFeatures({wgpu::FeatureName::TextureCompressionBC});

        std::vector<wgpu::FeatureName> features;
        if (mIsBCFormatSupported) {
            features.emplace_back(wgpu::FeatureName::TextureCompressionBC);
        }
        return features;
    }

    bool IsBCFormatSupported() const { return mIsBCFormatSupported; }

    bool mIsBCFormatSupported = false;
};

// Testing a special code path: clearing a non-renderable texture when DynamicUploader
// is unaligned doesn't throw validation errors.
TEST_P(CompressedTextureFormatSpecificTest, BC1RGBAUnorm_UnalignedDynamicUploader) {
    // Compatibility mode cannot bind a 2d-array texture as 2d.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // CopyT2B for compressed texture formats is unimplemented on OpenGL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(!IsBCFormatSupported());

    utils::UnalignDynamicUploader(device);

    wgpu::TextureDescriptor textureDescriptor = {};
    textureDescriptor.size = {4, 4, 1};
    textureDescriptor.format = wgpu::TextureFormat::BC1RGBAUnorm;
    textureDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 8;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(buffer, 0, 256);
    wgpu::Extent3D copyExtent = {4, 4, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copyExtent);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

DAWN_INSTANTIATE_TEST(CompressedTextureFormatSpecificTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"use_temporary_buffer_in_texture_to_texture_copy"}));

class CompressedTextureWriteTextureTest : public CompressedTextureFormatTest {
  protected:
    void SetUp() override {
        CompressedTextureFormatTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());
    }

    // Write the compressed texture data into the destination texture as is specified in
    // copyConfig.
    void WriteToCompressedTexture(wgpu::Texture compressedTexture, const CopyConfig& copyConfig) {
        DAWN_ASSERT(IsFormatSupported());

        std::vector<uint8_t> data = UploadData(copyConfig);

        wgpu::TexelCopyBufferLayout texelCopyBufferLayout = utils::CreateTexelCopyBufferLayout(
            copyConfig.bufferOffset, copyConfig.bytesPerRowAlignment, copyConfig.rowsPerImage);

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            compressedTexture, copyConfig.viewMipmapLevel, copyConfig.copyOrigin3D);

        queue.WriteTexture(&texelCopyTextureInfo, data.data(), data.size(), &texelCopyBufferLayout,
                           &copyConfig.copyExtent3D);
    }

    // Run the tests that write pre-prepared format data into a texture and verifies if we can
    // render correctly with the pixel values sampled from the texture.
    void TestWriteRegionIntoFormatTextures(const CopyConfig& config) {
        DAWN_ASSERT(IsFormatSupported());

        wgpu::Texture texture = device.CreateTexture(&config.textureDescriptor);
        WriteToCompressedTexture(texture, config);

        VerifyTexture(config, texture);
    }
};

// Test WriteTexture to a 2D texture with all parameters non-defaults.
TEST_P(CompressedTextureWriteTextureTest, Basic) {
    // TODO(crbug.com/dawn/976): Failing on Linux Intel OpenGL drivers.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    constexpr uint32_t kSizeWidthMultiplier = 5;
    constexpr uint32_t kSizeHeightMultiplier = 6;
    constexpr uint32_t kOriginWidthMultiplier = 1;
    constexpr uint32_t kOriginHeightMultiplier = 2;
    constexpr uint32_t kExtentWidthMultiplier = 3;
    constexpr uint32_t kExtentHeightMultiplier = 4;

    CopyConfig config;
    config.textureDescriptor.usage = kDefaultFormatTextureUsage;
    config.textureDescriptor.size = {BlockWidthInTexels() * kSizeWidthMultiplier,
                                     BlockHeightInTexels() * kSizeHeightMultiplier, 1};
    config.copyOrigin3D = {BlockWidthInTexels() * kOriginWidthMultiplier,
                           BlockHeightInTexels() * kOriginHeightMultiplier, 0};
    config.copyExtent3D = {BlockWidthInTexels() * kExtentWidthMultiplier,
                           BlockHeightInTexels() * kExtentHeightMultiplier, 1};
    config.bytesPerRowAlignment = 511;
    config.rowsPerImage = 5;
    config.textureDescriptor.format = GetParam().mTextureFormat;

    TestWriteRegionIntoFormatTextures(config);
}

// Test writing to multiple 2D texture array layers.
TEST_P(CompressedTextureWriteTextureTest, WriteMultiple2DArrayLayers) {
    // Compatibility mode cannot bind a 2d-array texture as 2d.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/976): Failing on Linux Intel OpenGL drivers.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // TODO(b/198674734): Width multiplier set to 7 because 5 results in square size for ASTC6x5.
    constexpr uint32_t kSizeWidthMultiplier = 7;
    constexpr uint32_t kSizeHeightMultiplier = 6;
    constexpr uint32_t kOriginWidthMultiplier = 1;
    constexpr uint32_t kOriginHeightMultiplier = 2;
    constexpr uint32_t kExtentWidthMultiplier = 3;
    constexpr uint32_t kExtentHeightMultiplier = 4;

    CopyConfig config;
    config.textureDescriptor.usage = kDefaultFormatTextureUsage;
    config.textureDescriptor.size = {BlockWidthInTexels() * kSizeWidthMultiplier,
                                     BlockHeightInTexels() * kSizeHeightMultiplier, 9};
    config.copyOrigin3D = {BlockWidthInTexels() * kOriginWidthMultiplier,
                           BlockHeightInTexels() * kOriginHeightMultiplier, 3};
    config.copyExtent3D = {BlockWidthInTexels() * kExtentWidthMultiplier,
                           BlockHeightInTexels() * kExtentHeightMultiplier, 6};
    config.bytesPerRowAlignment = 511;
    config.rowsPerImage = 5;
    config.textureDescriptor.format = GetParam().mTextureFormat;

    TestWriteRegionIntoFormatTextures(config);
}

// Test writing textures where the physical size of the destination subresource is different from
// its virtual size.
TEST_P(CompressedTextureWriteTextureTest,
       WriteIntoSubresourceWithPhysicalSizeNotEqualToVirtualSize) {
    // TODO(crbug.com/dawn/976): Failing on Linux Intel OpenGL drivers.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    // TODO(crbug.com/dawn/0000): diagnose this failure on QualComm OpenGL ES.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsOpenGLES() && IsQualcomm());

    CopyConfig config = GetDefaultFullConfig();

    // The virtual size of the texture at mipmap level == 2 is not a multiple of the texel
    // dimensions so paddings are required in the copies. We then test against the expected
    // physical size and a valid size smaller than the physical size for verification.
    const wgpu::Extent3D kPhysicalSize = GetPhysicalSizeAtLevel(config);
    for (unsigned int w : {kPhysicalSize.width - BlockWidthInTexels(), kPhysicalSize.width}) {
        for (unsigned int h :
             {kPhysicalSize.height - BlockHeightInTexels(), kPhysicalSize.height}) {
            config.copyExtent3D = {w, h, 1};
            TestWriteRegionIntoFormatTextures(config);
        }
    }
}

DAWN_INSTANTIATE_TEST_P(CompressedTextureWriteTextureTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kCompressedFormats.begin(),
                                                         utils::kCompressedFormats.end()));

}  // anonymous namespace
}  // namespace dawn
