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
// Light UBO           Froxel Record Buffer     per-froxel light list texture
// {4 x float4}         R_U8  {index into        RG_U16 {offset, point-count, spot-count}
// (spot/point            light texture}
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

// Max number of froxels limited by:
// - max texture size [min 2048]
// - chosen texture width [64]
// - size of CPU-side indices [16 bits]
// Also, increasing the number of froxels adds more pressure on the "record buffer" which stores
// the light indices per froxel. The record buffer is limited to 65536 entries, so with
// 8192 froxels, we can store 8 lights per froxels assuming they're all used. In practice, some
// froxels are not used, so we can store more.
static constexpr size_t FROXEL_BUFFER_ENTRY_COUNT_MAX = 8192;

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
    backend::Handle<backend::HwTexture> getFroxelTexture() const noexcept { return mFroxelTexture; }

    void setOptions(float zLightNear, float zLightFar) noexcept;

    /*
     * Allocate per-frame data structures for froxelization.
     *
     * driverApi         used to allocate memory in the stream
     * arena             use to allocate per-frame memory
     * viewport          viewport used to calculate froxel dimensions
     * projection        camera projection matrix
     * projectionNear    near plane
     * projectionFar     far plane
     *
     * return true if updateUniforms() needs to be called
     */
    bool prepare(backend::DriverApi& driverApi, ArenaScope& arena, Viewport const& viewport,
            const math::mat4f& projection, float projectionNear, float projectionFar) noexcept;

    Froxel getFroxelAt(size_t x, size_t y, size_t z) const noexcept;
    size_t getFroxelCountX() const noexcept { return mFroxelCountX; }
    size_t getFroxelCountY() const noexcept { return mFroxelCountY; }
    size_t getFroxelCountZ() const noexcept { return mFroxelCountZ; }
    size_t getFroxelCount() const noexcept { return mFroxelCount; }

    float getLightFar() const noexcept { return mZLightFar; }

    // update Records and Froxels texture with lights data. this is thread-safe.
    void froxelizeLights(FEngine& engine, CameraInfo const& camera,
            const FScene::LightSoa& lightData) noexcept;

    void updateUniforms(PerViewUib& s) {
        s.zParams = mParamsZ;
        s.fParams = mParamsF.yz;
        s.fParamsX = mParamsF.x;
        s.oneOverFroxelDimensionX = mOneOverDimension.x;
        s.oneOverFroxelDimensionY = mOneOverDimension.y;
    }

    // send froxel data to GPU
    void commit(backend::DriverApi& driverApi);


    /*
     * Only for testing/debugging...
     */

    struct FroxelEntry {
        union {
            uint32_t u32 = 0;
            struct {
                uint16_t offset;
                uint8_t count;
                uint8_t reserved;
            };
        };
    };
    // This depends on the maximum number of lights (currently 255),and can't be more than 16 bits.
    static_assert(CONFIG_MAX_LIGHT_INDEX <= std::numeric_limits<uint16_t>::max(), "can't have more than 65536 lights");
    using RecordBufferType = std::conditional_t<CONFIG_MAX_LIGHT_INDEX <= std::numeric_limits<uint8_t>::max(), uint8_t, uint16_t>;
    const utils::Slice<FroxelEntry>& getFroxelBufferUser() const { return mFroxelBufferUser; }
    const utils::Slice<RecordBufferType>& getRecordBufferUser() const { return mRecordBufferUser; }

    // this is chosen so froxelizePointAndSpotLight() vectorizes 4 froxel tests / spotlight
    // with 256 lights this implies 8 jobs (256 / 32) for froxelization.
    using LightGroupType = uint32_t;

private:
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

    using FroxelThreadData = std::array<LightGroupType, FROXEL_BUFFER_ENTRY_COUNT_MAX>;

    inline void setViewport(Viewport const& viewport) noexcept;
    inline void setProjection(const math::mat4f& projection, float near, float far) noexcept;
    bool update() noexcept;

    void froxelizeLoop(FEngine& engine,
            const CameraInfo& camera, const FScene::LightSoa& lightData) noexcept;

    void froxelizeAssignRecordsCompress() noexcept;

    void froxelizePointAndSpotLight(FroxelThreadData& froxelThread, size_t bit,
            math::mat4f const& projection, const LightParams& light) const noexcept;

    static void computeLightTree(LightTreeNode* lightTree,
            utils::Slice<RecordBufferType> const& lightList,
            const FScene::LightSoa& lightData, size_t lightRecordsOffset) noexcept;

    uint16_t getFroxelIndex(size_t ix, size_t iy, size_t iz) const noexcept {
        return uint16_t(ix + (iy * mFroxelCountX) + (iz * mFroxelCountX * mFroxelCountY));
    }

    size_t findSliceZ(float viewSpaceZ) const noexcept UTILS_PURE;

    std::pair<size_t, size_t> clipToIndices(math::float2 const& clip) const noexcept;

    static void computeFroxelLayout(
            math::uint2* dim, uint16_t* countX, uint16_t* countY, uint16_t* countZ,
            Viewport const& viewport) noexcept;

    // internal state dependant on the viewport and needed for froxelizing
    LinearAllocatorArena mArena;                    // ~256 KiB

    float* mDistancesZ = nullptr;                   // max 2.1 MiB (actual: resolution dependant)
    math::float4* mPlanesX = nullptr;
    math::float4* mPlanesY = nullptr;
    math::float4* mBoundingSpheres = nullptr;

    utils::Slice<FroxelThreadData> mFroxelShardedData;  // 256 KiB w/  256 lights
    utils::Slice<FroxelEntry> mFroxelBufferUser;        //  32 KiB w/ 8192 froxels

    // max 32 KiB  (actual: resolution dependant)
    utils::Slice<RecordBufferType> mRecordBufferUser;   //  16 KiB
    utils::Slice<LightRecord> mLightRecords;            // 256 KiB w/ 256 lights

    uint16_t mFroxelCountX = 0;
    uint16_t mFroxelCountY = 0;
    uint16_t mFroxelCountZ = 0;
    uint16_t mFroxelCount = 0;
    math::uint2 mFroxelDimension = {};

    math::mat4f mProjection;
    float mLinearizer = 0.0f;
    float mClipToFroxelX = 0.0f;
    float mClipToFroxelY = 0.0f;
    math::float2 mOneOverDimension = {};
    backend::BufferObjectHandle mRecordsBuffer;
    backend::Handle<backend::HwTexture> mFroxelTexture;

    // needed for update()
    Viewport mViewport;
    math::float4 mParamsZ = {};
    math::uint3 mParamsF = {};
    float mNear = 0.0f;        // camera near
    float mZLightFar = FEngine::CONFIG_Z_LIGHT_FAR;
    float mZLightNear = FEngine::CONFIG_Z_LIGHT_NEAR;  // light near (first slice)

    // track if we need to update our internal state before froxelizing
    uint8_t mDirtyFlags = 0;
    enum {
        VIEWPORT_CHANGED = 0x01,
        PROJECTION_CHANGED = 0x02
    };
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FROXELIZER_H
