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
 * OutputPlane denotes a client-owned image that the PathTracer renders into.
 *
 * All output planes must have 3 channels except AMBIENT_OCCLUSION, which must have 1.
 */
enum OutputPlane {
    AMBIENT_OCCLUSION = 0,
    BENT_NORMALS = 1,
    MESH_NORMALS = 2,
    MESH_POSITIONS = 3
};

static constexpr size_t OUTPUTPLANE_COUNT = 4;

struct Config;

/**
 * Signals that a region within each render target has become available, typically used for
 * progress notification. This can be called from any thread.
 */
using TileCallback = void(*)(filament::math::ushort2 topLeft, filament::math::ushort2 bottomRight,
        void* userData);

/**
 * Signals that all render targets are now available.  This can be called from any thread.
 */
using DoneCallback = void(*)(void* userData);

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
        image::LinearImage renderTargets[OUTPUTPLANE_COUNT];
        const SimpleMesh* meshes = nullptr;
        size_t numMeshes = 0;
        size_t samplesPerPixel = 256;
        SimpleCamera filmCamera;
        bool uvCamera = false;
        bool denoise = false;
        TileCallback tileCallback = nullptr;
        void* tileUserData = nullptr;
        DoneCallback doneCallback = nullptr;
        void* doneUserData = nullptr;
        float aoRayNear = std::numeric_limits<float>::epsilon() * 10.0f;
        float aoRayFar = std::numeric_limits<float>::infinity();
    };

    class Builder {
    public:

        /**
         * Adds a client-owned output plane to the path tracer.
         *
         * The AMBIENT_OCCLUSION output is required, others are optional.
         */
        Builder& outputPlane(OutputPlane target, image::LinearImage image);

        /**
         * Adds a set of geometry objects to the scene.
         */
        Builder& meshes(const SimpleMesh* meshes, size_t numMeshes);

        /**
         * Enables a traditional 3D rendering camera (not used for baking).
         */
        Builder& filmCamera(const SimpleCamera& camera);

        /**
         * Enables a UV-based cameras used for light map baking.
         */
        Builder& uvCamera();

        /**
         * Executes OpenImageDenoise after rendering the complete image.
         */
        Builder& denoise();

        /**
         * Signals that a region within each render target has become available, typically used for
         * progress notification. This can be called from any thread.
         */
        Builder& tileCallback(TileCallback onTile, void* userData);

        /**
         * Signals that all render targets are now available.  This can be called from any thread.
         */
        Builder& doneCallback(DoneCallback onTile, void* userData);

        /**
         * Sets the number of secondary rays (samples per hemisphere), defaults to 256.
         */
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
