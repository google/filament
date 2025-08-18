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

#include "dawn/native/opengl/UtilsGL.h"

#include <string>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/opengl/OpenGLFunctions.h"

namespace dawn::native::opengl {

GLuint ToOpenGLCompareFunction(wgpu::CompareFunction compareFunction) {
    switch (compareFunction) {
        case wgpu::CompareFunction::Never:
            return GL_NEVER;
        case wgpu::CompareFunction::Less:
            return GL_LESS;
        case wgpu::CompareFunction::LessEqual:
            return GL_LEQUAL;
        case wgpu::CompareFunction::Greater:
            return GL_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return GL_GEQUAL;
        case wgpu::CompareFunction::NotEqual:
            return GL_NOTEQUAL;
        case wgpu::CompareFunction::Equal:
            return GL_EQUAL;
        case wgpu::CompareFunction::Always:
            return GL_ALWAYS;

        case wgpu::CompareFunction::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

GLint GetStencilMaskFromStencilFormat(wgpu::TextureFormat depthStencilFormat) {
    switch (depthStencilFormat) {
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
        case wgpu::TextureFormat::Stencil8:
            return 0xFF;

        default:
            DAWN_UNREACHABLE();
    }
}

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
                            const Extent3D& size) {
    if (gl.IsAtLeastGL(4, 3) || gl.IsAtLeastGLES(3, 2)) {
        DAWN_GL_TRY(gl, CopyImageSubData(srcHandle, srcTarget, srcLevel, src.x, src.y, src.z,
                                         dstHandle, dstTarget, dstLevel, dst.x, dst.y, dst.z,
                                         size.width, size.height, size.depthOrArrayLayers));
        return {};
    }

    GLint prevReadFBO = 0, prevDrawFBO = 0;
    DAWN_GL_TRY(gl, GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO));
    DAWN_GL_TRY(gl, GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO));

    // Generate temporary framebuffers for the blits.
    GLuint readFBO = 0, drawFBO = 0;
    DAWN_GL_TRY(gl, GenFramebuffers(1, &readFBO));
    DAWN_GL_TRY(gl, GenFramebuffers(1, &drawFBO));
    DAWN_GL_TRY(gl, BindFramebuffer(GL_READ_FRAMEBUFFER, readFBO));
    DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO));

    // Reset state that may affect glBlitFramebuffer().
    DAWN_GL_TRY(gl, Disable(GL_SCISSOR_TEST));
    GLenum blitMask = 0;
    if (srcAspects & Aspect::Color) {
        blitMask |= GL_COLOR_BUFFER_BIT;
    }
    if (srcAspects & Aspect::Depth) {
        blitMask |= GL_DEPTH_BUFFER_BIT;
    }
    if (srcAspects & Aspect::Stencil) {
        blitMask |= GL_STENCIL_BUFFER_BIT;
    }

    // Iterate over all layers, doing a single blit for each.
    for (uint32_t layer = 0; layer < size.depthOrArrayLayers; ++layer) {
        // Set attachments for all aspects.
        for (Aspect aspect : IterateEnumMask(srcAspects)) {
            GLenum glAttachment;
            switch (aspect) {
                case Aspect::Color:
                    glAttachment = GL_COLOR_ATTACHMENT0;
                    break;
                case Aspect::Depth:
                    glAttachment = GL_DEPTH_ATTACHMENT;
                    break;
                case Aspect::Stencil:
                    glAttachment = GL_STENCIL_ATTACHMENT;
                    break;
                case Aspect::CombinedDepthStencil:
                case Aspect::None:
                case Aspect::Plane0:
                case Aspect::Plane1:
                case Aspect::Plane2:
                    DAWN_UNREACHABLE();
            }
            if (srcTarget == GL_TEXTURE_2D) {
                DAWN_GL_TRY(gl, FramebufferTexture2D(GL_READ_FRAMEBUFFER, glAttachment, srcTarget,
                                                     srcHandle, srcLevel));
            } else {
                DAWN_GL_TRY(gl, FramebufferTextureLayer(GL_READ_FRAMEBUFFER, glAttachment,
                                                        srcHandle, srcLevel, src.z + layer));
            }
            if (dstTarget == GL_TEXTURE_2D) {
                DAWN_GL_TRY(gl, FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachment, dstTarget,
                                                     dstHandle, dstLevel));
            } else if (dstTarget == GL_TEXTURE_CUBE_MAP) {
                GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
                DAWN_GL_TRY(gl, FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, glAttachment, target,
                                                     dstHandle, dstLevel));
            } else {
                DAWN_GL_TRY(gl, FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, glAttachment,
                                                        dstHandle, dstLevel, dst.z + layer));
            }
        }
        DAWN_GL_TRY(
            gl, BlitFramebuffer(src.x, src.y, src.x + size.width, src.y + size.height, dst.x, dst.y,
                                dst.x + size.width, dst.y + size.height, blitMask, GL_NEAREST));
    }
    DAWN_GL_TRY(gl, Enable(GL_SCISSOR_TEST));
    DAWN_GL_TRY(gl, DeleteFramebuffers(1, &readFBO));
    DAWN_GL_TRY(gl, DeleteFramebuffers(1, &drawFBO));
    DAWN_GL_TRY(gl, BindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO));
    DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO));
    return {};
}

