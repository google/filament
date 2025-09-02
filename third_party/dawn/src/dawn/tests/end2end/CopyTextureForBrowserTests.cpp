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

#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

// TODO(dawn:2205) Remove these stream operators if we move them to a standard location.
namespace wgpu {
std::ostream& operator<<(std::ostream& o, Origin3D origin) {
    o << origin.x << ", " << origin.y << ", " << origin.z;
    return o;
}

std::ostream& operator<<(std::ostream& o, Extent3D copySize) {
    o << copySize.width << ", " << copySize.height << ", " << copySize.depthOrArrayLayers;
    return o;
}
}  // namespace wgpu

namespace dawn {
namespace {

static constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

// Set default texture size to single line texture for color conversion tests.
static constexpr uint64_t kDefaultTextureWidth = 10;
static constexpr uint64_t kDefaultTextureHeight = 1;

enum class ColorSpace : uint32_t {
    SRGB = 0x00,
    DisplayP3 = 0x01,
};

using SrcFormat = wgpu::TextureFormat;
using DstFormat = wgpu::TextureFormat;
using SrcOrigin = wgpu::Origin3D;
using DstOrigin = wgpu::Origin3D;
using CopySize = wgpu::Extent3D;
using FlipY = bool;
using SrcColorSpace = ColorSpace;
using DstColorSpace = ColorSpace;
using SrcAlphaMode = wgpu::AlphaMode;
using DstAlphaMode = wgpu::AlphaMode;

std::ostream& operator<<(std::ostream& o, ColorSpace space) {
    o << static_cast<uint32_t>(space);
    return o;
}

DAWN_TEST_PARAM_STRUCT(AlphaTestParams, SrcAlphaMode, DstAlphaMode);
DAWN_TEST_PARAM_STRUCT(FormatTestParams, SrcFormat, DstFormat);
DAWN_TEST_PARAM_STRUCT(SubRectTestParams, SrcOrigin, DstOrigin, CopySize, FlipY);
DAWN_TEST_PARAM_STRUCT(ColorSpaceTestParams,
                       DstFormat,
                       SrcColorSpace,
                       DstColorSpace,
                       SrcAlphaMode,
                       DstAlphaMode);

// Color Space table
struct ColorSpaceInfo {
    ColorSpace index;
    std::array<float, 9> toXYZD50;             // 3x3 row major transform matrix
    std::array<float, 9> fromXYZD50;           // inverse transform matrix of toXYZD50, precomputed
    std::array<float, 7> gammaDecodingParams;  // Follow { A, B, G, E, epsilon, C, F } order
    std::array<float, 7> gammaEncodingParams;  // inverse op of decoding, precomputed
    bool isNonLinear;
    bool isExtended;  // For extended color space.
};
static constexpr size_t kSupportedColorSpaceCount = 2;
static constexpr std::array<ColorSpaceInfo, kSupportedColorSpaceCount> ColorSpaceTable = {{
    // sRGB,
    // Got primary attributes from https://drafts.csswg.org/css-color/#predefined-sRGB
    // Use matrices from
    // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html#WSMatrices
    // Get gamma-linear conversion params from https://en.wikipedia.org/wiki/SRGB with some
    // mathematics.
    {
        //
        ColorSpace::SRGB,
        {{
            //
            0.4360747, 0.3850649, 0.1430804,  //
            0.2225045, 0.7168786, 0.0606169,  //
            0.0139322, 0.0971045, 0.7141733   //
        }},

        {{
            //
            3.1338561, -1.6168667, -0.4906146,  //
            -0.9787684, 1.9161415, 0.0334540,   //
            0.0719453, -0.2289914, 1.4052427    //
        }},

        // {G, A, B, C, D, E, F, }
        {{2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 4.045e-02, 0.0, 0.0}},

        {{1.0 / 2.4, 1.13711 /*pow(1.055, 2.4)*/, 0.0, 12.92f, 3.1308e-03, -0.055, 0.0}},

        true,
        true  //
    },

    // Display P3, got primary attributes from
    // https://www.w3.org/TR/css-color-4/#valdef-color-display-p3
    // Use equations found in
    // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html,
    // Use Bradford method to do D65 to D50 transform.
    // Get matrices with help of http://www.russellcottrell.com/photo/matrixCalculator.htm
    // Gamma-linear conversion params is the same as Srgb.
    {
        //
        ColorSpace::DisplayP3,
        {{
            //
            0.5151114, 0.2919612, 0.1571274,  //
            0.2411865, 0.6922440, 0.0665695,  //
            -0.0010491, 0.0418832, 0.7842659  //
        }},

        {{
            //
            2.4039872, -0.9898498, -0.3976181,  //
            -0.8422138, 1.7988188, 0.0160511,   //
            0.0481937, -0.0973889, 1.2736887    //
        }},

        // {G, A, B, C, D, E, F, }
        {{2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 4.045e-02, 0.0, 0.0}},

        {{1.0 / 2.4, 1.13711 /*pow(1.055, 2.4)*/, 0.0, 12.92f, 3.1308e-03, -0.055, 0.0}},

        true,
        false  //
    }
    //
}};

template <typename Parent>
class CopyTextureForBrowserTests : public Parent {
  protected:
    struct TextureSpec {
        wgpu::Origin3D copyOrigin = {};
        wgpu::Extent3D textureSize = {kDefaultTextureWidth, kDefaultTextureHeight};
        uint32_t level = 0;
        wgpu::TextureFormat format = kTextureFormat;
    };

    enum class TextureCopyRole {
        SOURCE,
        DEST,
    };

