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

#include "RenderPass.h"

#include "RenderPrimitive.h"
#include "ShadowMap.h"

#include "details/Camera.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/View.h"

#include "components/RenderableManager.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>
#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>

#include "private/backend/CircularBuffer.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/JobSystem.h>
#include <utils/Panic.h>
#include <utils/Slice.h>
#include <utils/Systrace.h>
#include <utils/Range.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

using namespace utils;
using namespace filament::math;

namespace filament {

using namespace backend;

RenderPassBuilder& RenderPassBuilder::customCommand(
        FEngine& engine,
        uint8_t channel,
        RenderPass::Pass pass,
        RenderPass::CustomCommand custom,
        uint32_t order,
        RenderPass::Executor::CustomCommandFn const& command) {
    if (!mCustomCommands.has_value()) {
        // construct the vector the first time
        mCustomCommands.emplace(engine.getPerRenderPassArena());
    }
    mCustomCommands->emplace_back(channel, pass, custom, order, command);
    return *this;
}

RenderPass RenderPassBuilder::build(FEngine& engine) {
    ASSERT_POSTCONDITION(mRenderableSoa, "RenderPassBuilder::geometry() hasn't been called");
    assert_invariant(mScissorViewport.width  <= std::numeric_limits<int32_t>::max());
    assert_invariant(mScissorViewport.height <= std::numeric_limits<int32_t>::max());
    return RenderPass{ engine, *this };
}

// ------------------------------------------------------------------------------------------------

RenderPass::RenderPass(FEngine& engine, RenderPassBuilder const& builder) noexcept
        : mRenderableSoa(*builder.mRenderableSoa),
          mVisibleRenderables(builder.mVisibleRenderables),
          mUboHandle(builder.mUboHandle),
          mCameraPosition(builder.mCameraPosition),
          mCameraForwardVector(builder.mCameraForwardVector),
          mFlags(builder.mFlags),
          mVariant(builder.mVariant),
          mVisibilityMask(builder.mVisibilityMask),
          mScissorViewport(builder.mScissorViewport),
          mCustomCommands(engine.getPerRenderPassArena()) {

    if (mVisibleRenderables.empty()) {
        return;
    }

    // compute the number of commands we need
    updateSummedPrimitiveCounts(
            const_cast<FScene::RenderableSoa&>(mRenderableSoa), mVisibleRenderables);

    uint32_t commandCount =
            FScene::getPrimitiveCount(mRenderableSoa, mVisibleRenderables.last);
    const bool colorPass  = bool(builder.mCommandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(builder.mCommandTypeFlags & CommandTypeFlags::DEPTH);
    commandCount *= uint32_t(colorPass * 2 + depthPass);
    commandCount += 1; // for the sentinel

    uint32_t const customCommandCount =
            builder.mCustomCommands.has_value() ? builder.mCustomCommands->size() : 0;

    Command* const curr = builder.mArena.alloc<Command>(commandCount + customCommandCount);
    assert_invariant(curr);

    if (UTILS_UNLIKELY(builder.mArena.getAllocator().isHeapAllocation(curr))) {
        static bool sLogOnce = true;
        if (UTILS_UNLIKELY(sLogOnce)) {
            sLogOnce = false;
            PANIC_LOG("RenderPass arena is full, using slower system heap. Please increase "
                      "the appropriate constant (e.g. FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB).");
        }
    }

    mCommandBegin = curr;
    mCommandEnd = curr + commandCount + customCommandCount;

    appendCommands(engine, { curr, commandCount }, builder.mCommandTypeFlags);

    if (builder.mCustomCommands.has_value()) {
        Command* p = curr + commandCount;
        for (auto [channel, passId, command, order, fn]: builder.mCustomCommands.value()) {
            appendCustomCommand(p++, channel, passId, command, order, fn);
        }
    }

    // sort commands once we're done adding commands
    sortCommands(builder.mArena);

    if (engine.isAutomaticInstancingEnabled()) {
        instanceify(engine, builder.mArena);
    }
}

// this destructor is actually heavy because it inlines ~vector<>
RenderPass::~RenderPass() noexcept = default;

void RenderPass::resize(Arena& arena, size_t count) noexcept {
    if (mCommandBegin) {
        mCommandEnd = mCommandBegin + count;
        arena.rewind(mCommandEnd);
    }
}

void RenderPass::appendCommands(FEngine& engine,
        Slice<Command> commands, CommandTypeFlags const commandTypeFlags) noexcept {
    SYSTRACE_CALL();
    SYSTRACE_CONTEXT();

    utils::Range<uint32_t> const vr = mVisibleRenderables;
    // trace the number of visible renderables
    SYSTRACE_VALUE32("visibleRenderables", vr.size());
    if (UTILS_UNLIKELY(vr.empty())) {
        // no renderables, we still need the sentinel and the command buffer size should be
        // exactly 1.
        assert_invariant(commands.size() == 1);
        Command* curr = commands.data();
        curr->key = uint64_t(Pass::SENTINEL);
        return;
    }

    JobSystem& js = engine.getJobSystem();
    const RenderFlags renderFlags = mFlags;
    const Variant variant = mVariant;
    const FScene::VisibleMaskType visibilityMask = mVisibilityMask;

    // up-to-date summed primitive counts needed for generateCommands()
    FScene::RenderableSoa const& soa = mRenderableSoa;

    Command* curr = commands.data();
    size_t const commandCount = commands.size();

    auto stereoscopicEyeCount =
            renderFlags & IS_STEREOSCOPIC ? engine.getConfig().stereoscopicEyeCount : 1;

    const float3 cameraPosition(mCameraPosition);
    const float3 cameraForwardVector(mCameraForwardVector);
    auto work = [commandTypeFlags, curr, &soa, variant, renderFlags, visibilityMask, cameraPosition,
                 cameraForwardVector, stereoscopicEyeCount]
            (uint32_t startIndex, uint32_t indexCount) {
        RenderPass::generateCommands(commandTypeFlags, curr,
                soa, { startIndex, startIndex + indexCount }, variant, renderFlags, visibilityMask,
                cameraPosition, cameraForwardVector, stereoscopicEyeCount);
    };

    if (vr.size() <= JOBS_PARALLEL_FOR_COMMANDS_COUNT) {
        work(vr.first, vr.size());
    } else {
        auto* jobCommandsParallel = jobs::parallel_for(js, nullptr, vr.first, (uint32_t)vr.size(),
                std::cref(work), jobs::CountSplitter<JOBS_PARALLEL_FOR_COMMANDS_COUNT, 5>());
        js.runAndWait(jobCommandsParallel);
    }

    // always add an "eof" command
    // "eof" command. these commands are guaranteed to be sorted last in the
    // command buffer.
    curr[commandCount - 1].key = uint64_t(Pass::SENTINEL);

    // Go over all the commands and call prepareProgram().
    // This must be done from the main thread.
    for (Command const* first = curr, *last = curr + commandCount ; first != last ; ++first) {
        if (UTILS_LIKELY((first->key & CUSTOM_MASK) == uint64_t(CustomCommand::PASS))) {
            auto ma = first->primitive.mi->getMaterial();
            ma->prepareProgram(first->primitive.materialVariant);
        }
    }
}

void RenderPass::appendCustomCommand(Command* commands,
        uint8_t channel, Pass pass, CustomCommand custom, uint32_t order,
        Executor::CustomCommandFn command) {

    assert_invariant((uint64_t(order) << CUSTOM_ORDER_SHIFT) <=  CUSTOM_ORDER_MASK);

    channel = std::min(channel, uint8_t(0x3));

    uint32_t const index = mCustomCommands.size();
    mCustomCommands.push_back(std::move(command));

    uint64_t cmd = uint64_t(pass);
    cmd |= uint64_t(channel) << CHANNEL_SHIFT;
    cmd |= uint64_t(custom);
    cmd |= uint64_t(order) << CUSTOM_ORDER_SHIFT;
    cmd |= uint64_t(index);

    commands->key = cmd;
}

void RenderPass::sortCommands(Arena& arena) noexcept {
    SYSTRACE_NAME("sort and trim commands");

    std::sort(mCommandBegin, mCommandEnd);

    // find the last command
    Command const* const last = std::partition_point(mCommandBegin, mCommandEnd,
            [](Command const& c) {
                return c.key != uint64_t(Pass::SENTINEL);
            });

    resize(arena, uint32_t(last - mCommandBegin));
}

void RenderPass::execute(RenderPass const& pass,
        FEngine& engine, const char* name,
        backend::Handle<backend::HwRenderTarget> renderTarget,
        backend::RenderPassParams params) noexcept {
    DriverApi& driver = engine.getDriverApi();
    driver.beginRenderPass(renderTarget, params);
    pass.getExecutor().execute(engine, name);
    driver.endRenderPass();
}

void RenderPass::instanceify(FEngine& engine, Arena& arena) noexcept {
    SYSTRACE_NAME("instanceify");

    // instanceify works by scanning the **sorted** command stream, looking for repeat draw
    // commands. When one is found, it is replaced by an instanced command.
    // A "repeat" draw is one that ends-up using the same draw parameters and state.
    // Currently, this relies somewhat on luck that "repeat draws" are found consecutively,
    // we could improve this by including some or all of these "repeat" parameters in the
    // sorting key (e.g. raster state, primitive handle, etc...), the key could even use a small
    // hash of those parameters.

    UTILS_UNUSED uint32_t drawCallsSavedCount = 0;

    Command* curr = mCommandBegin;
    Command* const last = mCommandEnd;

    Command* firstSentinel = nullptr;
    PerRenderableData const* uboData = nullptr;
    PerRenderableData* stagingBuffer = nullptr;
    uint32_t stagingBufferSize = 0;
    uint32_t instancedPrimitiveOffset = 0;

    // TODO: for the case of instancing we could actually use 128 instead of 64 instances
    constexpr size_t maxInstanceCount = CONFIG_MAX_INSTANCES;

    while (curr != last) {

        // we can't have nice things! No more than maxInstanceCount due to UBO size limits
        Command const* const e = std::find_if_not(curr, std::min(last, curr + maxInstanceCount),
                [lhs = *curr](Command const& rhs) {
            // primitives must be identical to be instanced. Currently, instancing doesn't support
            // skinning/morphing.
            return  lhs.primitive.mi                == rhs.primitive.mi                 &&
                    lhs.primitive.primitiveHandle   == rhs.primitive.primitiveHandle    &&
                    lhs.primitive.rasterState       == rhs.primitive.rasterState        &&
                    lhs.primitive.skinningHandle    == rhs.primitive.skinningHandle     &&
                    lhs.primitive.skinningOffset    == rhs.primitive.skinningOffset     &&
                    lhs.primitive.morphWeightBuffer == rhs.primitive.morphWeightBuffer  &&
                    lhs.primitive.morphTargetBuffer == rhs.primitive.morphTargetBuffer  &&
                    lhs.primitive.skinningTexture   == rhs.primitive.skinningTexture    ;
        });

        uint32_t const instanceCount = e - curr;
        assert_invariant(instanceCount > 0);
        assert_invariant(instanceCount <= CONFIG_MAX_INSTANCES);

        if (UTILS_UNLIKELY(instanceCount > 1)) {
            drawCallsSavedCount += instanceCount - 1;

            // allocate our staging buffer only if needed
            if (UTILS_UNLIKELY(!stagingBuffer)) {
                // TODO: use stream inline buffer for small sizes
                // TODO: use a pool for larger heap buffers
                // buffer large enough for all instances data
                stagingBufferSize = sizeof(PerRenderableData) * (last - curr);
                stagingBuffer = (PerRenderableData*)::malloc(stagingBufferSize);
                uboData = mRenderableSoa.data<FScene::UBO>();
                assert_invariant(uboData);
            }

            // copy the ubo data to a staging buffer
            assert_invariant(instancedPrimitiveOffset + instanceCount
                             <= stagingBufferSize / sizeof(PerRenderableData));
            for (uint32_t i = 0; i < instanceCount; i++) {
                stagingBuffer[instancedPrimitiveOffset + i] = uboData[curr[i].primitive.index];
            }

            // make the first command instanced
            curr[0].primitive.instanceCount = instanceCount;
            curr[0].primitive.index = instancedPrimitiveOffset;
            instancedPrimitiveOffset += instanceCount;

            // cancel commands that are now instances
            firstSentinel = !firstSentinel ? curr : firstSentinel;
            for (uint32_t i = 1; i < instanceCount; i++) {
                curr[i].key = uint64_t(Pass::SENTINEL);
            }
        }

        curr = const_cast<Command*>(e);
    }

    if (UTILS_UNLIKELY(firstSentinel)) {
        //slog.d << "auto-instancing, saving " << drawCallsSavedCount << " draw calls, out of "
        //       << mCommandEnd - mCommandBegin << io::endl;

        // we have instanced primitives
        DriverApi& driver = engine.getDriverApi();

        // TODO: maybe use a pool? so we can reuse the buffer.
        // create a ubo to hold the instanced primitive data
        mInstancedUboHandle = driver.createBufferObject(
                sizeof(PerRenderableData) * instancedPrimitiveOffset + sizeof(PerRenderableUib),
                BufferObjectBinding::UNIFORM, backend::BufferUsage::STATIC);

        // copy our instanced ubo data
        driver.updateBufferObjectUnsynchronized(mInstancedUboHandle, {
                stagingBuffer, sizeof(PerRenderableData) * instancedPrimitiveOffset,
                +[](void* buffer, size_t, void*) {
                    ::free(buffer);
                }
        }, 0);

        stagingBuffer = nullptr;

        // remove all the canceled commands
        auto lastCommand = std::remove_if(firstSentinel, mCommandEnd, [](auto const& command) {
            return command.key == uint64_t(Pass::SENTINEL);
        });

        resize(arena, uint32_t(lastCommand - mCommandBegin));
    }

    assert_invariant(stagingBuffer == nullptr);
}


/* static */
UTILS_ALWAYS_INLINE // This function exists only to make the code more readable. we want it inlined.
inline              // and we don't need it in the compilation unit
void RenderPass::setupColorCommand(Command& cmdDraw, Variant variant,
        FMaterialInstance const* const UTILS_RESTRICT mi, bool inverseFrontFaces) noexcept {

    FMaterial const * const UTILS_RESTRICT ma = mi->getMaterial();
    variant = Variant::filterVariant(variant, ma->isVariantLit());

    // Below, we evaluate both commands to avoid a branch

    uint64_t keyBlending = cmdDraw.key;
    keyBlending &= ~(PASS_MASK | BLENDING_MASK);
    keyBlending |= uint64_t(Pass::BLENDED);
    keyBlending |= uint64_t(CustomCommand::PASS);

    BlendingMode const blendingMode = ma->getBlendingMode();
    bool const hasScreenSpaceRefraction = ma->getRefractionMode() == RefractionMode::SCREEN_SPACE;
    bool const isBlendingCommand = !hasScreenSpaceRefraction &&
            (blendingMode != BlendingMode::OPAQUE && blendingMode != BlendingMode::MASKED);

    uint64_t keyDraw = cmdDraw.key;
    keyDraw &= ~(PASS_MASK | BLENDING_MASK | MATERIAL_MASK);
    keyDraw |= uint64_t(hasScreenSpaceRefraction ? Pass::REFRACT : Pass::COLOR);
    keyDraw |= uint64_t(CustomCommand::PASS);
    keyDraw |= mi->getSortingKey(); // already all set-up for direct or'ing
    keyDraw |= makeField(variant.key, MATERIAL_VARIANT_KEY_MASK, MATERIAL_VARIANT_KEY_SHIFT);
    keyDraw |= makeField(ma->getRasterState().alphaToCoverage, BLENDING_MASK, BLENDING_SHIFT);

    cmdDraw.key = isBlendingCommand ? keyBlending : keyDraw;
    cmdDraw.primitive.rasterState = ma->getRasterState();

    // for SSR pass, the blending mode of opaques (including MASKED) must be off
    // see Material.cpp.
    const bool blendingMustBeOff = !isBlendingCommand && Variant::isSSRVariant(variant);
    cmdDraw.primitive.rasterState.blendFunctionSrcAlpha = blendingMustBeOff ?
            BlendFunction::ONE : cmdDraw.primitive.rasterState.blendFunctionSrcAlpha;
    cmdDraw.primitive.rasterState.blendFunctionDstAlpha = blendingMustBeOff ?
            BlendFunction::ZERO : cmdDraw.primitive.rasterState.blendFunctionDstAlpha;

    cmdDraw.primitive.rasterState.inverseFrontFaces = inverseFrontFaces;
    cmdDraw.primitive.rasterState.culling = mi->getCullingMode();
    cmdDraw.primitive.rasterState.colorWrite = mi->isColorWriteEnabled();
    cmdDraw.primitive.rasterState.depthWrite = mi->isDepthWriteEnabled();
    cmdDraw.primitive.rasterState.depthFunc = mi->getDepthFunc();
    cmdDraw.primitive.mi = mi;
    cmdDraw.primitive.materialVariant = variant;
    // we keep "RasterState::colorWrite" to the value set by material (could be disabled)
}

/* static */
UTILS_NOINLINE
void RenderPass::generateCommands(CommandTypeFlags commandTypeFlags, Command* const commands,
        FScene::RenderableSoa const& soa, Range<uint32_t> range,
        Variant variant, RenderFlags renderFlags,
        FScene::VisibleMaskType visibilityMask, float3 cameraPosition, float3 cameraForward,
        uint8_t instancedStereoEyeCount) noexcept {

    SYSTRACE_CALL();

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    // compute how much maximum storage we need
    // double the color pass for transparent objects that need to render twice
    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);
    const size_t commandsPerPrimitive = uint32_t(colorPass * 2 + depthPass);
    const size_t offsetBegin = FScene::getPrimitiveCount(soa, range.first) * commandsPerPrimitive;
    const size_t offsetEnd   = FScene::getPrimitiveCount(soa, range.last) * commandsPerPrimitive;
    Command* curr = commands + offsetBegin;
    Command* const last = commands + offsetEnd;

    /*
     * The switch {} below is to coerce the compiler into generating different versions of
     * "generateCommandsImpl" based on which pass we're processing.
     *
     *  We use a template function (as opposed to just inlining), so that the compiler is
     *  able to generate actual separate versions of generateCommandsImpl<>, which is much
     *  easier to debug and doesn't impact performance (it's just a predicted jump).
     */

    switch (commandTypeFlags & (CommandTypeFlags::COLOR | CommandTypeFlags::DEPTH)) {
        case CommandTypeFlags::COLOR:
            curr = generateCommandsImpl<CommandTypeFlags::COLOR>(commandTypeFlags, curr,
                    soa, range, variant, renderFlags, visibilityMask, cameraPosition, cameraForward,
                    instancedStereoEyeCount);
            break;
        case CommandTypeFlags::DEPTH:
            curr = generateCommandsImpl<CommandTypeFlags::DEPTH>(commandTypeFlags, curr,
                    soa, range, variant, renderFlags, visibilityMask, cameraPosition, cameraForward,
                    instancedStereoEyeCount);
            break;
        default:
            // we should never end-up here
            break;
    }

    assert_invariant(curr <= last);

    // commands may have been skipped, cancel all of them.
    while (curr != last) {
        curr->key = uint64_t(Pass::SENTINEL);
        ++curr;
    }
}

/* static */
template<RenderPass::CommandTypeFlags commandTypeFlags>
UTILS_NOINLINE
RenderPass::Command* RenderPass::generateCommandsImpl(RenderPass::CommandTypeFlags extraFlags,
        Command* UTILS_RESTRICT curr,
        FScene::RenderableSoa const& UTILS_RESTRICT soa, Range<uint32_t> range,
        Variant const variant, RenderFlags renderFlags, FScene::VisibleMaskType visibilityMask,
        float3 cameraPosition, float3 cameraForward, uint8_t instancedStereoEyeCount) noexcept {

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    constexpr bool isColorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    constexpr bool isDepthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);

