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

#include <gltfio/FilamentAsset.h>

namespace filament {
    class Engine;
    class Material;
}

namespace gltfio {

/**
 * AssetLoader consumes a blob of glTF 2.0 content (either JSON or GLB) and produces an "asset",
 * which is a bundle of Filament entities, material instances, vertex buffers, and index buffers.
 *
 * For JSON-based content, the loader does not fetch external buffer data or image data. Clients can
 * obtain the URI list from the asset or use the provided BindingHelper class.
 *
 * The loader also owns a cache of Material objects that may be re-used across multiple loads.
 *
 * TODO: currently the loader uses filamat to generate materials on the fly, but we may wish to
 * allow clients to load a small set of precompiled ubershader materials, in order to avoid the
 * filamat library on resource-constrained platforms.
 *
 * Example usage:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * auto engine = Engine::create();
 * auto loader = AssetLoader::create(engine);
 *
 * // Parse the glTF content and create Filament entities.
 * std::vector<uint8_t> content(...);
 * FilamentAsset* asset = loader->createAssetFromJson(content.data(), content.size());
 * content.clear();
 *
 * // Load buffer data and textures from disk.
 * BindingHelper(engine, ".").loadResources(asset);
 *
 * // Create the simple animation engine.
 * AnimationHelper animation(asset);
 *
 * // Free the glTF hierarchy as it is no longer needed.
 * asset->releaseSourceData();
 *
 * // Add renderables to the scene.
 * scene->addEntities(asset->getEntities(), asset->getEntityCount());
 *
 * // Execute the render loop and play the first animation.
 * do {
 *      animation.applyAnimation(0, time);
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
     * Creates an asset loader for the given engine.
     * 
     * The engine is held weakly, used only for the creation and destruction of Filament objects.
     */
    static AssetLoader* create(filament::Engine* engine);

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
    FilamentAsset* createAssetFromJson(uint8_t const* bytes, uint32_t nbytes);

    /**
     * Takes a pointer to the contents of a GLB glTF 2.0 file and returns a bundle
     * of Filament objects. Returns null on failure.
     */
    FilamentAsset* createAssetFromBinary(uint8_t const* bytes, uint32_t nbytes);

    /** Destroys the given asset and all of its associated Filament objects. */
    void destroyAsset(const FilamentAsset* asset);

    /**
     * Enables or disables shadows on all subsequently loaded assets.
     * 
     * Initially, loaded assets will cast and receive shadows.
     */
    void castShadowsByDefault(bool enable);
    void receiveShadowsByDefault(bool enable);

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
