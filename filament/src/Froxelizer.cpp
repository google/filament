/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "Froxelizer.h"

#include "Allocators.h"
#include "Intersections.h"

#include "details/Engine.h"
#include "details/Scene.h"

#include <private/filament/EngineEnums.h>
#include <private/backend/DriverApi.h>

#include <filament/Box.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/BinaryTreeArray.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Slice.h>
#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/fast.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <utils/architecture.h>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

// TODO: these should come from a configuration object on View or Camera
static constexpr size_t FROXEL_SLICE_COUNT = 16;
static constexpr float FROXEL_FIRST_SLICE_DEPTH = 5;
static constexpr float FROXEL_LAST_SLICE_DISTANCE = 100;

// The record buffer is limited by both the UBO size and our use of 16-bits indices.
constexpr size_t RECORD_BUFFER_ENTRY_COUNT  = CONFIG_MINSPEC_UBO_SIZE;    // 16 KiB UBO minspec

// Buffer needed for Froxelizer internal data structures (~256 KiB)
constexpr size_t PER_FROXELDATA_ARENA_SIZE = sizeof(float4) *
                                                 (FROXEL_BUFFER_MAX_ENTRY_COUNT +
                                                  FROXEL_BUFFER_MAX_ENTRY_COUNT + 3 +
                                                  FROXEL_SLICE_COUNT / 4 + 1);

// number of lights processed by one group (e.g. 32)
static constexpr size_t LIGHT_PER_GROUP = sizeof(Froxelizer::LightGroupType) * 8;

// number of groups (i.e. jobs) to use for froxelization (e.g. 8)
static constexpr size_t GROUP_COUNT =
        (CONFIG_MAX_LIGHT_COUNT + LIGHT_PER_GROUP - 1) / LIGHT_PER_GROUP;

// This depends on the maximum number of lights (currently 256)
static_assert(CONFIG_MAX_LIGHT_INDEX <= std::numeric_limits<Froxelizer::RecordBufferType>::max(),
        "can't have more than 256 lights");

// Record buffer cannot be larger than 65K entries because froxels use uint16_t to store indices
// to it.
static_assert(RECORD_BUFFER_ENTRY_COUNT <= 65536,
        "RecordBuffer cannot be larger than 65536 entries");

static_assert(RECORD_BUFFER_ENTRY_COUNT <= CONFIG_MINSPEC_UBO_SIZE,
        "RecordBuffer cannot be larger than the UBO minspec (16KiB)");

struct Froxelizer::FroxelThreadData :
        public std::array<LightGroupType, FROXEL_BUFFER_MAX_ENTRY_COUNT> {
};


// Returns false if the two matrices are different. May return false if they're the
// same, with some elements only differing by +0 or -0. Behaviour is undefined with NaNs.
static bool fuzzyEqual(mat4f const& UTILS_RESTRICT l, mat4f const& UTILS_RESTRICT r) noexcept {
    auto const li = reinterpret_cast<uint32_t const*>( reinterpret_cast<char const*>(&l) );
    auto const ri = reinterpret_cast<uint32_t const*>( reinterpret_cast<char const*>(&r) );
    uint32_t result = 0;
    for (size_t i = 0; i < sizeof(mat4f) / sizeof(uint32_t); i++) {
        // clang fully vectorizes this
        result |= li[i] ^ ri[i];
    }
    return result == 0;
}

size_t Froxelizer::getFroxelBufferByteCount(FEngine::DriverApi& driverApi) noexcept {
    // Make sure that targetSize is 16-byte aligned so that it'll fit properly into an array of
    // uvec4.
    size_t const targetSize = (driverApi.getMaxUniformBufferSize() / 16) * 16;
    return std::min(FROXEL_BUFFER_MAX_ENTRY_COUNT * sizeof(FroxelEntry), targetSize);
}

