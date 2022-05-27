/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef UBERZ_ARCHIVE_CACHE_H
#define UBERZ_ARCHIVE_CACHE_H

#include <filament/Engine.h>
#include <filament/Material.h>

#include <tsl/robin_map.h>

#include <uberz/ReadableArchive.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

namespace filament::uberz {

    struct ArchiveRequirements;

    // Stores a set of Filament materials and knows how to choose a suitable material when given a
    // set of requirements. Used by gltfio; users do not need to access this class directly.
    class ArchiveCache {
    public:
        ArchiveCache(filament::Engine& engine) : mEngine(engine) {}
        ~ArchiveCache();

        void load(const void* archiveData, uint64_t archiveByteCount);
        filament::Material* getMaterial(const ArchiveRequirements& requirements);
        filament::Material* getDefaultMaterial();
        const filament::Material* const* getMaterials() const noexcept { return mMaterials.data(); }
        size_t getMaterialsCount() const noexcept { return mMaterials.size(); }
        void destroyMaterials();

    private:
        filament::Engine& mEngine;
        utils::FixedCapacityVector<filament::Material*> mMaterials;
        struct ReadableArchive* mArchive = nullptr;
    };

    struct ArchiveRequirements {
        filament::Shading shadingModel;
        filament::BlendingMode blendingMode;
        tsl::robin_map<utils::CString, bool, utils::CString::Hasher> features;
    };

} // namespace filament::uberz

#endif // UBERZ_ARCHIVE_CACHE_H
