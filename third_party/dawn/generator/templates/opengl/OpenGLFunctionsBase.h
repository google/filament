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

#ifndef DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_
#define DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_

#include <unordered_set>

#include "dawn/native/Error.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

    struct OpenGLFunctionsBase {
      public:
        {% for block in header_blocks %}
            // {{block.description}}
            {% for proc in block.procs %}
                {{proc.PFNGLPROCNAME()}} {{proc.ProcName()}} = nullptr;
            {% endfor %}

        {% endfor%}

        bool IsGLExtensionSupported(const char* extension) const;

      protected:
#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
        MaybeError LoadDesktopGLProcs(GLGetProcProc getProc, int majorVersion, int minorVersion);
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
        MaybeError LoadOpenGLESProcs(GLGetProcProc getProc, int majorVersion, int minorVersion);
#endif

      private:
        template<typename T>
        MaybeError LoadProc(GLGetProcProc getProc, T* memberProc, const char* name);
        void InitializeSupportedGLExtensions();

        std::unordered_set<std::string> mSupportedGLExtensionsSet;
    };

}  // namespace dawn::native::opengl

#endif // DAWNNATIVE_OPENGL_OPENGLFUNCTIONSBASE_H_
