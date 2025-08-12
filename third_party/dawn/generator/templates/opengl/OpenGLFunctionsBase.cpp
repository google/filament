//* Copyright 2019 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/opengl/OpenGLFunctionsBase_autogen.h"

namespace dawn::native::opengl {

template<typename T>
MaybeError OpenGLFunctionsBase::LoadProc(GLGetProcProc getProc, T* memberProc, const char* name) {
    *memberProc = reinterpret_cast<T>(getProc(name));
    if (DAWN_UNLIKELY(memberProc == nullptr)) {
        return DAWN_INTERNAL_ERROR(std::string("Couldn't load GL proc: ") + name);
    }
    return {};
}

#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
MaybeError OpenGLFunctionsBase::LoadOpenGLESProcs(GLGetProcProc getProc, int majorVersion, int minorVersion) {
    {% for block in gles_blocks %}
        // OpenGL ES {{block.version.major}}.{{block.version.minor}}
        if (majorVersion > {{block.version.major}} || (majorVersion == {{block.version.major}} && minorVersion >= {{block.version.minor}})) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }

    {% endfor %}

    InitializeSupportedGLExtensions();

    {% for block in extension_gles_blocks %}
        // {{block.extension}}
        if (IsGLExtensionSupported("{{block.extension}}")) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
        {% endfor %}
        }
    {% endfor %}

    return {};
}
#endif

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
MaybeError OpenGLFunctionsBase::LoadDesktopGLProcs(GLGetProcProc getProc, int majorVersion, int minorVersion) {
    {% for block in desktop_gl_blocks %}
        // Desktop OpenGL {{block.version.major}}.{{block.version.minor}}
        if (majorVersion > {{block.version.major}} || (majorVersion == {{block.version.major}} && minorVersion >= {{block.version.minor}})) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }

    {% endfor %}

    InitializeSupportedGLExtensions();

    {% for block in extension_desktop_gl_blocks %}
        // {{block.extension}}
        if (IsGLExtensionSupported("{{block.extension}}")) {
            {% for proc in block.procs %}
                DAWN_TRY(LoadProc(getProc, &{{proc.ProcName()}}, "{{proc.glProcName()}}"));
            {% endfor %}
        }
    {% endfor %}

    return {};
}
#endif

void OpenGLFunctionsBase::InitializeSupportedGLExtensions() {
    int32_t numExtensions;
    GetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int32_t i = 0; i < numExtensions; ++i) {
        const char* extensionName = reinterpret_cast<const char*>(GetStringi(GL_EXTENSIONS, i));
        mSupportedGLExtensionsSet.insert(extensionName);
    }
}

bool OpenGLFunctionsBase::IsGLExtensionSupported(const char* extension) const {
    DAWN_ASSERT(extension != nullptr);
    return mSupportedGLExtensionsSet.contains(extension);
}

}  // namespace dawn::native::opengl
