// Copyright 2020 The Dawn & Tint Authors
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
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(DepthStencilCopyTestParams, TextureFormat);

constexpr std::array<wgpu::TextureFormat, 3> kValidDepthCopyTextureFormats = {
    wgpu::TextureFormat::Depth16Unorm,
    wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth32FloatStencil8,
};

constexpr std::array<wgpu::TextureFormat, 1> kValidDepthCopyFromBufferFormats = {
    wgpu::TextureFormat::Depth16Unorm,
};

uint32_t GetBytesPerPixel(wgpu::TextureFormat format, wgpu::TextureAspect aspect) {
    uint32_t bytesPerPixel = 0;
    switch (format) {
        case wgpu::TextureFormat::Depth24PlusStencil8: {
            DAWN_ASSERT(aspect == wgpu::TextureAspect::StencilOnly);
            bytesPerPixel = 1;
            break;
        }
        case wgpu::TextureFormat::Depth32FloatStencil8: {
            switch (aspect) {
                case wgpu::TextureAspect::DepthOnly:
                    bytesPerPixel = 4;
                    break;
                case wgpu::TextureAspect::StencilOnly:
                    bytesPerPixel = 1;
                    break;
                default:
                    DAWN_UNREACHABLE();
                    break;
            }
            break;
        }
        default:
            bytesPerPixel = utils::GetTexelBlockSizeInBytes(format);
            break;
    }
    return bytesPerPixel;
}

// Bytes of unorm 16 of 0.23 is 0x3AE1, of which the 2 bytes are different.
// This helps better test unorm 16 compute emulation path.
constexpr float kInitDepth = 0.23f;

// Bytes of unorm 16 of 0.23 is 0xB0A3.
// Use a non-zero clear depth to better test unorm16 compute emulation path.
constexpr float kClearDepth = 0.69f;

// Initialize other mip levels with differrent garbage values for better testing
constexpr float kGarbageDepth = 0.123456789f;

static_assert(kInitDepth != kGarbageDepth);
static_assert(kClearDepth != kGarbageDepth);

constexpr uint8_t kInitStencil = 1u;
constexpr uint8_t kClearStencil = 0u;
constexpr uint8_t kGarbageStencil = 99u;

static_assert(kInitStencil != kGarbageStencil);
static_assert(kClearStencil != kGarbageStencil);

class DepthStencilCopyTests : public DawnTestWithParams<DepthStencilCopyTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<DepthStencilCopyTestParams>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);

        // Skip formats other than Depth24PlusStencil8 if we're specifically testing with the packed
        // depth24_unorm_stencil8 toggle.
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("use_packed_depth24_unorm_stencil8_format") &&
                                 GetParam().mTextureFormat !=
                                     wgpu::TextureFormat::Depth24PlusStencil8);

        // Draw a square in the bottom left quarter of the screen.
        mVertexModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f( 0.0, -1.0),
                    vec2f(-1.0,  0.0),
                    vec2f(-1.0,  0.0),
                    vec2f( 0.0, -1.0),
                    vec2f( 0.0,  0.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth32FloatStencil8:
                if (SupportsFeatures({wgpu::FeatureName::Depth32FloatStencil8})) {
                    mIsFormatSupported = true;
                    return {wgpu::FeatureName::Depth32FloatStencil8};
                }
                return {};
            default:
                mIsFormatSupported = true;
                return {};
        }
    }

    bool IsValidDepthCopyTextureFormat() {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth16Unorm:
            case wgpu::TextureFormat::Depth32Float:
            case wgpu::TextureFormat::Depth32FloatStencil8:
                return true;
            default:
                return false;
        }
    }

    wgpu::Texture CreateTexture(
        uint32_t width,
        uint32_t height,
        wgpu::TextureUsage usage,
        uint32_t mipLevelCount = 1,
        uint32_t arrayLayerCount = 1,
        wgpu::TextureViewDimension bindingViewDimension = wgpu::TextureViewDimension::Undefined) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, arrayLayerCount};
        texDescriptor.format = GetParam().mTextureFormat;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        // Test cube texture copy for compat.
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (IsCompatibilityMode() &&
            bindingViewDimension != wgpu::TextureViewDimension::Undefined) {
            textureBindingViewDimensionDesc.textureBindingViewDimension = bindingViewDimension;
            texDescriptor.nextInChain = &textureBindingViewDimensionDesc;
        }
        return device.CreateTexture(&texDescriptor);
    }

    wgpu::Texture CreateDepthTexture(uint32_t width,
                                     uint32_t height,
                                     wgpu::TextureUsage usage,
                                     uint32_t mipLevelCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = GetParam().mTextureFormat;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        return device.CreateTexture(&texDescriptor);
    }

    // Initialize the depth/stencil values for the texture using a render pass.
    // The texture will be cleared to the "clear" values, and then bottom left corner will
    // be written with the "region" values.
    void InitializeDepthStencilTextureRegion(wgpu::Texture texture,
                                             float clearDepth,
                                             float regionDepth,
                                             uint8_t clearStencil,
                                             uint8_t regionStencil,
                                             uint32_t mipLevel = 0,
                                             uint32_t arrayLayer = 0,
                                             uint32_t arrayLayerCount = 1) {
        wgpu::TextureFormat format = GetParam().mTextureFormat;
        // Create the render pass used for the initialization.
        utils::ComboRenderPipelineDescriptor renderPipelineDesc;
        renderPipelineDesc.vertex.module = mVertexModule;
        renderPipelineDesc.cFragment.targetCount = 0;

        wgpu::DepthStencilState* depthStencil = renderPipelineDesc.EnableDepthStencil(format);

        if (utils::IsStencilOnlyFormat(format)) {
            depthStencil->depthCompare = wgpu::CompareFunction::Always;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                @fragment fn main() {}
            )");
        } else {
            depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, std::string(R"(
                @fragment fn main() -> @builtin(frag_depth) f32 {
                    return )" + std::to_string(regionDepth) + R"(;
                })")
                                                                                        .c_str());
        }
        if (!utils::IsDepthOnlyFormat(format)) {
            depthStencil->stencilFront.passOp = wgpu::StencilOperation::Replace;
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        for (uint32_t curArrayLayer = 0; curArrayLayer < arrayLayerCount; curArrayLayer++) {
            // Build the render pass used for initialization.
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.baseMipLevel = mipLevel;
            viewDesc.mipLevelCount = 1;
            viewDesc.baseArrayLayer = arrayLayer + curArrayLayer;
            viewDesc.arrayLayerCount = 1;

            utils::ComboRenderPassDescriptor renderPassDesc({}, texture.CreateView(&viewDesc));
            renderPassDesc.UnsetDepthStencilLoadStoreOpsForFormat(format);
            // TODO(dawn:1782): use different clear values for each array layer.
            renderPassDesc.cDepthStencilAttachmentInfo.depthClearValue = clearDepth;
            renderPassDesc.cDepthStencilAttachmentInfo.stencilClearValue = clearStencil;

            // Draw the quad (two triangles)
            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPassDesc);
            pass.SetPipeline(pipeline);
            pass.SetStencilReference(regionStencil);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::Texture CreateInitializeDepthStencilTextureAndCopyT2T(float clearDepth,
                                                                float regionDepth,
                                                                uint8_t clearStencil,
                                                                uint8_t regionStencil,
                                                                uint32_t width,
                                                                uint32_t height,
                                                                wgpu::TextureUsage usage,
                                                                uint32_t mipLevel = 0) {
        wgpu::Texture src = CreateTexture(
            width, height, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
            mipLevel + 1);

        wgpu::Texture dst =
            CreateTexture(width, height, usage | wgpu::TextureUsage::CopyDst, mipLevel + 1);

        InitializeDepthStencilTextureRegion(src, clearDepth, regionDepth, clearStencil,
                                            regionStencil, mipLevel);

        // Perform a T2T copy of all aspects
        {
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcView =
                utils::CreateTexelCopyTextureInfo(src, mipLevel, {0, 0, 0});
            wgpu::TexelCopyTextureInfo dstView =
                utils::CreateTexelCopyTextureInfo(dst, mipLevel, {0, 0, 0});
            wgpu::Extent3D copySize = {width >> mipLevel, height >> mipLevel, 1};
            commandEncoder.CopyTextureToTexture(&srcView, &dstView, &copySize);

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);
        }

        return dst;
    }

    uint32_t BufferSizeForTextureCopy(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm,
                                      wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                      bool alignForMultipleOf4Bytes = true) {
        uint32_t bytesPerPixel = GetBytesPerPixel(format, aspect);

        uint32_t bytesPerRow = Align(width * bytesPerPixel, kTextureBytesPerRowAlignment);

        // Bytes per image before last array layer
        uint32_t bytesPerImage = bytesPerRow * height;

        uint32_t result =
            bytesPerImage * (depth - 1) + (bytesPerRow * (height - 1) + width * bytesPerPixel);

        return alignForMultipleOf4Bytes ? Align(result, uint64_t(4)) : result;
    }

    wgpu::ShaderModule mVertexModule;

  private:
    bool mIsFormatSupported = false;
};

