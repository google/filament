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
            : mMaterial(material),
              mAvailable(0) {}
        ~Record() = default;

        Record(Record const& rhs) noexcept;
        Record& operator=(Record const& rhs) noexcept;
        Record(Record&& rhs) noexcept;
        Record& operator=(Record&& rhs) noexcept;

        void terminate(FEngine& engine);
        void reset() { mAvailable = 0; }
        std::pair<FMaterialInstance*, int32_t> getInstance();
        FMaterialInstance* getInstance(int32_t fixedInstanceindex);

    private:
        FMaterial const* mMaterial = nullptr;
        std::vector<FMaterialInstance*> mInstances;
        uint32_t mAvailable;

        friend class MaterialInstanceManager;
    };

    constexpr static int32_t INVALID_FIXED_INDEX = -1;

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
     * This returns a material instance given a material and an index. The `fixedIndex` should be
     * a value returned by getiFixedMaterialInstance.
     */
    FMaterialInstance* getMaterialInstance(FMaterial const* ma, int32_t const fixedIndex);

    /*
     * This returns a material instance and an index given a material. This is needed for the
     * case when two framegraph passes need to refer to the same material instance.
     * The returned index can be used with `getFixedMaterialInstance` to get a specific instance
     * of a material (and not a random entry in the record cache).
     */
    std::pair<FMaterialInstance*, int32_t> getFixedMaterialInstance(FMaterial const* ma);


    /*
     * Marks all of the material instances as unused. Typically, you'd call this at the beginning of
     * a frame.
     */
    void reset() {
        std::for_each(mMaterials.begin(), mMaterials.end(), [](auto& record) { record.reset(); });
    }

private:
    Record& getRecord(FMaterial const* material);

    std::vector<Record> mMaterials;
};

} // namespace filament
