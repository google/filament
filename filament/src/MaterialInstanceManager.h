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

#pragma once

#include <utils/bitset.h>

#include <vector>

namespace filament {

class FMaterial;
class FMaterialInstance;
class FEngine;

// This class manages a cache of material instances mapped by materials. Having a cache allows us to
// re-use instances across frames.
class MaterialInstanceManager {
public:
    class Record {
    public:
        Record(FMaterial const* material)
            : mMaterial(material) {}
        ~Record() = default;

        Record(Record const& rhs) noexcept;
        Record& operator=(Record const& rhs) noexcept;
        Record(Record&& rhs) noexcept;
        Record& operator=(Record&& rhs) noexcept;

        void terminate(FEngine& engine);
        void reset() { mAvailable = utils::bitset32{ (1 << (mInstances.size() + 1)) - 1 }; }
        FMaterialInstance* getInstance();

    private:
        FMaterial const* mMaterial = nullptr;
        std::vector<FMaterialInstance*> mInstances;
        utils::bitset32 mAvailable = {};

        friend class MaterialInstanceManager;
    };

    MaterialInstanceManager() noexcept;
    MaterialInstanceManager(MaterialInstanceManager const& rhs) noexcept;
    MaterialInstanceManager(MaterialInstanceManager&& rhs) noexcept;
    MaterialInstanceManager& operator=(MaterialInstanceManager const& rhs) noexcept;
    MaterialInstanceManager& operator=(MaterialInstanceManager&& rhs) noexcept;    

    ~MaterialInstanceManager();

    /*
     * Destroy all of the cached material instances. This needs to be done before the destruction of
     * the corresponding Material.
     */
    void terminate(FEngine& engine);

    /*
     * This returns a material instance given a material. The implementation will try to find an
     * available instance in the cache. If one is not found, then a new instance will be created and
     * added to the cache.
     */
    FMaterialInstance* getMaterialInstance(FMaterial const* ma);


    /*
     * Marks all of the material instances as unused. Typically, you'd call this at the beginning of
     * a frame.
     */
    void reset() {
        std::for_each(mMaterials.begin(), mMaterials.end(), [](auto& record) { record.reset(); });
    }

private:
    std::vector<Record> mMaterials;
};

} // namespace filament
