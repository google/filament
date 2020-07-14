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

/**
 * \struct MaterialKey MaterialProvider.h gltfio/MaterialProvider.h
 * \brief Small POD structure that specifies the requirements for a glTF material.
 * \note This key is processed by MurmurHashFn so please make padding explicit.
 */
struct alignas(4) MaterialKey {
    // -- 32 bit boundary --
    bool doubleSided : 1;
    bool unlit : 1;
    bool hasVertexColors : 1;
    bool hasBaseColorTexture : 1;
    bool hasNormalTexture : 1;
    bool hasOcclusionTexture : 1;
    bool hasEmissiveTexture : 1;
    bool useSpecularGlossiness : 1;
    AlphaMode alphaMode : 4;
    bool enableDiagnostics : 4;
    union {
        struct {
            bool hasMetallicRoughnessTexture : 1;
            uint8_t metallicRoughnessUV : 7;
        };
        struct {
            bool hasSpecularGlossinessTexture : 1;
            uint8_t specularGlossinessUV : 7;
        };
    };
    uint8_t baseColorUV;
    // -- 32 bit boundary --
    bool hasClearCoatTexture : 1;
    uint8_t clearCoatUV : 7;
    bool hasClearCoatRoughnessTexture : 1;
    uint8_t clearCoatRoughnessUV : 7;
    bool hasClearCoatNormalTexture : 1;
    uint8_t clearCoatNormalUV : 7;
    bool hasClearCoat : 1;
    bool hasTransmission : 1;
    bool hasTextureTransforms : 6;
    // -- 32 bit boundary --
    uint8_t emissiveUV;
    uint8_t aoUV;
    uint8_t normalUV;
    bool hasTransmissionTexture : 1;
    uint8_t transmissionUV : 7;
};

static_assert(sizeof(MaterialKey) == 12, "MaterialKey has unexpected padding.");

bool operator==(const MaterialKey& k1, const MaterialKey& k2);

// Define a mapping from a uv set index in the source asset to one of Filament's uv sets.
enum UvSet : uint8_t { UNUSED, UV0, UV1 };
constexpr int UvMapSize = 8;
using UvMap = std::array<UvSet, UvMapSize>;

inline uint8_t getNumUvSets(const UvMap& uvmap) {
    return std::max({
        uvmap[0], uvmap[1], uvmap[2], uvmap[3],
        uvmap[4], uvmap[5], uvmap[6], uvmap[7],
    });
};

enum MaterialSource {
    GENERATE_SHADERS,
    LOAD_UBERSHADERS,
};

/**
 * \class MaterialProvider MaterialProvider.h gltfio/MaterialProvider.h
 * \brief Interface to a provider of glTF materials (has two implementations).
 *
 * - The \c MaterialGenerator implementation generates materials at run time (which can be slow) and
 *   requires the filamat library, but produces streamlined shaders. See createMaterialGenerator().
 *
 * - The \c UbershaderLoader implementation uses a small number of pre-built materials with complex
 *   fragment shaders, but does not require any run time work or usage of filamat. See
 *   createUbershaderLoader().
 */
class MaterialProvider {
public:
    virtual ~MaterialProvider() {}

    /**
     * Returns the type of material provider (generator or ubershader).
     */
    virtual MaterialSource getSource() const noexcept = 0;

    /**
     * Creates or fetches a compiled Filament material, then creates an instance from it.
     *
     * @param config Specifies requirements; might be mutated due to resource constraints.
     * @param uvmap Output argument that gets populated with a small table that maps from a glTF uv
     *              index to a Filament uv index.
     * @param label Optional tag that is not a part of the cache key.
     */
    virtual filament::MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label = "material") = 0;

    /**
     * Gets a weak reference to the array of cached materials.
     */
    virtual const filament::Material* const* getMaterials() const noexcept = 0;

    /**
     * Gets the number of cached materials.
     */
    virtual size_t getMaterialsCount() const noexcept = 0;

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    virtual void destroyMaterials() = 0;
};

void constrainMaterial(MaterialKey* key, UvMap* uvmap);

void processShaderString(std::string* shader, const UvMap& uvmap,
        const MaterialKey& config);

/**
 * Creates a material provider that builds materials on the fly, composing GLSL at run time.
 *
 * Requires \c libfilamat to be linked in. Not available in \c libgltfio_core.
 */
MaterialProvider* createMaterialGenerator(filament::Engine* engine);

/**
 * Creates a material provider that loads a small set of pre-built materials.
 *
 * Requires \c libgltfio_resources to be linked in.
 */
MaterialProvider* createUbershaderLoader(filament::Engine* engine);

} // namespace gltfio

#endif // GLTFIO_MATERIALPROVIDER_H
