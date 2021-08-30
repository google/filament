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

#ifndef TNT_FILAMENT_RENDERPASS_H
#define TNT_FILAMENT_RENDERPASS_H

#include "Allocators.h"

#include "details/Camera.h"
#include "details/Scene.h"

#include "private/backend/DriverApiForward.h"

#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Allocator.h>
#include <utils/Range.h>
#include <utils/architecture.h>
#include <utils/compiler.h>
#include <utils/debug.h>

#include <functional>
#include <limits>
#include <vector>

namespace filament {

class FMaterialInstance;

class RenderPass {
public:
    /*
     *   Command key encoding
     *   --------------------
     *
     *   a     = alpha masking
     *   ppp   = priority
     *   t     = two-pass transparency ordering
     *   0     = reserved, must be zero
     *
     *   DEPTH command
     *   |   6  | 2| 2|1| 3 | 2|       16       |               32               |
     *   +------+--+--+-+---+--+----------------+--------------------------------+
     *   |000000|01|00|0|ppp|00|0000000000000000|          distanceBits          |
     *   +------+--+--+-+---+-------------------+--------------------------------+
     *   | correctness      |     optimizations (truncation allowed)             |
     *
     *
     *   COLOR command
     *   |   6  | 2| 2|1| 3 | 2|  6   |   10     |               32               |
     *   +------+--+--+-+---+--+------+----------+--------------------------------+
     *   |000001|01|00|a|ppp|00|000000| Z-bucket |          material-id           |
     *   |000010|01|00|a|ppp|00|000000| Z-bucket |          material-id           | refraction
     *   +------+--+--+-+---+--+------+----------+--------------------------------+
     *   | correctness      |      optimizations (truncation allowed)             |
     *
     *
     *   BLENDED command
     *   |   6  | 2| 2|1| 3 | 2|              32                |         15    |1|
     *   +------+--+--+-+---+--+--------------------------------+---------------+-+
     *   |000011|01|00|0|ppp|00|         ~distanceBits          |   blendOrder  |t|
     *   +------+--+--+-+---+--+--------------------------------+---------------+-+
     *   | correctness                                                            |
     *
     *
     *   pre-CUSTOM command
     *   |   6  | 2| 2|         22           |               32               |
     *   +------+--+--+----------------------+--------------------------------+
     *   | pass |00|00|        order         |      custom command index      |
     *   +------+--+--+----------------------+--------------------------------+
     *   | correctness                                                        |
     *
     *
     *   post-CUSTOM command
     *   |   6  | 2| 2|         22           |               32               |
     *   +------+--+--+----------------------+--------------------------------+
     *   | pass |11|00|        order         |      custom command index      |
     *   +------+--+--+----------------------+--------------------------------+
     *   | correctness                                                        |
     *
     *
     *   SENTINEL command
     *   |                                   64                                  |
     *   +--------.--------.--------.--------.--------.--------.--------.--------+
     *   |11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111|
     *   +-----------------------------------------------------------------------+
     */
    using CommandKey = uint64_t;

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


    /*
     * The sorting material key is 32 bits and encoded as:
     *
     * |     12     |   8    |     12     |
     * +------------+--------+------------+
     * |  material  |variant |  instance  |
     * +------------+--------+------------+
     *
     * The variant is inserted while building the commands, because we don't know it before that
     */
    static CommandKey makeMaterialSortingKey(uint32_t materialId, uint32_t instanceId) noexcept {
        CommandKey key = ((materialId << MATERIAL_ID_SHIFT) & MATERIAL_ID_MASK) |
                         ((instanceId << MATERIAL_INSTANCE_ID_SHIFT) & MATERIAL_INSTANCE_ID_MASK);
        return (key << MATERIAL_SHIFT) & MATERIAL_MASK;
    }

    template<typename T>
    static CommandKey makeField(T value, uint64_t mask, unsigned shift) noexcept {
        assert_invariant(!((uint64_t(value) << shift) & ~mask));
        return uint64_t(value) << shift;
    }

    template<typename T>
    static CommandKey select(T boolish) noexcept {
        return boolish ? std::numeric_limits<uint64_t>::max() : uint64_t(0);
    }

