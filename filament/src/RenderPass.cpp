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
#include "SharedHandle.h"

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
#include <string.h>

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

void RenderPass::BufferObjectHandleDeleter::operator()(
        backend::BufferObjectHandle handle) noexcept {
    if (handle) {
        driver.get().destroyBufferObject(handle);
    }
}

// ------------------------------------------------------------------------------------------------

RenderPass::RenderPass(FEngine& engine, RenderPassBuilder const& builder) noexcept
        : mRenderableSoa(*builder.mRenderableSoa),
          mScissorViewport(builder.mScissorViewport),
          mCustomCommands(engine.getPerRenderPassArena()) {

    // compute the number of commands we need
    updateSummedPrimitiveCounts(
            const_cast<FScene::RenderableSoa&>(mRenderableSoa), builder.mVisibleRenderables);

    uint32_t commandCount =
            FScene::getPrimitiveCount(mRenderableSoa, builder.mVisibleRenderables.last);
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

    appendCommands(engine, { curr, commandCount },
            builder.mUboHandle,
            builder.mVisibleRenderables,
            builder.mCommandTypeFlags,
            builder.mFlags,
            builder.mVisibilityMask,
            builder.mVariant,
            builder.mCameraPosition,
            builder.mCameraForwardVector);

    if (builder.mCustomCommands.has_value()) {
        Command* p = curr + commandCount;
        for (auto [channel, passId, command, order, fn]: builder.mCustomCommands.value()) {
            appendCustomCommand(p++, channel, passId, command, order, fn);
        }
    }

    // sort commands once we're done adding commands
    sortCommands(builder.mArena);

    if (engine.isAutomaticInstancingEnabled()) {
        uint32_t stereoscopicEyeCount = 1;
        if (builder.mFlags & IS_INSTANCED_STEREOSCOPIC) {
            stereoscopicEyeCount *= engine.getConfig().stereoscopicEyeCount;
        }
        instanceify(engine, builder.mArena, stereoscopicEyeCount);
    }
}

// this destructor is actually heavy because it inlines ~vector<>
RenderPass::~RenderPass() noexcept {
}

void RenderPass::resize(Arena& arena, size_t count) noexcept {
    if (mCommandBegin) {
        mCommandEnd = mCommandBegin + count;
        arena.rewind(mCommandEnd);
    }
}

