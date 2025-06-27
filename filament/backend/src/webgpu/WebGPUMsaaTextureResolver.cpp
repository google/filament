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

#include <cstdint>

namespace filament::backend {

namespace {

void resolveColorTextures(wgpu::CommandEncoder const& commandEncoder,
        const wgpu::TextureView sourceTextureView, const wgpu::TextureView destinationTextureView) {
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

WebGPUMsaaTextureResolver::WebGPUMsaaTextureResolver(wgpu::Device const& device)
    : mDevice{ device } {}

void WebGPUMsaaTextureResolver::resolve(wgpu::CommandEncoder const& commandEncoder,
        WebGPUTexture const& sourceTexture, const uint8_t sourceLevel, const uint8_t sourceLayer,
        WebGPUTexture const& destinationTexture, const uint8_t destinationLevel,
        const uint8_t destinationLayer) {

    FILAMENT_CHECK_PRECONDITION(destinationTexture.width == sourceTexture.width &&
                                destinationTexture.height == sourceTexture.height)
            << "invalid resolve: source and destination sizes don't match";

    FILAMENT_CHECK_PRECONDITION(sourceTexture.samples > 1 && destinationTexture.samples == 1)
            << "invalid resolve: source.samples=" << +sourceTexture.samples
            << ", destination.samples=" << +destinationTexture.samples;

    FILAMENT_CHECK_PRECONDITION(sourceTexture.format == destinationTexture.format)
            << "source and destination texture format don't match";

    FILAMENT_CHECK_PRECONDITION(!isStencilFormat(sourceTexture.format))
            << "can't resolve stencil formats";

    FILAMENT_CHECK_PRECONDITION(any(destinationTexture.usage & TextureUsage::BLIT_DST))
            << "destination texture doesn't have BLIT_DST";

    FILAMENT_CHECK_PRECONDITION(any(sourceTexture.usage & TextureUsage::BLIT_SRC))
            << "source texture doesn't have BLIT_SRC";

    FILAMENT_CHECK_PRECONDITION(
            sourceTexture.getTexture().GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "source texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    FILAMENT_CHECK_PRECONDITION(
            destinationTexture.getTexture().GetUsage() & wgpu::TextureUsage::RenderAttachment)
            << "destination texture usage doesn't have wgpu::TextureUsage::RenderAttachment";

    FILAMENT_CHECK_PRECONDITION(
            destinationTexture.getTexture().GetUsage() & wgpu::TextureUsage::TextureBinding)
            << "destination texture usage doesn't have wgpu::TextureUsage::TextureBinding";

    const wgpu::TextureFormat format{ sourceTexture.getViewFormat() };
    const wgpu::TextureViewDescriptor sourceTextureViewDescriptor{
        .label = "resolve_source_texture_view",
        .format = format,
        .dimension = sourceTexture.getViewDimension(),
        .baseMipLevel = sourceLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = sourceLayer,
        .arrayLayerCount = 1,
        .aspect = sourceTexture.getAspect(),
        .usage = sourceTexture.getTexture().GetUsage(),
    };
    const wgpu::TextureView sourceTextureView{ sourceTexture.getTexture().CreateView(
            &sourceTextureViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(sourceTextureView) << "Failed to create wgpu::TextureView sourceTextureView";
    const wgpu::TextureViewDescriptor destinationTextureViewDescriptor{
        .label = "resolve_destination_texture_view",
        .format = format,
        .dimension = destinationTexture.getViewDimension(),
        .baseMipLevel = destinationLevel,
        .mipLevelCount = 1,
        .baseArrayLayer = destinationLayer,
        .arrayLayerCount = 1,
        .aspect = destinationTexture.getAspect(),
        .usage = destinationTexture.getTexture().GetUsage(),
    };
    const wgpu::TextureView destinationTextureView{ destinationTexture.getTexture().CreateView(
            &destinationTextureViewDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(destinationTextureView) << "Failed to create wgpu::TextureView destinationTextureView.";

    if (isDepthFormat(sourceTexture.format)) {
        PANIC_PRECONDITION("DEPTH RESOLVE NOT IMPLEMENTED YET");
    } else {
        resolveColorTextures(commandEncoder, sourceTextureView, destinationTextureView);
    }
}

} // namespace filament::backend
