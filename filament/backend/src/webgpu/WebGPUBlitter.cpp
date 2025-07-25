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
#include "WebGPUStrings.h"
#include "webgpu/utils/StringPlaceholderTemplateProcessor.h"

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <math/vec2.h>
#include <utils/Hash.h>
#include <utils/Panic.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace filament::backend {

namespace {

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
// texture_2d<f32> or texture_multisampled_2d<f32> or texture_3d<f32>
constexpr std::string_view TEXTURE_TYPE_PLACEHOLDER{ "TEXTURE_TYPE" };
// "" for no sampler, otherwise "@group(0) @binding(2) var sourceSampler: sampler;"
constexpr std::string_view SAMPLER_DECLARATION_PLACEHOLDER{ "SAMPLER_DECLARATION" };
constexpr std::string_view FRAGMENT_SHADER_SNIPPET_PLACEHOLDER{ "FRAGMENT_SHADER_SNIPPET" };
constexpr std::string_view SHADER_SOURCE_TEMPLATE{ R"(
    struct BlitFragmentShaderArgs {
        depthPlane:        u32,
        scale:             vec2<f32>,
        sourceOffset:      vec2<u32>,
        destinationOffset: vec2<u32>,
    };

    @group(0) @binding(0) var sourceTexture: {{TEXTURE_TYPE}};
    @group(0) @binding(1) var<uniform> fragmentShaderArgs: BlitFragmentShaderArgs;
    {{SAMPLER_DECLARATION}}

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

    fn getNormalized2dSourceTextureCoordinates(
            position: vec2<f32>,
            sourceTexture: texture_2d<f32>) -> vec2<f32> {
        let sourceDimensions: vec2<u32> = textureDimensions(sourceTexture);
        return normalize2dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position),
            sourceDimensions
        );
    }

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

    fn getNormalized3dSourceTextureCoordinates(
            position: vec2<f32>,
            sourceTexture: texture_3d<f32>) -> vec3<f32> {
        let sourceDimensions: vec3<u32> = textureDimensions(sourceTexture);
        return normalize3dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position),
            sourceDimensions
        );
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
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_MSAA_INPUT{ R"(
    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> @location(0) vec4<f32> {
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

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_3D_INPUT{ R"(
    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> @location(0) vec4<f32> {
        let coordinates: vec3<f32> = getNormalized3dSourceTextureCoordinates(position.xy, sourceTexture);
        return textureSample(sourceTexture, sourceSampler, coordinates);
    }
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_2D_INPUT{ R"(
    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> @location(0) vec4<f32> {
        let coordinates: vec2<f32> = getNormalized2dSourceTextureCoordinates(position.xy, sourceTexture);
        return textureSample(sourceTexture, sourceSampler, coordinates);
    }
)" };

} // namespace

WebGPUBlitter::WebGPUBlitter(wgpu::Device const& device)
    : mDevice{ device } {}

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

    // TODO REMOVE (keeping for troubleshooting/debug purposes for now)
#if FWGPU_ENABLED(FWGPU_DEBUG_VALIDATION)
    FWGPU_LOGD << "ABOUT to blit:";
    FWGPU_LOGD << "  Source:";
    FWGPU_LOGD << "    texture:" << webGPUPrintableToString(args.source.texture.GetFormat());
    FWGPU_LOGD << "      depthOrArrayLayers: " << args.source.texture.GetDepthOrArrayLayers();
    FWGPU_LOGD << "      " << webGPUPrintableToString(args.source.texture.GetDimension());
    FWGPU_LOGD << "      format: " << webGPUTextureFormatToString(args.source.texture.GetFormat());
    FWGPU_LOGD << "      height: " << args.source.texture.GetHeight();
    FWGPU_LOGD << "      mipLevelCount: " << args.source.texture.GetMipLevelCount();
    FWGPU_LOGD << "      sampleCount: " << args.source.texture.GetSampleCount();
    FWGPU_LOGD << "      " << webGPUPrintableToString(args.source.texture.GetUsage());
    FWGPU_LOGD << "      width: " << args.source.texture.GetWidth();
    FWGPU_LOGD << "    origin: x: " << args.source.origin.x << " y: " << args.source.origin.y;
    FWGPU_LOGD << "    extent: width: " << args.source.extent.width
               << " height: " << args.source.extent.height;
    FWGPU_LOGD << "    mipLevel: " << args.source.mipLevel;
    FWGPU_LOGD << "    layerOrDepth: " << args.source.layerOrDepth;
    FWGPU_LOGD << "  Destination:";
    FWGPU_LOGD << "    texture:" << webGPUPrintableToString(args.destination.texture.GetFormat());
    FWGPU_LOGD << "      depthOrArrayLayers: " << args.destination.texture.GetDepthOrArrayLayers();
    FWGPU_LOGD << "      " << webGPUPrintableToString(args.destination.texture.GetDimension());
    FWGPU_LOGD << "      format: "
               << webGPUTextureFormatToString(args.destination.texture.GetFormat());
    FWGPU_LOGD << "      height: " << args.destination.texture.GetHeight();
    FWGPU_LOGD << "      mipLevelCount: " << args.destination.texture.GetMipLevelCount();
    FWGPU_LOGD << "      sampleCount: " << args.destination.texture.GetSampleCount();
    FWGPU_LOGD << "      " << webGPUPrintableToString(args.destination.texture.GetUsage());
    FWGPU_LOGD << "      width: " << args.destination.texture.GetWidth();
    FWGPU_LOGD << "    origin: x: " << args.destination.origin.x
               << " y: " << args.destination.origin.y;
    FWGPU_LOGD << "    extent: width: " << args.destination.extent.width
               << " height: " << args.destination.extent.height;
    FWGPU_LOGD << "    mipLevel: " << args.destination.mipLevel;
    FWGPU_LOGD << "    layerOrDepth: " << args.destination.layerOrDepth;
    FWGPU_LOGD << "  filter: " << (args.filter == SamplerMagFilter::NEAREST ? "NEAREST" : "LINEAR");
#endif
    FILAMENT_CHECK_PRECONDITION(args.source.texture.GetDimension() != wgpu::TextureDimension::e3D ||
                                args.source.texture.GetSampleCount() == 1)
            << "Multispampled 3D textures are not supported (the source 3D texture was configured "
               "for multisampling) (this should not be possible?)";

    FILAMENT_CHECK_PRECONDITION(args.destination.texture.GetSampleCount() == 1)
            << "Blitting does not currently support writing to multisampled output textures.";

    FILAMENT_CHECK_PRECONDITION(args.source.texture.GetUsage() & wgpu::TextureUsage::TextureBinding)
            << "source texture usage doesn't have wgpu::TextureUsage::TextureBinding";

    FILAMENT_CHECK_PRECONDITION(
            args.destination.texture.GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    // create input/output texture views...
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
        .aspect = wgpu::TextureAspect::All,
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
        .aspect = wgpu::TextureAspect::All,
        .usage = wgpu::TextureUsage::RenderAttachment,
    };
    const wgpu::TextureView destinationView{ args.destination.texture.CreateView(
            &destinationViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(destinationView)
            << "Failed to create destination texture view for baseArrayLayer "
            << destinationViewDescriptor.baseArrayLayer << " and mip level "
            << destinationViewDescriptor.baseMipLevel << " for render pass blit?";

    // create uniform buffer (for shader args)...
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
    const wgpu::BindGroupDescriptor textureBindGroupDescriptor{
        .label = "blit_texture_bind_group",
        .layout = getOrCreateTextureBindGroupLayout(args.filter, sourceDimension,
                args.source.texture.GetSampleCount() > 1),
        .entryCount = textureBindGroupEntriesCount,
        .entries = textureBindGroupEntries,
    };
    const wgpu::BindGroup textureBindGroup{ mDevice.CreateBindGroup(&textureBindGroupDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(textureBindGroup)
            << "Failed to create texture bind group for render pass blit?";
    // create color attachment (output)...
    const wgpu::RenderPassColorAttachment colorAttachment{
        .view = destinationView,
        .depthSlice = args.destination.texture.GetDimension() == wgpu::TextureDimension::e3D
                              ? args.destination.layerOrDepth
                              : wgpu::kDepthSliceUndefined,
        // we don't need this resolveTarget because we are doing a resolve in software (the shader),
        // but we probably should (leveraging a msaa sidecar texture) to take advantage of
        // hardware acceleration. Thus, this might be an opportunity for optimization
        .resolveTarget = nullptr,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
    };
    const wgpu::RenderPassDescriptor renderPassDescriptor{
        .label = "blit_render_pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment,
        .depthStencilAttachment = nullptr, // not applicable
        .occlusionQuerySet = nullptr,      // not applicable
        .timestampWrites = nullptr,        // not applicable
    };
    const wgpu::RenderPassEncoder renderPassEncoder{ commandEncoder.BeginRenderPass(
            &renderPassDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(renderPassEncoder)
            << "Failed to create render pass encoder for blit?";
    renderPassEncoder.SetPipeline(getOrCreateRenderPipeline(args.filter, sourceDimension,
            args.source.texture.GetSampleCount(), args.destination.texture.GetFormat()));
    renderPassEncoder.SetBindGroup(TEXTURE_BIND_GROUP_INDEX, textureBindGroup);
    renderPassEncoder.Draw(3); // draw the full-screen triangle
                               // with hard-coded vertices in the shader
    renderPassEncoder.End();
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
    FILAMENT_CHECK_POSTCONDITION(sampler) << "Failed to create blit sampler?";
    switch (filter) {
        case SamplerMagFilter::NEAREST:
            mNearestSampler = sampler;
            break;
        case SamplerMagFilter::LINEAR:
            mLinearSampler = sampler;
            break;
    }
}

wgpu::RenderPipeline const& WebGPUBlitter::getOrCreateRenderPipeline(
        const SamplerMagFilter filterType, const wgpu::TextureViewDimension sourceDimension,
        const uint32_t sourceSampleCount, const wgpu::TextureFormat destinationTextureFormat) {
    const size_t key{ hashRenderPipelineKey(filterType, sourceDimension, sourceSampleCount) };
    if (mRenderPipelines.find(key) == mRenderPipelines.end()) {
        mRenderPipelines[key] = createRenderPipeline(filterType, sourceDimension, sourceSampleCount,
                destinationTextureFormat);
    }
    return mRenderPipelines[key];
}

wgpu::RenderPipeline WebGPUBlitter::createRenderPipeline(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const uint32_t sourceSampleCount,
        const wgpu::TextureFormat destinationTextureFormat) {
    const wgpu::ColorTargetState colorTargetState{ .format = destinationTextureFormat };
    wgpu::ShaderModule const& shaderModule{ getOrCreateShaderModule(sourceDimension,
            sourceSampleCount > 1) };
    const wgpu::FragmentState fragmentState{
        .module = shaderModule,
        .entryPoint = FRAGMENT_SHADER_ENTRY_POINT,
        .constantCount = 0,   // should not matter, just being consistently defined
        .constants = nullptr, // should not matter, just being consistently defined
        .targetCount = 1,
        .targets = &colorTargetState,
    };
    const wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = "render_pass_blit_pipeline",
        .layout = getOrCreatePipelineLayout(filterType, sourceDimension, sourceSampleCount > 1),
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
        .depthStencil = nullptr, // not applicable, but explicitly set here for
                                 // consistent definition
        .multisample = {
            .count = sourceSampleCount,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };
    const wgpu::RenderPipeline pipeline{ mDevice.CreateRenderPipeline(&pipelineDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline) << "Failed to create pipeline for render pass blit?";
    return pipeline;
}

size_t WebGPUBlitter::hashRenderPipelineKey(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const uint32_t sourceSampleCount) {
    size_t seed{ std::hash<uint8_t>{}(static_cast<uint8_t>(filterType)) };
    utils::hash::combine(seed, static_cast<uint32_t>(sourceDimension));
    utils::hash::combine(seed, sourceSampleCount);
    return seed;
}

wgpu::PipelineLayout const& WebGPUBlitter::getOrCreatePipelineLayout(
        const SamplerMagFilter filterType, const wgpu::TextureViewDimension sourceDimension,
        const bool multisampledSource) {
    const size_t key{ hashPipelineLayoutKey(filterType, sourceDimension, multisampledSource) };
    if (mPipelineLayouts.find(key) == mPipelineLayouts.end()) {
        mPipelineLayouts[key] =
                createPipelineLayout(filterType, sourceDimension, multisampledSource);
    }
    return mPipelineLayouts[key];
}

wgpu::PipelineLayout WebGPUBlitter::createPipelineLayout(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {
    const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{
        .label = "render_pass_blit_pipeline_layout",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts =
                &getOrCreateTextureBindGroupLayout(filterType, sourceDimension, multisampledSource),
    };
    const wgpu::PipelineLayout pipelineLayout{ mDevice.CreatePipelineLayout(
            &pipelineLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipelineLayout)
            << "Failed to create pipeline layout for render pass blit?";
    return pipelineLayout;
}

size_t WebGPUBlitter::hashPipelineLayoutKey(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {
    size_t seed{ std::hash<uint8_t>{}(static_cast<uint8_t>(filterType)) };
    utils::hash::combine(seed, static_cast<uint32_t>(sourceDimension));
    utils::hash::combine(seed, multisampledSource);
    return seed;
}

wgpu::BindGroupLayout const& WebGPUBlitter::getOrCreateTextureBindGroupLayout(
        const SamplerMagFilter filterType, const wgpu::TextureViewDimension sourceDimension,
        const bool multisampledSource) {
    const size_t key{ hashTextureBindGroupLayoutKey(filterType, sourceDimension,
            multisampledSource) };
    if (mTextureBindGroupLayouts.find(key) == mTextureBindGroupLayouts.end()) {
        mTextureBindGroupLayouts[key] =
                createTextureBindGroupLayout(filterType, sourceDimension, multisampledSource);
    }
    return mTextureBindGroupLayouts[key];
}

wgpu::BindGroupLayout WebGPUBlitter::createTextureBindGroupLayout(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {
    const wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE] {
        {
            .binding = TEXTURE_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = {
                .sampleType = wgpu::TextureSampleType::Float, // only F32 scalar sample
                                                              // type supported for now
                                                              // see assumptions listed
                                                              // in blit function
                .viewDimension = sourceDimension,
                .multisampled = multisampledSource,
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
                .type =filterType == SamplerMagFilter::LINEAR
                                ? wgpu::SamplerBindingType::Filtering
                                : wgpu::SamplerBindingType::NonFiltering,
            },
        },
    };
    const wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDescriptor{
        .label = "render_pass_blit_texture_bind_group_layout",
        // TODO, doesnt make any sense but gets rid of the error. Are the entries 0 based?
        .entryCount = multisampledSource ? (MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE - 1)
                                         : (MAX_TEXTURE_BIND_GROUP_ENTRY_SIZE),
        .entries = bindGroupLayoutEntries,
    };
    const wgpu::BindGroupLayout textureBindGroupLayout{ mDevice.CreateBindGroupLayout(
            &textureBindGroupLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(textureBindGroupLayout)
            << "Failed to create texture bind group layout for render pass blit?";
    return textureBindGroupLayout;
}

size_t WebGPUBlitter::hashTextureBindGroupLayoutKey(const SamplerMagFilter filterType,
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {

    size_t seed{ std::hash<uint8_t>{}(static_cast<uint8_t>(filterType)) };
    utils::hash::combine(seed, static_cast<uint32_t>(sourceDimension));
    utils::hash::combine(seed, multisampledSource);
    return seed;
}

wgpu::ShaderModule const& WebGPUBlitter::getOrCreateShaderModule(
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {
    const size_t key{ hashShaderModuleKey(sourceDimension, multisampledSource) };
    if (mShaderModules.find(key) == mShaderModules.end()) {
        mShaderModules[key] = createShaderModule(sourceDimension, multisampledSource);
    }
    return mShaderModules[key];
}

wgpu::ShaderModule WebGPUBlitter::createShaderModule(
        const wgpu::TextureViewDimension sourceDimension, const bool multisampledSource) {
    const std::string_view textureType{
        multisampledSource
                ? "texture_multisampled_2d<f32>"
                : (sourceDimension == wgpu::TextureViewDimension::e3D ? "texture_3d<f32>"
                                                                      : "texture_2d<f32>")
    };
    const std::string_view samplerDeclaration{
        multisampledSource ? "" : "@group(0) @binding(2) var sourceSampler: sampler;"
    };
    const std::string_view fragmentShaderSnippet{
        multisampledSource ? FRAGMENT_SHADER_SNIPPET_MSAA_INPUT
                           : (sourceDimension == wgpu::TextureViewDimension::e3D
                                             ? FRAGMENT_SHADER_SNIPPET_3D_INPUT
                                             : FRAGMENT_SHADER_SNIPPET_2D_INPUT)
    };
    std::unordered_map<std::string_view, std::string_view> valueByPlaceholderName{
        { TEXTURE_TYPE_PLACEHOLDER, textureType },
        { SAMPLER_DECLARATION_PLACEHOLDER, samplerDeclaration },
        { FRAGMENT_SHADER_SNIPPET_PLACEHOLDER, fragmentShaderSnippet }
    };
    const std::string shaderSource{ webgpuutils::processPlaceholderTemplate(SHADER_SOURCE_TEMPLATE,
            PLACEHOLDER_PREFIX, PLACEHOLDER_SUFFIX, valueByPlaceholderName) };
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = shaderSource.data();
    const wgpu::ShaderModuleDescriptor shaderModuleDescriptor{
        .nextInChain = &wgslDescriptor,
        .label = "render_pass_blit_shaders",
    };
    const wgpu::ShaderModule shaderModule{ mDevice.CreateShaderModule(&shaderModuleDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(shaderModule)
            << "Failed to create shader module for render pass blit?";
    return shaderModule;
}

size_t WebGPUBlitter::hashShaderModuleKey(const wgpu::TextureViewDimension sourceDimension,
        const bool multisampledSource) {
    size_t seed{ std::hash<uint32_t>{}(static_cast<uint32_t>(sourceDimension)) };
    utils::hash::combine(seed, multisampledSource);
    return seed;
}

} // namespace filament::backend
