// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/BlitBufferToDepthStencil.h"

#include <string>
#include <string_view>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"

namespace dawn::native {

namespace {

constexpr char kBlitRG8ToDepthShaders[] = R"(

@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32
) -> @builtin(position) vec4f {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return vec4f(pos[vertex_index], 0.0, 1.0);
}

struct Params {
  origin : vec2u
};

@group(0) @binding(0) var src_tex : texture_2d<u32>;
@group(0) @binding(1) var<uniform> params : Params;

@fragment fn blit_to_depth(
    @builtin(position) position : vec4f
) -> @builtin(frag_depth) f32 {
  // Load the source texel.
  let src_texel = textureLoad(
    src_tex, vec2u(position.xy) - params.origin, 0u);

  let depth_u16_val = (src_texel.y << 8u) + src_texel.x;

  const one_over_max : f32 = 1.0 / f32(0xFFFFu);
  return f32(depth_u16_val) * one_over_max;
}

)";

constexpr std::string_view kTexture2DHead = R"(
fn textureLoadGeneral(tex: texture_2d<u32>, coords: vec2u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords, level);
}
@group(0) @binding(0) var src_tex : texture_2d<u32>;
)";

constexpr std::string_view kTexture2DArrayHead = R"(
fn textureLoadGeneral(tex: texture_2d_array<u32>, coords: vec2u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords, params.layer, level);
}
@group(0) @binding(0) var src_tex : texture_2d_array<u32>;
)";

constexpr std::string_view kBlitStencilShaderCommon = R"(
struct Params {
  origin : vec2u,
  layer: u32,
};
@group(0) @binding(1) var<uniform> params : Params;

struct VertexOutputs {
  @location(0) @interpolate(flat, either) stencil_val : u32,
  @builtin(position) position : vec4f,
};

// The instance_index here is not used for instancing.
// It represents the current stencil mask we're testing in the
// source.
// This is a cheap way to get the stencil value into the shader
// since WebGPU doesn't have push constants.
@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32,
  @builtin(instance_index) instance_index: u32,
) -> VertexOutputs {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return VertexOutputs(
    instance_index,
    vec4f(pos[vertex_index], 0.0, 1.0),
  );
}

// Do nothing (but also don't discard). Used for clearing
// stencil to 0.
@fragment fn frag_noop() {}

// Discard the fragment if the source texture doesn't
// have the stencil_val.
@fragment fn frag_check_src_stencil(input : VertexOutputs) {
  // Load the source stencil value.
  let src_val : u32 = textureLoadGeneral(
    src_tex, vec2u(input.position.xy) - params.origin, 0u)[0];

  // Discard it if it doesn't contain the stencil reference.
  if ((src_val & input.stencil_val) == 0u) {
    discard;
  }
}
)";

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateRG8ToDepth16UnormPipeline(DeviceBase* device) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    if (store->blitRG8ToDepth16UnormPipeline != nullptr) {
        return store->blitRG8ToDepth16UnormPipeline;
    }

    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitRG8ToDepthShaders;

    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = shaderModule.Get();
    fragmentState.entryPoint = "blit_to_depth";

    DepthStencilState dsState = {};
    dsState.format = wgpu::TextureFormat::Depth16Unorm;
    dsState.depthWriteEnabled = wgpu::OptionalBool::True;
    dsState.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.depthStencil = &dsState;
    renderPipelineDesc.fragment = &fragmentState;

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->blitRG8ToDepth16UnormPipeline = pipeline;
    return pipeline;
}

