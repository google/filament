/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef GLTFIO_MATERIALGENERATOR_H
#define GLTFIO_MATERIALGENERATOR_H

#include <filament/Engine.h>
#include <filament/Material.h>

#include <utils/Hash.h>

#include <tsl/robin_map.h>

#include <array>

namespace gltfio {
namespace details {

enum class AlphaMode : uint8_t {
    OPAQUE,
    MASKED,
    TRANSPARENT
};

struct MaterialKey {
    bool doubleSided : 1;
    bool unlit : 1;
    bool hasVertexColors : 1;
    bool hasBaseColorTexture : 1;
    bool hasMetallicRoughnessTexture : 1;
    bool hasNormalTexture : 1;
    bool hasOcclusionTexture : 1;
    bool hasEmissiveTexture : 1;
    AlphaMode alphaMode;
    uint8_t baseColorUV;
    uint8_t metallicRoughnessUV;
    uint8_t emissiveUV;
    uint8_t aoUV;
    uint8_t normalUV;
    float alphaMaskThreshold;
    bool hasTextureTransforms;
};

// Define a mapping from a uv set index in the source asset to one of Filament's uv sets.
enum UvSet : uint8_t { UNUSED, UV0, UV1 };
using UvMap = std::array<UvSet, 8>;

// The MaterialGenerator uses filamat to generate materials that each unconditionally make the
// minimum number of texture lookups. This complexity could be avoided if we were to use an
// ubershader approach, but this allows us to generate efficient and streamlined shaders that have
// no branching.
class MaterialGenerator final {
public:
    MaterialGenerator(filament::Engine* engine);

    // Creates or fetches a compiled Filament material. The given configuration key might be mutated
    // due to resource constraints. The second argument is populated with a small table that maps
    // from a glTF uv index to a Filament uv index. The third argument is an optional tag that
    // is not a part of the cache key.
    filament::Material* getOrCreateMaterial(MaterialKey* config, UvMap* uvmap,
            const char* label = "material");

    size_t getMaterialsCount() const noexcept;
    const filament::Material* const* getMaterials() const noexcept;
    void destroyMaterials();

private:
    using HashFn = utils::hash::MurmurHashFn<MaterialKey>;
    struct EqualFn { bool operator()(const MaterialKey& k1, const MaterialKey& k2) const; };
    tsl::robin_map<MaterialKey, filament::Material*, HashFn, EqualFn> mCache;
    std::vector<filament::Material*> mMaterials;
    filament::Engine* mEngine;
};

} // namespace details
} // namespace gltfio

#endif // GLTFIO_MATERIALGENERATOR_H
