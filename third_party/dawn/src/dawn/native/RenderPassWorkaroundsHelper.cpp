// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/RenderPassWorkaroundsHelper.h"

#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/BlitColorToColorWithDraw.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

namespace {
// Tracks the temporary resolve attachments used when the AlwaysResolveIntoZeroLevelAndLayer toggle
// is active so that the results can be copied from the temporary resolve attachment into the
// intended target after the render pass is complete. Also used by the
// ResolveMultipleAttachmentInSeparatePasses toggle to track resolves that need to be done in their
// own passes.
struct TemporaryResolveAttachment {
    TemporaryResolveAttachment(Ref<TextureViewBase> src,
                               Ref<TextureViewBase> dst,
                               wgpu::StoreOp storeOp = wgpu::StoreOp::Store)
        : copySrc(std::move(src)), copyDst(std::move(dst)), storeOp(storeOp) {}

    Ref<TextureViewBase> copySrc;
    Ref<TextureViewBase> copyDst;
    wgpu::StoreOp storeOp;
};

void CopyTextureView(CommandEncoder* encoder, TextureViewBase* src, TextureViewBase* dst) {
    TexelCopyTextureInfo srcTexelCopyTextureInfo = {};
    srcTexelCopyTextureInfo.texture = src->GetTexture();
    srcTexelCopyTextureInfo.aspect = wgpu::TextureAspect::All;
    srcTexelCopyTextureInfo.mipLevel = src->GetBaseMipLevel();
    srcTexelCopyTextureInfo.origin = {0, 0, src->GetBaseArrayLayer()};

    TexelCopyTextureInfo dstTexelCopyTextureInfo = {};
    dstTexelCopyTextureInfo.texture = dst->GetTexture();
    dstTexelCopyTextureInfo.aspect = wgpu::TextureAspect::All;
    dstTexelCopyTextureInfo.mipLevel = dst->GetBaseMipLevel();
    dstTexelCopyTextureInfo.origin = {0, 0, dst->GetBaseArrayLayer()};

    Extent3D extent3D = src->GetSingleSubresourceVirtualSize();

    auto internalUsageScope = encoder->MakeInternalUsageScope();
    encoder->APICopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);
}

void ResolveWithRenderPass(CommandEncoder* encoder,
                           TextureViewBase* src,
                           TextureViewBase* dst,
                           wgpu::StoreOp storeOp) {
    RenderPassColorAttachment attachment = {};
    attachment.view = src;
    attachment.resolveTarget = dst;
    attachment.loadOp = wgpu::LoadOp::Load;
    attachment.storeOp = storeOp;

    RenderPassDescriptor resolvePass = {};
    resolvePass.colorAttachmentCount = 1;
    resolvePass.colorAttachments = &attachment;

    // Begin and end an empty render pass to force the resolve.
    Ref<RenderPassEncoder> rpEncoder = encoder->BeginRenderPass(&resolvePass);
    rpEncoder->End();
}

void DiscardWithRenderPass(CommandEncoder* encoder, TextureViewBase* view) {
    ResolveWithRenderPass(encoder, view, nullptr, wgpu::StoreOp::Discard);
}

}  // namespace

RenderPassWorkaroundsHelper::RenderPassWorkaroundsHelper() = default;
RenderPassWorkaroundsHelper::~RenderPassWorkaroundsHelper() = default;

MaybeError RenderPassWorkaroundsHelper::Initialize(
    CommandEncoder* encoder,
    const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor) {
    DeviceBase* device = encoder->GetDevice();

    // dawn:56, dawn:1569
    // Handle Toggle AlwaysResolveIntoZeroLevelAndLayer. This swaps out the given resolve attachment
    // for a temporary one that has no layers or mip levels. The results are copied from the
    // temporary attachment into the given attachment when the render pass ends. (Handled in
    // Apply())
    if (device->IsToggleEnabled(Toggle::AlwaysResolveIntoZeroLevelAndLayer)) {
        for (uint8_t i = 0; i < renderPassDescriptor->colorAttachmentCount; ++i) {
            ColorAttachmentIndex colorIdx(i);
            const auto& colorAttachment = renderPassDescriptor->colorAttachments[i];
            TextureViewBase* resolveTarget = colorAttachment.resolveTarget;

            if (resolveTarget != nullptr && (resolveTarget->GetBaseMipLevel() != 0 ||
                                             resolveTarget->GetBaseArrayLayer() != 0)) {
                DAWN_ASSERT(colorAttachment.view);
                // Create a temporary texture to resolve into
                // TODO(dawn:1618): Defer allocation of temporary textures till submit time.
                TextureDescriptor descriptor = {};
                descriptor.usage = wgpu::TextureUsage::RenderAttachment |
                                   wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                   wgpu::TextureUsage::TextureBinding;
                descriptor.format = resolveTarget->GetFormat().format;
                descriptor.size = resolveTarget->GetSingleSubresourceVirtualSize();
                descriptor.dimension = wgpu::TextureDimension::e2D;
                descriptor.mipLevelCount = 1;

                // We are creating new resources. Device must already be locked via
                // APIBeginRenderPass.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                Ref<TextureBase> temporaryResolveTexture;
                Ref<TextureViewBase> temporaryResolveView;
                {
                    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

                    DAWN_TRY_ASSIGN(temporaryResolveTexture, device->CreateTexture(&descriptor));

                    TextureViewDescriptor viewDescriptor = {};
                    DAWN_TRY_ASSIGN(
                        temporaryResolveView,
                        device->CreateTextureView(temporaryResolveTexture.Get(), &viewDescriptor));

                    if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
                        // Since we want to load the original resolve target, we need to copy it to
                        // the temp resolve target.
                        CopyTextureView(encoder, resolveTarget, temporaryResolveView.Get());
                    }
                }

                mTempResolveTargets[colorIdx] = {std::move(temporaryResolveTexture),
                                                 std::move(temporaryResolveView)};

                mTempResolveTargetsMask.set(colorIdx);
            }
        }
    }

    return {};
}