    // Source texture contains red pixels and dst texture contains green pixels at start.
    static std::vector<utils::RGBA8> GetTextureData(
        const utils::TextureDataCopyLayout& layout,
        TextureCopyRole textureRole,
        wgpu::AlphaMode srcAlphaMode = wgpu::AlphaMode::Premultiplied,
        wgpu::AlphaMode dstAlphaMode = wgpu::AlphaMode::Unpremultiplied) {
        std::array<uint8_t, 4> alpha = {0, 102, 153, 255};  // 0.0, 0.4, 0.6, 1.0
        std::vector<utils::RGBA8> textureData(layout.texelBlockCount);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t sliceOffset = layout.texelBlocksPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                const uint32_t rowOffset = layout.texelBlocksPerRow * y;
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    // Source textures will have variable pixel data to cover cases like
                    // flipY.
                    if (textureRole == TextureCopyRole::SOURCE) {
                        if (srcAlphaMode == wgpu::AlphaMode::Unpremultiplied &&
                            dstAlphaMode == wgpu::AlphaMode::Premultiplied) {
                            // We expect each channel in dst
                            // texture will equal to the alpha channel value.
                            textureData[sliceOffset + rowOffset + x] = utils::RGBA8(
                                static_cast<uint8_t>(255), static_cast<uint8_t>(255),
                                static_cast<uint8_t>(255), static_cast<uint8_t>(alpha[x % 4]));
                        } else if (srcAlphaMode == wgpu::AlphaMode::Premultiplied &&
                                   dstAlphaMode == wgpu::AlphaMode::Unpremultiplied) {
                            // We expect each channel in dst
                            // texture will equal to 1.0.
                            textureData[sliceOffset + rowOffset + x] =
                                utils::RGBA8(static_cast<uint8_t>(alpha[x % 4]),
                                             static_cast<uint8_t>(alpha[x % 4]),
                                             static_cast<uint8_t>(alpha[x % 4]),
                                             static_cast<uint8_t>(alpha[x % 4]));
                        } else {
                            textureData[sliceOffset + rowOffset + x] = utils::RGBA8(
                                static_cast<uint8_t>((x + layer * x) % 256),
                                static_cast<uint8_t>((y + layer * y) % 256),
                                static_cast<uint8_t>(x % 256), static_cast<uint8_t>(x % 256));
                        }
                    } else {  // Dst textures will have be init as `green` to ensure subrect
                              // copy not cross bound.
                        textureData[sliceOffset + rowOffset + x] =
                            utils::RGBA8(static_cast<uint8_t>(0), static_cast<uint8_t>(255),
                                         static_cast<uint8_t>(0), static_cast<uint8_t>(255));
                    }
                }
            }
        }

