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

#ifndef TNT_FILAMENT_DETAILS_FROXEL_H
#define TNT_FILAMENT_DETAILS_FROXEL_H

#include "details/Allocators.h"
#include "details/Scene.h"
#include "details/Engine.h"

#include "driver/Handle.h"
#include "driver/GPUBuffer.h"
#include "driver/UniformBuffer.h"

#include <filament/Viewport.h>

#include <utils/compiler.h>
#include <utils/bitset.h>
#include <utils/Slice.h>

#include <math/mat4.h>
#include <math/vec4.h>

#include <vector>

namespace filament {
namespace details {

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
// {4 x float4}         R_U8  {index into        RG_U16 {offset, point-count, spot-sount}
// (spot/point            light texture}
//
//  +----+                     +-+                     +----+
// 0|....| <------------+     0| |         +-----------|0221| (e.g. offset=02, 2-point, 1-spot)
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

    void terminate(driver::DriverApi& driverApi) noexcept;

    // gpu buffer containing records. valid after construction.
    GPUBuffer const& getRecordBuffer() const noexcept { return mRecordsBuffer; }

    // gpu buffer containing froxels. valid after construction.
    GPUBuffer const& getFroxelBuffer() const noexcept { return mFroxelBuffer; }

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
    bool prepare(driver::DriverApi& driverApi, ArenaScope& arena, Viewport const& viewport,
            const math::mat4f& projection, float projectionNear, float projectionFar) noexcept;

    Froxel getFroxelAt(size_t x, size_t y, size_t z) const noexcept;
    size_t getFroxelCountX() const noexcept { return mFroxelCountX; }
    size_t getFroxelCountY() const noexcept { return mFroxelCountY; }
    size_t getFroxelCountZ() const noexcept { return mFroxelCountZ; }

    // update Records and Froxels texture with lights data. this is thread-safe.
    void froxelizeLights(FEngine& engine, math::mat4f const& viewMatrix,
            const FScene::LightSoa& lightData) noexcept;

    void updateUniforms(UniformBuffer& u) {
        u.setUniform(offsetof(FEngine::PerViewUib, zParams), mParamsZ);
        u.setUniform(offsetof(FEngine::PerViewUib, fParams), mParamsF);
        u.setUniform(offsetof(FEngine::PerViewUib, oneOverFroxelDimensionX), mOneOverDimension.x);
        u.setUniform(offsetof(FEngine::PerViewUib, oneOverFroxelDimensionY), mOneOverDimension.y);
    }

    // send froxel data to GPU
    void commit(driver::DriverApi& driverApi);


    /*
     * Only for testing/debugging...
     */

    struct FroxelEntry {
        union {
            uint32_t u32;
            struct {
                uint16_t offset = 0;
                union {
                    uint8_t count[2] = { 0, 0 };
                    struct {
                        uint8_t pointLightCount;
                        uint8_t spotLightCount;
                    };
                };
            };
        };
    };
    // This depends on the maximum number of lights (currently 256),and can't be more than 16 bits.
    using RecordBufferType = std::conditional_t<CONFIG_MAX_LIGHT_COUNT <= 255, uint8_t, uint16_t>;
    const utils::Slice<FroxelEntry>& getFroxelBufferUser() const { return mFroxelBufferUser; }
    const utils::Slice<RecordBufferType>& getRecordBufferUser() const { return mRecordBufferUser; }

private:
    struct FroxelRunEntry {
        uint32_t index;
        uint32_t count;
    };

    struct LightRecord {
        utils::bitset256 lights;
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

    void setViewport(Viewport const& viewport) noexcept;
    void setProjection(const math::mat4f& projection, float near, float far) noexcept;
    bool update() noexcept;

    void froxelizeLoop(FEngine& engine, utils::Slice<uint16_t>& froxelsList,
            utils::Slice<FroxelRunEntry> froxelsListIndices, const math::mat4f& viewMatrix,
            const FScene::LightSoa& lightData) noexcept;

    void froxelizePointAndSpotLight(
            utils::GrowingSlice<uint16_t>& froxels,
            math::mat4f const& projection, const LightParams& light) const noexcept;

    void froxelizeAssignRecordsCompress(
            const utils::Slice<uint16_t>& froxelsList,
            const utils::Slice<FroxelRunEntry>& froxelsListIndices) noexcept;

    uint16_t getFroxelIndex(size_t ix, size_t iy, size_t iz) const noexcept {
        return uint16_t(ix + (iy * mFroxelCountX) + (iz * mFroxelCountX * mFroxelCountY));
    }

    size_t findSliceZ(float viewSpaceZ) const noexcept UTILS_PURE;

    std::pair<size_t, size_t> clipToIndices(math::float2 const& clip) const noexcept;

    static void computeFroxelLayout(
            math::uint2* dim, uint16_t* countX, uint16_t* countY, uint16_t* countZ,
            Viewport const& viewport) noexcept;

    static math::float4 spherePlaneIntersection(math::float4 s, math::float4 p) noexcept;
    static float spherePlaneDistanceSquared(math::float4 s, float x, float z) noexcept;
    static math::float4 spherePlaneIntersection(math::float4 s, float py, float pz) noexcept;
    static math::float4 spherePlaneIntersection(math::float4 s, float pw) noexcept;
    static bool sphereConeIntersectionFast(math::float4 const& sphere, Froxelizer::LightParams const& cone) noexcept;
    static bool sphereConeIntersection(math::float4 const& sphere, Froxelizer::LightParams const& cone) noexcept;


    // internal state dependant on the viewport and needed for froxelizing
    LinearAllocatorArena mArena;                    // ~256 KiB

    float* mDistancesZ = nullptr;                   // max 2.1 MiB (actual: resolution dependant)
    math::float4* mPlanesX = nullptr;
    math::float4* mPlanesY = nullptr;
    math::float4* mBoundingSpheres = nullptr;

    utils::Slice<FroxelRunEntry> mFroxelListIndices;    // ~2 KiB
    utils::Slice<uint16_t> mFroxelList;                 // ~4 MiB + 510 B
    utils::Slice<FroxelEntry> mFroxelBufferUser;

    // max 32 KiB  (actual: resolution dependant)
    utils::Slice<RecordBufferType> mRecordBufferUser;   // max 64 KiB
    utils::Slice<LightRecord> mLightRecords;            // 256 KiB

    uint16_t mFroxelCountX = 0;
    uint16_t mFroxelCountY = 0;
    uint16_t mFroxelCountZ = 0;
    math::uint2 mFroxelDimension = {};

    math::mat4f mProjection;
    float mLinearizer = 0.0f;
    float mLog2ZLightFar = 0.0f;
    float mClipToFroxelX = 0.0f;
    float mClipToFroxelY = 0.0f;
    math::float2 mOneOverDimension = {};
    GPUBuffer mRecordsBuffer;
    GPUBuffer mFroxelBuffer;

    // needed for update()
    Viewport mViewport;
    math::float4 mParamsZ = {};
    math::uint4 mParamsF = {};
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

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FROXEL_H
