/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_LOCALPROGRAMCACHE_H
#define TNT_FILAMENT_LOCALPROGRAMCACHE_H

#include "MaterialDefinition.h"

#include <backend/Handle.h>

#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>
#include <backend/DriverApiForward.h>
#include <backend/Program.h>

#include <utility>

namespace filament {

class FMaterial;

// L0 cache for material programs. Manages recompiling them on-demand; owned by Material and
// MaterialInstance.
class LocalProgramCache {
    template<typename T>
    using is_supported_constant_parameter_t =
            std::enable_if_t<std::is_same_v<int32_t, T> || std::is_same_v<float, T> ||
                             std::is_same_v<bool, T>>;

public:
    using Programs = utils::Slice<const backend::Handle<backend::HwProgram>>;
    using SpecializationConstants = utils::Slice<const backend::Program::SpecializationConstant>;

    LocalProgramCache() = default;
    LocalProgramCache(LocalProgramCache const& other);

    LocalProgramCache& operator=(LocalProgramCache const& other);

    // Initialize for use in a Material.
    void initializeForMaterial(FEngine& engine, FMaterial const& material,
            utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                    specializationConstants);

    // Initialize for use in a MaterialInstance. Copies the set of spec constants currently in use
    // from its Material.
    void initializeForMaterialInstance(FEngine& engine, FMaterial const& material);

    bool isInitialized() const noexcept { return mMaterial != nullptr; }

    // prepareProgram creates the program for the material's given variant at the backend level.
    // Must be called outside of backend render pass.
    // Must be called before getProgram() below.
    backend::Handle<backend::HwProgram> prepareProgram(backend::DriverApi& driver,
            Variant const variant,
            backend::CompilerPriorityQueue const priorityQueue) const noexcept {
        backend::Handle<backend::HwProgram> program = mCachedPrograms[variant.key];
        if (UTILS_LIKELY(program)) {
            return program;
        }
        return prepareProgramSlow(driver, variant, priorityQueue);
    }

    // getProgram returns the backend program for the material's given variant.
    // Must be called after prepareProgram().
    [[nodiscard]]
    backend::Handle<backend::HwProgram> getProgram(Variant variant) const noexcept {
        variant = filterVariantForGetProgram(variant);
        backend::Handle<backend::HwProgram> program = mCachedPrograms[variant.key];
        assert_invariant(program);
        return program;
    }

    SpecializationConstants getSpecializationConstants() const noexcept {
        return mSpecializationConstants;
    }

    Programs getPrograms() const noexcept { return mCachedPrograms.as_slice(); }

    // Free all engine resources associated with this instance.
    void terminate(FEngine& engine);

    // Clear all cached programs. Used primarily by matdbg to "sever" a Material's connection to the
    // global material cache.
    void clear(FEngine& engine);

    // Get constant by ID.
    template<typename T, typename = is_supported_constant_parameter_t<T>>
    T getConstant(uint32_t id) const noexcept {
        return std::get<T>(getConstantImpl(id));
    }

    // Get constant by name.
    template<typename T, typename = is_supported_constant_parameter_t<T>>
    T getConstant(std::string_view name) const noexcept {
        return std::get<T>(getConstantImpl(name));
    }

    // Set constants by ID.
    void setConstants(
            std::initializer_list<std::pair<uint32_t, backend::Program::SpecializationConstant>>
                    constants) noexcept;

    // Set constants by name.
    void setConstants(std::initializer_list<
            std::pair<std::string_view, backend::Program::SpecializationConstant>>
                    constants) noexcept;

    // Set constants list directly.
    void setConstants(utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                    constants) noexcept;

private:
    // Apply any pending specialization constants. Invalidates programs as necessary.
    void flushConstants() const;

    backend::Handle<backend::HwProgram> prepareProgramSlow(backend::DriverApi& driver,
            Variant const variant,
            backend::CompilerPriorityQueue const priorityQueue) const noexcept;

    ProgramSpecialization getProgramSpecialization(Variant variant) const noexcept;

    Variant filterVariantForGetProgram(Variant const variant) const noexcept;

    void setConstantsImpl(utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                    constants) noexcept;

    backend::Program::SpecializationConstant getConstantImpl(uint32_t id) const noexcept;
    backend::Program::SpecializationConstant getConstantImpl(std::string_view name) const noexcept;

    FMaterial const* mMaterial = nullptr;
    mutable utils::FixedCapacityVector<backend::Handle<backend::HwProgram>> mCachedPrograms;
    SpecializationConstants mSpecializationConstants;
};

} // namespace filament

#endif  // TNT_FILAMENT_LOCALPROGRAMCACHE_H
