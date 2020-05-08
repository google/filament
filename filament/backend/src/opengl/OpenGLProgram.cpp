/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "OpenGLProgram.h"

#include "OpenGLDriver.h"

#include <utils/Log.h>
#include <utils/compiler.h>
#include <utils/Panic.h>

#include <cctype>

namespace filament {

using namespace filament::math;
using namespace utils;
using namespace backend;

OpenGLProgram::OpenGLProgram(OpenGLDriver* gl, const Program& programBuilder) noexcept
        :  HwProgram(programBuilder.getName()), mIsValid(false) {

    using Shader = Program::Shader;

    const auto& shadersSource = programBuilder.getShadersSource();

    // build all shaders
    #pragma nounroll
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        GLenum glShaderType;
        Shader type = (Shader)i;
        switch (type) {
            case Shader::VERTEX:
                glShaderType = GL_VERTEX_SHADER;
                break;
            case Shader::FRAGMENT:
                glShaderType = GL_FRAGMENT_SHADER;
                break;
        }

        if (!shadersSource[i].empty()) {
            GLint status;
            char const* const source = (const char*)shadersSource[i].data();

            GLuint shaderId = glCreateShader(glShaderType);
            glShaderSource(shaderId, 1, &source, nullptr);
            glCompileShader(shaderId);

            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
            if (UTILS_UNLIKELY(status != GL_TRUE)) {
                logCompilationError(slog.e, shaderId, source);
                glDeleteShader(shaderId);
                return;
            }
            this->gl.shaders[i] = shaderId;
            mValidShaderSet |= 1U << i;
        }
    }

    // we need at least a vertex and fragment program
    const uint8_t validShaderSet = mValidShaderSet;
    const uint8_t mask = VERTEX_SHADER_BIT | FRAGMENT_SHADER_BIT;
    if (UTILS_LIKELY((mValidShaderSet & mask) == mask)) {
        GLint status;
        GLuint program = glCreateProgram();
        for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
            if (validShaderSet & (1U << i)) {
                glAttachShader(program, this->gl.shaders[i]);
            }
        }
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (UTILS_UNLIKELY(status != GL_TRUE)) {
            char error[512];
            glGetProgramInfoLog(program, sizeof(error), nullptr, error);

            slog.e << "LINKING: " << error << io::endl;
            glDeleteProgram(program);
            return;
        }
        this->gl.program = program;

        // Associate each UniformBlock in the program to a known binding.
        auto const& uniformBlockInfo = programBuilder.getUniformBlockInfo();
        #pragma nounroll
        for (GLuint binding = 0, n = uniformBlockInfo.size(); binding < n; binding++) {
            auto const& name = uniformBlockInfo[binding];
            if (!name.empty()) {
                GLint index = glGetUniformBlockIndex(program, name.c_str());
                if (index >= 0) {
                    glUniformBlockBinding(program, GLuint(index), binding);
                }
                CHECK_GL_ERROR(utils::slog.e)
            }
        }

        if (programBuilder.hasSamplers()) {
            // if we have samplers, we need to do a bit of extra work
            // activate this program so we can set all its samplers once and for all (glUniform1i)
            gl->getContext().useProgram(program);

            auto const& samplerGroupInfo = programBuilder.getSamplerGroupInfo();
            auto& indicesRun = mIndicesRuns;
            uint8_t numUsedBindings = 0;
            uint8_t tmu = 0;

            #pragma nounroll
            for (size_t i = 0, c = samplerGroupInfo.size(); i < c; i++) {
                auto const& groupInfo = samplerGroupInfo[i];
                if (!groupInfo.empty()) {
                    // Cache the sampler uniform locations for each interface block
                    BlockInfo& info = mBlockInfos[numUsedBindings];
                    info.binding = uint8_t(i);
                    uint8_t count = 0;
                    for (uint8_t j = 0, m = uint8_t(groupInfo.size()); j < m; ++j) {
                        // find its location and associate a TMU to it
                        GLint loc = glGetUniformLocation(program, groupInfo[j].name.c_str());
                        if (loc >= 0) {
                            glUniform1i(loc, tmu);
                            indicesRun[tmu] = j;
                            count++;
                            tmu++;
                        } else {
                            // glGetUniformLocation could fail if the uniform is not used
                            // in the program. We should just ignore the error in that case.
                        }
                    }
                    if (count > 0) {
                        numUsedBindings++;
                        info.count = uint8_t(count - 1);
                    }
                }
            }
            mUsedBindingsCount = numUsedBindings;
        }
        mIsValid = true;
    }

    // Failing to compile a program can't be fatal, because this will happen a lot in
    // the material tools. We need to have a better way to handle these errors and
    // return to the editor. Also note the early "return" statements in this function.
    if (UTILS_UNLIKELY(!isValid())) {
        PANIC_LOG("Failed to compile GLSL program.");
    }
}

