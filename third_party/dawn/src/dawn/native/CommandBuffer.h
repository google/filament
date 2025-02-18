// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_COMMANDBUFFER_H_
#define SRC_DAWN_NATIVE_COMMANDBUFFER_H_

#include <string>
#include <vector>

#include "dawn/native/dawn_platform.h"

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

struct BeginRenderPassCmd;
struct CopyTextureToBufferCmd;
struct BufferCopy;
struct TextureCopy;

class CommandBufferBase : public ApiObjectBase {
  public:
    CommandBufferBase(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    static Ref<CommandBufferBase> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    const std::string& GetEncoderLabel() const;
    void SetEncoderLabel(std::string encoderLabel);

    MaybeError ValidateCanUseInSubmitNow() const;

    const CommandBufferResourceUsage& GetResourceUsages() const;

    const std::vector<IndirectDrawMetadata>& GetIndirectDrawMetadata();

    CommandIterator* GetCommandIteratorForTesting();

  protected:
    void DestroyImpl() override;

    CommandIterator mCommands;

  private:
    CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    CommandBufferResourceUsage mResourceUsages;
    std::vector<IndirectDrawMetadata> mIndirectDrawMetadata;
    std::string mEncoderLabel;
};

bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D& copySize,
                                   const uint32_t mipLevel,
                                   Aspect aspect);
bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D& copySize,
                                   const uint32_t mipLevel,
                                   wgpu::TextureAspect textureAspect);
SubresourceRange GetSubresourcesAffectedByCopy(const TextureCopy& copy, const Extent3D& copySize);

void LazyClearRenderPassAttachments(BeginRenderPassCmd* renderPass);

bool IsFullBufferOverwrittenInTextureToBufferCopy(const CopyTextureToBufferCmd* copy);
bool IsFullBufferOverwrittenInTextureToBufferCopy(const TextureCopy& source,
                                                  const BufferCopy& destination,
                                                  const Extent3D& copySize);

std::array<float, 4> ConvertToFloatColor(dawn::native::Color color);
std::array<int32_t, 4> ConvertToSignedIntegerColor(dawn::native::Color color);
std::array<uint32_t, 4> ConvertToUnsignedIntegerColor(dawn::native::Color color);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDBUFFER_H_