// Test copying both aspects in a T2T copy, then copying only stencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencil) {
    // TODO(crbug.com/344949343): Test fails on Intel D3D11 with packed depth24_unorm_stencil8.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsD3D11() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth24PlusStencil8 &&
                          HasToggleEnabled("use_packed_depth24_unorm_stencil8_format"));

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableStencil) {
    // Test fails on D3D11 with Intel when using packed depth24_unorm_stencil8 format.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsD3D11() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth24PlusStencil8 &&
                          HasToggleEnabled("use_packed_depth24_unorm_stencil8_format"));

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::CopySrc);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

// Test that part of a non-renderable, non-zero mip stencil aspect can be copied. Notably,
// this test has different behavior on some platforms than T2TBothAspectsThenCopyStencil.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonRenderableNonZeroMipStencil) {
    // Test fails on D3D11 with Intel when using packed depth24_unorm_stencil8 format.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsD3D11() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth24PlusStencil8 &&
                          HasToggleEnabled("use_packed_depth24_unorm_stencil8_format"));

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 9, 9, wgpu::TextureUsage::CopySrc, 1);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {4, 4}, 1,
                      wgpu::TextureAspect::StencilOnly);
}

// Test copying both aspects in a T2T copy, then copying only depth.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepth) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight, wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying only depth at a nonzero mip.
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyNonZeroMipDepth) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, 8, 8, wgpu::TextureUsage::RenderAttachment, 1);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, 4, 4, 0, 1,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying stencil, then copying depth
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyStencilThenDepth) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });
}

// Test copying both aspects in a T2T copy, then copying depth, then copying stencil
TEST_P(DepthStencilCopyTests, T2TBothAspectsThenCopyDepthThenStencil) {
    DAWN_TEST_UNSUPPORTED_IF(!IsValidDepthCopyTextureFormat());

    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    wgpu::Texture texture = CreateInitializeDepthStencilTextureAndCopyT2T(
        0.1f, 0.3f, 1u, 3u, kWidth, kHeight,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Check the depth
    ExpectAttachmentDepthTestData(texture, GetParam().mTextureFormat, kWidth, kHeight, 0, 0,
                                  {
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.1, 0.1, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                      0.3, 0.3, 0.1, 0.1,  //
                                  });

    // Check the stencil
    std::vector<uint8_t> expectedData = {
        1u, 1u, 1u, 1u,  //
        1u, 1u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
        3u, 3u, 1u, 1u,  //
    };
    EXPECT_TEXTURE_EQ(expectedData.data(), texture, {0, 0}, {kWidth, kHeight}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

class DepthCopyTests : public DepthStencilCopyTests {
  public:
    void DoCopyFromDepthTest(
        uint32_t bufferCopyOffset,
        uint32_t textureWidth,
        uint32_t textureHeight,
        uint32_t textureArrayLayerCount,
        uint32_t testLevel,
        // Test cube binding view dimension for compatibility mode
        wgpu::TextureViewDimension bindingViewDimension = wgpu::TextureViewDimension::Undefined) {
        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size =
            bufferCopyOffset +
            BufferSizeForTextureCopy(copyWidth, copyHeight, textureArrayLayerCount,
                                     GetParam().mTextureFormat, wgpu::TextureAspect::DepthOnly);
        wgpu::Buffer destinationBuffer = device.CreateBuffer(&bufferDescriptor);

        DoCopyFromDepthTestWithBuffer(destinationBuffer, bufferCopyOffset, textureWidth,
                                      textureHeight, textureArrayLayerCount, testLevel, true,
                                      bindingViewDimension);
    }

    void DoCopyFromDepthTestWithBuffer(
        wgpu::Buffer destinationBuffer,
        uint32_t bufferCopyOffset,
        uint32_t textureWidth,
        uint32_t textureHeight,
        uint32_t textureArrayLayerCount,
        uint32_t testLevel,
        bool checkBufferContent,
        // Test cube binding view dimension for compatibility mode
        wgpu::TextureViewDimension bindingViewDimension = wgpu::TextureViewDimension::Undefined) {
        uint32_t mipLevelCount = testLevel + 1;
        wgpu::Texture texture =
            CreateTexture(textureWidth, textureHeight,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                          mipLevelCount, textureArrayLayerCount, bindingViewDimension);

        for (uint32_t level = 0; level < mipLevelCount; level++) {
            float regionDepth = (level == testLevel) ? kInitDepth : kGarbageDepth;
            InitializeDepthStencilTextureRegion(texture, kClearDepth, regionDepth, 0, 0, level, 0,
                                                textureArrayLayerCount);
        }

        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        wgpu::Extent3D copySize = {copyWidth, copyHeight, textureArrayLayerCount};

        wgpu::TextureFormat format = GetParam().mTextureFormat;
        constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::DepthOnly;
        uint32_t bytesPerPixel = GetBytesPerPixel(format, aspect);
        uint32_t bytesPerRow = Align(copyWidth * bytesPerPixel, kTextureBytesPerRowAlignment);
        uint32_t bytesPerImage = bytesPerRow * copyHeight;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, testLevel, {0, 0, 0}, aspect);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            destinationBuffer, bufferCopyOffset, bytesPerRow, copyHeight);
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        if (checkBufferContent) {
            // Expected data pattern is that kInitDepth value at bottom left corner, while other
            // region is kClearDepth. Data of each layer is the same.
            if (format == wgpu::TextureFormat::Depth16Unorm) {
                uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
                uint16_t cleared = FloatToUnorm<uint16_t>(kClearDepth);
                std::vector<uint16_t> expectedData(copyWidth * copyHeight, cleared);
                for (uint32_t y = copyHeight / 2; y < copyHeight; y++) {
                    auto rowStart = expectedData.data() + y * copyWidth;
                    std::fill(rowStart, rowStart + copyWidth / 2, expected);
                }

                for (uint32_t z = 0; z < textureArrayLayerCount; ++z) {
                    uint32_t bufferOffsetPerArrayLayer = bytesPerImage * z;
                    for (uint32_t y = 0; y < copyHeight; ++y) {
                        EXPECT_BUFFER_U16_RANGE_EQ(
                            expectedData.data() + copyWidth * y, destinationBuffer,
                            bufferCopyOffset + bufferOffsetPerArrayLayer + y * bytesPerRow,
                            copyWidth);
                    }
                }
            } else {
                std::vector<float> expectedData(copyWidth * copyHeight, kClearDepth);
                for (uint32_t y = copyHeight / 2; y < copyHeight; y++) {
                    auto rowStart = expectedData.data() + y * copyWidth;
                    std::fill(rowStart, rowStart + copyWidth / 2, kInitDepth);
                }

                for (uint32_t z = 0; z < textureArrayLayerCount; ++z) {
                    uint32_t bufferOffsetPerArrayLayer = bytesPerImage * z;
                    for (uint32_t y = 0; y < copyHeight; ++y) {
                        EXPECT_BUFFER_FLOAT_RANGE_EQ(
                            expectedData.data() + copyWidth * y, destinationBuffer,
                            bufferCopyOffset + bufferOffsetPerArrayLayer + y * bytesPerRow,
                            copyWidth);
                    }
                }
            }
        }
    }
};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromDepthAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Only 1 pixel at bottom left has value, test compute emulation path for unorm 16
        {2, 2},
        // Odd number texture width to test compute emulation path for unorm 16
        {3, 3},
        // float 32 needs bytesPerRow alignment
        {65, 1},
        // unorm 16 and float 32 need bytesPerRow alignment
        {129, 1},
    };
    constexpr uint32_t kTestTextureArrayLayerCounts[] = {
        1,
        2,
    };

    for (const uint32_t sizeZ : kTestTextureArrayLayerCounts) {
        for (const auto& size : kTestTextureSizes) {
            DoCopyFromDepthTest(kBufferCopyOffset, size[0], size[1], sizeZ, kTestLevel);
        }
    }
}

