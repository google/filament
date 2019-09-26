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

#include "details/Froxelizer.h"

#include "Intersections.h"

#include "details/Engine.h"
#include "details/Scene.h"

#include <filament/Viewport.h>

#include <utils/Allocator.h>
#include <utils/BinaryTreeArray.h>
#include <utils/Systrace.h>

#include <math/mat4.h>
#include <math/fast.h>
#include <math/scalar.h>

#include <algorithm>

#include <stddef.h>

using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

namespace details {

/*
 * This enables froxels to be rectangular which allows us to use a but more froxel
 * with the same amount of memory in the GPU.
 * This requires backward compatibility breaking changes in the shaders.
 */
static constexpr bool SUPPORTS_NON_SQUARE_FROXELS = false;

/*
 * This changes the layout of the froxel info on the GPU such that it is more cache friendly,
 * i.e. the major axis is Z instead of X, because in a given froxel there is more chance to
 * hit another froxel at the same x,y coordinate.
 * This requires backward compatibility breaking changes in the shaders.
 */
static constexpr bool SUPPORTS_REMAPPED_FROXELS   = false;

// The Froxel buffer is set to FROXEL_BUFFER_WIDTH x n
// With n limited by the supported texture dimension, which is guaranteed to be at least 2048
// in all version of GLES.

// Make sure this matches the same constants in shading_lit.fs
constexpr size_t FROXEL_BUFFER_WIDTH_SHIFT  = 6u;
constexpr size_t FROXEL_BUFFER_WIDTH        = 1u << FROXEL_BUFFER_WIDTH_SHIFT;
constexpr size_t FROXEL_BUFFER_WIDTH_MASK   = FROXEL_BUFFER_WIDTH - 1u;
constexpr size_t FROXEL_BUFFER_HEIGHT       = (FROXEL_BUFFER_ENTRY_COUNT_MAX + FROXEL_BUFFER_WIDTH_MASK) / FROXEL_BUFFER_WIDTH;

constexpr size_t RECORD_BUFFER_WIDTH_SHIFT  = 5u;
constexpr size_t RECORD_BUFFER_WIDTH        = 1u << RECORD_BUFFER_WIDTH_SHIFT;

constexpr size_t RECORD_BUFFER_HEIGHT       = 2048;
constexpr size_t RECORD_BUFFER_ENTRY_COUNT  = RECORD_BUFFER_WIDTH * RECORD_BUFFER_HEIGHT; // 64K

// Buffer needed for Froxelizer internal data structures (~256 KiB)
constexpr size_t PER_FROXELDATA_ARENA_SIZE = sizeof(float4) *
                                                 (FROXEL_BUFFER_ENTRY_COUNT_MAX +
                                                  FROXEL_BUFFER_ENTRY_COUNT_MAX + 3 +
                                                  FEngine::CONFIG_FROXEL_SLICE_COUNT / 4 + 1);


// number of lights processed by one group (e.g. 32)
static constexpr size_t LIGHT_PER_GROUP = sizeof(Froxelizer::LightGroupType) * 8;

// number of groups (i.e. jobs) to use for froxelization (e.g. 8)
static constexpr size_t GROUP_COUNT =
        (CONFIG_MAX_LIGHT_COUNT + LIGHT_PER_GROUP - 1) / LIGHT_PER_GROUP;


// record buffer cannot be larger than 65K entries because we're using uint16_t to store indices
// so its maximum size is 128 KiB
static_assert(RECORD_BUFFER_ENTRY_COUNT <= 65536,
        "RecordBuffer cannot be larger than 65536 entries");

Froxelizer::Froxelizer(FEngine& engine)
        : mArena("froxel", PER_FROXELDATA_ARENA_SIZE) {

    DriverApi& driverApi = engine.getDriverApi();

    // RecordBuffer cannot be larger than 65536 entries, because indices are uint16_t
    GPUBuffer::ElementType type = std::is_same<RecordBufferType, uint8_t>::value
                                  ? GPUBuffer::ElementType::UINT8 : GPUBuffer::ElementType::UINT16;
    mRecordsBuffer = GPUBuffer(driverApi, { type, 1 }, RECORD_BUFFER_WIDTH, RECORD_BUFFER_HEIGHT);
    mFroxelBuffer  = GPUBuffer(driverApi, { GPUBuffer::ElementType::UINT16, 2 },
            FROXEL_BUFFER_WIDTH, FROXEL_BUFFER_HEIGHT);
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

    mRecordsBuffer.terminate(driverApi);
    mFroxelBuffer.terminate(driverApi);
}

void Froxelizer::setOptions(float zLightNear, float zLightFar) noexcept {
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

void Froxelizer::setProjection(const mat4f& projection, float near, float far) noexcept {
    if (UTILS_UNLIKELY(mat4f::fuzzyEqual(mProjection, projection))) {
        mProjection = projection;
        mNear = near;
        mDirtyFlags |= PROJECTION_CHANGED;
    }
}

bool Froxelizer::prepare(
        FEngine::DriverApi& driverApi, ArenaScope& arena, filament::Viewport const& viewport,
        const mat4f& projection, float projectionNear, float projectionFar) noexcept {
    setViewport(viewport);
    setProjection(projection, projectionNear, projectionFar);

    bool uniformsNeedUpdating = false;
    if (UTILS_UNLIKELY(mDirtyFlags)) {
        uniformsNeedUpdating = update();
    }

    /*
     * Allocations that need to persists until the driver consumes them are done from
     * the command stream.
     */

    // froxel buffer (~32 KiB)
    mFroxelBufferUser = {
            driverApi.allocatePod<FroxelEntry>(FROXEL_BUFFER_ENTRY_COUNT_MAX),
            FROXEL_BUFFER_ENTRY_COUNT_MAX };

    // record buffer (~64 KiB)
    mRecordBufferUser = {
            driverApi.allocatePod<RecordBufferType>(RECORD_BUFFER_ENTRY_COUNT),
            RECORD_BUFFER_ENTRY_COUNT };

    /*
     * Temporary allocations for processing all froxel data
     */

    // light records per froxel (~256 KiB)
    mLightRecords = {
            arena.allocate<LightRecord>(FROXEL_BUFFER_ENTRY_COUNT_MAX, CACHELINE_SIZE),
            FROXEL_BUFFER_ENTRY_COUNT_MAX };

    // froxel thread data (~256 KiB)
    mFroxelShardedData = {
            arena.allocate<FroxelThreadData>(GROUP_COUNT, CACHELINE_SIZE),
            uint32_t(GROUP_COUNT)
    };

    assert(mFroxelBufferUser.begin());
    assert(mRecordBufferUser.begin());
    assert(mLightRecords.begin());
    assert(mFroxelShardedData.begin());

    // initialize buffers that need to be
    memset(mLightRecords.data(), 0, mLightRecords.sizeInBytes());

    return uniformsNeedUpdating;
}

void Froxelizer::computeFroxelLayout(
        uint2* dim, uint16_t* countX, uint16_t* countY, uint16_t* countZ,
        filament::Viewport const& viewport) noexcept {

    if (SUPPORTS_NON_SQUARE_FROXELS == false) {
        // calculate froxel dimension from FROXEL_BUFFER_ENTRY_COUNT_MAX and viewport
        // - Start from the maximum number of froxels we can use in the x-y plane
        size_t froxelSliceCount = FEngine::CONFIG_FROXEL_SLICE_COUNT;
        size_t froxelPlaneCount = FROXEL_BUFFER_ENTRY_COUNT_MAX / froxelSliceCount;
        // - compute the number of square froxels we need in width and height, rounded down
        //   solving: |  froxelCountX * froxelCountY == froxelPlaneCount
        //            |  froxelCountX / froxelCountY == width / height
        size_t froxelCountX = size_t(std::sqrt(froxelPlaneCount * viewport.width  / viewport.height));
        size_t froxelCountY = size_t(std::sqrt(froxelPlaneCount * viewport.height / viewport.width));
        // - copmute the froxels dimensions, rounded up
        size_t froxelSizeX = (viewport.width  + froxelCountX - 1) / froxelCountX;
        size_t froxelSizeY = (viewport.height + froxelCountY - 1) / froxelCountY;
        // - and since our froxels must be square, only keep the largest dimension
        size_t froxelDimension = std::max(froxelSizeX, froxelSizeY);

        // Here we recompute the froxel counts which may have changed a little due to the rounding
        // and the squareness requirement of froxels
        froxelCountX = (viewport.width  + froxelDimension - 1) / froxelDimension;
        froxelCountY = (viewport.height + froxelDimension - 1) / froxelDimension;

        *dim = froxelDimension;
        *countX = uint16_t(froxelCountX);
        *countY = uint16_t(froxelCountY);
        *countZ = uint16_t(froxelSliceCount);
    } else {
        // TODO: don't hardcode this
        *countX = uint16_t(32);
        *countY = uint16_t(16);
        if (viewport.height > viewport.width) {
            std::swap(*countX, *countY);
        }
        *countZ = uint16_t(FEngine::CONFIG_FROXEL_SLICE_COUNT);
         dim->x = (viewport.width  + *countX - 1) / *countX;
         dim->y = (viewport.height + *countY - 1) / *countY;
    }
}

UTILS_NOINLINE
bool Froxelizer::update() noexcept {
    bool uniformsNeedUpdating = false;
    if (UTILS_UNLIKELY(mDirtyFlags & VIEWPORT_CHANGED)) {
        filament::Viewport const& viewport = mViewport;

        uint2 froxelDimension;
        uint16_t froxelCountX, froxelCountY, froxelCountZ;
        computeFroxelLayout(&froxelDimension, &froxelCountX, &froxelCountY, &froxelCountZ, viewport);

        mFroxelDimension = froxelDimension;
        mClipToFroxelX = (0.5f * viewport.width)  / froxelDimension.x;
        mClipToFroxelY = (0.5f * viewport.height) / froxelDimension.y;

        mOneOverDimension = 1.0f / float2(froxelDimension);
        uniformsNeedUpdating = true;

#ifndef NDEBUG
        size_t froxelSliceCount = FEngine::CONFIG_FROXEL_SLICE_COUNT;
        slog.d << "Froxel: " << viewport.width << "x" << viewport.height << " / "
               << froxelDimension.x << "x" << froxelDimension.y << io::endl
               << "Froxel: " << froxelCountX << "x" << froxelCountY << "x" << froxelSliceCount
               << " = " << (froxelCountX * froxelCountY * froxelSliceCount)
               << " (" << FROXEL_BUFFER_ENTRY_COUNT_MAX - froxelCountX * froxelCountY * froxelSliceCount << " lost)"
               << io::endl;
#endif

        mFroxelCountX = froxelCountX;
        mFroxelCountY = froxelCountY;
        mFroxelCountZ = froxelCountZ;
        // froxel count must fit on 16 bits
        const uint16_t froxelCount = uint16_t(froxelCountX * froxelCountY * froxelCountZ);
        mFroxelCount = froxelCount;

        if (mDistancesZ) {
            // this is a LinearAllocator arena, use rewind() instead of free (which is a no op).
            mArena.rewind(mDistancesZ);

            mBoundingSpheres = nullptr;
            mPlanesY = nullptr;
            mPlanesX = nullptr;
            mDistancesZ = nullptr;
        }

        mDistancesZ      = mArena.alloc<float>(froxelCountZ + 1);
        mPlanesX         = mArena.alloc<float4>(froxelCountX + 1);
        mPlanesY         = mArena.alloc<float4>(froxelCountY + 1);
        mBoundingSpheres = mArena.alloc<float4>(froxelCount);

        assert(mDistancesZ);
        assert(mPlanesX);
        assert(mPlanesY);
        assert(mBoundingSpheres);

        mDistancesZ[0] = 0.0f;
        const float zLightNear = mZLightNear;
        const float zLightFar = mZLightFar;
        const float linearizer = std::log2(zLightFar / zLightNear) / (mFroxelCountZ - 1);
        // for a strange reason when, vectorizing this loop, clang does some math in double
        // and generates conversions to float. not worth it for so little iterations.
        #pragma clang loop vectorize(disable) unroll(disable)
        for (ssize_t i = 1, n = mFroxelCountZ; i <= n; i++) {
            mDistancesZ[i] = zLightFar * std::exp2f((i - n) * linearizer);
        }

        // for the inverse-transformation (view-space z to z-slice)
        mLinearizer = 1 / linearizer;
        mZLightFar = zLightFar;
        mLog2ZLightFar = std::log2(zLightFar);

        mParamsZ[0] = 0; // updated when camera changes
        mParamsZ[1] = 0; // updated when camera changes
        mParamsZ[2] = -mLinearizer;
        mParamsZ[3] = mFroxelCountZ;
        if (SUPPORTS_REMAPPED_FROXELS) {
            mParamsF.x = uint32_t(mFroxelCountZ);
            mParamsF.y = uint32_t(mFroxelCountX * mFroxelCountZ);
            mParamsF.z = 1;
        } else {
            mParamsF[0] = 1;
            mParamsF[1] = uint32_t(mFroxelCountX);
            mParamsF[2] = uint32_t(mFroxelCountX * mFroxelCountY);
        }
    }

    if (UTILS_UNLIKELY(mDirtyFlags & (PROJECTION_CHANGED | VIEWPORT_CHANGED))) {
        assert(mDistancesZ);
        assert(mPlanesX);
        assert(mPlanesY);
        assert(mBoundingSpheres);

        // clip-space dimensions
        const float froxelWidthInClipSpace  = (2.0f * mFroxelDimension.x) / mViewport.width;
        const float froxelHeightInClipSpace = (2.0f * mFroxelDimension.y) / mViewport.height;
        const mat4f invProjection(Camera::inverseProjection(mProjection));

        for (size_t i = 0, n = mFroxelCountX; i <= n; ++i) {
            float x = (i * froxelWidthInClipSpace) - 1.0f;
            // clip-space
            float4 p0 = { x, -1, -1, 1 };
            float4 p1 = { x,  1, -1, 1 };
            // view-space
            p0 = mat4f::project(invProjection, p0);
            p1 = mat4f::project(invProjection, p1);
            mPlanesX[i] = float4(normalize(cross(p1.xyz, p0.xyz)), 0);
        }

        for (size_t i = 0, n = mFroxelCountY; i <= n; ++i) {
            float y = (i * froxelHeightInClipSpace) - 1.0f;
            // clip-space
            float4 p0 = { -1, y, -1, 1 };
            float4 p1 = {  1, y, -1, 1 };
            // view-space
            p0 = mat4f::project(invProjection, p0);
            p1 = mat4f::project(invProjection, p1);
            mPlanesY[i] = float4(normalize(cross(p1.xyz, p0.xyz)), 0);
        }

        // 3-planes intersection:
        //      -d0.(n1 x n2) - d1.(n2 x n0) - d2.(n0 x n1)
        // P = ---------------------------------------------
        //                      n0.(n1 x n2)

        // use stack memory here, it's only 16 KiB max
        assert(mFroxelCountX <= 2048);
        typename std::aligned_storage<sizeof(float2), alignof(float2)>::type stack[2048];
        float2* const UTILS_RESTRICT minMaxX = reinterpret_cast<float2*>(stack);

        float4* const        UTILS_RESTRICT boundingSpheres = mBoundingSpheres;
        float4  const* const UTILS_RESTRICT planesX = mPlanesX;
        float4  const* const UTILS_RESTRICT planesY = mPlanesY;
        float   const* const UTILS_RESTRICT planesZ = mDistancesZ;
        const size_t froxelCountX = mFroxelCountX;
        const size_t froxelCountY = mFroxelCountY;
        UTILS_ASSUME(froxelCountX > 0);
        UTILS_ASSUME(froxelCountY > 0);

        for (size_t iz = 0, fi = 0, nz = mFroxelCountZ; iz < nz; ++iz) {
            float4 planes[6];
            float3 minp;
            float3 maxp;

            // near/far planes for all froxels at iz
            planes[4] =  float4{ 0, 0, 1, planesZ[iz + 0] };
            planes[5] = -float4{ 0, 0, 1, planesZ[iz + 1] };

            // min/max for z is calculated trivially because near/far planes are parallel to
            // the camera.
            minp.z = -planesZ[iz+1];
            maxp.z = -planesZ[iz];
            assert(minp.z < maxp.z);

            for (size_t ix = 0, nx = froxelCountX; ix < nx; ++ix) {
                // left, right planes for all froxels at ix
                planes[0] =  planesX[ix];
                planes[1] = -planesX[ix + 1];
                minp.x = std::numeric_limits<float>::max();
                maxp.x = std::numeric_limits<float>::lowest();
                // min/max for x is calculated by intersecting the near/far and left/right planes
                for (size_t c = 0; c < 4; ++c) {
                    float4 const& p0 = planes[0 + (c  & 1)];    // {x,0,z,0}
                    float4 const& p2 = planes[4 + (c >> 1)];    // {0,0,+/-1,d}
                    float px = (p2.z * p2.w * p0.z) / p0.x;
                    minp.x = std::min(minp.x, px);
                    maxp.x = std::max(maxp.x, px);
                }
                assert(minp.x < maxp.x);
                minMaxX[ix] = float2{ minp.x, maxp.x };
            }

            for (size_t iy = 0, ny = froxelCountY; iy < ny; ++iy) {
                // bottom, top planes for all froxels at iy
                planes[2] =  planesY[iy];
                planes[3] = -planesY[iy + 1];
                minp.y = std::numeric_limits<float>::max();
                maxp.y = std::numeric_limits<float>::lowest();
                // min/max for y is calculated by intersecting the near/far and bottom/top planes
                for (size_t c = 0; c < 4; ++c) {
                    float4 const& p1 = planes[2 + (c &  1)];    // {0,y,z,0}
                    float4 const& p2 = planes[4 + (c >> 1)];    // {0,0,+/-1,d}
                    float py = (p2.z * p2.w * p1.z) / p1.y;
                    minp.y = std::min(minp.y, py);
                    maxp.y = std::max(maxp.y, py);
                }
                assert(minp.y < maxp.y);

                for (size_t ix = 0, nx = froxelCountX; ix < nx; ++ix) {
                    // note: clang vectorizes this loop!
                    assert(getFroxelIndex(ix, iy, iz) == fi);
                    minp.x = minMaxX[ix][0];
                    maxp.x = minMaxX[ix][1];
                    boundingSpheres[fi++] = { (maxp + minp) * 0.5f, length((maxp - minp) * 0.5f) };
                }
            }
        }

        //    linearizer = log2(zLightFar / zLightNear) / (zcount - 1)
        //    vz = -exp2((i - zcount) * linearizer) * zLightFar
        // => i = log2(zLightFar / -vz) / -linearizer + zcount

        float Pz = mProjection[2][2];
        float Pw = mProjection[3][2];
        if (mProjection[2][3] != 0) {
            // perspective projection
            // (clip) cz = Pz*vz+Pw, cw=-vz
            // (ndc)  nz = -Pz-Pw/vz
            // (win)  fz = -Pz*0.5+0.5 - Pw*0.5/vz
            // ->  = vz = -Pw / (2*fz + Pz-1)
            // i = log2(zLightFar*(2*fz + Pz-1) / Pw) / -linearizer + zcount
            mParamsZ[0] = 2.0f * mZLightFar / Pw;
            mParamsZ[1] =  mZLightFar * (Pz - 1.0f) / Pw;
        } else {
            // orthographic projection
            // (clip) cz = Pz*vz+Pw, cw=1
            // (ndc)  nz = Pz*vz+Pw
            // (win)  fz = Pz*vz*0.5 + Pw*0.5+0.5
            // ->  = vz = (2*fz - Pw-1)/Pz
            // i = log2(-vz / zLightFar) / linearizer + zcount
            // i = log2((-2*fz + Pw + 1)/(Pz*zLightFar)) / linearizer + zcount

            mParamsZ[0] = -2.0f / (Pz * mZLightFar);
            mParamsZ[1] = (1.0f + Pw) / (Pz * mZLightFar);
            mParamsZ[2] = mLinearizer;
        }
        uniformsNeedUpdating = true;
    }
    assert(mZLightNear >= mNear);
    mDirtyFlags = 0;
    return uniformsNeedUpdating;
}

Froxel Froxelizer::getFroxelAt(size_t x, size_t y, size_t z) const noexcept {
    assert(x < mFroxelCountX);
    assert(y < mFroxelCountY);
    assert(z < mFroxelCountZ);
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
size_t Froxelizer::findSliceZ(float z) const noexcept {
    // The vastly common case is that z<0, so we always do the math for this case
    // and we "undo" it below otherwise. This works because we're using fast::log2 which
    // doesn't care if given a negative number (we'd have to use abs() otherwise).

    // This whole function is now branch-less.

    int s = int((fast::log2(-z) - mLog2ZLightFar) * mLinearizer + mFroxelCountZ);

    // there are cases where z can be negative here, e.g.:
    // - the light is visible, but its center is behind the camera
    // - the camera's near is behind the camera (e.g. with shadowmap cameras)
    // in that case just return the first slice
    s = z<0 ? s : 0;

    // clamp between [0, mFroxelCountZ)
    return size_t(clamp(s, 0, mFroxelCountZ - 1));
}

std::pair<size_t, size_t> Froxelizer::clipToIndices(float2 const& clip) const noexcept {
    // clip coordinates between [-1, 1], conversion to index between [0, count[
    //  = floor((clip + 1) * ((0.5 * dimension) / froxelsize))
    //  = floor((clip + 1) * constant
    //  = floor(clip * constant + constant)
    const size_t xi = size_t(clamp(int(clip.x * mClipToFroxelX + mClipToFroxelX), 0, mFroxelCountX - 1));
    const size_t yi = size_t(clamp(int(clip.y * mClipToFroxelY + mClipToFroxelY), 0, mFroxelCountY - 1));
    return { xi, yi };
}


void Froxelizer::commit(backend::DriverApi& driverApi) {
    // send data to GPU
    mFroxelBuffer.commit(driverApi, mFroxelBufferUser);
    mRecordsBuffer.commit(driverApi, mRecordBufferUser);
#ifndef NDEBUG
    mFroxelBufferUser.clear();
    mRecordBufferUser.clear();
    mFroxelShardedData.clear();
#endif
}

void Froxelizer::froxelizeLights(FEngine& engine,
        CameraInfo const& UTILS_RESTRICT camera,
        const FScene::LightSoa& UTILS_RESTRICT lightData) noexcept {
    // note: this is called asynchronously
    froxelizeLoop(engine, camera, lightData);
    froxelizeAssignRecordsCompress();

#ifndef NDEBUG
    if (lightData.size()) {
        // go through every froxel
        auto const& recordBufferUser(mRecordBufferUser);
        auto gpuFroxelEntries(mFroxelBufferUser);
        gpuFroxelEntries.set(gpuFroxelEntries.begin(),
                mFroxelCountX * mFroxelCountY * mFroxelCountZ);
        for (auto const& entry : gpuFroxelEntries) {
            // go through every lights for that froxel
            for (size_t i = 0; i < entry.pointLightCount + entry.spotLightCount; i++) {
                // get the light index
                assert(entry.offset + i < RECORD_BUFFER_ENTRY_COUNT);

                size_t lightIndex = recordBufferUser[entry.offset + i];
                assert(lightIndex <= CONFIG_MAX_LIGHT_INDEX);

                // make sure it corresponds to an existing light
                assert(lightIndex < lightData.size() - FScene::DIRECTIONAL_LIGHTS_COUNT);
            }
        }
    }
#endif
}

void Froxelizer::froxelizeLoop(FEngine& engine,
        const CameraInfo& UTILS_RESTRICT camera,
        const FScene::LightSoa& UTILS_RESTRICT lightData) noexcept {
    SYSTRACE_CALL();

    Slice<FroxelThreadData> froxelThreadData = mFroxelShardedData;
    memset(froxelThreadData.data(), 0, froxelThreadData.sizeInBytes());

    auto& lcm = engine.getLightManager();
    auto const* UTILS_RESTRICT spheres      = lightData.data<FScene::POSITION_RADIUS>();
    auto const* UTILS_RESTRICT directions   = lightData.data<FScene::DIRECTION>();
    auto const* UTILS_RESTRICT instances    = lightData.data<FScene::LIGHT_INSTANCE>();

    auto process = [ this, &froxelThreadData,
                     spheres, directions, instances, &camera, &lcm ]
            (size_t count, size_t offset, size_t stride) {

        const mat4f& projection = mProjection;
        const mat3f& vn = camera.view.upperLeft();

        for (size_t i = offset; i < count; i += stride) {
            const size_t j = i + FScene::DIRECTIONAL_LIGHTS_COUNT;
            FLightManager::Instance li = instances[j];
            LightParams light = {
                    .position = (camera.view * float4{ spheres[j].xyz, 1 }).xyz, // to view-space
                    .cosSqr = lcm.getCosOuterSquared(li),   // spot only
                    .axis = vn * directions[j],             // spot only
                    .invSin = lcm.getSinInverse(li),        // spot only
                    .radius = spheres[j].w,
            };

            const size_t group = i % GROUP_COUNT;
            const size_t bit   = i / GROUP_COUNT;
            assert(bit < LIGHT_PER_GROUP);

            FroxelThreadData& threadData = froxelThreadData[group];
            const bool isSpot = light.invSin != std::numeric_limits<float>::infinity();
            threadData[0] |= isSpot << bit;
            froxelizePointAndSpotLight(threadData, bit, projection, light);
        }
    };

    // we do 64 lights per job
    JobSystem& js = engine.getJobSystem();

    constexpr bool SINGLE_THREADED = false;
    if (!SINGLE_THREADED) {
        auto parent = js.createJob();
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

    SYSTRACE_CALL();

    Slice<FroxelThreadData> froxelThreadData = mFroxelShardedData;

    // convert froxel data from N groups of M bits to LightRecord::bitset, so we can
    // easily compare adjacent froxels, for compaction. The conversion loops below get
    // inlined and vectorized in release builds.

    // keep these two loops separate, it helps the compiler a lot
    LightRecord::bitset spotLights;
    for (size_t i = 0; i < LightRecord::bitset::WORLD_COUNT; i++) {
        using container_type = LightRecord::bitset::container_type;
        constexpr size_t r = sizeof(container_type) / sizeof(LightGroupType);
        container_type b = froxelThreadData[i * r][0];
        for (size_t k = 0; k < r; k++) {
            b |= (container_type(froxelThreadData[i * r + k][0]) << (LIGHT_PER_GROUP * k));
        }
        spotLights.getBitsAt(i) = b;
    }

    // this gets very well vectorized...
    utils::Slice<LightRecord> records(mLightRecords);
    for (size_t j = 1, jc = FROXEL_BUFFER_ENTRY_COUNT_MAX; j < jc; j++) {
        for (size_t i = 0; i < LightRecord::bitset::WORLD_COUNT; i++) {
            using container_type = LightRecord::bitset::container_type;
            constexpr size_t r = sizeof(container_type) / sizeof(LightGroupType);
            container_type b = froxelThreadData[i * r][j];
            for (size_t k = 0; k < r; k++) {
                b |= (container_type(froxelThreadData[i * r + k][j]) << (LIGHT_PER_GROUP * k));
            }
            records[j - 1].lights.getBitsAt(i) = b;
        }
    }

    uint16_t offset = 0;
    FroxelEntry* const UTILS_RESTRICT froxels = mFroxelBufferUser.data();

    const size_t froxelCountX = mFroxelCountX;
    auto remap = [stride = size_t(froxelCountX * mFroxelCountY)](size_t i) -> size_t {
        if (SUPPORTS_REMAPPED_FROXELS) {
            // TODO: with the non-square froxel change these would be mask ops instead of divide.
            i = (i % stride) * FEngine::CONFIG_FROXEL_SLICE_COUNT + (i / stride);
        }
        return i;
    };

    RecordBufferType* const UTILS_RESTRICT froxelRecords = mRecordBufferUser.data();

    // how many froxel record entries were reused (for debugging)
    UTILS_UNUSED size_t reused = 0;

    for (size_t i = 0, c = getFroxelCount(); i < c;) {
        LightRecord b = records[i];
        if (b.lights.none()) {
            froxels[remap(i++)].u32 = 0;
            continue;
        }

        // We have a limitation of 255 spot + 255 point lights per froxel.
        // note: initializer list for union cannot have more than one element
        FroxelEntry entry; 
        entry.offset = offset;
        entry.pointLightCount = (uint8_t)std::min(size_t(255), (b.lights & ~spotLights).count());
        entry.spotLightCount  = (uint8_t)std::min(size_t(255), (b.lights &  spotLights).count());

        const size_t lightCount = entry.count[0] + entry.count[1];

        if (UTILS_UNLIKELY(offset + lightCount >= RECORD_BUFFER_ENTRY_COUNT)) {
#ifndef NDEBUG
            slog.d << "out of space: " << i << ", at " << offset << io::endl;
#endif
            // note: instead of dropping froxels we could look for similar records we've already
            // filed up.
            do { // this compiles to memset() when remap() is identity
                froxels[remap(i++)].u32 = 0;
            } while(i < c);
            goto out_of_memory;
        }

        // iterate the bitfield
        auto beginPoint = froxelRecords + offset;
        auto beginSpot  = froxelRecords + offset + entry.count[0];
        b.lights.forEachSetBit([&spotLights,
                point = beginPoint, spot = beginSpot, beginPoint, beginSpot]
                (size_t l) mutable {

            // make sure to keep this code branch-less
            const bool isSpot = spotLights[l];
            auto& p = isSpot ? spot      : point;
            auto  s = isSpot ? beginSpot : beginPoint;

            const size_t word = l / LIGHT_PER_GROUP;
            const size_t bit  = l % LIGHT_PER_GROUP;
            l = (bit * GROUP_COUNT) | (word % GROUP_COUNT);

            *p = (RecordBufferType)l;
            // we need to "cancel" the write if we have more than 255 spot or point lights
            // (this is a limitation of the data type used to store the light counts per froxel)
            p += (p - s < 255) ? 1 : 0;
        });

        offset += lightCount;

#ifndef NDEBUG
        if (lightCount) { reused--; }
#endif
        do {
#ifndef NDEBUG
            if (lightCount) { reused++; }
#endif
            froxels[remap(i++)].u32 = entry.u32;
            if (i >= c) break;

            if (records[i].lights != b.lights && i >= froxelCountX) {
                // if this froxel record doesn't match the previous one on its left,
                // we re-try with the record above it, which saves many froxel records
                // (north of 10% in practice).
                b = records[i - froxelCountX];
                entry.u32 = froxels[remap(i - froxelCountX)].u32;
            }
        } while(records[i].lights == b.lights);
    }
out_of_memory:
    ;
}

static inline float2 project(mat4f const& p, float3 const& v) noexcept {
    const float vx = v[0];
    const float vy = v[1];
    const float vz = v[2];

#ifdef DEBUG_PROJECTION
    const float x = p[0].x*vx + p[1].x*vy + p[2].x*vz + p[3].x;
    const float y = p[0].y*vx + p[1].y*vy + p[2].y*vz + p[3].y;
    const float w = p[0].w*vx + p[1].w*vy + p[2].w*vz + p[3].w;
#else
    // We know we're using a projection matrix (which has a bunch of zeros)
    // But we need to handle asymetric frustums and orthographic projections.
    //       orthographic ------------------------+
    //  asymmetric frustum ---------+             |
    //                              v             v
    const float x = p[0].x * vx + p[2].x * vz + p[3].x;
    const float y = p[1].y * vy + p[2].y * vz + p[3].y;
    const float w = p[2].w * vz               + p[3].w;
#endif
    return float2{ x, y } * (1 / w);
}

void Froxelizer::froxelizePointAndSpotLight(
        FroxelThreadData& froxelThread, size_t bit,
        mat4f const& UTILS_RESTRICT p,
        const Froxelizer::LightParams& UTILS_RESTRICT light) const noexcept {

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
    const size_t x1 = mFroxelCountX;
    const size_t y0 = 0;
    const size_t y1 = mFroxelCountY - 1;
    const size_t z0 = 0;
    const size_t z1 = mFroxelCountZ - 1;
#else
    // find a reasonable bounding-box in froxel space for the sphere by projecting
    // it's (clipped) bounding-box to clip-space and converting to froxel indices.
    Box aabb = { light.position, light.radius };
    const float znear = std::min(-mNear, aabb.center.z + aabb.halfExtent.z); // z values are negative
    const float zfar  =                  aabb.center.z - aabb.halfExtent.z;

    float2 xyLeftNear  = project(p, { aabb.center.xy - aabb.halfExtent.xy, znear });
    float2 xyLeftFar   = project(p, { aabb.center.xy - aabb.halfExtent.xy, zfar  });
    float2 xyRightNear = project(p, { aabb.center.xy + aabb.halfExtent.xy, znear });
    float2 xyRightFar  = project(p, { aabb.center.xy + aabb.halfExtent.xy, zfar  });

    // handle inverted frustums (e.g. x or y symetries)
    if (xyLeftNear.x > xyRightNear.x)   std::swap(xyLeftNear.x, xyRightNear.x);
    if (xyLeftNear.y > xyRightNear.y)   std::swap(xyLeftNear.y, xyRightNear.y);
    if (xyLeftFar.x  > xyRightFar.x)    std::swap(xyLeftFar.x, xyRightFar.x);
    if (xyLeftFar.y  > xyRightFar.y)    std::swap(xyLeftFar.y, xyRightFar.y);

    const auto imin = clipToIndices(min(xyLeftNear, xyLeftFar));
    const size_t x0 = imin.first;
    const size_t y0 = imin.second;
    const size_t z0 = findSliceZ(znear);

    const auto imax = clipToIndices(max(xyRightNear, xyRightFar));
    const size_t x1 = imax.first  + 1;  // x1 points to 1 past the last value (like end() does
    const size_t y1 = imax.second;      // y1 points to the last value
    const size_t z1 = findSliceZ(zfar); // z1 points to the last value

    assert(x0 < x1);
    assert(y0 <= y1);
    assert(z0 <= z1);
#endif

    const size_t zcenter = findSliceZ(s.z);
    float4 const * const UTILS_RESTRICT planesX = mPlanesX;
    float4 const * const UTILS_RESTRICT planesY = mPlanesY;
    float const * const UTILS_RESTRICT planesZ = mDistancesZ;
    float4 const * const UTILS_RESTRICT boundingSpheres = mBoundingSpheres;
    for (size_t iz = z0 ; iz <= z1; ++iz) {
        float4 cz(s);
        if (UTILS_LIKELY(iz != zcenter)) {
            cz = spherePlaneIntersection(s, (iz < zcenter) ? planesZ[iz + 1] : planesZ[iz]);
        }

        // find x & y slices that contain the sphere's center
        // (note: this changes with the Z slices
        const float2 clip = project(p, cz.xyz);
        const auto indices = clipToIndices(clip);
        const size_t xcenter = indices.first;
        const size_t ycenter = indices.second;

        if (cz.w > 0) { // intersection of light with this plane (slice)
            for (size_t iy = y0; iy <= y1; ++iy) {
                float4 cy(cz);
                if (UTILS_LIKELY(iy != ycenter)) {
                    float4 const& plane = iy < ycenter ? planesY[iy + 1] : planesY[iy];
                    cy = spherePlaneIntersection(cz, plane.y, plane.z);
                }
                if (cy.w > 0) { // intersection of light with this horizontal plane
                    size_t bx, ex; // horizontal begin/end indices
                    // find the begin index (left side)
                    for (bx = x0; bx <= xcenter; ++bx) {
                        if (spherePlaneDistanceSquared(cy, planesX[bx].x, planesX[bx].z) > 0) {
                            // intersection
                            break;
                        }
                    }

                    // find the end index (right side), x1 is past the end
                    for (ex = x1; --ex > xcenter;) {
                        if (spherePlaneDistanceSquared(cy, planesX[ex].x, planesX[ex].z) > 0) {
                            // intersection
                            break;
                        }
                    }
                    ++ex;

                    if (UTILS_UNLIKELY(bx >= ex)) {
                        continue;
                    }

                    assert(bx < mFroxelCountX && ex <= mFroxelCountX);

                    // The first entry reserved for type of light, i.e. point/spot
                    size_t fi = getFroxelIndex(bx, iy, iz) + 1;
                    if (light.invSin != std::numeric_limits<float>::infinity()) {
                        // This is a spotlight (common case)
                        // this loops gets vectorized (on arm64) w/ clang
                        while (bx++ != ex) {
                            // see if this froxel intersects the cone
                            bool intersect = sphereConeIntersectionFast(boundingSpheres[fi - 1],
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
        utils::Slice<RecordBufferType> const& lightList,
        const FScene::LightSoa& lightData,
        size_t lightRecordsOffset) noexcept {

    // number of lights in this record
    const size_t count = lightList.size();

    // the width of the tree is the next power-of-two (if not already a power of two)
    const size_t w = 1u << (log2i(count) + (utils::popcount(count) == 1 ? 0 : 1));

    // height of the tree
    const size_t h = log2i(w) + 1u;

    auto const* UTILS_RESTRICT zrange = lightData.data<FScene::SCREEN_SPACE_Z_RANGE>() + 1;
    BinaryTreeArray::traverse(h,
            [lightTree, lightRecordsOffset, zrange, indices = lightList.data(), count]
            (size_t index, size_t col, size_t next) {
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
            [lightTree](size_t index, size_t l, size_t r, size_t next) {
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

} // namespace details
} // namespace filament
