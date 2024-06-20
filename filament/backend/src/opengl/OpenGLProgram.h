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

#include "OpenGLContext.h"
#include "ShaderCompilerService.h"

#include <private/backend/Driver.h>
#include <backend/Program.h>

#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Slice.h>

#include <array>
#include <limits>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class OpenGLDriver;

struct PushConstantBundle {
    utils::Slice<std::pair<GLint, ConstantType>> vertexConstants;
    utils::Slice<std::pair<GLint, ConstantType>> fragmentConstants;
};

class OpenGLProgram : public HwProgram {
public:

    OpenGLProgram() noexcept;
    OpenGLProgram(OpenGLDriver& gld, Program&& program) noexcept;
    ~OpenGLProgram() noexcept;

    bool isValid() const noexcept { return mToken || gl.program != 0; }

    bool use(OpenGLDriver* const gld, OpenGLContext& context) noexcept {
        // both non-null is impossible by construction
        assert_invariant(!mToken || !gl.program);

        if (UTILS_UNLIKELY(mToken && !gl.program)) {
            // first time a program is used
            initialize(*gld);
        }

        if (UTILS_UNLIKELY(!gl.program)) {
            // compilation failed (token should be null)
            assert_invariant(!mToken);
            return false;
        }

        context.useProgram(gl.program);
        if (UTILS_UNLIKELY(mUsedBindingsCount)) {
            // We rely on GL state tracking to avoid unnecessary glBindTexture / glBindSampler
            // calls.

            // we need to do this if:
            // - the content of mSamplerBindings has changed
            // - the content of any bound sampler buffer has changed
            // ... since last time we used this program

            // Turns out the former might be relatively cheap to check, the latter requires
            // a bit less. Compared to what updateSamplers() actually does, which is
            // pretty little, I'm not sure if we'll get ahead.

            updateSamplers(gld);
        }
        return true;
    }

    // For ES2 only
    void updateUniforms(uint32_t index, GLuint id, void const* buffer, uint16_t age) noexcept;
    void setRec709ColorSpace(bool rec709) const noexcept;

    struct {
        GLuint program = 0;
    } gl;                                               // 4 bytes

    PushConstantBundle getPushConstants() {
        auto fragBegin = mPushConstants.begin() + mPushConstantFragmentStageOffset;
        return {
            .vertexConstants = utils::Slice(mPushConstants.begin(), fragBegin),
            .fragmentConstants = utils::Slice(fragBegin, mPushConstants.end()),
        };
    }

private:
    // keep these away from of other class attributes
    struct LazyInitializationData;

    void initialize(OpenGLDriver& gld);

    void initializeProgramState(OpenGLContext& context, GLuint program,
            LazyInitializationData& lazyInitializationData) noexcept;

    void updateSamplers(OpenGLDriver* gld) const noexcept;

    // number of bindings actually used by this program
    std::array<uint8_t, Program::SAMPLER_BINDING_COUNT> mUsedSamplerBindingPoints;   // 4 bytes

    ShaderCompilerService::program_token_t mToken{};    // 16 bytes

    uint8_t mUsedBindingsCount = 0u;                    // 1 byte
    UTILS_UNUSED uint8_t padding[2] = {};               // 2 byte

    // Push constant array offset for fragment stage constants.
    uint8_t mPushConstantFragmentStageOffset = 0u;      // 1 byte

    // only needed for ES2
    GLint mRec709Location = -1;                         // 4 bytes

    using LocationInfo = utils::FixedCapacityVector<GLint>;
    struct UniformsRecord {
        Program::UniformInfo uniforms;
        LocationInfo locations;
        mutable GLuint id = 0;
        mutable uint16_t age = std::numeric_limits<uint16_t>::max();
    };
    UniformsRecord const* mUniformsRecords = nullptr;               // 8 bytes

    // Note that this can be replaced with a raw pointer and an uint8_t (for size) to reduce the
    // size of the container to 9 bytes if there is a need in the future.
    utils::FixedCapacityVector<std::pair<GLint, ConstantType>> mPushConstants;// 16 bytes
};

// if OpenGLProgram is larger tha 64 bytes, it'll fall in a larger Handle bucket.
static_assert(sizeof(OpenGLProgram) <= 64); // currently 64 bytes

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGLPROGRAM_H
