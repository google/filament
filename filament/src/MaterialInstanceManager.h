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

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <utils/Hash.h>

namespace filament {

class FMaterial;
class FMaterialInstance;
class FEngine;

// This class manages a cache of material instances mapped by materials. Having a cache allows us to
// re-use instances across frames.
class MaterialInstanceManager {
public:
    MaterialInstanceManager() noexcept;
    MaterialInstanceManager(MaterialInstanceManager const& rhs) = delete;
    MaterialInstanceManager(MaterialInstanceManager&& rhs) noexcept;
    MaterialInstanceManager& operator=(MaterialInstanceManager const& rhs) = delete;
    MaterialInstanceManager& operator=(MaterialInstanceManager&& rhs) noexcept;

    ~MaterialInstanceManager();

    /*
     * Destroy all the cached material instances. This needs to be done before the destruction of
     * the corresponding Material.
     */
    void terminate(FEngine& engine);

    /**
     * Resets the anonymous material instances cache.
     */
    void reset();

    /**
     * This returns a material instance given a material and a tag.
     *
     * If the material instance doesn't exist in the cache, it is created and cached.
     *
     * @param ma    FMaterial to get a MaterialInstance for
     * @param tag   A unique tag identifying the MaterialInstance
     * @return      A FMaterialInstance pointer
     */
    FMaterialInstance* getMaterialInstance(FMaterial const* ma, uint32_t tag) const;

    /**
     * This returns a material instance given a material from a cache.
     *
     * If the material instance doesn't exist in the cache, it is created and cached.
     *
     * It is permissible to call the method several times, in which case a different MaterialInstance will be returned.
     * It is guaranteed to be different from MaterialInstances returned with a tag.
     *
     * @param ma    FMaterial to get a MaterialInstance for
     * @return      A FMaterialInstance pointer
     */
    FMaterialInstance* getMaterialInstance(FMaterial const* ma) const;

private:
    struct Key {
        FMaterial const* material;
        uint32_t tag;
        bool operator==(Key const& rhs) const noexcept {
            return material == rhs.material && tag == rhs.tag;
        }
    };

    struct Hasher {
        std::size_t operator()(Key const& key) const noexcept {
            std::size_t seed = 0;
            utils::hash::combine(seed, key.material);
            utils::hash::combine(seed, key.tag);
            return seed;
        }
    };

    struct AnonymousPool {
        std::vector<FMaterialInstance*> instances;
        uint32_t highWaterMark = 0;
    };

    mutable std::unordered_map<Key, FMaterialInstance*, Hasher> mMaterialInstances;
    mutable std::unordered_map<FMaterial const*, AnonymousPool> mAnonymousMaterialInstances;
};

} // namespace filament