// Test copying the depth-only aspect into a buffer at a non-zero offset.
TEST_P(DepthCopyTests, FromDepthAspectToBufferAtNonZeroOffset) {
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kBufferCopyOffsets[] = {4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Only 1 pixel at bottom left has value, test compute emulation path for unorm 16
        {2, 2},
        // Odd number texture width to test compute emulation path for unorm 16
        {3, 3},
        // float 32 needs bytesPerRow alignment
        {65, 1},
        // unorm 16 and float 32 need bytesPerRow alignment
        {129, 1},
    };
    constexpr uint32_t kTestTextureArrayLayerCounts[] = {
        1,
        2,
    };
    for (uint32_t offset : kBufferCopyOffsets) {
        for (const uint32_t sizeZ : kTestTextureArrayLayerCounts) {
            for (const auto& size : kTestTextureSizes) {
                DoCopyFromDepthTest(offset, size[0], size[1], sizeZ, kTestLevel);
            }
        }
    }
}

// Test copying the non-zero mip, depth-only aspect into a buffer.
TEST_P(DepthCopyTests, FromNonZeroMipDepthAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    DoCopyFromDepthTest(kBufferCopyOffset, kWidth, kHeight, 1, 1);
    DoCopyFromDepthTest(kBufferCopyOffset, kWidth, kHeight, 2, 2);
}

// Test buffer content outside of copy extent is preserved.
// This test is made specifially for compute blit for depth16unorm emulation path.
// The texel size is 2 byte, while in the compute shader we have to write 4 byte at a time.
// When the copy extent width is an odd number, buffer content outside of the copy range is
// inevitably written. So we need to make sure the original content of the buffer that's outside of
// the copy extent is still correctly preserved.
TEST_P(DepthCopyTests, PreserveBufferContent) {
    constexpr uint32_t kBufferCopyOffsets[] = {0u, 4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        {1, 1},
        {1, 2},
        {3, 3},
    };
    constexpr uint32_t kExtraBufferSize[] = {0u, 4u};
    const uint32_t kSizeZ = 1;
    constexpr uint32_t kTestLevel = 0;

    wgpu::TextureFormat format = GetParam().mTextureFormat;
    constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::DepthOnly;

    for (uint32_t extraBufferSize : kExtraBufferSize) {
        for (uint32_t offset : kBufferCopyOffsets) {
            for (const auto& size : kTestTextureSizes) {
                wgpu::BufferDescriptor bufferDescriptor = {};
                // Add wgpu::BufferUsage::MapRead to check the buffer content with mapAsync
                bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
                bufferDescriptor.size =
                    extraBufferSize + offset +
                    BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect);
                bufferDescriptor.mappedAtCreation = true;
                wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);
                constexpr uint8_t kOriginalValue = 0xff;
                {
                    // Fill the buffer with an original value other than 0 to check they are
                    // incorrectly overwritten outside of the copy range.
                    uint8_t* ptr = static_cast<uint8_t*>(buffer.GetMappedRange());
                    std::fill(ptr, ptr + bufferDescriptor.size, kOriginalValue);
                    buffer.Unmap();
                }

                // Don't check copy region content because the buffer doesn't have
                // wgpu::BufferUsage::CopySrc usage.
                DoCopyFromDepthTestWithBuffer(buffer, offset, size[0], size[1], kSizeZ, kTestLevel,
                                              false);

                std::vector<uint8_t> expected(bufferDescriptor.size, kOriginalValue);
                // Get the offset of the end of the copy range (without aligning with 4 bytes)
                uint32_t bufferEndOffset =
                    offset +
                    BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect, false);
                if (bufferDescriptor.size > bufferEndOffset) {
                    // Cannot use EXPECT_BUFFER_* helper here because it needs to align the copy
                    // size to a multiple of 4 bytes to call CopyBufferToBuffer. We are checking
                    // against format like Depth16Unorm so we need offset to be multiple of 2.
                    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, wgpu::kWholeMapSize);
                    const uint8_t* ptr = static_cast<const uint8_t*>(buffer.GetConstMappedRange());

                    // Check the content before copy range.
                    for (uint32_t i = 0; i < offset; i++) {
                        EXPECT_EQ(ptr[i], kOriginalValue);
                    }

                    // Check the content after copy range.
                    uint32_t checkSize = bufferDescriptor.size - bufferEndOffset;
                    for (uint32_t i = 0; i < checkSize; i++) {
                        EXPECT_EQ(ptr[bufferEndOffset + i], kOriginalValue);
                    }
                    buffer.Unmap();
                }
            }
        }
    }
}

