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

#ifndef TNT_FILAMENT_MATERIALPROGRAMS_H
#define TNT_FILAMENT_MATERIALPROGRAMS_H

#include "MaterialDefinition.h"

#include <backend/Handle.h>

#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>
#include <backend/DriverApiForward.h>
#include <backend/Program.h>

namespace filament {

class FMaterial;

// L0 cache for material programs. Manages recompiling them on-demand; owned by Material and
// MaterialInstance.
class MaterialPrograms {
public:
    MaterialPrograms(FEngine& engine, FMaterial const& material,
            utils::FixedCapacityVector<backend::Program::SpecializationConstant>
                    specializationConstants);

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
    backend::Handle<backend::HwProgram> getProgram(Variant const variant) const noexcept {
        backend::Handle<backend::HwProgram> program = mCachedPrograms[variant.key];
        assert_invariant(program);
        return program;
    }

    utils::Slice<const backend::Program::SpecializationConstant>
            getSpecializationConstants() const noexcept {
        return mSpecializationConstants;
    }

    void terminate(FEngine& engine);

    void release(FEngine& engine);

private:
    backend::Handle<backend::HwProgram> prepareProgramSlow(backend::DriverApi& driver,
            Variant const variant,
            backend::CompilerPriorityQueue const priorityQueue) const noexcept;

    ProgramSpecialization getProgramSpecialization(Variant variant) const noexcept;

    FMaterial const& mMaterial;
    mutable utils::FixedCapacityVector<backend::Handle<backend::HwProgram>> mCachedPrograms;
    utils::Slice<const backend::Program::SpecializationConstant> mSpecializationConstants;
};

} // namespace filament

#endif  // TNT_FILAMENT_MATERIALPROGRAMS_H