        return textureData;
    }

    void SetUp() override {
        Parent::SetUp();
        pipeline = MakeTestPipeline();

        uint32_t uniformBufferData[] = {
            0,  // copy have flipY option
            4,  // channelCount
            0,
            0,  // uvec2, subrect copy src origin
            0,
            0,  // uvec2, subrect copy dst origin
            0,
            0,                                                      // uvec2, subrect copy size
            static_cast<uint32_t>(wgpu::AlphaMode::Premultiplied),  // srcAlphaMode
            static_cast<uint32_t>(wgpu::AlphaMode::Premultiplied),  // dstAlphaMode
        };

        wgpu::BufferDescriptor uniformBufferDesc = {};
        uniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        uniformBufferDesc.size = sizeof(uniformBufferData);
        uniformBuffer = this->device.CreateBuffer(&uniformBufferDesc);
    }

    // Do the bit-by-bit comparison between the source and destination texture with GPU (compute
    // shader) instead of CPU after executing CopyTextureForBrowser() to avoid the errors caused by
    // comparing a value generated on CPU to the one generated on GPU.
    wgpu::ComputePipeline MakeTestPipeline() {
        std::string shader =
            R"(
            struct Uniforms {
                dstTextureFlipY : u32,
                channelCount    : u32,
                srcCopyOrigin   : vec2u,
                dstCopyOrigin   : vec2u,
                copySize        : vec2u,
                srcAlphaMode    : u32,
                dstAlphaMode    : u32,
            }
            struct OutputBuf {
                result : array<u32>
            }
            @group(0) @binding(0) var src : texture_2d<f32>;
            @group(0) @binding(1) var dst : texture_2d<f32>;
            @group(0) @binding(2) var<storage, read_write> output : OutputBuf;
            @group(0) @binding(3) var<uniform> uniforms : Uniforms;
            fn aboutEqual(value : f32, expect : f32) -> bool {
                // The value diff should be smaller than the hard coded tolerance.
                return abs(value - expect) < 0.01;
            }
            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                let srcSize = textureDimensions(src);
                let dstSize = textureDimensions(dst);
                let dstTexCoord = vec2u(GlobalInvocationID.xy);
                let nonCoveredColor = vec4f(0.0, 1.0, 0.0, 1.0); // should be green

                var success : bool = true;
                if (dstTexCoord.x < uniforms.dstCopyOrigin.x ||
                    dstTexCoord.y < uniforms.dstCopyOrigin.y ||
                    dstTexCoord.x >= uniforms.dstCopyOrigin.x + uniforms.copySize.x ||
                    dstTexCoord.y >= uniforms.dstCopyOrigin.y + uniforms.copySize.y) {
                    success = success &&
                              all(textureLoad(dst, vec2i(dstTexCoord), 0) == nonCoveredColor);
                } else {
                    // Calculate source texture coord.
                    var srcTexCoordInRect = dstTexCoord - uniforms.dstCopyOrigin;

                    // Note that |flipY| equals flip src texture in copy sub rect.
                    if (uniforms.dstTextureFlipY == 1u) {
                        srcTexCoordInRect.y = uniforms.copySize.y - srcTexCoordInRect.y - 1;
                    }

                    var srcTexCoord = srcTexCoordInRect + uniforms.srcCopyOrigin;

                    var srcColor = textureLoad(src, vec2i(srcTexCoord), 0);
                    var dstColor = textureLoad(dst, vec2i(dstTexCoord), 0);

                    // Expect the dst texture channels should be all equal to alpha value
                    // after premultiply.
                    let premultiplied = )" +
            std::to_string(static_cast<uint32_t>(wgpu::AlphaMode::Premultiplied)) + R"(u;
                    let unpremultiplied = )" +
            std::to_string(static_cast<uint32_t>(wgpu::AlphaMode::Unpremultiplied)) + R"(u;
                    let opaque = )" +
            std::to_string(static_cast<uint32_t>(wgpu::AlphaMode::Opaque)) + R"(u;
                    if (uniforms.srcAlphaMode == opaque) {
                        srcColor.a = 1.0;
                    }

                    if (uniforms.srcAlphaMode == unpremultiplied && uniforms.dstAlphaMode == premultiplied) {
                        srcColor = vec4f(srcColor.rgb * srcColor.a, srcColor.a);
                    }

                    if (uniforms.srcAlphaMode == premultiplied && uniforms.dstAlphaMode == unpremultiplied) {
                        if (srcColor.a != 0.0) {
                            srcColor = vec4f(srcColor.rgb / srcColor.a, srcColor.a);
                        }
                    }

                    // Not use loop and variable index format to workaround
                    // crbug.com/tint/638.
                    switch(uniforms.channelCount) {
                        case 1u: {
                            success = success && aboutEqual(dstColor.r, srcColor.r);
                            break;
                        }
                        case 2u: {
                            success = success &&
                                      aboutEqual(dstColor.r, srcColor.r) &&
                                      aboutEqual(dstColor.g, srcColor.g);
                            break;
                        }
                        case 4u: {
                            success = success &&
                                      aboutEqual(dstColor.r, srcColor.r) &&
                                      aboutEqual(dstColor.g, srcColor.g) &&
                                      aboutEqual(dstColor.b, srcColor.b) &&
                                      aboutEqual(dstColor.a, srcColor.a);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                let outputIndex = GlobalInvocationID.y * u32(dstSize.x) + GlobalInvocationID.x;
                if (success) {
                    output.result[outputIndex] = 1u;
                } else {
                    output.result[outputIndex] = 0u;
                }
            }
         )";
        wgpu::ShaderModule csModule = utils::CreateShaderModule(this->device, shader.c_str());

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = csModule;

        return this->device.CreateComputePipeline(&csDesc);
    }
    static uint32_t GetTextureFormatComponentCount(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8UnormSrgb:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::BGRA8UnormSrgb:
            case wgpu::TextureFormat::RGB10A2Unorm:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
                return 4;
            case wgpu::TextureFormat::RG8Unorm:
            case wgpu::TextureFormat::RG16Float:
            case wgpu::TextureFormat::RG32Float:
                return 2;
            case wgpu::TextureFormat::R8Unorm:
            case wgpu::TextureFormat::R16Float:
            case wgpu::TextureFormat::R32Float:
                return 1;
            default:
                DAWN_UNREACHABLE();
        }
    }

    wgpu::Texture CreateTexture(const TextureSpec& spec, wgpu::TextureUsage usage) {
        // Create and initialize src texture.
        wgpu::TextureDescriptor descriptor;
        descriptor.size = spec.textureSize;
        descriptor.format = spec.format;
        descriptor.mipLevelCount = spec.level + 1;
        descriptor.usage = usage;
        wgpu::Texture texture = this->device.CreateTexture(&descriptor);
        return texture;
    }

    wgpu::Texture CreateAndInitTexture(const TextureSpec& spec,
                                       wgpu::TextureUsage usage,
                                       utils::TextureDataCopyLayout copyLayout,
                                       void const* init,
                                       uint32_t initBytes) {
        wgpu::Texture texture = CreateTexture(spec, usage);

        wgpu::TexelCopyTextureInfo imageTextureInit =
            utils::CreateTexelCopyTextureInfo(texture, spec.level, {0, 0});

        wgpu::TexelCopyBufferLayout texelCopyBufferLayout;
        texelCopyBufferLayout.offset = 0;
        texelCopyBufferLayout.bytesPerRow = copyLayout.bytesPerRow;
        texelCopyBufferLayout.rowsPerImage = copyLayout.rowsPerImage;

        this->device.GetQueue().WriteTexture(&imageTextureInit, init, initBytes,
                                             &texelCopyBufferLayout, &copyLayout.mipSize);
        return texture;
    }

    void RunCopyExternalImageToTexture(
        const TextureSpec& srcSpec,
        wgpu::Texture srcTexture,
        const TextureSpec& dstSpec,
        wgpu::Texture dstTexture,
        const wgpu::Extent3D& copySize,
        const wgpu::CopyTextureForBrowserOptions options,
        const wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            srcTexture, srcSpec.level, srcSpec.copyOrigin, aspect);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            dstTexture, dstSpec.level, dstSpec.copyOrigin, aspect);
        this->device.GetQueue().CopyTextureForBrowser(
            &srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &copySize, &options);
    }

    void CheckResultInBuiltInComputePipeline(const TextureSpec& srcSpec,
                                             wgpu::Texture srcTexture,
                                             const TextureSpec& dstSpec,
                                             wgpu::Texture dstTexture,
                                             const wgpu::Extent3D& copySize,
                                             const wgpu::CopyTextureForBrowserOptions options) {
        // Update uniform buffer based on test config
        uint32_t uniformBufferData[] = {
            options.flipY,                                   // copy have flipY option
            GetTextureFormatComponentCount(dstSpec.format),  // channelCount
            srcSpec.copyOrigin.x,
            srcSpec.copyOrigin.y,  // src texture copy origin
            dstSpec.copyOrigin.x,
            dstSpec.copyOrigin.y,  // dst texture copy origin
            copySize.width,
            copySize.height,  // copy size
            static_cast<uint32_t>(options.srcAlphaMode),
            static_cast<uint32_t>(options.dstAlphaMode)};

        this->device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData,
                                            sizeof(uniformBufferData));

        // Create output buffer to store result
        wgpu::BufferDescriptor outputDesc;
        outputDesc.size = dstSpec.textureSize.width * dstSpec.textureSize.height * sizeof(uint32_t);
        outputDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer outputBuffer = this->device.CreateBuffer(&outputDesc);

        // Create texture views for test.
        wgpu::TextureViewDescriptor srcTextureViewDesc = {};
        srcTextureViewDesc.baseMipLevel = srcSpec.level;
        wgpu::TextureView srcTextureView = srcTexture.CreateView(&srcTextureViewDesc);

        wgpu::TextureViewDescriptor dstTextureViewDesc = {};
        dstTextureViewDesc.baseMipLevel = dstSpec.level;
        wgpu::TextureView dstTextureView = dstTexture.CreateView(&dstTextureViewDesc);

        // Create bind group based on the config.
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            this->device, pipeline.GetBindGroupLayout(0),
            {{0, srcTextureView}, {1, dstTextureView}, {2, outputBuffer}, {3, uniformBuffer}});

        // Start a pipeline to check pixel value in bit form.
        wgpu::CommandEncoder testEncoder = this->device.CreateCommandEncoder();

        wgpu::CommandBuffer testCommands;
        {
            wgpu::CommandEncoder encoder = this->device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(dstSpec.textureSize.width,
                                    dstSpec.textureSize.height);  // Verify dst texture content
            pass.End();

            testCommands = encoder.Finish();
        }
        this->device.GetQueue().Submit(1, &testCommands);

        std::vector<uint32_t> expectResult(dstSpec.textureSize.width * dstSpec.textureSize.height,
                                           1);
        EXPECT_BUFFER_U32_RANGE_EQ(expectResult.data(), outputBuffer, 0,
                                   dstSpec.textureSize.width * dstSpec.textureSize.height);
    }

    void DoTest(const TextureSpec& srcSpec,
                const TextureSpec& dstSpec,
                const wgpu::Extent3D& copySize = {kDefaultTextureWidth, kDefaultTextureHeight},
                const wgpu::CopyTextureForBrowserOptions options = {}) {
        // Create and initialize src texture.
        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                kTextureFormat,
                {srcSpec.textureSize.width, srcSpec.textureSize.height,
                 copySize.depthOrArrayLayers},
                srcSpec.level);

        std::vector<utils::RGBA8> srcTextureArrayCopyData = GetTextureData(
            srcCopyLayout, TextureCopyRole::SOURCE, options.srcAlphaMode, options.dstAlphaMode);

        wgpu::TextureUsage srcUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                      wgpu::TextureUsage::TextureBinding;
        wgpu::Texture srcTexture =
            CreateAndInitTexture(srcSpec, srcUsage, srcCopyLayout, srcTextureArrayCopyData.data(),
                                 srcTextureArrayCopyData.size() * sizeof(utils::RGBA8));

        bool testSubRectCopy = srcSpec.copyOrigin.x > 0 || srcSpec.copyOrigin.y > 0 ||
                               dstSpec.copyOrigin.x > 0 || dstSpec.copyOrigin.y > 0 ||
                               srcSpec.textureSize.width > copySize.width ||
                               srcSpec.textureSize.height > copySize.height ||
                               dstSpec.textureSize.width > copySize.width ||
                               dstSpec.textureSize.height > copySize.height;

        // Create and init dst texture.
        wgpu::Texture dstTexture;
        wgpu::TextureUsage dstUsage =
            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

        if (testSubRectCopy) {
            // For subrect copy tests, dst texture use kTextureFormat always.
            const utils::TextureDataCopyLayout dstCopyLayout =
                utils::GetTextureDataCopyLayoutForTextureAtLevel(
                    kTextureFormat,
                    {dstSpec.textureSize.width, dstSpec.textureSize.height,
                     copySize.depthOrArrayLayers},
                    dstSpec.level);

            const std::vector<utils::RGBA8> dstTextureArrayCopyData =
                GetTextureData(dstCopyLayout, TextureCopyRole::DEST);
            dstTexture = CreateAndInitTexture(
                dstSpec, dstUsage, dstCopyLayout, dstTextureArrayCopyData.data(),
                dstTextureArrayCopyData.size() * sizeof(utils::RGBA8));
        } else {
            dstTexture = CreateTexture(dstSpec, dstUsage);
        }

        // Perform the texture to texture copy
        RunCopyExternalImageToTexture(srcSpec, srcTexture, dstSpec, dstTexture, copySize, options);

        // Check Result
        CheckResultInBuiltInComputePipeline(srcSpec, srcTexture, dstSpec, dstTexture, copySize,
                                            options);
    }

    wgpu::Buffer uniformBuffer;
    wgpu::ComputePipeline pipeline;
};

