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
#include <utils/debug.h>

#include <private/backend/BackendUtils.h>

#include <ctype.h>

namespace filament::backend {

using namespace filament::math;
using namespace utils;
using namespace backend;

static void logCompilationError(utils::io::ostream& out,
        ShaderStage shaderType, const char* name,
        GLuint shaderId, CString const& sourceCode) noexcept;

static void logProgramLinkError(utils::io::ostream& out,
        const char* name, GLuint program) noexcept;

OpenGLProgram::OpenGLProgram() noexcept
    : mInitialized(false), mValid(true), mLazyInitializationData(nullptr) {
}

OpenGLProgram::OpenGLProgram(OpenGLDriver& gld, Program&& programBuilder) noexcept
        : HwProgram(std::move(programBuilder.getName())),
          mInitialized(false), mValid(true),
          mLazyInitializationData{ new(LazyInitializationData) } {

    OpenGLContext& context = gld.getContext();

    mLazyInitializationData->uniformBlockInfo = std::move(programBuilder.getUniformBlockBindings());
    mLazyInitializationData->samplerGroupInfo = std::move(programBuilder.getSamplerGroupInfo());

    // this cannot fail because we check compilation status after linking the program
    // shaders[] is filled with id of shader stages present.
    OpenGLProgram::compileShaders(context,
            std::move(programBuilder.getShadersSource()),
            programBuilder.getSpecializationConstants(),
            gl.shaders,
            mLazyInitializationData->shaderSourceCode);

    gld.runAtNextRenderPass(this, [this]() {
        // by this point we must not have a GL program
        assert_invariant(!gl.program);
        // we also can't be in the initialized state
        assert_invariant(!mInitialized);
        // we must have our lazy initialization data
        assert_invariant(mLazyInitializationData);
        // link the program, this also cannot fail because status is checked later.
        gl.program = OpenGLProgram::linkProgram(gl.shaders);
    });
}

OpenGLProgram::~OpenGLProgram() noexcept {
    if (!mInitialized) {
        // mLazyInitializationData is aliased with mIndicesRuns
        delete mLazyInitializationData;
    }
    const GLuint program = gl.program;
    UTILS_NOUNROLL
    for (GLuint shader: gl.shaders) {
        if (shader) {
            if (program) {
                glDetachShader(program, shader);
            }
            glDeleteShader(shader);
        }
    }
    if (program) {
        glDeleteProgram(program);
    }
}

/*
 * Compile shaders in the ShaderSource. This cannot fail because compilation failures are not
 * checked until after the program is linked.
 * This always returns the GL shader IDs or zero a shader stage is not present.
 */
void OpenGLProgram::compileShaders(OpenGLContext& context,
        Program::ShaderSource shadersSource,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
        GLuint shaderIds[Program::SHADER_TYPE_COUNT],
        std::array<CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept {

    std::string specificationConstantString;
    for (auto const& sc : specializationConstants) {
        specificationConstantString += "#define SPIRV_CROSS_CONSTANT_ID_" + std::to_string(sc.id) + ' ';
        specificationConstantString += std::visit([](auto&& arg) {
            return std::to_string(arg);
        }, sc.value);
        specificationConstantString += '\n';
    }
    if (!specificationConstantString.empty()) {
        specificationConstantString += '\n';
    }

    // build all shaders
    UTILS_NOUNROLL
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const ShaderStage stage = static_cast<ShaderStage>(i);
        GLenum glShaderType;
        switch (stage) {
            case ShaderStage::VERTEX:    glShaderType = GL_VERTEX_SHADER;    break;
            case ShaderStage::FRAGMENT:  glShaderType = GL_FRAGMENT_SHADER;  break;
            case ShaderStage::COMPUTE:   glShaderType = GL_COMPUTE_SHADER;   break;
        }

        if (UTILS_LIKELY(!shadersSource[i].empty())) {
            Program::ShaderBlob& shader = shadersSource[i];

            // remove GOOGLE_cpp_style_line_directive
            std::string_view source = process_GOOGLE_cpp_style_line_directive(context,
                    reinterpret_cast<char*>(shader.data()), shader.size());

            // add support for ARB_shading_language_packing if needed
            auto const packingFunctions = process_ARB_shading_language_packing(context);

            // split shader source, so we can insert the specification constants and the packing functions
            auto const [prolog, body] = splitShaderSource(source);

            const std::array<const char*, 4> sources = {
                    prolog.data(),
                    specificationConstantString.c_str(),
                    packingFunctions.data(),
                    body.data()
            };

            const std::array<GLint, 4> lengths = {
                    (GLint)prolog.length(),
                    (GLint)specificationConstantString.length(),
                    (GLint)packingFunctions.length(),
                    (GLint)body.length() - 1 // null terminated
            };

            GLuint shaderId = glCreateShader(glShaderType);
            glShaderSource(shaderId, sources.size(), sources.data(), lengths.data());
            glCompileShader(shaderId);

#ifndef NDEBUG
            // for debugging we return the original shader source (without the modifications we
            // made here), otherwise the line numbers wouldn't match.
            outShaderSourceCode[i] = { source.data(), source.length() };
#endif

            shaderIds[i] = shaderId;
        }
    }
}

// If usages of the Google-style line directive are present, remove them, as some
// drivers don't allow the quotation marks. This happens in-place.
std::string_view OpenGLProgram::process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
        char* source, size_t len) noexcept {
    if (!context.ext.GOOGLE_cpp_style_line_directive) {
        if (UTILS_UNLIKELY(requestsGoogleLineDirectivesExtension({ source, len }))) {
            removeGoogleLineDirectives(source, len); // length is unaffected
        }
    }
    return { source, len };
}