    struct PrimitiveInfo { // 24 bytes
        FMaterialInstance const* mi = nullptr;                          // 8 bytes (4)
        backend::Handle<backend::HwRenderPrimitive> primitiveHandle;    // 4 bytes
        backend::RasterState rasterState;                               // 4 bytes
        uint16_t index = 0;                                             // 2 bytes
        Variant materialVariant;                                        // 1 byte
        uint8_t reserved[13 - sizeof(void*)] = {};                      // 5 byte (9)
    };
    static_assert(sizeof(PrimitiveInfo) == 24);

    struct alignas(8) Command {     // 32 bytes
        CommandKey key = 0;         //  8 bytes
        PrimitiveInfo primitive;    // 24 bytes
        bool operator < (Command const& rhs) const noexcept { return key < rhs.key; }
        // placement new declared as "throw" to avoid the compiler's null-check
        inline void* operator new (std::size_t, void* ptr) {
            assert_invariant(ptr);
            return ptr;
        }
    };
    static_assert(sizeof(Command) == 32);
    static_assert(std::is_trivially_destructible_v<Command>,
            "Command isn't trivially destructible");

    using RenderFlags = uint8_t;
    static constexpr RenderFlags HAS_SHADOWING           = 0x01;
    static constexpr RenderFlags HAS_DIRECTIONAL_LIGHT   = 0x02;
    static constexpr RenderFlags HAS_DYNAMIC_LIGHTING    = 0x04;
    static constexpr RenderFlags HAS_INVERSE_FRONT_FACES = 0x08;
    static constexpr RenderFlags HAS_FOG                 = 0x10;
    static constexpr RenderFlags HAS_VSM                 = 0x20;

    // Arena used for commands
    using Arena = utils::Arena<
            utils::LinearAllocator,
            utils::LockingPolicy::NoLock,
            utils::TrackingPolicy::HighWatermark,
            utils::AreaPolicy::StaticArea>;

    /*
     * Create a RenderPass.
     * The Arena is used to allocate commands which are then owned by the Arena.
     */
    RenderPass(FEngine& engine, Arena& arena) noexcept;

    // Copy the RenderPass as is. This can be used to create a RenderPass from a "template"
    // by copying from an "empty" RenderPass.
    RenderPass(RenderPass const& rhs);

    // allocated commands ARE NOT freed, they're owned by the Arena
    ~RenderPass() noexcept;

    // if non-null, overrides the material's polygon offset
    void overridePolygonOffset(backend::PolygonOffset* polygonOffset) noexcept;

    // specifies the geometry to generate commands for
    void setGeometry(FScene::RenderableSoa const& soa, utils::Range<uint32_t> vr,
            backend::Handle<backend::HwBufferObject> uboHandle) noexcept;

    // specifies camera information (e.g. used for sorting commands)
    void setCamera(const CameraInfo& camera) noexcept { mCamera = camera; }

    //  flags controling how commands are generated
    void setRenderFlags(RenderFlags flags) noexcept { mFlags = flags; }

    // Sets the visibility mask, which is AND-ed against each Renderable's VISIBLE_MASK to determine
    // if the renderable is visible for this pass.
    // Defaults to all 1's, which means all renderables in this render pass will be rendered.
    void setVisibilityMask(FScene::VisibleMaskType mask) noexcept { mVisibilityMask = mask; }

    Command const* begin() const noexcept { return mCommandBegin; }
    Command const* end() const noexcept { return mCommandEnd; }
    bool empty() const noexcept { return begin() == end(); }

    // This is the main function of this class, this appends commands to the pass using
    // the current camera, geometry and flags set. This can be called multiple times if needed.
    void appendCommands(CommandTypeFlags commandTypeFlags) noexcept;

    // Appends a custom command.
    void appendCustomCommand(Pass pass, CustomCommand custom, uint32_t order,
            std::function<void()> command);

    // sorts commands, then trims sentinels
    void sortCommands() noexcept;

