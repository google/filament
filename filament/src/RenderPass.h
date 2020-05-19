/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_UTILS_RENDERPASS_H
#define TNT_UTILS_RENDERPASS_H

#include <filament/Viewport.h>

#include "details/Camera.h"
#include "details/Material.h"
#include "details/Scene.h"

#include "private/backend/DriverApiForward.h"

#include <private/filament/Variant.h>

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <limits>

namespace utils {
class JobSystem;
}

namespace filament {

class RenderPass {
public:
    static constexpr uint64_t DISTANCE_BITS_MASK            = 0xFFFFFFFFllu;
    static constexpr unsigned DISTANCE_BITS_SHIFT           = 0;

    static constexpr uint64_t BLEND_ORDER_MASK              = 0xFFFEllu;
    static constexpr unsigned BLEND_ORDER_SHIFT             = 1;

    static constexpr uint64_t BLEND_TWO_PASS_MASK           = 0x1llu;
    static constexpr unsigned BLEND_TWO_PASS_SHIFT          = 0;

    static constexpr uint64_t MATERIAL_INSTANCE_ID_MASK     = 0x00000FFFllu;
    static constexpr unsigned MATERIAL_INSTANCE_ID_SHIFT    = 0;

    static constexpr uint64_t MATERIAL_VARIANT_KEY_MASK     = 0x000FF000llu;
    static constexpr unsigned MATERIAL_VARIANT_KEY_SHIFT    = 12;

    static constexpr uint64_t MATERIAL_ID_MASK              = 0xFFF00000llu;
    static constexpr unsigned MATERIAL_ID_SHIFT             = 20;

    static constexpr uint64_t BLEND_DISTANCE_MASK           = 0xFFFFFFFF0000llu;
    static constexpr unsigned BLEND_DISTANCE_SHIFT          = 16;

    static constexpr uint64_t MATERIAL_MASK                 = 0xFFFFFFFFllu;
    static constexpr unsigned MATERIAL_SHIFT                = 0;

    static constexpr uint64_t Z_BUCKET_MASK                 = 0x3FF00000000llu;
    static constexpr unsigned Z_BUCKET_SHIFT                = 32;

    static constexpr uint64_t PRIORITY_MASK                 = 0x001C000000000000llu;
    static constexpr unsigned PRIORITY_SHIFT                = 50;

    static constexpr uint64_t BLENDING_MASK                 = 0x0020000000000000llu;
    static constexpr unsigned BLENDING_SHIFT                = 53;

    static constexpr uint64_t PASS_MASK                     = 0xFC00000000000000llu;
    static constexpr unsigned PASS_SHIFT                    = 58;

    static constexpr uint64_t CUSTOM_MASK                   = 0x0300000000000000llu;
    static constexpr unsigned CUSTOM_SHIFT                  = 56;

    static constexpr uint64_t CUSTOM_ORDER_MASK             = 0x003FFFFF00000000llu;
    static constexpr unsigned CUSTOM_ORDER_SHIFT            = 32;

    static constexpr uint64_t CUSTOM_INDEX_MASK             = 0x00000000FFFFFFFFllu;
    static constexpr unsigned CUSTOM_INDEX_SHIFT            = 0;


    enum class Pass : uint64_t {    // 6-bits max
        DEPTH    = uint64_t(0x00) << PASS_SHIFT,
        COLOR    = uint64_t(0x01) << PASS_SHIFT,
        REFRACT  = uint64_t(0x02) << PASS_SHIFT,
        BLENDED  = uint64_t(0x03) << PASS_SHIFT,
        SENTINEL = 0xffffffffffffffffllu
    };

    enum class CustomCommand : uint64_t {    // 2-bits max
        PROLOG  = uint64_t(0x0) << CUSTOM_SHIFT,
        PASS    = uint64_t(0x1) << CUSTOM_SHIFT,
        EPILOG  = uint64_t(0x2) << CUSTOM_SHIFT
    };

    enum CommandTypeFlags : uint8_t {
        COLOR = 0x1,    // generate the color pass only
        DEPTH = 0x2,    // generate the depth pass only ( e.g. shadowmap)

        // shadow-casters are rendered in the depth buffer, regardless of blending (or alpha masking)
        DEPTH_CONTAINS_SHADOW_CASTERS = 0x4,
        // alpha-blended objects are not rendered in the depth buffer
        DEPTH_FILTER_TRANSLUCENT_OBJECTS = 0x8,
        // alpha-tested objects are not rendered in the depth buffer
        DEPTH_FILTER_ALPHA_MASKED_OBJECTS = 0x10,

        // generate commands for shadow map
        SHADOW = DEPTH | DEPTH_CONTAINS_SHADOW_CASTERS,
        // generate commands for SSAO
        SSAO = DEPTH | DEPTH_FILTER_TRANSLUCENT_OBJECTS,
    };



