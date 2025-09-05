/*
 * Copyright (C) 2025 The Android Open Source Project
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
#ifndef TNT_FILAMENT_MATERIALCACHE_H
#define TNT_FILAMENT_MATERIALCACHE_H

#include "Cache.h"

#include <private/filament/Variant.h>

#include <backend/Handle.h>
#include <backend/Program.h>

namespace filament {

class Material;

class ProgramCache {
public:
    /*
     * A Specialization contains all specialization options which would result in a different
     * Program. In other words, a Program is a pure function of the fields of Specialization.
     */
    struct Specialization {
        Variant variant;
        utils::FixedCapacityVector<backend::Program::SpecializationConstant>
        specializationConstants;
        std::array<utils::FixedCapacityVector<backend::Program::PushConstant>,
                backend::Program::SHADER_TYPE_COUNT>
                pushConstants;

        bool operator==(Specialization const& rhs) const noexcept;

        struct Hash {
            size_t operator()(filament::ProgramCache::Specialization const& value) const noexcept;
        };
    };

    using Program = backend::Handle<backend::HwProgram>;

    // Programs are keyed by the specialization options of the program.
    using Inner = Cache<Specialization, Program, Specialization::Hash>;
    using Handle = Inner::Handle;
    using ReturnValue = Inner::ReturnValue;

    ProgramCache::ReturnValue hold(Specialization const& specialization, Material const& material,
            backend::CompilerPriorityQueue const priorityQueue) noexcept;

    Program& leak(Specialization const& specialization, Material const& material,
            backend::CompilerPriorityQueue const priorityQueue) noexcept;

    Program const& peek(Specialization const& specialization) const noexcept;

private:
    Inner mCache;
};

/*
 * A cache of all materials and shader programs compiled by Filament.
p */
class MaterialCache {
public:
    // Program caches are keyed by the material ID.
    using Inner = Cache<uint32_t, ProgramCache>;
    using Handle = Inner::Handle;
    using ReturnValue = Inner::ReturnValue;

    inline Handle get(uint32_t materialId) noexcept {
        return mCache.hold(materialId, []() {
            return ProgramCache();
        }).first;
    }

private:
    Inner mCache;
};

} // namespace filament


#endif  // TNT_FILAMENT_MATERIALCACHE_H
