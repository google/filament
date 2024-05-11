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

#ifndef TNT_FILAMENT_DETAILS_FROXELIZER_H
#define TNT_FILAMENT_DETAILS_FROXELIZER_H

#include "Allocators.h"

#include "details/Scene.h"
#include "details/Engine.h"

#include <filament/Viewport.h>

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/bitset.h>
#include <utils/Slice.h>

#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

// Max number of froxels limited by:
//   - max ubo size [min 16KiB]
//
// Also, increasing the number of froxels adds more pressure on the "record buffer" which stores
// the light indices per froxel. The record buffer is limited to min(16K[ubo], 64K[uint16]) entries,
// so with 8192 froxels, we can store 2 lights per froxels assuming they're all used. In practice,
// some froxels are not used, so we can store more.
constexpr size_t FROXEL_BUFFER_MAX_ENTRY_COUNT = 8192;

class FEngine;
class FCamera;
class FTexture;

class Froxel {
public:
    enum Planes {
        LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
    };
    math::float4 planes[6];
};

//
// Light UBO           Froxel Record UBO      per-froxel light list texture
// {4 x float4}            {index into        RG_U16 {offset, point-count, spot-count}
// (spot/point            light texture}
//                     {uint4 -> 16 indices}
//
//  +----+                     +-+                     +----+
// 0|....| <------------+     0| |         +-----------|0230| (e.g. offset=02, 3-lights)
// 1|....|<--------+     \    1| |        /            |    |
// 2:    :          \     +---2|0|<------+             |    |
// 3:    : <-------- \--------3|3|                     :    :
// 4:    :            +------- :1:                     :    :
//  :    :                     : :                     :    :
//  :    :                     | |                     |    |
//  :    :                     | |                     |    |
//  :    :                     +-+                     |    |
//  :    :                  65536 max                  +----+
//  |....|                                          h = num froxels
//  |....|
//  +----+
// 256 lights max
//

class Froxelizer {
public:
    explicit Froxelizer(FEngine& engine);
    ~Froxelizer();

    void terminate(backend::DriverApi& driverApi) noexcept;

    // gpu buffer containing records. valid after construction.
    backend::Handle<backend::HwBufferObject> getRecordBuffer() const noexcept {
        return mRecordsBuffer;
    }

    // gpu buffer containing froxels. valid after construction.
    backend::Handle<backend::HwBufferObject> getFroxelBuffer() const noexcept {
        return mFroxelsBuffer;
    }

    void setOptions(float zLightNear, float zLightFar) noexcept;

    /*
     * Allocate per-frame data structures for froxelization.
     *
     * driverApi         used to allocate memory in the stream
     * arena             used to allocate per-frame memory
     * viewport          used to calculate froxel dimensions
     * projection        camera projection matrix
     * projectionNear    near plane
     * projectionFar     far plane
     *
     * return true if updateUniforms() needs to be called
     */
    bool prepare(backend::DriverApi& driverApi, RootArenaScope& rootArenaScope, Viewport const& viewport,
            const math::mat4f& projection, float projectionNear, float projectionFar) noexcept;

    Froxel getFroxelAt(size_t x, size_t y, size_t z) const noexcept;
    size_t getFroxelCountX() const noexcept { return mFroxelCountX; }
    size_t getFroxelCountY() const noexcept { return mFroxelCountY; }
    size_t getFroxelCountZ() const noexcept { return mFroxelCountZ; }
    size_t getFroxelCount() const noexcept { return mFroxelCount; }

    float getLightFar() const noexcept { return mZLightFar; }

    // update Records and Froxels texture with lights data. this is thread-safe.
    void froxelizeLights(FEngine& engine, math::mat4f const& viewMatrix,
            const FScene::LightSoa& lightData) noexcept;

    void updateUniforms(PerViewUib& s) {
        s.zParams = mParamsZ;
        s.fParams = mParamsF;
        s.froxelCountXY = math::float2{ mViewport.width, mViewport.height } / mFroxelDimension;
    }

    // send froxel data to GPU
    void commit(backend::DriverApi& driverApi);


    /*
     * Only for testing/debugging...
     */

    struct FroxelEntry {
        inline FroxelEntry(uint16_t offset, uint8_t count) noexcept
            : u32((offset << 16) | count) { }
        inline uint8_t count() const noexcept { return u32 & 0xFFu; }
        inline uint16_t offset() const noexcept { return u32 >> 16u; }
        uint32_t u32 = 0;
    };

    // we can't change this easily because the shader expects 16 indices per uint4
    using RecordBufferType = uint8_t;

    const utils::Slice<FroxelEntry>& getFroxelBufferUser() const { return mFroxelBufferUser; }
    const utils::Slice<RecordBufferType>& getRecordBufferUser() const { return mRecordBufferUser; }

    // this is chosen so froxelizePointAndSpotLight() vectorizes 4 froxel tests / spotlight
    // with 256 lights this implies 8 jobs (256 / 32) for froxelization.
    using LightGroupType = uint32_t;

private:
    size_t getFroxelBufferEntryCount() const noexcept {
        return mFroxelBufferEntryCount;
    }

