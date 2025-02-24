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

#include "dawn/native/CopyTextureForBrowserHelper.h"

#include <utility>

#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {
static const char sCopyForBrowserShader[] = R"(
                struct GammaTransferParamsInternal {
                G: f32,
                A: f32,
                B: f32,
                C: f32,
                D: f32,
                E: f32,
                F: f32,
                padding: u32,
            };

            struct Uniforms {                                                    // offset   align   size
                scale: vec2f,                                                // 0        8       8
                offset: vec2f,                                               // 8        8       8
                steps_mask: u32,                                                 // 16       4       4
                // implicit padding;                                             // 20               12
                conversion_matrix: mat3x3<f32>,                                  // 32       16      48
                gamma_decoding_params: GammaTransferParamsInternal,              // 80       4       32
                gamma_encoding_params: GammaTransferParamsInternal,              // 112      4       32
                gamma_decoding_for_dst_srgb_params: GammaTransferParamsInternal, // 144      4       32
            };

            @binding(0) @group(0) var<uniform> uniforms : Uniforms;

            struct VertexOutputs {
                @location(0) texcoords : vec2f,
                @builtin(position) position : vec4f,
            };

            // Chromium uses unified equation to construct gamma decoding function
            // and gamma encoding function.
            // The logic is:
            //  if x < D
            //      linear = C * x + F
            //  nonlinear = pow(A * x + B, G) + E
            // (https://source.chromium.org/chromium/chromium/src/+/main:ui/gfx/color_transform.cc;l=541)
            // Expand the equation with sign() to make it handle all gamma conversions.
            fn gamma_conversion(v: f32, params: GammaTransferParamsInternal) -> f32 {
                // Linear part: C * x + F
                if (abs(v) < params.D) {
                    return sign(v) * (params.C * abs(v) + params.F);
                }

                // Gamma part: pow(A * x + B, G) + E
                return sign(v) * (pow(params.A * abs(v) + params.B, params.G) + params.E);
            }

            @vertex
            fn vs_main(
                @builtin(vertex_index) VertexIndex : u32
            ) -> VertexOutputs {
                var texcoord = array(
                    vec2f(-0.5, 0.0),
                    vec2f( 1.5, 0.0),
                    vec2f( 0.5, 2.0));

                var output : VertexOutputs;
                output.position = vec4f((texcoord[VertexIndex] * 2.0 - vec2f(1.0, 1.0)), 0.0, 1.0);
                output.texcoords = texcoord[VertexIndex] * uniforms.scale + uniforms.offset;

                return output;
            }

            @binding(1) @group(0) var mySampler: sampler;

            // Resource used in copyTexture entry point only.
            @binding(2) @group(0) var mySourceTexture: texture_2d<f32>;

            // Resource used in copyExternalTexture entry point only.
            @binding(2) @group(0) var mySourceExternalTexture: texture_external;

            fn discardIfOutsideOfCopy(texcoord : vec2f) {
                var clampedTexcoord =
                    clamp(texcoord, vec2f(0.0, 0.0), vec2f(1.0, 1.0));
                if (!all(clampedTexcoord == texcoord)) {
                    discard;
                }
            }

            fn transform(srcColor : vec4f) -> vec4f {
                var color = srcColor;
                let kUnpremultiplyStep = 0x01u;
                let kDecodeToLinearStep = 0x02u;
                let kConvertToDstGamutStep = 0x04u;
                let kEncodeToGammaStep = 0x08u;
                let kPremultiplyStep = 0x10u;
                let kDecodeForSrgbDstFormat = 0x20u;
                let kClearSrcAlphaToOne = 0x40u;

                // Unpremultiply step. Appling color space conversion op on premultiplied source texture
                // also needs to unpremultiply first.
                // This step is exclusive with clear src alpha to one step.
                if (bool(uniforms.steps_mask & kUnpremultiplyStep)) {
                    if (color.a != 0.0) {
                        color = vec4f(color.rgb / color.a, color.a);
                    }
                }

                // Linearize the source color using the source color space’s
                // transfer function if it is non-linear.
                if (bool(uniforms.steps_mask & kDecodeToLinearStep)) {
                    color = vec4f(gamma_conversion(color.r, uniforms.gamma_decoding_params),
                                      gamma_conversion(color.g, uniforms.gamma_decoding_params),
                                      gamma_conversion(color.b, uniforms.gamma_decoding_params),
                                      color.a);
                }

                // Convert unpremultiplied, linear source colors to the destination gamut by
                // multiplying by a 3x3 matrix. Calculate transformFromXYZD50 * transformToXYZD50
                // in CPU side and upload the final result in uniforms.
                if (bool(uniforms.steps_mask & kConvertToDstGamutStep)) {
                    color = vec4f(uniforms.conversion_matrix * color.rgb, color.a);
                }

                // Encode that color using the inverse of the destination color
                // space’s transfer function if it is non-linear.
                if (bool(uniforms.steps_mask & kEncodeToGammaStep)) {
                    color = vec4f(gamma_conversion(color.r, uniforms.gamma_encoding_params),
                                      gamma_conversion(color.g, uniforms.gamma_encoding_params),
                                      gamma_conversion(color.b, uniforms.gamma_encoding_params),
                                      color.a);
                }

                // Premultiply step.
                // This step is exclusive with clear src alpha to one step.
                if (bool(uniforms.steps_mask & kPremultiplyStep)) {
                    color = vec4f(color.rgb * color.a, color.a);
                }

                // Decode for copying from non-srgb formats to srgb formats
                if (bool(uniforms.steps_mask & kDecodeForSrgbDstFormat)) {
                    color = vec4f(gamma_conversion(color.r, uniforms.gamma_decoding_for_dst_srgb_params),
                                      gamma_conversion(color.g, uniforms.gamma_decoding_for_dst_srgb_params),
                                      gamma_conversion(color.b, uniforms.gamma_decoding_for_dst_srgb_params),
                                      color.a);
                }

                // Clear alpha to one step.
                // This step is exclusive with premultiply/unpremultiply step.
                if (bool(uniforms.steps_mask & kClearSrcAlphaToOne)) {
                    color.a = 1.0;
                }

                return color;
            }

            @fragment
            fn copyTexture(@location(0) texcoord : vec2f
            ) -> @location(0) vec4f {
                discardIfOutsideOfCopy(texcoord);

                var color = textureSample(mySourceTexture, mySampler, texcoord);
                return transform(color);
            }

            @fragment
            fn copyExternalTexture(@location(0) texcoord : vec2f
            ) -> @location(0) vec4f {
                discardIfOutsideOfCopy(texcoord);

                var color = textureSampleBaseClampToEdge(mySourceExternalTexture, mySampler, texcoord);
                return transform(color);
            }
        )";

