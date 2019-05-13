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

static constexpr size_t MIN_TILE_SIZE = 32;
static constexpr size_t MAX_TILES_COUNT = 2048;

static constexpr float inf = std::numeric_limits<float>::infinity();

struct PixelRectangle {
    ushort2 topLeft;
    ushort2 bottomRight;
};

// TODO: the following two functions should be shared with libs/ibl

static double3 hemisphereCosSample(double2 u) {
    const double phi = 2 * M_PI * u.x;
    const double cosTheta2 = 1 - u.y;
    const double cosTheta = std::sqrt(cosTheta2);
    const double sinTheta = std::sqrt(1 - cosTheta2);
    return { sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta };
}

inline double2 hammersley(uint32_t i, float iN) {
    constexpr float tof = 0.5f / 0x80000000U;
    uint32_t bits = i;
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
    return { i * iN, bits * tof };
}

static float3 randomPerp(float3 n) {
    float3 perp = cross(n, float3{1, 0, 0});
    float sqrlen = dot(perp, perp);
    if (sqrlen <= std::numeric_limits<float>::epsilon()) {
        perp = cross(n, float3{0, 1, 0});
        sqrlen = dot(perp, perp);
    }
    return perp / sqrlen;
};

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

PathTracer::Builder& PathTracer::Builder::samplesPerPixel(size_t numSamples) {
    mPathTracer->mSamplesPerPixel = numSamples;
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
    size_t samplesPerPixel;
    float aoRayNear;
    float aoRayFar;
    struct {
        std::atomic<int> numRemainingTiles;
        image::LinearImage gbuffer;
        RTCDevice device;
        RTCScene scene;
    } details;
};

static void renderTile(EmbreeContext* context, PixelRectangle rect) {
    image::LinearImage& image = context->renderTarget;
    RTCScene embreeScene = context->details.scene;
    const float inverseSampleCount = 1.0f / context->samplesPerPixel;

    // Precompute some camera parameters.
    const SimpleCamera& camera = context->filmCamera;
    const float tnear = context->aoRayNear;
    const float tfar = context->aoRayFar;
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

            RTCRayHit rayhit { .ray = ray };
            intersector.flags = RTC_INTERSECT_CONTEXT_FLAG_COHERENT;
            rtcIntersect1(embreeScene, &intersector, &rayhit);
            if (rayhit.ray.tfar != inf) {

                intersector.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;

                // TODO: For now we are using the geometric normal provided by embree which is not
                // necessarily normalized.
                float3 n = normalize(float3 { rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z });
                float3 b = randomPerp(n);
                float3 t = cross(n, b);
                mat3 tangentFrame = {t, b, n};

                RTCRay aoray {
                    .org_x = rayhit.ray.org_x + rayhit.ray.dir_x * rayhit.ray.tfar,
                    .org_y = rayhit.ray.org_y + rayhit.ray.dir_y * rayhit.ray.tfar,
                    .org_z = rayhit.ray.org_z + rayhit.ray.dir_z * rayhit.ray.tfar
                };

                float sum = 0;
                for (size_t nsamp = 0, len = context->samplesPerPixel; nsamp < len; nsamp++) {
                    const double2 u = hammersley(nsamp, inverseSampleCount);
                    const float3 dir = tangentFrame * hemisphereCosSample(u);
                    aoray.dir_x = dir.x;
                    aoray.dir_y = dir.y;
                    aoray.dir_z = dir.z;
                    aoray.tnear = tnear;
                    aoray.tfar = tfar;
                    rtcOccluded1(embreeScene, &intersector, &aoray);
                    if (aoray.tfar == -inf) {
                        sum += 1.0f;
                    }
                }
                image.getPixelRef(col, row)[0] = 1.0f - sum * inverseSampleCount;
            }
        }
    }
}

