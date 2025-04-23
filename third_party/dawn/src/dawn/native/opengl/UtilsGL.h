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

#ifndef SRC_DAWN_NATIVE_OPENGL_UTILSGL_H_
#define SRC_DAWN_NATIVE_OPENGL_UTILSGL_H_

#include "dawn/native/Format.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {
struct OpenGLFunctions;

GLuint ToOpenGLCompareFunction(wgpu::CompareFunction compareFunction);
GLint GetStencilMaskFromStencilFormat(wgpu::TextureFormat depthStencilFormat);
MaybeError CopyImageSubData(const OpenGLFunctions& gl,
                            Aspect srcAspects,
                            GLuint srcHandle,
                            GLenum srcTarget,
                            GLint srcLevel,
                            const Origin3D& src,
                            GLuint dstHandle,
                            GLenum dstTarget,
                            GLint dstLevel,
                            const Origin3D& dst,
                            const Extent3D& size);
bool HasAnisotropicFiltering(const OpenGLFunctions& gl);

const char* GLErrorAsString(GLenum error);

// Clear all errors on the context, emits logs only
void ClearErrors(const OpenGLFunctions& gl,
                 const char* file,
                 const char* function,
                 unsigned int line);

// Check for a single GL error.
MaybeError CheckError(const OpenGLFunctions& gl,
                      const char* call,
                      const char* file,
                      const char* function,
                      unsigned int line);

#define DAWN_GL_TRY_ALWAYS_CHECK(gl, call)                      \
    (ClearErrors(gl, __FILE__, __func__, __LINE__), (gl.call)); \
    DAWN_TRY(CheckError(gl, "gl" #call, __FILE__, __func__, __LINE__))
#define DAWN_GL_TRY_ALWAYS_CHECK_IGNORE_ERRORS(gl, call)        \
    (ClearErrors(gl, __FILE__, __func__, __LINE__), (gl.call)); \
    IgnoreErrors(CheckError(gl, "gl" #call, __FILE__, __func__, __LINE__))

#if defined(DAWN_ENABLE_ASSERTS)
#define DAWN_GL_TRY(gl, call) DAWN_GL_TRY_ALWAYS_CHECK(gl, call)
#define DAWN_GL_TRY_IGNORE_ERRORS(gl, call) DAWN_GL_TRY_ALWAYS_CHECK_IGNORE_ERRORS(gl, call)
#else
#define DAWN_GL_TRY(gl, call) (gl.call)
#define DAWN_GL_TRY_IGNORE_ERRORS(gl, call) (gl.call)
#endif

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_UTILSGL_H_