// Test compact buffer size edge case.
// This test is made specifially for compute blit for depth16unorm emulation path.
// When format is depth16unorm and width is an odd number, the size of the most compact buffer copy
// target can be something that's not a multiple of 4. We need to make sure access don't go out of
// bounds in the shader, when still writing to array<u32> in the compute shader.
TEST_P(DepthCopyTests, BufferCopySizeEdgeCase) {
    constexpr uint32_t kBufferCopyOffsets[] = {0u, 4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Cannot create compact copy buffer for {1, 1} here as storage buffer binding requires size
        // of at least 4 bytes.
        {3, 1},
        {1, 2},
        {3, 3},
    };
    const uint32_t kSizeZ = 1;
    constexpr uint32_t kTestLevel = 0;

    wgpu::TextureFormat format = GetParam().mTextureFormat;
    constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::DepthOnly;

    for (uint32_t offset : kBufferCopyOffsets) {
        for (const auto& size : kTestTextureSizes) {
            wgpu::BufferDescriptor bufferDescriptor = {};
            // Add wgpu::BufferUsage::MapRead to check the buffer content with mapAsync
            bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
            bufferDescriptor.size =
                offset +
                // Don't align for 4 bytes to get the smallest possible buffer for depth16unorm.
                BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect, false);
            wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

            // Don't check copy region content because the buffer doesn't have
            // wgpu::BufferUsage::CopySrc usage.
            DoCopyFromDepthTestWithBuffer(buffer, offset, size[0], size[1], kSizeZ, kTestLevel,
                                          false);

            // Unable to check the result since either MapAsync and CopyBufferToBuffer requires size
            // to be multiple of 4 bytes.
            // Just run and don't crash on DAWN_ASSERT.
        }
    }
}

class DepthCopyTests_Compat : public DepthCopyTests {
    void SetUp() override {
        DepthCopyTests::SetUp();
        DAWN_SUPPRESS_TEST_IF(!IsCompatibilityMode());
    }

  public:
    static constexpr uint32_t kCubeTextureLayerCount = 6;
};

// Test copying the depth-only aspect into a buffer.
TEST_P(DepthCopyTests_Compat, FromDepthAspect_Cube) {
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Only 1 pixel at bottom left has value, test compute emulation path for unorm 16
        {2, 2},
        // Odd number texture width to test compute emulation path for unorm 16
        {3, 3},
        // unorm 16 and float 32 need bytesPerRow alignment
        {129, 129},
    };

    for (const auto& size : kTestTextureSizes) {
        DoCopyFromDepthTest(kBufferCopyOffset, size[0], size[1], kCubeTextureLayerCount, kTestLevel,
                            wgpu::TextureViewDimension::Cube);
    }
}

// Test copying the depth-only aspect into a buffer at a non-zero offset.
TEST_P(DepthCopyTests_Compat, FromDepthAspectToBufferAtNonZeroOffset) {
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kBufferCopyOffsets[] = {4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Only 1 pixel at bottom left has value, test compute emulation path for unorm 16
        {2, 2},
        // Odd number texture width to test compute emulation path for unorm 16
        {3, 3},
        // unorm 16 and float 32 need bytesPerRow alignment
        {129, 129},
    };
    for (uint32_t offset : kBufferCopyOffsets) {
        for (const auto& size : kTestTextureSizes) {
            DoCopyFromDepthTest(offset, size[0], size[1], kCubeTextureLayerCount, kTestLevel,
                                wgpu::TextureViewDimension::Cube);
        }
    }
}

// Test copying the non-zero mip, depth-only aspect into a buffer.
TEST_P(DepthCopyTests_Compat, FromNonZeroMipDepthAspect) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    DoCopyFromDepthTest(kBufferCopyOffset, kWidth, kHeight, kCubeTextureLayerCount, 1,
                        wgpu::TextureViewDimension::Cube);
    DoCopyFromDepthTest(kBufferCopyOffset, kWidth, kHeight, kCubeTextureLayerCount, 2,
                        wgpu::TextureViewDimension::Cube);
}

class DepthCopyFromBufferTests : public DepthStencilCopyTests {
  public:
    void DoTest(uint32_t bufferCopyOffset, bool hasRenderAttachmentUsage) {
        constexpr uint32_t kWidth = 8;
        constexpr uint32_t kHeight = 1;

        wgpu::TextureUsage textureUsage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        // On D3D12 backend the resource flag D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL is set only
        // when wgpu::TextureUsage::RenderAttachment is set on the creation of a depth stencil
        // texture.
        if (hasRenderAttachmentUsage) {
            textureUsage |= wgpu::TextureUsage::RenderAttachment;
        }
        wgpu::Texture destTexture = CreateTexture(kWidth, kHeight, textureUsage);

        wgpu::BufferDescriptor descriptor;
        descriptor.size = bufferCopyOffset +
                          BufferSizeForTextureCopy(kWidth, kHeight, 1, GetParam().mTextureFormat);
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        descriptor.mappedAtCreation = true;
        wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

        constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(srcBuffer, bufferCopyOffset, kBytesPerRow, kHeight);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            destTexture, 0, {0, 0, 0}, wgpu::TextureAspect::DepthOnly);
        wgpu::Extent3D extent = {kWidth, kHeight, 1};

        // This expectation is the test as it performs the CopyTextureToBuffer.
        if (GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm) {
            uint16_t expected = FloatToUnorm<uint16_t>(kInitDepth);
            std::vector<uint16_t> expectedData = {
                0, 0, expected, expected, 0, 0, expected, expected,
            };
            size_t expectedSize = expectedData.size() * sizeof(uint16_t);
            memcpy(srcBuffer.GetMappedRange(bufferCopyOffset, expectedSize), expectedData.data(),
                   expectedSize);
            srcBuffer.Unmap();

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &extent);
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(expectedData.data(), destTexture, {0, 0}, {kWidth, kHeight}, 0,
                              wgpu::TextureAspect::DepthOnly);
        } else {
            std::vector<float> expectedData = {
                0.0, 0.0, kInitDepth, kInitDepth, 0.0, 0.0, kInitDepth, kInitDepth,
            };
            size_t expectedSize = expectedData.size() * sizeof(float);

            memcpy(srcBuffer.GetMappedRange(bufferCopyOffset, expectedSize), expectedData.data(),
                   expectedSize);
            srcBuffer.Unmap();

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &extent);
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(expectedData.data(), destTexture, {0, 0}, {kWidth, kHeight}, 0,
                              wgpu::TextureAspect::DepthOnly);
        }
    }
};