// Tragically, OpenGL 4.1 doesn't support unpackHalf2x16 and
// macOS doesn't support GL_ARB_shading_language_packing
std::string_view OpenGLProgram::process_ARB_shading_language_packing(OpenGLContext& context) noexcept {
    using namespace std::literals;
    if constexpr (BACKEND_OPENGL_VERSION == BACKEND_OPENGL_VERSION_GL) {
        if (context.state.major == 4 && context.state.minor == 1 &&
            !context.ext.ARB_shading_language_packing) {
            return R"(

// these don't handle denormals, NaNs or inf
float u16tofp32(highp uint v) {
    v <<= 16u;
    highp uint s = v & 0x80000000u;
    highp uint n = v & 0x7FFFFFFFu;
    highp uint nz = n == 0u ? 0u : 0xFFFFFFFF;
    return uintBitsToFloat(s | ((((n >> 3u) + (0x70u << 23))) & nz));
}
vec2 unpackHalf2x16(highp uint v) {
    return vec2(u16tofp32(v&0xFFFFu), u16tofp32(v>>16u));
}
uint fp32tou16(float val) {
    uint f32 = floatBitsToUint(val);
    uint f16 = 0u;
    uint sign = (f32 >> 16) & 0x8000u;
    int exponent = int((f32 >> 23) & 0xFFu) - 127;
    uint mantissa = f32 & 0x007FFFFFu;
    if (exponent > 15) {
        f16 = sign | (0x1Fu << 10);
    } else if (exponent > -15) {
        exponent += 15;
        mantissa >>= 13;
        f16 = sign | uint(exponent << 10) | mantissa;
    } else {
        f16 = sign;
    }
    return f16;
}
highp uint packHalf2x16(vec2 v) {
    highp uint x = fp32tou16(v.x);
    highp uint y = fp32tou16(v.y);
    return (y << 16) | x;
}
)"sv;
        }
    }
    return ""sv;
}

// split shader source code in two, the first section goes from the start to the line after the
// last #extension, and the 2nd part goes from there to the end.
std::array<std::string_view, 2> OpenGLProgram::splitShaderSource(std::string_view source) noexcept {
    auto start = source.find("#version");
    assert_invariant(start != std::string_view::npos);

    auto pos = source.rfind("\n#extension");
    if (pos == std::string_view::npos) {
        pos = start;
    } else {
        ++pos;
    }

    auto eol = source.find('\n', pos) + 1;
    assert_invariant(eol != std::string_view::npos);

    std::string_view version = source.substr(start, eol - start);
    std::string_view body = source.substr(version.length(), source.length() - version.length());
    return { version, body };
}

/*
 * Create a program from the given shader IDs and links it. This cannot fail because errors
 * are checked later. This always returns a valid GL program ID (which doesn't mean the
 * program itself is valid).
 */
GLuint OpenGLProgram::linkProgram(const GLuint shaderIds[Program::SHADER_TYPE_COUNT]) noexcept {
    GLuint program = glCreateProgram();
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        if (shaderIds[i]) {
            glAttachShader(program, shaderIds[i]);
        }
    }
    glLinkProgram(program);
    return program;
}

/*
 * Checks a program link status and logs errors and frees resources on failure.
 * Returns true on success.
 */
bool OpenGLProgram::checkProgramStatus(const char* name,
        GLuint& program, GLuint shaderIds[Program::SHADER_TYPE_COUNT],
        std::array<CString, Program::SHADER_TYPE_COUNT> const& shaderSourceCode) noexcept {

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (UTILS_LIKELY(status == GL_TRUE)) {
        return true;
    }

    // only if the link fails, we check the compilation status
    UTILS_NOUNROLL
    for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
        const ShaderStage type = static_cast<ShaderStage>(i);
        const GLuint shader = shaderIds[i];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            logCompilationError(slog.e, type, name, shader, shaderSourceCode[i]);
        }
        glDetachShader(program, shader);
        glDeleteShader(shader);
        shaderIds[i] = 0;
    }
    // log the link error as well
    logProgramLinkError(slog.e, name, program);
    glDeleteProgram(program);
    program = 0;
    return false;
}

