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

#ifndef GLTFIO_ASSETLOADER_H
#define GLTFIO_ASSETLOADER_H

#include <filament/Engine.h>
#include <filament/Material.h>

#include <gltfio/FilamentAsset.h>

#include <utils/NameComponentManager.h>

namespace gltfio {

/**
 * AssetLoader can be configured to generate materials using filamat, or to use a small set of
 * prebuilt ubershader materials.
 */
enum MaterialSource {
    GENERATE_SHADERS,
    LOAD_UBERSHADERS,
};

struct AssetConfiguration {
    class filament::Engine* engine;
    utils::NameComponentManager* names = nullptr;
    MaterialSource materials = GENERATE_SHADERS;
};

/**
 * AssetLoader consumes a blob of glTF 2.0 content (either JSON or GLB) and produces an "asset",
 * which is a bundle of Filament entities, material instances, textures, vertex buffers, and index
 * buffers.
 *
 * AssetLoader does not fetch external buffer data or create textures on its own. Clients can use
 * the provided ResourceLoader class for this, which obtains the URI list from the asset. This is
 * demonstrated in the code snippet below.
 *
 * AssetLoader also owns a cache of Material objects that may be re-used across multiple loads.
 *
 * Example usage:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * auto engine = Engine::create();
 * auto loader = AssetLoader::create({engine});
 *
 * // Parse the glTF content and create Filament entities.
 * std::vector<uint8_t> content(...);
 * FilamentAsset* asset = loader->createAssetFromJson(content.data(), content.size());
 * content.clear();
 *
 * // Load buffers and textures from disk.
 * ResourceLoader({engine, ".", true}).loadResources(asset);
 *
 * // Obtain the simple animation interface.
 * Animator* animator = asset->getAnimator();
 *
 * // Free the glTF hierarchy as it is no longer needed.
 * asset->releaseSourceData();
 *
 * // Add renderables to the scene.
 * scene->addEntities(asset->getEntities(), asset->getEntityCount());
 *
 * // Execute the render loop and play the first animation.
 * do {
 *      animator->applyAnimation(0, time);
 *      animator->updateBoneMatrices();
 *      if (renderer->beginFrame(swapChain)) {
 *          renderer->render(view);
 *          renderer->endFrame();
 *      }
 * } while (!quit);
 *
 * loader->destroyAsset(asset);
 * loader->destroyMaterials();
 * AssetLoader::destroy(&loader);
 * Engine::destroy(&engine);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class AssetLoader {
public:

    /**
     * Creates an asset loader for the given configuration, which specifies the Filament engine.
     *
     * The engine is held weakly, used only for the creation and destruction of Filament objects.
     * The optional name component manager can be used to assign names to renderables.
     * The material source specifies whether to use filamat to generate materials on the fly, or to
     * load a small set of precompiled ubershader materials.
     */
    static AssetLoader* create(const AssetConfiguration& config);

    /**
     * Frees the loader.
     *
     * This does not not automatically free the cache of materials (see destroyMaterials), nor
     * does it free the entities for created assets (see destroyAsset).
     */
    static void destroy(AssetLoader** loader);

    /**
     * Takes a pointer to the contents of a JSON-based glTF 2.0 file and returns a bundle
     * of Filament objects. Returns null on failure.
     */
    FilamentAsset* createAssetFromJson(const uint8_t* bytes, uint32_t nbytes);

    /**
     * Takes a pointer to the contents of a GLB glTF 2.0 file and returns a bundle
     * of Filament objects. Returns null on failure.
     */
    FilamentAsset* createAssetFromBinary(const uint8_t* bytes, uint32_t nbytes);

    /** Destroys the given asset and all of its associated Filament objects. */
    void destroyAsset(const FilamentAsset* asset);

    /** Gets cached materials, used internally to create material instances for assets. */
    size_t getMaterialsCount() const noexcept;
    const filament::Material* const* getMaterials() const noexcept;

    /** Destroys all cached materials. */
    void destroyMaterials();

protected:
    AssetLoader() noexcept = default;
    ~AssetLoader() = default;

public:
    AssetLoader(AssetLoader const&) = delete;
    AssetLoader(AssetLoader&&) = delete;
    AssetLoader& operator=(AssetLoader const&) = delete;
    AssetLoader& operator=(AssetLoader&&) = delete;
};

} // namespace gltfio

#endif // GLTFIO_ASSETLOADER_H
