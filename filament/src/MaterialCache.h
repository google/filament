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

#include "RefCountedMap.h"
#include "MaterialDefinition.h"

#include <private/filament/Variant.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Program.h>

#include <utils/Invocable.h>

namespace filament {

class Material;

/** A cache of all materials and shader programs compiled by Filament. */
class MaterialCache {
    // Program caches are keyed by the material UUID.
    //
    // We use unique_ptr here because we need these pointers to be stable.
    // TODO: investigate using a custom allocator here?
    using Inner = RefCountedMap<uint64_t, std::unique_ptr<MaterialDefinition>>;

public:
    ~MaterialCache();

    /** Acquire or create a new entry in the cache for the given material data. */
    MaterialDefinition* UTILS_NULLABLE acquire(FEngine& engine, const void* UTILS_NONNULL data,
            size_t size) noexcept;

    /** Release an entry in the cache, potentially freeing its GPU resources. */
    void release(FEngine& engine, uint64_t uuid) noexcept;

private:
    Inner mInner;
};

} // namespace filament


#endif  // TNT_FILAMENT_MATERIALCACHE_H
