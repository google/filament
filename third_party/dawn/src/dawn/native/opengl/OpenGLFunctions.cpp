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

#include "dawn/native/opengl/OpenGLFunctions.h"

#include <cctype>

namespace dawn::native::opengl {

MaybeError OpenGLFunctions::Initialize(GLGetProcProc getProc) {
    DAWN_TRY(mVersion.Initialize(getProc));
    if (mVersion.IsES()) {
#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
        DAWN_TRY(LoadOpenGLESProcs(getProc, mVersion.GetMajor(), mVersion.GetMinor()));
#else
        return DAWN_INTERNAL_ERROR("The OpenGLES backend is not enabled");
#endif
    } else {
#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
        DAWN_TRY(LoadDesktopGLProcs(getProc, mVersion.GetMajor(), mVersion.GetMinor()));
#else
        return DAWN_INTERNAL_ERROR("The OpenGL backend is not enabled");
#endif
    }

    return {};
}

const OpenGLVersion& OpenGLFunctions::GetVersion() const {
    return mVersion;
}

bool OpenGLFunctions::IsAtLeastGL(uint32_t majorVersion, uint32_t minorVersion) const {
    return mVersion.IsDesktop() && mVersion.IsAtLeast(majorVersion, minorVersion);
}

bool OpenGLFunctions::IsAtLeastGLES(uint32_t majorVersion, uint32_t minorVersion) const {
    return mVersion.IsES() && mVersion.IsAtLeast(majorVersion, minorVersion);
}

}  // namespace dawn::native::opengl