    static_assert(isColorPass != isDepthPass, "only color or depth pass supported");

    const bool depthContainsShadowCasters       = bool(extraFlags & CommandTypeFlags::DEPTH_CONTAINS_SHADOW_CASTERS);
    const bool depthFilterAlphaMaskedObjects    = bool(extraFlags & CommandTypeFlags::DEPTH_FILTER_ALPHA_MASKED_OBJECTS);
    const bool filterTranslucentObjects         = bool(extraFlags & CommandTypeFlags::FILTER_TRANSLUCENT_OBJECTS);

    auto const* const UTILS_RESTRICT soaWorldAABBCenter     = soa.data<FScene::WORLD_AABB_CENTER>();
    auto const* const UTILS_RESTRICT soaVisibility          = soa.data<FScene::VISIBILITY_STATE>();
    auto const* const UTILS_RESTRICT soaPrimitives          = soa.data<FScene::PRIMITIVES>();
    auto const* const UTILS_RESTRICT soaSkinning            = soa.data<FScene::SKINNING_BUFFER>();
    auto const* const UTILS_RESTRICT soaMorphing            = soa.data<FScene::MORPHING_BUFFER>();
    auto const* const UTILS_RESTRICT soaVisibilityMask      = soa.data<FScene::VISIBLE_MASK>();
    auto const* const UTILS_RESTRICT soaInstanceInfo        = soa.data<FScene::INSTANCES>();