MaybeError RenderPassWorkaroundsHelper::ApplyOnPostEncoding(
    CommandEncoder* encoder,
    const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor,
    RenderPassResourceUsageTracker* usageTracker,
    BeginRenderPassCmd* cmd,
    RenderPassEncoder::EndCallback* passEndCallbackOut) {
    auto device = encoder->GetDevice();

    // List of operations to perform on render pass end.
    absl::InlinedVector<RenderPassEncoder::EndCallback, 1> passEndOperations;

    // dawn:56, dawn:1569
    // swap the resolve targets for the temp resolve textures.
    if (mTempResolveTargetsMask.any()) {
        std::vector<TemporaryResolveAttachment> temporaryResolveAttachments;

        for (auto index : mTempResolveTargetsMask) {
            TextureViewBase* resolveTarget = cmd->colorAttachments[index].resolveTarget.Get();
            TextureViewBase* temporaryResolveView = mTempResolveTargets[index].view.Get();

            DAWN_ASSERT(resolveTarget);

            // Save the temporary and given render targets together for copying after
            // the render pass ends.
            temporaryResolveAttachments.emplace_back(temporaryResolveView, resolveTarget);

            // Replace the given resolve attachment with the temporary one.
            usageTracker->TextureViewUsedAs(temporaryResolveView,
                                            wgpu::TextureUsage::RenderAttachment);
            cmd->colorAttachments[index].resolveTarget = temporaryResolveView;
        }

        passEndOperations.emplace_back([encoder, temporaryResolveAttachments = std::move(
                                                     temporaryResolveAttachments)]() -> MaybeError {
            // Called once the render pass has been ended.
            // Handle any copies needed for the AlwaysResolveIntoZeroLevelAndLayer
            // workaround immediately after the render pass ends and before any additional
            // commands are recorded.
            for (auto& copyTarget : temporaryResolveAttachments) {
                CopyTextureView(encoder, copyTarget.copySrc.Get(), copyTarget.copyDst.Get());
            }
            return {};
        });
    }

    mShouldApplyExpandResolveEmulation =
        cmd->attachmentState->GetExpandResolveInfo().attachmentsToExpandResolve.any() &&
        device->CanTextureLoadResolveTargetInTheSameRenderpass();

    // Handle partial resolve. This identifies passes where there are MSAA color attachments with
    // wgpu::LoadOp::ExpandResolveTexture. If that's the case then the resolves are deferred by
    // removing the resolve targets and forcing the storeOp to Store. After the pass has ended an
    // new pass is recorded for each resolve target that resolves it separately.
    if (const auto* expandResolveRect =
            renderPassDescriptor.Get<RenderPassDescriptorResolveRect>()) {
        if (device->CanResolveSubRect()) {
            // When CanResolveSubRect is true, the resolve parameters are passed through 'cmd' to
            // execute the appropriate resolve operation.
            cmd->resolveRect = {.colorOffsetX = expandResolveRect->colorOffsetX,
                                .colorOffsetY = expandResolveRect->colorOffsetY,
                                .resolveOffsetX = expandResolveRect->resolveOffsetX,
                                .resolveOffsetY = expandResolveRect->resolveOffsetY,
                                .updateWidth = expandResolveRect->width,
                                .updateHeight = expandResolveRect->height};
        } else {
            std::vector<TemporaryResolveAttachment> temporaryResolveAttachments;

            for (auto i : cmd->attachmentState->GetColorAttachmentsMask()) {
                auto& attachmentInfo = cmd->colorAttachments[i];
                TextureViewBase* resolveTarget = attachmentInfo.resolveTarget.Get();
                if (!resolveTarget) {
                    continue;
                }
                // Save the color and resolve targets together for an explicit resolve pass
                // after this one ends, then remove the resolve target from this pass and
                // force the storeOp to Store.
                temporaryResolveAttachments.emplace_back(attachmentInfo.view.Get(), resolveTarget,
                                                         attachmentInfo.storeOp);
                attachmentInfo.storeOp = wgpu::StoreOp::Store;
                attachmentInfo.resolveTarget = nullptr;
            }
            for (auto& deferredResolve : temporaryResolveAttachments) {
                passEndOperations.emplace_back([device, encoder, resolveRect = *expandResolveRect,
                                                deferredResolve]() -> MaybeError {
                    // Do partial resolve first in one render pass.
                    DAWN_TRY(ResolveMultisampleWithDraw(device, encoder, resolveRect,
                                                        deferredResolve.copySrc.Get(),
                                                        deferredResolve.copyDst.Get()));

                    switch (deferredResolve.storeOp) {
                        case wgpu::StoreOp::Store:
                            // 'Store' has been handled in the main render pass already.
                            break;
                        case wgpu::StoreOp::Discard:
                            // Handle 'Discard', tagging the subresource as uninitialized.
                            DiscardWithRenderPass(encoder, deferredResolve.copySrc.Get());
                            break;
                        default:
                            DAWN_UNREACHABLE();
                    }
                    return {};
                });
            }
        }
    }

    // dawn:1550
    // Handle toggle ResolveMultipleAttachmentInSeparatePasses. This identifies passes where there
    // are multiple MSAA color targets and at least one of them has a resolve target. If that's the
    // case then the resolves are deferred by removing the resolve targets and forcing the storeOp
    // to Store. After the pass has ended an new pass is recorded for each resolve target that
    // resolves it separately.
    if (device->IsToggleEnabled(Toggle::ResolveMultipleAttachmentInSeparatePasses) &&
        cmd->attachmentState->GetColorAttachmentsMask().count() > 1) {
        bool splitResolvesIntoSeparatePasses = false;

        // This workaround needs to apply if there are multiple MSAA color targets (checked above)
        // and at least one resolve target.
        for (auto i : cmd->attachmentState->GetColorAttachmentsMask()) {
            if (cmd->colorAttachments[i].resolveTarget.Get() != nullptr) {
                splitResolvesIntoSeparatePasses = true;
                break;
            }
        }

        if (splitResolvesIntoSeparatePasses) {
            std::vector<TemporaryResolveAttachment> temporaryResolveAttachments;

            for (auto i : cmd->attachmentState->GetColorAttachmentsMask()) {
                auto& attachmentInfo = cmd->colorAttachments[i];
                TextureViewBase* resolveTarget = attachmentInfo.resolveTarget.Get();
                if (resolveTarget != nullptr) {
                    // Save the color and resolve targets together for an explicit resolve pass
                    // after this one ends, then remove the resolve target from this pass and
                    // force the storeOp to Store.
                    temporaryResolveAttachments.emplace_back(attachmentInfo.view.Get(),
                                                             resolveTarget, attachmentInfo.storeOp);
                    attachmentInfo.storeOp = wgpu::StoreOp::Store;
                    attachmentInfo.resolveTarget = nullptr;
                }
            }

            passEndOperations.emplace_back(
                [encoder, temporaryResolveAttachments =
                              std::move(temporaryResolveAttachments)]() -> MaybeError {
                    // Called once the render pass has been ended.
                    // Handles any separate resolve passes needed for the
                    // ResolveMultipleAttachmentInSeparatePasses workaround immediately after the
                    // render pass ends and before any additional commands are recorded.
                    for (auto& deferredResolve : temporaryResolveAttachments) {
                        ResolveWithRenderPass(encoder, deferredResolve.copySrc.Get(),
                                              deferredResolve.copyDst.Get(),
                                              deferredResolve.storeOp);
                    }
                    return {};
                });
        }
    }

    *passEndCallbackOut = [passEndOperations = std::move(passEndOperations)]() -> MaybeError {
        // Apply the operations in reverse order. Typically they can be:
        //   1.a) Full resolve pass.
        //   1.b) Partial resolve pass, and conditionally followed with a MSAA texture Discard pass.
        //   2) Copy from temp resolve to the actual resolve target.
        for (auto opIte = passEndOperations.rbegin(); opIte != passEndOperations.rend(); ++opIte) {
            DAWN_TRY((*opIte)());
        }
        return {};
    };

    return {};
}

MaybeError RenderPassWorkaroundsHelper::ApplyOnRenderPassStart(
    RenderPassEncoder* rpEncoder,
    const UnpackedPtr<RenderPassDescriptor>& rpDesc) {
    DeviceBase* device = rpEncoder->GetDevice();
    if (mShouldApplyExpandResolveEmulation) {
        // Perform ExpandResolveTexture load op's emulation after the render pass starts.
        // Backend that doesn't support CanTextureLoadResolveTargetInTheSameRenderpass() can
        // implement this load op internally.
        DAWN_TRY(ExpandResolveTextureWithDraw(device, rpEncoder, rpDesc));
    }

    return {};
}

}  // namespace dawn::native