// Test copying the depth-only aspect from a buffer.
TEST_P(DepthCopyFromBufferTests, BufferToDepthAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr bool kIsRenderable = false;
    DoTest(kBufferCopyOffset, kIsRenderable);
}

// Test copying the depth-only aspect from a buffer at a non-zero offset.
TEST_P(DepthCopyFromBufferTests, BufferToNonRenderableDepthAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    constexpr bool kIsRenderable = false;
    for (uint32_t offset : kBufferCopyOffsets) {
        DoTest(offset, kIsRenderable);
    }
}

// Test copying the depth-only aspect from a buffer at a non-zero offset.
TEST_P(DepthCopyFromBufferTests, BufferToRenderableDepthAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    constexpr bool kIsRenderable = true;
    for (uint32_t offset : kBufferCopyOffsets) {
        DoTest(offset, kIsRenderable);
    }
}

class StencilCopyTests : public DepthStencilCopyTests {
  public:
    void DoCopyFromStencilTest(
        uint32_t bufferCopyOffset,
        uint32_t textureWidth,
        uint32_t textureHeight,
        uint32_t textureArrayLayerCount,
        uint32_t testLevel,
        // Test cube binding view dimension for compatibility mode
        wgpu::TextureViewDimension bindingViewDimension = wgpu::TextureViewDimension::Undefined) {
        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        bufferDescriptor.size =
            bufferCopyOffset +
            BufferSizeForTextureCopy(copyWidth, copyHeight, textureArrayLayerCount,
                                     GetParam().mTextureFormat, wgpu::TextureAspect::StencilOnly);
        wgpu::Buffer destinationBuffer = device.CreateBuffer(&bufferDescriptor);

        DoCopyFromStencilTestWithBuffer(destinationBuffer, bufferCopyOffset, textureWidth,
                                        textureHeight, textureArrayLayerCount, testLevel, true);
    }
    void DoCopyFromStencilTestWithBuffer(wgpu::Buffer destinationBuffer,
                                         uint32_t bufferCopyOffset,
                                         uint32_t textureWidth,
                                         uint32_t textureHeight,
                                         uint32_t textureArrayLayerCount,
                                         uint32_t testLevel,
                                         bool checkBufferContent) {
        uint32_t mipLevelCount = testLevel + 1;
        wgpu::Texture depthStencilTexture =
            CreateTexture(textureWidth, textureHeight,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                          mipLevelCount, textureArrayLayerCount);

        for (uint32_t level = 0; level < mipLevelCount; level++) {
            uint8_t regionStencil = (level == testLevel) ? kInitStencil : kGarbageStencil;
            InitializeDepthStencilTextureRegion(depthStencilTexture, 0.f, 0.3f, kClearStencil,
                                                regionStencil, testLevel, 0,
                                                textureArrayLayerCount);
        }

        uint32_t copyWidth = textureWidth >> testLevel;
        uint32_t copyHeight = textureHeight >> testLevel;
        wgpu::Extent3D copySize = {copyWidth, copyHeight, textureArrayLayerCount};

        // Expected data pattern is that kInitStencil value at bottom left corner, while other
        // region is kClearStencil.

        wgpu::TextureFormat format = GetParam().mTextureFormat;
        constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::StencilOnly;
        uint32_t bytesPerPixel = GetBytesPerPixel(format, aspect);
        uint32_t bytesPerRow = Align(copyWidth * bytesPerPixel, kTextureBytesPerRowAlignment);
        uint32_t bytesPerImage = bytesPerRow * copyHeight;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(depthStencilTexture, testLevel, {0, 0, 0}, aspect);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            destinationBuffer, bufferCopyOffset, bytesPerRow, copyHeight);
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        if (checkBufferContent) {
            std::vector<uint8_t> expectedData(copyWidth * copyHeight, kClearStencil);
            // std::fill(expectedData.data(), expectedData.data() + expectedData.size(), 0x77);
            for (uint32_t y = copyHeight / 2; y < copyHeight; y++) {
                auto rowStart = expectedData.data() + y * copyWidth;
                std::fill(rowStart, rowStart + copyWidth / 2, kInitStencil);
            }

            for (uint32_t z = 0; z < textureArrayLayerCount; ++z) {
                uint32_t bufferOffsetPerArrayLayer = bytesPerImage * z;
                for (uint32_t y = 0; y < copyHeight; ++y) {
                    EXPECT_BUFFER_U8_RANGE_EQ(
                        expectedData.data() + copyWidth * y, destinationBuffer,
                        bufferCopyOffset + bufferOffsetPerArrayLayer + y * bytesPerRow, copyWidth);
                }
            }
        }
    }

    void DoCopyToStencilTest(uint32_t bufferCopyOffset) {
        // Create a stencil texture
        constexpr uint32_t kWidth = 4;
        constexpr uint32_t kHeight = 4;
        const bool hasDepth = !utils::IsStencilOnlyFormat(GetParam().mTextureFormat);

        wgpu::Texture depthStencilTexture =
            CreateTexture(kWidth, kHeight,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                              wgpu::TextureUsage::CopyDst);

        if (hasDepth) {
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

            // Clear depth to 0.7, so we can check that the stencil copy doesn't mutate the depth.
            utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
            passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mTextureFormat);
            passDescriptor.cDepthStencilAttachmentInfo.depthClearValue = 0.7;

            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
            pass.End();

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);
        }

        std::vector<uint8_t> stencilData = {
            1u,  2u,  3u,  4u,   //
            5u,  6u,  7u,  8u,   //
            9u,  10u, 11u, 12u,  //
            13u, 14u, 15u, 16u,  //
        };

        // After copying stencil data in, we will decrement stencil values in the bottom left
        // of the screen. This is the expected result.
        std::vector<uint8_t> expectedStencilData = {
            1u,  2u,  3u,  4u,   //
            5u,  6u,  7u,  8u,   //
            8u,  9u,  11u, 12u,  //
            12u, 13u, 15u, 16u,  //
        };

        // Upload the stencil data.
        {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = bufferCopyOffset + BufferSizeForTextureCopy(
                                                     kWidth, kHeight, 1, GetParam().mTextureFormat,
                                                     wgpu::TextureAspect::StencilOnly);
            descriptor.usage = wgpu::BufferUsage::CopySrc;
            descriptor.mappedAtCreation = true;
            wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);
            uint8_t* mappedPtr = static_cast<uint8_t*>(srcBuffer.GetMappedRange(bufferCopyOffset));
            constexpr uint32_t kBytesPerRow = kTextureBytesPerRowAlignment;
            for (uint32_t y = 0; y < kHeight; ++y) {
                memcpy(mappedPtr + y * kBytesPerRow, stencilData.data() + y * kWidth, kWidth);
            }
            srcBuffer.Unmap();

            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
                srcBuffer, bufferCopyOffset, kBytesPerRow, kHeight);
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                depthStencilTexture, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
            wgpu::Extent3D copySize = {kWidth, kHeight, 1};
            commandEncoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo,
                                               &copySize);
            wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
            queue.Submit(1, &commandBuffer);
        }

        // Decrement the stencil value in a render pass to ensure the data is visible to the
        // pipeline.
        {
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            // Create a render pipline which decrements the stencil value for passing fragments.
            // A quad is drawn in the bottom left.
            utils::ComboRenderPipelineDescriptor renderPipelineDesc;
            renderPipelineDesc.vertex.module = mVertexModule;
            renderPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() {
            })");
            renderPipelineDesc.cFragment.targetCount = 0;
            wgpu::DepthStencilState* depthStencil =
                renderPipelineDesc.EnableDepthStencil(GetParam().mTextureFormat);
            depthStencil->stencilFront.passOp = wgpu::StencilOperation::DecrementClamp;
            if (!hasDepth) {
                depthStencil->depthWriteEnabled = wgpu::OptionalBool::False;
                depthStencil->depthCompare = wgpu::CompareFunction::Always;
            }

            wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);

            // Create a render pass which loads the stencil. We want to load the values we
            // copied in. Also load the canary depth values so they're not lost.
            utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
            passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
            passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mTextureFormat);

            // Draw the quad in the bottom left (two triangles).
            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
            pass.SetPipeline(pipeline);
            pass.Draw(6);
            pass.End();

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);
        }

        // Copy back the stencil data and check it is correct.
        EXPECT_TEXTURE_EQ(expectedStencilData.data(), depthStencilTexture, {0, 0},
                          {kWidth, kHeight}, 0, wgpu::TextureAspect::StencilOnly);

        if (hasDepth) {
            ExpectAttachmentDepthTestData(depthStencilTexture, GetParam().mTextureFormat, kWidth,
                                          kHeight, 0, 0,
                                          {
                                              0.7, 0.7, 0.7, 0.7,  //
                                              0.7, 0.7, 0.7, 0.7,  //
                                              0.7, 0.7, 0.7, 0.7,  //
                                              0.7, 0.7, 0.7, 0.7,  //
                                          });
        }
    }
};

