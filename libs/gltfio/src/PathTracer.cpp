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

#include "PathTracer.h"

#include <math/mat4.h>
#include <math/vec2.h>

#include <cgltf.h>

#include <utils/JobSystem.h>

#ifdef FILAMENT_HAS_EMBREE
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#endif

using namespace filament::math;

static constexpr uint16_t TILE_SIZE = 32;
static constexpr float inf = std::numeric_limits<float>::infinity();

struct PixelRectangle {
    ushort2 topLeft;
    ushort2 bottomRight;
};

static float3 sampleSphere() {
    float z = 2.0f * rand() / RAND_MAX - 1.0f;
    float t = 2.0f * rand() / RAND_MAX * M_PI;
    float r = std::sqrt(1.0f - z * z);
    return { r * std::cos(t), r * std::sin(t), z};
}

namespace gltfio {

PathTracer::Builder::Builder() {
    mPathTracer = new PathTracer();
}

PathTracer::Builder::~Builder() {
    delete mPathTracer;
}

PathTracer::Builder& PathTracer::Builder::renderTarget(image::LinearImage target) {
    mPathTracer->mRenderTarget = target;
    return *this;

}

PathTracer::Builder& PathTracer::Builder::sourceAsset(AssetHandle asset) {
    mPathTracer->mSourceAsset = asset;
    return *this;

}

PathTracer::Builder& PathTracer::Builder::filmCamera(const SimpleCamera& filmCamera) {
    mPathTracer->mFilmCamera = filmCamera;
    return *this;

}

PathTracer::Builder& PathTracer::Builder::uvCamera(const char* uvAttribute) {
    mPathTracer->mUvCamera = uvAttribute;
    return *this;

}

PathTracer::Builder& PathTracer::Builder::tileCallback(TileCallback onTile, void* userData) {
    mPathTracer->mTileCallback = onTile;
    mPathTracer->mTileUserData = userData;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::doneCallback(DoneCallback onDone, void* userData) {
    mPathTracer->mDoneCallback = onDone;
    mPathTracer->mDoneUserData = userData;
    return *this;
}

PathTracer PathTracer::Builder::build() {
    return *mPathTracer;
}

#ifndef FILAMENT_HAS_EMBREE

bool PathTracer::render() {
    puts("Embree is not available.");
    return false;
}

#else

// Note that the embree context can outlive the user-facing PathTracer object due to work
// occuring on multiple background threads.
struct EmbreeContext {
    image::LinearImage renderTarget;
    const cgltf_data* sourceAsset;
    SimpleCamera filmCamera;
    const char* uvCamera;
    PathTracer::TileCallback tileCallback;
    void* tileUserData;
    PathTracer::DoneCallback doneCallback;
    void* doneUserData;
    std::atomic<int> numRemainingTiles;
    RTCDevice embreeDevice;
    RTCScene embreeScene;
};

static void renderTile(EmbreeContext* context, PixelRectangle rect) {
    image::LinearImage& image = context->renderTarget;

    // Precompute some camera parameters.
    const SimpleCamera& camera = context->filmCamera;
    const float iw = 1.0f / image.getWidth();
    const float ih = 1.0f / image.getHeight();
    const float theta = camera.vfovDegrees * M_PI / 180;
    const float f = tanf(theta / 2);
    const float a = camera.aspectRatio;
    const float3 org = camera.eyePosition;
    const uint16_t hm1 = image.getHeight() - 1;

    // Compute the camera basis: view, right, and up vectors.
    const float3 v = normalize(camera.targetPosition - org);
    const float3 r = normalize(cross(v, camera.upVector));
    const float3 u = cross(r, v);

    // Given a pixel row and column, generate a ray from the eye through the film.
    auto generateCameraRay = [=] (uint16_t row, uint16_t col) {
        const uint16_t x = col;
        const uint16_t y = hm1 - row;
        const float s = (2.0f * (x + 0.5f) * iw - 1.0f);
        const float t = (2.0f * (y + 0.5f) * ih - 1.0f);
        const float3 dir = normalize(a * f * s * r - f * t * u + v);
        return RTCRay {
            .org_x = org.x,
            .org_y = org.y,
            .org_z = org.z,
            .tnear = 0,
            .dir_x = dir.x,
            .dir_y = dir.y,
            .dir_z = dir.z,
            .time = 0,
            .tfar = inf,
            .mask = 0xffffffff
        };
    };

    // Loop over all pixels in the tile.
    for (size_t row = rect.topLeft.y, len = rect.bottomRight.y; row < len; ++row) {
        for (size_t col = rect.topLeft.x, len = rect.bottomRight.x; col < len; ++col) {
            RTCIntersectContext intersector;
            rtcInitIntersectContext(&intersector);
            RTCRay ray = generateCameraRay(row, col);

            // For now perform a simple visibility test and set the pixel to white or black.
            rtcOccluded1(context->embreeScene, &intersector, &ray);
            float* dst = image.getPixelRef(col, row);
            *dst = (ray.tfar == -inf) ? 1.0f : 0.0f;
        }
    }

    // Signal that we're done with the tile. For simplicity we decrement an atomic tile count
    // to know when we're done with the entire image.
    context->tileCallback(image, rect.topLeft, rect.bottomRight, context->tileUserData);
    if (--context->numRemainingTiles == 0) {
        context->doneCallback(image, context->doneUserData);
        rtcReleaseScene(context->embreeScene);
        rtcReleaseDevice(context->embreeDevice);
        delete context;
    }
}

bool PathTracer::render() {
    auto sourceAsset = (const cgltf_data*) mSourceAsset;
    EmbreeContext* context = new EmbreeContext {
        .renderTarget = mRenderTarget,
        .sourceAsset = sourceAsset,
        .filmCamera = mFilmCamera,
        .uvCamera = mUvCamera,
        .tileCallback = mTileCallback,
        .tileUserData = mTileUserData,
        .doneCallback = mDoneCallback,
        .doneUserData = mDoneUserData
    };

    // Create the embree device and scene.
    auto device = context->embreeDevice = rtcNewDevice(nullptr);
    assert(device && "Unable to create embree device.");
    auto scene = context->embreeScene = rtcNewScene(device);
    assert(scene);

    // Create an embree Geometry object. For AO rendering we only care about positions and indices.
    auto createGeometry = [device](const cgltf_accessor* positions, const cgltf_accessor* indices) {
        assert(positions->component_type == cgltf_component_type_r_32f);
        assert(positions->type == cgltf_type_vec3);
        assert(indices->component_type == cgltf_component_type_r_32u);
        assert(indices->type == cgltf_type_scalar);

        auto geo = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                positions->buffer_view->buffer->data,
                positions->offset + positions->buffer_view->offset,
                positions->stride,
                positions->count);

        rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                indices->buffer_view->buffer->data,
                indices->offset + indices->buffer_view->offset,
                indices->stride * 3,
                indices->count / 3);

        rtcCommitGeometry(geo);
        return geo;
    };

