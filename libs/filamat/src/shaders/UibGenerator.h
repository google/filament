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

#ifndef TNT_FILAMAT_UIBGENERATOR_H
#define TNT_FILAMAT_UIBGENERATOR_H

#include <backend/DriverEnums.h>

#include <utils/BitmaskEnum.h>

#include <type_traits>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class BufferInterfaceBlock;

class UibGenerator {
public:
    // tag to represent a generated ubo.
    enum class Ubo : uint8_t {
        FrameUniforms,              // uniforms updated per view
        ObjectUniforms,             // uniforms updated per renderable
        BonesUniforms,              // bones data, per renderable
        MorphingUniforms,           // morphing uniform/sampler updated per render primitive
        LightsUniforms,             // lights data array
        ShadowUniforms,             // punctual shadow data
        FroxelRecordUniforms,       // froxel records
        FroxelsUniforms,            // froxels
        MaterialParams,             // material instance ubo
        // Update utils::Enum::count<>() below when adding values here
        // These are limited by CONFIG_BINDING_COUNT (currently 10)
        // When adding an UBO here, make sure to also update
        //      MaterialBuilder::writeCommonChunks() if needed
    };

    struct Binding {
        backend::descriptor_set_t set;
        backend::descriptor_binding_t binding;
    };

    // return the BufferInterfaceBlock for the given UBO tag
    static BufferInterfaceBlock const& get(Ubo ubo) noexcept;

    // return the {set, binding } for the given UBO tag
    static Binding getBinding(Ubo ubo) noexcept;

    // deprecate these...
    static BufferInterfaceBlock const& getPerViewUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableUib() noexcept;
    static BufferInterfaceBlock const& getLightsUib() noexcept;
    static BufferInterfaceBlock const& getShadowUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableBonesUib() noexcept;
    static BufferInterfaceBlock const& getPerRenderableMorphingUib() noexcept;
    static BufferInterfaceBlock const& getFroxelRecordUib() noexcept;
    static BufferInterfaceBlock const& getFroxelsUib() noexcept;
};

} // namespace filament

template<>
struct utils::EnableIntegerOperators<filament::UibGenerator::Ubo> : public std::true_type {};

template<>
inline constexpr size_t utils::Enum::count<filament::UibGenerator::Ubo>() { return 9; }


#endif // TNT_FILAMAT_UIBGENERATOR_H
