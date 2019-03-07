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

#include <utils/Path.h>

namespace gltfio {

namespace details {
    class FFilamentAsset;
    class AssetPool;
}

struct ResourceConfiguration {
    class filament::Engine* engine;
    utils::Path basePath;
    bool normalizeSkinningWeights;
    bool recomputeBoundingBoxes;
};

/**
 * ResourceLoader asynchronously uploads vertex buffers and textures to the GPU, computes
 * surface orientation quaternions, and optionally normalizes skinning weights.
 *
 * For a usage example, see the comment block for AssetLoader.
 *
 * In theory, this class could cache a map of URL's to data blobs and could therefore be useful
 * across multiple assets. However, clients should feel free to immediately destroy this after
 * calling loadResources. There is no need to wait for resources to finish uploading because this is
 * done in the the background.
 *
 * The resource loader must be destroyed on the same thread that calls Renderer::render because it
 * listens to BufferDescriptor callbacks in order to determine when to free CPU-side data blobs.
 *
 * TODO: the GPU upload is asynchronous but the load-from-disk and image decode is not.
 */
class ResourceLoader {
public:
    ResourceLoader(const ResourceConfiguration& config);
    ~ResourceLoader();
    bool loadResources(FilamentAsset* asset);
private:
    bool createTextures(details::FFilamentAsset* asset) const;
    void computeTangents(details::FFilamentAsset* asset) const;
    void normalizeSkinningWeights(details::FFilamentAsset* asset) const;
    void updateBoundingBoxes(details::FFilamentAsset* asset) const;
    details::AssetPool* mPool;
    const ResourceConfiguration mConfig;
};

} // namespace gltfio

#endif // GLTFIO_RESOURCELOADER_H