// Follow the same order of skcms_TransferFunction
// https://source.chromium.org/chromium/chromium/src/+/main:third_party/skia/include/third_party/skcms/skcms.h;l=46;
struct GammaTransferParamsInternal {
    float G = 0.0;
    float A = 0.0;
    float B = 0.0;
    float C = 0.0;
    float D = 0.0;
    float E = 0.0;
    float F = 0.0;
    uint32_t padding = 0;
};

struct Uniform {
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
    uint32_t stepsMask = 0;
    const std::array<uint32_t, 3> padding = {};  // 12 bytes padding
    std::array<float, 12> conversionMatrix = {};
    GammaTransferParamsInternal gammaDecodingParams = {};
    GammaTransferParamsInternal gammaEncodingParams = {};
    GammaTransferParamsInternal gammaDecodingForDstSrgbParams = {};
};
static_assert(sizeof(Uniform) == 176);

enum class SourceTextureType { Texture2D, ExternalTexture };

struct TextureInfo {
    Origin3D origin;
    Extent3D size;
};

// TODO(crbug.com/dawn/856): Expand copyTextureForBrowser to support any
// non-depth, non-stencil, non-compressed texture format pair copy.
MaybeError ValidateCopyTextureSourceFormat(const wgpu::TextureFormat srcFormat) {
    switch (srcFormat) {
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA16Float:
            break;
        default:
            return DAWN_VALIDATION_ERROR("Source texture format (%s) is not supported.", srcFormat);
    }

    return {};
}

MaybeError ValidateCopyForBrowserDestinationFormat(const wgpu::TextureFormat dstFormat) {
    switch (dstFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
            break;
        default:
            return DAWN_VALIDATION_ERROR("Destination texture format (%s) is not supported.",
                                         dstFormat);
    }

    return {};
}

RenderPipelineBase* GetCachedCopyTexturePipeline(InternalPipelineStore* store,
                                                 wgpu::TextureFormat dstFormat) {
    auto pipeline = store->copyTextureForBrowserPipelines.find(dstFormat);
    if (pipeline != store->copyTextureForBrowserPipelines.end()) {
        return pipeline->second.Get();
    }
    return nullptr;
}

