/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef GLTFIO_RESOURCELOADER_H
#define GLTFIO_RESOURCELOADER_H

#include <gltfio/FilamentAsset.h>

#include <backend/BufferDescriptor.h>

namespace filament {
    class Engine;
}

namespace gltfio {

struct FFilamentAsset;
class AssetPool;

/**
 * \struct ResourceConfiguration ResourceLoader.h gltfio/ResourceLoader.h
 * \brief Construction parameters for ResourceLoader.
 */
struct ResourceConfiguration {
    //! The engine that the loader should pass to builder objects (e.g.
    //! filament::Texture::Builder).
    class filament::Engine* engine;

    //! Optional path or URI that points to the base glTF file. This is used solely
    //! to resolve relative paths. The string pointer is not retained.
    const char* gltfPath;

    //! If true, adjusts skinning weights to sum to 1. Well formed glTF files do not need this,
    //! but it is useful for robustness.
    bool normalizeSkinningWeights;

    //! If true, computes the bounding boxes of all \c POSITION attibutes. Well formed glTF files
    //! do not need this, but it is useful for robustness.
    bool recomputeBoundingBoxes;
};

/**
 * \class ResourceLoader ResourceLoader.h gltfio/ResourceLoader.h
 * \brief Prepares and uploads vertex buffers and textures to the GPU.
 *
 * For a usage example, see the documentation for AssetLoader.
 *
 * ResourceLoader must be destroyed on the same thread that calls filament::Renderer::render()
 * because it listens to filament::backend::BufferDescriptor callbacks in order to determine when to
 * free CPU-side data blobs.
 *
 * \todo If clients persist their ResourceLoader, Filament textures are currently re-created upon
 * subsequent re-loads of the same asset. To fix this, we would need to enable shared ownership
 * of Texture objects between ResourceLoader and FilamentAsset.
 */
class ResourceLoader {
public:
    using BufferDescriptor = filament::backend::BufferDescriptor;

    ResourceLoader(const ResourceConfiguration& config);
    ~ResourceLoader();

    /**
     * Feeds the binary content of an external resource into the loader's URI cache.
     *
     * On some platforms, `ResourceLoader` does not know how to download external resources on its
     * own (external resources might come from a filesystem, a database, or the internet) so this
     * method allows clients to download external resources and push them to the loader.
     *
     * Every resource should be passed in before calling #loadResources or #asyncBeginLoad. See
     * also FilamentAsset#getResourceUris.
     *
     * When loading GLB files (as opposed to JSON-based glTF files), clients typically do not
     * need to call this method.
     */
    void addResourceData(const char* uri, BufferDescriptor&& buffer);

    /**
     * Checks if the given resource has already been added to the URI cache.
     */
    bool hasResourceData(const char* uri) const;

    /**
     * Loads resources for the given asset from the filesystem or data cache and "finalizes" the
     * asset by transforming the vertex data format if necessary, decoding image files, supplying
     * tangent data, etc.
     *
     * Returns false if resources have already been loaded, or if one or more resources could not
     * be loaded.
     *
     * Note: this method is synchronous and blocks until all textures have been decoded.
     * For an asynchronous alternative, see #asyncBeginLoad.
     */
    bool loadResources(FilamentAsset* asset);

    /**
     * Starts an asynchronous resource load.
     *
     * Returns false if the loading process was unable to start.
     *
     * This is an alternative to #loadResources and requires periodic calls to #asyncUpdateLoad.
     * On multi-threaded systems this creates threads for texture decoding.
     */
    bool asyncBeginLoad(FilamentAsset* asset);

    /**
     * Gets the status of an asynchronous resource load as a percentage in [0,1].
     */
    float asyncGetLoadProgress() const;

    /**
     * Updates an asynchronous load by performing any pending work that must take place
     * on the main thread.
     *
     * Clients must periodically call this until #asyncGetLoadProgress returns 100%.
     * After progress reaches 100%, calling this is harmless; it just does nothing.
     */
    void asyncUpdateLoad();

private:
    bool loadResources(FFilamentAsset* asset, bool async);
    void applySparseData(FFilamentAsset* asset) const;
    void normalizeSkinningWeights(FFilamentAsset* asset) const;
    void updateBoundingBoxes(FFilamentAsset* asset) const;
    AssetPool* mPool;
    struct Impl;
    Impl* pImpl;
};

} // namespace gltfio

#endif // GLTFIO_RESOURCELOADER_H