    const bool hasShadowing = renderFlags & HAS_SHADOWING;
    const bool viewInverseFrontFaces = renderFlags & HAS_INVERSE_FRONT_FACES;
    const bool hasInstancedStereo = renderFlags & IS_STEREOSCOPIC;

    Command cmdColor;

    Command cmdDepth;
    if constexpr (isDepthPass) {
        cmdDepth.primitive.materialVariant = variant;
        cmdDepth.primitive.rasterState = {};
        cmdDepth.primitive.rasterState.colorWrite = Variant::isPickingVariant(variant) || Variant::isVSMVariant(variant);
        cmdDepth.primitive.rasterState.depthWrite = true;
        cmdDepth.primitive.rasterState.depthFunc = RasterState::DepthFunc::GE;
        cmdDepth.primitive.rasterState.alphaToCoverage = false;
    }

    const float cameraPositionDotCameraForward = dot(cameraPosition, cameraForward);

    for (uint32_t i = range.first; i < range.last; ++i) {
        // Check if this renderable passes the visibilityMask.
        if (UTILS_UNLIKELY(!(soaVisibilityMask[i] & visibilityMask))) {
            continue;
        }

        Variant renderableVariant = variant;

        // Signed distance from camera plane to object's center. Positive distances are in front of
        // the camera. Some objects with a center behind the camera can still be visible
        // so their distance will be negative (this happens a lot for the shadow map).

        // Using the center is not very good with large AABBs. Instead, we can try to use
        // the closest point on the bounding sphere instead:
        //      d = soaWorldAABBCenter[i] - cameraPosition;
        //      d -= normalize(d) * length(soaWorldAABB[i].halfExtent);
        // However this doesn't work well at all for large planes.

        // Code below is equivalent to:
        // float3 d = soaWorldAABBCenter[i] - cameraPosition;
        // float distance = dot(d, cameraForward);
        // but saves a couple of instruction, because part of the math is done outside the loop.
        float distance = dot(soaWorldAABBCenter[i], cameraForward) - cameraPositionDotCameraForward;

        // We negate the distance to the camera in order to create a bit pattern that will
        // be sorted properly, this works because:
        // - positive distances (now negative), will still be sorted by their absolute value
        //   due to float representation.
        // - negative distances (now positive) will be sorted BEFORE everything else, and we
        //   don't care too much about their order (i.e. should objects far behind the camera
        //   be sorted first? -- unclear, and probably irrelevant).
        //   Here, objects close to the camera (but behind) will be drawn first.
        // An alternative that keeps the mathematical ordering is given here:
        //   distanceBits ^= ((int32_t(distanceBits) >> 31) | 0x80000000u);
        distance = -distance;
        const uint32_t distanceBits = reinterpret_cast<uint32_t&>(distance);

        // calculate the per-primitive face winding order inversion
        const bool inverseFrontFaces = viewInverseFrontFaces ^ soaVisibility[i].reversedWindingOrder;
        const bool hasMorphing = soaVisibility[i].morphing;
        const bool hasSkinningOrMorphing = soaVisibility[i].skinning || hasMorphing;

        cmdColor.key = makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmdColor.key |= makeField(soaVisibility[i].channel, CHANNEL_MASK, CHANNEL_SHIFT);
        cmdColor.primitive.index = (uint16_t)i;
        cmdColor.primitive.instanceCount =
                soaInstanceInfo[i].count | PrimitiveInfo::USER_INSTANCE_MASK;
        cmdColor.primitive.instanceBufferHandle = soaInstanceInfo[i].handle;

        // soaInstanceInfo[i].count is the number of instances the user has requested, either for
        // manual or hybrid instancing. Instanced stereo multiplies the number of instances by the
        // eye count.
        if (UTILS_UNLIKELY(hasInstancedStereo)) {
            cmdColor.primitive.instanceCount =
                    (soaInstanceInfo[i].count * instancedStereoEyeCount) |
                    PrimitiveInfo::USER_INSTANCE_MASK;
        }

        // if we are already an SSR variant, the SRE bit is already set,
        // there is no harm setting it again
        static_assert(Variant::SPECIAL_SSR & Variant::SRE);
        renderableVariant.setShadowReceiver(
                Variant::isSSRVariant(variant) || (soaVisibility[i].receiveShadows & hasShadowing));
        renderableVariant.setSkinning(hasSkinningOrMorphing);

        if constexpr (isDepthPass) {
            cmdDepth.key = uint64_t(Pass::DEPTH);
            cmdDepth.key |= uint64_t(CustomCommand::PASS);
            cmdDepth.key |= makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
            cmdDepth.key |= makeField(soaVisibility[i].channel, CHANNEL_MASK, CHANNEL_SHIFT);
            cmdDepth.key |= makeField(distanceBits >> 22u, Z_BUCKET_MASK, Z_BUCKET_SHIFT);
            cmdDepth.primitive.index = (uint16_t)i;
            cmdDepth.primitive.instanceCount =
                    soaInstanceInfo[i].count | PrimitiveInfo::USER_INSTANCE_MASK;
            cmdDepth.primitive.instanceBufferHandle = soaInstanceInfo[i].handle;
            cmdDepth.primitive.materialVariant.setSkinning(hasSkinningOrMorphing);
            cmdDepth.primitive.rasterState.inverseFrontFaces = inverseFrontFaces;

            if (UTILS_UNLIKELY(hasInstancedStereo)) {
                cmdColor.primitive.instanceCount =
                        (soaInstanceInfo[i].count * instancedStereoEyeCount) |
                        PrimitiveInfo::USER_INSTANCE_MASK;
            }
        }
        if constexpr (isColorPass) {
            renderableVariant.setFog(soaVisibility[i].fog && Variant::isFogVariant(variant));
        }

        const bool shadowCaster = soaVisibility[i].castShadows & hasShadowing;
        const bool writeDepthForShadowCasters = depthContainsShadowCasters & shadowCaster;

        const Slice<FRenderPrimitive>& primitives = soaPrimitives[i];
        const FRenderableManager::SkinningBindingInfo& skinning = soaSkinning[i];
        const FRenderableManager::MorphingBindingInfo& morphing = soaMorphing[i];

        /*
         * This is our hot loop. It's written to avoid branches.
         * When modifying this code, always ensure it stays efficient.
         */
        for (size_t pi = 0, c = primitives.size(); pi < c; ++pi) {
            auto const& primitive = primitives[pi];
            auto const& morphTargets = morphing.targets[pi];
            FMaterialInstance const* const mi = primitive.getMaterialInstance();
            FMaterial const* const ma = mi->getMaterial();

            if constexpr (isColorPass) {
                cmdColor.primitive.primitiveHandle = primitive.getHwHandle();
                RenderPass::setupColorCommand(cmdColor, renderableVariant, mi, inverseFrontFaces);

                cmdColor.primitive.skinningHandle = skinning.handle;
                cmdColor.primitive.skinningOffset = skinning.offset;
                cmdColor.primitive.skinningTexture = skinning.handleSampler;

                cmdColor.primitive.morphWeightBuffer = morphing.handle;
                cmdColor.primitive.morphTargetBuffer = morphTargets.buffer->getHwHandle();

                const bool blendPass = Pass(cmdColor.key & PASS_MASK) == Pass::BLENDED;
                if (blendPass) {
                    // TODO: at least for transparent objects, AABB should be per primitive
                    //       but that would break the "local" blend-order, which relies on
                    //       all primitives having the same Z
                    // blend pass:
                    //   This will sort back-to-front for blended, and honor explicit ordering
                    //   for a given Z value, or globally.
                    cmdColor.key &= ~BLEND_ORDER_MASK;
                    cmdColor.key &= ~BLEND_DISTANCE_MASK;
                    // write the distance
                    cmdColor.key |= makeField(~distanceBits,
                            BLEND_DISTANCE_MASK, BLEND_DISTANCE_SHIFT);
                    // clear the distance if global ordering is enabled
                    cmdColor.key &= ~select(primitive.isGlobalBlendOrderEnabled(),
                            BLEND_DISTANCE_MASK);
                    // write blend order
                    cmdColor.key |= makeField(primitive.getBlendOrder(),
                            BLEND_ORDER_MASK, BLEND_ORDER_SHIFT);


                    const TransparencyMode mode = mi->getTransparencyMode();

                    // handle transparent objects, two techniques:
                    //
                    //   - TWO_PASSES_ONE_SIDE: draw the front faces in the depth buffer then
                    //     front faces with depth test in the color buffer.
                    //     In this mode we actually do not change the user's culling mode
                    //
                    //   - TWO_PASSES_TWO_SIDES: draw back faces first,
                    //     then front faces, both in the color buffer.
                    //     In this mode, we override the user's culling mode.

                    // TWO_PASSES_TWO_SIDES: this command will be issued 2nd, draw front faces
                    cmdColor.primitive.rasterState.culling =
                            (mode == TransparencyMode::TWO_PASSES_TWO_SIDES) ?
                            CullingMode::BACK : cmdColor.primitive.rasterState.culling;

                    uint64_t key = cmdColor.key;

                    // draw this command AFTER THE NEXT ONE
                    key |= makeField(1, BLEND_TWO_PASS_MASK, BLEND_TWO_PASS_SHIFT);

                    // correct for TransparencyMode::DEFAULT -- i.e. cancel the command
                    key |= select(mode == TransparencyMode::DEFAULT);

                    // cancel command if asked to filter translucent objects
                    key |= select(filterTranslucentObjects);

                    // cancel command if both front and back faces are culled
                    key |= select(mi->getCullingMode() == CullingMode::FRONT_AND_BACK);

                    *curr = cmdColor;
                    curr->key = key;
                    ++curr;

                    // TWO_PASSES_TWO_SIDES: this command will be issued first, draw back sides (i.e. cull front)
                    cmdColor.primitive.rasterState.culling =
                            (mode == TransparencyMode::TWO_PASSES_TWO_SIDES) ?
                            CullingMode::FRONT : cmdColor.primitive.rasterState.culling;

                    // TWO_PASSES_ONE_SIDE: this command will be issued first, draw (back side) in depth buffer only
                    cmdColor.primitive.rasterState.depthWrite |=  select(mode == TransparencyMode::TWO_PASSES_ONE_SIDE);
                    cmdColor.primitive.rasterState.colorWrite &= ~select(mode == TransparencyMode::TWO_PASSES_ONE_SIDE);
                    cmdColor.primitive.rasterState.depthFunc =
                            (mode == TransparencyMode::TWO_PASSES_ONE_SIDE) ?
                            SamplerCompareFunc::GE : cmdColor.primitive.rasterState.depthFunc;

                } else {
                    // color pass:
                    // This will bucket objects by Z, front-to-back and then sort by material
                    // in each buckets. We use the top 10 bits of the distance, which
                    // bucketizes the depth by its log2 and in 4 linear chunks in each bucket.
                    cmdColor.key &= ~Z_BUCKET_MASK;
                    cmdColor.key |= makeField(distanceBits >> 22u, Z_BUCKET_MASK, Z_BUCKET_SHIFT);
                }

                *curr = cmdColor;

                // cancel command if both front and back faces are culled
                curr->key |= select(mi->getCullingMode() == CullingMode::FRONT_AND_BACK);

                ++curr;
            }

            if constexpr (isDepthPass) {
                const RasterState rs = ma->getRasterState();
                const TransparencyMode mode = mi->getTransparencyMode();
                const BlendingMode blendingMode = ma->getBlendingMode();
                const bool translucent = (blendingMode != BlendingMode::OPAQUE
                        && blendingMode != BlendingMode::MASKED);

                cmdDepth.key |= mi->getSortingKey(); // already all set-up for direct or'ing

                // unconditionally write the command
                cmdDepth.primitive.primitiveHandle = primitive.getHwHandle();
                cmdDepth.primitive.mi = mi;
                cmdDepth.primitive.rasterState.culling = mi->getCullingMode();

                cmdDepth.primitive.skinningHandle = skinning.handle;
                cmdDepth.primitive.skinningOffset = skinning.offset;
                cmdDepth.primitive.skinningTexture = skinning.handleSampler;

                cmdDepth.primitive.morphWeightBuffer = morphing.handle;
                cmdDepth.primitive.morphTargetBuffer = morphTargets.buffer->getHwHandle();

                // FIXME: should writeDepthForShadowCasters take precedence over mi->getDepthWrite()?
                cmdDepth.primitive.rasterState.depthWrite = (1 // only keep bit 0
                        & (mi->isDepthWriteEnabled() | (mode == TransparencyMode::TWO_PASSES_ONE_SIDE))
                        & !(filterTranslucentObjects & translucent)
                        & !(depthFilterAlphaMaskedObjects & rs.alphaToCoverage))
                            | writeDepthForShadowCasters;

                *curr = cmdDepth;

                // cancel command if both front and back faces are culled
                curr->key |= select(mi->getCullingMode() == CullingMode::FRONT_AND_BACK);

                ++curr;
            }
        }
    }
    return curr;
}

