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

#include "WebGPUMsaaTextureResolver.h"

#include "WebGPUTexture.h"

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

namespace filament::backend {

namespace {

void resolveColorTextures(wgpu::CommandEncoder const& commandEncoder,
        wgpu::TextureView const& sourceTextureView,
        wgpu::TextureView const& destinationTextureView) {
    const wgpu::RenderPassColorAttachment colorAttachment{
        .view = sourceTextureView,
        .depthSlice = wgpu::kDepthSliceUndefined, // being explicit for consistent behavior
        .resolveTarget = destinationTextureView,
        .loadOp = wgpu::LoadOp::Load,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = {}, // being explicit for consistent behavior
    };
    const wgpu::RenderPassDescriptor renderPassDescriptor{
        .label = "resolve_render_pass",
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment,
        .depthStencilAttachment = nullptr, // being explicit for consistent behavior
        .occlusionQuerySet = nullptr,      // being explicit for consistent behavior
        .timestampWrites = nullptr,        // being explicit for consistent behavior
    };
    const wgpu::RenderPassEncoder renderPassEncoder{ commandEncoder.BeginRenderPass(
            &renderPassDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(renderPassEncoder)
            << "Failed to create wgpu::RenderPassEncoder for WebGPUDriver::resolve";
    renderPassEncoder.End(); // only the implicit resolve is happening in the pass
}

} // namespace

void WebGPUMsaaTextureResolver::resolve(ResolveRequest const& request) {
    ResolveRequest::TextureInfo const& source{ request.source };
    ResolveRequest::TextureInfo const& destination{ request.destination };
    FILAMENT_CHECK_PRECONDITION(destination.texture.GetWidth() == source.texture.GetWidth() &&
                                destination.texture.GetHeight() == source.texture.GetHeight())
            << "invalid resolve: source and destination sizes don't match";

    FILAMENT_CHECK_PRECONDITION(
            source.texture.GetSampleCount() > 1 && destination.texture.GetSampleCount() == 1)
            << "invalid resolve: source.samples=" << source.texture.GetSampleCount()
            << ", destination.samples=" << destination.texture.GetSampleCount();

    FILAMENT_CHECK_PRECONDITION(source.texture.GetFormat() == destination.texture.GetFormat())
            << "source and destination texture format don't match";
    const wgpu::TextureFormat format{ source.texture.GetFormat() };

    FILAMENT_CHECK_PRECONDITION(!hasDepth(format)) << "can't resolve depth formats";

    FILAMENT_CHECK_PRECONDITION(!hasStencil(format)) << "can't resolve stencil formats";

    FILAMENT_CHECK_PRECONDITION(source.texture.GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "source texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    FILAMENT_CHECK_PRECONDITION(
            destination.texture.GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    FILAMENT_CHECK_PRECONDITION(destination.texture.GetUsage() & wgpu::TextureUsage::TextureBinding)
            << "destination texture usage doesn't have wgpu::TextureUsage::TextureBinding";

    const wgpu::TextureViewDescriptor sourceTextureViewDescriptor{
        .label = "resolve_source_texture_view",
        .format = request.viewFormat,
        .dimension = source.viewDimension,
        .baseMipLevel = source.mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = source.layer,
        .arrayLayerCount = 1,
        .aspect = source.aspect,
        .usage = source.texture.GetUsage(),
    };
    const wgpu::TextureView sourceTextureView{ source.texture.CreateView(
            &sourceTextureViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(sourceTextureView)
            << "Failed to create wgpu::TextureView sourceTextureView";
    const wgpu::TextureViewDescriptor destinationTextureViewDescriptor{
        .label = "resolve_destination_texture_view",
        .format = request.viewFormat,
        .dimension = destination.viewDimension,
        .baseMipLevel = destination.mipLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = destination.layer,
        .arrayLayerCount = 1,
        .aspect = destination.aspect,
        .usage = destination.texture.GetUsage(),
    };
    const wgpu::TextureView destinationTextureView{ destination.texture.CreateView(
            &destinationTextureViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(destinationTextureView)
            << "Failed to create wgpu::TextureView destinationTextureView.";

    if (hasDepth(format)) {
        PANIC_PRECONDITION("DEPTH RESOLVE NOT IMPLEMENTED YET");
    } else {
        resolveColorTextures(request.commandEncoder, sourceTextureView, destinationTextureView);
    }
}

} // namespace filament::backend
