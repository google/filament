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

#ifndef GLTFIO_BINDINGHELPER_H
#define GLTFIO_BINDINGHELPER_H

#include <gltfio/FilamentAsset.h>

#include <utils/Path.h>

namespace filament {
    class Engine;
}

namespace gltfio {

class UrlCache;

/**
 * BindingHelper asynchronously uploads vertex buffers and textures to the GPU.
 *
 * For a usage example, see the comment block for AssetLoader.
 *
 * This class holds a map of URL's to data blobs and can therefore be used for multiple assets.
 * However, if desired, clients can immediately destroy the helper after calling loadResources.
 * There is no need to wait for resources to finish uploading because this is done in the
 * the background.
 *
 * The helper must be destroyed on the same thread that calls Renderer::render because it listens to
 * BufferDescriptor callbacks in order to determine when to free CPU-side data blobs.
 */
class BindingHelper {
public:
    BindingHelper(filament::Engine* engine, const char* basePath);
    ~BindingHelper();
    bool loadResources(FilamentAsset* asset);
private:
    bool isBase64(const BufferBinding& bb);
    bool isFile(const BufferBinding& bb);
    void* loadBase64(const BufferBinding& bb);
    void* loadFile(const BufferBinding& bb);
    UrlCache* mCache;
    filament::Engine* mEngine;
    utils::Path mBasePath;
};

} // namespace gltfio

#endif // GLTFIO_BINDINGHELPER_H