bool HasAnisotropicFiltering(const OpenGLFunctions& gl) {
    return gl.IsAtLeastGL(4, 6) || gl.IsGLExtensionSupported("GL_EXT_texture_filter_anisotropic");
}

const char* GLErrorAsString(GLenum error) {
#define ERROR_CASE_STRING(errorEnum) \
    case errorEnum:                  \
        return #errorEnum

    switch (error) {
        ERROR_CASE_STRING(GL_INVALID_ENUM);
        ERROR_CASE_STRING(GL_INVALID_OPERATION);
        ERROR_CASE_STRING(GL_INVALID_VALUE);
        ERROR_CASE_STRING(GL_INVALID_FRAMEBUFFER_OPERATION);
        ERROR_CASE_STRING(GL_OUT_OF_MEMORY);
        ERROR_CASE_STRING(GL_CONTEXT_LOST);
        default:
            return "<Unknown OpenGL error>";
    }

#undef ERROR_CASE_STRING
}

void ClearErrors(const OpenGLFunctions& gl,
                 const char* file,
                 const char* function,
                 unsigned int line) {
    GLenum error = gl.GetError();
    if (error == GL_NO_ERROR) [[likely]] {
        return;
    }

    std::string message = std::string("Preexisting OpenGL errors: ") + GLErrorAsString(error);

    error = gl.GetError();
    while (error != GL_NO_ERROR) {
        // Skip GL_CONTEXT_LOST errors, they will be generated continuously and result in an
        // infinite loop.
        if (error == GL_CONTEXT_LOST) {
            break;
        }

        message += std::string(", ") + GLErrorAsString(error);
        error = gl.GetError();
    }

    DebugLog(file, function, line) << message;
}

MaybeError CheckError(const OpenGLFunctions& gl,
                      const char* call,
                      const char* file,
                      const char* function,
                      unsigned int line) {
    GLenum error = gl.GetError();
    if (error == GL_NO_ERROR) [[likely]] {
        return {};
    }

    std::string message = std::string(call) + " failed with " + GLErrorAsString(error);

    // Check that only one GL error was generated, ClearErrors should have been called first.
    GLenum nextError = gl.GetError();
    while (nextError != GL_NO_ERROR) {
        // Skip GL_CONTEXT_LOST errors, they will be generated continuously and result in an
        // infinite loop.
        if (nextError == GL_CONTEXT_LOST) {
            break;
        }

        message += std::string(", ") + GLErrorAsString(nextError);
        nextError = gl.GetError();
    }

    DebugLog(file, function, line) << message;

    switch (error) {
        case GL_OUT_OF_MEMORY:
            return DAWN_OUT_OF_MEMORY_ERROR(message);
        case GL_CONTEXT_LOST:
            return DAWN_DEVICE_LOST_ERROR(message);
        default:
            return DAWN_INTERNAL_ERROR(message);
    }
}

}  // namespace dawn::native::opengl
