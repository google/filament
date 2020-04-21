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

#include "DriverBase.h"
#include "OpenGLDriver.h"

#include "private/backend/Driver.h"
#include "private/backend/Program.h"

#include <utils/compiler.h>
#include <utils/Log.h>

#include <vector>

#include <stddef.h>
#include <stdint.h>


namespace filament {

class OpenGLProgram : public backend::HwProgram {
public:

    OpenGLProgram() noexcept = default;
    OpenGLProgram(OpenGLDriver* gl, const backend::Program& builder) noexcept;
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
        GLuint shaders[backend::Program::SHADER_TYPE_COUNT];
        GLuint program;
    } gl; // 12 bytes

    static void logCompilationError(utils::io::ostream& out, GLuint shaderId, char const* source) noexcept;

private:
    static constexpr uint8_t TEXTURE_UNIT_COUNT = OpenGLContext::MAX_TEXTURE_UNIT_COUNT;
    static constexpr uint8_t VERTEX_SHADER_BIT   = uint8_t(1) << size_t(backend::Program::Shader::VERTEX);
    static constexpr uint8_t FRAGMENT_SHADER_BIT = uint8_t(1) << size_t(backend::Program::Shader::FRAGMENT);

    struct BlockInfo {
        uint8_t binding : 3;    // binding (i.e.: index in mSamplerBindings)
        uint8_t unused  : 1;    // padding / available
        uint8_t count   : 4;    // number of TMUs actually used minus 1

        // if TEXTURE_UNIT_COUNT > 16, the count bitfield must be increased accordingly
        static_assert(TEXTURE_UNIT_COUNT <= 16, "TEXTURE_UNIT_COUNT must be <= 16");

        // if SAMPLER_BINDING_COUNT > 8, the binding bitfield must be increased accordingly
        static_assert(backend::Program::SAMPLER_BINDING_COUNT <= 8, "SAMPLER_BINDING_COUNT must be <= 8");
    };

    uint8_t mUsedBindingsCount = 0;
    uint8_t mValidShaderSet = 0;
    bool mIsValid = false;

    // information about each USED sampler buffer (no gaps)
    std::array<BlockInfo, backend::Program::SAMPLER_BINDING_COUNT> mBlockInfos;   // 8 bytes

    // runs of indices into SamplerGroup -- run start index and size given by BlockInfo
    std::array<uint8_t, TEXTURE_UNIT_COUNT> mIndicesRuns;    // 16 bytes

    void updateSamplers(OpenGLDriver* gl) noexcept;
};


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGLPROGRAM_H
