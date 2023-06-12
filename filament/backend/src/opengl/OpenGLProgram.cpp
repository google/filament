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

#include "BlobCacheKey.h"
#include "OpenGLDriver.h"
#include "ShaderCompilerService.h"

#include <utils/debug.h>
#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Systrace.h>

#include <private/backend/BackendUtils.h>

namespace filament::backend {

using namespace filament::math;
using namespace utils;
using namespace backend;

struct OpenGLProgram::LazyInitializationData {
    Program::UniformBlockInfo uniformBlockInfo;
    Program::SamplerGroupInfo samplerGroupInfo;
    std::array<Program::UniformInfo, Program::UNIFORM_BINDING_COUNT> bindingUniformInfo;
};


OpenGLProgram::OpenGLProgram() noexcept = default;

OpenGLProgram::OpenGLProgram(OpenGLDriver& gld, Program&& program) noexcept
        : HwProgram(std::move(program.getName())) {

    auto* const lazyInitializationData = new(std::nothrow) LazyInitializationData();
    lazyInitializationData->samplerGroupInfo = std::move(program.getSamplerGroupInfo());
    if (UTILS_UNLIKELY(gld.getContext().isES2())) {
        lazyInitializationData->bindingUniformInfo = std::move(program.getBindingUniformInfo());
    } else {
        lazyInitializationData->uniformBlockInfo = std::move(program.getUniformBlockBindings());
    }

    ShaderCompilerService& compiler = gld.getShaderCompilerService();
    mToken = compiler.createProgram(name, std::move(program));

    ShaderCompilerService::setUserData(mToken, lazyInitializationData);
}

OpenGLProgram::~OpenGLProgram() noexcept {
    if (mToken) {
        // if the token is non-nullptr it means the program has not been used, and
        // we need to clean-up.
        assert_invariant(gl.program == 0);

        LazyInitializationData* const lazyInitializationData =
                (LazyInitializationData *)ShaderCompilerService::getUserData(mToken);
        delete lazyInitializationData;

        ShaderCompilerService::terminate(mToken);
    }

    delete [] mUniformsRecords;
    const GLuint program = gl.program;
    if (program) {
        glDeleteProgram(program);
    }
}

void OpenGLProgram::initialize(OpenGLDriver& gld) {

    SYSTRACE_CALL();

    assert_invariant(gl.program == 0);
    assert_invariant(mToken);

    LazyInitializationData* const lazyInitializationData =
            (LazyInitializationData *)ShaderCompilerService::getUserData(mToken);

    ShaderCompilerService& compiler = gld.getShaderCompilerService();
    gl.program = compiler.getProgram(mToken);

    assert_invariant(mToken == nullptr);
    if (gl.program) {
        assert_invariant(lazyInitializationData);
        initializeProgramState(gld.getContext(), gl.program, *lazyInitializationData);
        delete lazyInitializationData;
    }
}

/*
 * Initializes our internal state from a valid program. This must only be called after
 * checkProgramStatus() has been successfully called.
 */
void OpenGLProgram::initializeProgramState(OpenGLContext& context, GLuint program,
        LazyInitializationData& lazyInitializationData) noexcept {

    SYSTRACE_CALL();

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    if (!context.isES2()) {
        // Note: This is only needed, because the layout(binding=) syntax is not permitted in glsl
        // (ES3.0 and GL4.1). The backend needs a way to associate a uniform block to a binding point.
        UTILS_NOUNROLL
        for (GLuint binding = 0, n = lazyInitializationData.uniformBlockInfo.size();
                binding < n; binding++) {
            auto const& name = lazyInitializationData.uniformBlockInfo[binding];
            if (!name.empty()) {
                GLuint const index = glGetUniformBlockIndex(program, name.c_str());
                if (index != GL_INVALID_INDEX) {
                    glUniformBlockBinding(program, index, binding);
                }
                CHECK_GL_ERROR(utils::slog.e)
            }
        }
    } else
#endif
    {
        // ES2 initialization of (fake) UBOs
        UniformsRecord* const uniformsRecords = new UniformsRecord[Program::UNIFORM_BINDING_COUNT];
        UTILS_NOUNROLL
        for (GLuint binding = 0, n = Program::UNIFORM_BINDING_COUNT; binding < n; binding++) {
            Program::UniformInfo& uniforms = lazyInitializationData.bindingUniformInfo[binding];
            uniformsRecords[binding].locations.reserve(uniforms.size());
            uniformsRecords[binding].locations.resize(uniforms.size());
            for (size_t j = 0, c = uniforms.size(); j < c; j++) {
                GLint const loc = glGetUniformLocation(program, uniforms[j].name.c_str());
                uniformsRecords[binding].locations[j] = loc;
                if (UTILS_UNLIKELY(binding == 0)) {
                    // This is a bit of a gross hack here, we stash the location of
                    // "frameUniforms.rec709", which obviously the backend shouldn't know about,
                    // which is used for emulating the "rec709" colorspace in the shader.
                    // The backend also shouldn't know that binding 0 is where frameUniform is.
                    std::string_view const uniformName{
                            uniforms[j].name.data(), uniforms[j].name.size() };
                    if (uniformName == "frameUniforms.rec709") {
                        mRec709Location = loc;
                    }
                }
            }
            uniformsRecords[binding].uniforms = std::move(uniforms);
        }
        mUniformsRecords = uniformsRecords;
    }

    uint8_t usedBindingCount = 0;
    uint8_t tmu = 0;

    UTILS_NOUNROLL
    for (size_t i = 0, c = lazyInitializationData.samplerGroupInfo.size(); i < c; i++) {
        auto const& samplers = lazyInitializationData.samplerGroupInfo[i].samplers;
        if (samplers.empty()) {
            // this binding point doesn't have any samplers, skip it.
            continue;
        }

        // keep this in the loop, so we skip it in the rare case a program doesn't have
        // sampler. The context cache will prevent repeated calls to GL.
        context.useProgram(program);

        bool atLeastOneSamplerUsed = false;
        UTILS_NOUNROLL
        for (const Program::Sampler& sampler: samplers) {
            // find its location and associate a TMU to it
            GLint const loc = glGetUniformLocation(program, sampler.name.c_str());
            if (loc >= 0) {
                // this can fail if the program doesn't use this sampler
                glUniform1i(loc, tmu);
                atLeastOneSamplerUsed = true;
            }
            tmu++;
        }

        // if this program doesn't use any sampler from this HwSamplerGroup, just cancel the
        // whole group.
        if (atLeastOneSamplerUsed) {
            // Cache the sampler uniform locations for each interface block
            mUsedSamplerBindingPoints[usedBindingCount] = i;
            usedBindingCount++;
        } else {
            tmu -= samplers.size();
        }
    }
    mUsedBindingsCount = usedBindingCount;
}

void OpenGLProgram::updateSamplers(OpenGLDriver* const gld) const noexcept {
    using GLTexture = OpenGLDriver::GLTexture;

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    bool const es2 = gld->getContext().isES2();
#endif

    // cache a few member variable locally, outside the loop
    auto const& UTILS_RESTRICT samplerBindings = gld->getSamplerBindings();
    auto const& UTILS_RESTRICT usedBindingPoints = mUsedSamplerBindingPoints;

    for (uint8_t i = 0, tmu = 0, n = mUsedBindingsCount; i < n; i++) {
        auto const binding = usedBindingPoints[i];
        auto const * const sb = samplerBindings[binding];
        assert_invariant(sb);
        for (uint8_t j = 0, m = sb->textureUnitEntries.size(); j < m; ++j, ++tmu) { // "<=" on purpose here
            const GLTexture* const t = sb->textureUnitEntries[j].texture;
            if (t) { // program may not use all samplers of sampler group
                gld->bindTexture(tmu, t);
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
                if (UTILS_LIKELY(!es2)) {
                    GLuint const s = sb->textureUnitEntries[j].sampler;
                    gld->bindSampler(tmu, s);
                }
#endif
            }
        }
    }
    CHECK_GL_ERROR(utils::slog.e)
}

void OpenGLProgram::updateUniforms(uint32_t index, void const* buffer, uint16_t age) noexcept {
    assert_invariant(mUniformsRecords);
    assert_invariant(buffer);

    // only update the uniforms if the UBO has changed since last time we updated
    UniformsRecord const& records = mUniformsRecords[index];
    if (records.age == age) {
        return;
    }
    records.age = age;

    assert_invariant(records.uniforms.size() == records.locations.size());

    for (size_t i = 0, c = records.uniforms.size(); i < c; i++) {
        Program::Uniform const& u = records.uniforms[i];
        GLint const loc = records.locations[i];
        if (loc < 0) {
            continue;
        }
        // u.offset is in 'uint32_t' units
        GLfloat const* const bf = reinterpret_cast<GLfloat const*>(buffer) + u.offset;
        GLint const* const bi = reinterpret_cast<GLint const*>(buffer) + u.offset;

        switch(u.type) {
            case UniformType::FLOAT:
                glUniform1fv(loc, u.size, bf);
                break;
            case UniformType::FLOAT2:
                glUniform2fv(loc, u.size, bf);
                break;
            case UniformType::FLOAT3:
                glUniform3fv(loc, u.size, bf);
                break;
            case UniformType::FLOAT4:
                glUniform4fv(loc, u.size, bf);
                break;

            case UniformType::BOOL:
            case UniformType::INT:
            case UniformType::UINT:
                glUniform1iv(loc, u.size, bi);
                break;
            case UniformType::BOOL2:
            case UniformType::INT2:
            case UniformType::UINT2:
                glUniform2iv(loc, u.size, bi);
                break;
            case UniformType::BOOL3:
            case UniformType::INT3:
            case UniformType::UINT3:
                glUniform3iv(loc, u.size, bi);
                break;
            case UniformType::BOOL4:
            case UniformType::INT4:
            case UniformType::UINT4:
                glUniform4iv(loc, u.size, bi);
                break;

            case UniformType::MAT3:
                glUniformMatrix3fv(loc, u.size, GL_FALSE, bf);
                break;
            case UniformType::MAT4:
                glUniformMatrix4fv(loc, u.size, GL_FALSE, bf);
                break;

            case UniformType::STRUCT:
                // not supported
                break;
        }
    }
}

void OpenGLProgram::setRec709ColorSpace(bool rec709) const noexcept {
    glUniform1i(mRec709Location, rec709);
}


} // namespace filament::backend
