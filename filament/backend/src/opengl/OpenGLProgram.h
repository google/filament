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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGLPROGRAM_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGLPROGRAM_H

#include "DriverBase.h"
#include "OpenGLDriver.h"

#include "private/backend/Driver.h"
#include "backend/Program.h"

#include <utils/compiler.h>
#include <utils/Log.h>

#include <vector>

#include <stddef.h>
#include <stdint.h>


namespace filament::backend {

class OpenGLProgram : public HwProgram {
public:

    OpenGLProgram() noexcept;
    OpenGLProgram(OpenGLDriver& gld, Program&& builder) noexcept;
    ~OpenGLProgram() noexcept;

    bool isValid() const noexcept { return mValid; }

    void use(OpenGLDriver* const gld, OpenGLContext& context) noexcept {
        if (UTILS_UNLIKELY(!mInitialized)) {
            initialize(context);
        }

        context.useProgram(gl.program);
        if (UTILS_UNLIKELY(mUsedBindingsCount)) {
            // We rely on GL state tracking to avoid unnecessary glBindTexture / glBindSampler
            // calls.

            // we need to do this if:
            // - the content of mSamplerBindings has changed
            // - the content of any bound sampler buffer has changed
            // ... since last time we used this program

            // turns out the former might be relatively cheap to check, the later requires
            // a bit less. Compared to what updateSamplers() actually does, which is
            // pretty little, I'm not sure if we'll get ahead.

            updateSamplers(gld);
        }
    }

    struct {
        GLuint shaders[Program::SHADER_TYPE_COUNT] = {};
        GLuint program = 0;
    } gl; // 12 bytes

private:
    // keep these away from of other class attributes
    struct LazyInitializationData {
        Program::UniformBlockInfo uniformBlockInfo;
        Program::SamplerGroupInfo samplerGroupInfo;
        std::array<utils::CString, Program::SHADER_TYPE_COUNT> shaderSourceCode;
    };

    static void compileShaders(OpenGLContext& context,
            Program::ShaderSource shadersSource,
            utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
            GLuint shaderIds[Program::SHADER_TYPE_COUNT],
            std::array<utils::CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept;

    static std::string_view process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
            char* source, size_t len) noexcept;

    static std::string_view process_ARB_shading_language_packing(OpenGLContext& context) noexcept;

    static std::array<std::string_view, 2> splitShaderSource(std::string_view source) noexcept;

    static GLuint linkProgram(const GLuint shaderIds[Program::SHADER_TYPE_COUNT]) noexcept;

    static bool checkProgramStatus(const char* name,
            GLuint& program, GLuint shaderIds[Program::SHADER_TYPE_COUNT],
            std::array<utils::CString, Program::SHADER_TYPE_COUNT> const& shaderSourceCode) noexcept;

    void initialize(OpenGLContext& context);

    void initializeProgramState(OpenGLContext& context, GLuint program,
            LazyInitializationData const& lazyInitializationData) noexcept;

    void updateSamplers(OpenGLDriver* gld) const noexcept;

    // number of bindings actually used by this program
    uint8_t mUsedBindingsCount = 0u;
    // whether lazy initialization has been performed
    bool mInitialized : 1;
    // whether lazy initialization was successful
    bool mValid : 1;
    UTILS_UNUSED uint8_t padding[2] = {};

    union {
        // when mInitialized == true:
        // information about each USED sampler buffer per binding (no gaps)
        std::array<uint8_t, Program::SAMPLER_BINDING_COUNT> mUsedSamplerBindingPoints;   // 4 bytes
        // when mInitialized == false:
        // lazy initialization data pointer
        LazyInitializationData* mLazyInitializationData;
    };
};

// if OpenGLProgram is larger tha 64 bytes, it'll fall in a larger Handle bucket.
static_assert(sizeof(OpenGLProgram) <= 64);

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGLPROGRAM_H