Froxelizer::Froxelizer(FEngine& engine)
        : mArena("froxel", PER_FROXELDATA_ARENA_SIZE),
          mZLightNear(FROXEL_FIRST_SLICE_DEPTH),
          mZLightFar(FROXEL_LAST_SLICE_DISTANCE)
{
    static_assert(std::is_same_v<RecordBufferType, uint8_t>,
            "Record Buffer must use bytes");

    DriverApi& driverApi = engine.getDriverApi();

    if (UTILS_UNLIKELY(driverApi.getFeatureLevel() == FeatureLevel::FEATURE_LEVEL_0)) {
        return;
    }

    size_t const froxelBufferByteCount = getFroxelBufferByteCount(engine.getDriverApi());
    mFroxelBufferEntryCount = froxelBufferByteCount / sizeof(FroxelEntry);

    mRecordsBuffer = driverApi.createBufferObject(RECORD_BUFFER_ENTRY_COUNT,
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    mFroxelsBuffer = driverApi.createBufferObject(
            froxelBufferByteCount,
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);
}

Froxelizer::~Froxelizer() {
    // make sure we called terminate()
}

void Froxelizer::terminate(DriverApi& driverApi) noexcept {
    // call reset() on our LinearAllocator arenas
    mArena.reset();

    mBoundingSpheres = nullptr;
    mPlanesY = nullptr;
    mPlanesX = nullptr;
    mDistancesZ = nullptr;

    if (mRecordsBuffer) {
        driverApi.destroyBufferObject(mRecordsBuffer);
    }
    if (mFroxelsBuffer) {
        driverApi.destroyBufferObject(mFroxelsBuffer);
    }
}

void Froxelizer::setOptions(float const zLightNear, float const zLightFar) noexcept {
    if (UTILS_UNLIKELY(mZLightNear != zLightNear || mZLightFar != zLightFar)) {
        mZLightNear = zLightNear;
        mZLightFar = zLightFar;
        mDirtyFlags |= VIEWPORT_CHANGED;
    }
}


void Froxelizer::setViewport(filament::Viewport const& viewport) noexcept {
    if (UTILS_UNLIKELY(mViewport != viewport)) {
        mViewport = viewport;
        mDirtyFlags |= VIEWPORT_CHANGED;
    }
}

void Froxelizer::setProjection(const mat4f& projection,
        float const near, UTILS_UNUSED float far) noexcept {
    if (UTILS_UNLIKELY(!fuzzyEqual(mProjection, projection))) {
        mProjection = projection;
        mNear = near;
        mDirtyFlags |= PROJECTION_CHANGED;
    }
}

bool Froxelizer::prepare(
        FEngine::DriverApi& driverApi, RootArenaScope& rootArenaScope,
        filament::Viewport const& viewport,
        const mat4f& projection, float const projectionNear, float const projectionFar) noexcept {
    setViewport(viewport);
    setProjection(projection, projectionNear, projectionFar);

    bool uniformsNeedUpdating = false;
    if (UTILS_UNLIKELY(mDirtyFlags)) {
        uniformsNeedUpdating = update();
    }

    /*
     * Allocations that need to persist until the driver consumes them are done from
     * the command stream.
     */

    // froxel buffer (~32 KiB)
    mFroxelBufferUser = {
            driverApi.allocatePod<FroxelEntry>(getFroxelBufferEntryCount()),
            getFroxelBufferEntryCount() };

    // record buffer (~16 KiB)
    mRecordBufferUser = {
            driverApi.allocatePod<RecordBufferType>(RECORD_BUFFER_ENTRY_COUNT),
            RECORD_BUFFER_ENTRY_COUNT };

    /*
     * Temporary allocations for processing all froxel data
     */

    // light records per froxel (~256 KiB)
    mLightRecords = {
            rootArenaScope.allocate<LightRecord>(getFroxelBufferEntryCount(), CACHELINE_SIZE),
            getFroxelBufferEntryCount() };

    // froxel thread data (~256 KiB)
    mFroxelShardedData = {
            rootArenaScope.allocate<FroxelThreadData>(GROUP_COUNT, CACHELINE_SIZE),
            uint32_t(GROUP_COUNT)
    };

    assert_invariant(mFroxelBufferUser.begin());
    assert_invariant(mRecordBufferUser.begin());
    assert_invariant(mLightRecords.begin());
    assert_invariant(mFroxelShardedData.begin());

    // initialize buffers that need to be
    memset(mLightRecords.data(), 0, mLightRecords.sizeInBytes());

    return uniformsNeedUpdating;
}

void Froxelizer::computeFroxelLayout(
        uint2* dim, uint16_t* countX, uint16_t* countY, uint16_t* countZ,
        size_t const froxelBufferEntryCount, filament::Viewport const& viewport) noexcept {

    auto roundTo8 = [](uint32_t const v) { return (v + 7u) & ~7u; };

    const uint32_t width  = std::max(16u, viewport.width);
    const uint32_t height = std::max(16u, viewport.height);

    // calculate froxel dimension from FROXEL_BUFFER_ENTRY_COUNT_MAX and viewport
    // - Start from the maximum number of froxels we can use in the x-y plane
    size_t const froxelSliceCount = FROXEL_SLICE_COUNT;
    size_t const froxelPlaneCount = froxelBufferEntryCount / froxelSliceCount;
    // - compute the number of square froxels we need in width and height, rounded down
    //   solving: |  froxelCountX * froxelCountY == froxelPlaneCount
    //            |  froxelCountX / froxelCountY == width / height
    size_t froxelCountX = size_t(std::sqrt(froxelPlaneCount * width  / height));
    size_t froxelCountY = size_t(std::sqrt(froxelPlaneCount * height / width));
    // - compute the froxels dimensions, rounded up
    size_t const froxelSizeX = (width  + froxelCountX - 1) / froxelCountX;
    size_t const froxelSizeY = (height + froxelCountY - 1) / froxelCountY;
    // - and since our froxels must be square, only keep the largest dimension

    //  make sure we're at lease multiple of 8 to improve performance in the shader
    size_t const froxelDimension = roundTo8((roundTo8(froxelSizeX) >= froxelSizeY) ? froxelSizeX : froxelSizeY);

    // Here we recompute the froxel counts which may have changed a little due to the rounding
    // and the squareness requirement of froxels
    froxelCountX = (width  + froxelDimension - 1) / froxelDimension;
    froxelCountY = (height + froxelDimension - 1) / froxelDimension;

    assert_invariant(froxelCountX);
    assert_invariant(froxelCountY);
    assert_invariant(froxelCountX * froxelCountY <= froxelPlaneCount);

    *dim = froxelDimension;
    *countX = uint16_t(froxelCountX);
    *countY = uint16_t(froxelCountY);
    *countZ = uint16_t(froxelSliceCount);
}

UTILS_NOINLINE
void Froxelizer::updateBoundingSpheres(
        float4* const UTILS_RESTRICT boundingSpheres,
        size_t froxelCountX, size_t froxelCountY, size_t froxelCountZ,
        float4 const* UTILS_RESTRICT planesX,
        float4 const* UTILS_RESTRICT planesY,
        float const* UTILS_RESTRICT planesZ) noexcept {

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    // TODO: this could potentially be parallel_for'ized

    /*
     * Now compute the bounding sphere of each froxel, which is needed for spotlights
     * We intersect 3 planes of the frustum to find each 8 corners.
     */

    UTILS_ASSUME(froxelCountX > 0);
    UTILS_ASSUME(froxelCountY > 0);

    for (size_t iz = 0, fi = 0, nz = froxelCountZ; iz < nz; ++iz) {
        float4 planes[6];
        planes[4] =  float4{ 0, 0, 1, planesZ[iz + 0] };
        planes[5] = -float4{ 0, 0, 1, planesZ[iz + 1] };
        for (size_t iy = 0, ny = froxelCountY; iy < ny; ++iy) {
            planes[2] =  planesY[iy];
            planes[3] = -planesY[iy + 1];
            for (size_t ix = 0, nx = froxelCountX; ix < nx; ++ix) {
                planes[0] =  planesX[ix];
                planes[1] = -planesX[ix + 1];

                float3 const p0 = planeIntersection(planes[0], planes[2], planes[4]);
                float3 const p1 = planeIntersection(planes[1], planes[2], planes[4]);
                float3 const p2 = planeIntersection(planes[0], planes[3], planes[4]);
                float3 const p3 = planeIntersection(planes[1], planes[3], planes[4]);
                float3 const p4 = planeIntersection(planes[0], planes[2], planes[5]);
                float3 const p5 = planeIntersection(planes[1], planes[2], planes[5]);
                float3 const p6 = planeIntersection(planes[0], planes[3], planes[5]);
                float3 const p7 = planeIntersection(planes[1], planes[3], planes[5]);

                float3 const c = (p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7) * 0.125f;

                float const d0 = length2(p0 - c);
                float const d1 = length2(p1 - c);
                float const d2 = length2(p2 - c);
                float const d3 = length2(p3 - c);
                float const d4 = length2(p4 - c);
                float const d5 = length2(p5 - c);
                float const d6 = length2(p6 - c);
                float const d7 = length2(p7 - c);

                float const r = std::sqrt(std::max({ d0, d1, d2, d3, d4, d5, d6, d7 }));

                assert_invariant(getFroxelIndex(ix, iy, iz, froxelCountX, froxelCountY) == fi);
                boundingSpheres[fi++] = { c, r };
            }
        }
    }
}

UTILS_NOINLINE
bool Froxelizer::update() noexcept {
    bool uniformsNeedUpdating = false;
    if (UTILS_UNLIKELY(mDirtyFlags & VIEWPORT_CHANGED)) {
        filament::Viewport const& viewport = mViewport;

        uint2 froxelDimension;
        uint16_t froxelCountX, froxelCountY, froxelCountZ;
        computeFroxelLayout(&froxelDimension, &froxelCountX, &froxelCountY, &froxelCountZ,
                getFroxelBufferEntryCount(), viewport);

        mFroxelDimension = froxelDimension;
        mClipToFroxelX = (0.5f * float(viewport.width))  / float(froxelDimension.x);
        mClipToFroxelY = (0.5f * float(viewport.height)) / float(froxelDimension.y);

        uniformsNeedUpdating = true;

#ifndef NDEBUG
        slog.d << "Froxel: " << viewport.width << "x" << viewport.height << " / "
               << froxelDimension.x << "x" << froxelDimension.y << io::endl
               << "Froxel: " << froxelCountX << "x" << froxelCountY << "x" << froxelCountZ
               << " = " << (froxelCountX * froxelCountY * froxelCountZ)
               << " (" << getFroxelBufferEntryCount() - froxelCountX * froxelCountY * froxelCountZ << " lost)"
               << io::endl;
#endif

        mFroxelCountX = froxelCountX;
        mFroxelCountY = froxelCountY;
        mFroxelCountZ = froxelCountZ;
        const uint32_t froxelCount = uint32_t(froxelCountX * froxelCountY * froxelCountZ);
        mFroxelCount = froxelCount;

        if (mDistancesZ) {
            // this is a LinearAllocator arena, use rewind() instead of free (which is a no op).
            mArena.rewind(mDistancesZ);
        }

        mDistancesZ      = mArena.alloc<float>(froxelCountZ + 1);
        mPlanesX         = mArena.alloc<float4>(froxelCountX + 1);
        mPlanesY         = mArena.alloc<float4>(froxelCountY + 1);
        mBoundingSpheres = mArena.alloc<float4>(froxelCount);

        assert_invariant(mDistancesZ);
        assert_invariant(mPlanesX);
        assert_invariant(mPlanesY);
        assert_invariant(mBoundingSpheres);

        mDistancesZ[0] = 0.0f;
        const float zLightNear = mZLightNear;
        const float zLightFar = mZLightFar;
        const float linearizer = std::log2(zLightFar / zLightNear) / float(std::max(1u, mFroxelCountZ - 1u));
        // for a strange reason when, vectorizing this loop, clang does some math in double
        // and generates conversions to float. not worth it for so little iterations.
#if defined(__clang__)
        #pragma clang loop vectorize(disable) unroll(disable)
#endif
        for (ssize_t i = 1, n = mFroxelCountZ; i <= n; i++) {
            mDistancesZ[i] = zLightFar * std::exp2(float(i - n) * linearizer);
        }

        // for the inverse-transformation (view-space z to z-slice)
        mLinearizer = 1.0f / linearizer;
        mZLightFar = zLightFar;

        mParamsZ[0] = 0; // updated when camera changes
        mParamsZ[1] = 0; // updated when camera changes
        mParamsZ[2] = 0; // updated when camera changes
        mParamsZ[3] = mFroxelCountZ;
        mParamsF[0] = 1;
        mParamsF[1] = uint32_t(mFroxelCountX);
        mParamsF[2] = uint32_t(mFroxelCountX * mFroxelCountY);
    }

    if (UTILS_UNLIKELY(mDirtyFlags & (PROJECTION_CHANGED | VIEWPORT_CHANGED))) {
        assert_invariant(mDistancesZ);
        assert_invariant(mPlanesX);
        assert_invariant(mPlanesY);
        assert_invariant(mBoundingSpheres);

        // clip-space dimensions
        const float froxelWidthInClipSpace  = float(2 * mFroxelDimension.x) / float(mViewport.width);
        const float froxelHeightInClipSpace = float(2 * mFroxelDimension.y) / float(mViewport.height);
        float4 * const UTILS_RESTRICT planesX = mPlanesX;
        float4 * const UTILS_RESTRICT planesY = mPlanesY;

        // Planes are transformed by the inverse-transpose of the transform matrix.
        // So to transform a plane in clip-space to view-space, we need to apply
        // the transpose(inverse(viewFromClipMatrix)), i.e.: transpose(projection)
        const mat4f trProjection(transpose(mProjection));

        // generate the horizontal planes from their clip-space equation
        for (size_t i = 0, n = mFroxelCountX; i <= n; ++i) {
            float const x = (float(i) * froxelWidthInClipSpace) - 1.0f;
            float4 const p = trProjection * float4{ -1, 0, 0, x };
            planesX[i] = float4{ normalize(p.xyz), 0 };  // p.w is guaranteed to be 0
        }

        // generate the vertical planes from their clip-space equation
        for (size_t i = 0, n = mFroxelCountY; i <= n; ++i) {
            float const y = (float(i) * froxelHeightInClipSpace) - 1.0f;
            float4 const p = trProjection * float4{ 0, 1, 0, -y };
            planesY[i] = float4{ normalize(p.xyz), 0 };  // p.w is guaranteed to be 0
        }

        updateBoundingSpheres(mBoundingSpheres,
                mFroxelCountX, mFroxelCountY, mFroxelCountZ,
                planesX, planesY, mDistancesZ);

        // note: none of the values below are affected by the projection offset, scale or rotation.
        float const Pz = mProjection[2][2];
        float const Pw = mProjection[3][2];
        if (mProjection[2][3] != 0) {
            // With our inverted DX convention, we have the simple relation:
            // z_view = -near / z_screen
            // ==> i = log2(-z / far) / linearizer + zcount
            // ==> i = -log2(z_screen * (far/near)) * (1/linearizer) + zcount
            // ==> i = log2(z_screen * (far/near)) * (-1/linearizer) + zcount
            mParamsZ[0] = mZLightFar / Pw;
            mParamsZ[1] = 0.0f;
            mParamsZ[2] = -mLinearizer;
        } else {
            // orthographic projection
            // z_view = (1 - z_screen) * (near - far) - near
            // z_view = z_screen * (far - near) - far
            // our ortho matrix is in inverted-DX convention
            //   Pz =   1 / (far - near)
            //   Pw = far / (far - near)
            mParamsZ[0] = -1.0f / (Pz * mZLightFar);  // -(far-near) / mZLightFar
            mParamsZ[1] =    Pw / (Pz * mZLightFar);  //         far / mZLightFar
            mParamsZ[2] = mLinearizer;
        }
        uniformsNeedUpdating = true;
    }
    assert_invariant(mZLightNear >= mNear);
    mDirtyFlags = 0;
    return uniformsNeedUpdating;
}

Froxel Froxelizer::getFroxelAt(size_t const x, size_t const y, size_t const z) const noexcept {
    assert_invariant(x < mFroxelCountX);
    assert_invariant(y < mFroxelCountY);
    assert_invariant(z < mFroxelCountZ);
    Froxel froxel;
    froxel.planes[Froxel::LEFT]   =  mPlanesX[x];
    froxel.planes[Froxel::BOTTOM] =  mPlanesY[y];
    froxel.planes[Froxel::NEAR]   =  float4{ 0, 0, 1, mDistancesZ[z] };
    froxel.planes[Froxel::RIGHT]  = -mPlanesX[x + 1];
    froxel.planes[Froxel::TOP]    = -mPlanesY[y + 1];
    froxel.planes[Froxel::FAR]    = -float4{ 0, 0, 1, mDistancesZ[z+1] };
    return froxel;
}

UTILS_NOINLINE
size_t Froxelizer::findSliceZ(float const z) const noexcept {
    // The vastly common case is that z<0, so we always do the math for this case
    // and we "undo" it below otherwise. This works because we're using fast::log2 which
    // doesn't care if given a negative number (we'd have to use abs() otherwise).

    // This whole function is now branch-less.

    int s = int( fast::log2(-z / mZLightFar) * mLinearizer + float(mFroxelCountZ) );

    // there are cases where z can be negative here, e.g.:
    // - the light is visible, but its center is behind the camera
    // - the camera's near is behind the camera (e.g. with shadowmap cameras)
    // in that case just return the first slice
    s = z < 0 ? s : 0;

    // clamp between [0, mFroxelCountZ)
    return size_t(clamp(s, 0, mFroxelCountZ - 1));
}

std::pair<size_t, size_t> Froxelizer::clipToIndices(float2 const& clip) const noexcept {
    // clip coordinates between [-1, 1], conversion to index between [0, count[
    // (clip + 1) * 0.5 * dimension / froxelsize
    // clip * 0.5 * dimension / froxelsize + 0.5 * dimension / froxelsize
    const size_t xi = size_t(clamp(int(clip.x * mClipToFroxelX + mClipToFroxelX), 0, mFroxelCountX - 1));
    const size_t yi = size_t(clamp(int(clip.y * mClipToFroxelY + mClipToFroxelY), 0, mFroxelCountY - 1));
    return { xi, yi };
}


void Froxelizer::commit(DriverApi& driverApi) {
    // send data to GPU
    driverApi.updateBufferObject(mFroxelsBuffer,
            { mFroxelBufferUser.data(), getFroxelBufferEntryCount() * sizeof(FroxelEntry) }, 0);

    driverApi.updateBufferObject(mRecordsBuffer,
            { mRecordBufferUser.data(), RECORD_BUFFER_ENTRY_COUNT }, 0);

#ifndef NDEBUG
    mFroxelBufferUser.clear();
    mRecordBufferUser.clear();
    mFroxelShardedData.clear();
#endif
}

void Froxelizer::froxelizeLights(FEngine& engine,
        mat4f const& UTILS_RESTRICT viewMatrix,
        const FScene::LightSoa& UTILS_RESTRICT lightData) noexcept {
    // note: this is called asynchronously
    froxelizeLoop(engine, viewMatrix, lightData);
    froxelizeAssignRecordsCompress();

#ifndef NDEBUG
    if (lightData.size()) {
        // go through every froxel
        auto const& recordBufferUser(mRecordBufferUser);
        auto gpuFroxelEntries(mFroxelBufferUser);
        gpuFroxelEntries.set(gpuFroxelEntries.begin(),
                mFroxelCountX * mFroxelCountY * mFroxelCountZ);
        for (auto const& entry : gpuFroxelEntries) {
            // go through every light for that froxel
            for (size_t i = 0; i < entry.count(); i++) {
                // get the light index
                assert_invariant(entry.offset() + i < RECORD_BUFFER_ENTRY_COUNT);

                size_t const lightIndex = recordBufferUser[entry.offset() + i];
                assert_invariant(lightIndex <= CONFIG_MAX_LIGHT_INDEX);

                // make sure it corresponds to an existing light
                assert_invariant(lightIndex < lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT);
            }
        }
    }
#endif
}

void Froxelizer::froxelizeLoop(FEngine& engine,
        const mat4f& UTILS_RESTRICT viewMatrix,
        const FScene::LightSoa& UTILS_RESTRICT lightData) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    Slice<FroxelThreadData> froxelThreadData = mFroxelShardedData;
    memset(froxelThreadData.data(), 0, froxelThreadData.sizeInBytes());

    auto& lcm = engine.getLightManager();
    auto const* UTILS_RESTRICT spheres      = lightData.data<FScene::POSITION_RADIUS>();
    auto const* UTILS_RESTRICT directions   = lightData.data<FScene::DIRECTION>();
    auto const* UTILS_RESTRICT instances    = lightData.data<FScene::LIGHT_INSTANCE>();

    auto process = [ this, &froxelThreadData,
                     spheres, directions, instances, &viewMatrix, &lcm ]
            (size_t const count, size_t const offset, size_t const stride) {

        FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "FroxelizeLoop Job");

        const mat4f& projection = mProjection;
        const mat3f& vn = viewMatrix.upperLeft();

        // We use minimum cone angle of 0.5 degrees because too small angles cause issues in the
        // sphere/cone intersection test, due to floating-point precision.
        constexpr float maxInvSin = 114.59301f;         // 1 / sin(0.5 degrees)
        constexpr float maxCosSquared = 0.99992385f;    // cos(0.5 degrees)^2

        for (size_t i = offset; i < count; i += stride) {
            const size_t j = i + FScene::DIRECTIONAL_LIGHTS_COUNT;
            FLightManager::Instance const li = instances[j];
            LightParams light = {
                    .position = (viewMatrix * float4{ spheres[j].xyz, 1 }).xyz,     // to view-space
                    .cosSqr = std::min(maxCosSquared, lcm.getCosOuterSquared(li)),  // spot only
                    .axis = vn * directions[j],                                     // spot only
                    .invSin = lcm.getSinInverse(li),                                // spot only
                    .radius = spheres[j].w,
            };
            // infinity means "point-light"
            if (light.invSin != std::numeric_limits<float>::infinity()) {
                light.invSin = std::min(maxInvSin, light.invSin);
            }

            const size_t group = i % GROUP_COUNT;
            const size_t bit   = i / GROUP_COUNT;
            assert_invariant(bit < LIGHT_PER_GROUP);

            FroxelThreadData& threadData = froxelThreadData[group];
            froxelizePointAndSpotLight(threadData, bit, projection, light);
        }
    };

    // we do 64 lights per job
    JobSystem& js = engine.getJobSystem();

    constexpr bool SINGLE_THREADED = false;
    if constexpr (!SINGLE_THREADED) {
        auto *parent = js.createJob();
        for (size_t i = 0; i < GROUP_COUNT; i++) {
            js.run(jobs::createJob(js, parent, std::cref(process),
                    lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT, i, GROUP_COUNT));
        }
        js.runAndWait(parent);
    } else {
        js.runAndWait(jobs::createJob(js, nullptr, std::cref(process),
                lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT, 0, 1)
        );
    }
}