RenderPipelineBase* GetCachedCopyExternalTexturePipeline(InternalPipelineStore* store,
                                                         wgpu::TextureFormat dstFormat) {
    auto pipeline = store->copyExternalTextureForBrowserPipelines.find(dstFormat);
    if (pipeline != store->copyExternalTextureForBrowserPipelines.end()) {
        return pipeline->second.Get();
    }
    return nullptr;
}

ResultOrError<Ref<RenderPipelineBase>> CreateCopyForBrowserPipeline(
    DeviceBase* device,
    wgpu::TextureFormat dstFormat,
    ShaderModuleBase* shaderModule,
    const char* fragmentEntryPoint) {
    // Prepare vertex stage.
    VertexState vertex = {};
    vertex.module = shaderModule;
    vertex.entryPoint = "vs_main";

    // Prepare frgament stage.
    FragmentState fragment = {};
    fragment.module = shaderModule;
    fragment.entryPoint = fragmentEntryPoint;

    // Prepare color state.
    ColorTargetState target = {};
    target.format = dstFormat;

    // Create RenderPipeline.
    RenderPipelineDescriptor renderPipelineDesc = {};

    // Generate the layout based on shader modules.
    renderPipelineDesc.layout = nullptr;

    renderPipelineDesc.vertex = vertex;
    renderPipelineDesc.fragment = &fragment;

    renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    fragment.targetCount = 1;
    fragment.targets = &target;

    return device->CreateRenderPipeline(&renderPipelineDesc);
}

ResultOrError<ShaderModuleBase*> GetOrCreateCopyForBrowserShaderModule(
    DeviceBase* device,
    InternalPipelineStore* store) {
    if (store->copyForBrowser == nullptr) {
        DAWN_TRY_ASSIGN(store->copyForBrowser,
                        utils::CreateShaderModule(device, sCopyForBrowserShader));
    }

    return store->copyForBrowser.Get();
}

ResultOrError<RenderPipelineBase*> GetOrCreateCopyTextureForBrowserPipeline(
    DeviceBase* device,
    wgpu::TextureFormat dstFormat) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    if (GetCachedCopyTexturePipeline(store, dstFormat) == nullptr) {
        ShaderModuleBase* shaderModule;
        DAWN_TRY_ASSIGN(shaderModule, GetOrCreateCopyForBrowserShaderModule(device, store));
        Ref<RenderPipelineBase> pipeline;
        DAWN_TRY_ASSIGN(
            pipeline, CreateCopyForBrowserPipeline(device, dstFormat, shaderModule, "copyTexture"));
        store->copyTextureForBrowserPipelines.emplace(dstFormat, std::move(pipeline));
    }

    return GetCachedCopyTexturePipeline(store, dstFormat);
}

ResultOrError<RenderPipelineBase*> GetOrCreateCopyExternalTextureForBrowserPipeline(
    DeviceBase* device,
    wgpu::TextureFormat dstFormat) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    if (GetCachedCopyExternalTexturePipeline(store, dstFormat) == nullptr) {
        ShaderModuleBase* shaderModule;
        DAWN_TRY_ASSIGN(shaderModule, GetOrCreateCopyForBrowserShaderModule(device, store));
        Ref<RenderPipelineBase> pipeline;
        DAWN_TRY_ASSIGN(pipeline, CreateCopyForBrowserPipeline(device, dstFormat, shaderModule,
                                                               "copyExternalTexture"));
        store->copyExternalTextureForBrowserPipelines.emplace(dstFormat, std::move(pipeline));
    }

    return GetCachedCopyExternalTexturePipeline(store, dstFormat);
}

// Whether the format of dst texture of CopyTextureForBrowser() is srgb or non-srgb.
bool IsSrgbDstFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return true;
        default:
            return false;
    }
}

