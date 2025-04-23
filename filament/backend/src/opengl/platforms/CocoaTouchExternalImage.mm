/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CocoaTouchExternalImage.h"

#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include "../GLUtils.h"

#include <math/vec2.h>

#include <utils/compiler.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <utils/Log.h>

namespace filament::backend {

static const char* s_vertexES = R"SHADER(#version 300 es
in vec4 pos;
void main() {
    gl_Position = pos;
}
)SHADER";

static const char* s_fragmentES = R"SHADER(#version 300 es
precision mediump float;
uniform sampler2D samplerLuminance;
uniform sampler2D samplerColor;
layout(location = 0) out vec4 fragColor;
void main() {
    float luminance = texelFetch(samplerLuminance, ivec2(gl_FragCoord.xy), 0).r;
    // The color plane is half the size of the luminance plane.
    vec2 color = texelFetch(samplerColor, ivec2(gl_FragCoord.xy) / 2, 0).ra;

    vec4 ycbcr = vec4(luminance, color, 1.0);

    mat4 ycbcrToRgbTransform = mat4(
        vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
        vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
        vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
        vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
    );

    fragColor = ycbcrToRgbTransform * ycbcr;
}
)SHADER";

CocoaTouchExternalImage::SharedGl::SharedGl() noexcept {
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S,       GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T,       GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R,       GL_CLAMP_TO_EDGE);

    GLint status;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &s_vertexES, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    assert_invariant(status == GL_TRUE);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &s_fragmentES, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    assert_invariant(status == GL_TRUE);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    assert_invariant(status == GL_TRUE);

    // Save current program state.
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    glUseProgram(program);
    GLint luminanceLoc = glGetUniformLocation(program, "samplerLuminance");
    GLint colorLoc = glGetUniformLocation(program, "samplerColor");
    glUniform1i(luminanceLoc, 0);
    glUniform1i(colorLoc, 1);

    // Restore state.
    glUseProgram(currentProgram);
}

CocoaTouchExternalImage::SharedGl::~SharedGl() noexcept {
    glDeleteSamplers(1, &sampler);
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
}

CocoaTouchExternalImage::CocoaTouchExternalImage(const CVOpenGLESTextureCacheRef textureCache,
        const SharedGl& sharedGl) noexcept : mSharedGl(sharedGl), mTextureCache(textureCache) {

    glGenFramebuffers(1, &mFBO);

    CHECK_GL_ERROR(utils::slog.e)
}

CocoaTouchExternalImage::~CocoaTouchExternalImage() noexcept {
    release();
    glDeleteFramebuffers(1, &mFBO);
}

