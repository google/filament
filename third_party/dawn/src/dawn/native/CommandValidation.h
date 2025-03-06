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

#ifndef SRC_DAWN_NATIVE_COMMANDVALIDATION_H_
#define SRC_DAWN_NATIVE_COMMANDVALIDATION_H_

#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Constants.h"
#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Texture.h"
#include "dawn/native/UsageValidationMode.h"

namespace dawn::native {

enum class BufferSizeType { Size, AllocatedSize };

class QuerySetBase;
struct SyncScopeResourceUsage;
struct TexelBlockInfo;

MaybeError ValidateSyncScopeResourceUsage(const SyncScopeResourceUsage& usage);

MaybeError ValidateTimestampQuery(const DeviceBase* device,
                                  const QuerySetBase* querySet,
                                  uint32_t queryIndex,
                                  Feature requiredFeature = Feature::TimestampQuery);

MaybeError ValidatePassTimestampWrites(const DeviceBase* device,
                                       const PassTimestampWrites* timestampWrites);

MaybeError ValidateWriteBuffer(const DeviceBase* device,
                               const BufferBase* buffer,
                               uint64_t bufferOffset,
                               uint64_t size);

template <typename A, typename B>
DAWN_FORCE_INLINE uint64_t Safe32x32(A a, B b) {
    static_assert(std::is_same<A, uint32_t>::value, "'a' must be uint32_t");
    static_assert(std::is_same<B, uint32_t>::value, "'b' must be uint32_t");
    return uint64_t(a) * uint64_t(b);
}

ResultOrError<uint64_t> ComputeRequiredBytesInCopy(const TexelBlockInfo& blockInfo,
                                                   const Extent3D& copySize,
                                                   uint32_t bytesPerRow,
                                                   uint32_t rowsPerImage);

void ApplyDefaultTexelCopyBufferLayoutOptions(TexelCopyBufferLayout* layout,
                                              const TexelBlockInfo& blockInfo,
                                              const Extent3D& copyExtent);
MaybeError ValidateLinearTextureData(const TexelCopyBufferLayout& layout,
                                     uint64_t byteSize,
                                     const TexelBlockInfo& blockInfo,
                                     const Extent3D& copyExtent);
MaybeError ValidateTextureCopyRange(DeviceBase const* device,
                                    const TexelCopyTextureInfo& TexelCopyTextureInfo,
                                    const Extent3D& copySize);
ResultOrError<Aspect> SingleAspectUsedByTexelCopyTextureInfo(const TexelCopyTextureInfo& view);
MaybeError ValidateLinearToDepthStencilCopyRestrictions(const TexelCopyTextureInfo& dst);

MaybeError ValidateTexelCopyBufferInfo(DeviceBase const* device,
                                       const TexelCopyBufferInfo& texelCopyBufferInfo);
MaybeError ValidateTexelCopyTextureInfo(DeviceBase const* device,
                                        const TexelCopyTextureInfo& TexelCopyTextureInfo,
                                        const Extent3D& copySize);

MaybeError ValidateCopySizeFitsInBuffer(const Ref<BufferBase>& buffer,
                                        uint64_t offset,
                                        uint64_t size,
                                        BufferSizeType checkBufferSizeType = BufferSizeType::Size);

bool IsRangeOverlapped(uint32_t startA, uint32_t startB, uint32_t length);

MaybeError ValidateTextureToTextureCopyCommonRestrictions(DeviceBase const* device,
                                                          const TexelCopyTextureInfo& src,
                                                          const TexelCopyTextureInfo& dst,
                                                          const Extent3D& copySize);
MaybeError ValidateTextureToTextureCopyRestrictions(DeviceBase const* device,
                                                    const TexelCopyTextureInfo& src,
                                                    const TexelCopyTextureInfo& dst,
                                                    const Extent3D& copySize);

MaybeError ValidateCanUseAs(const TextureBase* textureView,
                            wgpu::TextureUsage usage,
                            UsageValidationMode mode);
MaybeError ValidateCanUseAs(const TextureViewBase* textureView,
                            wgpu::TextureUsage usage,
                            UsageValidationMode mode);
MaybeError ValidateCanUseAs(const BufferBase* buffer, wgpu::BufferUsage usage);
MaybeError ValidateCanUseAsInternal(const BufferBase* buffer, wgpu::BufferUsage usage);

using ColorAttachmentFormats = absl::InlinedVector<const Format*, kMaxColorAttachments>;
MaybeError ValidateColorAttachmentBytesPerSample(DeviceBase* device,
                                                 const ColorAttachmentFormats& formats);

struct StorageAttachmentInfoForValidation {
    uint64_t offset;
    // This format is assumed to support StorageAttachment.
    wgpu::TextureFormat format;
};
MaybeError ValidatePLSInfo(
    const DeviceBase* device,
    uint64_t totalSize,
    ityp::span<size_t, StorageAttachmentInfoForValidation> storageAttachments);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDVALIDATION_H_
