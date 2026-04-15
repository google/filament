// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dawn/replay/BlitBufferToDepthTexture.h"

#include "dawn/common/Strings.h"

namespace dawn::replay {

namespace {

constexpr std::string_view kShaderSrc = DAWN_MULTILINE(
    const kPixelSize = 4;

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
    srcOffset : u32,
    bytesPerRow : u32,
    dstOrigin : vec2u
    };

    @group(0) @binding(0) var<storage, read> src_buf : array<f32>;
    @group(0) @binding(1) var<uniform> params : Params;

    @fragment fn blit_buffer_to_texture(
        @builtin(position) screen_position : vec4f
    ) -> @builtin(frag_depth) f32 {
        let iposition = vec2u(screen_position.xy) - params.dstOrigin;

        let srcOffset = params.srcOffset + iposition.x * kPixelSize + iposition.y * params.bytesPerRow;

        return src_buf[srcOffset >> 2];
    }
);

ResultOrError<wgpu::RenderPipeline> CreatePipeline(wgpu::Device device,
                                                   wgpu::TextureFormat format) {
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    wgslDesc.code = kShaderSrc;
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDesc);

    wgpu::FragmentState fragmentState = {};
    fragmentState.module = shaderModule;

    wgpu::DepthStencilState depthStencilState = {};
    depthStencilState.format = format;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = wgpu::CompareFunction::Always;

    wgpu::RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "blit_buffer_to_texture";
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.fragment = &fragmentState;
    renderPipelineDesc.depthStencil = &depthStencilState;

    return device.CreateRenderPipeline(&renderPipelineDesc);
}

}  // namespace

MaybeError BlitBufferToDepthTexture::Blit(wgpu::Device device,
                                          const wgpu::TexelCopyTextureInfo& dst,
                                          void const* data,
                                          size_t dataSize,
                                          const wgpu::TexelCopyBufferLayout& src,
                                          const wgpu::Extent3D& copyExtent) {
    // This function assumes bytesPerRow is multiples of 4. Normally it's required that
    // bytesPerRow is aligned to 256. However some backends might enable
    // DawnTexelCopyBufferRowAlignment feature to relax the alignment. Currently only D3D11 backend
    // enables this feature, and the relaxed alignment there is 4.
    DAWN_ASSERT((src.bytesPerRow % 4) == 0);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.label = "Init Texture Data Buffer";
    bufferDesc.size = dataSize;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    device.GetQueue().WriteBuffer(buffer, 0, data, dataSize);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPipeline pipeline;
    auto it = mPipelines.find(dst.texture.GetFormat());
    if (it != mPipelines.end()) {
        pipeline = it->second;
    } else {
        DAWN_TRY_ASSIGN(pipeline, CreatePipeline(device, dst.texture.GetFormat()));
        mPipelines[dst.texture.GetFormat()] = pipeline;
    }

    wgpu::BindGroupLayout bgl = pipeline.GetBindGroupLayout(0);

    wgpu::TextureViewDimension viewDimension;
    uint32_t baseDepth = 0;
    uint32_t baseArray = 0;
    uint32_t depthStep = 0;
    uint32_t arrayStep = 0;
    switch (dst.texture.GetDimension()) {
        case wgpu::TextureDimension::e1D:
            DAWN_UNREACHABLE();
            break;
        case wgpu::TextureDimension::e3D:
            viewDimension = wgpu::TextureViewDimension::e3D;
            baseDepth = static_cast<uint32_t>(dst.origin.z);
            depthStep = 1;
            break;
        default:
            viewDimension = wgpu::TextureViewDimension::e2D;
            baseArray = static_cast<uint32_t>(dst.origin.z);
            arrayStep = 1;
            break;
    }

    for (uint32_t z{0}; z < copyExtent.depthOrArrayLayers; ++z) {
        wgpu::TextureView dstView;
        {
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.dimension = viewDimension;
            viewDesc.baseArrayLayer = baseArray + arrayStep * static_cast<uint32_t>(z);
            viewDesc.arrayLayerCount = 1;
            viewDesc.baseMipLevel = dst.mipLevel;
            viewDesc.mipLevelCount = 1;
            dstView = dst.texture.CreateView(&viewDesc);
        }

        const uint64_t srcOffset = src.offset + z * src.rowsPerImage * src.bytesPerRow;
        const uint32_t shaderReadOffset = uint32_t(srcOffset);
        wgpu::Buffer paramsBuffer;
        {
            wgpu::BufferDescriptor desc;
            desc.label = "Init Texture Param Buffer";
            desc.size = sizeof(uint32_t) * 4;
            desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
            paramsBuffer = device.CreateBuffer(&desc);

            uint32_t params[4];
            params[0] = shaderReadOffset;
            params[1] = src.bytesPerRow;
            params[2] = dst.origin.x;
            params[3] = dst.origin.y;
            encoder.WriteBuffer(paramsBuffer.Get(), 0, reinterpret_cast<const uint8_t*>(&params[0]),
                                sizeof(params));
        }

        wgpu::BindGroupEntry bgEntries[2] = {};
        bgEntries[0].binding = 0;
        bgEntries[0].buffer = buffer;
        bgEntries[1].binding = 1;
        bgEntries[1].buffer = paramsBuffer;

        wgpu::BindGroupDescriptor bgDesc = {};
        bgDesc.layout = bgl;
        bgDesc.entryCount = 2;
        bgDesc.entries = &bgEntries[0];

        wgpu::BindGroup bindGroup = device.CreateBindGroup(&bgDesc);

        wgpu::RenderPassColorAttachment colorAttachment;
        colorAttachment.view = dstView;
        if (depthStep) {
            colorAttachment.depthSlice = baseDepth + depthStep * z;
        }
        colorAttachment.loadOp = wgpu::LoadOp::Load;
        colorAttachment.storeOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor rpDesc = {};
        rpDesc.colorAttachmentCount = 0;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
        depthStencilAttachment.view = dstView;
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;

        rpDesc.depthStencilAttachment = &depthStencilAttachment;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        // Bind the resources.
        pass.SetBindGroup(0, bindGroup);
        pass.SetViewport(static_cast<float>(dst.origin.x), static_cast<float>(dst.origin.y),
                         static_cast<float>(copyExtent.width),
                         static_cast<float>(copyExtent.height), 0.f, 1.f);

        // Draw to perform the blit.
        pass.SetPipeline(pipeline);
        pass.Draw(3, 1, 0, 0);
        pass.End();
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);

    buffer.Destroy();

    return {};
}

}  // namespace dawn::replay