    struct LightRecord {
        using bitset = utils::bitset<uint64_t, (CONFIG_MAX_LIGHT_COUNT + 63) / 64>;
        bitset lights;
    };

    struct LightParams {
        math::float3 position;
        float cosSqr;
        math::float3 axis;
        // this must be initialized to indicate this is a point light
        float invSin = std::numeric_limits<float>::infinity();
        // radius is not used in the hot loop, so leave it at the end
        float radius;
    };

    struct LightTreeNode {
        float min;          // lights z-range min
        float max;          // lights z-range max

        uint16_t next;      // next node when range test fails
        uint16_t offset;    // offset in record buffer

        uint8_t isLeaf;
        uint8_t count;      // light count in record buffer
        uint16_t reserved;
    };

    struct FroxelThreadData;

    inline void setViewport(Viewport const& viewport) noexcept;
    inline void setProjection(const math::mat4f& projection, float near, float far) noexcept;
    bool update() noexcept;

    void froxelizeLoop(FEngine& engine,
            math::mat4f const& viewMatrix, const FScene::LightSoa& lightData) noexcept;

    void froxelizeAssignRecordsCompress() noexcept;

    void froxelizePointAndSpotLight(FroxelThreadData& froxelThread, size_t bit,
            math::mat4f const& projection, const LightParams& light) const noexcept;

    static void computeLightTree(LightTreeNode* lightTree,
            utils::Slice<RecordBufferType> const& lightList,
            const FScene::LightSoa& lightData, size_t lightRecordsOffset) noexcept;

    static void updateBoundingSpheres(
            math::float4* UTILS_RESTRICT boundingSpheres,
            size_t froxelCountX, size_t froxelCountY, size_t froxelCountZ,
            math::float4 const* UTILS_RESTRICT planesX,
            math::float4 const* UTILS_RESTRICT planesY,
            float const* UTILS_RESTRICT planesZ) noexcept;

    static size_t getFroxelIndex(size_t ix, size_t iy, size_t iz,
            size_t froxelCountX, size_t froxelCountY) noexcept {
        return ix + (iy * froxelCountX) + (iz * froxelCountX * froxelCountY);
    }

    size_t getFroxelIndex(size_t ix, size_t iy, size_t iz) const noexcept {
        return getFroxelIndex(ix, iy, iz, mFroxelCountX, mFroxelCountY);
    }

    size_t findSliceZ(float viewSpaceZ) const noexcept UTILS_PURE;

    std::pair<size_t, size_t> clipToIndices(math::float2 const& clip) const noexcept;

    static void computeFroxelLayout(
            math::uint2* dim, uint16_t* countX, uint16_t* countY, uint16_t* countZ,
            size_t froxelBufferEntryCount, Viewport const& viewport) noexcept;

    // internal state dependent on the viewport and needed for froxelizing
    LinearAllocatorArena mArena;                        // ~256 KiB

    // 4096 froxels fits in a 16KiB buffer, the minimum guaranteed in GLES 3.x and Vulkan 1.1
    size_t mFroxelBufferEntryCount = 4096;

    // allocations in the private froxel arena
    float* mDistancesZ = nullptr;
    math::float4* mPlanesX = nullptr;
    math::float4* mPlanesY = nullptr;
    math::float4* mBoundingSpheres = nullptr;           // 128 KiB w/ 8192 froxels

    // allocations in the per frame arena
    utils::Slice<FroxelThreadData> mFroxelShardedData;  // 256 KiB w/  256 lights and 8192 froxels
    utils::Slice<FroxelEntry> mFroxelBufferUser;        //  32 KiB w/ 8192 froxels
    utils::Slice<LightRecord> mLightRecords;            // 256 KiB w/  256 lights

    // allocations in the command stream
    utils::Slice<RecordBufferType> mRecordBufferUser;   //  16 KiB

    uint16_t mFroxelCountX = 0;
    uint16_t mFroxelCountY = 0;
    uint16_t mFroxelCountZ = 0;
    uint32_t mFroxelCount = 0;
    math::uint2 mFroxelDimension = {};

    math::mat4f mProjection;
    float mLinearizer = 0.0f;
    float mClipToFroxelX = 0.0f;
    float mClipToFroxelY = 0.0f;
    backend::BufferObjectHandle mRecordsBuffer;
    backend::BufferObjectHandle mFroxelsBuffer;

    // needed for update()
    Viewport mViewport;
    math::float4 mParamsZ = {};
    math::uint3 mParamsF = {};
    float mNear = 0.0f;        // camera near
    float mZLightNear;
    float mZLightFar;

    // track if we need to update our internal state before froxelizing
    uint8_t mDirtyFlags = 0;
    enum {
        VIEWPORT_CHANGED = 0x01,
        PROJECTION_CHANGED = 0x02
    };
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FROXELIZER_H