void RenderPass::updateSummedPrimitiveCounts(
        FScene::RenderableSoa& renderableData, Range<uint32_t> vr) noexcept {
    auto const* const UTILS_RESTRICT primitives = renderableData.data<FScene::PRIMITIVES>();
    uint32_t* const UTILS_RESTRICT summedPrimitiveCount = renderableData.data<FScene::SUMMED_PRIMITIVE_COUNT>();
    uint32_t count = 0;
    for (uint32_t const i : vr) {
        summedPrimitiveCount[i] = count;
        count += primitives[i].size();
    }
    // we're guaranteed to have enough space at the end of vr
    summedPrimitiveCount[vr.last] = count;
}

// ------------------------------------------------------------------------------------------------

void RenderPass::Executor::overridePolygonOffset(backend::PolygonOffset const* polygonOffset) noexcept {
    if ((mPolygonOffsetOverride = (polygonOffset != nullptr))) { // NOLINT(*-assignment-in-if-condition)
        mPolygonOffset = *polygonOffset;
    }
}

void RenderPass::Executor::overrideScissor(backend::Viewport const* scissor) noexcept {
    if ((mScissorOverride = (scissor != nullptr))) { // NOLINT(*-assignment-in-if-condition)
        mScissor = *scissor;
    }
}

void RenderPass::Executor::overrideScissor(backend::Viewport const& scissor) noexcept {
    mScissorOverride = true;
    mScissor = scissor;
}

