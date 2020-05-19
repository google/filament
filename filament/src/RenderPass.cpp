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

#include "details/Material.h"
#include "details/MaterialInstance.h"
#include "details/RenderPrimitive.h"
#include "details/ShadowMap.h"
#include "details/View.h"

// NOTE: We only need Renderer.h here because the definition of some FRenderer methods are here
#include "details/Renderer.h"

#include <private/filament/UibGenerator.h>

#include <utils/JobSystem.h>
#include <utils/Systrace.h>

#include <utility>

using namespace utils;
using namespace filament::math;

namespace filament {

using namespace backend;

RenderPass::RenderPass(FEngine& engine,
        GrowingSlice<RenderPass::Command> commands) noexcept
        : mEngine(engine), mCommands(commands),
          mCustomCommands(engine.getPerRenderPassAllocator()) {
    mCustomCommands.reserve(8); // preallocate allocate a reasonable number of custom commands
}

void RenderPass::setGeometry(FScene::RenderableSoa const& soa, Range<uint32_t> vr,
        backend::Handle<backend::HwUniformBuffer> uboHandle) noexcept {
    mRenderableSoa = &soa;
    mVisibleRenderables = vr;
    mUboHandle = uboHandle;
}

void RenderPass::setCamera(const CameraInfo& camera) noexcept {
    mCamera = camera;
}

void RenderPass::setRenderFlags(RenderPass::RenderFlags flags) noexcept {
    mFlags = flags;
}

void RenderPass::overridePolygonOffset(backend::PolygonOffset* polygonOffset) noexcept {
    if ((mPolygonOffsetOverride = (polygonOffset != nullptr))) {
        mPolygonOffset = *polygonOffset;
    }
}

RenderPass::Command* RenderPass::newCommandBuffer() noexcept {
    GrowingSlice<Command>& commands = mCommands;
    commands = GrowingSlice<Command>(commands.end(), commands.capacity() - commands.size());
    return commands.begin();
}

RenderPass::Command* RenderPass::appendCommands(CommandTypeFlags const commandTypeFlags) noexcept {
    SYSTRACE_CONTEXT();

    FEngine& engine = mEngine;
    JobSystem& js = engine.getJobSystem();
    GrowingSlice<Command>& commands = mCommands;
    const RenderFlags renderFlags = mFlags;
    const FScene::VisibleMaskType visibilityMask = mVisibilityMask;
    CameraInfo const& camera = mCamera;
    utils::Range<uint32_t> vr = mVisibleRenderables;
    if (UTILS_UNLIKELY(vr.empty())) {
        return commands.end();
    }
    assert(mRenderableSoa);

    // trace the number of visible renderables
    SYSTRACE_VALUE32("visibleRenderables", vr.size());

    // up-to-date summed primitive counts needed for generateCommands()
    FScene::RenderableSoa const& soa = *mRenderableSoa;
    updateSummedPrimitiveCounts(const_cast<FScene::RenderableSoa&>(soa), vr);

    // compute how much maximum storage we need for this pass
    uint32_t growBy = FScene::getPrimitiveCount(soa, vr.last);
    // double the color pass for transparent objects that need to render twice
    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);
    growBy *= uint32_t(colorPass * 2 + depthPass);
    Command* const curr = commands.grow(growBy);

    // we extract camera position/forward outside of the loop, because these are not cheap.
    const float3 cameraPosition(camera.getPosition());
    const float3 cameraForwardVector(camera.getForwardVector());
    auto work = [commandTypeFlags, curr, &soa, renderFlags, visibilityMask, cameraPosition,
                 cameraForwardVector]
            (uint32_t startIndex, uint32_t indexCount) {
        RenderPass::generateCommands(commandTypeFlags, curr,
                soa, { startIndex, startIndex + indexCount }, renderFlags, visibilityMask,
                cameraPosition, cameraForwardVector);
    };

    auto jobCommandsParallel = jobs::parallel_for(js, nullptr, vr.first, (uint32_t)vr.size(),
            std::cref(work), jobs::CountSplitter<JOBS_PARALLEL_FOR_COMMANDS_COUNT, 8>());

    { // scope for systrace
        SYSTRACE_NAME("jobCommandsParallel");
        js.runAndWait(jobCommandsParallel);
    }

    // always add an "eof" command
    // "eof" command. these commands are guaranteed to be sorted last in the
    // command buffer.
    commands.grow(1)->key = uint64_t(Pass::SENTINEL);

    mCommandsHighWatermark = std::max(mCommandsHighWatermark, size_t(commands.size()));

    return commands.end();
}