bool CocoaTouchExternalImage::set(CVPixelBufferRef image) noexcept {
    // Release references to a previous external image, if we're holding any.
    release();

    if (!image) {
        return false;
    }

    OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_POSTCONDITION(formatType == kCVPixelFormatType_32BGRA ||
            formatType == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
            << "iOS external images must be in either 32BGRA or 420f format.";

    size_t planeCount = CVPixelBufferGetPlaneCount(image);
    FILAMENT_CHECK_POSTCONDITION(planeCount == 0 || planeCount == 2)
            << "The OpenGL backend does not support images with plane counts of " << planeCount
            << ".";

    // The pixel buffer must be locked whenever we do rendering with it. We'll unlock it before
    // releasing.
    UTILS_UNUSED_IN_RELEASE CVReturn lockStatus = CVPixelBufferLockBaseAddress(image, 0);
    assert_invariant(lockStatus == kCVReturnSuccess);

    if (planeCount == 0) {
        mImage = image;
        mTexture = createTextureFromImage(image, GL_RGBA, GL_BGRA, 0);
        mEncodedToRgb = false;
    }

    if (planeCount == 2) {
        CVOpenGLESTextureRef yPlane = createTextureFromImage(image, GL_LUMINANCE, GL_LUMINANCE, 0);
        CVOpenGLESTextureRef colorPlane = createTextureFromImage(image, GL_LUMINANCE_ALPHA,
                GL_LUMINANCE_ALPHA, 1);

        size_t width, height;
        width = CVPixelBufferGetWidthOfPlane(image, 0);
        height = CVPixelBufferGetHeightOfPlane(image, 0);
        mRgbTexture = encodeColorConversionPass(CVOpenGLESTextureGetName(yPlane),
                CVOpenGLESTextureGetName(colorPlane), width, height);
        mEncodedToRgb = true;

        // The external image was retained when it was passed to the driver. We're finished
        // with it now, so release it.
        CVPixelBufferUnlockBaseAddress(image, 0);
        CVPixelBufferRelease(image);

        // Likewise with the temporary CVOpenGLESTextureRefs.
        CFRelease(yPlane);
        CFRelease(colorPlane);
    }

    return true;
}

GLuint CocoaTouchExternalImage::getGlTexture() const noexcept {
    if (mEncodedToRgb) {
        return mRgbTexture;
    }
    return CVOpenGLESTextureGetName(mTexture);
}

GLuint CocoaTouchExternalImage::getInternalFormat() const noexcept {
    if (mEncodedToRgb) {
        return GL_RGBA8;
    }
    return GL_R8;
}

GLuint CocoaTouchExternalImage::getTarget() const noexcept {
    if (mEncodedToRgb) {
        return GL_TEXTURE_2D;
    }
    return CVOpenGLESTextureGetTarget(mTexture);
}

void CocoaTouchExternalImage::release() noexcept {
    if (mImage) {
        CVPixelBufferUnlockBaseAddress(mImage, 0);
        CVPixelBufferRelease(mImage);
    }
    if (mTexture) {
        CFRelease(mTexture);
    }
    if (mEncodedToRgb) {
        glDeleteTextures(1, &mRgbTexture);
        mRgbTexture = 0;
    }
}

CVOpenGLESTextureRef CocoaTouchExternalImage::createTextureFromImage(CVPixelBufferRef image, GLuint
        glFormat, GLenum format, size_t plane) noexcept {
    const size_t width = CVPixelBufferGetWidthOfPlane(image, plane);
    const size_t height = CVPixelBufferGetHeightOfPlane(image, plane);

    CVOpenGLESTextureRef texture = nullptr;
    UTILS_UNUSED_IN_RELEASE CVReturn success =
            CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            mTextureCache, image, nullptr, GL_TEXTURE_2D, glFormat, width, height,
            format, GL_UNSIGNED_BYTE, plane, &texture);
    assert_invariant(success == kCVReturnSuccess);

    return texture;
}

GLuint CocoaTouchExternalImage::encodeColorConversionPass(GLuint yPlaneTexture,
    GLuint colorTexture, size_t width, size_t height) noexcept {

    const math::float2 vtx[3] = {{ -1.0f,  3.0f },
                                 { -1.0f, -1.0f },
                                 {  3.0f, -1.0f }};

    GLuint texture;
    glGenTextures(1, &texture);

    mState.save();

    // Create a texture to hold the result of the RGB conversion.
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    CHECK_GL_ERROR(utils::slog.e)

    // source textures
    glBindSampler(0, mSharedGl.sampler);
    glBindSampler(1, mSharedGl.sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, yPlaneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    // destination texture
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)

    // geometry
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vtx);

    // draw
    glViewport(0, 0, width, height);
    glUseProgram(mSharedGl.program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    CHECK_GL_ERROR(utils::slog.e)

    mState.restore();

    return texture;
}

void CocoaTouchExternalImage::State::save() noexcept {
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &array);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexAttrib);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);
    glGetIntegerv(GL_VIEWPORT, viewport);

    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_SAMPLER_BINDING, &sampler[0]);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding[0]);

    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_SAMPLER_BINDING, &sampler[1]);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding[1]);
}

void CocoaTouchExternalImage::State::restore() noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBinding[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureBinding[1]);

    glBindBuffer(GL_ARRAY_BUFFER, array);
    glBindVertexArray(vertexArray);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glBindSampler(0, sampler[0]);
    glBindSampler(1, sampler[1]);
    glActiveTexture(activeTexture);
    if (!vertexAttrib) {
        glDisableVertexAttribArray(0);
    }
}

} // namespace filament::backend