template <typename T>
MaybeError DoCopyForBrowser(DeviceBase* device,
                            const TextureInfo* sourceInfo,
                            T* sourceResource,
                            const TexelCopyTextureInfo* destination,
                            const Extent3D* copySize,
                            const CopyTextureForBrowserOptions* options,
                            RenderPipelineBase* pipeline) {
    // TODO(crbug.com/dawn/856): In D3D12 and Vulkan, compatible texture format can directly
    // copy to each other. This can be a potential fast path.

    // Noop copy
    if (copySize->width == 0 || copySize->height == 0 || copySize->depthOrArrayLayers == 0) {
        return {};
    }

    // Prepare bind group layout.
    Ref<BindGroupLayoutBase> layout;
    DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

    // Prepare binding 0 resource: uniform buffer.
    Uniform uniformData = {
        copySize->width / static_cast<float>(sourceInfo->size.width),
        copySize->height / static_cast<float>(sourceInfo->size.height),  // scale
        sourceInfo->origin.x / static_cast<float>(sourceInfo->size.width),
        sourceInfo->origin.y / static_cast<float>(sourceInfo->size.height)  // offset
    };

    // The NDC to framebuffer space transform maps inverts the Y coordinate such that NDC [-1, 1]
    // (resp [-1, -1]) maps to framebuffer space [0, 0] (resp [0, height-1]). So we need to undo
    // this flip when converting positions to texcoords.
    // https://www.w3.org/TR/webgpu/#coordinate-systems
    if (!options->flipY) {
        uniformData.scaleY *= -1.0;
        uniformData.offsetY += copySize->height / static_cast<float>(sourceInfo->size.height);
    }

    uint32_t stepsMask = 0u;

    // Steps to do color space conversion
    // From https://skia.org/docs/user/color/
    // - unpremultiply if the source color is premultiplied; Alpha is not involved in color
    // management, and we need to divide it out if it’s multiplied in.
    // - linearize the source color using the source color space’s transfer function
    // - convert those unpremultiplied, linear source colors to XYZ D50 gamut by multiplying by
    // a 3x3 matrix.
    // - convert those XYZ D50 colors to the destination gamut by multiplying by a 3x3 matrix.
    // - encode that color using the inverse of the destination color space’s transfer function.
    // - premultiply by alpha if the destination is premultiplied.
    // The reason to choose XYZ D50 as intermediate color space:
    // From http://www.brucelindbloom.com/index.html?WorkingSpaceInfo.html
    // "Since the Lab TIFF specification, the ICC profile specification and
    // Adobe Photoshop all use a D50"
    constexpr uint32_t kUnpremultiplyStep = 0x01;
    constexpr uint32_t kDecodeToLinearStep = 0x02;
    constexpr uint32_t kConvertToDstGamutStep = 0x04;
    constexpr uint32_t kEncodeToGammaStep = 0x08;
    constexpr uint32_t kPremultiplyStep = 0x10;
    constexpr uint32_t kDecodeForSrgbDstFormat = 0x20;
    constexpr uint32_t kClearSrcAlphaToOne = 0x40;

    if (options->srcAlphaMode == wgpu::AlphaMode::Premultiplied) {
        if (options->needsColorSpaceConversion ||
            options->dstAlphaMode == wgpu::AlphaMode::Unpremultiplied) {
            stepsMask |= kUnpremultiplyStep;
        }
    } else if (options->srcAlphaMode == wgpu::AlphaMode::Opaque) {
        // Simply clear src alpha channel to 1.0
        stepsMask |= kClearSrcAlphaToOne;
    }

    if (options->needsColorSpaceConversion) {
        stepsMask |= kDecodeToLinearStep;
        const float* decodingParams = options->srcTransferFunctionParameters;

        uniformData.gammaDecodingParams = {decodingParams[0], decodingParams[1], decodingParams[2],
                                           decodingParams[3], decodingParams[4], decodingParams[5],
                                           decodingParams[6]};

        stepsMask |= kConvertToDstGamutStep;
        const float* matrix = options->conversionMatrix;
        uniformData.conversionMatrix = {{
            matrix[0],
            matrix[1],
            matrix[2],
            0.0,
            matrix[3],
            matrix[4],
            matrix[5],
            0.0,
            matrix[6],
            matrix[7],
            matrix[8],
            0.0,
        }};

        stepsMask |= kEncodeToGammaStep;
        const float* encodingParams = options->dstTransferFunctionParameters;

        uniformData.gammaEncodingParams = {encodingParams[0], encodingParams[1], encodingParams[2],
                                           encodingParams[3], encodingParams[4], encodingParams[5],
                                           encodingParams[6]};
    }

    if (options->dstAlphaMode == wgpu::AlphaMode::Premultiplied) {
        if (options->needsColorSpaceConversion ||
            options->srcAlphaMode == wgpu::AlphaMode::Unpremultiplied) {
            stepsMask |= kPremultiplyStep;
        }
    }

    // Copy to *-srgb texture should keep the bytes exactly the same as copy
    // to non-srgb texture. Add an extra decode-to-linear step so that after the
    // sampler of *-srgb format texture applying encoding, the bytes keeps the same
    // as non-srgb format texture.
    // NOTE: CopyTextureForBrowser() doesn't need to accept *-srgb format texture as
    // source input. But above operation also valid for *-srgb format texture input and
    // non-srgb format dst texture.
    // TODO(crbug.com/dawn/1195): Reinterpret to non-srgb texture view on *-srgb texture
    // and use it as render attachment when possible.
    // TODO(crbug.com/dawn/1195): Opt the condition for this extra step. It is possible to
    // bypass this extra step in some cases.
    bool isSrgbDstFormat = IsSrgbDstFormat(destination->texture->GetFormat().format);
    if (isSrgbDstFormat) {
        stepsMask |= kDecodeForSrgbDstFormat;
        // Get gamma-linear conversion params from https://en.wikipedia.org/wiki/SRGB with some
        // mathematics. Order: {G, A, B, C, D, E, F, }
        uniformData.gammaDecodingForDstSrgbParams = {
            2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 4.045e-02, 0.0, 0.0};
    }

    // Upload uniform data
    uniformData.stepsMask = stepsMask;

    Ref<BufferBase> uniformBuffer;
    DAWN_TRY_ASSIGN(
        uniformBuffer,
        utils::CreateBufferFromData(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                    {uniformData}));

    // Prepare binding 1 resource: sampler
    // Use default configuration, filterMode set to Nearest for min and mag.
    SamplerDescriptor samplerDesc = {};
    Ref<SamplerBase> sampler;
    DAWN_TRY_ASSIGN(sampler, device->CreateSampler(&samplerDesc));

    // Create bind group after all binding entries are set.
    UsageValidationMode mode =
        options->internalUsage ? UsageValidationMode::Internal : UsageValidationMode::Default;
    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup, utils::MakeBindGroup(
                                   device, layout,
                                   {{0, uniformBuffer}, {1, sampler}, {2, sourceResource}}, mode));

    // Create command encoder.
    CommandEncoderDescriptor commandEncoderDesc;
    DawnEncoderInternalUsageDescriptor internalUsageDesc;
    if (options->internalUsage) {
        internalUsageDesc.useInternalUsages = true;
        commandEncoderDesc.nextInChain = &internalUsageDesc;
    }
    Ref<CommandEncoder> encoder;
    DAWN_TRY_ASSIGN(encoder, device->CreateCommandEncoder(&commandEncoderDesc));

    // Prepare dst texture view as color Attachment.
    TextureViewDescriptor dstTextureViewDesc;
    dstTextureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    dstTextureViewDesc.baseMipLevel = destination->mipLevel;
    dstTextureViewDesc.mipLevelCount = 1;
    dstTextureViewDesc.baseArrayLayer = destination->origin.z;
    dstTextureViewDesc.arrayLayerCount = 1;
    Ref<TextureViewBase> dstView;

    DAWN_TRY_ASSIGN(dstView, device->CreateTextureView(destination->texture, &dstTextureViewDesc));
    // Prepare render pass color attachment descriptor.
    RenderPassColorAttachment colorAttachmentDesc;

    colorAttachmentDesc.view = dstView.Get();
    colorAttachmentDesc.loadOp = wgpu::LoadOp::Load;
    colorAttachmentDesc.storeOp = wgpu::StoreOp::Store;
    colorAttachmentDesc.clearValue = {0.0, 0.0, 0.0, 1.0};

    // Create render pass.
    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachmentDesc;
    Ref<RenderPassEncoder> passEncoder = encoder->BeginRenderPass(&renderPassDesc);

    // Start pipeline and encode commands to complete
    // the copy from src texture to dst texture with transformation.
    passEncoder->APISetPipeline(pipeline);
    passEncoder->APISetBindGroup(0, bindGroup.Get());
    passEncoder->APISetViewport(destination->origin.x, destination->origin.y, copySize->width,
                                copySize->height, 0.0, 1.0);
    passEncoder->APIDraw(3);
    passEncoder->End();

    // Finsh encoding.
    Ref<CommandBufferBase> commandBuffer;
    DAWN_TRY_ASSIGN(commandBuffer, encoder->Finish());
    CommandBufferBase* submitCommandBuffer = commandBuffer.Get();

    // Submit command buffer.
    device->GetQueue()->APISubmit(1, &submitCommandBuffer);
    return {};
}