class CopyTextureForBrowser_Basic : public CopyTextureForBrowserTests<DawnTest> {
  protected:
    void DoBasicCopyTest(const wgpu::Extent3D& copySize,
                         const wgpu::CopyTextureForBrowserOptions options = {}) {
        TextureSpec textureSpec;
        textureSpec.textureSize = copySize;

        DoTest(textureSpec, textureSpec, copySize, options);
    }
};

class CopyTextureForBrowser_Formats
    : public CopyTextureForBrowserTests<DawnTestWithParams<FormatTestParams>> {
  protected:
    bool IsDstFormatSrgbFormats() {
        return GetParam().mDstFormat == wgpu::TextureFormat::RGBA8UnormSrgb ||
               GetParam().mDstFormat == wgpu::TextureFormat::BGRA8UnormSrgb;
    }

    wgpu::TextureFormat GetNonSrgbFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return wgpu::TextureFormat::RGBA8Unorm;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return wgpu::TextureFormat::BGRA8Unorm;
            default:
                return format;
        }
    }

    wgpu::Texture CreateAndInitSourceTextureForColorFormatConversion(
        const TextureSpec& srcSpec,
        wgpu::TextureUsage srcUsage,
        utils::TextureDataCopyLayout srcCopyLayout) {
        // Create and init source texture.
        // This fixed source texture data is for color conversion tests.
        // The source data can fill a texture in default width and height.
        std::vector<utils::RGBA8> srcRGBA8UnormTextureArrayCopyData{
            // Take RGBA8Unorm as example:
            // R channel has different values
            utils::RGBA8(0, 255, 255, 255),    // r = 0.0
            utils::RGBA8(102, 255, 255, 255),  // r = 0.4
            utils::RGBA8(153, 255, 255, 255),  // r = 0.6

            // G channel has different values
            utils::RGBA8(255, 0, 255, 255),    // g = 0.0
            utils::RGBA8(255, 102, 255, 255),  // g = 0.4
            utils::RGBA8(255, 153, 255, 255),  // g = 0.6

            // B channel has different values
            utils::RGBA8(255, 255, 0, 255),    // b = 0.0
            utils::RGBA8(255, 255, 102, 255),  // b = 0.4
            utils::RGBA8(255, 255, 153, 255),  // b = 0.6

            // A channel set to 0
            utils::RGBA8(255, 255, 255, 0)  // a = 0
        };

        std::vector<uint16_t> srcRGBA16FloatTextureArrayCopyData{
            // R channel has different values
            // r = 0.0
            Float32ToFloat16(0.0), Float32ToFloat16(1.0), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // r = 0.4
            Float32ToFloat16(0.4), Float32ToFloat16(1.0), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // r = 0.6
            Float32ToFloat16(0.6), Float32ToFloat16(1.0), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // G channel has different values
            // g = 0.0
            Float32ToFloat16(1.0), Float32ToFloat16(0.0), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // g = 0.4
            Float32ToFloat16(1.0), Float32ToFloat16(0.4), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // g = 0.6
            Float32ToFloat16(1.0), Float32ToFloat16(0.6), Float32ToFloat16(1.0),
            Float32ToFloat16(1.0),

            // B channel has different values
            // b = 0.0
            Float32ToFloat16(1.0), Float32ToFloat16(1.0), Float32ToFloat16(0.0),
            Float32ToFloat16(1.0),

            // b = 0.4
            Float32ToFloat16(1.0), Float32ToFloat16(1.0), Float32ToFloat16(0.4),
            Float32ToFloat16(1.0),

            // b = 0.6
            Float32ToFloat16(1.0), Float32ToFloat16(1.0), Float32ToFloat16(0.6),
            Float32ToFloat16(1.0),

            // A channel set to 0
            // a = 0
            Float32ToFloat16(1.0), Float32ToFloat16(1.0), Float32ToFloat16(1.0),
            Float32ToFloat16(0.0)};

        switch (srcSpec.format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
                return CreateAndInitTexture(
                    srcSpec, srcUsage, srcCopyLayout, srcRGBA8UnormTextureArrayCopyData.data(),
                    srcRGBA8UnormTextureArrayCopyData.size() * sizeof(utils::RGBA8));
            case wgpu::TextureFormat::RGBA16Float:
                return CreateAndInitTexture(
                    srcSpec, srcUsage, srcCopyLayout, srcRGBA16FloatTextureArrayCopyData.data(),
                    srcRGBA16FloatTextureArrayCopyData.size() * sizeof(uint16_t));
            default:
                DAWN_UNREACHABLE();
        }
    }

    void DoColorConversionTest() {
        TextureSpec srcTextureSpec;
        srcTextureSpec.format = GetParam().mSrcFormat;

        TextureSpec dstTextureSpec;
        dstTextureSpec.format = GetParam().mDstFormat;

        wgpu::Extent3D copySize = {kDefaultTextureWidth, kDefaultTextureHeight};
        wgpu::CopyTextureForBrowserOptions options = {};

        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                srcTextureSpec.format,
                {srcTextureSpec.textureSize.width, srcTextureSpec.textureSize.height,
                 copySize.depthOrArrayLayers},
                srcTextureSpec.level);

        wgpu::TextureUsage srcUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                      wgpu::TextureUsage::TextureBinding;
        wgpu::Texture srcTexture = CreateAndInitSourceTextureForColorFormatConversion(
            srcTextureSpec, srcUsage, srcCopyLayout);

        // Create dst texture.
        wgpu::Texture dstTexture = CreateTexture(
            dstTextureSpec, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
                                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        // (Off-topic) spot-test the defaulting of TexelCopyTextureInfo.aspect.
        wgpu::TextureAspect aspect = wgpu::TextureAspect::Undefined;
        // Perform the texture to texture copy
        RunCopyExternalImageToTexture(srcTextureSpec, srcTexture, dstTextureSpec, dstTexture,
                                      copySize, options, aspect);

        wgpu::Texture result;
        TextureSpec resultSpec = dstTextureSpec;

        // To construct the expected value for the case that dst texture is srgb format,
        // we need to ensure it is byte level equal to the comparable non-srgb format texture.
        // We schedule an copy from srgb texture to non-srgb texture which keeps the bytes
        // same and bypass the sampler to do gamma correction when comparing the expected values
        // in compute shader.
        if (IsDstFormatSrgbFormats()) {
            resultSpec.format = GetNonSrgbFormat(dstTextureSpec.format);
            wgpu::Texture intermediateTexture = CreateTexture(
                resultSpec, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
                                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            // Perform the texture to texture copy
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0});
            wgpu::TexelCopyTextureInfo intermediateTexelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(intermediateTexture, 0, {0, 0, 0});

            encoder.CopyTextureToTexture(&dstTexelCopyTextureInfo,
                                         &intermediateTexelCopyTextureInfo,
                                         &(dstTextureSpec.textureSize));
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            result = intermediateTexture;
        } else {
            result = dstTexture;
        }

        // Check Result
        CheckResultInBuiltInComputePipeline(srcTextureSpec, srcTexture, resultSpec, result,
                                            copySize, options);
    }
};