    // Helper to execute all the commands generated by this RenderPass
    void execute(const char* name,
            backend::Handle<backend::HwRenderTarget> renderTarget,
            backend::RenderPassParams params) const noexcept {
        getExecutor().execute(name, renderTarget, params);
    }

    /*
     * Executor holds the range of commands to execute for a given pass
     */
    class Executor {
        using CustomCommandFn = std::function<void()>;
        using CustomCommandVector = std::vector<CustomCommandFn,
                utils::STLAllocator<CustomCommandFn, LinearAllocatorArena>>;

        friend class RenderPass;
        FEngine& mEngine;
        Command const* mBegin;
        Command const* mEnd;
        FScene::RenderableSoa const& mRenderableSoa;
        const CustomCommandVector mCustomCommands;
        const backend::Handle<backend::HwBufferObject> mUboHandle;
        const backend::PolygonOffset mPolygonOffset;
        const bool mPolygonOffsetOverride;

        Executor(RenderPass const* pass, Command const* b, Command const* e) noexcept
                : mEngine(pass->mEngine), mBegin(b), mEnd(e), mRenderableSoa(*pass->mRenderableSoa),
                  mCustomCommands(pass->mCustomCommands), mUboHandle(pass->mUboHandle),
                  mPolygonOffset(pass->mPolygonOffset),
                  mPolygonOffsetOverride(pass->mPolygonOffsetOverride) {
            assert_invariant(b >= pass->begin());
            assert_invariant(e <= pass->end());
        }

        void recordDriverCommands(backend::DriverApi& driver,
                const Command* first, const Command* last,
                FScene::RenderableSoa const& soa) const noexcept;

    public:
        void execute(const char* name,
                backend::Handle<backend::HwRenderTarget> renderTarget,
                backend::RenderPassParams params) const noexcept;
    };

    // returns a new executor for this pass
    Executor getExecutor() const {
        return { this, mCommandBegin, mCommandEnd };
    }

    // returns a new executor for this pass with a custom range
    Executor getExecutor(Command const* b, Command const* e) const {
        return { this, b, e };
    }

private:
    friend class FRenderer;

    Command* append(size_t count) noexcept;
    void resize(size_t count) noexcept;

    // on 64-bits systems, we process batches of 256 (64 bytes) cache-lines, or 512 (32 bytes) commands
    // on 32-bits systems, we process batches of 512 (32 bytes) cache-lines, or 512 (32 bytes) commands
    static constexpr size_t JOBS_PARALLEL_FOR_COMMANDS_COUNT = 512;
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

    static void setupColorCommand(Command& cmdDraw,
            FMaterialInstance const* mi, bool inverseFrontFaces) noexcept;

    static void updateSummedPrimitiveCounts(
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> vr) noexcept;

    using CustomCommandFn = std::function<void()>;
    using CustomCommandVector = std::vector<CustomCommandFn,
            utils::STLAllocator<CustomCommandFn, LinearAllocatorArena>>;

    // a reference to the Engine, mostly to get to things like JobSystem
    FEngine& mEngine;

    // Arena where all Commands are allocated. The Arena owns the commands.
    Arena& mCommandArena;

    // Pointer to the first command
    Command* mCommandBegin = nullptr;

    // Pointer to one past the last command
    Command* mCommandEnd = nullptr;

    // the SOA containing the renderables we're interested in
    FScene::RenderableSoa const* mRenderableSoa = nullptr;

    // The range of visible renderables in the SOA above
    utils::Range<uint32_t> mVisibleRenderables{};

    // the UBO containing the data for the renderables
    backend::Handle<backend::HwBufferObject> mUboHandle;

    // info about the camera
    CameraInfo mCamera;

    // info about the scene features (e.g.: has shadows, lighting, etc...)
    RenderFlags mFlags{};

    // Additional visibility mask
    FScene::VisibleMaskType mVisibilityMask = std::numeric_limits<FScene::VisibleMaskType>::max();

    // whether to override the polygon offset setting
    bool mPolygonOffsetOverride = false;

    // value of the override
    backend::PolygonOffset mPolygonOffset{};

    // a vector for our custom commands
    mutable CustomCommandVector mCustomCommands;
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERPASS_H