void Froxelizer::froxelizeAssignRecordsCompress() noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    Slice<FroxelThreadData> const froxelThreadData = mFroxelShardedData;

    // convert froxel data from N groups of M bits to LightRecord::bitset, so we can
    // easily compare adjacent froxels, for compaction. The conversion loops below get
    // inlined and vectorized in release builds.

    // this gets very well vectorized...

    Slice<LightRecord> records(mLightRecords);
    for (size_t j = 0, jc = getFroxelBufferEntryCount(); j < jc; j++) {
        for (size_t i = 0; i < LightRecord::bitset::WORLD_COUNT; i++) {
            using container_type = LightRecord::bitset::container_type;
            constexpr size_t r = sizeof(container_type) / sizeof(LightGroupType);
            container_type b = froxelThreadData[i * r][j];
            for (size_t k = 0; k < r; k++) {
                b |= (container_type(froxelThreadData[i * r + k][j]) << (LIGHT_PER_GROUP * k));
            }
            records[j].lights.getBitsAt(i) = b;
        }
    }

    LightRecord::bitset allLights{};
    for (size_t j = 0, jc = getFroxelBufferEntryCount(); j < jc; j++) {
        allLights |= records[j].lights;
    }

    uint16_t offset = 0;
    FroxelEntry* const UTILS_RESTRICT froxels = mFroxelBufferUser.data();

    const size_t froxelCountX = mFroxelCountX;
    RecordBufferType* const UTILS_RESTRICT froxelRecords = mRecordBufferUser.data();

    // initialize the first record with all lights in the scene -- this will be used only if
    // we run out of record space.
    const uint8_t allLightsCount = (uint8_t)std::min(size_t(255), allLights.count());
    offset += allLightsCount;
    allLights.forEachSetBit([point = froxelRecords, froxelRecords](size_t l) mutable {
        // make sure to keep this code branch-less
        const size_t word = l / LIGHT_PER_GROUP;
        const size_t bit  = l % LIGHT_PER_GROUP;
        l = (bit * GROUP_COUNT) | (word % GROUP_COUNT);
        *point = (RecordBufferType)l;
        // we need to "cancel" the write operation if we have more than 255 spot or point lights
        // (this is a limitation of the data type used to store the light counts per froxel)
        point += (point - froxelRecords < 255) ? 1 : 0;
    });

    // how many froxel record entries were reused (for debugging)
    UTILS_UNUSED size_t reused = 0;

    for (size_t i = 0, c = mFroxelCount; i < c;) {
        LightRecord b = records[i];
        if (b.lights.none()) {
            froxels[i++].u32 = 0;
            continue;
        }

        // We have a limitation of 255 spot + 255 point lights per froxel.
        // note: initializer list for union cannot have more than one element
        FroxelEntry entry{ offset, uint8_t(std::min(size_t(255), b.lights.count())) };
        const size_t lightCount = entry.count();

        if (UTILS_UNLIKELY(offset + lightCount >= RECORD_BUFFER_ENTRY_COUNT)) {
#ifndef NDEBUG
            slog.d << "out of space: " << i << ", at " << offset << io::endl;
#endif
            // note: instead of dropping froxels we could look for similar records we've already
            // filed up.
            do {
                froxels[i] = { 0u, allLightsCount };
                if (records[i].lights.none()) {
                    froxels[i].u32 = 0;
                }
            } while(++i < c);
            goto out_of_memory;
        }

        // iterate the bitfield
        auto * const beginPoint = froxelRecords + offset;
        b.lights.forEachSetBit([point = beginPoint, beginPoint](size_t l) mutable {
            // make sure to keep this code branch-less
            const size_t word = l / LIGHT_PER_GROUP;
            const size_t bit  = l % LIGHT_PER_GROUP;
            l = (bit * GROUP_COUNT) | (word % GROUP_COUNT);
            *point = (RecordBufferType)l;
            // we need to "cancel" the write operation if we have more than 255 spot or point lights
            // (this is a limitation of the data type used to store the light counts per froxel)
            point += (point - beginPoint < 255) ? 1 : 0;
        });

        offset += lightCount;

#ifndef NDEBUG
        if (lightCount) { reused--; }
#endif
        do {
#ifndef NDEBUG
            if (lightCount) { reused++; }
#endif
            froxels[i++].u32 = entry.u32;
            if (i >= c) break;

            if (records[i].lights != b.lights && i >= froxelCountX) {
                // if this froxel record doesn't match the previous one on its left,
                // we re-try with the record above it, which saves many froxel records
                // (north of 10% in practice).
                b = records[i - froxelCountX];
                entry.u32 = froxels[i - froxelCountX].u32;
            }
        } while(records[i].lights == b.lights);
    }
