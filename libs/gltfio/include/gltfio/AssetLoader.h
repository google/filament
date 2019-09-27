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
#include <gltfio/MaterialProvider.h>

namespace utils {
    class NameComponentManager;
}

/**
 * Loader and pipeline for glTF 2.0 assets.
 */
namespace gltfio {

/**
 * \struct AssetConfiguration AssetLoader.h gltfio/AssetLoader.h
 * \brief Construction parameters for AssetLoader.
 */
struct AssetConfiguration {
    //! The engine that the loader should pass to builder objects (e.g.
    //! filament::VertexBuffer::Builder).
    class filament::Engine* engine;

    //! Controls whether the loader uses filamat to generate materials on the fly, or loads a small
    //! set of precompiled ubershader materials. See createMaterialGenerator() and
    //! createUbershaderLoader().
    MaterialProvider* materials;

    //! Optional manager for associating string names with entities in the transform hierarchy.
    utils::NameComponentManager* names = nullptr;

    //! Overrides the factory used for creating entities in the transform hierarchy. If this is not
    //! specified, AssetLoader will use the singleton EntityManager associated with the current
    //! process.
    utils::EntityManager* entities = nullptr;
};

/**
 * \class AssetLoader AssetLoader.h gltfio/AssetLoader.h
 * \brief Consumes glTF content and produces FilamentAsset objects.
 *
 * AssetLoader consumes a blob of glTF 2.0 content (either JSON or GLB) and produces a FilamentAsset
 * object, which is a bundle of Filament entities, material instances, textures, vertex buffers,
 * and index buffers.
 *
 * Clients must use AssetLoader to create and destroy FilamentAsset objects.
 *
 * AssetLoader does not fetch external buffer data or create textures on its own. Clients can use
 * ResourceLoader for this, which obtains the URI list from the asset. This is demonstrated in the
 * code snippet below.
 *
 * AssetLoader also owns a cache of filament::Material objects that may be re-used across multiple
 * loads.
 *
 * Example usage:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * auto engine = Engine::create();
 * auto materials = createMaterialGenerator(engine);
 * auto loader = AssetLoader::create({engine, materials});
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
 * materials->destroyMaterials();
 * delete materials;
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
     * This does not not automatically free the cache of materials, nor
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

    /**
     * Takes a pointer to an opaque pipeline object and returns a bundle of Filament objects.
     *
     * This exists solely for interop with AssetPipeline, which is optional according to the build
     * configuration.
     */
    FilamentAsset* createAssetFromHandle(const void* cgltf);

    /**
     * Allows clients to enable diagnostic shading on newly-loaded assets.
     */
    void enableDiagnostics(bool enable = true);

    /** Destroys the given asset and all of its associated Filament objects. */
    void destroyAsset(const FilamentAsset* asset);

    /**
     * Gets a weak reference to an array of cached materials, used internally to create material
     * instances for assets.
     */
    const filament::Material* const* getMaterials() const noexcept;

    /**
     * Gets the number of cached materials.
     */
    size_t getMaterialsCount() const noexcept;

    /*! \cond PRIVATE */
protected:
    AssetLoader() noexcept = default;
    ~AssetLoader() = default;

public:
    AssetLoader(AssetLoader const&) = delete;
    AssetLoader(AssetLoader&&) = delete;
    AssetLoader& operator=(AssetLoader const&) = delete;
    AssetLoader& operator=(AssetLoader&&) = delete;
    /*! \endcond */
};

} // namespace gltfio

#endif // GLTFIO_ASSETLOADER_H