ResultOrError<InternalPipelineStore::BlitR8ToStencilPipelines> GetOrCreateR8ToStencilPipelines(
    DeviceBase* device,
    wgpu::TextureFormat format,
    wgpu::TextureViewDimension viewDimension,
    BindGroupLayoutBase* bgl) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->blitR8ToStencilPipelines.find({format, viewDimension});
        if (it != store->blitR8ToStencilPipelines.end()) {
            return InternalPipelineStore::BlitR8ToStencilPipelines{it->second};
        }
    }

    Ref<PipelineLayoutBase> pipelineLayout;
    {
        PipelineLayoutDescriptor plDesc = {};
        plDesc.bindGroupLayoutCount = 1;

        plDesc.bindGroupLayouts = &bgl;
        DAWN_TRY_ASSIGN(pipelineLayout, device->CreatePipelineLayout(&plDesc));
    }

    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    std::string shader;
    switch (viewDimension) {
        case wgpu::TextureViewDimension::e2D:
            shader = kTexture2DHead;
            break;
        case wgpu::TextureViewDimension::e2DArray:
            shader = kTexture2DArrayHead;
            break;
        default:
            DAWN_UNREACHABLE();
    }
    shader += kBlitStencilShaderCommon;
    wgslDesc.code = shader.c_str();

    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = shaderModule.Get();

    DepthStencilState dsState = {};
    dsState.format = format;
    dsState.depthWriteEnabled = wgpu::OptionalBool::False;
    dsState.depthCompare = wgpu::CompareFunction::Always;
    dsState.stencilFront.passOp = wgpu::StencilOperation::Replace;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.layout = pipelineLayout.Get();
    renderPipelineDesc.vertex.module = shaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.depthStencil = &dsState;
    renderPipelineDesc.fragment = &fragmentState;

    // Build a pipeline to clear stencil to 0. We need a pipeline, and not just a render pass load
    // op because the copy region may be a subregion of the stencil texture.
    Ref<RenderPipelineBase> clearPipeline;
    fragmentState.entryPoint = "frag_noop";
    DAWN_TRY_ASSIGN(clearPipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    // Build 8 pipelines masked to replace each bit of the stencil.
    std::array<Ref<RenderPipelineBase>, 8> setStencilPipelines;
    fragmentState.entryPoint = "frag_check_src_stencil";
    for (uint32_t bit = 0; bit < 8; ++bit) {
        dsState.stencilWriteMask = 1u << bit;
        DAWN_TRY_ASSIGN(setStencilPipelines[bit],
                        device->CreateRenderPipeline(&renderPipelineDesc));
    }

    InternalPipelineStore::BlitR8ToStencilPipelines pipelines{std::move(clearPipeline),
                                                              std::move(setStencilPipelines)};
    store->blitR8ToStencilPipelines.emplace(std::make_pair(format, viewDimension), pipelines);
    return pipelines;
}

MaybeError BlitRG8ToDepth16Unorm(DeviceBase* device,
                                 CommandEncoder* commandEncoder,
                                 TextureBase* dataTexture,
                                 const TextureCopy& dst,
                                 const Extent3D& copyExtent) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    DAWN_ASSERT(dst.texture->GetFormat().format == wgpu::TextureFormat::Depth16Unorm);
    DAWN_ASSERT(dataTexture->GetFormat().format == wgpu::TextureFormat::RG8Uint);

    // Allow internal usages since we need to use the destination
    // as a render attachment.
    auto scope = commandEncoder->MakeInternalUsageScope();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateRG8ToDepth16UnormPipeline(device));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    for (uint32_t z = 0; z < copyExtent.depthOrArrayLayers; ++z) {
        Ref<TextureViewBase> srcView;
        {
            TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseArrayLayer = z;
            viewDesc.arrayLayerCount = 1;
            viewDesc.mipLevelCount = 1;
            DAWN_TRY_ASSIGN(srcView, dataTexture->CreateView(&viewDesc));
        }

        Ref<TextureViewBase> dstView;
        {
            TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseArrayLayer = dst.origin.z + z;
            viewDesc.arrayLayerCount = 1;
            viewDesc.baseMipLevel = dst.mipLevel;
            viewDesc.mipLevelCount = 1;
            DAWN_TRY_ASSIGN(dstView, dst.texture->CreateView(&viewDesc));
        }

        Ref<BufferBase> paramsBuffer;
        {
            BufferDescriptor bufferDesc = {};
            bufferDesc.size = sizeof(uint32_t) * 2;
            bufferDesc.usage = wgpu::BufferUsage::Uniform;
            bufferDesc.mappedAtCreation = true;
            DAWN_TRY_ASSIGN(paramsBuffer, device->CreateBuffer(&bufferDesc));

            uint32_t* params =
                static_cast<uint32_t*>(paramsBuffer->GetMappedRange(0, bufferDesc.size));
            params[0] = dst.origin.x;
            params[1] = dst.origin.y;
            DAWN_TRY(paramsBuffer->Unmap());
        }

        Ref<BindGroupBase> bindGroup;
        {
            std::array<BindGroupEntry, 2> bgEntries = {};
            bgEntries[0].binding = 0;
            bgEntries[0].textureView = srcView.Get();
            bgEntries[1].binding = 1;
            bgEntries[1].buffer = paramsBuffer.Get();

            BindGroupDescriptor bgDesc = {};
            bgDesc.layout = bgl.Get();
            bgDesc.entryCount = bgEntries.size();
            bgDesc.entries = bgEntries.data();
            DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bgDesc));
        }

        RenderPassDepthStencilAttachment dsAttachment;
        dsAttachment.view = dstView.Get();
        dsAttachment.depthClearValue = 0.0;
        dsAttachment.depthLoadOp = wgpu::LoadOp::Load;
        dsAttachment.depthStoreOp = wgpu::StoreOp::Store;

        RenderPassDescriptor rpDesc = {};
        rpDesc.depthStencilAttachment = &dsAttachment;

        Ref<RenderPassEncoder> pass = commandEncoder->BeginRenderPass(&rpDesc);
        // Bind the resources.
        pass->APISetBindGroup(0, bindGroup.Get());
        // Discard all fragments outside the copy region.
        pass->APISetScissorRect(dst.origin.x, dst.origin.y, copyExtent.width, copyExtent.height);

        // Draw to perform the blit.
        pass->APISetPipeline(pipeline.Get());
        pass->APIDraw(3, 1, 0, 0);

        pass->End();
    }
    return {};
}

}  // anonymous namespace

