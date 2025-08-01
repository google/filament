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

#include "SharedHandle.h"

#include "details/Camera.h"
#include "details/Scene.h"

#include "private/filament/Variant.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/Allocator.h>
#include <utils/BitmaskEnum.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Range.h>
#include <utils/Slice.h>
#include <utils/architecture.h>
#include <utils/debug.h>

#include <math/mathfwd.h>

#include <functional>
#include <limits>
#include <optional>
#include <type_traits>
#include <tuple>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace filament {

namespace backend {
class CommandBufferQueue;
}

class FMaterialInstance;
class FRenderPrimitive;
class RenderPassBuilder;
class ColorPassDescriptorSet;

class RenderPass {
public:
    /*
     *   Command key encoding
     *   --------------------
     *
     *   CC    = Channel
     *   PP    = Pass
     *   a     = alpha masking
     *   ppp   = priority
     *   t     = two-pass transparency ordering
     *   0     = reserved, must be zero
     *
     *
     * TODO: we need to add a "primitive id" in the low-bits of material-id, so that
     *       auto-instancing can work better
     *
     *   DEPTH command (b00)
     *   |  |  | 2| 2| 2|1| 3 | 2|  6   |   10     |               32               |
     *   +--+--+--+--+--+-+---+--+------+----------+--------------------------------+
     *   |CC|00|00|01|00|0|ppp|00|000000| Z-bucket |          material-id           |
     *   +--+--+--+--+--+-+---+--+------+----------+--------------------------------+
     *   | correctness        |      optimizations (truncation allowed)             |
     *
     *
     *   COLOR (b01) and REFRACT (b10) commands
     *   |  | 2| 2| 2| 2|1| 3 | 2|  6   |   10     |               32               |
     *   +--+--+--+--+--+-+---+--+------+----------+--------------------------------+
     *   |CC|00|01|01|00|a|ppp|00|000000| Z-bucket |          material-id           |
     *   |CC|00|10|01|00|a|ppp|00|000000| Z-bucket |          material-id           | refraction
     *   +--+--+--+--+--+-+---+--+------+----------+--------------------------------+
     *   | correctness        |      optimizations (truncation allowed)             |
     *
     *
     *   BLENDED command (b11)
     *   | 2| 2| 2| 2| 2|1| 3 | 2|              32                |         15    |1|
     *   +--+--+--+--+--+-+---+--+--------------------------------+---------------+-+
     *   |CC|00|11|01|00|0|ppp|00|         ~distanceBits          |   blendOrder  |t|
     *   +--+--+--+--+--+-+---+--+--------------------------------+---------------+-+
     *   | correctness                                                              |
     *
     *
     *   pre-CUSTOM command
     *   | 2| 2| 2| 2| 2|         22           |               32               |
     *   +--+--+--+--+--+----------------------+--------------------------------+
     *   |CC|00|PP|00|00|        order         |      custom command index      |
     *   +--+--+--+--+--+----------------------+--------------------------------+
     *   | correctness                                                          |
     *
     *
     *   post-CUSTOM command
     *   | 2| 2| 2| 2| 2|         22           |               32               |
     *   +--+--+--+--+--+----------------------+--------------------------------+
     *   |CC|00|PP|11|00|        order         |      custom command index      |
     *   +--+--+--+--+--+----------------------+--------------------------------+
     *   | correctness                                                          |
     *
     *
     *   SENTINEL command
     *   |                                   64                                  |
     *   +--------.--------.--------.--------.--------.--------.--------.--------+
     *   |11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111|
     *   +-----------------------------------------------------------------------+
     */
    using CommandKey = uint64_t;

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

    static constexpr uint64_t CUSTOM_MASK                   = 0x0300000000000000llu;
    static constexpr unsigned CUSTOM_SHIFT                  = 56;

    static constexpr uint64_t PASS_MASK                     = 0x0C00000000000000llu;
    static constexpr unsigned PASS_SHIFT                    = 58;

    static constexpr uint64_t CHANNEL_MASK                  = 0xC000000000000000llu;
    static constexpr unsigned CHANNEL_SHIFT                 = 62;