out_of_memory:
    // FIXME: on big-endian systems we need to change the endianness of the record buffer
    ;
}

static float2 project(mat4f const& p, float3 const& v) noexcept {
    const float vx = v[0];
    const float vy = v[1];
    const float vz = v[2];
    const float x = p[0].x * vx + p[1].x * vy + p[2].x * vz + p[3].x;
    const float y = p[0].y * vx + p[1].y * vy + p[2].y * vz + p[3].y;
    const float w = p[0].w * vx + p[1].w * vy + p[2].w * vz + p[3].w;
    return float2{ x, y } * (1.0f / w);
}

void Froxelizer::froxelizePointAndSpotLight(
        FroxelThreadData& froxelThread, size_t bit,
        mat4f const& UTILS_RESTRICT p,
        const LightParams& UTILS_RESTRICT light) const noexcept {

    if (UTILS_UNLIKELY(light.position.z + light.radius < -mZLightFar)) { // z values are negative
        // This light is fully behind LightFar, it doesn't light anything
        // (we could avoid this check if we culled lights using LightFar instead of the
        // culling camera's far plane)
        return;
    }

    // the code below works with radius^2
    const float4 s = { light.position, light.radius * light.radius };

#ifdef DEBUG_FROXEL
    const size_t x0 = 0;
    const size_t x1 = mFroxelCountX - 1;
    const size_t y0 = 0;
    const size_t y1 = mFroxelCountY - 1;
    const size_t z0 = 0;
    const size_t z1 = mFroxelCountZ - 1;
#else
    // find a reasonable bounding-box in froxel space for the sphere by projecting
    // its (clipped) bounding-box to clip-space and converting to froxel indices.
    Box const aabb = { light.position, light.radius };
    const float znear = std::min(-mNear, aabb.center.z + aabb.halfExtent.z); // z values are negative
    const float zfar  =                  aabb.center.z - aabb.halfExtent.z;

    // TODO: we need to investigate if doing all this actually saves time
    //       e.g.: we could only do the z-min/max which is much easier to compute.

    const float2 pts[8] = {
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{ 1, 1 }, znear }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{ 1,-1 }, znear }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{-1, 1 }, znear }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{-1,-1 }, znear }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{ 1, 1 }, zfar  }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{ 1,-1 }, zfar  }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{-1, 1 }, zfar  }),
        project(p, { aabb.center.xy + aabb.halfExtent.xy * float2{-1,-1 }, zfar  }),
    };

    float2 pmin = std::numeric_limits<float>::max();
    float2 pmax = 0;
    for (auto pt: pts) {
        pmin = min(pmin, pt);
        pmax = max(pmax, pt);
    }

    const auto [x0, y0] = clipToIndices(pmin);
    const size_t z0 = findSliceZ(znear);

    const auto [x1, y1] = clipToIndices(pmax);
    const size_t z1 = findSliceZ(zfar);

    assert_invariant(x0 <= x1);
    assert_invariant(y0 <= y1);
    assert_invariant(z0 <= z1);