OpenGLProgram::~OpenGLProgram() noexcept {
    const size_t validShaderSet = mValidShaderSet;
    const bool isValid = mIsValid;
    GLuint program = gl.program;
    if (validShaderSet) {
        #pragma nounroll
        for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
            if (validShaderSet & (1U << i)) {
                const GLuint shader = gl.shaders[i];
                if (isValid) {
                    glDetachShader(program, shader);
                }
                glDeleteShader(shader);
            }
        }
    }
    if (isValid) {
        glDeleteProgram(program);
    }
}

void OpenGLProgram::updateSamplers(OpenGLDriver* gl) noexcept {
    using GLTexture = OpenGLDriver::GLTexture;

    // cache a few member variable locally, outside of the loop
    auto const& UTILS_RESTRICT samplerBindings = gl->getSamplerBindings();
    auto const& UTILS_RESTRICT indicesRun = mIndicesRuns;
    auto const& UTILS_RESTRICT blockInfos = mBlockInfos;

    UTILS_ASSUME(mUsedBindingsCount > 0);
    for (uint8_t i = 0, tmu = 0, n = mUsedBindingsCount; i < n; i++) {
        BlockInfo blockInfo = blockInfos[i];
        HwSamplerGroup const * const UTILS_RESTRICT hwsb = samplerBindings[blockInfo.binding];
        SamplerGroup const& UTILS_RESTRICT sb = *(hwsb->sb);
        SamplerGroup::Sampler const* const UTILS_RESTRICT samplers = sb.getSamplers();
        for (uint8_t j = 0, m = blockInfo.count ; j <= m; ++j, ++tmu) { // "<=" on purpose here
            const uint8_t index = indicesRun[tmu];
            assert(index < sb.getSize());

            Handle<HwTexture> th = samplers[index].t;
            if (UTILS_UNLIKELY(!th)) {
#ifndef NDEBUG
                slog.w << "no texture bound to unit " << +index << io::endl;
#endif
                continue;
            }

            const GLTexture* const UTILS_RESTRICT t = gl->handle_cast<const GLTexture*>(th);
            if (UTILS_UNLIKELY(t->gl.fence)) {
                glWaitSync(t->gl.fence, 0, GL_TIMEOUT_IGNORED);
                glDeleteSync(t->gl.fence);
                t->gl.fence = nullptr;
            }

            gl->bindTexture(tmu, t);

            GLuint sampler = gl->getSampler(samplers[index].s);
            gl->getContext().bindSampler(tmu, sampler);
        }
    }
    CHECK_GL_ERROR(utils::slog.e)
}

void UTILS_NOINLINE OpenGLProgram::logCompilationError(
        io::ostream& out, GLuint shaderId, char const* source) noexcept {
    char error[512];
    glGetShaderInfoLog(shaderId, sizeof(error), nullptr, error);
    out << "COMPILE ERROR: " << io::endl << error << io::endl;

    size_t lc = 1;
    char* shader = strdup(source);
    char* start = shader;
    char* endl = strchr(start, '\n');

    while (endl != nullptr) {
        *endl = '\0';
        out << lc++ << ":   ";
        out << start << io::endl;
        start = endl + 1;
        endl = strchr(start, '\n');
    }

    free(shader);
}

} // namespace filament
