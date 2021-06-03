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

#ifndef GLTFIO_FRESOURCELOADER_H
#define GLTFIO_FRESOURCELOADER_H

#include <gltfio/ResourceLoader.h>
#include <backend/BufferDescriptor.h>
#include <utils/JobSystem.h>
#include <tsl/robin_map.h>

#include <memory>
#include <string>

#include "upcast.h"

namespace filament {
class Engine;
}

namespace gltfio {
class AssetPool;
struct FFilamentAsset;
struct TextureSlot;
struct TextureCacheEntry;

struct FResourceLoader : public ResourceLoader {
    FResourceLoader(const ResourceConfiguration& config) {
        mGltfPath = std::string(config.gltfPath ? config.gltfPath : "");
        mEngine = config.engine;
        mNormalizeSkinningWeights = config.normalizeSkinningWeights;
        mRecomputeBoundingBoxes = config.recomputeBoundingBoxes;
    }

    ~FResourceLoader();

    void addResourceData(const char* uri, BufferDescriptor&& buffer);
    bool hasResourceData(const char* uri) const;
    void evictResourceData();
    bool loadResources(FilamentAsset* asset);
    bool asyncBeginLoad(FilamentAsset* asset);
    float asyncGetLoadProgress() const;
    void asyncUpdateLoad();
    void asyncCancelLoad();

    bool loadResources(FFilamentAsset* asset, bool async);
    void applySparseData(FFilamentAsset* asset) const;
    void normalizeSkinningWeights(FFilamentAsset* asset) const;
    void updateBoundingBoxes(FFilamentAsset* asset) const;

    void computeTangents(FFilamentAsset* asset);
    bool createTextures(bool async);
    void cancelTextureDecoding();
    void addTextureCacheEntry(const TextureSlot& tb);
    void bindTextureToMaterial(const TextureSlot& tb);
    void decodeSingleTexture();
    void uploadPendingTextures();
    void releasePendingTextures();

    filament::Engine* mEngine;
    bool mNormalizeSkinningWeights;
    bool mRecomputeBoundingBoxes;
    std::string mGltfPath;

    AssetPool* mPool;

    using BufferTextureCache = tsl::robin_map<const void*, std::unique_ptr<TextureCacheEntry>>;
    using UriTextureCache = tsl::robin_map<std::string, std::unique_ptr<TextureCacheEntry>>;
    using UriDataCache = tsl::robin_map<std::string, gltfio::ResourceLoader::BufferDescriptor>;

    // User-provided resource data with URI string keys, populated with addResourceData().
    // This is used on platforms without traditional file systems, such as Android, iOS, and WebGL.
    UriDataCache mUriDataCache;

    // The two texture caches are populated while textures are being decoded, and they are no longer
    // used after all textures have been finalized. Since multiple glTF textures might be loaded
    // from a single URI or buffer pointer, these caches prevent needless re-decoding. There are
    // two caches: one for URI-based textures and one for buffer-based textures.
    BufferTextureCache mBufferTextureCache;
    UriTextureCache mUriTextureCache;
    int mNumDecoderTasks;
    int mNumDecoderTasksFinished;
    utils::JobSystem::Job* mDecoderRootJob = nullptr;
    FFilamentAsset* mCurrentAsset = nullptr;
};

FILAMENT_UPCAST(ResourceLoader)

} // namespace gltfio

#endif // GLTFIO_FRESOURCELOADER_H
