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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_PROGRAM_H
#define TNT_FILAMENT_BACKEND_PRIVATE_PROGRAM_H

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <backend/DriverEnums.h>

#include <array>

namespace filament::backend {

class Program {
public:

    static constexpr size_t SHADER_TYPE_COUNT = 2;
    static constexpr size_t BINDING_COUNT = CONFIG_BINDING_COUNT;

    enum class Shader : uint8_t {
        VERTEX = 0,
        FRAGMENT = 1
    };

    struct Sampler {
        utils::CString name = {};   // name of the sampler in the shader
        uint16_t binding = 0;       // binding point of the sampler in the shader
        bool strict = false;        // if true, this sampler must always have a bound texture
    };

    struct SamplerGroupData {
        utils::FixedCapacityVector<Sampler> samplers;
        ShaderStageFlags stageFlags = ALL_SHADER_STAGE_FLAGS;
    };

    using SamplerGroupInfo = std::array<SamplerGroupData, BINDING_COUNT>;
    using UniformBlockInfo = std::array<utils::CString, BINDING_COUNT>;

    Program() noexcept;
    Program(const Program& rhs) = delete;
    Program& operator=(const Program& rhs) = delete;
    Program(Program&& rhs) noexcept;
    Program& operator=(Program&& rhs) noexcept;
    ~Program() noexcept;

    // sets the material name and variant for diagnostic purposes only
    Program& diagnostics(utils::CString const& name,
            utils::Invocable<utils::io::ostream&(utils::io::ostream& out)>&& logger);

    // sets one of the program's shader (e.g. vertex, fragment)
    Program& shader(Shader shader, void const* data, size_t size) noexcept;

    // sets the 'bindingPoint' uniform block's name for this program.
    //
    // Note: This is only needed for GLES3.0 backends, because the layout(binding=) syntax is
    //       not permitted in glsl. The backend needs a way to associate a uniform block
    //       to a binding point.
    //
    Program& setUniformBlock(size_t bindingPoint, utils::CString uniformBlockName) noexcept;

    // sets the 'bindingPoint' sampler group descriptor for this program.
    // 'samplers' can be destroyed after this call.
    // This effectively associates a set of (BindingPoints, index) to a texture unit in the shader.
    // Or more precisely, what layout(binding=) is set to in GLSL.
    Program& setSamplerGroup(size_t bindingPoint, ShaderStageFlags stageFlags,
            Sampler const* samplers, size_t count) noexcept;

    Program& withVertexShader(void const* data, size_t size) {
        return shader(Shader::VERTEX, data, size);
    }

    Program& withFragmentShader(void const* data, size_t size) {
        return shader(Shader::FRAGMENT, data, size);
    }

    using ShaderBlob = utils::FixedCapacityVector<uint8_t>;
    using ShaderSource = std::array<ShaderBlob, SHADER_TYPE_COUNT>;
    ShaderSource const& getShadersSource() const noexcept { return mShadersSource; }
    ShaderSource& getShadersSource() noexcept { return mShadersSource; }

    UniformBlockInfo const& getUniformBlockInfo() const noexcept { return mUniformBlocks; }
    UniformBlockInfo& getUniformBlockInfo() noexcept { return mUniformBlocks; }

    SamplerGroupInfo const& getSamplerGroupInfo() const { return mSamplerGroups; }
    SamplerGroupInfo& getSamplerGroupInfo() { return mSamplerGroups; }

    const utils::CString& getName() const noexcept { return mName; }

    bool hasSamplers() const noexcept { return mHasSamplers; }

private:
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const Program& builder);

    UniformBlockInfo mUniformBlocks = {};
    SamplerGroupInfo mSamplerGroups = {};
    ShaderSource mShadersSource;
    bool mHasSamplers = false;
    utils::CString mName;
    utils::Invocable<utils::io::ostream&(utils::io::ostream& out)> mLogger;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_PROGRAM_H
