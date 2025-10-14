/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WebGPUBlitter.h"

#include "WebGPUConstants.h"
#include "WebGPUTextureHelpers.h"
#include "webgpu/utils/StringPlaceholderTemplateProcessor.h"

#include <backend/DriverEnums.h>

#include <math/vec2.h>
#include <utils/Hash.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <string.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace filament::backend {

namespace {

/**
 * @return true if copyToTextureToTexture validation would succeed and we could use that for the
 * "blit"
 * ( see https://www.w3.org/TR/webgpu/#dom-gpucommandencoder-copytexturetotexture )
 * NOTE: this does not validate usage of the source and/or destination.
 *       If canDoDirectCopy(...) == true and the textures don't have the appropriate usages
 *       we should panic (we should not be doing a renderpass blit when we could have been
 *       doing a direct copy)
 */
[[nodiscard]] constexpr bool canDoDirectCopy(const uint32_t sourceSampleCount,
        const wgpu::TextureFormat sourceFormat, const wgpu::Origin2D sourceOrigin,
        const wgpu::Extent2D sourceExtent, const uint32_t destinationSampleCount,
        const wgpu::TextureFormat destinationFormat, const wgpu::Origin2D destinationOrigin,
        const wgpu::Extent2D destinationExtent) {
    const wgpu::Extent2D sourceBlockSize{ getBlockSize(sourceFormat) };
    const wgpu::Extent2D destinationBlockSize{ getBlockSize(destinationFormat) };
    return sourceSampleCount == destinationSampleCount &&
           areCopyCompatible(sourceFormat, destinationFormat) &&
           (sourceExtent.width == destinationExtent.width) &&
           (sourceExtent.height == destinationExtent.height) &&
           ((sourceOrigin.x % sourceBlockSize.width) == 0) &&
           ((sourceOrigin.y % sourceBlockSize.height) == 0) &&
           ((destinationOrigin.x % destinationBlockSize.width) == 0) &&
           ((destinationOrigin.y % destinationBlockSize.height) == 0);
}

struct BlitFragmentShaderArgs final {
    uint32_t depthPlane{ 0 };
    uint32_t padding{ 0 }; // Add 4 bytes of padding to align the vec2 members
    math::float2 scale{ 0.0f, 0.0f };
    math::uint2 sourceOffset{ 0, 0 };
    math::uint2 destinationOffset{ 0, 0 };
};
static_assert(sizeof(BlitFragmentShaderArgs) == 32,
        "BlitFragmentShaderArgs must not have implicit padding.");

// Note: would be nice to generate the constexpr string for the shader source from the constexpr
// variables below, e.g. TEXTURE_BINDING_INDEX, to avoid them getting out of sync while still having
// the compile/build time optimization
constexpr uint32_t TEXTURE_BIND_GROUP_INDEX{ 0 };
constexpr size_t MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE{ 3 }; // texture and uniform and sampler
constexpr uint32_t TEXTURE_BINDING_INDEX{ 0 };
constexpr uint32_t UNIFORM_BINDING_INDEX{ 1 };
constexpr uint32_t SAMPLER_BINDING_INDEX{ 2 };
constexpr std::string_view VERTEX_SHADER_ENTRY_POINT{ "vertexShaderMain" };
constexpr std::string_view FRAGMENT_SHADER_ENTRY_POINT{ "fragmentShaderMain" };
// note that the placeholders below must start and end with this prefix and suffix and should not
// otherwise be present in the template:
constexpr std::string_view PLACEHOLDER_PREFIX{ "{{" };
constexpr std::string_view PLACEHOLDER_SUFFIX{ "}}" };

#define PLACEHOLDER_DEF(name) constexpr std::string_view name = #name

PLACEHOLDER_DEF(FRAGMENT_SHADER_SNIPPET);
PLACEHOLDER_DEF(TEXTURE_SAMPLE_IMPL);
PLACEHOLDER_DEF(TEXTURE_SAMPLE_IMPL_MAIN);

// texture_multisampled_2d<f32>
// texture_2d<f32/i32/u32>
// texture_3d<f32/i32/u32>
// texture_depth_2d
// texture_depth_multisampled_2d
PLACEHOLDER_DEF(TEXTURE_TYPE);

// vec2, vec3
PLACEHOLDER_DEF(VECTOR_DIM);

// vec_<f32>, vec_<u32>, vec_<i32>, f32
PLACEHOLDER_DEF(RET_TYPE);

// @builtin(frag_depth), @location(0)
PLACEHOLDER_DEF(RET_ATTRIBUTE);

// <f32>, <u32>, <i32>
PLACEHOLDER_DEF(DST_PRIM_TYPE);
PLACEHOLDER_DEF(SRC_PRIM_TYPE);

#undef PLACEHOLDER_DEF

constexpr std::string_view SHADER_SOURCE_TEMPLATE{ R"(
    struct BlitFragmentShaderArgs {
        depthPlane:        u32,
        scale:             vec2<f32>,
        sourceOffset:      vec2<u32>,
        destinationOffset: vec2<u32>,
    };

    @group(0) @binding(0) var sourceTexture: {{TEXTURE_TYPE}};
    @group(0) @binding(1) var<uniform> fragmentShaderArgs: BlitFragmentShaderArgs;

    fn getUnnormalizedSourceTextureCoordinates(position: vec2<f32>) -> vec2<f32> {
        // These coordinates match the Vulkan vkCmdBlitImage spec:
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBlitImage.html
        let uvOffset: vec2<f32> = position - vec2<f32>(f32(fragmentShaderArgs.destinationOffset.x),f32(fragmentShaderArgs.destinationOffset.y));
        let uvScaled: vec2<f32> = uvOffset * fragmentShaderArgs.scale;
        return uvScaled + vec2<f32>(f32(fragmentShaderArgs.sourceOffset.x), f32(fragmentShaderArgs.sourceOffset.y));
    }

    fn normalize2dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates: vec2<f32>,
            sourceDimensions: vec2<u32>) -> vec2<f32> {
        return unnormalizedSourceTextureCoordinates /
            vec2<f32>(f32(sourceDimensions.x), f32(sourceDimensions.y));
    }