class CopyTextureForBrowser_SubRects
    : public CopyTextureForBrowserTests<DawnTestWithParams<SubRectTestParams>> {
  protected:
    void DoCopySubRectTest() {
        TextureSpec srcTextureSpec;
        srcTextureSpec.copyOrigin = GetParam().mSrcOrigin;
        srcTextureSpec.textureSize = {6, 7};

        TextureSpec dstTextureSpec;
        dstTextureSpec.copyOrigin = GetParam().mDstOrigin;
        dstTextureSpec.textureSize = {8, 5};
        wgpu::CopyTextureForBrowserOptions options = {};
        options.flipY = GetParam().mFlipY;

        wgpu::Extent3D copySize = GetParam().mCopySize;

        DoTest(srcTextureSpec, dstTextureSpec, copySize, options);
    }
};

class CopyTextureForBrowser_AlphaMode
    : public CopyTextureForBrowserTests<DawnTestWithParams<AlphaTestParams>> {
  protected:
    void DoAlphaModeTest() {
        constexpr uint32_t kWidth = 10;
        constexpr uint32_t kHeight = 10;

        TextureSpec textureSpec;
        textureSpec.textureSize = {kWidth, kHeight};

        wgpu::CopyTextureForBrowserOptions options = {};
        options.srcAlphaMode = GetParam().mSrcAlphaMode;
        options.dstAlphaMode = GetParam().mDstAlphaMode;

        DoTest(textureSpec, textureSpec, {kWidth, kHeight}, options);
    }
};

