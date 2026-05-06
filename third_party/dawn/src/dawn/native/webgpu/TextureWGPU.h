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

#ifndef SRC_DAWN_NATIVE_WEBGPU_TEXTUREWGPU_H_
#define SRC_DAWN_NATIVE_WEBGPU_TEXTUREWGPU_H_

#include "dawn/native/Error.h"
#include "dawn/native/Texture.h"
#include "dawn/native/webgpu/Forward.h"
#include "dawn/native/webgpu/ObjectWGPU.h"
#include "dawn/native/webgpu/RecordableObject.h"
#include "dawn/webgpu.h"

namespace dawn::native::webgpu {

class Texture final : public TextureBase, public RecordableObject, public ObjectWGPU<WGPUTexture> {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        const SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& descriptor);

    void SynchronizeTextureBeforeUse();

    void SetPendingBeginAccess(bool concurrentRead, bool initialized);

    MaybeError AddReferenced(CaptureContext& captureContext) override;
    MaybeError CaptureCreationParameters(CaptureContext& context) override;
    MaybeError CaptureContentIfNeeded(CaptureContext& context,
                                      schema::ObjectId id,
                                      bool newResource) override;

  private:
    Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor);
    Texture(Device* device,
            const UnpackedPtr<TextureDescriptor>& descriptor,
            const SharedTextureMemory* memory);

    void DestroyImpl(DestroyReason reason) override;
    void SetLabelImpl() override;

    ExecutionSerial OnEndAccess() override;

    bool mPendingBeginAccess = false;
    bool mPendingConcurrentRead = false;
    bool mPendingInitialized = false;
};

class TextureView final : public TextureViewBase,
                          public RecordableObject,
                          public ObjectWGPU<WGPUTextureView> {
  public:
    static ResultOrError<Ref<TextureView>> Create(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor);

    MaybeError AddReferenced(CaptureContext& captureContext) override;
    MaybeError CaptureCreationParameters(CaptureContext& context) override;

  private:
    TextureView(TextureBase* texture,
                const UnpackedPtr<TextureViewDescriptor>& descriptor,
                WGPUTextureView innerView);
    ~TextureView() override = default;
    void SetLabelImpl() override;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_TEXTUREWGPU_H_