void RenderPass::Executor::execute(FEngine& engine, const char*) const noexcept {
    execute(engine, mCommands.begin(), mCommands.end());
}

UTILS_NOINLINE // no need to be inlined
void RenderPass::Executor::execute(FEngine& engine,
        const Command* first, const Command* last) const noexcept {

    SYSTRACE_CALL();
    SYSTRACE_CONTEXT();

    DriverApi& driver = engine.getDriverApi();
    size_t const capacity = engine.getMinCommandBufferSize();
    CircularBuffer const& circularBuffer = driver.getCircularBuffer();

    if (first != last) {
        SYSTRACE_VALUE32("commandCount", last - first);

        PipelineState pipeline{
                .polygonOffset = mPolygonOffset,
                .scissor = mScissor
        }, dummyPipeline;

        auto* const pPipelinePolygonOffset =
                mPolygonOffsetOverride ? &dummyPipeline.polygonOffset : &pipeline.polygonOffset;

        auto* const pScissor =
                mScissorOverride ? &dummyPipeline.scissor : &pipeline.scissor;

        FMaterialInstance const* UTILS_RESTRICT mi = nullptr;
        FMaterial const* UTILS_RESTRICT ma = nullptr;
        auto const* UTILS_RESTRICT pCustomCommands = mCustomCommands.data();

        // Maximum space occupied in the CircularBuffer by a single `Command`. This must be
        // reevaluated when the inner loop below adds DriverApi commands or when we change the
        // CommandStream protocol. Currently, the maximum is 240 bytes, and we use 256 to be on
        // the safer side.
        size_t const maxCommandSizeInBytes = 256;

        // Number of Commands that can be issued and guaranteed to fit in the current
        // CircularBuffer allocation. In practice, we'll have tons of headroom especially if
        // skinning and morphing aren't used. With a 2 MiB buffer (the default) a batch is
        // 8192 commands (i.e. draw calls).
        size_t const batchCommandCount = capacity / maxCommandSizeInBytes;
        while(first != last) {
            Command const* const batchLast = std::min(first + batchCommandCount, last);

            // actual number of commands we need to write (can be smaller than batchCommandCount)
            size_t const commandCount = batchLast - first;
            size_t const commandSizeInBytes = commandCount * maxCommandSizeInBytes;

            // check we have enough capacity to write these commandCount commands, if not,
            // request a new CircularBuffer allocation of `capacity` bytes.
            if (UTILS_UNLIKELY(circularBuffer.getUsed() > capacity - commandSizeInBytes)) {
                engine.flush(); // TODO: we should use a "fast" flush if possible
            }

            first--;
            while (++first != batchLast) {
                assert_invariant(first->key != uint64_t(Pass::SENTINEL));

                /*
                 * Be careful when changing code below, this is the hot inner-loop
                 */

                if (UTILS_UNLIKELY((first->key & CUSTOM_MASK) != uint64_t(CustomCommand::PASS))) {
                    mi = nullptr; // custom command could change the currently bound MaterialInstance
                    uint32_t const index = (first->key & CUSTOM_INDEX_MASK) >> CUSTOM_INDEX_SHIFT;
                    assert_invariant(index < mCustomCommands.size());
                    pCustomCommands[index]();
                    continue;
                }

                // primitiveHandle may be invalid if no geometry was set on the renderable.
                if (UTILS_UNLIKELY(!first->primitive.primitiveHandle)) {
                    continue;
                }

                // per-renderable uniform
                const PrimitiveInfo info = first->primitive;
                pipeline.rasterState = info.rasterState;

                if (UTILS_UNLIKELY(mi != info.mi)) {
                    // this is always taken the first time
                    mi = info.mi;
                    assert_invariant(mi);

                    ma = mi->getMaterial();

                    auto const& scissor = mi->getScissor();
                    if (UTILS_UNLIKELY(mi->hasScissor())) {
                        // scissor is set, we need to apply the offset/clip
                        // clang vectorizes this!
                        constexpr int32_t maxvali = std::numeric_limits<int32_t>::max();
                        const backend::Viewport scissorViewport = mScissorViewport;
                        // compute new left/bottom, assume no overflow
                        int32_t l = scissor.left + scissorViewport.left;
                        int32_t b = scissor.bottom + scissorViewport.bottom;
                        // compute right/top without overflowing, scissor.width/height guaranteed
                        // to convert to int32
                        int32_t r = (l > maxvali - int32_t(scissor.width)) ?
                                    maxvali : l + int32_t(scissor.width);
                        int32_t t = (b > maxvali - int32_t(scissor.height)) ?
                                    maxvali : b + int32_t(scissor.height);
                        // clip to the viewport
                        l = std::max(l, scissorViewport.left);
                        b = std::max(b, scissorViewport.bottom);
                        r = std::min(r, scissorViewport.left + int32_t(scissorViewport.width));
                        t = std::min(t, scissorViewport.bottom + int32_t(scissorViewport.height));
                        assert_invariant(r >= l && t >= b);
                        *pScissor = { l, b, uint32_t(r - l), uint32_t(t - b) };
                    } else {
                        // no scissor set (common case), 'scissor' has its default value, use that.
                        *pScissor = scissor;
                    }

                    *pPipelinePolygonOffset = mi->getPolygonOffset();
                    pipeline.stencilState = mi->getStencilState();
                    mi->use(driver);
                }

                assert_invariant(ma);
                pipeline.program = ma->getProgram(info.materialVariant);

                uint16_t const instanceCount =
                        info.instanceCount & PrimitiveInfo::INSTANCE_COUNT_MASK;
                auto getPerObjectUboHandle =
                        [this, &info, &instanceCount]() -> std::pair<Handle<backend::HwBufferObject>, uint32_t> {
                            if (info.instanceBufferHandle) {
                                // "hybrid" instancing -- instanceBufferHandle takes the place of the UBO
                                return { info.instanceBufferHandle, 0 };
                            }
                            bool const userInstancing =
                                    (info.instanceCount & PrimitiveInfo::USER_INSTANCE_MASK) != 0u;
                            if (!userInstancing && instanceCount > 1) {
                                // automatic instancing
                                return {
                                        mInstancedUboHandle,
                                        info.index * sizeof(PerRenderableData) };
                            } else {
                                // manual instancing
                                return { mUboHandle, info.index * sizeof(PerRenderableData) };
                            }
                        };

                // Bind per-renderable uniform block. There is no need to attempt to skip this command
                // because the backends already do this.
                auto const [perObjectUboHandle, offset] = getPerObjectUboHandle();
                assert_invariant(perObjectUboHandle);
                driver.bindBufferRange(BufferObjectBinding::UNIFORM,
                        +UniformBindingPoints::PER_RENDERABLE,
                        perObjectUboHandle,
                        offset,
                        sizeof(PerRenderableUib));

                if (UTILS_UNLIKELY(info.skinningHandle)) {
                    // note: we can't bind less than sizeof(PerRenderableBoneUib) due to glsl limitations
                    driver.bindBufferRange(BufferObjectBinding::UNIFORM,
                            +UniformBindingPoints::PER_RENDERABLE_BONES,
                            info.skinningHandle,
                            info.skinningOffset * sizeof(PerRenderableBoneUib::BoneData),
                            sizeof(PerRenderableBoneUib));
                    // note: always bind the skinningTexture because the shader needs it.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_SKINNING,
                            info.skinningTexture);
                    // note: even if only skinning is enabled, binding morphTargetBuffer is needed.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_MORPHING,
                            info.morphTargetBuffer);
                }

                if (UTILS_UNLIKELY(info.morphWeightBuffer)) {
                    // Instead of using a UBO per primitive, we could also have a single UBO for all
                    // primitives and use bindUniformBufferRange which might be more efficient.
                    driver.bindUniformBuffer(+UniformBindingPoints::PER_RENDERABLE_MORPHING,
                            info.morphWeightBuffer);
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_MORPHING,
                            info.morphTargetBuffer);
                    // note: even if only morphing is enabled, binding skinningTexture is needed.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_SKINNING,
                            info.skinningTexture);
                }

                driver.draw(pipeline, info.primitiveHandle, instanceCount);
            }
        }

        // If the remaining space is less than half the capacity, we flush right away to
        // allow some headroom for commands that might come later.
        if (UTILS_UNLIKELY(circularBuffer.getUsed() > capacity / 2)) {
            engine.flush();
        }
    }

    if (mInstancedUboHandle) {
        driver.destroyBufferObject(mInstancedUboHandle);
    }

}

// ------------------------------------------------------------------------------------------------

RenderPass::Executor::Executor(RenderPass const* pass, Command const* b, Command const* e) noexcept
        : mCommands(b, e),
          mCustomCommands(pass->mCustomCommands.data(), pass->mCustomCommands.size()),
          mUboHandle(pass->mUboHandle),
          mInstancedUboHandle(pass->mInstancedUboHandle),
          mScissorViewport(pass->mScissorViewport),
          mPolygonOffsetOverride(false),
          mScissorOverride(false) {
    assert_invariant(b >= pass->begin());
    assert_invariant(e <= pass->end());
}

RenderPass::Executor::Executor(Executor const& rhs) = default;

// this destructor is actually heavy because it inlines ~vector<>
RenderPass::Executor::~Executor() noexcept = default;

} // namespace filament
