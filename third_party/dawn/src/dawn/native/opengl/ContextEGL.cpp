// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/native/opengl/ContextEGL.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/opengl/DisplayEGL.h"
#include "dawn/native/opengl/UtilsEGL.h"

#ifndef EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE
#define EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE 0x33AF
#endif

// https://chromium.googlesource.com/angle/angle/+/main/extensions/EGL_ANGLE_create_context_backwards_compatible.txt
#ifndef EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE
#define EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE 0x3483
#endif

// https://chromium.googlesource.com/angle/angle.git/+/refs/heads/chromium/5996/extensions/EGL_ANGLE_create_context_extensions_enabled.txt
#ifndef EGL_EXTENSIONS_ENABLED_ANGLE
#define EGL_EXTENSIONS_ENABLED_ANGLE 0x345F
#endif

namespace dawn::native::opengl {

// static
ResultOrError<std::unique_ptr<ContextEGL>> ContextEGL::Create(Ref<DisplayEGL> display,
                                                              wgpu::BackendType backend,
                                                              bool useRobustness,
                                                              bool useANGLETextureSharing,
                                                              bool forceES31AndMinExtensions) {
    auto context = std::make_unique<ContextEGL>(std::move(display));
    DAWN_TRY(context->Initialize(backend, useRobustness, useANGLETextureSharing,
                                 forceES31AndMinExtensions));
    return std::move(context);
}

ContextEGL::ContextEGL(Ref<DisplayEGL> display) : mDisplay(std::move(display)) {}

ContextEGL::~ContextEGL() {
    if (mOffscreenSurface != EGL_NO_SURFACE) {
        mDisplay->egl.DestroySurface(mDisplay->GetDisplay(), mOffscreenSurface);
        mOffscreenSurface = EGL_NO_SURFACE;
    }
    if (mContext != EGL_NO_CONTEXT) {
        mDisplay->egl.DestroyContext(mDisplay->GetDisplay(), mContext);
        mContext = EGL_NO_CONTEXT;
    }
}

MaybeError ContextEGL::Initialize(wgpu::BackendType backend,
                                  bool useRobustness,
                                  bool useANGLETextureSharing,
                                  bool forceES31AndMinExtensions) {
    const EGLFunctions& egl = mDisplay->egl;

    // Unless EGL_KHR_no_config is present, we need to choose an EGLConfig on context creation that
    // will lock the EGLContext to be use with a single kind of color buffer. In that case the
    // display should only list one kind of color buffer as potentially supported.
    EGLConfig contextConfig = kNoConfig;
    if (!egl.HasExt(EGLExt::NoConfigContext)) {
        DAWN_ASSERT(mDisplay->GetPotentialSurfaceFormats().size() == 1);
        wgpu::TextureFormat format = mDisplay->GetPotentialSurfaceFormats()[0];

        contextConfig = mDisplay->ChooseConfig(EGL_WINDOW_BIT, format);
        if (contextConfig == kNoConfig) {
            return DAWN_FORMAT_INTERNAL_ERROR(
                "Couldn't find an EGLConfig rendering to a window for %s.", format);
        }
    }

    DAWN_TRY(CheckEGL(egl, egl.BindAPI(mDisplay->GetAPIEnum()), "eglBindAPI"));

    absl::InlinedVector<EGLint, 10> attribs;
    auto AddAttrib = [&](EGLint attrib, EGLint value) {
        attribs.push_back(attrib);
        attribs.push_back(value);
    };

    if (egl.HasExt(EGLExt::CreateContext)) {
        switch (backend) {
            case wgpu::BackendType::OpenGLES:
                AddAttrib(EGL_CONTEXT_MAJOR_VERSION, 3);
                AddAttrib(EGL_CONTEXT_MINOR_VERSION, 1);
                break;
            case wgpu::BackendType::OpenGL:
                AddAttrib(EGL_CONTEXT_MAJOR_VERSION, 4);
                AddAttrib(EGL_CONTEXT_MINOR_VERSION, 4);
                break;
            default:
                DAWN_UNREACHABLE();
        }
    } else {
        // Without EGL 1.5 or EGL_KHR_create_context we have to request ES 2.0 or above and see what
        // context version we end up getting.
        AddAttrib(EGL_CONTEXT_CLIENT_VERSION, 2);
    }

    if (useRobustness) {
        DAWN_ASSERT(egl.HasExt(EGLExt::CreateContextRobustness));
        // EGL_EXT_create_context_robustness is promoted to 1.5 but with a different enum value.
        if (egl.GetMinorVersion() >= 5) {
            AddAttrib(EGL_CONTEXT_OPENGL_ROBUST_ACCESS, EGL_TRUE);
        } else {
            AddAttrib(EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT, EGL_TRUE);
        }
    }

    if (useANGLETextureSharing) {
        DAWN_ASSERT(egl.HasExt(EGLExt::DisplayTextureShareGroup));
        AddAttrib(EGL_DISPLAY_TEXTURE_SHARE_GROUP_ANGLE, EGL_TRUE);
    }

    mForceES31AndMinExtensions = forceES31AndMinExtensions;
    if (forceES31AndMinExtensions) {
        if (egl.HasExt(EGLExt::ANGLECreateContextBackwardsCompatible)) {
            AddAttrib(EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_FALSE);
        }
        if (egl.HasExt(EGLExt::ANGLECreateContextExtensionsEnabled)) {
            AddAttrib(EGL_EXTENSIONS_ENABLED_ANGLE, EGL_FALSE);
        }
    }

    // The attrib list is finished with an EGL_NONE tag.
    attribs.push_back(EGL_NONE);

    mContext =
        egl.CreateContext(mDisplay->GetDisplay(), contextConfig, EGL_NO_CONTEXT, attribs.data());
    DAWN_TRY(CheckEGL(egl, mContext != EGL_NO_CONTEXT, "eglCreateContext"));

    // When EGL_KHR_surfaceless_context is not supported, we need to create a pbuffer to act
    // as an offscreen surface.
    if (!egl.HasExt(EGLExt::SurfacelessContext)) {
        // The first potential surface format is always a good choice to find the config. Either we
        // only support this one and the context will be compatible with the pbuffer, or
        // EGL_KHR_no_config_context is supported and the context is compatible with any config.
        wgpu::TextureFormat format = mDisplay->GetPotentialSurfaceFormats()[0];

        EGLConfig pbufferConfig = mDisplay->ChooseConfig(EGL_PBUFFER_BIT, format);
        if (pbufferConfig == kNoConfig) {
            return DAWN_FORMAT_INTERNAL_ERROR(
                "Couldn't find an EGLConfig rendering to a window for %s.", format);
        }

        EGLint pbufferAttribs[] = {
            EGL_WIDTH,  1,  //
            EGL_HEIGHT, 1,  //
            EGL_NONE,
        };
        mOffscreenSurface =
            egl.CreatePbufferSurface(mDisplay->GetDisplay(), pbufferConfig, pbufferAttribs);
        DAWN_TRY(
            CheckEGL(egl, mOffscreenSurface != EGL_NO_SURFACE, "Creating the offscreen surface."));
    }

    return {};
}

// Request compat mode required extensions explicitly when mForceES31AndMinExtensions is true
void ContextEGL::RequestRequiredExtensionsExplicitly() {
    if (!mForceES31AndMinExtensions) {
        return;
    }

    const EGLFunctions& egl = mDisplay->egl;
    // Copied from third_party/angle/include/GLES/gl.h
    typedef void(KHRONOS_APIENTRY * PFNGLREQUESTEXTENSIONANGLEPROC)(const GLchar* name);

    auto proc = egl.GetProcAddress("glRequestExtensionANGLE");
    if (!proc) {
        return;
    }

    auto glRequestExtension = reinterpret_cast<PFNGLREQUESTEXTENSIONANGLEPROC>(proc);

    // src/dawn/native/opengl/supported_extensions.json
    glRequestExtension("GL_OES_texture_stencil8");
    glRequestExtension("GL_EXT_texture_compression_s3tc");
    glRequestExtension("GL_EXT_texture_compression_s3tc_srgb");
    glRequestExtension("GL_OES_EGL_image");
    glRequestExtension("GL_EXT_texture_format_BGRA8888");
    glRequestExtension("GL_APPLE_texture_format_BGRA8888");
    glRequestExtension("GL_EXT_color_buffer_float");
    glRequestExtension("GL_EXT_color_buffer_half_float");
}

void ContextEGL::MakeCurrent() {
    EGLBoolean success = mDisplay->egl.MakeCurrent(mDisplay->GetDisplay(), mCurrentSurface,
                                                   mCurrentSurface, mContext);
    IgnoreErrors(CheckEGL(mDisplay->egl, success == EGL_TRUE, "eglMakeCurrent"));
}

// ScopedMakeSurfaceCurrent

[[nodiscard]] ContextEGL::ScopedMakeSurfaceCurrent ContextEGL::MakeSurfaceCurrentScope(
    EGLSurface surface) {
    return {this, surface};
}

ContextEGL::ScopedMakeSurfaceCurrent::ScopedMakeSurfaceCurrent(ContextEGL* context,
                                                               EGLSurface surface)
    : mContext(context) {
    mContext->mCurrentSurface = surface;
}

ContextEGL::ScopedMakeSurfaceCurrent::~ScopedMakeSurfaceCurrent() {
    mContext->mCurrentSurface = mContext->mOffscreenSurface;
}

}  // namespace dawn::native::opengl
