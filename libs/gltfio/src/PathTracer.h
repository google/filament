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

#ifndef GLTFIO_PATHTRACER_H
#define GLTFIO_PATHTRACER_H

#include <gltfio/AssetPipeline.h>
#include <gltfio/SimpleCamera.h>

#include <image/LinearImage.h>

namespace gltfio {

/**
 * PathTracer uses Intel embree to render an AssetPipeline scene by splitting the render target
 * into tiles and invoking a JobSystem task for each tile.
 *
 * The lifetime of the PathTracer object is independent of the background rendering process so
 * clients can create a PathTracer and immediately discard it. The passed-in progress callbacks are
 * triggered from a separate thread.
 */
class PathTracer {
public:
    using TileCallback = AssetPipeline::RenderTileCallback;
    using DoneCallback = AssetPipeline::RenderDoneCallback;
    using AssetHandle = AssetPipeline::AssetHandle;

    class Builder {
    public:
        Builder();
        ~Builder();
        Builder& renderTarget(image::LinearImage target);
        Builder& sourceAsset(AssetHandle asset);
        Builder& filmCamera(const SimpleCamera& camera);
        Builder& uvCamera(const char* uvAttribute);
        Builder& tileCallback(TileCallback onTile, void* userData);
        Builder& doneCallback(DoneCallback onTile, void* userData);
        PathTracer build();
    private:
        PathTracer* mPathTracer;
    };

    bool render();

private:
    PathTracer() {}
    image::LinearImage mRenderTarget;
    AssetHandle mSourceAsset;
    SimpleCamera mFilmCamera;
    const char* mUvCamera = nullptr;
    TileCallback mTileCallback = nullptr;
    void* mTileUserData = nullptr;
    DoneCallback mDoneCallback = nullptr;
    void* mDoneUserData = nullptr;
};

} // namespace gltfio

#endif // GLTFIO_PATHTRACER_H