    static constexpr uint64_t CUSTOM_ORDER_MASK             = 0x003FFFFF00000000llu;
    static constexpr unsigned CUSTOM_ORDER_SHIFT            = 32;

    static constexpr uint64_t CUSTOM_INDEX_MASK             = 0x00000000FFFFFFFFllu;
    static constexpr unsigned CUSTOM_INDEX_SHIFT            = 0;

    // we assume Variant fits in 8-bits.
    static_assert(sizeof(Variant::type_t) == 1);

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

    enum class CommandTypeFlags : uint32_t {
        COLOR = 0x1,    // generate the color pass only
        DEPTH = 0x2,    // generate the depth pass only ( e.g. shadowmap)

        // shadow-casters are rendered in the depth buffer, regardless of blending (or alpha masking)
        DEPTH_CONTAINS_SHADOW_CASTERS = 0x4,
        // alpha-tested objects are not rendered in the depth buffer
        DEPTH_FILTER_ALPHA_MASKED_OBJECTS = 0x08,

        // alpha-blended objects are not rendered in the depth buffer
        FILTER_TRANSLUCENT_OBJECTS = 0x10,

        // generate commands for shadow map
        SHADOW = DEPTH | DEPTH_CONTAINS_SHADOW_CASTERS,
        // generate commands for SSAO
        SSAO = DEPTH | FILTER_TRANSLUCENT_OBJECTS,
        // generate commands for screen-space reflections
        SCREEN_SPACE_REFLECTIONS = COLOR | FILTER_TRANSLUCENT_OBJECTS
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
    static CommandKey makeMaterialSortingKey(uint32_t const materialId, uint32_t const instanceId) noexcept {
        CommandKey const key = ((materialId << MATERIAL_ID_SHIFT) & MATERIAL_ID_MASK) |
                         ((instanceId << MATERIAL_INSTANCE_ID_SHIFT) & MATERIAL_INSTANCE_ID_MASK);
        return (key << MATERIAL_SHIFT) & MATERIAL_MASK;
    }

    template<typename T>
    static CommandKey makeField(T value, uint64_t const mask, unsigned const shift) noexcept {
        assert_invariant(!((uint64_t(value) << shift) & ~mask));
        return uint64_t(value) << shift;
    }

    template<typename T>
    static CommandKey select(T boolish) noexcept {
        return boolish ? std::numeric_limits<uint64_t>::max() : uint64_t(0);
    }

    template<typename T>
    static CommandKey select(T boolish, uint64_t value) noexcept {
        return boolish ? value : uint64_t(0);
    }

    struct PrimitiveInfo { // 56 bytes
        union {
            FMaterialInstance const* mi;
            uint64_t padding; // make this field 64 bits on all platforms
        };
        backend::RenderPrimitiveHandle rph;                 // 4 bytes
        backend::VertexBufferInfoHandle vbih;               // 4 bytes
        backend::DescriptorSetHandle dsh;                   // 4 bytes
        uint32_t indexOffset;                               // 4 bytes
        uint32_t indexCount;                                // 4 bytes
        uint32_t index = 0;                                 // 4 bytes
        uint32_t skinningOffset = 0;                        // 4 bytes
        uint32_t morphingOffset = 0;                        // 4 bytes

        backend::RasterState rasterState;                   // 4 bytes

        uint16_t instanceCount;                             // 2 bytes [MSb: user]
        Variant materialVariant;                            // 1 byte
        backend::PrimitiveType type : 3;                    // 1 byte       3 bits
        bool hasSkinning : 1;                               //              1 bit
        bool hasMorphing : 1;                               //              1 bit
        bool hasHybridInstancing : 1;                       //              1 bit

        uint32_t rfu[2];                                    // 16 bytes
    };
    static_assert(sizeof(PrimitiveInfo) == 56);

    struct alignas(8) Command {     // 64 bytes
        CommandKey key = 0;         //  8 bytes
        PrimitiveInfo info;    // 56 bytes
        bool operator < (Command const& rhs) const noexcept { return key < rhs.key; }
        // placement new declared as "throw" to avoid the compiler's null-check
        void* operator new (size_t, void* ptr) {
            assert_invariant(ptr);
            return ptr;
        }
    };
    static_assert(sizeof(Command) == 64);
    static_assert(std::is_trivially_destructible_v<Command>,
            "Command isn't trivially destructible");

