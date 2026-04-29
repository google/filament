// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_TEXELBUFFERVIEW_H_
#define SRC_DAWN_NATIVE_TEXELBUFFERVIEW_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

bool IsFormatSupportedForTexelBuffer(wgpu::TextureFormat format);
ResultOrError<const Format*> ValidateTexelBufferFormat(DeviceBase* device,
                                                       wgpu::TextureFormat format);

class TexelBufferViewBase : public ApiObjectBase {
  public:
    TexelBufferViewBase(BufferBase* buffer,
                        const UnpackedPtr<TexelBufferViewDescriptor>& descriptor);
    TexelBufferViewBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);
    ~TexelBufferViewBase() override;

    static Ref<TexelBufferViewBase> MakeError(DeviceBase* device, StringView label = {});

    ObjectType GetType() const override;

    BufferBase* GetBuffer() const;
    wgpu::TextureFormat GetFormat() const;
    uint64_t GetOffset() const;
    uint64_t GetSize() const;

  protected:
    void DestroyImpl(DestroyReason reason) override;

  private:
    ApiObjectList* GetObjectTrackingList() override;

    Ref<BufferBase> mBuffer;
    wgpu::TextureFormat mFormat = wgpu::TextureFormat::Undefined;
    uint64_t mOffset = 0;
    uint64_t mSize = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TEXELBUFFERVIEW_H_