void RenderPass::appendCommands(FEngine& engine,
        Slice<Command> commands,
        backend::BufferObjectHandle const uboHandle,
        utils::Range<uint32_t> const vr,
        CommandTypeFlags const commandTypeFlags,
        RenderFlags const renderFlags,
        FScene::VisibleMaskType const visibilityMask,
        Variant const variant,
        float3 const cameraPosition,
        float3 const cameraForwardVector) noexcept {
    SYSTRACE_CALL();
    SYSTRACE_CONTEXT();

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

    // up-to-date summed primitive counts needed for generateCommands()
    FScene::RenderableSoa const& soa = mRenderableSoa;

    Command* curr = commands.data();
    size_t const commandCount = commands.size();

    auto stereoscopicEyeCount = engine.getConfig().stereoscopicEyeCount;

    auto work = [commandTypeFlags, curr, &soa,
                 boh = uboHandle,
                 variant, renderFlags, visibilityMask,
                 cameraPosition, cameraForwardVector, stereoscopicEyeCount]
            (uint32_t startIndex, uint32_t indexCount) {
        RenderPass::generateCommands(commandTypeFlags, curr,
                soa, { startIndex, startIndex + indexCount }, boh,
                variant, renderFlags, visibilityMask,
                cameraPosition, cameraForwardVector, stereoscopicEyeCount);
    };

    if (vr.size() <= JOBS_PARALLEL_FOR_COMMANDS_COUNT) {
        work(vr.first, vr.size());
    } else {
        auto* jobCommandsParallel = jobs::parallel_for(js, nullptr, vr.first, (uint32_t)vr.size(),
                std::cref(work), jobs::CountSplitter<JOBS_PARALLEL_FOR_COMMANDS_COUNT, 5>());
        js.runAndWait(jobCommandsParallel);
    }

    // Always add an "eof" command
    // "eof" command. These commands are guaranteed to be sorted last in the
    // command buffer.
    curr[commandCount - 1].key = uint64_t(Pass::SENTINEL);

    // Go over all the commands and call prepareProgram().
    // This must be done from the main thread.
    for (Command const* first = curr, *last = curr + commandCount ; first != last ; ++first) {
        if (UTILS_LIKELY((first->key & CUSTOM_MASK) == uint64_t(CustomCommand::PASS))) {
            auto ma = first->info.mi->getMaterial();
            ma->prepareProgram(first->info.materialVariant);
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

void RenderPass::instanceify(FEngine& engine, Arena& arena, int32_t eyeCount) noexcept {
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

        // Currently, if we have skinnning or morphing, we can't use auto instancing. This is
        // because the morphing/skinning data for comparison is not easily accessible.
        // Additionally, we can't have a different skinning/morphing per instance anyway.
        // And thirdly, the info.index meaning changes with instancing, it is the index into
        // the instancing buffer no longer the index into the soa.
        Command const* e = curr + 1;
        if (UTILS_LIKELY(!curr->info.hasSkinning && !curr->info.hasMorphing)) {
            // we can't have nice things! No more than maxInstanceCount due to UBO size limits
            e = std::find_if_not(curr, std::min(last, curr + maxInstanceCount),
                    [lhs = *curr](Command const& rhs) {
                        // primitives must be identical to be instanced.
                        // Currently, instancing doesn't support skinning/morphing.
                        return lhs.info.mi == rhs.info.mi &&
                               lhs.info.rph == rhs.info.rph &&
                               lhs.info.vbih == rhs.info.vbih &&
                               lhs.info.indexOffset == rhs.info.indexOffset &&
                               lhs.info.indexCount == rhs.info.indexCount &&
                               lhs.info.rasterState == rhs.info.rasterState;
                    });
        }

        uint32_t const instanceCount = e - curr;
        assert_invariant(instanceCount > 0);
        assert_invariant(instanceCount <= CONFIG_MAX_INSTANCES);

        if (UTILS_UNLIKELY(instanceCount > 1)) {
            drawCallsSavedCount += instanceCount - 1;

            // allocate our staging buffer only if needed
            if (UTILS_UNLIKELY(!stagingBuffer)) {

                // create a temporary UBO for instancing
                size_t const count = mCommandEnd - mCommandBegin;
                mInstancedUboHandle = BufferObjectSharedHandle{
                        engine.getDriverApi().createBufferObject(
                                count * sizeof(PerRenderableData) + sizeof(PerRenderableUib),
                                BufferObjectBinding::UNIFORM, BufferUsage::STATIC),
                        engine.getDriverApi() };

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
                stagingBuffer[instancedPrimitiveOffset + i] = uboData[curr[i].info.index];
            }

            // make the first command instanced
            curr[0].info.instanceCount = instanceCount * eyeCount;
            curr[0].info.index = instancedPrimitiveOffset;
            curr[0].info.boh = mInstancedUboHandle;

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
    cmdDraw.info.rasterState = ma->getRasterState();

    // for SSR pass, the blending mode of opaques (including MASKED) must be off
    // see Material.cpp.
    const bool blendingMustBeOff = !isBlendingCommand && Variant::isSSRVariant(variant);
    cmdDraw.info.rasterState.blendFunctionSrcAlpha = blendingMustBeOff ?
            BlendFunction::ONE : cmdDraw.info.rasterState.blendFunctionSrcAlpha;
    cmdDraw.info.rasterState.blendFunctionDstAlpha = blendingMustBeOff ?
            BlendFunction::ZERO : cmdDraw.info.rasterState.blendFunctionDstAlpha;

    cmdDraw.info.rasterState.inverseFrontFaces = inverseFrontFaces;
    cmdDraw.info.rasterState.culling = mi->getCullingMode();
    cmdDraw.info.rasterState.colorWrite = mi->isColorWriteEnabled();
    cmdDraw.info.rasterState.depthWrite = mi->isDepthWriteEnabled();
    cmdDraw.info.rasterState.depthFunc = mi->getDepthFunc();
    cmdDraw.info.materialVariant = variant;
    // we keep "RasterState::colorWrite" to the value set by material (could be disabled)
}

/* static */
UTILS_NOINLINE
void RenderPass::generateCommands(CommandTypeFlags commandTypeFlags, Command* const commands,
        FScene::RenderableSoa const& soa, Range<uint32_t> range,
        backend::BufferObjectHandle renderablesUbo,
        Variant variant, RenderFlags renderFlags,
        FScene::VisibleMaskType visibilityMask, float3 cameraPosition, float3 cameraForward,
        uint8_t stereoEyeCount) noexcept {

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
                    soa, range, renderablesUbo,
                    variant, renderFlags, visibilityMask, cameraPosition, cameraForward,
                    stereoEyeCount);
            break;
        case CommandTypeFlags::DEPTH:
            curr = generateCommandsImpl<CommandTypeFlags::DEPTH>(commandTypeFlags, curr,
                    soa, range, renderablesUbo,
                    variant, renderFlags, visibilityMask, cameraPosition, cameraForward,
                    stereoEyeCount);
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
        backend::BufferObjectHandle renderablesUbo,
        Variant const variant, RenderFlags renderFlags, FScene::VisibleMaskType visibilityMask,
        float3 cameraPosition, float3 cameraForward, uint8_t stereoEyeCount) noexcept {

    constexpr bool isColorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    constexpr bool isDepthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);
    static_assert(isColorPass != isDepthPass, "only color or depth pass supported");

    bool const depthContainsShadowCasters =
            bool(extraFlags & CommandTypeFlags::DEPTH_CONTAINS_SHADOW_CASTERS);

    bool const depthFilterAlphaMaskedObjects =
            bool(extraFlags & CommandTypeFlags::DEPTH_FILTER_ALPHA_MASKED_OBJECTS);

    bool const filterTranslucentObjects =
            bool(extraFlags & CommandTypeFlags::FILTER_TRANSLUCENT_OBJECTS);

    bool const hasShadowing =
            renderFlags & HAS_SHADOWING;

    bool const viewInverseFrontFaces =
            renderFlags & HAS_INVERSE_FRONT_FACES;

    bool const hasInstancedStereo =
            renderFlags & IS_INSTANCED_STEREOSCOPIC;

    float const cameraPositionDotCameraForward = dot(cameraPosition, cameraForward);

    auto const* const UTILS_RESTRICT soaWorldAABBCenter = soa.data<FScene::WORLD_AABB_CENTER>();
    auto const* const UTILS_RESTRICT soaVisibility      = soa.data<FScene::VISIBILITY_STATE>();
    auto const* const UTILS_RESTRICT soaPrimitives      = soa.data<FScene::PRIMITIVES>();
    auto const* const UTILS_RESTRICT soaSkinning        = soa.data<FScene::SKINNING_BUFFER>();
    auto const* const UTILS_RESTRICT soaMorphing        = soa.data<FScene::MORPHING_BUFFER>();
    auto const* const UTILS_RESTRICT soaVisibilityMask  = soa.data<FScene::VISIBLE_MASK>();
    auto const* const UTILS_RESTRICT soaInstanceInfo    = soa.data<FScene::INSTANCES>();

    Command cmd;

    if constexpr (isDepthPass) {
        cmd.info.materialVariant = variant;
        cmd.info.rasterState = {};
        cmd.info.rasterState.colorWrite = Variant::isPickingVariant(variant) || Variant::isVSMVariant(variant);
        cmd.info.rasterState.depthWrite = true;
        cmd.info.rasterState.depthFunc = RasterState::DepthFunc::GE;
        cmd.info.rasterState.alphaToCoverage = false;
    }

    for (uint32_t i = range.first; i < range.last; ++i) {
        // Check if this renderable passes the visibilityMask.
        if (UTILS_UNLIKELY(!(soaVisibilityMask[i] & visibilityMask))) {
            continue;
        }

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
        float const distance = -dot(soaWorldAABBCenter[i], cameraForward) - cameraPositionDotCameraForward;
        uint32_t const distanceBits = reinterpret_cast<uint32_t const&>(distance);

        // calculate the per-primitive face winding order inversion
        bool const inverseFrontFaces = viewInverseFrontFaces ^ soaVisibility[i].reversedWindingOrder;
        bool const hasMorphing = soaVisibility[i].morphing;
        bool const hasSkinning = soaVisibility[i].skinning;
        bool const hasSkinningOrMorphing = hasSkinning || hasMorphing;

        // if we are already an SSR variant, the SRE bit is already set,
        // there is no harm setting it again
        static_assert(Variant::SPECIAL_SSR & Variant::SRE);
        Variant renderableVariant = variant;
        renderableVariant.setShadowReceiver(
                Variant::isSSRVariant(variant) || (soaVisibility[i].receiveShadows & hasShadowing));
        renderableVariant.setSkinning(hasSkinningOrMorphing);

        FRenderableManager::SkinningBindingInfo const& skinning = soaSkinning[i];
        FRenderableManager::MorphingBindingInfo const& morphing = soaMorphing[i];

        if constexpr (isColorPass) {
            renderableVariant.setFog(soaVisibility[i].fog && Variant::isFogVariant(variant));
            cmd.key = uint64_t(Pass::COLOR);
        } else if constexpr (isDepthPass) {
            cmd.key = uint64_t(Pass::DEPTH);
            cmd.key |= uint64_t(CustomCommand::PASS);
            cmd.key |= makeField(distanceBits >> 22u, Z_BUCKET_MASK, Z_BUCKET_SHIFT);
            cmd.info.materialVariant.setSkinning(hasSkinningOrMorphing);
            cmd.info.rasterState.inverseFrontFaces = inverseFrontFaces;
        }

        cmd.key |= makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmd.key |= makeField(soaVisibility[i].channel, CHANNEL_MASK, CHANNEL_SHIFT);
        cmd.info.index = i;
        cmd.info.hasHybridInstancing = (bool)soaInstanceInfo[i].handle;
        cmd.info.instanceCount = soaInstanceInfo[i].count;
        cmd.info.hasMorphing = (bool)morphing.handle;
        cmd.info.hasSkinning = (bool)skinning.handle;

        // soaInstanceInfo[i].count is the number of instances the user has requested, either for
        // manual or hybrid instancing. Instanced stereo multiplies the number of instances by the
        // eye count.
        if (hasInstancedStereo) {
            cmd.info.instanceCount *= stereoEyeCount;
        }

        if (cmd.info.hasHybridInstancing) {
            // with hybrid instancing, we already know which UBO to use
            cmd.info.boh = soaInstanceInfo[i].handle;
        } else {
            // with no- or user- instancing, we can only know after instanceify()
            cmd.info.boh = renderablesUbo;
        }

        const bool shadowCaster = soaVisibility[i].castShadows & hasShadowing;
        const bool writeDepthForShadowCasters = depthContainsShadowCasters & shadowCaster;

        const Slice<FRenderPrimitive>& primitives = soaPrimitives[i];
        /*
         * This is our hot loop. It's written to avoid branches.
         * When modifying this code, always ensure it stays efficient.
         */
        for (size_t pi = 0, c = primitives.size(); pi < c; ++pi) {
            auto const& primitive = primitives[pi];
            auto const& morphTargets = morphing.targets[pi];
            FMaterialInstance const* const mi = primitive.getMaterialInstance();
            FMaterial const* const ma = mi->getMaterial();

            // TODO: we should disable the SKN variant if this primitive doesn't have either
            //       skinning or morphing.

            cmd.info.mi = mi;
            cmd.info.rph = primitive.getHwHandle();
            cmd.info.vbih = primitive.getVertexBufferInfoHandle();
            cmd.info.indexOffset = primitive.getIndexOffset();
            cmd.info.indexCount = primitive.getIndexCount();
            cmd.info.type = primitive.getPrimitiveType();
            cmd.info.morphTargetBuffer = morphTargets.buffer->getHwHandle();
            cmd.info.morphingOffset = morphTargets.offset;

            if constexpr (isColorPass) {
                RenderPass::setupColorCommand(cmd, renderableVariant, mi, inverseFrontFaces);
                const bool blendPass = Pass(cmd.key & PASS_MASK) == Pass::BLENDED;
                if (blendPass) {
                    // TODO: at least for transparent objects, AABB should be per primitive
                    //       but that would break the "local" blend-order, which relies on
                    //       all primitives having the same Z
                    // blend pass:
                    //   This will sort back-to-front for blended, and honor explicit ordering
                    //   for a given Z value, or globally.
                    cmd.key &= ~BLEND_ORDER_MASK;
                    cmd.key &= ~BLEND_DISTANCE_MASK;
                    // write the distance
                    cmd.key |= makeField(~distanceBits,
                            BLEND_DISTANCE_MASK, BLEND_DISTANCE_SHIFT);
                    // clear the distance if global ordering is enabled
                    cmd.key &= ~select(primitive.isGlobalBlendOrderEnabled(),
                            BLEND_DISTANCE_MASK);
                    // write blend order
                    cmd.key |= makeField(primitive.getBlendOrder(),
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
                    cmd.info.rasterState.culling =
                            (mode == TransparencyMode::TWO_PASSES_TWO_SIDES) ?
                            CullingMode::BACK : cmd.info.rasterState.culling;

                    uint64_t key = cmd.key;

                    // draw this command AFTER THE NEXT ONE
                    key |= makeField(1, BLEND_TWO_PASS_MASK, BLEND_TWO_PASS_SHIFT);

                    // correct for TransparencyMode::DEFAULT -- i.e. cancel the command
                    key |= select(mode == TransparencyMode::DEFAULT);

                    // cancel command if asked to filter translucent objects
                    key |= select(filterTranslucentObjects);

                    // cancel command if both front and back faces are culled
                    key |= select(mi->getCullingMode() == CullingMode::FRONT_AND_BACK);

                    *curr = cmd;
                    curr->key = key;
                    ++curr;

                    // TWO_PASSES_TWO_SIDES: this command will be issued first, draw back sides (i.e. cull front)
                    cmd.info.rasterState.culling =
                            (mode == TransparencyMode::TWO_PASSES_TWO_SIDES) ?
                            CullingMode::FRONT : cmd.info.rasterState.culling;

                    // TWO_PASSES_ONE_SIDE: this command will be issued first, draw (back side) in depth buffer only
                    cmd.info.rasterState.depthWrite |=  select(mode == TransparencyMode::TWO_PASSES_ONE_SIDE);
                    cmd.info.rasterState.colorWrite &= ~select(mode == TransparencyMode::TWO_PASSES_ONE_SIDE);
                    cmd.info.rasterState.depthFunc =
                            (mode == TransparencyMode::TWO_PASSES_ONE_SIDE) ?
                            SamplerCompareFunc::GE : cmd.info.rasterState.depthFunc;
                } else {
                    // color pass:
                    // This will bucket objects by Z, front-to-back and then sort by material
                    // in each buckets. We use the top 10 bits of the distance, which
                    // bucketizes the depth by its log2 and in 4 linear chunks in each bucket.
                    cmd.key &= ~Z_BUCKET_MASK;
                    cmd.key |= makeField(distanceBits >> 22u, Z_BUCKET_MASK, Z_BUCKET_SHIFT);
                }
            } else if constexpr (isDepthPass) {
                const RasterState rs = ma->getRasterState();
                const TransparencyMode mode = mi->getTransparencyMode();
                const BlendingMode blendingMode = ma->getBlendingMode();
                const bool translucent = (blendingMode != BlendingMode::OPAQUE
                        && blendingMode != BlendingMode::MASKED);

                cmd.key |= mi->getSortingKey(); // already all set-up for direct or'ing
                cmd.info.rasterState.culling = mi->getCullingMode();

                // FIXME: should writeDepthForShadowCasters take precedence over mi->getDepthWrite()?
                cmd.info.rasterState.depthWrite = (1 // only keep bit 0
                        & (mi->isDepthWriteEnabled() | (mode == TransparencyMode::TWO_PASSES_ONE_SIDE))
                                                   & !(filterTranslucentObjects & translucent)
                                                   & !(depthFilterAlphaMaskedObjects & rs.alphaToCoverage))
                                                  | writeDepthForShadowCasters;
            }

            *curr = cmd;
            // cancel command if both front and back faces are culled
            curr->key |= select(mi->getCullingMode() == CullingMode::FRONT_AND_BACK);
            ++curr;
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
backend::Viewport RenderPass::Executor::applyScissorViewport(
        backend::Viewport const& scissorViewport, backend::Viewport const& scissor) noexcept {
    // scissor is set, we need to apply the offset/clip
    // clang vectorizes this!
    constexpr int32_t maxvali = std::numeric_limits<int32_t>::max();
    // compute new left/bottom, assume no overflow
    int32_t l = scissor.left + scissorViewport.left;
    int32_t b = scissor.bottom + scissorViewport.bottom;
    // compute right/top without overflowing, scissor.width/height guaranteed
    // to convert to int32
    int32_t r = (l > maxvali - int32_t(scissor.width)) ? maxvali : l + int32_t(scissor.width);
    int32_t t = (b > maxvali - int32_t(scissor.height)) ? maxvali : b + int32_t(scissor.height);
    // clip to the viewport
    l = std::max(l, scissorViewport.left);
    b = std::max(b, scissorViewport.bottom);
    r = std::min(r, scissorViewport.left + int32_t(scissorViewport.width));
    t = std::min(t, scissorViewport.bottom + int32_t(scissorViewport.height));
    assert_invariant(r >= l && t >= b);
    return { l, b, uint32_t(r - l), uint32_t(t - b) };
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

        bool const scissorOverride = mScissorOverride;
        if (UTILS_UNLIKELY(scissorOverride)) {
            // initialize with scissor overide
            driver.scissor(mScissor);
        }

        bool const polygonOffsetOverride = mPolygonOffsetOverride;
        PipelineState pipeline{
                // initialize with polygon offset override
                .polygonOffset = mPolygonOffset,
        };

        PipelineState currentPipeline{};
        Handle<HwRenderPrimitive> currentPrimitiveHandle{};
        bool rebindPipeline = true;

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
                if (UTILS_UNLIKELY(!first->info.rph)) {
                    continue;
                }

                // per-renderable uniform
                PrimitiveInfo const info = first->info;
                pipeline.rasterState = info.rasterState;
                pipeline.vertexBufferInfo = info.vbih;
                pipeline.primitiveType = info.type;
                assert_invariant(pipeline.vertexBufferInfo);

                if (UTILS_UNLIKELY(mi != info.mi)) {
                    // this is always taken the first time
                    mi = info.mi;
                    assert_invariant(mi);

                    ma = mi->getMaterial();

                   if (UTILS_LIKELY(!scissorOverride)) {
                       backend::Viewport scissor = mi->getScissor();
                       if (UTILS_UNLIKELY(mi->hasScissor())) {
                           scissor = applyScissorViewport(mScissorViewport, scissor);
                       }
                       driver.scissor(scissor);
                   }

                   if (UTILS_LIKELY(!polygonOffsetOverride)) {
                       pipeline.polygonOffset = mi->getPolygonOffset();
                   }
                    pipeline.stencilState = mi->getStencilState();
                    mi->use(driver);

                    // FIXME: MaterialInstance changed (not necessarily the program though),
                    //  however, texture bindings may have changed and currently we need to
                    //  rebind the pipeline when that happens.
                    rebindPipeline = true;
                }

                assert_invariant(ma);
                pipeline.program = ma->getProgram(info.materialVariant);

                // Bind per-renderable uniform block. There is no need to attempt to skip this command
                // because the backends already do this.
                size_t const offset = info.hasHybridInstancing ?
                                      0 : info.index * sizeof(PerRenderableData);

                assert_invariant(info.boh);

                driver.bindBufferRange(BufferObjectBinding::UNIFORM,
                        +UniformBindingPoints::PER_RENDERABLE,
                        info.boh, offset, sizeof(PerRenderableUib));

                if (UTILS_UNLIKELY(info.hasSkinning)) {

                    FScene::RenderableSoa const& soa = *mRenderableSoa;

                    const FRenderableManager::SkinningBindingInfo& skinning =
                            soa.elementAt<FScene::SKINNING_BUFFER>(info.index);

                    // note: we can't bind less than sizeof(PerRenderableBoneUib) due to glsl limitations
                    driver.bindBufferRange(BufferObjectBinding::UNIFORM,
                            +UniformBindingPoints::PER_RENDERABLE_BONES,
                            skinning.handle,
                            skinning.offset * sizeof(PerRenderableBoneUib::BoneData),
                            sizeof(PerRenderableBoneUib));
                    // note: always bind the skinningTexture because the shader needs it.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_SKINNING,
                            skinning.handleSampler);
                    // note: even if only skinning is enabled, binding morphTargetBuffer is needed.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_MORPHING,
                            info.morphTargetBuffer);

                    // FIXME: Currently we need to rebind the PipelineState when texture or
                    //  UBO binding change.
                    rebindPipeline = true;
                }

                if (UTILS_UNLIKELY(info.hasMorphing)) {

                    FScene::RenderableSoa const& soa = *mRenderableSoa;

                    const FRenderableManager::SkinningBindingInfo& skinning =
                            soa.elementAt<FScene::SKINNING_BUFFER>(info.index);

                    const FRenderableManager::MorphingBindingInfo& morphing =
                            soa.elementAt<FScene::MORPHING_BUFFER>(info.index);

                    // Instead of using a UBO per primitive, we could also have a single UBO for all
                    // primitives and use bindUniformBufferRange which might be more efficient.
                    driver.bindUniformBuffer(+UniformBindingPoints::PER_RENDERABLE_MORPHING,
                            morphing.handle);
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_MORPHING,
                            info.morphTargetBuffer);
                    // note: even if only morphing is enabled, binding skinningTexture is needed.
                    driver.bindSamplers(+SamplerBindingPoints::PER_RENDERABLE_SKINNING,
                            skinning.handleSampler);

                    // FIXME: Currently we need to rebind the PipelineState when texture or
                    //  UBO binding change.
                    rebindPipeline = true;
                }

                if (rebindPipeline ||
                        (memcmp(&pipeline,  &currentPipeline, sizeof(PipelineState)) != 0)) {
                    rebindPipeline = false;
                    currentPipeline = pipeline;
                    driver.bindPipeline(pipeline);

                    driver.setPushConstant(ShaderStage::VERTEX,
                            +PushConstantIds::MORPHING_BUFFER_OFFSET, int32_t(info.morphingOffset));
                }

                if (info.rph != currentPrimitiveHandle) {
                    currentPrimitiveHandle = info.rph;
                    driver.bindRenderPrimitive(info.rph);
                }

                driver.draw2(info.indexOffset, info.indexCount, info.instanceCount);
            }
        }

        // If the remaining space is less than half the capacity, we flush right away to
        // allow some headroom for commands that might come later.
        if (UTILS_UNLIKELY(circularBuffer.getUsed() > capacity / 2)) {
            engine.flush();
        }
    }
}

// ------------------------------------------------------------------------------------------------

RenderPass::Executor::Executor(RenderPass const* pass, Command const* b, Command const* e,
        BufferObjectSharedHandle instancedUbo) noexcept
        : mRenderableSoa(&pass->mRenderableSoa),
          mCommands(b, e),
          mCustomCommands(pass->mCustomCommands.data(), pass->mCustomCommands.size()),
          mInstancedUboHandle(std::move(instancedUbo)),
          mScissorViewport(pass->mScissorViewport),
          mPolygonOffsetOverride(false),
          mScissorOverride(false) {
    assert_invariant(b >= pass->begin());
    assert_invariant(e <= pass->end());
}

RenderPass::Executor::Executor() noexcept
        : mPolygonOffsetOverride(false),
          mScissorOverride(false) {
}

RenderPass::Executor::Executor(Executor&& rhs) noexcept = default;

RenderPass::Executor& RenderPass::Executor::operator=(Executor&& rhs) noexcept = default;

// this destructor is actually heavy because it inlines ~vector<>
RenderPass::Executor::~Executor() noexcept = default;

} // namespace filament