RenderPass::Command* RenderPass::appendCustomCommand(Pass pass, CustomCommand custom, uint32_t order,
        std::function<void()> command) {

    assert((uint64_t(order) << CUSTOM_ORDER_SHIFT) <=  CUSTOM_ORDER_MASK);

    uint32_t index = mCustomCommands.size();
    mCustomCommands.push_back(std::move(command));

    uint64_t cmd = uint64_t(pass);
    cmd |= uint64_t(custom);
    cmd |= uint64_t(order) << CUSTOM_ORDER_SHIFT;
    cmd |= uint64_t(index);

    Command* const curr = mCommands.grow(1);
    curr->key = cmd;
    return curr + 1;
}

RenderPass::Command* RenderPass::sortCommands() noexcept {
    SYSTRACE_NAME("sort and trim commands");

    GrowingSlice<Command>& commands = mCommands;

    std::sort(commands.begin(), commands.end());

    // find the last command
    Command const* const last = std::partition_point(commands.begin(), commands.end(),
            [](Command const& c) {
                return c.key != uint64_t(Pass::SENTINEL);
            });

    commands.resize(uint32_t(last - commands.begin()));

    return commands.end();
}

void RenderPass::execute(const char* name,
        backend::Handle<backend::HwRenderTarget> renderTarget,
        backend::RenderPassParams params) const noexcept {

    FEngine& engine = mEngine;
    Command const* const first = mCommands.begin();
    Command const* const last = mCommands.end();

    // Take care not to upload data within the render pass (synchronize can commit froxel data)
    DriverApi& driver = engine.getDriverApi();

    // Now, execute all commands
    driver.beginRenderPass(renderTarget, params);
    RenderPass::recordDriverCommands(driver, first, last);
    driver.endRenderPass();
}

UTILS_NOINLINE // no need to be inlined
void RenderPass::recordDriverCommands(FEngine::DriverApi& driver, const Command* first,
        const Command* last) const noexcept {
    SYSTRACE_CALL();

    if (first != last) {
        SYSTRACE_VALUE32("commandCount", last - first);

        PolygonOffset dummyPolyOffset;
        PipelineState pipeline{ .polygonOffset = mPolygonOffset };
        PolygonOffset* const pPipelinePolygonOffset =
                mPolygonOffsetOverride ? &dummyPolyOffset : &pipeline.polygonOffset;

        Handle<HwUniformBuffer> uboHandle = mUboHandle;
        FMaterialInstance const* UTILS_RESTRICT mi = nullptr;
        FMaterial const* UTILS_RESTRICT ma = nullptr;
        auto const& customCommands = mCustomCommands;

        first--;
        while (++first != last) {
            /*
             * Be careful when changing code below, this is the hot inner-loop
             */

            if (UTILS_UNLIKELY((first->key & CUSTOM_MASK) != uint64_t(CustomCommand::PASS))) {
                uint32_t index = (first->key & CUSTOM_INDEX_MASK) >> CUSTOM_INDEX_SHIFT;
                customCommands[index]();
                continue;
            }

            // per-renderable uniform
            const PrimitiveInfo info = first->primitive;
            pipeline.rasterState = info.rasterState;
            if (UTILS_UNLIKELY(mi != info.mi)) {
                // this is always taken the first time
                mi = info.mi;
                ma = mi->getMaterial();
                pipeline.scissor = mi->getScissor();
                *pPipelinePolygonOffset = mi->getPolygonOffset();
                mi->use(driver);
            }

            pipeline.program = ma->getProgram(info.materialVariant.key);
            size_t offset = info.index * sizeof(PerRenderableUib);
            driver.bindUniformBufferRange(BindingPoints::PER_RENDERABLE,
                    uboHandle, offset, sizeof(PerRenderableUib));
            if (UTILS_UNLIKELY(info.perRenderableBones)) {
                driver.bindUniformBuffer(BindingPoints::PER_RENDERABLE_BONES,
                        info.perRenderableBones);
            }
            driver.draw(pipeline, info.primitiveHandle);
        }
        mCustomCommands.clear();
    }
}