MaybeError ValidateCopyForBrowserDestination(DeviceBase* device,
                                             const TexelCopyTextureInfo& destination,
                                             const Extent3D& copySize,
                                             const CopyTextureForBrowserOptions& options) {
    DAWN_TRY(device->ValidateObject(destination.texture));
    DAWN_TRY(destination.texture->ValidateCanUseInSubmitNow());
    DAWN_TRY_CONTEXT(ValidateTexelCopyTextureInfo(device, destination, copySize),
                     "validating the TexelCopyTextureInfo for the destination");
    DAWN_TRY_CONTEXT(ValidateTextureCopyRange(device, destination, copySize),
                     "validating that the copy fits in the destination");

    UsageValidationMode mode =
        options.internalUsage ? UsageValidationMode::Internal : UsageValidationMode::Default;
    DAWN_TRY(ValidateCanUseAs(destination.texture, wgpu::TextureUsage::CopyDst, mode));
    DAWN_TRY(ValidateCanUseAs(destination.texture, wgpu::TextureUsage::RenderAttachment, mode));

    DAWN_INVALID_IF(destination.texture->GetSampleCount() > 1,
                    "The destination texture sample count (%u) is not 1.",
                    destination.texture->GetSampleCount());

    DAWN_TRY(ValidateCopyForBrowserDestinationFormat(destination.texture->GetFormat().format));

    // The valid destination formats are all color formats.
    DAWN_INVALID_IF(
        destination.aspect != wgpu::TextureAspect::All,
        "Destination %s aspect (%s) doesn't select all the aspects of the destination format.",
        destination.texture, destination.aspect);

    return {};
}

