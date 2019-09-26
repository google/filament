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

#ifndef GLTFIO_RESOURCELOADER_H
#define GLTFIO_RESOURCELOADER_H

#include <filament/Engine.h>

#include <gltfio/FilamentAsset.h>

#include <backend/BufferDescriptor.h>

#include <utils/Path.h>

namespace gltfio {

namespace details {
    struct FFilamentAsset;
    class AssetPool;
}

/**
 * \struct ResourceConfiguration ResourceLoader.h gltfio/ResourceLoader.h
 * \brief Construction parameters for ResourceLoader.
 */
struct ResourceConfiguration {
    //! The engine that the loader should pass to builder objects (e.g.
    //! filament::Texture::Builder).
    class filament::Engine* engine;

    //! Optional path or URI that points to the base glTF file. This is used solely
    //! to resolve relative paths.
    utils::Path gltfPath;

    //! If true, adjusts skinning weights to sum to 1. Well formed glTF files do not need this,
    //! but it is useful for robustness.
    bool normalizeSkinningWeights;

    //! If true, computes the bounding boxes of all \c POSITION attibutes. Well formed glTF files
    //! do not need this, but it is useful for robustness.
    bool recomputeBoundingBoxes;
};

/**
 * \class ResourceLoader ResourceLoader.h gltfio/ResourceLoader.h
 * \brief Asynchronously uploads vertex buffers and textures to the GPU and computes tangents.
 *
 * For a usage example, see the documentation for AssetLoader.
 *
 * In theory, this class could cache a map of URL's to data blobs and could therefore be useful
 * across multiple assets. However, clients should feel free to immediately destroy this after
 * calling loadResources. There is no need to wait for resources to finish uploading because this is
 * done in the the background.
 *
 * ResourceLoader must be destroyed on the same thread that calls filament::Renderer::render()
 * because it listens to filament::backend::BufferDescriptor callbacks in order to determine when to
 * free CPU-side data blobs.
 *
 * \todo The GPU upload is asynchronous but the load-from-disk and image decode is not.
 */
class ResourceLoader {
public:
    using BufferDescriptor = filament::backend::BufferDescriptor;

    ResourceLoader(const ResourceConfiguration& config);
    ~ResourceLoader();

    /**
     * Loads resources for the given asset from the filesystem or data cache and "finalizes" the
     * asset by transforming the vertex data format if necessary, decoding image files, supplying
     * tangent data, etc.
     *
     * Returns false if resources have already been loaded, or if one or more resources could not
     * be loaded.
     */
    bool loadResources(FilamentAsset* asset);

    /**
     * Adds raw resource data into a cache for platforms that do not have filesystem or network
     * access.
     */
    void addResourceData(std::string url, BufferDescriptor&& buffer);

private:
    bool createTextures(details::FFilamentAsset* asset) const;
    void computeTangents(details::FFilamentAsset* asset) const;
    void normalizeSkinningWeights(details::FFilamentAsset* asset) const;
    void updateBoundingBoxes(details::FFilamentAsset* asset) const;
    details::AssetPool* mPool;
    const ResourceConfiguration mConfig;

    struct Impl;
    Impl* pImpl;
};

} // namespace gltfio

#endif // GLTFIO_RESOURCELOADER_H

