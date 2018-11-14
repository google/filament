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

#ifndef TNT_FILAMENT_DRIVER_PROGRAM_H
#define TNT_FILAMENT_DRIVER_PROGRAM_H

#include <filament/EngineEnums.h>
#include <filament/SamplerBindingMap.h>

#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/Log.h>

#include <array>

namespace filament {

class Program {
public:

    static constexpr size_t NUM_SHADER_TYPES = 2;
    static constexpr size_t NUM_UNIFORM_BINDINGS = filament::BindingPoints::COUNT;
    static constexpr size_t NUM_SAMPLER_BINDINGS = filament::BindingPoints::COUNT;

    enum class Shader : uint8_t {
        VERTEX = 0,
        FRAGMENT = 1
    };

    Program() noexcept;
    Program(const Program& rhs);
    Program(Program&& rhs) noexcept;
    Program& operator=(const Program& rhs);
    Program& operator=(Program&& rhs) noexcept;
    ~Program() noexcept;

    // sets the material name and variant for diagnostic purposes only
    Program& diagnostics(utils::CString const& name, uint8_t variantKey = 0);
    Program& diagnostics(utils::CString&& name, uint8_t variantKey = 0) noexcept;

    // sets one of the program's shader (e.g. vertex, fragment)
    Program& shader(Shader shader, utils::CString const& source);
    Program& shader(Shader shader, utils::CString&& source) noexcept;

    // sets a uniform interface block for this program
    Program& addUniformBlock(size_t index, const UniformInterfaceBlock* ib);

    // sets a sampler interface block for this program
    Program& addSamplerBlock(size_t index, const SamplerInterfaceBlock* ib);

    template <typename T>
    Program& withVertexShader(T&& source) {
        return shader(Shader::VERTEX, std::forward<T>(source));
    }

    template <typename T>
    Program& withFragmentShader(T&& source) {
        return shader(Shader::FRAGMENT, std::forward<T>(source));
    }

    // sets up sampler bindings for this program
    Program& withSamplerBindings(const SamplerBindingMap* bindings);

    std::array<utils::CString, NUM_SHADER_TYPES> const& getShadersSource() const noexcept {
        return mShadersSource;
    }

    std::array<UniformInterfaceBlock const*, NUM_UNIFORM_BINDINGS> const&
    getUniformInterfaceBlocks() const noexcept {
        return mUniformInterfaceBlocks;
    }

    std::array<SamplerInterfaceBlock const*, NUM_SAMPLER_BINDINGS> const&
    getSamplerInterfaceBlocks() const noexcept {
        return mSamplerInterfaceBlocks;
    }

    const SamplerBindingMap* getSamplerBindings() const noexcept {
        return mSamplerBindings;
    }

    const utils::CString& getName() const noexcept {
        return mName;
    }

    uint8_t getVariant() const noexcept {
        return mVariant;
    }

    bool hasSamplers() const noexcept {
        return mSamplerCount > 0;
    }

private:
#if !defined(NDEBUG)
    friend utils::io::ostream& operator<< (utils::io::ostream& out, const Program& builder);
#endif

    std::array<UniformInterfaceBlock const *, NUM_UNIFORM_BINDINGS> mUniformInterfaceBlocks;
    std::array<SamplerInterfaceBlock const *, NUM_SAMPLER_BINDINGS> mSamplerInterfaceBlocks;
    const SamplerBindingMap* mSamplerBindings = nullptr;
    std::array<utils::CString, NUM_SHADER_TYPES> mShadersSource;
    size_t mSamplerCount = 0;
    utils::CString mName{"unknown"};
    uint8_t mVariant;
};

} // namespace filament;

#endif // TNT_FILAMENT_DRIVER_PROGRAM_H