MaybeError BlitR8ToStencil(DeviceBase* device,
                           CommandEncoder* commandEncoder,
                           TextureBase* dataTexture,
                           const TextureCopy& dst,
                           const Extent3D& copyExtent) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    const Format& format = dst.texture->GetFormat();
    DAWN_ASSERT(dst.aspect == Aspect::Stencil);

    // Allow internal usages since we need to use the destination
    // as a render attachment.
    auto scope = commandEncoder->MakeInternalUsageScope();

    // In compat mode view dimension needs to match texture binding view dimension.
    wgpu::TextureViewDimension textureViewDimension;
    if (!device->HasFlexibleTextureViews()) {
        textureViewDimension = dataTexture->GetCompatibilityTextureBindingViewDimension();
    } else {
        textureViewDimension = wgpu::TextureViewDimension::e2DArray;
    }

    // This bgl is the same for all the render pipelines.
    Ref<BindGroupLayoutBase> bgl;
    {
        std::array<BindGroupLayoutEntry, 2> bglEntries = {};
        // Binding 0: the r8uint texture.
        bglEntries[0].binding = 0;
        bglEntries[0].visibility = wgpu::ShaderStage::Fragment;
        bglEntries[0].texture.sampleType = wgpu::TextureSampleType::Uint;
        bglEntries[0].texture.viewDimension = textureViewDimension;
        // Binding 1: the params buffer.
        bglEntries[1].binding = 1;
        bglEntries[1].visibility = wgpu::ShaderStage::Fragment;
        bglEntries[1].buffer.type = wgpu::BufferBindingType::Uniform;
        bglEntries[1].buffer.minBindingSize = 4 * sizeof(uint32_t);

        BindGroupLayoutDescriptor bglDesc = {};
        bglDesc.entryCount = bglEntries.size();
        bglDesc.entries = bglEntries.data();

        DAWN_TRY_ASSIGN(bgl, device->CreateBindGroupLayout(&bglDesc));
    }

    InternalPipelineStore::BlitR8ToStencilPipelines pipelines;
    DAWN_TRY_ASSIGN(pipelines, GetOrCreateR8ToStencilPipelines(device, format.format,
                                                               textureViewDimension, bgl.Get()));

    // Build the params buffer, containing the copy dst origin.
    Ref<BufferBase> paramsBuffer;
    {
        BufferDescriptor bufferDesc = {};
        bufferDesc.size = sizeof(uint32_t) * 4;
        bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        bufferDesc.mappedAtCreation = true;
        DAWN_TRY_ASSIGN(paramsBuffer, device->CreateBuffer(&bufferDesc));

        uint32_t* params = static_cast<uint32_t*>(paramsBuffer->GetMappedRange(0, bufferDesc.size));
        params[0] = dst.origin.x;
        params[1] = dst.origin.y;
        params[2] = 0;
        DAWN_TRY(paramsBuffer->Unmap());
    }

    Ref<TextureViewBase> srcView;
    {
        TextureViewDescriptor viewDesc = {};
        viewDesc.dimension = textureViewDimension;
        // Array layer in texture views used in bind groups must consist of the entire array.
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = dataTexture->GetArrayLayers();
        viewDesc.mipLevelCount = 1;
        DAWN_TRY_ASSIGN(srcView, dataTexture->CreateView(&viewDesc));
    }

    // For each layer, blit the stencil data.
    for (uint32_t z = 0; z < copyExtent.depthOrArrayLayers; ++z) {
        if (z >= 1) {
            // Pass the array layer info via the uniform buffer.
            commandEncoder->APIWriteBuffer(paramsBuffer.Get(), sizeof(uint32_t) * 2,
                                           reinterpret_cast<const uint8_t*>(&z), sizeof(uint32_t));
        }

        Ref<TextureViewBase> dstView;
        {
            TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = textureViewDimension;
            viewDesc.baseArrayLayer = dst.origin.z + z;
            viewDesc.arrayLayerCount = 1;
            viewDesc.baseMipLevel = dst.mipLevel;
            viewDesc.mipLevelCount = 1;
            DAWN_TRY_ASSIGN(dstView, dst.texture->CreateView(&viewDesc));
        }

        Ref<BindGroupBase> bindGroup;
        {
            std::array<BindGroupEntry, 2> bgEntries = {};
            bgEntries[0].binding = 0;
            bgEntries[0].textureView = srcView.Get();
            bgEntries[1].binding = 1;
            bgEntries[1].buffer = paramsBuffer.Get();

            BindGroupDescriptor bgDesc = {};
            bgDesc.layout = bgl.Get();
            bgDesc.entryCount = bgEntries.size();
            bgDesc.entries = bgEntries.data();
            DAWN_TRY_ASSIGN(bindGroup,
                            device->CreateBindGroup(&bgDesc, UsageValidationMode::Internal));
        }

        RenderPassDepthStencilAttachment dsAttachment;
        dsAttachment.depthClearValue = 0.0;
        dsAttachment.view = dstView.Get();
        if (format.HasDepth()) {
            dsAttachment.depthLoadOp = wgpu::LoadOp::Load;
            dsAttachment.depthStoreOp = wgpu::StoreOp::Store;
        }
        dsAttachment.stencilLoadOp = wgpu::LoadOp::Load;
        dsAttachment.stencilStoreOp = wgpu::StoreOp::Store;

        RenderPassDescriptor rpDesc = {};
        rpDesc.depthStencilAttachment = &dsAttachment;

        Ref<RenderPassEncoder> pass = commandEncoder->BeginRenderPass(&rpDesc);
        // Bind the resources.
        pass->APISetBindGroup(0, bindGroup.Get());
        // Discard all fragments outside the copy region.
        pass->APISetScissorRect(dst.origin.x, dst.origin.y, copyExtent.width, copyExtent.height);

        // Clear the copy region to 0.
        pass->APISetStencilReference(0);
        pass->APISetPipeline(pipelines.clearPipeline.Get());
        pass->APIDraw(3, 1, 0, 0);

        // Perform 8 draws. Each will load the source stencil data, and will
        // set the bit index in the destination stencil attachment if it the
        // source also has that bit using stencil operation `Replace`.
        // If it doesn't match, the fragment will be discarded.
        pass->APISetStencilReference(255);
        for (uint32_t bit = 0; bit < 8; ++bit) {
            pass->APISetPipeline(pipelines.setStencilPipelines[bit].Get());
            // Draw one instance, and use the stencil value as firstInstance.
            // This is a cheap way to get the stencil value into the shader
            // since WebGPU doesn't have push constants.
            pass->APIDraw(3, 1, 0, 1u << bit);
        }
        pass->End();
    }
    return {};
}

