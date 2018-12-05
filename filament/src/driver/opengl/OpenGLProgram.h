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

#ifndef TNT_FILAMENT_DRIVER_OPENGLPROGRAM_H
#define TNT_FILAMENT_DRIVER_OPENGLPROGRAM_H

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include <utils/compiler.h>
#include <utils/Log.h>

#include "driver/opengl/gl_headers.h"
#include "driver/Driver.h"
#include "driver/DriverBase.h"
#include "driver/Program.h"
#include "driver/opengl/OpenGLDriver.h"

namespace filament {

class OpenGLProgram : public HwProgram {
public:

    OpenGLProgram(OpenGLDriver* gl, const Program& builder) noexcept;
    ~OpenGLProgram() noexcept;

    bool isValid() const noexcept { return mIsValid; }

    void use(OpenGLDriver* const gl) noexcept {
        if (UTILS_UNLIKELY(mUsedBindingsCount)) {
            // We rely on GL state tracking to avoid unnecessary glBindTexture / glBindSampler
            // calls.

            // we need to do this if:
            // - the content of mSamplerBindings has changed
            // - the content of any bound sampler buffer has changed
            // ... since last time we used this program

            // turns out the former might be relatively cheap to check, the later requires
            // a bit less. Compared to what updateSamplers() actually does, which is
            // pretty little, I'm not sure we'll get ahead.

            updateSamplers(gl);
        }
    }

    struct {
        GLuint shaders[Program::NUM_SHADER_TYPES];
        GLuint program;
    } gl; // 12 bytes

    static void logCompilationError(utils::io::ostream& out, GLuint shaderId, char const* source) noexcept;

private:
    static constexpr uint8_t NUM_TEXTURE_UNITS = OpenGLDriver::MAX_TEXTURE_UNITS;
    static constexpr uint8_t VERTEX_SHADER_BIT   = uint8_t(1) << size_t(Program::Shader::VERTEX);
    static constexpr uint8_t FRAGMENT_SHADER_BIT = uint8_t(1) << size_t(Program::Shader::FRAGMENT);

    struct BlockInfo {
        uint8_t binding : 3;    // binding (i.e.: index in mSamplerBindings)
        uint8_t unused  : 1;    // padding / available
        uint8_t count   : 4;    // number of TMUs actually used minus 1

        // if NUM_TEXTURE_UNITS > 16, the count bitfield must be increased accordingly
        static_assert(NUM_TEXTURE_UNITS <= 16, "NUM_TEXTURE_UNITS must be <= 16");

        // if NUM_SAMPLER_BINDINGS > 8, the binding bitfield must be increased accordingly
        static_assert(Program::NUM_SAMPLER_BINDINGS <= 8, "NUM_SAMPLER_BINDINGS must be <= 8");
    };

    uint8_t mUsedBindingsCount = 0;
    uint8_t mValidShaderSet = 0;
    bool mIsValid = false;

    // information about each USED sampler buffer (no gaps)
    std::array<BlockInfo, Program::NUM_SAMPLER_BINDINGS> mBlockInfos;   // 8 bytes

    // runs of indices into SamplerBuffer -- run start index and size given by BlockInfo
    std::array<uint8_t, NUM_TEXTURE_UNITS> mIndicesRuns;    // 16 bytes

    void updateSamplers(OpenGLDriver* gl) noexcept;
};


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGLPROGRAM_H
