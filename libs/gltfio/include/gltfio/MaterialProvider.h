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

#ifndef GLTFIO_MATERIALPROVIDER_H
#define GLTFIO_MATERIALPROVIDER_H

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

#include <array>

namespace gltfio {

enum class AlphaMode : uint8_t {
    OPAQUE,
    MASK,
    BLEND
};

// NOTE: This key is processed by MurmurHashFn so please make padding explicit.
struct alignas(4) MaterialKey {
    // -- 32 bit boundary --
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
    // -- 32 bit boundary --
    uint8_t emissiveUV;
    uint8_t aoUV;
    uint8_t normalUV;
    bool hasTextureTransforms : 8;
    // -- 32 bit boundary --
    float alphaMaskThreshold;
};

static_assert(sizeof(MaterialKey) == 12, "MaterialKey has unexpected padding.");

inline bool operator==(const MaterialKey& k1, const MaterialKey& k2) {
    return
        (k1.doubleSided == k2.doubleSided) &&
        (k1.unlit == k2.unlit) &&
        (k1.hasVertexColors == k2.hasVertexColors) &&
        (k1.hasBaseColorTexture == k2.hasBaseColorTexture) &&
        (k1.hasMetallicRoughnessTexture == k2.hasMetallicRoughnessTexture) &&
        (k1.hasNormalTexture == k2.hasNormalTexture) &&
        (k1.hasOcclusionTexture == k2.hasOcclusionTexture) &&
        (k1.hasEmissiveTexture == k2.hasEmissiveTexture) &&
        (k1.alphaMode == k2.alphaMode) &&
        (k1.baseColorUV == k2.baseColorUV) &&
        (k1.metallicRoughnessUV == k2.metallicRoughnessUV) &&
        (k1.emissiveUV == k2.emissiveUV) &&
        (k1.aoUV == k2.aoUV) &&
        (k1.normalUV == k2.normalUV) &&
        (k1.alphaMaskThreshold == k2.alphaMaskThreshold);
}

// Define a mapping from a uv set index in the source asset to one of Filament's uv sets.
enum UvSet : uint8_t { UNUSED, UV0, UV1 };
using UvMap = std::array<UvSet, 8>;

/**
 * MaterialProvider is an interface to a provider of glTF materials with two implementations.
 *
 * - The "MaterialGenerator" implementation generates materials at run time (which can be slow)
 *   and requires the filamat library, but produces streamlined shaders.
 *
 * - The "Ubermaterial" implementation uses a small number of pre-built materials with complex
 *   fragment shaders, but does not require any run time work or usage of filamat.
 */
class MaterialProvider {
public:
    static MaterialProvider* createMaterialGenerator(filament::Engine* engine);

    static MaterialProvider* createUbermaterial(filament::Engine* engine) {
         /* TODO: ubermaterial not yet implemented  */
        return nullptr;
    }

    virtual ~MaterialProvider() {}

    // Creates or fetches a compiled Filament material, then creates an instance from it. The given
    // configuration key might be mutated due to resource constraints. The second argument is
    // populated with a small table that maps from a glTF uv index to a Filament uv index. The third
    // argument is an optional tag that is not a part of the cache key.
    virtual filament::MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label = "material") = 0;

    virtual size_t getMaterialsCount() const noexcept = 0;
    virtual const filament::Material* const* getMaterials() const noexcept = 0;
    virtual void destroyMaterials() = 0;
};

} // namespace gltfio

#endif // GLTFIO_MATERIALPROVIDER_H