static void renderTileToGbuffer(EmbreeContext* context, PixelRectangle rect) {
    image::LinearImage& image = context->renderTarget;
    image::LinearImage& gbuffer = context->details.gbuffer;
    RTCScene embreeScene = context->details.scene;

    const float tnear = context->aoRayNear;
    const float tfar = context->aoRayFar;
    const float iw = 1.0f / image.getWidth();
    const float ih = 1.0f / image.getHeight();

    auto generateOrthoCameraRay = [=] (uint16_t row, uint16_t col) {
        return RTCRay {
            .org_x = float(col) * iw,
            .org_y = float(row) * ih,
            .org_z = 1,
            .tnear = 0,
            .dir_x = 0,
            .dir_y = 0,
            .dir_z = -1,
            .time = 0,
            .tfar = inf,
            .mask = 0xffffffff
        };
    };

    for (size_t row = rect.topLeft.y, len = rect.bottomRight.y; row < len; ++row) {
        for (size_t col = rect.topLeft.x, len = rect.bottomRight.x; col < len; ++col) {
            RTCIntersectContext intersector;
            rtcInitIntersectContext(&intersector);

            RTCRay ray = generateOrthoCameraRay(row, col);
            RTCRayHit rayhit { .ray = ray };
            intersector.flags = RTC_INTERSECT_CONTEXT_FLAG_COHERENT;
            rtcIntersect1(embreeScene, &intersector, &rayhit);

            if (rayhit.ray.tfar != inf) {
                image.getPixelRef(col, row)[0] = 1.0f;
                float* position = gbuffer.getPixelRef(col, row);
                float* normal = position + 3;
                RTCGeometry geo = rtcGetGeometry(embreeScene, rayhit.hit.geomID);
                rtcInterpolate0(geo, rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, position, 3);
                rtcInterpolate0(geo, rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, normal, 3);
            }
        }
    }
}

static void renderTileFromGbuffer(EmbreeContext* context, PixelRectangle rect) {
    image::LinearImage& image = context->renderTarget;
    image::LinearImage& gbuffer = context->details.gbuffer;
    RTCScene embreeScene = context->details.scene;
    const float inverseSampleCount = 1.0f / context->samplesPerPixel;

    const float tnear = context->aoRayNear;
    const float tfar = context->aoRayFar;

    for (size_t row = rect.topLeft.y, len = rect.bottomRight.y; row < len; ++row) {
        for (size_t col = rect.topLeft.x, len = rect.bottomRight.x; col < len; ++col) {
            RTCIntersectContext intersector;
            rtcInitIntersectContext(&intersector);
            intersector.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;
            float* p = gbuffer.getPixelRef(col, row);
            float* normal = p + 3;
            if (normal[0] || normal[1] || normal[2]) {
                float* position = gbuffer.getPixelRef(col, row);
                float* normal = position + 3;
                float3 n = { normal[0], normal[1], normal[2] };
                float3 b = randomPerp(n);
                float3 t = cross(n, b);
                mat3 tangentFrame = {t, b, n};
                RTCRay aoray { .org_x = position[0], .org_y = position[1], .org_z = position[2] };
                float sum = 0;
                for (size_t nsamp = 0, len = context->samplesPerPixel; nsamp < len; nsamp++) {
                    const double2 u = hammersley(nsamp, inverseSampleCount);
                    const float3 dir = tangentFrame * hemisphereCosSample(u);
                    aoray.dir_x = dir.x;
                    aoray.dir_y = dir.y;
                    aoray.dir_z = dir.z;
                    aoray.tnear = tnear;
                    aoray.tfar = tfar;
                    rtcOccluded1(embreeScene, &intersector, &aoray);
                    if (aoray.tfar == -inf) {
                        sum += 1.0f;
                    }
                }
                image.getPixelRef(col, row)[0] = 1.0f - sum * inverseSampleCount;
            }
        }
    }
}

template <typename RenderFn, typename CompletionFn>
void spawnTileJobs(EmbreeContext* context, RenderFn render, CompletionFn done) {
    const size_t width = context->renderTarget.getWidth();
    const size_t height = context->renderTarget.getHeight();

    // Compute a reasonable tile size that will not create too many jobs.
    int numTiles = 0;
    size_t tileSize = MIN_TILE_SIZE;
    while (true) {
        int numCols = (width + tileSize - 1) / tileSize;
        int numRows = (height + tileSize - 1) / tileSize;
        numTiles = numCols * numRows;
        if (numTiles <= MAX_TILES_COUNT) {
            break;
        }
        tileSize *= 2;
    }
    context->details.numRemainingTiles = numTiles;

    // Spawn one job per tile.
    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    utils::JobSystem::Job* parent = js->createJob();
    for (size_t row = 0; row < height; row += tileSize) {
        for (size_t col = 0; col < width; col += tileSize) {
            PixelRectangle rect;
            rect.topLeft = {col, row};
            rect.bottomRight = {col + tileSize, row + tileSize};
            rect.bottomRight.x = std::min(rect.bottomRight.x, (uint16_t) width);
            rect.bottomRight.y = std::min(rect.bottomRight.y, (uint16_t) height);
            utils::JobSystem::Job* tile = utils::jobs::createJob(*js, parent, [=] {
                render(context, rect);
                // Decrement an atomic tile count to know when we're done.
                if (--context->details.numRemainingTiles == 0) {
                    done(context);
                }

            });
            js->run(tile);
        }
    }
    js->run(parent);
}

