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
#include <gltfio/FilamentInstance.h>
#include <gltfio/MaterialProvider.h>

#include <utils/compiler.h>

namespace utils {
    class EntityManager;
    class NameComponentManager;
}

/**
 * Loader and pipeline for glTF 2.0 assets.
 */
namespace filament::gltfio {

class NodeManager;

// Use this struct to enable mikktspace-based tangent-space computation.
/**
 * \struct AssetConfigurationExtended AssetLoader.h gltfio/AssetLoader.h
 * \brief extends struct AssetConfiguration
 * Useful if client needs mikktspace tangents.
 */
struct AssetConfigurationExtended {
    //! Optional The same parameter as provided to \struct ResourceConfiguration ResourceLoader.h
    //! gltfio/ResourceLoader.h
    char const* gltfPath;
};

/**
 * \struct AssetConfiguration AssetLoader.h gltfio/AssetLoader.h
 * \brief Construction parameters for AssetLoader.
 */
struct AssetConfiguration {
    //! The engine that the loader should pass to builder objects (e.g.
    //! filament::VertexBuffer::Builder).
    class filament::Engine* engine;

    //! Controls whether the loader uses filamat to generate materials on the fly, or loads a small
    //! set of precompiled ubershader materials. Deleting the MaterialProvider is the client's
    //! responsibility. See createJitShaderProvider() and createUbershaderProvider().
    MaterialProvider* materials;

    //! Optional manager for associating string names with entities in the transform hierarchy.
    utils::NameComponentManager* names = nullptr;

    //! Overrides the factory used for creating entities in the transform hierarchy. If this is not
    //! specified, AssetLoader will use the singleton EntityManager associated with the current
    //! process.
    utils::EntityManager* entities = nullptr;

    //! Optional default node name for anonymous nodes
    char* defaultNodeName = nullptr;

    //! Optional to enable mikktspace tangents.
    AssetConfigurationExtended* ext = nullptr;
};

/**
 * \class AssetLoader AssetLoader.h gltfio/AssetLoader.h
 * \brief Consumes glTF content and produces FilamentAsset objects.
 *
 * AssetLoader consumes a blob of glTF 2.0 content (either JSON or GLB) and produces a FilamentAsset
 * object, which is a bundle of Filament textures, vertex buffers, index buffers, etc. An asset is
 * composed of 1 or more FilamentInstance objects which contain entities and components.
 *
 * Clients must use AssetLoader to create and destroy FilamentAsset objects. This is similar to
 * how filament::Engine is used to create and destroy core objects like VertexBuffer.
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
 * auto materials = createJitShaderProvider(engine);
 * auto decoder = createStbProvider(engine);
 * auto loader = AssetLoader::create({engine, materials});
 *
 * // Parse the glTF content and create Filament entities.
 * std::vector<uint8_t> content(...);
 * FilamentAsset* asset = loader->createAsset(content.data(), content.size());
 * content.clear();
 *
 * // Load buffers and textures from disk.
 * ResourceLoader resourceLoader({engine, ".", true});
 * resourceLoader.addTextureProvider("image/png", decoder)
 * resourceLoader.addTextureProvider("image/jpeg", decoder)
 * resourceLoader.loadResources(asset);
 *
 * // Free the glTF hierarchy as it is no longer needed.
 * asset->releaseSourceData();
 *
 * // Add renderables to the scene.
 * scene->addEntities(asset->getEntities(), asset->getEntityCount());
 *
 * // Extract the animator interface from the FilamentInstance.
 * auto animator = asset->getInstance()->getAnimator();
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
 * scene->removeEntities(asset->getEntities(), asset->getEntityCount());
 * loader->destroyAsset(asset);
 * materials->destroyMaterials();
 * delete materials;
 * delete decoder;
 * AssetLoader::destroy(&loader);
 * Engine::destroy(&engine);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class UTILS_PUBLIC AssetLoader {
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
     * Takes a pointer to the contents of a GLB or a JSON-based glTF 2.0 file and returns an asset
     * with one instance, or null on failure.
     */
    FilamentAsset* createAsset(const uint8_t* bytes, uint32_t nbytes);

    /**
     * Consumes the contents of a glTF 2.0 file and produces a primary asset with one or more
     * instances. The primary asset has ownership over the instances.
     *
     * The returned instances share their textures, materials, and vertex buffers with the primary
     * asset. However each instance has its own unique set of entities, transform components,
     * material instances, and renderable components. Instances are freed when the primary asset is
     * freed.
     *
     * Light components are not instanced, they belong only to the primary asset.
     *
     * Clients must use ResourceLoader to load resources on the primary asset.
     *
     * The entity accessor and renderable stack API in the primary asset can be used to control the
     * union of all instances. The individual FilamentInstance objects can be used to access each
     * instance's partition of entities.  Similarly, the Animator in the primary asset controls all
     * instances. To animate instances individually, use FilamentInstance::getAnimator().
     *
     * @param bytes the contents of a glTF 2.0 file (JSON or GLB)
     * @param numBytes the number of bytes in "bytes"
     * @param instances destination pointer, to be populated by the requested number of instances
     * @param numInstances requested number of instances
     * @return the primary asset that has ownership over all instances
     */
    FilamentAsset* createInstancedAsset(const uint8_t* bytes, uint32_t numBytes,
            FilamentInstance** instances, size_t numInstances);

    /**
     * Adds a new instance to the asset.
     *
     * Use this with caution. It is more efficient to pre-allocate a max number of instances, and
     * gradually add them to the scene as needed. Instances can also be "recycled" by removing and
     * re-adding them to the scene.
     *
     * NOTE: destroyInstance() does not exist because gltfio favors flat arrays for storage of
     * entity lists and instance lists, which would be slow to shift. We also wish to discourage
     * create/destroy churn, as noted above.
     *
     * This cannot be called after FilamentAsset::releaseSourceData().
     * See also AssetLoader::createInstancedAsset().
     */
    FilamentInstance* createInstance(FilamentAsset* asset);

    /**
     * Allows clients to enable diagnostic shading on newly-loaded assets.
     */
    void enableDiagnostics(bool enable = true);

    /**
     * Destroys the given asset, all of its associated Filament objects, and all associated
     * FilamentInstance objects.
     *
     * This destroys entities, components, material instances, vertex buffers, index buffers,
     * and textures. This does not necessarily immediately free all source data, since
     * texture decoding or GPU uploading might be underway.
     */
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

    utils::NameComponentManager* getNames() const noexcept;

    NodeManager& getNodeManager() noexcept;

    MaterialProvider& getMaterialProvider() noexcept;

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

} // namespace filament::gltfio

#endif // GLTFIO_ASSETLOADER_H
