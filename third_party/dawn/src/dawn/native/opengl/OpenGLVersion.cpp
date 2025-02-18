// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/opengl/OpenGLVersion.h"

#include <cctype>
#include <string>
#include <tuple>

namespace dawn::native::opengl {

MaybeError OpenGLVersion::Initialize(GLGetProcProc getProc) {
    PFNGLGETSTRINGPROC getString = reinterpret_cast<PFNGLGETSTRINGPROC>(getProc("glGetString"));
    if (getString == nullptr) {
        return DAWN_INTERNAL_ERROR("Couldn't load glGetString");
    }

    const char* version = reinterpret_cast<const char*>(getString(GL_VERSION));

    if (strstr(version, "OpenGL ES") != nullptr) {
        // ES spec states that the GL_VERSION string will be in the following format:
        // "OpenGL ES N.M vendor-specific information"
        mStandard = Standard::ES;
        mMajorVersion = version[10] - '0';
        mMinorVersion = version[12] - '0';

        // The minor version shouldn't get to two digits.
        DAWN_ASSERT(strlen(version) <= 13 || !isdigit(version[13]));
    } else {
        // OpenGL spec states the GL_VERSION string will be in the following format:
        // <version number><space><vendor-specific information>
        // The version number is either of the form major number.minor number or major
        // number.minor number.release number, where the numbers all have one or more
        // digits
        mStandard = Standard::Desktop;
        mMajorVersion = version[0] - '0';
        mMinorVersion = version[2] - '0';

        // The minor version shouldn't get to two digits.
        DAWN_ASSERT(strlen(version) <= 3 || !isdigit(version[3]));
    }

    return {};
}

bool OpenGLVersion::IsDesktop() const {
    return mStandard == Standard::Desktop;
}

bool OpenGLVersion::IsES() const {
    return mStandard == Standard::ES;
}

OpenGLVersion::Standard OpenGLVersion::GetStandard() const {
    return mStandard;
}

uint32_t OpenGLVersion::GetMajor() const {
    return mMajorVersion;
}

uint32_t OpenGLVersion::GetMinor() const {
    return mMinorVersion;
}

bool OpenGLVersion::IsAtLeast(uint32_t majorVersion, uint32_t minorVersion) const {
    return std::tie(mMajorVersion, mMinorVersion) >= std::tie(majorVersion, minorVersion);
}

}  // namespace dawn::native::opengl