// Test copying the stencil-only aspect into a buffer.
TEST_P(StencilCopyTests, FromStencilAspect) {
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Test compute emulation path for stencil 8
        {2, 2},
        {3, 3},
        // stencil 8 needs bytesPerRow alignment
        {257, 1},
    };
    constexpr uint32_t kTestTextureArrayLayerCounts[] = {
        1,
        2,
    };
    for (const uint32_t sizeZ : kTestTextureArrayLayerCounts) {
        for (const auto& size : kTestTextureSizes) {
            DoCopyFromStencilTest(kBufferCopyOffset, size[0], size[1], sizeZ, kTestLevel);
        }
    }
}

// Test copying the stencil-only aspect into a buffer at a non-zero offset
TEST_P(StencilCopyTests, FromStencilAspectAtNonZeroOffset) {
    constexpr uint32_t kTestLevel = 0;
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Test compute emulation path for stencil 8
        {2, 2},
        {3, 3},
        // stencil 8 needs bytesPerRow alignment
        {257, 1},
    };
    constexpr uint32_t kTestTextureArrayLayerCounts[] = {
        1,
        2,
    };
    for (uint32_t offset : kBufferCopyOffsets) {
        for (const uint32_t sizeZ : kTestTextureArrayLayerCounts) {
            for (const auto& size : kTestTextureSizes) {
                DoCopyFromStencilTest(offset, size[0], size[1], sizeZ, kTestLevel);
            }
        }
    }
}

// Test copying the non-zero mip, stencil-only aspect into a buffer.
TEST_P(StencilCopyTests, FromNonZeroMipStencilAspect) {
    // TODO(crbug.com/dawn/2273): Failing on ANGLE/D3D11 for unknown reasons.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, 1, 1);
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, 2, 2);
}

// Test buffer content outside of copy extent is preserved.
// This test is made specifially for compute blit for stencil8 emulation path.
// The texel size is 1 byte, while in the compute shader we have to write 4 byte at a time.
// When the texture width % 4 != 0, buffer content outside of the copy range is
// inevitably written. So we need to make sure the original content of the buffer that's outside of
// the copy extent is still correctly preserved.
TEST_P(StencilCopyTests, PreserveBufferContent) {
    constexpr uint32_t kBufferCopyOffsets[] = {0u, 4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        {1, 1},
        {1, 2},
        {3, 3},
    };
    constexpr uint32_t kExtraBufferSize[] = {0u, 4u};
    const uint32_t kSizeZ = 1;
    constexpr uint32_t kTestLevel = 0;

    wgpu::TextureFormat format = GetParam().mTextureFormat;
    constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::StencilOnly;

    for (uint32_t extraBufferSize : kExtraBufferSize) {
        for (uint32_t offset : kBufferCopyOffsets) {
            for (const auto& size : kTestTextureSizes) {
                wgpu::BufferDescriptor bufferDescriptor = {};
                // Add wgpu::BufferUsage::MapRead to check the buffer content with mapAsync
                bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
                bufferDescriptor.size =
                    extraBufferSize + offset +
                    BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect);
                bufferDescriptor.mappedAtCreation = true;
                wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);
                constexpr uint8_t kOriginalValue = 0xff;
                {
                    // Fill the buffer with an original value other than 0 to check they are
                    // incorrectly overwritten outside of the copy range.
                    uint8_t* ptr = static_cast<uint8_t*>(buffer.GetMappedRange());
                    std::fill(ptr, ptr + bufferDescriptor.size, kOriginalValue);
                    buffer.Unmap();
                }

                // Don't check copy region content because the buffer doesn't have
                // wgpu::BufferUsage::CopySrc usage.
                DoCopyFromStencilTestWithBuffer(buffer, offset, size[0], size[1], kSizeZ,
                                                kTestLevel, false);

                std::vector<uint8_t> expected(bufferDescriptor.size, kOriginalValue);
                // Get the offset of the end of the copy range (without aligning with 4 bytes)
                uint32_t bufferEndOffset =
                    offset +
                    BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect, false);
                if (bufferDescriptor.size > bufferEndOffset) {
                    // Cannot use EXPECT_BUFFER_* helper here because it needs to align the copy
                    // size to a multiple of 4 bytes to call CopyBufferToBuffer. We are checking
                    // stencil8.
                    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, wgpu::kWholeMapSize);
                    const uint8_t* ptr = static_cast<const uint8_t*>(buffer.GetConstMappedRange());

                    // Check the content before copy range.
                    for (uint32_t i = 0; i < offset; i++) {
                        EXPECT_EQ(ptr[i], kOriginalValue);
                    }

                    // Check the content after copy range.
                    uint32_t checkSize = bufferDescriptor.size - bufferEndOffset;
                    for (uint32_t i = 0; i < checkSize; i++) {
                        EXPECT_EQ(ptr[bufferEndOffset + i], kOriginalValue);
                    }
                    buffer.Unmap();
                }
            }
        }
    }
}

