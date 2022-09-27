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
#include <variant>

namespace filament::backend {

class Program {
public:

    static constexpr size_t SHADER_TYPE_COUNT = 3;
    static constexpr size_t UNIFORM_BINDING_COUNT = CONFIG_UNIFORM_BINDING_COUNT;
    static constexpr size_t SAMPLER_BINDING_COUNT = CONFIG_SAMPLER_BINDING_COUNT;

    struct Sampler {
        utils::CString name = {};   // name of the sampler in the shader
        uint32_t binding = 0;       // binding point of the sampler in the shader
    };

    struct SamplerGroupData {
        utils::FixedCapacityVector<Sampler> samplers;
        ShaderStageFlags stageFlags = ShaderStageFlags::ALL_SHADER_STAGE_FLAGS;
    };

    using UniformBlockInfo = std::array<utils::CString, UNIFORM_BINDING_COUNT>;
    using SamplerGroupInfo = std::array<SamplerGroupData, SAMPLER_BINDING_COUNT>;
    using ShaderBlob = utils::FixedCapacityVector<uint8_t>;
    using ShaderSource = std::array<ShaderBlob, SHADER_TYPE_COUNT>;

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
    // string-based shaders are null terminated, consequently the size parameter must include the
    // null terminating character.
    Program& shader(ShaderStage shader, void const* data, size_t size);

    // Note: This is only needed for GLES3.0 backends, because the layout(binding=) syntax is
    //       not permitted in glsl. The backend needs a way to associate a uniform block
    //       to a binding point.
    Program& uniformBlockBindings(
            utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> const& uniformBlockBindings) noexcept;

    // sets the 'bindingPoint' sampler group descriptor for this program.
    // 'samplers' can be destroyed after this call.
    // This effectively associates a set of (BindingPoints, index) to a texture unit in the shader.
    // Or more precisely, what layout(binding=) is set to in GLSL.
    Program& setSamplerGroup(size_t bindingPoint, ShaderStageFlags stageFlags,
            Sampler const* samplers, size_t count) noexcept;

    struct SpecializationConstant {
        uint32_t id;                                // id set in glsl
        std::variant<int32_t, float, bool> value;   // value and type
    };

    Program& specializationConstants(
            utils::FixedCapacityVector<SpecializationConstant> specConstants) noexcept;


    ShaderSource const& getShadersSource() const noexcept { return mShadersSource; }
    ShaderSource& getShadersSource() noexcept { return mShadersSource; }

    UniformBlockInfo const& getUniformBlockBindings() const noexcept { return mUniformBlocks; }
    UniformBlockInfo& getUniformBlockBindings() noexcept { return mUniformBlocks; }

    SamplerGroupInfo const& getSamplerGroupInfo() const { return mSamplerGroups; }
    SamplerGroupInfo& getSamplerGroupInfo() { return mSamplerGroups; }

    utils::CString const& getName() const noexcept { return mName; }
    utils::CString& getName() noexcept { return mName; }

    utils::FixedCapacityVector<SpecializationConstant> const& getSpecializationConstants() const noexcept {
        return mSpecializationConstants;
    }
    utils::FixedCapacityVector<SpecializationConstant>& getSpecializationConstants() noexcept {
        return mSpecializationConstants;
    }

private:
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const Program& builder);

    UniformBlockInfo mUniformBlocks = {};
    SamplerGroupInfo mSamplerGroups = {};
    ShaderSource mShadersSource;
    utils::CString mName;
    utils::Invocable<utils::io::ostream&(utils::io::ostream& out)> mLogger;
    utils::FixedCapacityVector<SpecializationConstant> mSpecializationConstants;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_PROGRAM_H