class CopyTextureForBrowser_ColorSpace
    : public CopyTextureForBrowserTests<DawnTestWithParams<ColorSpaceTestParams>> {
  protected:
    const ColorSpaceInfo& GetColorSpaceInfo(ColorSpace colorSpace) {
        uint32_t index = static_cast<uint32_t>(colorSpace);
        DAWN_ASSERT(index < ColorSpaceTable.size());
        DAWN_ASSERT(ColorSpaceTable[index].index == colorSpace);
        return ColorSpaceTable[index];
    }

    std::array<float, 9> GetConversionMatrix(ColorSpace src, ColorSpace dst) {
        const ColorSpaceInfo& srcColorSpace = GetColorSpaceInfo(src);
        const ColorSpaceInfo& dstColorSpace = GetColorSpaceInfo(dst);

        const std::array<float, 9> toXYZD50 = srcColorSpace.toXYZD50;
        const std::array<float, 9> fromXYZD50 = dstColorSpace.fromXYZD50;

        // Fuse the transform matrix. The color space transformation equation is:
        // Pixels = fromXYZD50 * toXYZD50 * Pixels.
        // Calculate fromXYZD50 * toXYZD50 to simplify
        // Add a padding in each row for Mat3x3 in wgsl uniform(mat3x3, Align(16), Size(48)).
        std::array<float, 9> fuseMatrix = {};

        // Mat3x3 * Mat3x3
        for (uint32_t row = 0; row < 3; ++row) {
            for (uint32_t col = 0; col < 3; ++col) {
                // Transpose the matrix from row major to column major for wgsl.
                fuseMatrix[col * 3 + row] = fromXYZD50[row * 3 + 0] * toXYZD50[col] +
                                            fromXYZD50[row * 3 + 1] * toXYZD50[3 + col] +
                                            fromXYZD50[row * 3 + 2] * toXYZD50[3 * 2 + col];
            }
        }

        return fuseMatrix;
    }

    // TODO(crbug.com/dawn/1140): Generate source data automatically.
    std::vector<utils::RGBA8> GetSourceData(wgpu::AlphaMode srcTextureAlphaMode) {
        if (srcTextureAlphaMode == wgpu::AlphaMode::Premultiplied) {
            return std::vector<utils::RGBA8>{
                utils::RGBA8(0, 102, 102, 102),  // a = 0.4
                utils::RGBA8(102, 0, 0, 102),    // a = 0.4
                utils::RGBA8(153, 0, 0, 153),    // a = 0.6
                utils::RGBA8(255, 0, 0, 255),    // a = 1.0

                utils::RGBA8(153, 0, 153, 153),  // a = 0.6
                utils::RGBA8(0, 102, 0, 102),    // a = 0.4
                utils::RGBA8(0, 153, 0, 153),    // a = 0.6
                utils::RGBA8(0, 255, 0, 255),    // a = 1.0

                utils::RGBA8(255, 255, 0, 255),  // a = 1.0
                utils::RGBA8(0, 0, 102, 102),    // a = 0.4
                utils::RGBA8(0, 0, 153, 153),    // a = 0.6
                utils::RGBA8(0, 0, 255, 255),    // a = 1.0
            };
        }

        return std::vector<utils::RGBA8>{
            // Take RGBA8Unorm as example:
            // R channel has different values
            utils::RGBA8(0, 255, 255, 255),  // r = 0.0
            utils::RGBA8(102, 0, 0, 255),    // r = 0.4
            utils::RGBA8(153, 0, 0, 255),    // r = 0.6
            utils::RGBA8(255, 0, 0, 255),    // r = 1.0

            // G channel has different values
            utils::RGBA8(255, 0, 255, 255),  // g = 0.0
            utils::RGBA8(0, 102, 0, 255),    // g = 0.4
            utils::RGBA8(0, 153, 0, 255),    // g = 0.6
            utils::RGBA8(0, 255, 0, 255),    // g = 1.0

            // B channel has different values
            utils::RGBA8(255, 255, 0, 255),  // b = 0.0
            utils::RGBA8(0, 0, 102, 255),    // b = 0.4
            utils::RGBA8(0, 0, 153, 255),    // b = 0.6
            utils::RGBA8(0, 0, 255, 255),    // b = 1.0
        };
    }

    // TODO(crbug.com/dawn/1140): Current expected values are from ColorSync utils
    // tool on Mac. Should implement CPU or compute shader algorithm to do color
    // conversion and use the result as expected data.
    std::vector<float> GetExpectedData(ColorSpace srcColorSpace,
                                       ColorSpace dstColorSpace,
                                       wgpu::AlphaMode srcTextureAlphaMode,
                                       wgpu::AlphaMode dstTextureAlphaMode) {
        if (srcTextureAlphaMode == wgpu::AlphaMode::Premultiplied) {
            return GetExpectedDataForPremultipliedSource(srcColorSpace, dstColorSpace,
                                                         dstTextureAlphaMode);
        }

        return GetExpectedDataForSeparateSource(srcColorSpace, dstColorSpace);
    }

    std::vector<float> GeneratePremultipliedResult(std::vector<float> result) {
        // Four channels per pixel
        for (uint32_t i = 0; i < result.size(); i += 4) {
            result[i] *= result[i + 3];
            result[i + 1] *= result[i + 3];
            result[i + 2] *= result[i + 3];
        }

        return result;
    }

    std::vector<float> GetExpectedDataForPremultipliedSource(ColorSpace srcColorSpace,
                                                             ColorSpace dstColorSpace,
                                                             wgpu::AlphaMode dstTextureAlphaMode) {
        if (srcColorSpace == dstColorSpace) {
            std::vector<float> expected = {
                0.0, 1.0, 1.0, 0.4,  //
                1.0, 0.0, 0.0, 0.4,  //
                1.0, 0.0, 0.0, 0.6,  //
                1.0, 0.0, 0.0, 1.0,  //

                1.0, 0.0, 1.0, 0.6,  //
                0.0, 1.0, 0.0, 0.4,  //
                0.0, 1.0, 0.0, 0.6,  //
                0.0, 1.0, 0.0, 1.0,  //

                1.0, 1.0, 0.0, 1.0,  //
                0.0, 0.0, 1.0, 0.4,  //
                0.0, 0.0, 1.0, 0.6,  //
                0.0, 0.0, 1.0, 1.0,  //
            };

            return dstTextureAlphaMode == wgpu::AlphaMode::Premultiplied
                       ? GeneratePremultipliedResult(expected)
                       : expected;
        }

        switch (srcColorSpace) {
            case ColorSpace::DisplayP3: {
                switch (dstColorSpace) {
                    case ColorSpace::SRGB: {
                        std::vector<float> expected = {
                            -0.5118, 1.0183,  1.0085,  0.4,  //
                            1.093,   -0.2267, -0.1501, 0.4,  //
                            1.093,   -0.2267, -0.1501, 0.6,  //
                            1.093,   -0.2267, -0.1501, 1.0,  //

                            1.093,   -0.2266, 1.0337,  0.6,  //
                            -0.5118, 1.0183,  -0.3107, 0.4,  //
                            -0.5118, 1.0183,  -0.3107, 0.6,  //
                            -0.5118, 1.0183,  -0.3107, 1.0,  //

                            0.9999,  1.0001,  -0.3462, 1.0,  //
                            0.0002,  0.0004,  1.0419,  0.4,  //
                            0.0002,  0.0004,  1.0419,  0.6,  //
                            0.0002,  0.0004,  1.0419,  1.0,  //
                        };

                        return dstTextureAlphaMode == wgpu::AlphaMode::Premultiplied
                                   ? GeneratePremultipliedResult(expected)
                                   : expected;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            }
            default:
                break;
        }
        DAWN_UNREACHABLE();
    }

    std::vector<float> GetExpectedDataForSeparateSource(ColorSpace srcColorSpace,
                                                        ColorSpace dstColorSpace) {
        if (srcColorSpace == dstColorSpace) {
            return std::vector<float>{
                0.0, 1.0, 1.0, 1.0,  //
                0.4, 0.0, 0.0, 1.0,  //
                0.6, 0.0, 0.0, 1.0,  //
                1.0, 0.0, 0.0, 1.0,  //

                1.0, 0.0, 1.0, 1.0,  //
                0.0, 0.4, 0.0, 1.0,  //
                0.0, 0.6, 0.0, 1.0,  //
                0.0, 1.0, 0.0, 1.0,  //

                1.0, 1.0, 0.0, 1.0,  //
                0.0, 0.0, 0.4, 1.0,  //
                0.0, 0.0, 0.6, 1.0,  //
                0.0, 0.0, 1.0, 1.0,  //
            };
        }

        switch (srcColorSpace) {
            case ColorSpace::DisplayP3: {
                switch (dstColorSpace) {
                    case ColorSpace::SRGB: {
                        return std::vector<float>{
                            -0.5118, 1.0183,  1.0085,  1.0,  //
                            0.4401,  -0.0665, -0.0337, 1.0,  //
                            0.6578,  -0.1199, -0.0723, 1.0,  //
                            1.093,   -0.2267, -0.1501, 1.0,  //

                            1.093,   -0.2266, 1.0337,  1.0,  //
                            -0.1894, 0.4079,  -0.1027, 1.0,  //
                            -0.2969, 0.6114,  -0.1720, 1.0,  //
                            -0.5118, 1.0183,  -0.3107, 1.0,  //

                            0.9999,  1.0001,  -0.3462, 1.0,  //
                            0.0000,  0.0001,  0.4181,  1.0,  //
                            0.0001,  0.0001,  0.6260,  1.0,  //
                            0.0002,  0.0004,  1.0419,  1.0,  //
                        };
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
            }
            default:
                break;
        }
        DAWN_UNREACHABLE();
    }

    void DoColorSpaceConversionTest() {
        constexpr uint32_t kWidth = 12;
        constexpr uint32_t kHeight = 1;

        TextureSpec srcTextureSpec;
        srcTextureSpec.textureSize = {kWidth, kHeight};

        TextureSpec dstTextureSpec;
        dstTextureSpec.textureSize = {kWidth, kHeight};
        dstTextureSpec.format = GetParam().mDstFormat;

        ColorSpace srcColorSpace = GetParam().mSrcColorSpace;
        ColorSpace dstColorSpace = GetParam().mDstColorSpace;

        ColorSpaceInfo srcColorSpaceInfo = GetColorSpaceInfo(srcColorSpace);
        ColorSpaceInfo dstColorSpaceInfo = GetColorSpaceInfo(dstColorSpace);

        std::array<float, 9> matrix = GetConversionMatrix(srcColorSpace, dstColorSpace);

        wgpu::CopyTextureForBrowserOptions options = {};
        options.needsColorSpaceConversion = srcColorSpace != dstColorSpace;
        options.srcAlphaMode = GetParam().mSrcAlphaMode;
        options.srcTransferFunctionParameters = srcColorSpaceInfo.gammaDecodingParams.data();
        options.conversionMatrix = matrix.data();
        options.dstTransferFunctionParameters = dstColorSpaceInfo.gammaEncodingParams.data();
        options.dstAlphaMode = GetParam().mDstAlphaMode;

        std::vector<utils::RGBA8> sourceTextureData = GetSourceData(options.srcAlphaMode);
        const wgpu::Extent3D& copySize = {kWidth, kHeight};

        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                kTextureFormat,
                {srcTextureSpec.textureSize.width, srcTextureSpec.textureSize.height},
                srcTextureSpec.level);

        wgpu::TextureUsage srcUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                      wgpu::TextureUsage::TextureBinding;
        wgpu::Texture srcTexture = this->CreateAndInitTexture(
            srcTextureSpec, srcUsage, srcCopyLayout, sourceTextureData.data(),
            sourceTextureData.size() * sizeof(utils::RGBA8));

        // Create dst texture.
        wgpu::Texture dstTexture = this->CreateTexture(
            dstTextureSpec, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
                                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        // Perform the texture to texture copy
        this->RunCopyExternalImageToTexture(srcTextureSpec, srcTexture, dstTextureSpec, dstTexture,
                                            copySize, options);

        std::vector<float> expectedData = GetExpectedData(
            srcColorSpace, dstColorSpace, options.srcAlphaMode, options.dstAlphaMode);

        // The value provided by Apple's ColorSync Utility.
        float tolerance = 0.001;
        if (dstTextureSpec.format == wgpu::TextureFormat::RGBA16Float) {
            EXPECT_TEXTURE_FLOAT16_EQ(expectedData.data(), dstTexture, {0, 0}, {kWidth, kHeight},
                                      dstTextureSpec.format, tolerance);
        } else {
            EXPECT_TEXTURE_EQ(expectedData.data(), dstTexture, {0, 0}, {kWidth, kHeight},
                              dstTextureSpec.format, tolerance);
        }
    }
};

// Verify CopyTextureForBrowserTests works with internal pipeline.
// The case do copy without any transform.
TEST_P(CopyTextureForBrowser_Basic, PassthroughCopy) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DoBasicCopyTest({10, 1});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyOnXDirection) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DoBasicCopyTest({1000, 1});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyOnYDirection) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DoBasicCopyTest({1, 1000});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyFromLargeTexture) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    DoBasicCopyTest({899, 999});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyFlipY) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;

    DoBasicCopyTest({901, 1001}, options);
}