    @vertex
    fn vertexShaderMain(@builtin(vertex_index) vertexIndex: u32) -> @builtin(position) vec4<f32> {
        let fullScreenTriangleVertices = array<vec2<f32>, 3>(
            vec2<f32>(-1.0, -1.0),
            vec2<f32>( 3.0, -1.0),
            vec2<f32>(-1.0,  3.0)
        );
        return vec4<f32>(fullScreenTriangleVertices[vertexIndex].xy, 0.0, 1.0);
    }

    {{FRAGMENT_SHADER_SNIPPET}}
)"
};

constexpr std::string_view FLOAT_TEXTURE_SAMPLE_IMPL_TEMPLATE{ R"(
        return textureSample(sourceTexture, sourceSampler, coordinates);
)" };

constexpr std::string_view INT_TEXTURE_SAMPLE_IMPL_TEMPLATE { R"(
        let texelCoords = {{VECTOR_DIM}}<u32>(coordinates * {{VECTOR_DIM}}<f32>(sourceDimensions));
        return textureLoad(sourceTexture, texelCoords, 0);
)"};

constexpr std::string_view TEXTURE_SAMPLE_IMPL_MAIN_TEMPLATE { R"(
    // Assumes that "@group(0) @binding(2) var sourceSampler: sampler;" has been declared in the main program;
    fn sampleTextureImpl(sourceTexture: {{TEXTURE_TYPE}}, sourceDimensions: {{VECTOR_DIM}}<u32>,
                  coordinates: {{VECTOR_DIM}}<f32>) -> vec4{{DST_PRIM_TYPE}} {
        {{TEXTURE_SAMPLE_IMPL}}
    }
)"};

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_MSAA_INPUT_TEMPLATE{ R"(
    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        var color: vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 0.0);
        let numberOfSamples: u32 = textureNumSamples(sourceTexture);
        let coordinatesF: vec2<f32> = getUnnormalizedSourceTextureCoordinates(position.xy);
        let coordinates: vec2<u32> = vec2<u32>(u32(coordinatesF.x), u32(coordinatesF.y));
        for (var sampleIndex: u32 = 0; sampleIndex < numberOfSamples; sampleIndex++) {
            color += textureLoad(sourceTexture, coordinates, sampleIndex);
        }
        color /= f32(numberOfSamples);
        return color;
    }
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_3D_INPUT_TEMPLATE{ R"(
    @group(0) @binding(2) var sourceSampler: sampler;

    {{TEXTURE_SAMPLE_IMPL_MAIN}}

    fn normalize3dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates: vec2<f32>,
            sourceDimensions: vec3<u32>) -> vec3<f32> {
        let uvNormalized: vec2<f32> = normalize2dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates,
            sourceDimensions.xy
        );
        return vec3<f32>(
            uvNormalized,
            (f32(fragmentShaderArgs.depthPlane) + 0.5) / f32(sourceDimensions.z)
        );
    }

    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        let sourceDimensions: vec3<u32> = textureDimensions(sourceTexture);
        let coordinates: vec3<f32> = normalize3dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position),
            sourceDimensions
        );
        return sampleTextureImpl(sourceTexture, sourceDimensions, coordinates);
    }
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_2D_INPUT_TEMPLATE { R"(
    @group(0) @binding(2) var sourceSampler: sampler;

    {{TEXTURE_SAMPLE_IMPL_MAIN}}

    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        let sourceDimensions: vec2<u32> = textureDimensions(sourceTexture);
        let coordinates: vec2<f32> = normalize2dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position.xy),
            sourceDimensions
        );
        return sampleTextureImpl(sourceTexture, sourceDimensions, coordinates);
    }
)" };

} // namespace

