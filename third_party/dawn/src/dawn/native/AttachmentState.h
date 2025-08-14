// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_
#define SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_

#include <array>
#include <bitset>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/IntegerTypes.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

class AttachmentState final : public RefCounted,
                              public CachedObject,
                              public ContentLessObjectCacheable<AttachmentState> {
  public:
    struct ExpandResolveInfo {
        // Mask indicates which attachments use `ExpandResolveTexture` load op.
        ColorAttachmentMask attachmentsToExpandResolve;
        // Mask indicates which attachments have resolve target. Only enabled if
        // attachmentsToExpandResolve has any bit set.
        ColorAttachmentMask resolveTargetsMask;
    };

    // Note: Descriptors must be validated before the AttachmentState is constructed.
    explicit AttachmentState(const RenderBundleEncoderDescriptor* descriptor);
    explicit AttachmentState(const UnpackedPtr<RenderPipelineDescriptor>& descriptor,
                             const PipelineLayoutBase* layout);
    explicit AttachmentState(const UnpackedPtr<RenderPassDescriptor>& descriptor);

    // Constructor used to avoid re-parsing descriptors when we already parsed them for cache keys.
    AttachmentState(const AttachmentState& blueprint);

    ColorAttachmentMask GetColorAttachmentsMask() const;
    wgpu::TextureFormat GetColorAttachmentFormat(ColorAttachmentIndex index) const;
    bool HasDepthStencilAttachment() const;
    wgpu::TextureFormat GetDepthStencilFormat() const;
    uint32_t GetSampleCount() const;
    const ExpandResolveInfo& GetExpandResolveInfo() const;
    bool HasPixelLocalStorage() const;
    const std::vector<wgpu::TextureFormat>& GetStorageAttachmentSlots() const;
    std::vector<ColorAttachmentIndex> ComputeStorageAttachmentPackingInColorAttachments() const;

    struct EqualityFunc {
        bool operator()(const AttachmentState* a, const AttachmentState* b) const;
    };
    size_t ComputeContentHash() override;

  protected:
    void DeleteThis() override;

  private:
    ColorAttachmentMask mColorAttachmentsSet;
    PerColorAttachment<wgpu::TextureFormat> mColorFormats;
    // Default (texture format Undefined) indicates there is no depth stencil attachment.
    wgpu::TextureFormat mDepthStencilFormat = wgpu::TextureFormat::Undefined;
    uint32_t mSampleCount = 0;

    ExpandResolveInfo mExpandResolveInfo;

    bool mHasPLS = false;
    std::vector<wgpu::TextureFormat> mStorageAttachmentSlots;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_