TEST_P(CopyTextureForBrowser_Basic, VerifyFlipYInSlimTexture) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;

    DoBasicCopyTest({1, 1001}, options);
}

DAWN_INSTANTIATE_TEST(CopyTextureForBrowser_Basic,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Verify |CopyTextureForBrowser| doing color conversion correctly when
// the source texture is RGBA8Unorm format.
TEST_P(CopyTextureForBrowser_Formats, ColorConversion) {
    // BGRA8UnormSrgb is unsupported in Compatibility mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode() &&
                             GetParam().mDstFormat == wgpu::TextureFormat::BGRA8UnormSrgb);

    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    // Skip OpenGL backend on linux because it fails on using *-srgb format as
    // dst texture format
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux() && IsDstFormatSrgbFormats());

    DoColorConversionTest();
}

DAWN_INSTANTIATE_TEST_P(
    CopyTextureForBrowser_Formats,
    {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::TextureFormat>({wgpu::TextureFormat::RGBA8Unorm,
                                      wgpu::TextureFormat::BGRA8Unorm,
                                      wgpu::TextureFormat::RGBA16Float}),
    std::vector<wgpu::TextureFormat>(
        {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R16Float, wgpu::TextureFormat::R32Float,
         wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RG16Float,
         wgpu::TextureFormat::RG32Float, wgpu::TextureFormat::RGBA8Unorm,
         wgpu::TextureFormat::RGBA8UnormSrgb, wgpu::TextureFormat::BGRA8Unorm,
         wgpu::TextureFormat::BGRA8UnormSrgb, wgpu::TextureFormat::RGB10A2Unorm,
         wgpu::TextureFormat::RGBA16Float, wgpu::TextureFormat::RGBA32Float}));

// Verify |CopyTextureForBrowser| doing subrect copy.
// Source texture is a full red texture and dst texture is a full
// green texture originally. After the subrect copy, affected part
// in dst texture should be red and other part should remain green.
TEST_P(CopyTextureForBrowser_SubRects, CopySubRect) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    // Tests skip due to crbug.com/dawn/592.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    // Tests skip due to crbug.com/dawn/1104.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    DoCopySubRectTest();
}