void OpenGLProgram::initialize(OpenGLContext& context) {
    // by this point we must have a GL program
    assert_invariant(gl.program);
    // we also can't be in the initialized state
    assert_invariant(!mInitialized);
    // we must have our lazy initialization data
    assert_invariant(mLazyInitializationData);

    // we must copy mLazyInitializationData locally because it is aliased with mIndicesRuns
    auto* const initializationData = mLazyInitializationData;

    // check status of program linking and shader compilation, logs error and free all resources
    // in case of error.
    mValid = OpenGLProgram::checkProgramStatus(name.c_str_safe(),
            gl.program, gl.shaders, initializationData->shaderSourceCode);

    if (UTILS_LIKELY(mValid)) {
        initializeProgramState(context, gl.program, *initializationData);
    }

    // and destroy all temporary init data
    delete initializationData;
    // mInitialized means mLazyInitializationData is no more valid
    mInitialized = true;
}

/*
 * Initializes our internal state from a valid program. This must only be called after
 * checkProgramStatus() has been successfully called.
 */
void OpenGLProgram::initializeProgramState(OpenGLContext& context, GLuint program,
        LazyInitializationData const& lazyInitializationData) noexcept {

    // Note: This is only needed, because the layout(binding=) syntax is not permitted in glsl
    // (ES3.0 and GL4.1). The backend needs a way to associate a uniform block to a binding point.
    UTILS_NOUNROLL
    for (GLuint binding = 0, n = lazyInitializationData.uniformBlockInfo.size(); binding < n; binding++) {
        auto const& name = lazyInitializationData.uniformBlockInfo[binding];
        if (!name.empty()) {
            GLint index = glGetUniformBlockIndex(program, name.c_str());
            if (index >= 0) {
                glUniformBlockBinding(program, GLuint(index), binding);
            }
            CHECK_GL_ERROR(utils::slog.e)
        }
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
            GLint loc = glGetUniformLocation(program, sampler.name.c_str());
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

void OpenGLProgram::updateSamplers(OpenGLDriver* gld) const noexcept {
    using GLTexture = OpenGLDriver::GLTexture;

    // cache a few member variable locally, outside the loop
    auto const& UTILS_RESTRICT samplerBindings = gld->getSamplerBindings();
    auto const& UTILS_RESTRICT usedBindingPoints = mUsedSamplerBindingPoints;

    for (uint8_t i = 0, tmu = 0, n = mUsedBindingsCount; i < n; i++) {
        auto const binding = usedBindingPoints[i];
        auto const * const sb = samplerBindings[binding];
        assert_invariant(sb);
        for (uint8_t j = 0, m = sb->textureUnitEntries.size(); j < m; ++j, ++tmu) { // "<=" on purpose here
            const GLTexture* const t = sb->textureUnitEntries[j].texture;
            GLuint s = sb->textureUnitEntries[j].sampler;
            if (t) { // program may not use all samplers of sampler group
                if (UTILS_UNLIKELY(t->gl.fence)) {
                    glWaitSync(t->gl.fence, 0, GL_TIMEOUT_IGNORED);
                    glDeleteSync(t->gl.fence);
                    t->gl.fence = nullptr;
                }
                gld->bindTexture(tmu, t);
                gld->bindSampler(tmu, s);
            }
        }
    }
    CHECK_GL_ERROR(utils::slog.e)
}

UTILS_NOINLINE
void logCompilationError(io::ostream& out, ShaderStage shaderType,
        const char* name, GLuint shaderId, CString const& sourceCode) noexcept {

    auto to_string = [](ShaderStage type) -> const char* {
        switch (type) {
            case ShaderStage::VERTEX:   return "vertex";
            case ShaderStage::FRAGMENT: return "fragment";
            case ShaderStage::COMPUTE:  return "compute";
        }
    };

    { // scope for the temporary string storage
        GLint length = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

        CString infoLog(length);
        glGetShaderInfoLog(shaderId, length, nullptr, infoLog.data());

        out << "Compilation error in " << to_string(shaderType) << " shader \"" << name << "\":\n"
            << "\"" << infoLog.c_str() << "\""
            << io::endl;
    }

#ifndef NDEBUG
    std::string_view shader{ sourceCode.data(), sourceCode.size() };
    size_t lc = 1;
    size_t start = 0;
    std::string line;
    while (true) {
        size_t end = shader.find('\n', start);
        if (end == std::string::npos) {
            line = shader.substr(start);
        } else {
            line = shader.substr(start, end - start);
        }
        out << lc++ << ":   " << line.c_str() << '\n';
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    out << io::endl;
#endif
}

UTILS_NOINLINE
void logProgramLinkError(io::ostream& out, char const* name, GLuint program) noexcept {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    CString infoLog(length);
    glGetProgramInfoLog(program, length, nullptr, infoLog.data());

    out << "Link error in \"" << name << "\":\n"
        << "\"" << infoLog.c_str() << "\""
        << io::endl;
}

} // namespace filament::backend