    using RenderFlags = uint8_t;
    static constexpr RenderFlags HAS_SHADOWING             = 0x01;
    static constexpr RenderFlags HAS_INVERSE_FRONT_FACES   = 0x02;
    static constexpr RenderFlags IS_INSTANCED_STEREOSCOPIC = 0x04;
    static constexpr RenderFlags HAS_DEPTH_CLAMP           = 0x08;

    // Arena used for commands
    using Arena = utils::Arena<
            utils::LinearAllocatorWithFallback,
            utils::LockingPolicy::NoLock,
            utils::TrackingPolicy::HighWatermark,
            utils::AreaPolicy::StaticArea>;

    // RenderPass can only be moved
    RenderPass(RenderPass&& rhs) = default;
    RenderPass& operator=(RenderPass&& rhs) = delete;  // could be supported if needed

    // RenderPass can't be copied
    RenderPass(RenderPass const& rhs) = delete;
    RenderPass& operator=(RenderPass const& rhs) = delete;

    // allocated commands ARE NOT freed, they're owned by the Arena
    ~RenderPass() noexcept;

    // Specifies the viewport for the scissor rectangle, that is, the final scissor rect is
    // offset by the viewport's left-top and clipped to the viewport's width/height.
    void setScissorViewport(backend::Viewport const viewport) noexcept {
        mScissorViewport = viewport;
    }

    Command const* begin() const noexcept { return mCommandBegin; }
    Command const* end() const noexcept { return mCommandEnd; }
    bool empty() const noexcept { return begin() == end(); }

    class BufferObjectHandleDeleter {
        std::reference_wrapper<backend::DriverApi> driver;
    public:
        explicit BufferObjectHandleDeleter(backend::DriverApi& driver) noexcept : driver(driver) { }
        void operator()(backend::BufferObjectHandle handle) noexcept;
    };

    class DescriptorSetHandleDeleter {
        std::reference_wrapper<backend::DriverApi> driver;
    public:
        explicit DescriptorSetHandleDeleter(backend::DriverApi& driver) noexcept : driver(driver) { }
        void operator()(backend::DescriptorSetHandle handle) noexcept;
    };

    using BufferObjectSharedHandle = SharedHandle<
            backend::HwBufferObject, BufferObjectHandleDeleter>;

    using DescriptorSetSharedHandle = SharedHandle<
            backend::HwDescriptorSet, DescriptorSetHandleDeleter>;

    /*
     * Executor holds the range of commands to execute for a given pass
     */
    class Executor {
        using CustomCommandFn = std::function<void()>;
        friend class RenderPass;
        friend class RenderPassBuilder;

        // these fields are constant after creation
        utils::Slice<Command> mCommands;
        utils::Slice<CustomCommandFn> mCustomCommands;
        BufferObjectSharedHandle mInstancedUboHandle;
        DescriptorSetSharedHandle mInstancedDescriptorSetHandle;
        ColorPassDescriptorSet const* mColorPassDescriptorSet = nullptr;
        // this stores either the scissor-viewport or the scissor override
        backend::Viewport mScissor{ 0, 0, INT32_MAX, INT32_MAX };

        // value of the polygon offset override
        backend::PolygonOffset mPolygonOffset{};
        // whether to override the polygon offset from the MaterialInstance
        bool mPolygonOffsetOverride : 1;
        // whether to override the scissor rectangle from the MaterialInstance
        bool mScissorOverride : 1;
        // whether the scissor-viewport is set
        bool mHasScissorViewport : 1;

        Executor(RenderPass const& pass, Command const* b, Command const* e) noexcept;

        void execute(FEngine const& engine, backend::DriverApi& driver,
                Command const* first, Command const* last) const noexcept;

        static backend::Viewport applyScissorViewport(
                backend::Viewport const& scissorViewport,
                backend::Viewport const& scissor) noexcept;

    public:
        // fixme: needed in ShadowMapManager
        Executor() noexcept;

        // can't be copied
        Executor(Executor const& rhs) noexcept = delete;
        Executor& operator=(Executor const& rhs) noexcept = delete;

