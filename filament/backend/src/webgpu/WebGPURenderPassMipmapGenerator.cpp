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

#include "WebGPURenderPassMipmapGenerator.h"

#include "WebGPUTextureHelpers.h"

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <string_view>

namespace filament::backend {

namespace {

// The shaders expect a single-sampled 2D texture
// with a format compatible with scalar sample format f32
// (this is checked with the getCompatibilityFor function)
constexpr uint32_t TEXTURE_BIND_GROUP_INDEX{ 0 };
constexpr size_t TEXTURE_BIND_GROUP_ENTRY_SIZE{ 2 }; // sampler and texture
constexpr uint32_t SAMPLER_BINDING_INDEX{ 0 };
constexpr uint32_t TEXTURE_BINDING_INDEX{ 1 };
constexpr std::string_view VERTEX_SHADER_ENTRY_POINT{ "vertexShaderMain" };
constexpr std::string_view FRAGMENT_SHADER_ENTRY_POINT{ "fragmentShaderMain" };
// Note: would be nice to generate this constexpr string from the constexpr variables above
// to avoid them getting out of sync while still having the compile/build time optimization
constexpr std::string_view SHADER_SOURCE{ R"(
    @group(0) @binding(0) var previousMipLevelSampler: sampler;
    @group(0) @binding(1) var previousMipLevelTexture: texture_2d<f32>;

    struct VertexShaderOutput {
      @builtin(position) position: vec4<f32>,
      @location(0) textureCoordinate: vec2<f32>,
    };

    @vertex
    fn vertexShaderMain(@builtin(vertex_index) vertexIndex: u32) -> VertexShaderOutput {
        let fullScreenTriangleVertices = array<vec2<f32>, 3>(
            vec2<f32>(-1.0, -1.0),
            vec2<f32>(3.0, -1.0),
            vec2<f32>(-1.0, 3.0)
        );
        var out: VertexShaderOutput;
        let vertex = fullScreenTriangleVertices[vertexIndex];
        out.position = vec4<f32>(vertex * 2.0 - 1.0, 0.0, 1.0);
        out.textureCoordinate = vec2<f32>(vertex.x, 1.0 - vertex.y);
        return out;
    }

    @fragment
    fn fragmentShaderMain(in: VertexShaderOutput) -> @location(0) vec4<f32> {
        return textureSample(previousMipLevelTexture,
                             previousMipLevelSampler,
                             in.textureCoordinate);
    }
)" };

[[nodiscard]] wgpu::Sampler createPreviousMipLevelSampler(wgpu::Device const& device) {
    const wgpu::SamplerDescriptor descriptor{
        .label = "previous_mip_level_sampler",
        .addressModeU = wgpu::AddressMode::ClampToEdge,
        .addressModeV = wgpu::AddressMode::ClampToEdge,
        .addressModeW = wgpu::AddressMode::Undefined, // only 2D textures supported for now
        .magFilter = wgpu::FilterMode::Linear,
        .minFilter = wgpu::FilterMode::Linear,
        .mipmapFilter = wgpu::MipmapFilterMode::Nearest, // unused for this purpose
        .lodMinClamp = 0.0f,  // should not matter, just being consistently defined
        .lodMaxClamp = 32.0f, // should not matter, just being consistently defined
        .compare = wgpu::CompareFunction::Undefined, // should not matter, just being consistently
                                                     // defined
        .maxAnisotropy = 1, // should not matter, just being consistently defined
    };
    const wgpu::Sampler sampler{ device.CreateSampler(&descriptor) };
    FILAMENT_CHECK_POSTCONDITION(sampler) << "Failed to create previous mip level sampler?";
    return sampler;
}

[[nodiscard]] wgpu::ShaderModule createShaderModule(wgpu::Device const& device) {
    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = SHADER_SOURCE.data();
    const wgpu::ShaderModuleDescriptor shaderModuleDescriptor{
        .nextInChain = &wgslDescriptor,
        .label = "render_pass_mipmap_generation_shaders",
    };
    const wgpu::ShaderModule shaderModule{ device.CreateShaderModule(&shaderModuleDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(shaderModule)
            << "Failed to create shader module for render pass mipmap generation?";
    return shaderModule;
}

[[nodiscard]] wgpu::BindGroupLayout createTextureBindGroupLayout(wgpu::Device const& device) {
    const wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[TEXTURE_BIND_GROUP_ENTRY_SIZE] {
        {
            .binding = SAMPLER_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .sampler = {
                .type = wgpu::SamplerBindingType::Filtering, // only hardware accelerated filtering
                                                             // supported
            },
        },
        {
            .binding = TEXTURE_BINDING_INDEX,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = {
                .sampleType = wgpu::TextureSampleType::Float, // only F32 scalar sample
                                                              // type supported for now
                .viewDimension = wgpu::TextureViewDimension::e2D, // only 2D supported for now
                .multisampled = false, // MSAA textures not supported for now
            },
        },
    };
    const wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDescriptor{
        .label = "render_pass_mipmap_generation_texture_bind_group_layout",
        .entryCount = TEXTURE_BIND_GROUP_ENTRY_SIZE,
        .entries = bindGroupLayoutEntries,
    };
    const wgpu::BindGroupLayout textureBindGroupLayout{ device.CreateBindGroupLayout(
            &textureBindGroupLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(textureBindGroupLayout)
            << "Failed to create texture bind group layout for render pass mipmap generation?";
    return textureBindGroupLayout;
}

[[nodiscard]] wgpu::PipelineLayout createPipelineLayout(wgpu::Device const& device,
        wgpu::BindGroupLayout const& textureBindGroupLayout) {
    const wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{
        .label = "render_pass_mipmap_generation_pipeline_layout",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &textureBindGroupLayout,
    };
    const wgpu::PipelineLayout pipelineLayout{ device.CreatePipelineLayout(
            &pipelineLayoutDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipelineLayout)
            << "Failed to create pipeline layout for render pass mipmap generation?";
    return pipelineLayout;
}

[[nodiscard]] wgpu::RenderPipeline createPipeline(wgpu::Device const& device,
        wgpu::ShaderModule const& shaderModule, wgpu::PipelineLayout const& pipelineLayout,
        const wgpu::TextureFormat textureFormat) {
    const wgpu::ColorTargetState colorTargetState{ .format = textureFormat };
    const wgpu::FragmentState fragmentState{
        .module = shaderModule,
        .entryPoint = FRAGMENT_SHADER_ENTRY_POINT,
        .constantCount = 0,   // should not matter, just being consistently defined
        .constants = nullptr, // should not matter, just being consistently defined
        .targetCount = 1,
        .targets = &colorTargetState,
    };
    const wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = "render_pass_mipmap_generation_pipeline",
        .layout = pipelineLayout,
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
            .count = 1, // no multi-sampling
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragmentState,
    };
    const wgpu::RenderPipeline pipeline{ device.CreateRenderPipeline(&pipelineDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create pipeline for render pass mipmap generation?";
    return pipeline;
}

} // namespace

WebGPURenderPassMipmapGenerator::WebGPURenderPassMipmapGenerator(wgpu::Device const& device)
    : mDevice{ device },
      mPreviousMipLevelSampler{ createPreviousMipLevelSampler(mDevice) },
      mShaderModule{ createShaderModule(mDevice) },
      mTextureBindGroupLayout{ createTextureBindGroupLayout(mDevice) },
      mPipelineLayout{ createPipelineLayout(mDevice, mTextureBindGroupLayout) } {}

WebGPURenderPassMipmapGenerator::FormatCompatibility
WebGPURenderPassMipmapGenerator::getCompatibilityFor(const wgpu::TextureFormat format,
        const wgpu::TextureDimension dimension, const uint32_t sampleCount) {
    // check that the format is compatible (currently expects that the scalar sample type is
    // f32)...
    switch (format) {
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth24Plus:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return {
                .compatible = false,
                .reason = "A depth texture format requires special sampler treatment and does "
                          "not "
                          "generally support linear filtering needed for render pass based "
                          "mipmap "
                          "generation, thus this texture is not supported, as it is a kind of "
                          "depth texture.",
            };
        case wgpu::TextureFormat::Undefined:
        case wgpu::TextureFormat::External:
            return {
                .compatible = false,
                .reason = "Undefined or External textures are not supported for render pass "
                          "based "
                          "mipmap generation",
            };
        default:
            const ScalarSampleType scalarSampleType{ getScalarSampleTypeFrom(format) };
            switch (scalarSampleType) {
                case ScalarSampleType::U32:
                    return {
                        .compatible = false,
                        .reason = "The provided texture format requires an unsigned integer "
                                  "sampler, which does not natively support linear filtering, "
                                  "which is needed for render pass based mipmap generation.",
                    };
                case ScalarSampleType::I32:
                    return {
                        .compatible = false,
                        .reason = "The provided texture format requires a signed integer "
                                  "sampler, which does not natively support linear filtering, "
                                  "which is needed for render pass based mipmap generation.",
                    };
                case ScalarSampleType::F32:
                    break;
            }
    }
    // check that the dimensionality is compatible (currently expects 2D textures)...
    switch (dimension) {
        case wgpu::TextureDimension::Undefined:
            return {
                .compatible = false,
                .reason = "Texture has an undefined texture dimension (expected 2D), thus it is "
                          "not "
                          "supported by the render pass based mipmap generation implementation.",
            };
        case wgpu::TextureDimension::e1D:
            return {
                .compatible = false,
                .reason = "Texture has a 1D texture dimension (expected 2D), thus it is not "
                          "current supported by the render pass based mipmap generation "
                          "implementation (but could be extended to later if needed).",
            };
        case wgpu::TextureDimension::e3D:
            return {
                .compatible = false,
                .reason = "Texture has a 3D texture dimension (expected 2D), thus it is not "
                          "current supported by the render pass based mipmap generation "
                          "implementation (but could be extended to later if REALLY needed).",
            };
        case wgpu::TextureDimension::e2D:
            break;
    }
    // check that the texture is single-sampled (for now)...
    if (sampleCount > 1) {
        return {
            .compatible = false,
            .reason = "Multi-sampled textures are not currently supported with the render pass "
                      "based mipmap generation implementation (but could be extended to later if "
                      "REALLY needed. The idea would be to resolve the texture to a single "
                      "sampled "
                      "texture before generating single-sample mipmaps, but that may still not "
                      "necessarily be what is desired.).",
        };
    }
    return { .compatible = true, .reason = "" };
}

void WebGPURenderPassMipmapGenerator::generateMipmaps(wgpu::Queue const& queue,
        wgpu::Texture const& texture) {
    const uint32_t mipLevelCount{ texture.GetMipLevelCount() };
    if (mipLevelCount < 2) {
        return; // nothing to do
    }
    wgpu::RenderPipeline const& pipeline{ getOrCreatePipelineFor(texture.GetFormat()) };
    const wgpu::CommandEncoderDescriptor commandEncoderDescriptor{
        .label = "mipmap_generation_render_pass_cmd_encoder",
    };
    const wgpu::CommandEncoder commandEncoder{ mDevice.CreateCommandEncoder(
            &commandEncoderDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(commandEncoder)
            << "Failed to create command encoder for layer for render pass mipmap generation?";
    const uint32_t layerCount{ texture.GetDepthOrArrayLayers() };
    for (uint32_t layer = 0; layer < layerCount; layer++) {
        for (uint32_t mipLevel = 1; mipLevel < mipLevelCount; mipLevel++) {
            generateMipmap(commandEncoder, texture, pipeline, layer, mipLevel);
        }
    }
    // submit the command buffer...
    const wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "mipmap_generation_render_pass_cmd_buffer",
    };
    const wgpu::CommandBuffer commandBuffer{ commandEncoder.Finish(&commandBufferDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(commandBuffer)
            << "Failed to create command buffer for layer for render pass mipmap generation?";
    queue.Submit(1, &commandBuffer);
}

void WebGPURenderPassMipmapGenerator::generateMipmap(wgpu::CommandEncoder const& commandEncoder,
        const wgpu::Texture& texture, const wgpu::RenderPipeline& pipeline, const uint32_t layer,
        const uint32_t mipLevel) {
    // create texture views for this pass...
    const wgpu::TextureFormat format{ texture.GetFormat() };
    // only 2D textures supported at this time...
    const wgpu::TextureViewDimension dimension{ wgpu::TextureViewDimension::e2D };
    const wgpu::TextureViewDescriptor sourceViewDescriptor{
        .label = "previous_mip_level_texture_view",
        .format = format,
        .dimension = dimension,
        .baseMipLevel = mipLevel - 1,
        .mipLevelCount = 1,
        .baseArrayLayer = layer,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::All,
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    const wgpu::TextureView sourceView{ texture.CreateView(&sourceViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create source texture view for layer " << layer << " and mip level "
            << mipLevel << " for render pass mipmap generation?";
    const wgpu::TextureViewDescriptor destinationViewDescriptor{
        .label = "next_mip_level_texture_view",
        .format = format,
        .dimension = dimension,
        .baseMipLevel = mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = layer,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::All,
        .usage = wgpu::TextureUsage::RenderAttachment,
    };
    const wgpu::TextureView destinationView{ texture.CreateView(&destinationViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create destination texture view for layer " << layer << " and mip level "
            << mipLevel << " for render pass mipmap generation?";
    // create the render pass...
    const wgpu::BindGroupEntry textureBindGroupEntries[TEXTURE_BIND_GROUP_ENTRY_SIZE]{
        {
            .binding = SAMPLER_BINDING_INDEX,
            .sampler = mPreviousMipLevelSampler,
        },
        {
            .binding = TEXTURE_BINDING_INDEX,
            .textureView = sourceView,
        },
    };
    const wgpu::BindGroupDescriptor textureBindGroupDescriptor{
        .label = "render_pass_mipmap_generation_texture_bind_group",
        .layout = mTextureBindGroupLayout,
        .entryCount = TEXTURE_BIND_GROUP_ENTRY_SIZE,
        .entries = textureBindGroupEntries,
    };
    const wgpu::BindGroup textureBindGroup{ mDevice.CreateBindGroup(&textureBindGroupDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create texture bind group for layer " << layer << " and mip level "
            << mipLevel << " for render pass mipmap generation?";
    const wgpu::RenderPassColorAttachment colorAttachment{
        .view = destinationView,
        .depthSlice = wgpu::kDepthSliceUndefined, // not applicable
        .resolveTarget = nullptr,                 // not applicable
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 },
    };
    const wgpu::RenderPassDescriptor renderPassDescriptor{
        .label = "mipmap_generation_render_pass",
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
            << mipLevel << " for render pass mipmap generation?";
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(TEXTURE_BIND_GROUP_INDEX, textureBindGroup);
    renderPassEncoder.Draw(3); // draw the full-screen triangle
                                          // with hard-coded vertices in the shader
    renderPassEncoder.End();
}

wgpu::RenderPipeline& WebGPURenderPassMipmapGenerator::getOrCreatePipelineFor(
        const wgpu::TextureFormat textureFormat) {
    const uint32_t key{ static_cast<uint32_t>(textureFormat) };
    if (mPipelineByTextureFormat.find(key) == mPipelineByTextureFormat.end()) {
        mPipelineByTextureFormat[key] =
                createPipeline(mDevice, mShaderModule, mPipelineLayout, textureFormat);
    }
    return mPipelineByTextureFormat[key];
}

} // namespace filament::backend
