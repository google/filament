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

#include <rays/PathTracer.h>

#include <image/ImageOps.h>
#include <image/LinearImage.h>

#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec2.h>

#include <utils/JobSystem.h>

#ifdef FILAMENT_HAS_EMBREE
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#define FILAMENT_HAS_DENOISE 1
#endif

#ifdef FILAMENT_HAS_DENOISE
#include <OpenImageDenoise/oidn.h>
#endif

using namespace filament::math;
using namespace image;

static constexpr size_t MIN_TILE_SIZE = 32;
static constexpr size_t MAX_TILES_COUNT = 2048;

static constexpr float inf = std::numeric_limits<float>::infinity();

static constexpr float EMPTY_SENTINEL = 2.0f;

struct PixelRectangle {
    ushort2 topLeft;
    ushort2 bottomRight;
};

// TODO: the following two functions should be shared with libs/ibl

static double3 hemisphereCosSample(double2 u) {
    const double phi = 2 * F_PI * u.x;
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

namespace filament {
namespace rays {

PathTracer::Builder& PathTracer::Builder::outputPlane(OutputPlane target, LinearImage image) {
    mConfig.renderTargets[(int) target] = image;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::meshes(const SimpleMesh* meshes, size_t numMeshes) {
    mConfig.meshes = meshes;
    mConfig.numMeshes = numMeshes;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::filmCamera(const SimpleCamera& filmCamera) {
    mConfig.filmCamera = filmCamera;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::uvCamera(bool dilate) {
    mConfig.uvCamera = true;
    mConfig.dilate = dilate;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::denoise(bool enable) {
    mConfig.denoise = enable;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::tileCallback(TileCallback onTile, void* userData) {
    mConfig.tileCallback = onTile;
    mConfig.tileUserData = userData;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::doneCallback(DoneCallback onDone, void* userData) {
    mConfig.doneCallback = onDone;
    mConfig.doneUserData = userData;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::samplesPerPixel(size_t numSamples) {
    mConfig.samplesPerPixel = numSamples;
    return *this;
}

PathTracer::Builder& PathTracer::Builder::occlusionRayBounds(float aoRayNear, float aoRayFar) {
    mConfig.aoRayNear = aoRayNear;
    mConfig.aoRayFar = aoRayFar;
    return *this;
}

PathTracer PathTracer::Builder::build() {
    // TODO: check for valid configuration (consistent sizes etc)
    return PathTracer(mConfig);
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
    PathTracer::Config config;
    std::atomic<int> numRemainingTiles;
    RTCDevice device;
    RTCScene scene;
};

static void renderTile(EmbreeContext* context, PixelRectangle rect) {
    LinearImage& ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    RTCScene embreeScene = context->scene;
    const float inverseSampleCount = 1.0f / context->config.samplesPerPixel;

    // Precompute some camera parameters.
    const SimpleCamera& camera = context->config.filmCamera;
    const float tnear = context->config.aoRayNear;
    const float tfar = context->config.aoRayFar;
    const float iw = 1.0f / ao.getWidth();
    const float ih = 1.0f / ao.getHeight();
    const float theta = camera.vfovDegrees * F_PI / 180;
    const float f = tanf(theta / 2);
    const float a = camera.aspectRatio;
    const float3 org = camera.eyePosition;
    const uint16_t hm1 = ao.getHeight() - 1;

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
                for (size_t nsamp = 0, len = context->config.samplesPerPixel; nsamp < len; nsamp++) {
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
                ao.getPixelRef(col, row)[0] = 1.0f - sum * inverseSampleCount;
            }
        }
    }
}

static void renderTileToGbuffer(EmbreeContext* context, PixelRectangle rect) {
    LinearImage& ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    LinearImage& meshNormals = context->config.renderTargets[(int) MESH_NORMALS];
    LinearImage& meshPositions = context->config.renderTargets[(int) MESH_POSITIONS];
    RTCScene embreeScene = context->scene;

    const float tnear = context->config.aoRayNear;
    const float tfar = context->config.aoRayFar;
    const float iw = 1.0f / ao.getWidth();
    const float ih = 1.0f / ao.getHeight();

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

            float* position = meshPositions.getPixelRef(col, row);
            float* normal = meshNormals.getPixelRef(col, row);
            if (rayhit.ray.tfar != inf) {
                RTCGeometry geo = rtcGetGeometry(embreeScene, rayhit.hit.geomID);
                rtcInterpolate0(geo, rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, position, 3);
                rtcInterpolate0(geo, rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, normal, 3);

                // AO won't be computed until the second pass, but we show an instant preview of
                // the chart shapes by setting a placeholder value in the AO map.
                ao.getPixelRef(col, row)[0] = 0.5f;
            } else {
                normal[0] = EMPTY_SENTINEL;
            }
        }
    }
}

static void dilateCharts(EmbreeContext* context) {
    LinearImage& ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    LinearImage& meshNormals = context->config.renderTargets[(int) MESH_NORMALS];
    LinearImage& bentNormals = context->config.renderTargets[(int) BENT_NORMALS];
    auto presence = [] (const LinearImage& normals, uint32_t col, uint32_t row, void*) {
        return normals.getPixelRef(col, row)[0] != EMPTY_SENTINEL;
    };
    LinearImage coords = image::computeCoordField(meshNormals, presence, nullptr);
    LinearImage dilated = image::voronoiFromCoordField(coords, ao);
    blitImage(ao, dilated);
    dilated = image::voronoiFromCoordField(coords, bentNormals);
    blitImage(bentNormals, dilated);
}

static void denoise(EmbreeContext* context) {
#ifdef FILAMENT_HAS_DENOISE
    LinearImage ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    const LinearImage& meshNormals = context->config.renderTargets[(int) MESH_NORMALS];

    // The denoiser requires color inputs, so convert 1-chan to 3-chan.
    const size_t width = ao.getWidth();
    const size_t height = ao.getHeight();
    LinearImage denoiseSource = combineChannels({ ao, ao, ao });

    // Construct a fake albedo image, which for our purposes can be white everywhere that a surface
    // is present. This is optional but the denoise library doesn't produce good results without it.
    LinearImage fakeAlbedo(width, height, 3);
    for (int32_t row = 0; row < height; ++row) {
        for (uint32_t col = 0; col < width; ++col) {
            const float* normal = meshNormals.getPixelRef(col, row);
            float* albedo = fakeAlbedo.getPixelRef(col, row);
            albedo[0] = albedo[1] = albedo[2] = (normal[0] == EMPTY_SENTINEL ? 0.0f : 1.0f);
        }
    }

    // Invoke the denoiser.
    LinearImage denoiseTarget(width, height, 3);
    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
    oidnCommitDevice(device);
    OIDNFilter filter = oidnNewFilter(device, "RT");
    oidnSetSharedFilterImage(filter, "color",  denoiseSource.getPixelRef(),
                            OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0);
    oidnSetSharedFilterImage(filter, "normal",  (void*) meshNormals.getPixelRef(),
                            OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0);
    oidnSetSharedFilterImage(filter, "albedo",  fakeAlbedo.getPixelRef(),
                            OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0);
    oidnSetSharedFilterImage(filter, "output", denoiseTarget.getPixelRef(),
                            OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0);
    oidnCommitFilter(filter);
    oidnExecuteFilter(filter);

    // Check for errors.
    const char* errorMessage;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE) {
        printf("OpenImageDenoise Error: %s\n", errorMessage);
        oidnReleaseFilter(filter);
        oidnReleaseDevice(device);
        return;
    }
    oidnReleaseFilter(filter);
    oidnReleaseDevice(device);

    blitImage(ao, extractChannel(denoiseTarget, 0));
#endif
}

static void renderTileFromGbuffer(EmbreeContext* context, PixelRectangle rect) {
    LinearImage& ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    LinearImage& meshNormals = context->config.renderTargets[(int) MESH_NORMALS];
    LinearImage& meshPositions = context->config.renderTargets[(int) MESH_POSITIONS];
    LinearImage& bentNormals = context->config.renderTargets[(int) BENT_NORMALS];
    RTCScene embreeScene = context->scene;
    const float inverseSampleCount = 1.0f / context->config.samplesPerPixel;

    const float tnear = context->config.aoRayNear;
    const float tfar = context->config.aoRayFar;
    const size_t spp = context->config.samplesPerPixel;

    for (size_t row = rect.topLeft.y, len = rect.bottomRight.y; row < len; ++row) {
        for (size_t col = rect.topLeft.x, len = rect.bottomRight.x; col < len; ++col) {
            RTCIntersectContext intersector;
            rtcInitIntersectContext(&intersector);
            intersector.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;
            float* position = meshPositions.getPixelRef(col, row);
            float* normal = meshNormals.getPixelRef(col, row);
            if (normal[0] != EMPTY_SENTINEL) {
                float3 n = { normal[0], normal[1], normal[2] };
                float3 b = randomPerp(n);
                float3 t = cross(n, b);
                mat3 tangentFrame = {t, b, n};
                RTCRay aoray { .org_x = position[0], .org_y = position[1], .org_z = position[2] };
                float sum = 0;
                float3 bentNormal = {0, 0, 0};
                for (size_t nsamp = 0; nsamp < spp; nsamp++) {
                    const double2 u = hammersley(nsamp, inverseSampleCount);
                    const float3 dir = tangentFrame * hemisphereCosSample(u);
                    aoray.dir_x = dir.x;
                    aoray.dir_y = dir.y;
                    aoray.dir_z = dir.z;
                    aoray.tnear = tnear;
                    aoray.tfar = tfar;
                    rtcOccluded1(embreeScene, &intersector, &aoray);
                    if (aoray.tfar == -inf) {
                        bentNormal += normalize(dir);
                        sum += 1.0f;
                    }
                }
                if (bentNormals) {
                    bentNormal = normalize(bentNormal / sum);
                    float* pBentNormal = bentNormals.getPixelRef(col, row);
                    pBentNormal[0] = bentNormal[0];
                    pBentNormal[1] = bentNormal[1];
                    pBentNormal[2] = bentNormal[2];
                }
                ao.getPixelRef(col, row)[0] = 1.0f - sum * inverseSampleCount;
            }
        }
    }
}

template <typename RenderFn, typename CompletionFn>
void spawnTileJobs(EmbreeContext* context, RenderFn render, CompletionFn done) {
    LinearImage& ao = context->config.renderTargets[(int) AMBIENT_OCCLUSION];
    const size_t width = ao.getWidth();
    const size_t height = ao.getHeight();

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
    context->numRemainingTiles = numTiles;

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
                if (--context->numRemainingTiles == 0) {
                    done(context);
                }
            });
            js->run(tile);
        }
    }
    js->run(parent);
}

bool PathTracer::render() {
    const LinearImage& ao = mConfig.renderTargets[(int) AMBIENT_OCCLUSION];
    const size_t width = ao.getWidth();
    const size_t height = ao.getHeight();

    EmbreeContext* context = new EmbreeContext { .config = mConfig };

    if (!context->config.renderTargets[(int) MESH_NORMALS]) {
        context->config.renderTargets[(int) MESH_NORMALS] = LinearImage(width, height, 3);
    }

    if (!context->config.renderTargets[(int) MESH_POSITIONS]) {
        context->config.renderTargets[(int) MESH_POSITIONS] = LinearImage(width, height, 3);
    }

    if (!context->config.tileCallback) {
        context->config.tileCallback = [] (ushort2, ushort2, void* userData) {};
    }

    if (!context->config.doneCallback) {
        context->config.doneCallback = [] (void* userData) {};
    }

    // Create the embree device.
    RTCDevice device = context->device = rtcNewDevice(nullptr);
    rtcSetDeviceErrorFunction(device, [](void* userPtr, RTCError code, const char* str) {
        printf("Embree error: %s.\n", str);
    }, nullptr);

    // Populates an embree scene from 3D position data, ignoring normals and UVs.
    auto populate3DScene = [=] () {
        for (size_t i = 0; i < context->config.numMeshes; ++i) {
            const SimpleMesh& mesh = context->config.meshes[i];
            RTCGeometry geo = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                    mesh.positions, 0, mesh.positionsStride, mesh.numVertices);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                    mesh.indices, 0, sizeof(uint32_t) * 3, mesh.numIndices / 3);
            rtcCommitGeometry(geo);
            rtcAttachGeometry(context->scene, geo);
            rtcReleaseGeometry(geo);
        }
        rtcCommitScene(context->scene);
    };

    // Populates an embree scene from 2D position data sourced from UV rather than the standard 3D
    // vertex positions.
    auto populate2DScene = [=] () {
        for (size_t i = 0; i < context->config.numMeshes; ++i) {
            const SimpleMesh& mesh = context->config.meshes[i];
            RTCGeometry geo = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
            rtcSetGeometryVertexAttributeCount(geo, 2);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                    mesh.uvs, 0, mesh.uvsStride, mesh.numVertices);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3,
                    mesh.positions, 0, mesh.positionsStride, mesh.numVertices);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT3,
                    mesh.normals, 0, mesh.normalsStride, mesh.numVertices);
            rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                    mesh.indices, 0, sizeof(uint32_t) * 3, mesh.numIndices / 3);
            rtcCommitGeometry(geo);
            rtcAttachGeometry(context->scene, geo);
            rtcReleaseGeometry(geo);
        }
        rtcCommitScene(context->scene);
    };

    if (mConfig.uvCamera) {

        context->scene = rtcNewScene(device);
        populate2DScene();

        // First render XYZ positions into the G-Buffer.
        spawnTileJobs(context, [](EmbreeContext* context, PixelRectangle rect) {
            renderTileToGbuffer(context, rect);
            context->config.tileCallback(rect.topLeft, rect.bottomRight,
                    context->config.tileUserData);
        }, [populate3DScene](EmbreeContext* context) {

            rtcReleaseScene(context->scene);

            // Now that the G-Buffer is ready, render the 3D scene.
            context->scene = rtcNewScene(context->device);
            populate3DScene();
            spawnTileJobs(context, [](EmbreeContext* context, PixelRectangle rect) {
                renderTileFromGbuffer(context, rect);
                context->config.tileCallback(rect.topLeft, rect.bottomRight,
                        context->config.tileUserData);
            }, [](EmbreeContext* context) {
                if (context->config.dilate) {
                    dilateCharts(context);
                }
                if (context->config.denoise) {
                    denoise(context);
                }
                context->config.doneCallback(context->config.doneUserData);
                rtcReleaseScene(context->scene);
                rtcReleaseDevice(context->device);
                delete context;
            });
        });

    } else {

        // For the simple case of rendering from a film camera, we need only one pass.
        context->scene = rtcNewScene(device);
        populate3DScene();
        spawnTileJobs(context, [](EmbreeContext* context, PixelRectangle rect) {
            renderTile(context, rect);
            context->config.tileCallback(rect.topLeft, rect.bottomRight,
                    context->config.tileUserData);
        }, [](EmbreeContext* context) {
            if (context->config.denoise) {
                denoise(context);
            }
            context->config.doneCallback(context->config.doneUserData);
            rtcReleaseScene(context->scene);
            rtcReleaseDevice(context->device);
            delete context;
        });
    }

    return true;
}

#endif // FILAMENT_HAS_EMBREE

} // namespace filament
} // namespace rays
