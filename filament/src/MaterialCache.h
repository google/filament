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

#include "MaterialDefinition.h"
#include "ProgramSpecialization.h"

#include <private/filament/Variant.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>
#include <utils/InternPool.h>
#include <utils/Invocable.h>
#include <utils/RefCountedMap.h>

namespace filament {

class Material;

class MaterialCache {
    // A newtype around a material parser used as a key for the material cache. The material file's
    // CRC32 is used as the hash function.
    struct MaterialKey {
        struct Hash {
            size_t operator()(MaterialKey const& x) const noexcept;
        };
        bool operator==(MaterialKey const& rhs) const noexcept;

        MaterialParser const* UTILS_NONNULL parser;
    };

public:
    using SpecializationConstantInternPool =
            utils::InternPool<backend::Program::SpecializationConstant>;

    using ProgramCache =
            utils::RefCountedMap<ProgramSpecialization, backend::Handle<backend::HwProgram>>;

    ~MaterialCache();

    SpecializationConstantInternPool& getSpecializationConstantsInternPool() {
        return mSpecializationConstantsInternPool;
    }

    ProgramCache& getProgramCache() {
        return mPrograms;
    }

    // Acquire or create a new entry in the cache for the given material data.
    MaterialDefinition* UTILS_NULLABLE acquireMaterial(FEngine& engine, const void* UTILS_NONNULL data,
            size_t size) noexcept;

    // Release an entry in the cache, potentially freeing its GPU resources.
    void releaseMaterial(FEngine& engine, MaterialDefinition const& definition) noexcept;

private:
    // TODO: investigate using custom allocators for the below data structures?

    // We use unique_ptr here because we need these pointers to be stable.
    utils::RefCountedMap<MaterialKey, std::unique_ptr<MaterialDefinition>, MaterialKey::Hash>
            mDefinitions;

    utils::RefCountedMap<ProgramSpecialization, backend::Handle<backend::HwProgram>> mPrograms;

    utils::InternPool<backend::Program::SpecializationConstant> mSpecializationConstantsInternPool;
};

} // namespace filament

#endif  // TNT_FILAMENT_MATERIALCACHE_H
