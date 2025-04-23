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

#ifndef SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_
#define SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_

#include "dawn/native/Texture.h"

#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;
struct GLFormat;
class SharedTextureMemory;

enum class OwnsHandle : uint8_t {
    Yes,
    No,
};

class Texture final : public TextureBase {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& descriptor);

    Texture(Device* device,
            const UnpackedPtr<TextureDescriptor>& descriptor,
            GLuint handle,
            OwnsHandle ownsHandle);

    GLuint GetHandle() const;
    GLenum GetGLTarget() const;
    const GLFormat& GetGLFormat() const;

    MaybeError EnsureSubresourceContentInitialized(const SubresourceRange& range);

    MaybeError SynchronizeTextureBeforeUse();

  private:
    ~Texture() override;

    void DestroyImpl() override;
    MaybeError ClearTexture(const SubresourceRange& range, TextureBase::ClearValue clearValue);

    GLuint mHandle;
    OwnsHandle mOwnsHandle = OwnsHandle::No;
    GLenum mTarget;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor);

    GLuint GetHandle() const;
    GLenum GetGLTarget() const;
    MaybeError BindToFramebuffer(GLenum target, GLenum attachment, GLuint depthLayer = 0);

  private:
    TextureView(TextureBase* texture,
                const UnpackedPtr<TextureViewDescriptor>& descriptor,
                GLuint handle,
                OwnsHandle ownsHandle);

    ~TextureView() override;
    void DestroyImpl() override;
    GLenum GetInternalFormat() const;

    // TODO(crbug.com/dawn/1355): Delete this handle on texture destroy.
    GLuint mHandle;
    GLenum mTarget;
    OwnsHandle mOwnsHandle = OwnsHandle::No;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_TEXTUREGL_H_