    // Populate the embree mesh from a flattened glTF asset, which is guaranteed to have only
    // one mesh per node, and only one primitive per mesh.
    cgltf_node** nodes = sourceAsset->scene->nodes;
    for (cgltf_size i = 0, len = sourceAsset->scene->nodes_count; i < len; ++i) {
        assert(nodes[i]->mesh);
        assert(nodes[i]->mesh->primitives_count == 1);
        const cgltf_primitive& prim = nodes[i]->mesh->primitives[0];
        for (cgltf_size j = 0, len = prim.attributes_count; j < len; ++j) {
            const cgltf_attribute& attr = prim.attributes[j];
            if (attr.type == cgltf_attribute_type_position) {
                auto geo = createGeometry(attr.data, prim.indices);
                rtcAttachGeometry(scene, geo);
                rtcReleaseGeometry(geo);
            }
        }
    }
    rtcCommitScene(scene);

    // Compute the number of jobs by pre-running the loop.
    const size_t width = mRenderTarget.getWidth();
    const size_t height = mRenderTarget.getHeight();
    int numTiles = 0;
    for (size_t row = 0; row < height; row += TILE_SIZE) {
        for (size_t col = 0; col < width; col += TILE_SIZE, ++numTiles);
    }
    context->numRemainingTiles = numTiles;

    // Kick off one job per tile.
    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    utils::JobSystem::Job* parent = js->createJob();
    for (size_t row = 0; row < height; row += TILE_SIZE) {
        for (size_t col = 0; col < width; col += TILE_SIZE) {
            PixelRectangle rect;
            rect.topLeft = {col, row};
            rect.bottomRight = {col + TILE_SIZE, row + TILE_SIZE};
            rect.bottomRight.x = std::min(rect.bottomRight.x, (uint16_t) width);
            rect.bottomRight.y = std::min(rect.bottomRight.y, (uint16_t) height);
            utils::JobSystem::Job* tile = utils::jobs::createJob(*js, parent, [context, rect] {
                renderTile(context, rect);
            });
            js->run(tile);
        }
    }
    js->run(parent);

    return true;
}

#endif // FILAMENT_HAS_EMBREE

} // namespace gltfio