        // can be moved
        Executor(Executor&& rhs) noexcept;
        Executor& operator=(Executor&& rhs) noexcept;

        ~Executor() noexcept;

        // if non-null, overrides the material's polygon offset
        void overridePolygonOffset(backend::PolygonOffset const* polygonOffset) noexcept;

        void overrideScissor(backend::Viewport const& scissor) noexcept;

        void execute(FEngine const& engine, backend::DriverApi& driver) const noexcept;
    };

    // returns a new executor for this pass
    Executor getExecutor() const {
        return getExecutor(mCommandBegin, mCommandEnd);
    }

    Executor getExecutor(Command const* b, Command const* e) const {
        return { *this, b, e };
    }

private:
    friend class FRenderer;
    friend class RenderPassBuilder;
    RenderPass(FEngine const& engine, backend::DriverApi& driver,
            RenderPassBuilder const& builder) noexcept;

    // This is the main function of this class, this appends commands to the pass using
    // the current camera, geometry and flags set. This can be called multiple times if needed.
    void appendCommands(FEngine const& engine,
            utils::Slice<Command> commands,
            utils::Range<uint32_t> visibleRenderables,
            CommandTypeFlags commandTypeFlags,
            RenderFlags renderFlags,
            FScene::VisibleMaskType visibilityMask,
            Variant variant,
            math::float3 cameraPosition,
            math::float3 cameraForwardVector) const noexcept;

    // Appends a custom command.
    void appendCustomCommand(Command* commands,
            uint8_t channel, Pass pass, CustomCommand custom, uint32_t order,
            Executor::CustomCommandFn command);

    static Command* resize(Arena& arena, Command* last) noexcept;

    // sorts commands then trims sentinels
    static Command* sortCommands(
            Command* begin, Command* end) noexcept;

    // instanceify commands then trims sentinels
    Command* instanceify(backend::DriverApi& driver,
            backend::DescriptorSetLayoutHandle perRenderableDescriptorSetLayoutHandle,
            Command* begin, Command* end,
            int32_t eyeCount) const noexcept;

    // We choose the command count per job to minimize JobSystem overhead.
    static constexpr size_t JOBS_PARALLEL_FOR_COMMANDS_COUNT = 128;
    static constexpr size_t JOBS_PARALLEL_FOR_COMMANDS_SIZE  =
            sizeof(Command) * JOBS_PARALLEL_FOR_COMMANDS_COUNT;

    static_assert(JOBS_PARALLEL_FOR_COMMANDS_SIZE % utils::CACHELINE_SIZE == 0,
            "Size of Commands jobs must be multiple of a cache-line size");

    static inline void generateCommands(CommandTypeFlags commandTypeFlags, Command* commands,
            FScene::RenderableSoa const& soa, utils::Range<uint32_t> range,
            Variant variant, RenderFlags renderFlags,
            FScene::VisibleMaskType visibilityMask,
            math::float3 cameraPosition, math::float3 cameraForward,
            uint8_t instancedStereoEyeCount) noexcept;

    template<CommandTypeFlags commandTypeFlags>
    static Command* generateCommandsImpl(CommandTypeFlags extraFlags,
            Command* curr, FScene::RenderableSoa const& soa, utils::Range<uint32_t> range,
            Variant variant, RenderFlags renderFlags, FScene::VisibleMaskType visibilityMask,
            math::float3 cameraPosition, math::float3 cameraForward,
            uint8_t instancedStereoEyeCount) noexcept;

    static void setupColorCommand(Command& cmdDraw, Variant variant,
            FMaterialInstance const* mi, bool inverseFrontFaces, bool hasDepthClamp) noexcept;

    static void updateSummedPrimitiveCounts(
            FScene::RenderableSoa& renderableData, utils::Range<uint32_t> vr) noexcept;

