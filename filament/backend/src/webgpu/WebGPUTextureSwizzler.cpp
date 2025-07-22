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

#include "WebGPUTextureSwizzler.h"

#include "WebGPUTextureHelpers.h"

#include <backend/DriverEnums.h>

#include <utils/Hash.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <sstream> // for shader template search and replace
#include <string>  // for shader template search and replace
#include <string_view>

namespace filament::backend {

namespace {

// Note: would be nice to generate the constexpr string for the template from the constexpr
// variables below, e.g. TEXTURE_BINDING_INDEX, to avoid them getting out of sync while still having
// the compile/build time optimization
constexpr uint32_t BIND_GROUP_INDEX{ 0 };
constexpr size_t BIND_GROUP_ENTRY_SIZE{ 2 }; // texture and uniform
constexpr uint32_t TEXTURE_BINDING_INDEX{ 0 };
constexpr uint32_t UNIFORM_BINDING_INDEX{ 1 };
constexpr std::string_view VERTEX_SHADER_ENTRY_POINT{ "vertexShaderMain" };
constexpr std::string_view FRAGMENT_SHADER_ENTRY_POINT{ "fragmentShaderMain" };
// note that the placeholders below must start and end with this prefix and suffix:
constexpr std::string_view PLACEHOLDER_PREFIX{ "{{" };
constexpr std::string_view PLACEHOLDER_SUFFIX{ "}}" };
// texture_2d<...> or texture_multisampled_2d<...>
constexpr std::string_view TEXTURE_TYPE_PLACEHOLDER{ "TEXTURE_TYPE" };
// f32, i32, u32
constexpr std::string_view SCALAR_TYPE_PLACEHOLDER{ "SCALAR_TYPE" };
// empty for single-sampled, ", @builtin(sample_index) sampleIndex" for multi-sampled
constexpr std::string_view SAMPLE_INDEX_PARAM_PLACEHOLDER{ "SAMPLE_INDEX_PARAM" };
// "sampleIndex" for multi-sampled, "0" for single-sampled
constexpr std::string_view SAMPLE_INDEX_OR_LEVEL_PLACEHOLDER{ "SAMPLE_INDEX_OR_LEVEL" };
constexpr std::string_view SHADER_SOURCE_TEMPLATE{ R"(
    struct TextureSwizzle {
        redSource:   u32,
        greenSource: u32,
        blueSource:  u32,
        alphaSource: u32,
    };

    @group(0) @binding(0) var sourceTexture: {{TEXTURE_TYPE}};
    @group(0) @binding(1) var<uniform> swizzle: TextureSwizzle;

    fn getComponent(texel: vec4<{{SCALAR_TYPE}}>, index: u32) -> {{SCALAR_TYPE}} {
        switch index {
            case 0u: { return 0; }
            case 1u: { return 1; }
            case 2u: { return texel.r; }
            case 3u: { return texel.g; }
            case 4u: { return texel.b; }
            case 5u: { return texel.a; }
            default: { return 0; }
        }
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

    @fragment
    fn fragmentShaderMain(@builtin(position) vec4<f32> textureCoordinate{{SAMPLE_INDEX_PARAM}}) -> @location(0) vec4<{{SCALAR_TYPE}}> {
        let x = u32(textureCoordinate.x);
        let y = u32(textureCoordinate.y);
        let originalColor = textureLoad(sourceTexture, vec2<u32>(x, y), {{SAMPLE_INDEX_OR_LEVEL}});
        return vec4<{{SCALAR_TYPE}}>(
            getComponent(originalColor, swizzle.redSource),
            getComponent(originalColor, swizzle.greenSource),
            getComponent(originalColor, swizzle.blueSource),
            getComponent(originalColor, swizzle.alphaSource));
    }
)" };

[[nodiscard]] constexpr uint32_t toSwizzleIndex(const TextureSwizzle swizzle) {
    switch (swizzle) {
        case TextureSwizzle::SUBSTITUTE_ZERO: return 0;
        case TextureSwizzle::SUBSTITUTE_ONE:  return 1;
        case TextureSwizzle::CHANNEL_0:       return 2;
        case TextureSwizzle::CHANNEL_1:       return 3;
        case TextureSwizzle::CHANNEL_2:       return 4;
        case TextureSwizzle::CHANNEL_3:       return 5;
    }
}

} // namespace

WebGPUTextureSwizzler::WebGPUTextureSwizzler(wgpu::Device const& device)
    : mDevice{ device } {}

void WebGPUTextureSwizzler::swizzle(wgpu::Queue const& queue, wgpu::Texture const& source,
        wgpu::Texture const& destination, const TextureSwizzle red, const TextureSwizzle green,
        const TextureSwizzle blue, const TextureSwizzle alpha) {
    FILAMENT_CHECK_PRECONDITION(source.GetFormat() == destination.GetFormat())
            << "Source and destination formats must match";
    FILAMENT_CHECK_PRECONDITION(!hasDepth(source.GetFormat()))
            << "depth texture swizzling not supported";
    FILAMENT_CHECK_PRECONDITION(!hasStencil(source.GetFormat()))
            << "stencil texture swizzling not supported";
    FILAMENT_CHECK_PRECONDITION(source.GetDimension() == destination.GetDimension())
            << "Source and destination dimensions must match";
    FILAMENT_CHECK_PRECONDITION(source.GetDimension() == wgpu::TextureDimension::e2D)
            << "Source and destination dimensions must be 2D";
    FILAMENT_CHECK_PRECONDITION(source.GetWidth() == destination.GetWidth())
            << "Source and destination widths must match";
    FILAMENT_CHECK_PRECONDITION(source.GetHeight() == destination.GetHeight())
            << "Source and destination heights must match";
    FILAMENT_CHECK_PRECONDITION(
            source.GetDepthOrArrayLayers() == destination.GetDepthOrArrayLayers())
            << "Source and destination depthOrArrayLayers must match";
    FILAMENT_CHECK_PRECONDITION(source.GetMipLevelCount() == destination.GetMipLevelCount())
            << "Source and destination mip level counts must match";
    FILAMENT_CHECK_PRECONDITION(source.GetSampleCount() == destination.GetSampleCount())
            << "Source and destination sample counts must match";
    const wgpu::CommandEncoderDescriptor commandEncoderDescriptor{
        .label = "texture_swizzle_render_pass_cmd_encoder",
    };
    const wgpu::CommandEncoder commandEncoder{ mDevice.CreateCommandEncoder(
            &commandEncoderDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(commandEncoder)
            << "Failed to create command encoder for texture swizzle?";
    wgpu::BindGroupLayout const& bindGroupLayout{ getOrCreateBindGroupLayout(
            getScalarSampleTypeFrom(source.GetFormat()), source.GetSampleCount() > 1) };
    wgpu::RenderPipeline const& pipeline{ getOrCreateRenderPipelineFor(source.GetFormat(),
            source.GetSampleCount()) };
    // Create and upload the uniform buffer with swizzle parameters.
    uint32_t swizzleIndices[]{
        toSwizzleIndex(red),
        toSwizzleIndex(green),
        toSwizzleIndex(blue),
        toSwizzleIndex(alpha),
    };
    wgpu::BufferDescriptor bufferDescriptor{
        .label = "texture_swizzle_uniform_buffer",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = sizeof(swizzleIndices),
        .mappedAtCreation = false, // explicitly set for consistency
    };
    const wgpu::Buffer uniformBuffer{ mDevice.CreateBuffer(&bufferDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(uniformBuffer)
            << "Failed to create uniform buffer for texture swizzle?";
    queue.WriteBuffer(uniformBuffer, 0, swizzleIndices, sizeof(swizzleIndices));
    const uint32_t layerCount{ source.GetDepthOrArrayLayers() };
    for (uint32_t layer = 0; layer < layerCount; layer++) {
        for (uint32_t mipLevel = 1; mipLevel < source.GetMipLevelCount(); mipLevel++) {
            swizzle(commandEncoder, pipeline, bindGroupLayout, source, destination, layer, mipLevel,
                    uniformBuffer);
        }
    }
    // submit the command buffer...
    const wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "texture_swizzle_render_pass_cmd_buffer",
    };
    const wgpu::CommandBuffer commandBuffer{ commandEncoder.Finish(&commandBufferDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(commandBuffer)
            << "Failed to create command buffer for texture swizzle?";
    queue.Submit(1, &commandBuffer);
}

void WebGPUTextureSwizzler::swizzle(wgpu::CommandEncoder const& commandEncoder,
        wgpu::RenderPipeline const& renderPipeline, wgpu::BindGroupLayout const& bindGroupLayout,
        wgpu::Texture const& source, wgpu::Texture const& destination, const uint32_t layer,
        const uint32_t mipLevel, wgpu::Buffer const& uniformBuffer) {
    // create texture views for this pass...
    const wgpu::TextureFormat format{ source.GetFormat() };
    // only 2D textures supported at this time...
    const wgpu::TextureViewDimension dimension{ wgpu::TextureViewDimension::e2D };
    const wgpu::TextureViewDescriptor sourceViewDescriptor{
        .label = "source_of_texture_swizzle_view",
        .format = format,
        .dimension = dimension,
        .baseMipLevel = mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = layer,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::All,
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    const wgpu::TextureView sourceView{ source.CreateView(&sourceViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(sourceView)
            << "Failed to create source texture view for layer " << layer << " and mip level "
            << mipLevel << " for render pass texture swizzle?";
    const wgpu::TextureViewDescriptor destinationViewDescriptor{
        .label = "destination_of_texture_swizzle_view",
        .format = format,
        .dimension = dimension,
        .baseMipLevel = mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = layer,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::All,
        .usage = wgpu::TextureUsage::RenderAttachment,
    };
    const wgpu::TextureView destinationView{ destination.CreateView(&destinationViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(destinationView)
            << "Failed to create destination texture view for layer " << layer << " and mip level "
            << mipLevel << " for render pass texture swizzle?";
    // create the render pass...
    const wgpu::BindGroupEntry bindGroupEntries[BIND_GROUP_ENTRY_SIZE]{
        {
            .binding = TEXTURE_BINDING_INDEX,
            .textureView = sourceView,
        },
        {
            .binding = UNIFORM_BINDING_INDEX,
            .buffer = uniformBuffer,
        },
    };
    const wgpu::BindGroupDescriptor bindGroupDescriptor{
        .label = "render_pass_texture_swizzle_bind_group",
        .layout = bindGroupLayout,
        .entryCount = BIND_GROUP_ENTRY_SIZE,
        .entries = bindGroupEntries,
    };
    const wgpu::BindGroup bindGroup{ mDevice.CreateBindGroup(&bindGroupDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(bindGroup)
            << "Failed to create bind group for layer " << layer << " and mip level " << mipLevel
            << " for render pass texture swizzle?";
    const wgpu::RenderPassColorAttachment colorAttachment{
        .view = destinationView,
        .depthSlice = wgpu::kDepthSliceUndefined, // not applicable
        .resolveTarget = nullptr,                 // not applicable
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
    };
    const wgpu::RenderPassDescriptor renderPassDescriptor{
        .label = "texture_swizzle_render_pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment,
        .depthStencilAttachment = nullptr, // not applicable
        .occlusionQuerySet = nullptr,      // not applicable
        .timestampWrites = nullptr,        // not applicable
    };
    const wgpu::RenderPassEncoder renderPassEncoder{ commandEncoder.BeginRenderPass(
            &renderPassDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(renderPassEncoder)
            << "Failed to create render pass encoder for layer " << layer << " and mip level "
            << mipLevel << " for render pass texture swizzle?";
    renderPassEncoder.SetPipeline(renderPipeline);
    renderPassEncoder.SetBindGroup(BIND_GROUP_INDEX, bindGroup);
    renderPassEncoder.Draw(3); // draw the full-screen triangle
                               // with hard-coded vertices in the shader
    renderPassEncoder.End();
}

wgpu::RenderPipeline const& WebGPUTextureSwizzler::getOrCreateRenderPipelineFor(
        const wgpu::TextureFormat format, const uint32_t sampleCount) {
    const bool multiSampled{ sampleCount > 1 };
    const size_t key{ hashPipelineKey(format, sampleCount) };
    if (mPipelines.find(key) == mPipelines.end()) {
        const ScalarSampleType scalarTextureFormat{ getScalarSampleTypeFrom(format) };
        const wgpu::ShaderModule& shaderModule{ getOrCreateShaderModuleFor(scalarTextureFormat,
                multiSampled) };
        const wgpu::PipelineLayout& pipelineLayout{ getOrCreatePipelineLayoutFor(
                scalarTextureFormat, multiSampled) };
        mPipelines[key] = createRenderPipeline(shaderModule, pipelineLayout, format, sampleCount);
    }
    return mPipelines[key];
}

wgpu::RenderPipeline WebGPUTextureSwizzler::createRenderPipeline(
        wgpu::ShaderModule const& shaderModule, wgpu::PipelineLayout const& pipelineLayout,
        const wgpu::TextureFormat textureFormat, const uint32_t sampleCount) {
    const wgpu::ColorTargetState colorTargetState{ .format = textureFormat };
    const wgpu::FragmentState fragmentState{
        .module = shaderModule,
        .entryPoint = VERTEX_SHADER_ENTRY_POINT,
        .constantCount = 0,   // should not matter, just being consistently defined
        .constants = nullptr, // should not matter, just being consistently defined
        .targetCount = 1,
        .targets = &colorTargetState,
    };
    const wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = "render_pass_texture_swizzle_pipeline",
        .layout = pipelineLayout,
        .vertex = {
          .module = shaderModule,
            .entryPoint = FRAGMENT_SHADER_ENTRY_POINT,
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = 0,
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
            .count = sampleCount,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };
    const wgpu::RenderPipeline pipeline{ mDevice.CreateRenderPipeline(&pipelineDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create pipeline for render pass texture swizzle?";
    return pipeline;
}

wgpu::PipelineLayout const& WebGPUTextureSwizzler::getOrCreatePipelineLayoutFor(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    const size_t key{ hashPipelineLayoutKey(scalarTextureFormat, multiSampled) };
    if (mPipelineLayouts.find(key) == mPipelineLayouts.end()) {
        mPipelineLayouts[key] = createPipelineLayout(scalarTextureFormat, multiSampled);
    }
    return mPipelineLayouts[key];
}

wgpu::PipelineLayout WebGPUTextureSwizzler::createPipelineLayout(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    wgpu::BindGroupLayout const& bindGroupLayout{ getOrCreateBindGroupLayout(scalarTextureFormat,
            multiSampled) };
#ifndef NDEBUG
    std::stringstream labelStream{};
    labelStream << "texture_swizzle_pipeline_layout_" << toWGSLString(scalarTextureFormat)
                << (multiSampled ? "_multisampled" : "_singlesampled");
    const wgpu::StringView label{ labelStream.str() };
#else
    const wgpu::StringView label{ "texture_swizzle_pipeline_layout" };
#endif
    const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{
        .label = label,
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bindGroupLayout,
    };
    const wgpu::PipelineLayout pipelineLayout{ mDevice.CreatePipelineLayout(
            &pipelineLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipelineLayout)
            << "Failed to create pipeline layout for texture swizzling?";
    return pipelineLayout;
}

wgpu::BindGroupLayout const& WebGPUTextureSwizzler::getOrCreateBindGroupLayout(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    const size_t key{ hashBindGroupLayoutKey(scalarTextureFormat, multiSampled) };
    if (mBindGroupLayouts.find(key) == mBindGroupLayouts.end()) {
        mBindGroupLayouts[key] = createBindGroupLayout(scalarTextureFormat, multiSampled);
    }
    return mBindGroupLayouts[key];
}

wgpu::BindGroupLayout WebGPUTextureSwizzler::createBindGroupLayout(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    wgpu::TextureSampleType sampleType{ wgpu::TextureSampleType::Undefined };
    switch (scalarTextureFormat) {
        case ScalarSampleType::F32:
            sampleType = wgpu::TextureSampleType::UnfilterableFloat;
            break;
        case ScalarSampleType::I32:
            sampleType = wgpu::TextureSampleType::Sint;
            break;
        case ScalarSampleType::U32:
            sampleType = wgpu::TextureSampleType::Uint;
            break;
    }
    const wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[BIND_GROUP_ENTRY_SIZE] {
        {
            .binding = TEXTURE_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = {
                .sampleType = sampleType,
                .viewDimension = wgpu::TextureViewDimension::e2D, // only 2D supported for now
                .multisampled = multiSampled,
            },
        },
        {
            .binding = UNIFORM_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .buffer = {
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = false, // set explicitly for consistency
                .minBindingSize = 0, // TODO set based on the swizzle data size
            },
        },
    };
    const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor{
        .label = "render_pass_texture_swizzle_bind_group_layout",
        .entryCount = BIND_GROUP_ENTRY_SIZE,
        .entries = bindGroupLayoutEntries,
    };
    const wgpu::BindGroupLayout bindGroupLayout{ mDevice.CreateBindGroupLayout(
            &bindGroupLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(bindGroupLayout)
            << "Failed to create texture bind group layout for render pass texture swizzle?";
    return bindGroupLayout;
}

wgpu::ShaderModule const& WebGPUTextureSwizzler::getOrCreateShaderModuleFor(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    const size_t key{ hashShaderModuleKey(scalarTextureFormat, multiSampled) };
    if (mShaderModules.find(key) == mShaderModules.end()) {
        const wgpu::ShaderModule shaderModule{ createShaderModule(scalarTextureFormat,
                multiSampled) };
        mShaderModules[key] = shaderModule;
    }
    return mShaderModules[key];
}

wgpu::ShaderModule WebGPUTextureSwizzler::createShaderModule(
        const ScalarSampleType scalarTextureFormat, const bool multiSampled) {
    // build the proper source from the template...
    const std::string_view scalarType{ toWGSLString(scalarTextureFormat) };
    std::stringstream textureTypeStream{};
    textureTypeStream << (multiSampled ? "texture_multisampled_2d" : "texture_2d") << '<'
                      << scalarType << '>';
    const std::string textureType{ textureTypeStream.str() };
    const std::string_view sampleIndexParam{ multiSampled ? ", @builtin(sample_index) sampleIndex"
                                                          : "" };
    const std::string_view sampleIndexOrLevel{ multiSampled ? "sampleIndex" : "0" };
    const char* const sourceData = SHADER_SOURCE_TEMPLATE.data();
    std::stringstream processedShaderSource{};
    size_t positionCursorInTemplateString{ 0 };
    while (positionCursorInTemplateString < SHADER_SOURCE_TEMPLATE.size()) {
        const size_t positionOfNextPlaceholder{ SHADER_SOURCE_TEMPLATE.find(PLACEHOLDER_PREFIX,
                positionCursorInTemplateString) };
        if (positionOfNextPlaceholder == std::string::npos) {
            // no more placeholders, so just stream the rest of the source code string
            processedShaderSource << std::string_view(sourceData + positionCursorInTemplateString,
                    SHADER_SOURCE_TEMPLATE.size() - positionCursorInTemplateString);
            break;
        }
        const size_t positionOfPlaceholder{ positionOfNextPlaceholder + PLACEHOLDER_PREFIX.size() };
        // stream up to the placeholder...
        processedShaderSource << std::string_view(sourceData + positionCursorInTemplateString,
                positionOfPlaceholder);
        const size_t positionAfterPlaceholder{ SHADER_SOURCE_TEMPLATE.find(
                PLACEHOLDER_SUFFIX, positionOfPlaceholder) };
        assert_invariant(positionAfterPlaceholder != std::string::npos &&
                         "Malformed shader with missing suffix to placeholder");
        const std::string_view placeholder{ std::string_view(sourceData + positionOfPlaceholder,
                positionAfterPlaceholder - positionOfPlaceholder) };
        // stream the value in place of the placeholder...
        if (placeholder == TEXTURE_TYPE_PLACEHOLDER) {
            processedShaderSource << textureType;
        } else if (placeholder == SCALAR_TYPE_PLACEHOLDER) {
            processedShaderSource << scalarType;
        } else if (placeholder == SAMPLE_INDEX_PARAM_PLACEHOLDER) {
            processedShaderSource << sampleIndexParam;
        } else if (placeholder == SAMPLE_INDEX_OR_LEVEL_PLACEHOLDER) {
            processedShaderSource << sampleIndexOrLevel;
        } else {
            PANIC_POSTCONDITION(
                    "Unknown/unhandled placeholder %s when processing texture swizzler shader",
                    placeholder.data());
        }
        // update the cursor for after the placeholder...
        positionCursorInTemplateString = positionAfterPlaceholder + PLACEHOLDER_SUFFIX.size();
    }
    const std::string shaderSource{ processedShaderSource.str() };
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = shaderSource.data();
#ifndef NDEBUG
    std::stringstream labelStream{};
    labelStream << "texture_swizzle_shaders_" << scalarType
                << (multiSampled ? "_multisampled" : "_singlesampled");
    const wgpu::StringView label{ labelStream.str() };
#else
    const wgpu::StringView label{ "texture_swizzle_shaders" };
#endif
    const wgpu::ShaderModuleDescriptor shaderModuleDescriptor{
        .nextInChain = &wgslDescriptor,
        .label = label,
    };
    const wgpu::ShaderModule shaderModule{ mDevice.CreateShaderModule(&shaderModuleDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(shaderModule)
            << "Failed to create shader module for texture swizzling? scalarType " << scalarType
            << " multiSampled " << multiSampled;
    return shaderModule;
}

size_t WebGPUTextureSwizzler::hashBindGroupLayoutKey(const ScalarSampleType textureScalarFormat,
        const bool multiSampled) {
    size_t seed{ std::hash<uint32_t>{}(static_cast<uint8_t>(textureScalarFormat)) };
    utils::hash::combine(seed, multiSampled);
    return seed;
}

size_t WebGPUTextureSwizzler::hashPipelineLayoutKey(const ScalarSampleType textureScalarFormat,
        const bool multiSampled) {
    size_t seed{ std::hash<uint32_t>{}(static_cast<uint8_t>(textureScalarFormat)) };
    utils::hash::combine(seed, multiSampled);
    return seed;
}

size_t WebGPUTextureSwizzler::hashShaderModuleKey(const ScalarSampleType textureScalarFormat,
        const bool multiSampled) {
    size_t seed{ std::hash<uint32_t>{}(static_cast<uint8_t>(textureScalarFormat)) };
    utils::hash::combine(seed, multiSampled);
    return seed;
}

size_t WebGPUTextureSwizzler::hashPipelineKey(const wgpu::TextureFormat format,
        const uint32_t sampleCount) {
    size_t seed{ std::hash<uint32_t>{}(static_cast<uint32_t>(format)) };
    utils::hash::combine(seed, sampleCount);
    return seed;
}

} // namespace filament::backend