// It can perform a direct memory copy if the formats are compatible and no scaling or format
// conversion is needed. Otherwise, it uses a render pass with a custom shader to perform
// the blit. This allows for scaling, format conversion, and resolving multisampled textures.
// The blitter caches pipelines, layouts, and shaders to avoid recompilation.
WebGPUBlitter::WebGPUBlitter(wgpu::Device const& device)
    : mDevice{ device } {}

// Performs a blit operation from a source to a destination texture.
// This function first checks if a direct texture-to-texture copy can be performed.
// A direct copy is possible if the source and destination textures have the same sample count,
// compatible formats, and the copy extents match. If a direct copy is not possible, this
// function falls back to a render pass-based blit. The render pass uses a shader to read from
// the source texture and write to the destination texture, allowing for format conversion,
// scaling, and MSAA resolves.
void WebGPUBlitter::blit(wgpu::Queue const& queue, wgpu::CommandEncoder const& commandEncoder,
        BlitArgs const& args) {
    // current assumptions/simplifications made in this implementation:
    // 1. We can safely convert to and from f32 floats in the WGSL shader for all input/output
    //    formats (where WebGPU is doing the conversions). Note that the Metal blitter
    //    implementation does essentially the same. This may not be ideal for certain
    //    signed or unsigned integer formats and may represent a possible optimization opportunity
    //    in the future (or we may run into issues/bugs without and be forced to add the complexity
    //    working with different formats as needed).
    //    One idea for accomplishing this would be to define the shader as a template, do search
    //    and replace of placeholders at the right places to get a shader with the desired formats,
    //    and cache compiled shaders, where the key includes input/output formats. But, we would
    //    like to defer that complexity until warranted (which is maybe never).
    // 2. We do not need to support multisampled output textures (only multisampled input,
    //    essentially doing a resolve in those instances). Again,
    //    the Metal blitter implementation does not support this either.
    //    One idea for accomplishing this, if needed (which is maybe never), would be to define
    //    a version of each fragment shader with or without the need to generate multisampled color.
    //    Alternatively, as in the varied input/output format problem (see 1), we could also
    //    implement a shader template with placeholders, etc.
    // 3. We do not need to support multisampled 3D textures. This is a safe assumption, as at this
    //    time no major known graphics API supports this, including WebGPU. Furthermore, the Metal
    //    blitter does not support this either.

    const bool willDirectCopy{ canDoDirectCopy(args.source.texture.GetSampleCount(),
            args.source.texture.GetFormat(), args.source.origin, args.source.extent,
            args.destination.texture.GetSampleCount(), args.destination.texture.GetFormat(),
            args.destination.origin, args.destination.extent) };
#if FWGPU_ENABLED(FWGPU_DEBUG_BLIT)
    FWGPU_LOGD << "ABOUT to blit using " << (willDirectCopy ? "copy-texture-to-texture" : "renderpass");
    FWGPU_LOGD << "  Source:";
    FWGPU_LOGD << "    " << webGPUTextureToString(args.source.texture);
    FWGPU_LOGD << "    aspect: " << webGPUPrintableToString(args.source.aspect);
    FWGPU_LOGD << "    origin: x: " << args.source.origin.x << " y: " << args.source.origin.y;
    FWGPU_LOGD << "    extent: width: " << args.source.extent.width << " height: " << args.source.extent.height;
    FWGPU_LOGD << "    mipLevel: " << args.source.mipLevel;
    FWGPU_LOGD << "    layerOrDepth: " << args.source.layerOrDepth;
    FWGPU_LOGD << "  Destination:";
    FWGPU_LOGD << "    " << webGPUTextureToString(args.destination.texture);
    FWGPU_LOGD << "    aspect: " << webGPUPrintableToString(args.destination.aspect);
    FWGPU_LOGD << "    origin: x: " << args.destination.origin.x << " y: " << args.destination.origin.y;
    FWGPU_LOGD << "    extent: width: " << args.destination.extent.width << " height: " << args.destination.extent.height;
    FWGPU_LOGD << "    mipLevel: " << args.destination.mipLevel;
    FWGPU_LOGD << "    layerOrDepth: " << args.destination.layerOrDepth;
    FWGPU_LOGD << "  filter: " << (args.filter == SamplerMagFilter::NEAREST ? "NEAREST" : "LINEAR");
#endif
    FILAMENT_CHECK_PRECONDITION(args.source.texture.GetDimension() != wgpu::TextureDimension::e3D ||
                                args.source.texture.GetSampleCount() == 1)
            << "Multispampled 3D textures are not supported (the source 3D texture was configured "
               "for multisampling) (this should not be possible?)";
    if (willDirectCopy) {
        FILAMENT_CHECK_PRECONDITION(args.source.texture.GetUsage() & wgpu::TextureUsage::CopySrc)
                << "source texture usage doesn't have wgpu::TextureUsage::CopySrc";

        FILAMENT_CHECK_PRECONDITION(
                args.destination.texture.GetUsage() & wgpu::TextureUsage::CopyDst)
                << "destination texture usage doesn't have wgpu::TextureUsage::CopyDst";

        copyByteByByte(commandEncoder, args);
        return;
    }

    FILAMENT_CHECK_PRECONDITION(args.destination.texture.GetSampleCount() == 1)
            << "Blitting does not currently support writing to multisampled output textures.";

    FILAMENT_CHECK_PRECONDITION(args.source.texture.GetUsage() & wgpu::TextureUsage::TextureBinding)
            << "Source texture usage doesn't have wgpu::TextureUsage::TextureBinding";

    FILAMENT_CHECK_PRECONDITION(
            args.destination.texture.GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "Destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    const wgpu::TextureViewDimension sourceDimension{ args.source.texture.GetDimension() ==
                                                                      wgpu::TextureDimension::e3D
                                                              ? wgpu::TextureViewDimension::e3D
                                                              : wgpu::TextureViewDimension::e2D };
    const wgpu::TextureViewDescriptor sourceViewDescriptor{
        .label = "blit_source",
        .format = args.source.texture.GetFormat(),
        .dimension = sourceDimension,
        .baseMipLevel = args.source.mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = args.source.texture.GetDimension() == wgpu::TextureDimension::e3D
                                  ? 1
                                  : args.source.layerOrDepth,
        .arrayLayerCount = 1,
        .aspect = args.source.aspect,
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    const wgpu::TextureView sourceView{ args.source.texture.CreateView(&sourceViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(sourceView)
            << "Failed to create source texture view for baseArrayLayer "
            << sourceViewDescriptor.baseArrayLayer << " and mip level "
            << sourceViewDescriptor.baseMipLevel << " for render pass blit?";
    const wgpu::TextureViewDescriptor destinationViewDescriptor{
        .label = "blit_destination",
        .format = args.destination.texture.GetFormat(),
        .dimension = args.destination.texture.GetDimension() == wgpu::TextureDimension::e3D
                             ? wgpu::TextureViewDimension::e3D
                             : wgpu::TextureViewDimension::e2D,
        .baseMipLevel = args.destination.mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = args.destination.texture.GetDimension() == wgpu::TextureDimension::e3D
                                  ? 1
                                  : args.destination.layerOrDepth,
        .arrayLayerCount = 1,
        .aspect = args.destination.aspect,
        .usage = wgpu::TextureUsage::RenderAttachment,
    };
    const wgpu::TextureView destinationView{ args.destination.texture.CreateView(
            &destinationViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(destinationView)
            << "Failed to create destination texture view for baseArrayLayer "
            << destinationViewDescriptor.baseArrayLayer << " and mip level "
            << destinationViewDescriptor.baseMipLevel << " for render pass blit?";

    const BlitFragmentShaderArgs blitFragmentShaderArgs{
        .depthPlane = args.source.layerOrDepth,
        .scale = { static_cast<float>(args.source.extent.width) / args.destination.extent.width,
            static_cast<float>(args.source.extent.height) / args.destination.extent.height },
        .sourceOffset = { args.source.origin.x, args.source.origin.y },
        .destinationOffset = { args.destination.origin.x, args.destination.origin.y },
    };
    wgpu::BufferDescriptor uniformBufferDescriptor{
        .label = "blit_args_uniform_buffer",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = sizeof(BlitFragmentShaderArgs),
        // explicitly set for consistency, alt
        .mappedAtCreation = false,
    };
    const wgpu::Buffer uniformBuffer{ mDevice.CreateBuffer(&uniformBufferDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(uniformBuffer)
            << "Failed to create uniform buffer for texture swizzle?";
    queue.WriteBuffer(uniformBuffer, /* buffer offset */ 0, &blitFragmentShaderArgs,
            sizeof(BlitFragmentShaderArgs));
    size_t textureBindGroupEntriesCount{ 2 }; // just the texture and uniform by default
    // get/create/bind sample if needed...
    wgpu::Sampler sampler{ nullptr };
    if (args.source.texture.GetSampleCount() <= 1) {
        textureBindGroupEntriesCount++; // add the sampler to the count
        switch (args.filter) {
            case SamplerMagFilter::NEAREST:
                if (!mNearestSampler) {
                    createSampler(args.filter);
                }
                sampler = mNearestSampler;
                break;
            case SamplerMagFilter::LINEAR:
                if (!mLinearSampler) {
                    createSampler(args.filter);
                }
                sampler = mLinearSampler;
                break;
        }
    }
    // create bind group...
    const wgpu::BindGroupEntry textureBindGroupEntries[MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE]{
        {
            .binding = TEXTURE_BINDING_INDEX,
            .textureView = sourceView,
        },
        {
            .binding = UNIFORM_BINDING_INDEX,
            .buffer = uniformBuffer,
            .offset = 0,              // explicitly defining for consistency
            .size = wgpu::kWholeSize, // explicitly defining for consistency
        },
        {
            .binding = SAMPLER_BINDING_INDEX,
            .sampler = sampler, // note: might be nullptr if we don't need it
        }
    };
    const bool multisampledSource{ args.source.texture.GetSampleCount() > 1 };
    const bool depthSource{ hasDepth(args.source.texture.GetFormat()) };
    const bool depthDestination{ hasDepth(args.destination.texture.GetFormat()) };

    wgpu::TextureSampleType srcSampleType = {};
    if (depthSource) {
        srcSampleType = wgpu::TextureSampleType::Depth;
    } else if (multisampledSource) {
        srcSampleType = wgpu::TextureSampleType::UnfilterableFloat;
    } else if (isIntFormat(args.source.texture.GetFormat())) {
        srcSampleType = wgpu::TextureSampleType::Sint;
    } else if (isUIntFormat(args.source.texture.GetFormat())) {
        srcSampleType = wgpu::TextureSampleType::Uint;
    } else {
        srcSampleType = wgpu::TextureSampleType::Float;
    }

    const PipelineLayoutKey pipelineLayoutKey{
        .sourceDimension = sourceDimension,
        .sourceSampleType = srcSampleType,
        .filterType = args.filter,
        .multisampledSource = multisampledSource,
    };
    const wgpu::BindGroupDescriptor textureBindGroupDescriptor{
        .label = "blit_texture_bind_group",
        .layout = getOrCreateTextureBindGroupLayout(pipelineLayoutKey),
        .entryCount = textureBindGroupEntriesCount,
        .entries = textureBindGroupEntries,
    };
    const wgpu::BindGroup textureBindGroup{ mDevice.CreateBindGroup(&textureBindGroupDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(textureBindGroup)
            << "Failed to create texture bind group for render pass blit?";
    // create color or depth attachment (output)...
    wgpu::RenderPassDescriptor renderPassDescriptor{
        .label = "blit_render_pass",
        .colorAttachmentCount = 0,         // updated if non-depth destination
        .colorAttachments = nullptr,       // updated if non-depth destination
        .depthStencilAttachment = nullptr, // updated if depth destination
        .occlusionQuerySet = nullptr,      // not applicable
        .timestampWrites = nullptr,        // not applicable
    };
    if (depthDestination) {
        const wgpu::RenderPassDepthStencilAttachment depthStencilAttachment{
            .view = destinationView,
            .depthLoadOp = wgpu::LoadOp::Clear,
            .depthStoreOp = wgpu::StoreOp::Store,
            .depthClearValue = 0.0f, // explicitly defining for consistency
            .depthReadOnly = false,
            .stencilLoadOp = wgpu::LoadOp::Undefined,   // stencil not supported
            .stencilStoreOp = wgpu::StoreOp::Undefined, // stencil not supported
        };
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    } else {
        // color destination...
        const wgpu::RenderPassColorAttachment colorAttachment{
            .view = destinationView,
            .depthSlice = args.destination.texture.GetDimension() == wgpu::TextureDimension::e3D
                                  ? args.destination.layerOrDepth
                                  : wgpu::kDepthSliceUndefined,
            // we don't need this resolveTarget because we are doing a resolve in software (the
            // shader), but we probably should (leveraging a msaa sidecar texture) to take advantage
            // of hardware acceleration. Thus, this might be an opportunity for optimization
            .resolveTarget = nullptr,
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
        };
        renderPassDescriptor.colorAttachmentCount = 1;
        renderPassDescriptor.colorAttachments = &colorAttachment;
    }
    const wgpu::RenderPassEncoder renderPassEncoder{ commandEncoder.BeginRenderPass(
            &renderPassDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(renderPassEncoder)
            << "Failed to create render pass encoder for blit.";
    const RenderPipelineKey renderPipelineKey{
        .sourceDimension = sourceDimension,
        .sourceTextureFormat = args.source.texture.GetFormat(),
        .sourceSampleType = srcSampleType,
        .destinationTextureFormat = args.destination.texture.GetFormat(),
        .sourceSampleCount = static_cast<uint8_t>(args.source.texture.GetSampleCount()),
        .filterType = args.filter,
    };
    renderPassEncoder.SetPipeline(getOrCreateRenderPipeline(renderPipelineKey));
    renderPassEncoder.SetBindGroup(TEXTURE_BIND_GROUP_INDEX, textureBindGroup);
    renderPassEncoder.Draw(3); // draw the full-screen triangle
                               // with hard-coded vertices in the shader
    renderPassEncoder.End();
}

void WebGPUBlitter::copyByteByByte(wgpu::CommandEncoder const& commandEncoder, BlitArgs const& args) {
    const wgpu::TexelCopyTextureInfo sourceCopyInfo{
        .texture = args.source.texture,
        .mipLevel = args.source.mipLevel,
        .origin = { args.source.origin.x, args.source.origin.y, args.source.layerOrDepth },
        .aspect = args.source.aspect,
    };
    const wgpu::TexelCopyTextureInfo destinationCopyInfo{
        .texture = args.destination.texture,
        .mipLevel = args.destination.mipLevel,
        .origin = {
            args.destination.origin.x,
            args.destination.origin.y,
            args.destination.layerOrDepth,
        },
        .aspect = args.destination.aspect,
    };
    const wgpu::Extent3D copySize{ args.source.extent.width, args.source.extent.height, 1 };
    commandEncoder.CopyTextureToTexture(&sourceCopyInfo, &destinationCopyInfo, &copySize);
}

void WebGPUBlitter::createSampler(const SamplerMagFilter filter) {
    wgpu::StringView label;
    wgpu::FilterMode magFilter;
    wgpu::FilterMode minFilter;
    wgpu::MipmapFilterMode mipmapFilter;
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            label = "blit_sampler_nearest";
            magFilter = wgpu::FilterMode::Nearest;
            minFilter = wgpu::FilterMode::Nearest;
            mipmapFilter = wgpu::MipmapFilterMode::Nearest;
            break;
        case SamplerMagFilter::LINEAR:
            label = "blit_sampler_linear";
            magFilter = wgpu::FilterMode::Linear;
            minFilter = wgpu::FilterMode::Linear;
            mipmapFilter = wgpu::MipmapFilterMode::Linear;
            break;
    }
    const wgpu::SamplerDescriptor descriptor{
        .label = label,
        .addressModeU = wgpu::AddressMode::ClampToEdge,
        .addressModeV = wgpu::AddressMode::ClampToEdge,
        .addressModeW = wgpu::AddressMode::ClampToEdge,
        .magFilter = magFilter,
        // minFilter and mipmapFilter shouldn't really matter, as the source texture should
        // have 1 mip level, but for consistency (also, Metal blitter implementation does this)...
        .minFilter = minFilter,
        .mipmapFilter = mipmapFilter,
        .lodMinClamp = 0.0f,  // should not matter, just being consistently defined
        .lodMaxClamp = 32.0f, // should not matter, just being consistently defined
        .compare = wgpu::CompareFunction::Undefined, // should not matter, just being consistently
                                                     // defined
        .maxAnisotropy = 1, // should not matter, just being consistently defined
    };
    const wgpu::Sampler sampler{ mDevice.CreateSampler(&descriptor) };
    FILAMENT_CHECK_POSTCONDITION(sampler) << "Failed to create blit sampler.";
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            mNearestSampler = sampler;
            break;
        case SamplerMagFilter::LINEAR:
            mLinearSampler = sampler;
            break;
    }
}

// Caches and returns a render pipeline for a given blit configuration.
// If a pipeline for the given configuration does not exist, it creates and caches one.
wgpu::RenderPipeline const& WebGPUBlitter::getOrCreateRenderPipeline(
        RenderPipelineKey const& key) {
    if (mRenderPipelines.find(key) == mRenderPipelines.end()) {
        mRenderPipelines[key] = createRenderPipeline(key);
    }
    return mRenderPipelines[key];
}

// Creates a render pipeline for a blit operation.
// The pipeline is configured with the appropriate vertex and fragment shaders,
// pipeline layout, and render target state based on the blit parameters.
wgpu::RenderPipeline WebGPUBlitter::createRenderPipeline(RenderPipelineKey const& key) {
    const bool depthDestination{ hasDepth(key.destinationTextureFormat) };
    const wgpu::ColorTargetState colorTargetState{ .format = key.destinationTextureFormat };
    const wgpu::DepthStencilState depthStencilState{
        .format = key.destinationTextureFormat,
        .depthWriteEnabled = true,
        .depthCompare = wgpu::CompareFunction::Always,
    };
    const ShaderModuleKey shaderModuleKey{
        .sourceDimension = key.sourceDimension,
        .sourceTextureFormat = key.sourceTextureFormat,
        .destinationTextureFormat = key.destinationTextureFormat,
        .multisampledSource = key.sourceSampleCount > 1,
    };
    wgpu::ShaderModule const& shaderModule{ getOrCreateShaderModule(shaderModuleKey) };
    const wgpu::FragmentState fragmentState{
        .module = shaderModule,
        .entryPoint = FRAGMENT_SHADER_ENTRY_POINT,
        .constantCount = 0,   // should not matter, just being consistently defined
        .constants = nullptr, // should not matter, just being consistently defined
        .targetCount = depthDestination ? 0u : 1u,
        .targets = depthDestination ? nullptr : &colorTargetState,
    };
    const PipelineLayoutKey pipelineLayoutKey{
        .sourceDimension = key.sourceDimension,
        .sourceSampleType = key.sourceSampleType,
        .filterType = key.filterType,
        .multisampledSource = key.sourceSampleCount > 1,
    };
    const wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = "render_pass_blit_pipeline",
        .layout = getOrCreatePipelineLayout(pipelineLayoutKey),
        .vertex = {
            .module = shaderModule,
            .entryPoint = VERTEX_SHADER_ENTRY_POINT,
            .constantCount = 0, // should not matter, just being consistently defined
            .constants = nullptr, // should not matter, just being consistently defined
            .bufferCount = 0, // using hardcoded vertices in the shader for
                              // performance and simplicity
            .buffers = nullptr,
        },
        .primitive = {
            .topology = wgpu::PrimitiveTopology::TriangleList,
            .stripIndexFormat = wgpu::IndexFormat::Undefined, // should not matter
                                                              // (not using a strip topology),
                                                              // just being consistently defined
            .frontFace = wgpu::FrontFace::Undefined, // should not matter (no cull mode),
                                                     // just being consistently defined
            .cullMode = wgpu::CullMode::None,
            .unclippedDepth = false, // should not matter, just being consistently defined
        },
        .depthStencil = depthDestination ? &depthStencilState : nullptr,
        .multisample = {
            .count = key.sourceSampleCount,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };
    const wgpu::RenderPipeline pipeline{ mDevice.CreateRenderPipeline(&pipelineDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline) << "Failed to create pipeline for render pass blit.";
    return pipeline;
}

bool WebGPUBlitter::RenderPipelineKey::operator==(RenderPipelineKey const& other) const {
    return memcmp(this, &other, sizeof(RenderPipelineKey)) == 0;
}

// Caches and returns a pipeline layout for a given blit configuration.
// If a layout for the given configuration does not exist, it creates and caches one.
wgpu::PipelineLayout const& WebGPUBlitter::getOrCreatePipelineLayout(
        PipelineLayoutKey const& key) {
    if (mPipelineLayouts.find(key) == mPipelineLayouts.end()) {
        mPipelineLayouts[key] = createPipelineLayout(key);
    }
    return mPipelineLayouts[key];
}

// Creates a pipeline layout for a blit operation.
// The layout defines the bind group layouts used by the blit pipeline.
wgpu::PipelineLayout WebGPUBlitter::createPipelineLayout(PipelineLayoutKey const& key) {
    const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{
        .label = "render_pass_blit_pipeline_layout",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &getOrCreateTextureBindGroupLayout(key),
    };
    const wgpu::PipelineLayout pipelineLayout{ mDevice.CreatePipelineLayout(
            &pipelineLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipelineLayout)
            << "Failed to create pipeline layout for render pass blit.";
    return pipelineLayout;
}

bool WebGPUBlitter::PipelineLayoutKey::operator==(const PipelineLayoutKey& other) const {
    return memcmp(this, &other, sizeof(PipelineLayoutKey)) == 0;
}

// Caches and returns a bind group layout for a given blit configuration.
// If a layout for the given configuration does not exist, it creates and caches one.
wgpu::BindGroupLayout const& WebGPUBlitter::getOrCreateTextureBindGroupLayout(
        PipelineLayoutKey const& key) {
    if (mTextureBindGroupLayouts.find(key) == mTextureBindGroupLayouts.end()) {
        mTextureBindGroupLayouts[key] = createTextureBindGroupLayout(key);
    }
    return mTextureBindGroupLayouts[key];
}

// Creates a bind group layout for the blit operation's texture resources.
// This layout specifies the bindings for the source texture, a uniform buffer with blit parameters,
// and optionally a sampler.
wgpu::BindGroupLayout WebGPUBlitter::createTextureBindGroupLayout(PipelineLayoutKey const& key) {
    const wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE]{
        {
            .binding = TEXTURE_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = {
                .sampleType = key.sourceSampleType,
                .viewDimension = key.sourceDimension,
                .multisampled = key.multisampledSource,
            },
        },
        {
            .binding = UNIFORM_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .buffer = {
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = false, // set explicitly for consistency
                .minBindingSize = sizeof(BlitFragmentShaderArgs),
            },
        },
        {
            .binding = SAMPLER_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .sampler = {
                .type = key.filterType == SamplerMagFilter::LINEAR
                                ? wgpu::SamplerBindingType::Filtering
                                : wgpu::SamplerBindingType::NonFiltering,
            },
        },
    };
    const wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDescriptor{
        .label = "render_pass_blit_texture_bind_group_layout",
        // No need for the last entry (sampler) if source is multisampled.
        .entryCount = key.multisampledSource ? (MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE - 1)
                                             : (MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE),
        .entries = bindGroupLayoutEntries,
    };
    const wgpu::BindGroupLayout textureBindGroupLayout{ mDevice.CreateBindGroupLayout(
            &textureBindGroupLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(textureBindGroupLayout)
            << "Failed to create texture bind group layout for render pass blit.";
    return textureBindGroupLayout;
}

// Caches and returns a shader module for a given blit configuration.
// If a shader module for the given configuration does not exist, it creates and caches one.
wgpu::ShaderModule const& WebGPUBlitter::getOrCreateShaderModule(ShaderModuleKey const& key) {
    if (mShaderModules.find(key) == mShaderModules.end()) {
        mShaderModules[key] = createShaderModule(key);
    }
    return mShaderModules[key];
}

// Creates a shader module containing the vertex and fragment shaders for the blit operation.
// The shader source is generated from a template, with placeholders filled in based on the
// blit configuration (e.g., texture type, sample count).
wgpu::ShaderModule WebGPUBlitter::createShaderModule(ShaderModuleKey const& key) {
    bool const isDepthSrc = hasDepth(key.sourceTextureFormat);
    bool const isDepthDst = hasDepth(key.destinationTextureFormat);

    std::string_view vecDim;
    if (key.sourceDimension == wgpu::TextureViewDimension::e3D) {
        vecDim = "vec3";
    } else {
        vecDim = "vec2";
    }

    std::string_view srcPrimType;
    std::string_view sampleImpl;
    if (isIntFormat(key.sourceTextureFormat)) {
        srcPrimType = "<i32>";
        sampleImpl = INT_TEXTURE_SAMPLE_IMPL_TEMPLATE;
    } else if (isUIntFormat(key.sourceTextureFormat)) {
        srcPrimType = "<u32>";
        sampleImpl = INT_TEXTURE_SAMPLE_IMPL_TEMPLATE;
    } else {
        srcPrimType = "<f32>";
        sampleImpl = FLOAT_TEXTURE_SAMPLE_IMPL_TEMPLATE;
    }

    std::string_view fragmentTemplate;
    std::string_view srcTextureType;
    if (isDepthSrc && key.multisampledSource) {
        srcTextureType = "texture_depth_multisampled_2d";
        fragmentTemplate = FRAGMENT_SHADER_SNIPPET_2D_INPUT_TEMPLATE;
    } else if (isDepthSrc && !key.multisampledSource) {
        srcTextureType = "texture_depth_2d";
        fragmentTemplate = FRAGMENT_SHADER_SNIPPET_2D_INPUT_TEMPLATE;
    } else if (key.multisampledSource) {
        srcTextureType = "texture_multisampled_2d{{SRC_PRIM_TYPE}}";
        fragmentTemplate = FRAGMENT_SHADER_SNIPPET_MSAA_INPUT_TEMPLATE;
    } else if (key.sourceDimension == wgpu::TextureViewDimension::e3D) {
        srcTextureType = "texture_3d{{SRC_PRIM_TYPE}}";
        fragmentTemplate = FRAGMENT_SHADER_SNIPPET_3D_INPUT_TEMPLATE;
    } else {
        srcTextureType = "texture_2d{{SRC_PRIM_TYPE}}";
        fragmentTemplate = FRAGMENT_SHADER_SNIPPET_2D_INPUT_TEMPLATE;
    }

    std::string_view dstPrimType;
    if (isIntFormat(key.destinationTextureFormat)) {
        dstPrimType = "<i32>";
    } else if (isUIntFormat(key.destinationTextureFormat)) {
        dstPrimType = "<u32>";
    } else {
        dstPrimType = "<f32>";
    }

    std::string_view retAttribute;
    std::string_view retType;
    if (isDepthDst) {
        retAttribute = "@builtin(frag_depth)";
        retType = "f32";
    } else {
        retAttribute = "@location(0)";
        retType = "vec4{{DST_PRIM_TYPE}}";
    }

    using ReplacementMap = std::unordered_map<std::string_view, std::string_view>;
    auto const replace = [&](std::string_view src, ReplacementMap const& replacements) {
        return webgpuutils::processPlaceholderTemplate(
                src, PLACEHOLDER_PREFIX, PLACEHOLDER_SUFFIX, replacements);
    };

    // The dependency chain is shader module -> fragment source -> sampling function
    // Hence we need to fill in the templates in reverse order.

    // Fill in sampling implementation in the sampling func
    std::string source = replace(TEXTURE_SAMPLE_IMPL_MAIN_TEMPLATE, {
        { TEXTURE_SAMPLE_IMPL, sampleImpl },
    });

    // Fill in sampling func in the fragment
    source = replace(fragmentTemplate, {
        { TEXTURE_SAMPLE_IMPL_MAIN, source },
    });

    // Fill in fragment in the shader module, first pass
    source = replace(SHADER_SOURCE_TEMPLATE, {
        { FRAGMENT_SHADER_SNIPPET, source },
    });

    // Fill in fragment in the shader module, second pass
    source = replace(source, {
        { TEXTURE_TYPE, srcTextureType },
        { RET_ATTRIBUTE, retAttribute },
        { RET_TYPE, retType },
    });

    // Fill in fragment in the shader module, third dpass
    // TEXTURE_TYPE, RET_ATTRIBUTE, RET_TYPE have a depedency on SRC_PRIM_TYPE, DST_PRIM_TYPE,
    // VECTOR_DIM so we need another pass to fill them in.
    source = replace(source, {
        { VECTOR_DIM, vecDim },
        { SRC_PRIM_TYPE, srcPrimType },
        { DST_PRIM_TYPE, dstPrimType },
    });

    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = source.data();
    const wgpu::ShaderModuleDescriptor shaderModuleDescriptor{
        .nextInChain = &wgslDescriptor,
        .label = "render_pass_blit_shaders",
    };
    const wgpu::ShaderModule shaderModule{ mDevice.CreateShaderModule(&shaderModuleDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(shaderModule)
            << "Failed to create shader module for render pass blit.";
    return shaderModule;
}

bool WebGPUBlitter::ShaderModuleKey::operator==(const ShaderModuleKey& other) const {
    return memcmp(this, &other, sizeof(ShaderModuleKey)) == 0;
}

} // namespace filament::backend
