/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "OpenGLBlitter.h"

#include "GLUtils.h"
#include "OpenGLContext.h"

#include <utils/compiler.h>
#include <utils/Log.h>

#include <math/vec2.h>

#include <assert.h>

using namespace filament::math;
using namespace utils;

namespace filament {

static const char s_vertexES[] = R"SHADER(#version 300 es
in vec4 pos;
void main() {
    gl_Position = pos;
}
)SHADER";

static const char s_vertexGL[] = R"SHADER(#version 410 core
in vec4 pos;
void main() {
    gl_Position = pos;
}
)SHADER";

static const char s_fragmentES[] = R"SHADER(#version 300 es
#extension GL_OES_EGL_image_external_essl3 : enable
precision mediump float;
uniform samplerExternalOES sampler;
out vec4 fragColor;
void main() {
    fragColor = texelFetch(sampler, ivec2(gl_FragCoord.xy), 0);
}
)SHADER";

static const char s_fragmentGL[] = R"SHADER(#version 410 core
precision mediump float;
uniform sampler2D sampler;
out vec4 fragColor;
void main() {
    fragColor = texelFetch(sampler, ivec2(gl_FragCoord.xy), 0);
}
)SHADER";

void OpenGLBlitter::init() noexcept {
    glGenSamplers(1, &mSampler);
    glSamplerParameteri(mSampler, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
    glSamplerParameteri(mSampler, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_S,       GL_CLAMP_TO_EDGE);
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_T,       GL_CLAMP_TO_EDGE);
    glSamplerParameteri(mSampler, GL_TEXTURE_WRAP_R,       GL_CLAMP_TO_EDGE);

    GLint status;
    char const* vsource[2] = { s_vertexES, s_vertexGL };
    char const* fsource[2] = { s_fragmentES, s_fragmentGL };
    const size_t index = GLES30_HEADERS ? 0 : 1;

    mVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mVertexShader, 1, vsource + index, nullptr);
    glCompileShader(mVertexShader);
    glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    mFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mFragmentShader, 1, fsource + index, nullptr);
    glCompileShader(mFragmentShader);
    glGetShaderiv(mFragmentShader, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    mProgram = glCreateProgram();
    glAttachShader(mProgram, mVertexShader);
    glAttachShader(mProgram, mFragmentShader);
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    glUseProgram(mProgram);
    GLint loc = glGetUniformLocation(mProgram, "sampler");
    GLuint tmu = 0;
    glUniform1i(loc, tmu);

    glGenFramebuffers(1, &mFBO);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLBlitter::terminate() noexcept {
    glDeleteSamplers(1, &mSampler);
    glDetachShader(mProgram, mVertexShader);
    glDetachShader(mProgram, mFragmentShader);
    glDeleteShader(mVertexShader);
    glDeleteShader(mFragmentShader);
    glDeleteProgram(mProgram);
    glDeleteFramebuffers(1, &mFBO);
}

void OpenGLBlitter::blit(GLuint srcTextureExternal, GLuint dstTexture2d, GLuint w, GLuint h) noexcept {
    const float2 vtx[3] = {{ -1.0f,  3.0f },
                           { -1.0f, -1.0f },
                           {  3.0f, -1.0f }};

    // we're using tmu 0 as the source texture
    GLuint tmu = 0;

    // source texture
    glBindSampler(tmu, mSampler);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, srcTextureExternal);
    CHECK_GL_ERROR(utils::slog.e)

    // destination texture
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture2d, 0);
    CHECK_GL_ERROR(utils::slog.e)
    CHECK_GL_FRAMEBUFFER_STATUS(utils::slog.e)

    // geometry
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vtx);

    // blit...
    glViewport(0, 0, w, h);
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLBlitter::State::save() noexcept {
    mHasState = true;
    // TODO: technically we should also save glVertexAttribPointer
    GLuint tmu = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &array);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_COLOR_WRITEMASK, writeMask);
    scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    stencilTest = glIsEnabled(GL_STENCIL_TEST);
    cullFace = glIsEnabled(GL_CULL_FACE);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexAttrib);

    // save the current active texture first
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    // we're using tmu 0 as the source texture
    glActiveTexture(GL_TEXTURE0 + tmu);

    // save what depends on glActiveTexture
    glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);

    /*
     * Set the state
     */

    // we're using tmu 0 as the source texture
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLBlitter::State::restore() noexcept {
    GLuint tmu = 0;
    glColorMask(
            (GLboolean)writeMask[0],
            (GLboolean)writeMask[1],
            (GLboolean)writeMask[2],
            (GLboolean)writeMask[3]);
    if (cullFace) {
        glEnable(GL_CULL_FACE);
    }
    if (stencilTest) {
        glEnable(GL_STENCIL_TEST);
    }
    if (scissorTest) {
        glEnable(GL_SCISSOR_TEST);
    }
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glUseProgram(static_cast<GLuint>(program));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(array));
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(framebuffer));
    if (!vertexAttrib) {
        glDisableVertexAttribArray(0);
    }

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texture));
    glBindSampler(tmu, static_cast<GLuint>(sampler));
    glActiveTexture(static_cast<GLenum>(activeTexture));
    CHECK_GL_ERROR(utils::slog.e)
}

} // namespace filament