/* static */
UTILS_ALWAYS_INLINE // this function exists only to make the code more readable. we want it inlined.
inline              // and we don't need it in the compilation unit
void RenderPass::setupColorCommand(Command& cmdDraw, bool hasDepthPass,
        FMaterialInstance const* const UTILS_RESTRICT mi, bool inverseFrontFaces) noexcept {

    FMaterial const * const UTILS_RESTRICT ma = mi->getMaterial();
    uint8_t variant =
            Variant::filterVariant(cmdDraw.primitive.materialVariant.key, ma->isVariantLit());

    // Below, we evaluate both commands to avoid a branch

    uint64_t keyBlending = cmdDraw.key;
    keyBlending &= ~(PASS_MASK | BLENDING_MASK);
    keyBlending |= uint64_t(Pass::BLENDED);
    keyBlending |= uint64_t(CustomCommand::PASS);

    bool hasScreenSpaceRefraction = ma->getRefractionMode() == RefractionMode::SCREEN_SPACE;
    bool hasBlending = !hasScreenSpaceRefraction && ma->getRasterState().hasBlending();

    uint64_t keyDraw = cmdDraw.key;
    keyDraw &= ~(PASS_MASK | BLENDING_MASK | MATERIAL_MASK);
    keyDraw |= uint64_t(hasScreenSpaceRefraction ? Pass::REFRACT : Pass::COLOR);
    keyDraw |= uint64_t(CustomCommand::PASS);
    keyDraw |= mi->getSortingKey(); // already all set-up for direct or'ing
    keyDraw |= makeField(variant, MATERIAL_VARIANT_KEY_MASK, MATERIAL_VARIANT_KEY_SHIFT);
    keyDraw |= makeField(ma->getRasterState().alphaToCoverage, BLENDING_MASK, BLENDING_SHIFT);

    cmdDraw.key = hasBlending ? keyBlending : keyDraw;
    cmdDraw.primitive.rasterState = ma->getRasterState();
    cmdDraw.primitive.rasterState.inverseFrontFaces = inverseFrontFaces;
    cmdDraw.primitive.rasterState.culling = mi->getCullingMode();
    cmdDraw.primitive.rasterState.colorWrite = mi->getColorWrite();
    cmdDraw.primitive.rasterState.depthWrite = mi->getDepthWrite();
    cmdDraw.primitive.rasterState.depthFunc = mi->getDepthFunc();
    cmdDraw.primitive.mi = mi;
    cmdDraw.primitive.materialVariant.key = variant;

    // Code below is branch-less with clang.

    bool skipDepthWrite = hasDepthPass &
            cmdDraw.primitive.rasterState.depthWrite &
            ~(cmdDraw.primitive.rasterState.alphaToCoverage | hasBlending);

    // If we have:
    //      depth-prepass AND
    //      depth-write is enabled AND
    //      we're not doing alpha-to-coverage AND
    //      we're not alpha blending
    // THEN, we deactivate depth write (because it'll be done by the depth-prepass)
    cmdDraw.primitive.rasterState.depthWrite =
            skipDepthWrite ? false : cmdDraw.primitive.rasterState.depthWrite;

    // we keep "RasterState::colorWrite" to the value set by material (could be disabled)
}

/* static */
UTILS_NOINLINE
void RenderPass::generateCommands(uint32_t commandTypeFlags, Command* const commands,
        FScene::RenderableSoa const& soa, Range<uint32_t> range, RenderFlags renderFlags,
        FScene::VisibleMaskType visibilityMask, float3 cameraPosition, float3 cameraForward) noexcept {

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    // compute how much maximum storage we need
    uint32_t offset = FScene::getPrimitiveCount(soa, range.first);
    // double the color pass for transparents that need to render twice
    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);
    offset *= uint32_t(colorPass * 2 + depthPass);
    Command* const curr = commands + offset;

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
            generateCommandsImpl<CommandTypeFlags::COLOR>(commandTypeFlags, curr,
                    soa, range, renderFlags, visibilityMask, cameraPosition, cameraForward);
            break;
        case CommandTypeFlags::DEPTH:
            generateCommandsImpl<CommandTypeFlags::DEPTH>(commandTypeFlags, curr,
                    soa, range, renderFlags, visibilityMask, cameraPosition, cameraForward);
            break;
        default:
            // we should never end-up here
            break;
    }
}