// Test compact buffer size edge case.
// This test is made specifially for compute blit for stencil8 emulation path.
// When texture width % 4 != 0, the size of the most compact buffer copy
// target can be something that's not a multiple of 4. We need to make sure access don't go out of
// bounds in the shader, when still writing to array<u32> in the compute shader.
TEST_P(StencilCopyTests, BufferCopySizeEdgeCase) {
    constexpr uint32_t kBufferCopyOffsets[] = {0u, 4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Storage buffer binding requires size of at least 4 bytes.
        {5, 1}, {6, 1}, {7, 1}, {1, 2}, {2, 2}, {3, 3},
    };
    const uint32_t kSizeZ = 1;
    constexpr uint32_t kTestLevel = 0;

    wgpu::TextureFormat format = GetParam().mTextureFormat;
    constexpr wgpu::TextureAspect aspect = wgpu::TextureAspect::StencilOnly;

    for (uint32_t offset : kBufferCopyOffsets) {
        for (const auto& size : kTestTextureSizes) {
            wgpu::BufferDescriptor bufferDescriptor = {};
            // Add wgpu::BufferUsage::MapRead to check the buffer content with mapAsync
            bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
            bufferDescriptor.size =
                offset +
                // Don't align for 4 bytes to get the smallest possible buffer for depth16unorm.
                BufferSizeForTextureCopy(size[0], size[1], kSizeZ, format, aspect, false);
            wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

            // Don't check copy region content because the buffer doesn't have
            // wgpu::BufferUsage::CopySrc usage.
            DoCopyFromStencilTestWithBuffer(buffer, offset, size[0], size[1], kSizeZ, kTestLevel,
                                            false);

            // Unable to check the result since either MapAsync and CopyBufferToBuffer requires size
            // to be multiple of 4 bytes.
            // Just run and don't crash on DAWN_ASSERT.
        }
    }
}

// Test copying to the stencil-aspect of a texture
TEST_P(StencilCopyTests, ToStencilAspect) {
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyToStencilTest(kBufferCopyOffset);
}

// Test copying to the stencil-aspect of a texture at non-zero offset
TEST_P(StencilCopyTests, ToStencilAspectAtNonZeroOffset) {
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {8, 512};
    for (uint32_t offset : kBufferCopyOffsets) {
        DoCopyToStencilTest(offset);
    }
}

// Test uploading to the non-zero mip, stencil-only aspect of a texture,
// and then checking the contents with a stencil test.
TEST_P(StencilCopyTests, CopyNonzeroMipThenReadWithStencilTest) {
    // Copies to a single aspect are unsupported on OpenGL.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());

    // Create a stencil texture
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kMipLevel = 1;

    wgpu::Texture depthStencilTexture =
        CreateTexture(kWidth, kHeight,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                          wgpu::TextureUsage::CopyDst,
                      kMipLevel + 1);

    std::vector<uint8_t> stencilData = {
        7u, 7u,  //
        7u, 7u,  //
    };

    // Upload the stencil data.
    {
        wgpu::TexelCopyBufferLayout dataLayout = {};
        dataLayout.bytesPerRow = kWidth >> kMipLevel;

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            depthStencilTexture, 1, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
        wgpu::Extent3D copySize = {kWidth >> kMipLevel, kHeight >> kMipLevel, 1};

        queue.WriteTexture(&texelCopyTextureInfo, stencilData.data(), stencilData.size(),
                           &dataLayout, &copySize);
    }

    // Check the stencil contents.
    ExpectAttachmentStencilTestData(depthStencilTexture, GetParam().mTextureFormat,
                                    kWidth >> kMipLevel, kWidth >> kMipLevel, 0u, kMipLevel, 7u);
}

class StencilCopyTests_Compat : public StencilCopyTests {
    void SetUp() override {
        StencilCopyTests::SetUp();
        DAWN_SUPPRESS_TEST_IF(!IsCompatibilityMode());
    }

  public:
    static constexpr uint32_t kCubeTextureLayerCount = 6;
};

TEST_P(StencilCopyTests_Compat, FromStencilAspect) {
    constexpr uint32_t kTestLevel = 0;
    constexpr uint32_t kBufferCopyOffset = 0;
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Test compute emulation path for stencil 8
        {2, 2},
        {3, 3},
        // stencil 8 needs bytesPerRow alignment
        {257, 257},
    };
    for (const auto& size : kTestTextureSizes) {
        DoCopyFromStencilTest(kBufferCopyOffset, size[0], size[1], kCubeTextureLayerCount,
                              kTestLevel);
    }
}

// Test copying the stencil-only aspect into a buffer at a non-zero offset
TEST_P(StencilCopyTests_Compat, FromStencilAspectAtNonZeroOffset) {
    constexpr uint32_t kTestLevel = 0;
    constexpr std::array<uint32_t, 2> kBufferCopyOffsets = {4u, 512u};
    constexpr uint32_t kTestTextureSizes[][2] = {
        // Original test parameter
        {4, 4},
        // Test compute emulation path for stencil 8
        {2, 2},
        {3, 3},
        // stencil 8 needs bytesPerRow alignment
        {257, 257},
    };
    for (uint32_t offset : kBufferCopyOffsets) {
        for (const auto& size : kTestTextureSizes) {
            DoCopyFromStencilTest(offset, size[0], size[1], kCubeTextureLayerCount, kTestLevel);
        }
    }
}

// Test copying the non-zero mip, stencil-only aspect into a buffer.
TEST_P(StencilCopyTests_Compat, FromNonZeroMipStencilAspect) {
    // TODO(crbug.com/dawn/2273): Failing on ANGLE/D3D11 for unknown reasons.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());
    constexpr uint32_t kWidth = 9;
    constexpr uint32_t kHeight = 9;
    constexpr uint32_t kBufferCopyOffset = 0;
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, kCubeTextureLayerCount, 1);
    DoCopyFromStencilTest(kBufferCopyOffset, kWidth, kHeight, kCubeTextureLayerCount, 2);
}

class DepthStencilCopyTests_RegressionDawn1083 : public DepthStencilCopyTests {};