    // Command key encoding
    // --------------------
    //
    // a     = alpha masking
    // ppp   = priority
    // t     = two-pass transparency ordering
    // 0     = reserved, must be zero
    //
    // DEPTH command
    // |   6  | 2| 2|1| 3 | 2|       16       |               32               |
    // +------+--+--+-+---+--+----------------+--------------------------------+
    // |000000|01|00|0|ppp|00|0000000000000000|          distanceBits          |
    // +------+--+--+-+---+-------------------+--------------------------------+
    // | correctness      |     optimizations (truncation allowed)             |
    //
    //
    // COLOR command
    // |   6  | 2| 2|1| 3 | 2|  6   |   10     |               32               |
    // +------+--+--+-+---+--+------+----------+--------------------------------+
    // |000001|01|00|a|ppp|00|000000| Z-bucket |          material-id           |
    // |000010|01|00|a|ppp|00|000000| Z-bucket |          material-id           | refraction
    // +------+--+--+-+---+--+------+----------+--------------------------------+
    // | correctness      |      optimizations (truncation allowed)             |
    //
    //
    // BLENDED command
    // |   6  | 2| 2|1| 3 | 2|              32                |         15    |1|
    // +------+--+--+-+---+--+--------------------------------+---------------+-+
    // |000011|01|00|0|ppp|00|         ~distanceBits          |   blendOrder  |t|
    // +------+--+--+-+---+--+--------------------------------+---------------+-+
    // | correctness                                                            |
    //
    //
    // pre-CUSTOM command
    // |   6  | 2| 2|         22           |               32               |
    // +------+--+--+----------------------+--------------------------------+
    // | pass |00|00|        order         |      custom command index      |
    // +------+--+--+----------------------+--------------------------------+
    // | correctness                                                        |
    //
    //
    // post-CUSTOM command
    // |   6  | 2| 2|         22           |               32               |
    // +------+--+--+----------------------+--------------------------------+
    // | pass |11|00|        order         |      custom command index      |
    // +------+--+--+----------------------+--------------------------------+
    // | correctness                                                        |
    //
    //
    // SENTINEL command
    // |                                   64                                  |
    // +--------.--------.--------.--------.--------.--------.--------.--------+
    // |11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111|
    // +-----------------------------------------------------------------------+
    //
    using CommandKey = uint64_t;


    // The sorting material key is 32 bits and encoded as:
    //
    // |     12     |   8    |     12     |
    // +------------+--------+------------+
    // |  material  |variant |  instance  |
    // +------------+--------+------------+
    //
    // The variant is inserted while building the commands, because we don't know it before that
    //
    static CommandKey makeMaterialSortingKey(uint32_t materialId, uint32_t instanceId) noexcept {
        CommandKey key = ((materialId << MATERIAL_ID_SHIFT) & MATERIAL_ID_MASK) |
                         ((instanceId << MATERIAL_INSTANCE_ID_SHIFT) & MATERIAL_INSTANCE_ID_MASK);
        return (key << MATERIAL_SHIFT) & MATERIAL_MASK;
    }

    template<typename T>
    static CommandKey makeField(T value, uint64_t mask, unsigned shift) noexcept {
        assert(!((uint64_t(value) << shift) & ~mask));
        return uint64_t(value) << shift;
    }

    template<typename T>
    static CommandKey makeFieldTruncate(T value, uint64_t mask, unsigned shift) noexcept {
        return (uint64_t(value) << shift) & mask;
    }


    template<typename T>
    static CommandKey select(T boolish) noexcept {
        return boolish ? -1llu : 0llu;
    }

    struct PrimitiveInfo { // 24 bytes
        FMaterialInstance const* mi = nullptr;                          // 8 bytes (4)
        backend::Handle<backend::HwRenderPrimitive> primitiveHandle;    // 4 bytes
        backend::Handle<backend::HwUniformBuffer> perRenderableBones;   // 4 bytes
        backend::RasterState rasterState;                               // 4 bytes
        uint16_t index = 0;                                             // 2 bytes
        Variant materialVariant;                                        // 1 byte
        uint8_t reserved = {};                                          // 1 byte
    };

    struct alignas(8) Command {     // 32 bytes
        CommandKey key = 0;         //  8 bytes
        PrimitiveInfo primitive;    // 24 bytes
        bool operator < (Command const& rhs) const noexcept { return key < rhs.key; }
        // placement new declared as "throw" to avoid the compiler's null-check
        inline void* operator new (std::size_t size, void* ptr) {
            assert(ptr);
            return ptr;
        }
    };
    static_assert(std::is_trivially_destructible<Command>::value,
            "Command isn't trivially destructible");

    using RenderFlags = uint8_t;
    static constexpr RenderFlags HAS_SHADOWING           = 0x01;
    static constexpr RenderFlags HAS_DIRECTIONAL_LIGHT   = 0x02;
    static constexpr RenderFlags HAS_DYNAMIC_LIGHTING    = 0x04;
    static constexpr RenderFlags HAS_INVERSE_FRONT_FACES = 0x08;
    static constexpr RenderFlags HAS_FOG                 = 0x10;