bool PathTracer::render() {
    auto sourceAsset = (const cgltf_data*) mSourceAsset;
    const size_t width = mRenderTarget.getWidth();
    const size_t height = mRenderTarget.getHeight();
    const cgltf_node** nodes = (const cgltf_node**) sourceAsset->scene->nodes;

    EmbreeContext* context = new EmbreeContext {
        .renderTarget = mRenderTarget,
        .sourceAsset = sourceAsset,
        .filmCamera = mFilmCamera,
        .uvCamera = mUvCamera,
        .tileCallback = mTileCallback,
        .tileUserData = mTileUserData,
        .doneCallback = mDoneCallback,
        .doneUserData = mDoneUserData,
        .samplesPerPixel = mSamplesPerPixel,
        .aoRayNear = mAoRayNear,
        .aoRayFar = mAoRayFar
    };

    // Create the embree device.
    auto device = context->details.device = rtcNewDevice(nullptr);
    assert(device && "Unable to create embree device.");

    rtcSetDeviceErrorFunction(device, [](void* userPtr, RTCError code, const char* str) {
        printf("Embree error: %s.\n", str);
    }, nullptr);

    // Creates an embree Geometry object from 3D position data.
    auto createGeo3D = [device](const cgltf_accessor* positions, const cgltf_accessor* indices) {
        assert(positions->component_type == cgltf_component_type_r_32f);
        assert(positions->type == cgltf_type_vec3);
        assert(indices->component_type == cgltf_component_type_r_32u);
        assert(indices->type == cgltf_type_scalar);

        RTCGeometry geo = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

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

    // Creates an embree Geometry object from 2D position data by setting Z = 0.
    auto createGeo2D = [device](const cgltf_accessor* positions2d,
            const cgltf_accessor* attrib0, const cgltf_accessor* attrib1,
            const cgltf_accessor* indices) {

        assert(positions2d->component_type == cgltf_component_type_r_32f);
        assert(positions2d->type == cgltf_type_vec2);

        assert(attrib0->component_type == cgltf_component_type_r_32f);
        assert(attrib0->type == cgltf_type_vec3);

        assert(attrib1->component_type == cgltf_component_type_r_32f);
        assert(attrib1->type == cgltf_type_vec3);

        assert(indices->component_type == cgltf_component_type_r_32u);
        assert(indices->type == cgltf_type_scalar);

        RTCGeometry geo = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        rtcSetGeometryVertexAttributeCount(geo, 2);

        float* dstpositions = (float*) rtcSetNewGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX, 0,
                RTC_FORMAT_FLOAT3, sizeof(float3), positions2d->count);
        float* dstattrib0 = (float*) rtcSetNewGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
                0, RTC_FORMAT_FLOAT3, sizeof(float3), attrib0->count);
        float* dstattrib1 = (float*) rtcSetNewGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
                1, RTC_FORMAT_FLOAT3, sizeof(float3), attrib1->count);

        for (size_t i = 0; i < positions2d->count; ++i) {
            cgltf_accessor_read_float(positions2d, i, dstpositions, 2);
            cgltf_accessor_read_float(attrib0, i, dstattrib0, 3);
            cgltf_accessor_read_float(attrib1, i, dstattrib1, 3);
            dstpositions[2] = 0.0f;
            dstpositions += 3;
            dstattrib0 += 3;
            dstattrib1 += 3;
        }

        rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                indices->buffer_view->buffer->data, indices->offset + indices->buffer_view->offset,
                indices->stride * 3, indices->count / 3);

        rtcCommitGeometry(geo);
        return geo;
    };

    // Populates an embree scene from a flattened glTF asset, which is guaranteed to have only one
    // mesh per node and only one primitive per mesh. 
    auto populate3DScene = [=] () {
        RTCScene scene = context->details.scene;
        for (cgltf_size i = 0, len = sourceAsset->scene->nodes_count; i < len; ++i) {
            assert(nodes[i]->mesh);
            assert(nodes[i]->mesh->primitives_count == 1);
            const cgltf_primitive& prim = nodes[i]->mesh->primitives[0];
            for (cgltf_size j = 0, len = prim.attributes_count; j < len; ++j) {
                const cgltf_attribute& attr = prim.attributes[j];
                if (attr.type == cgltf_attribute_type_position) {
                    auto geo = createGeo3D(attr.data, prim.indices);
                    rtcAttachGeometry(scene, geo);
                    rtcReleaseGeometry(geo);
                    break;
                }
            }
        }
        rtcCommitScene(scene);
    };

    // If we're baking (non-null UV camera) then we first use 2D position data sourced from UV
    // rather than the standard 3D vertex positions.
    if (mUvCamera) {

        context->details.scene = rtcNewScene(device);
        for (cgltf_size i = 0, len = sourceAsset->scene->nodes_count; i < len; ++i) {
            assert(nodes[i]->mesh);
            assert(nodes[i]->mesh->primitives_count == 1);
            const cgltf_primitive& prim = nodes[i]->mesh->primitives[0];
            RTCGeometry geo = nullptr;

            // Collect UV positions, world-space positions, and normals.
            const cgltf_attribute* pos2d = nullptr;
            const cgltf_attribute* attr0 = nullptr;
            const cgltf_attribute* attr1 = nullptr;
            for (cgltf_size j = 0, len = prim.attributes_count; j < len; ++j) {
                const cgltf_attribute& attr = prim.attributes[j];
                if (!strcmp(attr.name, mUvCamera)) {
                    pos2d = &attr;
                }
                if (attr.type == cgltf_attribute_type_position) {
                    attr0 = &attr;
                }
                if (attr.type == cgltf_attribute_type_normal) {
                    attr1 = &attr;
                }
            }
            if (pos2d && attr0 && attr1) {
                geo = createGeo2D(pos2d->data, attr0->data, attr1->data, prim.indices);
                rtcAttachGeometry(context->details.scene, geo);
                rtcReleaseGeometry(geo);
            } else {
                printf("Expected attributes not found for mesh %zu of %zu.\n", i, len);
            }
        }
        rtcCommitScene(context->details.scene);

        context->details.gbuffer = image::LinearImage(width, height, 6);

        // First render XYZ positions into the G-Buffer.
        spawnTileJobs(context, [populate3DScene](EmbreeContext* context, PixelRectangle rect) {
            renderTileToGbuffer(context, rect);
            context->tileCallback(context->renderTarget, rect.topLeft, rect.bottomRight,
                    context->tileUserData);
        }, [populate3DScene](EmbreeContext* context) {

            rtcReleaseScene(context->details.scene);

            // Now that the G-Buffer is ready, render the 3D scene.
            context->details.scene = rtcNewScene(context->details.device);
            populate3DScene();
            spawnTileJobs(context, [](EmbreeContext* context, PixelRectangle rect) {
                renderTileFromGbuffer(context, rect);
                context->tileCallback(context->renderTarget, rect.topLeft, rect.bottomRight,
                        context->tileUserData);
            }, [](EmbreeContext* context) {
                context->doneCallback(context->renderTarget, context->doneUserData);
                rtcReleaseScene(context->details.scene);
                rtcReleaseDevice(context->details.device);
                delete context;
            });
        });

    } else {

        // For the simple case of rendering from a film camera, we need only one pass.
        context->details.scene = rtcNewScene(device);
        populate3DScene();
        spawnTileJobs(context, [](EmbreeContext* context, PixelRectangle rect) {
            renderTile(context, rect);
            context->tileCallback(context->renderTarget, rect.topLeft, rect.bottomRight,
                    context->tileUserData);
        }, [](EmbreeContext* context) {
            context->doneCallback(context->renderTarget, context->doneUserData);
            rtcReleaseScene(context->details.scene);
            rtcReleaseDevice(context->details.device);
            delete context;
        });
    }

    return true;
}

#endif // FILAMENT_HAS_EMBREE

} // namespace gltfio