MaybeError ValidateCopyForBrowserOptions(const CopyTextureForBrowserOptions& options) {
    DAWN_INVALID_IF(options.nextInChain != nullptr, "nextInChain must be nullptr");

    DAWN_TRY(ValidateAlphaMode(options.srcAlphaMode));
    DAWN_TRY(ValidateAlphaMode(options.dstAlphaMode));

    if (options.needsColorSpaceConversion) {
        DAWN_INVALID_IF(options.srcTransferFunctionParameters == nullptr,
                        "srcTransferFunctionParameters is nullptr when doing color conversion");
        DAWN_INVALID_IF(options.conversionMatrix == nullptr,
                        "conversionMatrix is nullptr when doing color conversion");
        DAWN_INVALID_IF(options.dstTransferFunctionParameters == nullptr,
                        "dstTransferFunctionParameters is nullptr when doing color conversion");
    }
    return {};
}
}  // anonymous namespace

MaybeError ValidateCopyTextureForBrowser(DeviceBase* device,
                                         const TexelCopyTextureInfo* source,
                                         const TexelCopyTextureInfo* destination,
                                         const Extent3D* copySize,
                                         const CopyTextureForBrowserOptions* options) {
    // Validate source
    DAWN_TRY(device->ValidateObject(source->texture));
    DAWN_TRY(source->texture->ValidateCanUseInSubmitNow());
    DAWN_TRY_CONTEXT(ValidateTexelCopyTextureInfo(device, *source, *copySize),
                     "validating the TexelCopyTextureInfo for the source");
    DAWN_TRY_CONTEXT(ValidateTextureCopyRange(device, *source, *copySize),
                     "validating that the copy fits in the source");
    DAWN_INVALID_IF(source->origin.z > 0, "Source has a non-zero z origin (%u).", source->origin.z);
    DAWN_INVALID_IF(source->texture->GetSampleCount() > 1,
                    "The source texture sample count (%u) is not 1. ",
                    source->texture->GetSampleCount());
    DAWN_INVALID_IF(
        options->internalUsage && !device->HasFeature(Feature::DawnInternalUsages),
        "The internalUsage is true while the dawn-internal-usages feature is not enabled.");
    UsageValidationMode mode =
        options->internalUsage ? UsageValidationMode::Internal : UsageValidationMode::Default;
    DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc, mode));
    DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::TextureBinding, mode));
    DAWN_TRY(ValidateCopyTextureSourceFormat(source->texture->GetFormat().format));

    // Validate destination
    DAWN_TRY(ValidateCopyForBrowserDestination(device, *destination, *copySize, *options));

    // Validate copy common rules and copySize.
    DAWN_INVALID_IF(copySize->depthOrArrayLayers > 1, "Copy is for more than one array layer (%u)",
                    copySize->depthOrArrayLayers);
    DAWN_TRY(
        ValidateTextureToTextureCopyCommonRestrictions(device, *source, *destination, *copySize));

    // Validate options
    DAWN_TRY(ValidateCopyForBrowserOptions(*options));

    return {};
}