MaybeError BlitStagingBufferToDepth(DeviceBase* device,
                                    BufferBase* buffer,
                                    const TexelCopyBufferLayout& src,
                                    const TextureCopy& dst,
                                    const Extent3D& copyExtent) {
    const Format& format = dst.texture->GetFormat();
    DAWN_ASSERT(format.format == wgpu::TextureFormat::Depth16Unorm);

    TextureDescriptor dataTextureDesc = {};
    dataTextureDesc.format = wgpu::TextureFormat::RG8Uint;
    dataTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    dataTextureDesc.size = copyExtent;

    Ref<TextureBase> dataTexture;
    DAWN_TRY_ASSIGN(dataTexture, device->CreateTexture(&dataTextureDesc));
    {
        TextureCopy rg8Dst;
        rg8Dst.texture = dataTexture.Get();
        rg8Dst.mipLevel = 0;
        rg8Dst.origin = {};
        rg8Dst.aspect = Aspect::Color;
        DAWN_TRY(device->CopyFromStagingToTexture(buffer, src, rg8Dst, copyExtent));
    }

    Ref<CommandEncoderBase> commandEncoder;
    DAWN_TRY_ASSIGN(commandEncoder, device->CreateCommandEncoder());

    DAWN_TRY(
        BlitRG8ToDepth16Unorm(device, commandEncoder.Get(), dataTexture.Get(), dst, copyExtent));

    Ref<CommandBufferBase> commandBuffer;
    DAWN_TRY_ASSIGN(commandBuffer, commandEncoder->Finish());

    CommandBufferBase* commands = commandBuffer.Get();
    device->GetQueue()->APISubmit(1, &commands);
    return {};
}

