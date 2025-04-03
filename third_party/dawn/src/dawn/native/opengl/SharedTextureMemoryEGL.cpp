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

#include "dawn/native/opengl/SharedTextureMemoryEGL.h"

#include <utility>

#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/native/opengl/UtilsGL.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include <android/hardware_buffer.h>

#include "dawn/native/AHBFunctions.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace dawn::native::opengl {

ResultOrError<Ref<SharedTextureMemory>> SharedTextureMemoryEGL::Create(
    Device* device,
    StringView label,
    const SharedTextureMemoryAHardwareBufferDescriptor* descriptor) {
#if DAWN_PLATFORM_IS(ANDROID)
    const EGLFunctions& egl = device->GetEGL(false);
    DAWN_ASSERT(egl.HasExt(EGLExt::ImageNativeBuffer) && egl.HasExt(EGLExt::GetNativeClientBuffer));

    ::AHardwareBuffer* aHardwareBuffer = static_cast<struct AHardwareBuffer*>(descriptor->handle);
    DAWN_INVALID_IF(aHardwareBuffer == nullptr, "AHardwareBuffer is missing.");

    // Reflect the properties of the AHardwareBuffer.
    SharedTextureMemoryProperties properties =
        GetAHBSharedTextureMemoryProperties(device->GetOrLoadAHBFunctions(), aHardwareBuffer);
    DAWN_INVALID_IF(properties.format == wgpu::TextureFormat::Undefined,
                    "Unknown AHardwareBuffer format cannot be imported.");

    // If the format of the AHB is unknown due to not having an equivalent wgpu::TextureFormat or
    // being an unknowable Android video format, disable all usages except sampling.
    if (properties.format == wgpu::TextureFormat::External) {
        properties.usage &= wgpu::TextureUsage::TextureBinding;
    }

    const EGLAttrib attribs[] = {
        EGL_NONE,
    };
    EGLClientBuffer clientBuffer = egl.GetNativeClientBuffer(aHardwareBuffer);
    ::EGLImage image = egl.CreateImage(device->GetEGLDisplay(), EGL_NO_CONTEXT,
                                       EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attribs);
    DAWN_INVALID_IF(image == nullptr, "EGLImage creation failed, 0x%X", egl.GetError());

    auto result = AcquireRef(new SharedTextureMemoryEGL(device, label, properties, image));
    result->Initialize();
    return result;
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(ANDROID)
}

SharedTextureMemoryEGL::SharedTextureMemoryEGL(Device* device,
                                               StringView label,
                                               const SharedTextureMemoryProperties& properties,
                                               ::EGLImage image)
    : SharedTextureMemory(device, label, properties), mEGLImage(image) {}

void SharedTextureMemoryEGL::DestroyImpl() {
    if (mEGLImage) {
        Device* device = ToBackend(GetDevice());
        const EGLFunctions& egl = device->GetEGL(false);

        egl.DestroyImage(device->GetEGLDisplay(), mEGLImage);
        mEGLImage = nullptr;
    }
}

ResultOrError<GLuint> SharedTextureMemoryEGL::GenerateGLTexture() {
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    GLuint tex;
    DAWN_GL_TRY(gl, GenTextures(1, &tex));
    DAWN_GL_TRY(gl, BindTexture(GL_TEXTURE_2D, tex));
    DAWN_GL_TRY(gl, EGLImageTargetTexture2DOES(GL_TEXTURE_2D, mEGLImage));
    DAWN_GL_TRY(gl, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

    return tex;
}

}  // namespace dawn::native::opengl