    RenderPass(FEngine& engine, utils::GrowingSlice<Command> commands) noexcept;
    void overridePolygonOffset(backend::PolygonOffset* polygonOffset) noexcept;
    void setGeometry(FScene::RenderableSoa const& soa, utils::Range<uint32_t> vr,
            backend::Handle<backend::HwUniformBuffer> uboHandle) noexcept;
    void setCamera(const CameraInfo& camera) noexcept;
    void setRenderFlags(RenderFlags flags) noexcept;

    // Sets the visibility mask, which is AND-ed against each Renderable's VISIBLE_MASK to determine
    // if the renderable is visible for this pass.
    // Defaults to all 1's, which means all renderables in this render pass will be rendered.
    void setVisibilityMask(FScene::VisibleMaskType mask) noexcept { mVisibilityMask = mask; }

    // Resets the visibility mask to the default value of all 1's.
    void clearVisibilityMask() noexcept {
        mVisibilityMask = std::numeric_limits<FScene::VisibleMaskType>::max();
    }

    Command* begin() noexcept { return mCommands.begin(); }
    Command* end() noexcept { return mCommands.end(); }

    Command const* begin() const noexcept { return mCommands.begin(); }
    Command const* end() const noexcept { return mCommands.end(); }

    Command* newCommandBuffer() noexcept;

    // returns mCommands.end()
    Command* appendCommands(CommandTypeFlags commandTypeFlags) noexcept;

    // returns mCommands.end()
    Command* appendCustomCommand(Pass pass, CustomCommand custom, uint32_t order,
            std::function<void()> command);

    // sorts commands, then trims sentinels and returns
    // the new mCommands.end()
    Command* sortCommands() noexcept;

    void execute(const char* name,
            backend::Handle<backend::HwRenderTarget> renderTarget,
            backend::RenderPassParams params) const noexcept;

    utils::GrowingSlice<Command>& getCommands() { return mCommands; }
    utils::Slice<Command> const& getCommands() const { return mCommands; }

    size_t getCommandsHighWatermark() const noexcept {
        return mCommandsHighWatermark * sizeof(Command);
    }

private:
    friend class FRenderer;

    // on 64-bits systems, we process batches of 4 (64 bytes) cache-lines, or 8 (32 bytes) commands
    // on 32-bits systems, we process batches of 8 (32 bytes) cache-lines, or 8 (32 bytes) commands
    static constexpr size_t JOBS_PARALLEL_FOR_COMMANDS_COUNT = 16;
    static constexpr size_t JOBS_PARALLEL_FOR_COMMANDS_SIZE  =
            sizeof(Command) * JOBS_PARALLEL_FOR_COMMANDS_COUNT;

    static_assert(JOBS_PARALLEL_FOR_COMMANDS_SIZE % utils::CACHELINE_SIZE == 0,
            "Size of Commands jobs must be multiple of a cache-line size");

    static inline void generateCommands(uint32_t commandTypeFlags, Command* commands,
            FScene::RenderableSoa const& soa, utils::Range<uint32_t> range, RenderFlags renderFlags,
            FScene::VisibleMaskType visibilityMask, math::float3 cameraPosition, math::float3 cameraForward) noexcept;

    template<uint32_t commandTypeFlags>
    static inline void generateCommandsImpl(uint32_t, Command* commands,
            FScene::RenderableSoa const& soa, utils::Range<uint32_t> range,
            RenderFlags renderFlags, FScene::VisibleMaskType visibilityMask,
            math::float3 cameraPosition, math::float3 cameraForward) noexcept;

    static void setupColorCommand(Command& cmdDraw, bool hasDepthPass,
            FMaterialInstance const* mi, bool inverseFrontFaces) noexcept;

    void recordDriverCommands(FEngine::DriverApi& driver, const Command* first,
            const Command* last) const noexcept;

    static void updateSummedPrimitiveCounts(
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> vr) noexcept;

    using CustomCommandFn = std::function<void()>;
    using CustomCommandVector = std::vector<CustomCommandFn,
            utils::STLAllocator<CustomCommandFn, LinearAllocatorArena>>;

    // a reference to the Engine, mostly to get to things like JobSystem
    FEngine& mEngine;

    utils::GrowingSlice<Command> mCommands;

    // the SOA containing the renderables we're interested in
    FScene::RenderableSoa const* mRenderableSoa = nullptr;
    // and the range of visible renderables in the SOA above
    utils::Range<uint32_t> mVisibleRenderables{};
    // the UBO containing the data for the renderables
    backend::Handle<backend::HwUniformBuffer> mUboHandle;

    // info about the camera
    CameraInfo mCamera;
    // info about the scene features (e.g.: has shadows, lighting, etc...)
    RenderFlags mFlags{};
    FScene::VisibleMaskType mVisibilityMask = std::numeric_limits<FScene::VisibleMaskType>::max();
    // whether to override the polygon offset setting
    bool mPolygonOffsetOverride = false;
    // value of the override
    backend::PolygonOffset mPolygonOffset{};

    // a vector for our custom commands
    mutable CustomCommandVector mCustomCommands;

    // high watermark for debugging
    size_t mCommandsHighWatermark = 0;
};

} // namespace filament

#endif // TNT_UTILS_RENDERPASS_H