MaybeError BlitBufferToDepth(DeviceBase* device,
                             CommandEncoder* commandEncoder,
                             BufferBase* buffer,
                             const TexelCopyBufferLayout& src,
                             const TextureCopy& dst,
                             const Extent3D& copyExtent) {
    const Format& format = dst.texture->GetFormat();
    DAWN_ASSERT(format.format == wgpu::TextureFormat::Depth16Unorm);

    TextureDescriptor dataTextureDesc = {};
    dataTextureDesc.format = wgpu::TextureFormat::RG8Uint;
    dataTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    dataTextureDesc.size = copyExtent;

    Ref<TextureBase> dataTexture;
    DAWN_TRY_ASSIGN(dataTexture, device->CreateTexture(&dataTextureDesc));
    {
        TexelCopyBufferInfo bufferSrc;
        bufferSrc.buffer = buffer;
        bufferSrc.layout = src;

        TexelCopyTextureInfo textureDst;
        textureDst.texture = dataTexture.Get();
        commandEncoder->APICopyBufferToTexture(&bufferSrc, &textureDst, &copyExtent);
    }

    DAWN_TRY(BlitRG8ToDepth16Unorm(device, commandEncoder, dataTexture.Get(), dst, copyExtent));
    return {};
}

MaybeError BlitStagingBufferToStencil(DeviceBase* device,
                                      BufferBase* buffer,
                                      const TexelCopyBufferLayout& src,
                                      const TextureCopy& dst,
                                      const Extent3D& copyExtent) {
    TextureDescriptor dataTextureDesc = {};
    dataTextureDesc.format = wgpu::TextureFormat::R8Uint;
    dataTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    dataTextureDesc.size = copyExtent;

    Ref<TextureBase> dataTexture;
    DAWN_TRY_ASSIGN(dataTexture, device->CreateTexture(&dataTextureDesc));
    {
        TextureCopy r8Dst;
        r8Dst.texture = dataTexture.Get();
        r8Dst.mipLevel = 0;
        r8Dst.origin = {};
        r8Dst.aspect = Aspect::Color;
        DAWN_TRY(device->CopyFromStagingToTexture(buffer, src, r8Dst, copyExtent));
    }

    Ref<CommandEncoderBase> commandEncoder;
    DAWN_TRY_ASSIGN(commandEncoder, device->CreateCommandEncoder());

    DAWN_TRY(BlitR8ToStencil(device, commandEncoder.Get(), dataTexture.Get(), dst, copyExtent));

    Ref<CommandBufferBase> commandBuffer;
    DAWN_TRY_ASSIGN(commandBuffer, commandEncoder->Finish());

    CommandBufferBase* commands = commandBuffer.Get();
    device->GetQueue()->APISubmit(1, &commands);
    return {};
}

MaybeError BlitBufferToStencil(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               BufferBase* buffer,
                               const TexelCopyBufferLayout& src,
                               const TextureCopy& dst,
                               const Extent3D& copyExtent) {
    TextureDescriptor dataTextureDesc = {};
    dataTextureDesc.format = wgpu::TextureFormat::R8Uint;
    dataTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    dataTextureDesc.size = copyExtent;

    Ref<TextureBase> dataTexture;
    DAWN_TRY_ASSIGN(dataTexture, device->CreateTexture(&dataTextureDesc));
    {
        TexelCopyBufferInfo bufferSrc;
        bufferSrc.buffer = buffer;
        bufferSrc.layout = src;

        TexelCopyTextureInfo textureDst;
        textureDst.texture = dataTexture.Get();
        commandEncoder->APICopyBufferToTexture(&bufferSrc, &textureDst, &copyExtent);
    }

    DAWN_TRY(BlitR8ToStencil(device, commandEncoder, dataTexture.Get(), dst, copyExtent));
    return {};
}

}  // namespace dawn::native