#endif

    const size_t zcenter = findSliceZ(s.z);
    float4 const * const UTILS_RESTRICT planesX = mPlanesX;
    float4 const * const UTILS_RESTRICT planesY = mPlanesY;
    float const * const UTILS_RESTRICT planesZ = mDistancesZ;
    float4 const * const UTILS_RESTRICT boundingSpheres = mBoundingSpheres;
    for (size_t iz = z0 ; iz <= z1; ++iz) {
        float4 cz(s);
        // froxel that contain the center of the sphere is special, we don't even need to do the
        // intersection check, it's always true.
        if (UTILS_LIKELY(iz != zcenter)) {
            cz = spherePlaneIntersection(s, (iz < zcenter) ? planesZ[iz + 1] : planesZ[iz]);
        }

        if (cz.w > 0) { // intersection of light with this plane (slice)
            // the sphere (light) intersects this slice's plane, and we now have a new smaller
            // sphere centered there. Now, find x & y slices that contain the sphere's center
            // (note: this changes with the Z slices)
            const float2 clip = project(p, cz.xyz);
            auto const [xcenter, ycenter] = clipToIndices(clip);

            for (size_t iy = y0; iy <= y1; ++iy) {
                float4 cy(cz);
                // froxel that contain the center of the sphere is special, we don't even need to
                // do the intersection check, it's always true.
                if (UTILS_LIKELY(iy != ycenter)) {
                    float4 const& plane = iy < ycenter ? planesY[iy + 1] : planesY[iy];
                    cy = spherePlaneIntersection(cz, plane);
                }

                if (cy.w > 0) {
                    // The reduced sphere from the previous stage intersects this horizontal plane,
                    // and we now have new smaller sphere centered on these two previous planes
                    size_t bx = std::numeric_limits<size_t>::max(); // horizontal begin index
                    size_t ex = 0; // horizontal end index

                    // find the "begin" index (left side)
                    for (size_t ix = x0; ix < x1 + 1; ++ix) {
                        // The froxel that contains the center of the sphere is special,
                        // we don't even need to do the intersection check, it's always true.
                        if (UTILS_LIKELY(ix != xcenter)) {
                            float4 const& plane = ix < xcenter ? planesX[ix + 1] : planesX[ix];
                            if (spherePlaneIntersection(cy, plane).w > 0) {
                                // The reduced sphere from the previous stage intersects this
                                // vertical plane, we record the min/max froxel indices
                                bx = std::min(bx, ix);
                                ex = std::max(ex, ix);
                            }
                        } else {
                            // this is the froxel containing the center of the sphere, it is
                            // definitely participating
                            bx = std::min(bx, ix);
                            ex = std::max(ex, ix);
                        }
                    }

                    if (UTILS_UNLIKELY(bx > ex)) {
                        continue;
                    }

                    // the loops below assume 1-past the end for the right side of the range
                    ex++;
                    assert_invariant(bx <= mFroxelCountX && ex <= mFroxelCountX);

                    size_t fi = getFroxelIndex(bx, iy, iz);
                    if (light.invSin != std::numeric_limits<float>::infinity()) {
                        // This is a spotlight (common case)
                        // this loops gets vectorized (on arm64) w/ clang
                        while (bx++ != ex) {
                            // see if this froxel intersects the cone
                            bool const intersect = sphereConeIntersectionFast(boundingSpheres[fi],
                                    light.position, light.axis, light.invSin, light.cosSqr);
                            froxelThread[fi++] |= LightGroupType(intersect) << bit;
                        }
                    } else {
                        // this loops gets vectorized (on arm64) w/ clang
                        while (bx++ != ex) {
                            froxelThread[fi++] |= LightGroupType(1) << bit;
                        }
                    }
                }
            }
        }
    }
}

