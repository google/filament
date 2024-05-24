/*
 * Copyright (C) 2023 The Android Open Source Project
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

#define COREVIDEO_SILENCE_GL_DEPRECATION

#include "CocoaExternalImage.h"
#include <utils/Panic.h>
#include "../GLUtils.h"

namespace filament::backend {

static const char *s_vertex = R"SHADER(#version 410 core
void main() {
    float x = -1.0 + float(((gl_VertexID & 1) <<2));
    float y = -1.0 + float(((gl_VertexID & 2) <<1));
    gl_Position=vec4(x, y, 0.0, 1.0);
}
)SHADER";

static const char *s_fragment = R"SHADER(#version 410 core
precision mediump float;

uniform sampler2DRect rectangle;
layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(rectangle, gl_FragCoord.xy);
}
)SHADER";

CocoaExternalImage::SharedGl::SharedGl() noexcept {
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GLint status;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &s_vertex, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    assert_invariant(status == GL_TRUE);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &s_fragment, nullptr);
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
    GLint samplerLoc = glGetUniformLocation(program, "rectangle");
    glUniform1i(samplerLoc, 0);

    // Restore state.
    glUseProgram(currentProgram);
}

CocoaExternalImage::SharedGl::~SharedGl() noexcept {
    glDeleteSamplers(1, &sampler);
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
}

CocoaExternalImage::CocoaExternalImage(const CVOpenGLTextureCacheRef textureCache,
        const SharedGl &sharedGl) noexcept : mSharedGl(sharedGl), mTextureCache(textureCache) {
    glGenFramebuffers(1, &mFBO);
    CHECK_GL_ERROR(utils::slog.e)
}

CocoaExternalImage::~CocoaExternalImage() noexcept {
    glDeleteFramebuffers(1, &mFBO);
    release();
}

bool CocoaExternalImage::set(CVPixelBufferRef image) noexcept {
    // Release references to a previous external image, if we're holding any.
    release();

    if (!image) {
        return false;
    }

    OSType formatType = CVPixelBufferGetPixelFormatType(image);
    FILAMENT_CHECK_POSTCONDITION(formatType == kCVPixelFormatType_32BGRA)
            << "macOS external images must be 32BGRA format.";

    // The pixel buffer must be locked whenever we do rendering with it. We'll unlock it before
    // releasing.
    UTILS_UNUSED_IN_RELEASE CVReturn lockStatus = CVPixelBufferLockBaseAddress(image, 0);
    assert_invariant(lockStatus == kCVReturnSuccess);

    mImage = image;
    mTexture = createTextureFromImage(image);
    mRgbaTexture = encodeCopyRectangleToTexture2D(CVOpenGLTextureGetName(mTexture),
            CVPixelBufferGetWidth(image), CVPixelBufferGetHeight(image));
    CHECK_GL_ERROR(utils::slog.e)

    return true;
}

GLuint CocoaExternalImage::getGlTexture() const noexcept {
    return mRgbaTexture;
}

GLuint CocoaExternalImage::getInternalFormat() const noexcept {
    if (mRgbaTexture) {
        return GL_RGBA8;
    }
    return 0;
}

GLuint CocoaExternalImage::getTarget() const noexcept {
    if (mRgbaTexture) {
        return GL_TEXTURE_2D;
    }
    return 0;
}

void CocoaExternalImage::release() noexcept {
    if (mImage) {
        CVPixelBufferUnlockBaseAddress(mImage, 0);
        CVPixelBufferRelease(mImage);
    }
    if (mTexture) {
        CFRelease(mTexture);
    }
    if (mRgbaTexture) {
        glDeleteTextures(1, &mRgbaTexture);
        mRgbaTexture = 0;
    }
}

CVOpenGLTextureRef CocoaExternalImage::createTextureFromImage(CVPixelBufferRef image) noexcept {
    CVOpenGLTextureRef texture = nullptr;
    UTILS_UNUSED_IN_RELEASE CVReturn success =
            CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            mTextureCache, image, nil, &texture);
    assert_invariant(success == kCVReturnSuccess);

    return texture;
}

GLuint CocoaExternalImage::encodeCopyRectangleToTexture2D(GLuint rectangle,
        size_t width, size_t height) noexcept {
    GLuint texture;
    glGenTextures(1, &texture);

    mState.save();

    // Create a texture to hold the result of the blit image.
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    CHECK_GL_ERROR(utils::slog.e)

    // source textures
    glBindSampler(0, mSharedGl.sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, rectangle);
    CHECK_GL_ERROR(utils::slog.e)

    // destination texture
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    CHECK_GL_ERROR(utils::slog.e)

    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e, GL_FRAMEBUFFER)
    CHECK_GL_ERROR(utils::slog.e)

    // draw
    glViewport(0, 0, width, height);
    CHECK_GL_ERROR(utils::slog.e)
    glUseProgram(mSharedGl.program);
    CHECK_GL_ERROR(utils::slog.e)
    glDisableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    CHECK_GL_ERROR(utils::slog.e)

    mState.restore();
    CHECK_GL_ERROR(utils::slog.e)

    return texture;
}

void CocoaExternalImage::State::save() noexcept {
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
    glGetIntegerv(GL_SAMPLER_BINDING, &samplerBinding);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexAttrib);
}

void CocoaExternalImage::State::restore() noexcept {
    glActiveTexture(activeTexture);
    glBindTexture(GL_TEXTURE_2D, textureBinding);
    glBindSampler(0, samplerBinding);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    if (vertexAttrib) {
        glEnableVertexAttribArray(0);
    }
}

} // namespace filament::backend
