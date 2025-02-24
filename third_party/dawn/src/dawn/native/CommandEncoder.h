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

#ifndef SRC_DAWN_NATIVE_COMMANDENCODER_H_
#define SRC_DAWN_NATIVE_COMMANDENCODER_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/StackAllocated.h"
#include "dawn/native/EncodingContext.h"
#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

enum class UsageValidationMode;

Color ClampClearColorValueToLegalRange(const Color& originalColor, const Format& format);

ResultOrError<UnpackedPtr<CommandEncoderDescriptor>> ValidateCommandEncoderDescriptor(
    const DeviceBase* device,
    const CommandEncoderDescriptor* descriptor);

class CommandEncoder final : public ApiObjectBase {
  public:
    static Ref<CommandEncoder> Create(DeviceBase* device,
                                      const UnpackedPtr<CommandEncoderDescriptor>& descriptor);
    static Ref<CommandEncoder> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;

    CommandIterator AcquireCommands();
    CommandBufferResourceUsage AcquireResourceUsages();
    std::vector<IndirectDrawMetadata> AcquireIndirectDrawMetadata();

    void TrackUsedQuerySet(QuerySetBase* querySet);
    void TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex);

    // Dawn API
    ComputePassEncoder* APIBeginComputePass(const ComputePassDescriptor* descriptor);
    RenderPassEncoder* APIBeginRenderPass(const RenderPassDescriptor* descriptor);

    void APICopyBufferToBuffer(BufferBase* source,
                               uint64_t sourceOffset,
                               BufferBase* destination,
                               uint64_t destinationOffset,
                               uint64_t size);
    void InternalCopyBufferToBufferWithAllocatedSize(BufferBase* source,
                                                     uint64_t sourceOffset,
                                                     BufferBase* destination,
                                                     uint64_t destinationOffset,
                                                     uint64_t size);
    void APICopyBufferToTexture(const TexelCopyBufferInfo* source,
                                const TexelCopyTextureInfo* destination,
                                const Extent3D* copySize);
    void APICopyTextureToBuffer(const TexelCopyTextureInfo* source,
                                const TexelCopyBufferInfo* destination,
                                const Extent3D* copySize);
    void APICopyTextureToTexture(const TexelCopyTextureInfo* source,
                                 const TexelCopyTextureInfo* destination,
                                 const Extent3D* copySize);
    void APIClearBuffer(BufferBase* destination, uint64_t destinationOffset, uint64_t size);

    void APIInjectValidationError(StringView message);
    void APIInsertDebugMarker(StringView groupLabel);
    void APIPopDebugGroup();
    void APIPushDebugGroup(StringView groupLabel);

    void APIResolveQuerySet(QuerySetBase* querySet,
                            uint32_t firstQuery,
                            uint32_t queryCount,
                            BufferBase* destination,
                            uint64_t destinationOffset);
    void APIWriteBuffer(BufferBase* buffer,
                        uint64_t bufferOffset,
                        const uint8_t* data,
                        uint64_t size);
    void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

    CommandBufferBase* APIFinish(const CommandBufferDescriptor* descriptor = nullptr);

    Ref<ComputePassEncoder> BeginComputePass(const ComputePassDescriptor* descriptor = nullptr);
    Ref<RenderPassEncoder> BeginRenderPass(const RenderPassDescriptor* rawDescriptor);
    ResultOrError<Ref<CommandBufferBase>> Finish(
        const CommandBufferDescriptor* descriptor = nullptr);

    // `InternalUsageScope` is a scoped class that temporarily changes validation such that the
    // command encoder includes internal resource usages.
    friend class InternalUsageScope;
    class [[nodiscard]] InternalUsageScope : public NonMovable, public StackAllocated {
      public:
        ~InternalUsageScope();

      private:
        // Only CommandEncoder can make this class.
        friend class CommandEncoder;
        InternalUsageScope(CommandEncoder* encoder);

        raw_ptr<CommandEncoder> mEncoder;
        UsageValidationMode mUsageValidationMode;
    };

    [[nodiscard]] InternalUsageScope MakeInternalUsageScope();

  private:
    CommandEncoder(DeviceBase* device, const UnpackedPtr<CommandEncoderDescriptor>& descriptor);
    CommandEncoder(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    void DestroyImpl() override;

    MaybeError ValidateFinish() const;

    EncodingContext mEncodingContext;
    absl::flat_hash_set<BufferBase*> mTopLevelBuffers;
    absl::flat_hash_set<TextureBase*> mTopLevelTextures;
    absl::flat_hash_set<QuerySetBase*> mUsedQuerySets;

    uint64_t mDebugGroupStackSize = 0;

    UsageValidationMode mUsageValidationMode;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDENCODER_H_