/*
 *
 * lightTree            output the light tree structure there (must be large enough to hold a complete tree)
 * lightList            list if lights
 * lightData            scene's light data SoA
 * lightRecordsOffset   offset in the record buffer where to find the light list
 */
void Froxelizer::computeLightTree(
        LightTreeNode* lightTree,
        Slice<RecordBufferType> const& lightList,
        const FScene::LightSoa& lightData,
        size_t lightRecordsOffset) noexcept {

    // number of lights in this record
    const size_t count = lightList.size();

    // the width of the tree is the next power-of-two (if not already a power of two)
    const size_t w = 1u << (log2i(count) + (popcount(count) == 1 ? 0 : 1));

    // height of the tree
    const size_t h = log2i(w) + 1u;

    auto const* UTILS_RESTRICT zrange = lightData.data<FScene::SCREEN_SPACE_Z_RANGE>() + 1;
    BinaryTreeArray::traverse(h,
            [lightTree, lightRecordsOffset, zrange, indices = lightList.data(), count]
            (size_t const index, size_t const col, size_t const next) {
                // indices[] cannot be accessed past 'col'
                const float min = (col < count) ? zrange[indices[col]].x : 1.0f;
                const float max = (col < count) ? zrange[indices[col]].y : 0.0f;
                lightTree[index] = {
                        .min = min,
                        .max = max,
                        .next = uint16_t(next),
                        .offset = uint16_t(lightRecordsOffset + col),
                        .isLeaf = 1,
                        .count = 1,
                        .reserved = 0,
                };
            },
            [lightTree](size_t const index, size_t const l, size_t const r, size_t const next) {
                lightTree[index] = {
                        .min = std::min(lightTree[l].min, lightTree[r].min),
                        .max = std::max(lightTree[l].max, lightTree[r].max),
                        .next = uint16_t(next),
                        .offset = 0,
                        .isLeaf = 0,
                        .count = 0,
                        .reserved = 0,
                };
            });
}

} // namespace filament