/* static */
template<uint32_t commandTypeFlags>
UTILS_NOINLINE
void RenderPass::generateCommandsImpl(uint32_t extraFlags,
        Command* UTILS_RESTRICT curr,
        FScene::RenderableSoa const& UTILS_RESTRICT soa, Range<uint32_t> range,
        RenderFlags renderFlags, FScene::VisibleMaskType visibilityMask,
        float3 cameraPosition, float3 cameraForward) noexcept {

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & CommandTypeFlags::DEPTH);
    const bool depthContainsShadowCasters = bool(extraFlags & CommandTypeFlags::DEPTH_CONTAINS_SHADOW_CASTERS);
    const bool depthFilterTranslucentObjects = bool(extraFlags & CommandTypeFlags::DEPTH_FILTER_TRANSLUCENT_OBJECTS);
    const bool depthFilterAlphaMaskedObjects = bool(extraFlags & CommandTypeFlags::DEPTH_FILTER_ALPHA_MASKED_OBJECTS);

    auto const* const UTILS_RESTRICT soaWorldAABBCenter = soa.data<FScene::WORLD_AABB_CENTER>();
    auto const* const UTILS_RESTRICT soaReversedWinding = soa.data<FScene::REVERSED_WINDING_ORDER>();
    auto const* const UTILS_RESTRICT soaVisibility      = soa.data<FScene::VISIBILITY_STATE>();
    auto const* const UTILS_RESTRICT soaPrimitives      = soa.data<FScene::PRIMITIVES>();
    auto const* const UTILS_RESTRICT soaBonesUbh        = soa.data<FScene::BONES_UBH>();
    auto const* const UTILS_RESTRICT soaVisibilityMask  = soa.data<FScene::VISIBLE_MASK>();

    const bool hasShadowing = renderFlags & HAS_SHADOWING;
    const bool viewInverseFrontFaces = renderFlags & HAS_INVERSE_FRONT_FACES;

    Variant materialVariant;
    materialVariant.setDirectionalLighting(renderFlags & HAS_DIRECTIONAL_LIGHT);
    materialVariant.setDynamicLighting(renderFlags & HAS_DYNAMIC_LIGHTING);
    materialVariant.setFog(renderFlags & HAS_FOG);
    materialVariant.setShadowReceiver(false); // this is set per Renderable

    Command cmdColor;

    Command cmdDepth;
    cmdDepth.primitive.materialVariant = Variant{ Variant::DEPTH_VARIANT };
    cmdDepth.primitive.rasterState = {};
    cmdDepth.primitive.rasterState.colorWrite = false;
    cmdDepth.primitive.rasterState.depthWrite = true;
    cmdDepth.primitive.rasterState.depthFunc = RasterState::DepthFunc::L;
    cmdDepth.primitive.rasterState.alphaToCoverage = false;

    for (uint32_t i = range.first; i < range.last; ++i) {
        // Check if this renderable passes the visibilityMask. If it doesn't, encode SENTINEL
        // commands (no-op).
        if (UTILS_UNLIKELY(!(soaVisibilityMask[i] & visibilityMask))) {
            // We need to encode a SENTINEL for each command that would have been generated
            // otherwise. Color passes get 2 commands per primitive; depth passes get 1.
            const Slice<FRenderPrimitive>& primitives = soaPrimitives[i];
            const size_t commandsToEncode = (colorPass * 2 + depthPass) * primitives.size();
            for (size_t j = 0; j < commandsToEncode; j++) {
                curr->key = uint64_t(Pass::SENTINEL);
                ++curr;
            }
            continue;
        }

        // Signed distance from camera to object's center. Positive distances are in front of
        // the camera. Some objects with a center behind the camera can still be visible
        // so their distance will be negative (this happens a lot for the shadow map).

        // Using the center is not very good with large AABBs. Instead we can try to use
        // the closest point on the bounding sphere instead:
        //      d = soaWorldAABBCenter[i] - cameraPosition;
        //      d -= normalize(d) * length(soaWorldAABB[i].halfExtent);
        // However this doesn't work well at all for large planes.

        // Code below is equivalent to:
        // float3 d = soaWorldAABBCenter[i] - cameraPosition;
        // float distance = dot(d, cameraForward);
        // but saves a couple of instruction, because part of the math is done outside of the loop.
        float distance = dot(soaWorldAABBCenter[i], cameraForward) - dot(cameraPosition, cameraForward);


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
        const bool inverseFrontFaces = viewInverseFrontFaces ^ soaReversedWinding[i];

        cmdColor.key = makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmdColor.primitive.index = (uint16_t)i;
        cmdColor.primitive.perRenderableBones = soaBonesUbh[i];
        materialVariant.setShadowReceiver(soaVisibility[i].receiveShadows & hasShadowing);
        materialVariant.setSkinning(soaVisibility[i].skinning || soaVisibility[i].morphing);

        // we're assuming we're always doing the depth (either way, it's correct)
        // this will generate front to back rendering
        cmdDepth.key = uint64_t(Pass::DEPTH);
        cmdDepth.key |= uint64_t(CustomCommand::PASS);
        cmdDepth.key |= makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmdDepth.key |= makeField(distanceBits, DISTANCE_BITS_MASK, DISTANCE_BITS_SHIFT);
        cmdDepth.primitive.index = (uint16_t)i;
        cmdDepth.primitive.perRenderableBones = soaBonesUbh[i];
        cmdDepth.primitive.materialVariant.setSkinning(soaVisibility[i].skinning || soaVisibility[i].morphing);
        cmdDepth.primitive.rasterState.inverseFrontFaces = inverseFrontFaces;

        const bool shadowCaster = soaVisibility[i].castShadows & hasShadowing;
        const bool writeDepthForShadowCasters = depthContainsShadowCasters & shadowCaster;

        const Slice<FRenderPrimitive>& primitives = soaPrimitives[i];

        /*
         * This is our hot loop. It's written to avoid branches.
         * When modifying this code, always ensure it stays efficient.
         */
        for (auto const& primitive : primitives) {
            FMaterialInstance const* const mi = primitive.getMaterialInstance();
            if (colorPass) {
                cmdColor.primitive.primitiveHandle = primitive.getHwHandle();
                cmdColor.primitive.materialVariant = materialVariant;
                RenderPass::setupColorCommand(cmdColor, depthPass, mi, inverseFrontFaces);

                const bool blendPass = Pass(cmdColor.key & PASS_MASK) == Pass::BLENDED;
                if (blendPass) {
                    // TODO: at least for transparent objects, AABB should be per primitive
                    // blend pass:
                    // this will sort back-to-front for blended, and honor explicit ordering
                    // for a given Z value
                    cmdColor.key &= ~BLEND_ORDER_MASK;
                    cmdColor.key &= ~BLEND_DISTANCE_MASK;
                    cmdColor.key |= makeField(~distanceBits,
                            BLEND_DISTANCE_MASK, BLEND_DISTANCE_SHIFT);
                    cmdColor.key |= makeField(primitive.getBlendOrder(),
                            BLEND_ORDER_MASK, BLEND_ORDER_SHIFT);

                    const TransparencyMode mode = mi->getMaterial()->getTransparencyMode();

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

                    // handle the case where this primitive is empty / no-op
                    key |= select(primitive.getPrimitiveType() == PrimitiveType::NONE);

                    // correct for TransparencyMode::DEFAULT -- i.e. cancel the command
                    key |= select(mode == TransparencyMode::DEFAULT);

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
                            SamplerCompareFunc::LE : cmdColor.primitive.rasterState.depthFunc;
                } else {
                    // color pass:
                    // This will bucket objects by Z, front-to-back and then sort by material
                    // in each buckets. We use the top 10 bits of the distance, which
                    // bucketizes the depth by its log2 and in 4 linear chunks in each bucket.
                    cmdColor.key &= ~Z_BUCKET_MASK;
                    cmdColor.key |= makeField(distanceBits >> 22u, Z_BUCKET_MASK,
                            Z_BUCKET_SHIFT);

                    curr->key = uint64_t(Pass::SENTINEL);
                    ++curr;
                }

                *curr = cmdColor;
                // handle the case where this primitive is empty / no-op
                curr->key |= select(primitive.getPrimitiveType() == PrimitiveType::NONE);
                ++curr;
            }

            if (depthPass) {
                RasterState rs = mi->getMaterial()->getRasterState();

                // unconditionally write the command
                cmdDepth.primitive.primitiveHandle = primitive.getHwHandle();
                cmdDepth.primitive.mi = mi;
                cmdDepth.primitive.rasterState.culling = mi->getCullingMode();
                *curr = cmdDepth;

                // FIXME: should writeDepthForShadowCasters take precedence over rs.depthWrite?
                bool issueDepth = (rs.depthWrite
                        & !(depthFilterTranslucentObjects & rs.hasBlending())
                        & !(depthFilterAlphaMaskedObjects & rs.alphaToCoverage))
                                | writeDepthForShadowCasters;

                curr->key |= select(!issueDepth);

                // handle the case where this primitive is empty / no-op
                curr->key |= select(primitive.getPrimitiveType() == PrimitiveType::NONE);
                ++curr;
            }
        }
    }
}

void RenderPass::updateSummedPrimitiveCounts(
        FScene::RenderableSoa& renderableData, Range<uint32_t> vr) noexcept {
    auto const* const UTILS_RESTRICT primitives = renderableData.data<FScene::PRIMITIVES>();
    uint32_t* const UTILS_RESTRICT summedPrimitiveCount = renderableData.data<FScene::SUMMED_PRIMITIVE_COUNT>();
    uint32_t count = 0;
    for (uint32_t i : vr) {
        summedPrimitiveCount[i] = count;
        count += primitives[i].size();
    }
    // we're guaranteed to have enough space at the end of vr
    summedPrimitiveCount[vr.last] = count;
}

} // namespace filament
