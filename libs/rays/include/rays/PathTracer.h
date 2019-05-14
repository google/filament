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

#ifndef RAYS_PATHTRACER_H
#define RAYS_PATHTRACER_H

#include <rays/SimpleCamera.h>
#include <rays/SimpleMesh.h>

#include <image/LinearImage.h>

#include <math/vec2.h>

#include <limits>

namespace filament {
namespace rays {

/**
 * Signals that a region of the generated image has become available, typically used for progress
 * notification. This can be called from any thread.
 */
using TileCallback = void(*)(image::LinearImage target,
        filament::math::ushort2 topLeft, filament::math::ushort2 bottomRight,
        void* userData);

/**
 * Signals that the generated image is now completely rendered.  This can be called from any
 * thread.
 */
using DoneCallback = void(*)(image::LinearImage target, void* userData);

/**
 * PathTracer renders an asset by splitting the render target into tiles and invoking a JobSystem
 * task for each tile.
 *
 * The lifetime of the PathTracer object is independent of the background rendering process, so
 * clients can create a PathTracer and immediately discard it. However the passed-in mesh list and
 * the vertex data that it references must stay alive for the duration of the render.
 *
 * The progress callbacks are triggered from a separate thread.
 */
class PathTracer {
public:

    struct Config {
        image::LinearImage renderTarget;
        const SimpleMesh* meshes;
        size_t numMeshes = 0;
        size_t samplesPerPixel = 256;
        SimpleCamera filmCamera;
        bool uvCamera = false;
        TileCallback tileCallback = nullptr;
        void* tileUserData = nullptr;
        DoneCallback doneCallback = nullptr;
        void* doneUserData = nullptr;
        float aoRayNear = std::numeric_limits<float>::epsilon() * 10.0f;
        float aoRayFar = std::numeric_limits<float>::infinity();
    };

    class Builder {
    public:
        Builder& renderTarget(image::LinearImage target);
        Builder& meshes(const SimpleMesh* meshes, size_t numMeshes);
        Builder& filmCamera(const SimpleCamera& camera);
        Builder& uvCamera();
        Builder& tileCallback(TileCallback onTile, void* userData);
        Builder& doneCallback(DoneCallback onTile, void* userData);
        Builder& samplesPerPixel(size_t numSamples);
        PathTracer build();
    private:
        Config mConfig;
    };

    bool render();

private:
    PathTracer(const Config& config) : mConfig(config) {}
    const Config mConfig;
};

} // namespace rays
} // namespace filament

#endif // RAYS_PATHTRACER_H