    FScene::RenderableSoa const& mRenderableSoa;
    ColorPassDescriptorSet const* const mColorPassDescriptorSet;
    backend::Viewport mScissorViewport{ 0, 0, INT32_MAX, INT32_MAX };
    Command const* /* const */ mCommandBegin = nullptr;   // Pointer to the first command
    Command const* /* const */ mCommandEnd = nullptr;     // Pointer to one past the last command
    mutable BufferObjectSharedHandle mInstancedUboHandle; // ubo for instanced primitives
    mutable DescriptorSetSharedHandle mInstancedDescriptorSetHandle; // a descriptor-set to hold the ubo
    // a vector for our custom commands
    using CustomCommandVector = utils::FixedCapacityVector<Executor::CustomCommandFn>;
    mutable CustomCommandVector mCustomCommands;
};

class RenderPassBuilder {
    friend class RenderPass;

    RenderPass::Arena& mArena;
    RenderPass::CommandTypeFlags mCommandTypeFlags{};
    FScene::RenderableSoa const* mRenderableSoa = nullptr;
    utils::Range<uint32_t> mVisibleRenderables{};
    math::float3 mCameraPosition{};
    math::float3 mCameraForwardVector{};
    RenderPass::RenderFlags mFlags{};
    Variant mVariant{};
    ColorPassDescriptorSet const* mColorPassDescriptorSet = nullptr;
    FScene::VisibleMaskType mVisibilityMask = std::numeric_limits<FScene::VisibleMaskType>::max();

    using CustomCommandRecord = std::tuple<
            uint8_t,
            RenderPass::Pass,
            RenderPass::CustomCommand,
            uint32_t,
            RenderPass::Executor::CustomCommandFn>;

    using CustomCommandContainer = std::vector<CustomCommandRecord>;

    // we make this optional because it's not used often, and we don't want to have
    // to construct it by default. For the same reason we use a std::vector<>
    std::optional<CustomCommandContainer> mCustomCommands;

public:
    explicit RenderPassBuilder(RenderPass::Arena& arena) : mArena(arena) { }

    RenderPassBuilder& commandTypeFlags(RenderPass::CommandTypeFlags const commandTypeFlags) noexcept {
        mCommandTypeFlags = commandTypeFlags;
        return *this;
    }

    // specifies the geometry to generate commands for
    RenderPassBuilder& geometry(
            FScene::RenderableSoa const& soa, utils::Range<uint32_t> const vr) noexcept {
        mRenderableSoa = &soa;
        mVisibleRenderables = vr;
        return *this;
    }

    // Specifies camera information (e.g. used for sorting commands)
    RenderPassBuilder& camera(math::float3 position, math::float3 forward) noexcept {
        mCameraPosition = position;
        mCameraForwardVector = forward;
        return *this;
    }

    //  flags controlling how commands are generated
    RenderPassBuilder& renderFlags(RenderPass::RenderFlags const flags) noexcept {
        mFlags = flags;
        return *this;
    }

    // like above but allows to set specific flags
    RenderPassBuilder& renderFlags(
            RenderPass::RenderFlags const mask, RenderPass::RenderFlags value) noexcept {
        value &= mask;
        mFlags &= ~mask;
        mFlags |= value;
        return *this;
    }

    // variant to use
    RenderPassBuilder& variant(Variant const variant) noexcept {
        mVariant = variant;
        return *this;
    }

    // variant to use
    RenderPassBuilder& colorPassDescriptorSet(ColorPassDescriptorSet const* colorPassDescriptorSet) noexcept {
        mColorPassDescriptorSet = colorPassDescriptorSet;
        return *this;
    }

    // Sets the visibility mask, which is AND-ed against each Renderable's VISIBLE_MASK to
    // determine if the renderable is visible for this pass.
    // Defaults to all 1's, which means all renderables in this render pass will be rendered.
    RenderPassBuilder& visibilityMask(FScene::VisibleMaskType const mask) noexcept {
        mVisibilityMask = mask;
        return *this;
    }

    RenderPassBuilder& customCommand(
            uint8_t channel,
            RenderPass::Pass pass,
            RenderPass::CustomCommand custom,
            uint32_t order,
            const RenderPass::Executor::CustomCommandFn& command);

    RenderPass build(FEngine const& engine, backend::DriverApi& driver) const;
};


} // namespace filament

template<> struct utils::EnableBitMaskOperators<filament::RenderPass::CommandTypeFlags>
        : public std::true_type {};

#endif // TNT_FILAMENT_RENDERPASS_H
