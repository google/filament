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

#include "dawn/native/opengl/DisplayEGL.h"

#include <string>
#include <utility>

namespace dawn::native::opengl {

// static
ResultOrError<Ref<DisplayEGL>> DisplayEGL::CreateFromDynamicLoading(
    wgpu::BackendType backend,
    const char* libName,
    std::span<const std::string> searchPaths) {
    Ref<DisplayEGL> display = AcquireRef(new DisplayEGL(backend));
    DAWN_TRY(display->InitializeWithDynamicLoading(libName, searchPaths));
    return std::move(display);
}

// static
ResultOrError<Ref<DisplayEGL>> DisplayEGL::CreateFromProcAndDisplay(wgpu::BackendType backend,
                                                                    EGLGetProcProc getProc,
                                                                    EGLDisplay eglDisplay) {
    Ref<DisplayEGL> display = AcquireRef(new DisplayEGL(backend));
    DAWN_TRY(display->InitializeWithProcAndDisplay(getProc, eglDisplay));
    return std::move(display);
}

DisplayEGL::DisplayEGL(wgpu::BackendType backend) : egl(mFunctions) {
    switch (backend) {
        case wgpu::BackendType::OpenGL:
            mApiEnum = EGL_OPENGL_API;
            mApiBit = EGL_OPENGL_BIT;
            mRenderableBit = EGL_OPENGL_BIT;
            break;
        case wgpu::BackendType::OpenGLES:
            mApiEnum = EGL_OPENGL_ES_API;
            mApiBit = EGL_OPENGL_ES3_BIT;
            mRenderableBit = EGL_OPENGL_ES2_BIT;
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

DisplayEGL::~DisplayEGL() {
    if (mDisplay != EGL_NO_DISPLAY) {
        // Note that we don't call eglTerminate on purpose here. Calls to eglGetDisplay may return
        // the same display multiple times in which case eglInitialize is a noop (if the display is
        // already initialized) and eglTerminate on one of the handles would terminate the
        // EGLDisplay for everyone.
        mDisplay = EGL_NO_DISPLAY;
    }
}

MaybeError DisplayEGL::InitializeWithDynamicLoading(const char* libName,
                                                    std::span<const std::string> searchPaths) {
    std::string err;
    if (!mLib.Valid() && !mLib.Open(libName, searchPaths, &err)) {
        return DAWN_VALIDATION_ERROR("Failed to load %s: \"%s\".", libName, err.c_str());
    }

    EGLGetProcProc getProc = reinterpret_cast<EGLGetProcProc>(mLib.GetProc("eglGetProcAddress"));
    if (!getProc) {
        return DAWN_VALIDATION_ERROR("Couldn't get \"eglGetProcAddress\" from %s.", libName);
    }

    return InitializeWithProcAndDisplay(getProc, EGL_NO_DISPLAY);
}

MaybeError DisplayEGL::InitializeWithProcAndDisplay(EGLGetProcProc getProc, EGLDisplay display) {
    // Load the EGL functions.
    DAWN_TRY(mFunctions.LoadClientProcs(getProc));

    mDisplay = display;
    if (mDisplay == EGL_NO_DISPLAY) {
        mDisplay = egl.GetDisplay(EGL_DEFAULT_DISPLAY);
    }
    if (mDisplay == EGL_NO_DISPLAY) {
        return DAWN_VALIDATION_ERROR("Couldn't create the default EGL display.");
    }

    DAWN_TRY(mFunctions.LoadDisplayProcs(mDisplay));

    // We require at least EGL 1.4.
    DAWN_INVALID_IF(
        egl.GetMajorVersion() < 1 || (egl.GetMajorVersion() == 1 && egl.GetMinorVersion() < 4),
        "EGL version (%u.%u) must be at least 1.4", egl.GetMajorVersion(), egl.GetMinorVersion());

    return {};
}

EGLDisplay DisplayEGL::GetDisplay() const {
    return mDisplay;
}

EGLint DisplayEGL::GetAPIEnum() const {
    return mApiEnum;
}

EGLint DisplayEGL::GetAPIBit() const {
    return mApiBit;
}

absl::Span<const wgpu::TextureFormat> DisplayEGL::GetPotentialSurfaceFormats() const {
    static constexpr wgpu::TextureFormat kFormatWhenConfigRequired[] = {
        wgpu::TextureFormat::RGBA8Unorm};
    static constexpr wgpu::TextureFormat kFormatsWithNoConfigContext[] = {
        wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8UnormSrgb,
        wgpu::TextureFormat::RGB10A2Unorm, wgpu::TextureFormat::RGBA16Float};

    if (egl.HasExt(EGLExt::NoConfigContext)) {
        return {kFormatsWithNoConfigContext};
    }
    return {kFormatWhenConfigRequired};
}

EGLConfig DisplayEGL::ChooseConfig(EGLint surfaceType,
                                   wgpu::TextureFormat color,
                                   wgpu::TextureFormat depthStencil) const {
    absl::InlinedVector<EGLint, 20> attribs;
    auto AddAttrib = [&](EGLint attrib, EGLint value) {
        attribs.push_back(attrib);
        attribs.push_back(value);
    };

    AddAttrib(EGL_SURFACE_TYPE, surfaceType);
    AddAttrib(EGL_RENDERABLE_TYPE, mRenderableBit);
    AddAttrib(EGL_CONFORMANT, mRenderableBit);

    switch (color) {
        case wgpu::TextureFormat::RGBA8UnormSrgb:
            if (!egl.HasExt(EGLExt::GLColorspace)) {
                return kNoConfig;
            }
            [[fallthrough]];
        case wgpu::TextureFormat::RGBA8Unorm:
            AddAttrib(EGL_RED_SIZE, 8);
            AddAttrib(EGL_BLUE_SIZE, 8);
            AddAttrib(EGL_GREEN_SIZE, 8);
            AddAttrib(EGL_ALPHA_SIZE, 8);
            break;

        case wgpu::TextureFormat::RGB10A2Unorm:
            AddAttrib(EGL_RED_SIZE, 10);
            AddAttrib(EGL_BLUE_SIZE, 10);
            AddAttrib(EGL_GREEN_SIZE, 10);
            AddAttrib(EGL_ALPHA_SIZE, 2);
            break;

        case wgpu::TextureFormat::RGBA16Float:
            if (!egl.HasExt(EGLExt::PixelFormatFloat)) {
                return kNoConfig;
            }
            AddAttrib(EGL_RED_SIZE, 16);
            AddAttrib(EGL_BLUE_SIZE, 16);
            AddAttrib(EGL_GREEN_SIZE, 16);
            AddAttrib(EGL_ALPHA_SIZE, 16);
            AddAttrib(EGL_COLOR_COMPONENT_TYPE_EXT, EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT);
            break;

        default:
            return kNoConfig;
    }

    switch (depthStencil) {
        case wgpu::TextureFormat::Depth24PlusStencil8:
            AddAttrib(EGL_DEPTH_SIZE, 24);
            AddAttrib(EGL_STENCIL_SIZE, 8);
            break;
        case wgpu::TextureFormat::Depth16Unorm:
            AddAttrib(EGL_DEPTH_SIZE, 16);
            break;
        case wgpu::TextureFormat::Undefined:
            break;

        default:
            return kNoConfig;
    }

    // The attrib list is finished with an EGL_NONE tag.
    attribs.push_back(EGL_NONE);

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EGLint numConfigs = 0;
    if (egl.ChooseConfig(mDisplay, attribs.data(), &config, 1, &numConfigs) == EGL_FALSE ||
        numConfigs == 0) {
        return kNoConfig;
    }

    return config;
}

}  // namespace dawn::native::opengl