// Regression test for crbug.com/dawn/1083. Checks that T2T copies with
// various mip/layer counts/offsets works.
TEST_P(DepthStencilCopyTests_RegressionDawn1083, Run) {
    // TODO(crbug.com/dawn/1648): Diagnose failure on NVIDIA GLES.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsNvidia());

    // TODO(crbug.com/dawn/1651): Failing on Mac AMD.
    DAWN_SUPPRESS_TEST_IF(IsAMD() && IsMetal() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth32FloatStencil8);

    // TODO(crbug.com/dawn/1828): depth16unorm broken on Apple GPUs.
    DAWN_SUPPRESS_TEST_IF(IsApple() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::Depth16Unorm);

    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    uint32_t mipLevelCount = 3;
    uint32_t arrayLayerCount = 3;
    wgpu::TextureDescriptor texDesc = {};
    texDesc.size = {8, 8, arrayLayerCount};
    texDesc.format = GetParam().mTextureFormat;
    texDesc.mipLevelCount = mipLevelCount;

    for (uint32_t mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel) {
        uint32_t mipWidth = texDesc.size.width >> mipLevel;
        uint32_t mipHeight = texDesc.size.height >> mipLevel;
        for (uint32_t srcArrayLayer = 0; srcArrayLayer < arrayLayerCount; ++srcArrayLayer) {
            for (uint32_t dstArrayLayer = 0; dstArrayLayer < arrayLayerCount; ++dstArrayLayer) {
                // Test copying 1 layer, and all possible layers.
                for (uint32_t layerCount :
                     {1u, arrayLayerCount - std::max(srcArrayLayer, dstArrayLayer)}) {
                    texDesc.usage =
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
                    wgpu::Texture src = device.CreateTexture(&texDesc);

                    texDesc.usage =
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst;
                    wgpu::Texture dst = device.CreateTexture(&texDesc);

                    // Initialize texture data.
                    uint8_t stencilValue = 42;
                    float d1 = 0.1;
                    float d2 = 0.01;
                    for (uint32_t z = 0; z < layerCount; ++z) {
                        InitializeDepthStencilTextureRegion(src, d1, d2, stencilValue, stencilValue,
                                                            mipLevel, srcArrayLayer + z);
                    }

                    // Perform a T2T copy
                    {
                        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
                        wgpu::TexelCopyTextureInfo srcView =
                            utils::CreateTexelCopyTextureInfo(src, mipLevel, {0, 0, srcArrayLayer});
                        wgpu::TexelCopyTextureInfo dstView =
                            utils::CreateTexelCopyTextureInfo(dst, mipLevel, {0, 0, dstArrayLayer});
                        wgpu::Extent3D copySize = {mipWidth, mipHeight, layerCount};
                        commandEncoder.CopyTextureToTexture(&srcView, &dstView, &copySize);

                        wgpu::CommandBuffer commands = commandEncoder.Finish();
                        queue.Submit(1, &commands);
                    }

                    // Helper to compute expected depth data.
                    auto GetExpectedDepthData = [&](uint32_t mipLevel) -> std::vector<float> {
                        if (mipLevel == 0) {
                            return {
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d1, d1, d1, d1, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                                d2, d2, d2, d2, d1, d1, d1, d1,  //
                            };
                        } else if (mipLevel == 1) {
                            return {
                                d1, d1, d1, d1,  //
                                d1, d1, d1, d1,  //
                                d2, d2, d1, d1,  //
                                d2, d2, d1, d1,  //
                            };
                        } else if (mipLevel == 2) {
                            return {
                                d1, d1,  //
                                d2, d1,  //
                            };
                        }
                        DAWN_UNREACHABLE();
                    };

                    // Check the depth
                    for (uint32_t z = 0; z < layerCount; ++z) {
                        ExpectAttachmentDepthTestData(dst, GetParam().mTextureFormat, mipWidth,
                                                      mipHeight, dstArrayLayer + z, mipLevel,
                                                      GetExpectedDepthData(mipLevel))
                            << "depth aspect" << "\nmipLevelCount: " << mipLevelCount
                            << "\narrayLayerCount: " << arrayLayerCount
                            << "\nmipLevel: " << mipLevel
                            << "\nsrcArrayLayer: " << srcArrayLayer + z
                            << "\ndstArrayLayer: " << dstArrayLayer + z;

                        // Check the stencil
                        if (!utils::IsDepthOnlyFormat(GetParam().mTextureFormat)) {
                            ExpectAttachmentStencilTestData(dst, GetParam().mTextureFormat,
                                                            mipWidth, mipHeight, dstArrayLayer + z,
                                                            mipLevel, stencilValue)
                                << "stencil aspect" << "\nmipLevelCount: " << mipLevelCount
                                << "\narrayLayerCount: " << arrayLayerCount
                                << "\nmipLevel: " << mipLevel
                                << "\nsrcArrayLayer: " << srcArrayLayer + z
                                << "\ndstArrayLayer: " << dstArrayLayer + z;
                        }
                    }
                }
            }
        }
    }
}

DAWN_INSTANTIATE_TEST_P(
    DepthStencilCopyTests,
    {D3D11Backend(), D3D11Backend({"use_packed_depth24_unorm_stencil8_format"}), D3D12Backend(),
     D3D12Backend({"use_packed_depth24_unorm_stencil8_format"}),
     D3D12Backend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     MetalBackend(),
     MetalBackend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     MetalBackend({"use_blit_for_buffer_to_depth_texture_copy",
                   "use_blit_for_buffer_to_stencil_texture_copy"}),
     OpenGLBackend(), OpenGLESBackend(),
     // Test with the vulkan_use_s8 toggle forced on and off.
     VulkanBackend({"vulkan_use_s8"}, {}), VulkanBackend({}, {"vulkan_use_s8"})},
    std::vector<wgpu::TextureFormat>(utils::kDepthAndStencilFormats.begin(),
                                     utils::kDepthAndStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthCopyTests,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                                       "copy_with_non_zero_buffer_offset"}),
                         MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyTextureFormats.begin(),
                                                         kValidDepthCopyTextureFormats.end()));
DAWN_INSTANTIATE_TEST_P(DepthCopyTests_Compat,
                        {OpenGLBackend(), OpenGLESBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyFromBufferFormats.begin(),
                                                         kValidDepthCopyFromBufferFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthCopyFromBufferTests,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                                       "copy_with_non_zero_buffer_offset"}),
                         MetalBackend(),
                         MetalBackend({"use_blit_for_buffer_to_depth_texture_copy"}),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(kValidDepthCopyFromBufferFormats.begin(),
                                                         kValidDepthCopyFromBufferFormats.end()));

DAWN_INSTANTIATE_TEST_P(
    StencilCopyTests,
    {D3D11Backend(), D3D11Backend({"use_packed_depth24_unorm_stencil8_format"}), D3D12Backend(),
     D3D12Backend({"use_packed_depth24_unorm_stencil8_format"}),
     D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                   "copy_with_non_zero_buffer_offset"}),
     MetalBackend(), MetalBackend({"metal_use_combined_depth_stencil_format_for_stencil8"}),
     MetalBackend(
         {"metal_use_both_depth_and_stencil_attachments_for_combined_depth_stencil_formats"}),
     MetalBackend({"use_blit_for_buffer_to_stencil_texture_copy"}), OpenGLBackend(),
     OpenGLESBackend(), OpenGLESBackend({"gl_force_es_31_and_no_extensions"}),
     // Test with the vulkan_use_s8 toggle forced on and off.
     VulkanBackend({"vulkan_use_s8"}, {}), VulkanBackend({}, {"vulkan_use_s8"})},
    std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(), utils::kStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(StencilCopyTests_Compat,
                        {OpenGLBackend(), OpenGLESBackend(),
                         OpenGLESBackend({"gl_force_es_31_and_no_extensions"})},
                        std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(),
                                                         utils::kStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(
    DepthStencilCopyTests_RegressionDawn1083,
    {D3D11Backend(), D3D12Backend(),
     D3D12Backend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     MetalBackend(),
     MetalBackend({"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource"}),
     OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::TextureFormat>{wgpu::TextureFormat::Depth16Unorm,
                                     wgpu::TextureFormat::Depth32FloatStencil8,
                                     wgpu::TextureFormat::Depth24Plus});

}  // anonymous namespace
}  // namespace dawn