DAWN_INSTANTIATE_TEST_P(CopyTextureForBrowser_SubRects,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::Origin3D>({{1, 1}, {1, 2}, {2, 1}}),
                        std::vector<wgpu::Origin3D>({{1, 1}, {1, 2}, {2, 1}}),
                        std::vector<wgpu::Extent3D>({{1, 1}, {2, 1}, {1, 2}, {2, 2}}),
                        std::vector<bool>({true, false}));

// Verify |CopyTextureForBrowser| doing alpha changes.
// Test srcAlphaMode and dstAlphaMode: Premultiplied, Unpremultiplied.
TEST_P(CopyTextureForBrowser_AlphaMode, alphaMode) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    // Tests skip due to crbug.com/dawn/1104.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    DoAlphaModeTest();
}

DAWN_INSTANTIATE_TEST_P(
    CopyTextureForBrowser_AlphaMode,
    {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
     VulkanBackend()},
    std::vector<wgpu::AlphaMode>({wgpu::AlphaMode::Premultiplied, wgpu::AlphaMode::Unpremultiplied,
                                  wgpu::AlphaMode::Opaque}),
    std::vector<wgpu::AlphaMode>({wgpu::AlphaMode::Premultiplied, wgpu::AlphaMode::Unpremultiplied,
                                  wgpu::AlphaMode::Opaque}));

// Verify |CopyTextureForBrowser| doing color space conversion.
TEST_P(CopyTextureForBrowser_ColorSpace, colorSpaceConversion) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux());

    // Tests skip due to crbug.com/dawn/1104.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    DoColorSpaceConversionTest();
}

DAWN_INSTANTIATE_TEST_P(CopyTextureForBrowser_ColorSpace,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>({wgpu::TextureFormat::RGBA16Float,
                                                          wgpu::TextureFormat::RGBA32Float}),
                        std::vector<ColorSpace>({ColorSpace::SRGB, ColorSpace::DisplayP3}),
                        std::vector<ColorSpace>({ColorSpace::SRGB}),
                        std::vector<wgpu::AlphaMode>({wgpu::AlphaMode::Premultiplied,
                                                      wgpu::AlphaMode::Unpremultiplied}),
                        std::vector<wgpu::AlphaMode>({wgpu::AlphaMode::Premultiplied,
                                                      wgpu::AlphaMode::Unpremultiplied}));

}  // anonymous namespace
}  // namespace dawn