MaybeError ValidateCopyExternalTextureForBrowser(DeviceBase* device,
                                                 const ImageCopyExternalTexture* source,
                                                 const TexelCopyTextureInfo* destination,
                                                 const Extent3D* copySize,
                                                 const CopyTextureForBrowserOptions* options) {
    // Validate source
    DAWN_TRY(device->ValidateObject(source->externalTexture));
    DAWN_TRY(source->externalTexture->ValidateCanUseInSubmitNow());

    Extent2D sourceSize;
    sourceSize.width = source->naturalSize.width;
    sourceSize.height = source->naturalSize.height;

    // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
    // overflows.
    DAWN_INVALID_IF(
        static_cast<uint64_t>(source->origin.x) + static_cast<uint64_t>(copySize->width) >
                static_cast<uint64_t>(sourceSize.width) ||
            static_cast<uint64_t>(source->origin.y) + static_cast<uint64_t>(copySize->height) >
                static_cast<uint64_t>(sourceSize.height) ||
            static_cast<uint64_t>(source->origin.z) > 0,
        "Texture copy range (origin: %s, copySize: %s) touches outside of %s source size (%s).",
        &source->origin, copySize, source->externalTexture, &sourceSize);
    DAWN_INVALID_IF(source->origin.z > 0, "Source has a non-zero z origin (%u).", source->origin.z);
    DAWN_INVALID_IF(
        options->internalUsage && !device->HasFeature(Feature::DawnInternalUsages),
        "The internalUsage is true while the dawn-internal-usages feature is not enabled.");

    // Validate destination
    DAWN_TRY(ValidateCopyForBrowserDestination(device, *destination, *copySize, *options));

    // Validate copySize
    DAWN_INVALID_IF(copySize->depthOrArrayLayers > 1, "Copy is for more than one array layer (%u)",
                    copySize->depthOrArrayLayers);

    // Validate options
    DAWN_TRY(ValidateCopyForBrowserOptions(*options));

    return {};
}

MaybeError DoCopyExternalTextureForBrowser(DeviceBase* device,
                                           const ImageCopyExternalTexture* source,
                                           const TexelCopyTextureInfo* destination,
                                           const Extent3D* copySize,
                                           const CopyTextureForBrowserOptions* options) {
    TextureInfo info;
    info.origin = source->origin;
    info.size = {source->naturalSize.width, source->naturalSize.height, 1};

    RenderPipelineBase* pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateCopyExternalTextureForBrowserPipeline(
                                  device, destination->texture->GetFormat().format));
    return DoCopyForBrowser<ExternalTextureBase>(device, &info, source->externalTexture,
                                                 destination, copySize, options, pipeline);
}

MaybeError DoCopyTextureForBrowser(DeviceBase* device,
                                   const TexelCopyTextureInfo* source,
                                   const TexelCopyTextureInfo* destination,
                                   const Extent3D* copySize,
                                   const CopyTextureForBrowserOptions* options) {
    TextureInfo info;
    info.origin = source->origin;
    info.size = source->texture->GetSize(source->aspect);

    Ref<TextureViewBase> srcTextureView = nullptr;
    TextureViewDescriptor srcTextureViewDesc = {};
    srcTextureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    srcTextureViewDesc.baseMipLevel = source->mipLevel;
    srcTextureViewDesc.mipLevelCount = 1;
    srcTextureViewDesc.arrayLayerCount = 1;
    DAWN_TRY_ASSIGN(srcTextureView,
                    device->CreateTextureView(source->texture, &srcTextureViewDesc));

    RenderPipelineBase* pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateCopyTextureForBrowserPipeline(
                                  device, destination->texture->GetFormat().format));

    return DoCopyForBrowser<TextureViewBase>(device, &info, srcTextureView.Get(), destination,
                                             copySize, options, pipeline);
}
}  // namespace dawn::native
