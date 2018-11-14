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

#include "details/Culler.h"
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

using namespace utils;
using namespace math;

namespace filament {

using namespace driver;

namespace details {

RenderPass::~RenderPass() noexcept = default;

UTILS_ALWAYS_INLINE // this allows the compiler to devirtualize some calls
inline              // this removes the code from the compilation unit
void RenderPass::render(
        FEngine& engine, JobSystem& js,
        FScene& scene, Range<uint32_t> vr,
        uint32_t commandTypeFlags, RenderFlags renderFlags,
        const CameraInfo& camera, Viewport const& viewport,
        GrowingSlice<Command>& commands) noexcept {

    SYSTRACE_CONTEXT();

    // trace the number of visible renderables
    SYSTRACE_VALUE32("visibleRenderables", vr.size());

    FScene::RenderableSoa const& soa = scene.getRenderableData();

    // up-to-date summed primitive counts needed for generateCommands()
    updateSummedPrimitiveCounts(const_cast<FScene::RenderableSoa&>(soa), vr);

    // compute how much maximum storage we need for this pass
    uint32_t growBy = FScene::getPrimitiveCount(soa, vr.last);
    // double the color pass for transparent objects that need to render twice
    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & (CommandTypeFlags::DEPTH | CommandTypeFlags::SHADOW));
    growBy *= uint32_t(colorPass * 2 + depthPass);
    Command* const curr = commands.grow(growBy);

    // we extract camera position/forward outside of the loop, because these are not cheap.
    const float3 cameraPosition(camera.getPosition());
    const float3 cameraForwardVector(camera.getForwardVector());
    auto work = [commandTypeFlags, curr, &soa, renderFlags, cameraPosition, cameraForwardVector]
            (uint32_t startIndex, uint32_t indexCount) {
        RenderPass::generateCommands(commandTypeFlags, curr,
                soa, { startIndex, startIndex + indexCount }, renderFlags,
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

    { // sort all commands
        SYSTRACE_NAME("sort commands");
        std::sort(commands.begin(), commands.end());
    }

    // Take care not to upload data within the render pass (synchronize can commit froxel data)
    driver::DriverApi& driver = engine.getDriverApi();
    beginRenderPass(driver, viewport, camera);

    // Now, execute all commands
    RenderPass::recordDriverCommands(driver, scene, commands);

    endRenderPass(driver, viewport);

    // Kick the GPU since we're done with this render target
    driver.flush();
    // Wake-up the driver thread
    engine.flush();
}

UTILS_NOINLINE // no need to be inlined
void RenderPass::recordDriverCommands(
        FEngine::DriverApi& UTILS_RESTRICT driver,  // using restrict here is very important
        FScene& UTILS_RESTRICT scene,
        Slice<Command> const& commands) noexcept {
    SYSTRACE_CALL();

    if (!commands.empty()) {
        Driver::PipelineState pipeline;
        Handle<HwUniformBuffer> uboHandle = scene.getRenderableUBO();
        FMaterialInstance const* UTILS_RESTRICT mi = nullptr;
        FMaterial const* UTILS_RESTRICT ma = nullptr;
        Command const* UTILS_RESTRICT c;
        for (c = commands.cbegin(); c->key != -1LLU; ++c) {
            /*
             * Be careful when changing code below, this is the hot inner-loop
             */

            // per-renderable uniform
            const PrimitiveInfo info = c->primitive;
            pipeline.rasterState = info.rasterState;
            if (UTILS_UNLIKELY(mi != info.mi)) {
                // this is always taken the first time
                mi = info.mi;
                pipeline.polygonOffset = mi->getPolygonOffset();
                ma = mi->getMaterial();
                mi->use(driver);
            }

            pipeline.program = ma->getProgram(info.materialVariant.key);
            size_t offset = info.index * sizeof(PerRenderableUib);
            if (info.perRenderableBones) {
                driver.bindUniformBuffer(BindingPoints::PER_RENDERABLE_BONES, info.perRenderableBones);
            }
            driver.bindUniformBufferRange(BindingPoints::PER_RENDERABLE, uboHandle, offset, sizeof(PerRenderableUib));
            driver.draw(pipeline, info.primitiveHandle);
        }

        SYSTRACE_VALUE32("commandCount", c - commands.cbegin());
    }
}

/* static */
UTILS_ALWAYS_INLINE // this function exists only to make the code more readable. we want it inlined.
inline              // and we don't need it in the compilation unit
void RenderPass::setupColorCommand(Command& cmdDraw, bool hasDepthPass,
        FMaterialInstance const* const UTILS_RESTRICT mi) noexcept {

    FMaterial const * const UTILS_RESTRICT ma = mi->getMaterial();
    uint8_t variant =
            Variant::filterVariant(cmdDraw.primitive.materialVariant.key, ma->isVariantLit());

    // Below, we evaluate both commands to avoid a branch

    uint64_t keyBlending = cmdDraw.key;
    keyBlending &= ~(PASS_MASK | BLENDING_MASK);
    keyBlending |= uint64_t(Pass::BLENDED);
    keyBlending |= makeField(ma->getBlendingMode(), BLENDING_MASK, BLENDING_SHIFT);

    uint64_t keyDraw = cmdDraw.key;
    keyDraw &= ~(PASS_MASK | BLENDING_MASK | MATERIAL_MASK);
    keyDraw |= uint64_t(Pass::COLOR);
    keyDraw |= mi->getSortingKey(); // already all set-up for direct or'ing
    keyDraw |= makeField(variant, MATERIAL_VARIANT_KEY_MASK, MATERIAL_VARIANT_KEY_SHIFT);
    keyDraw |= makeField(ma->getRasterState().alphaToCoverage, BLENDING_MASK, BLENDING_SHIFT);

    bool hasBlending = ma->getRasterState().hasBlending();
    cmdDraw.key = hasBlending ? keyBlending : keyDraw;
    cmdDraw.primitive.rasterState = ma->getRasterState();
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
        FScene::RenderableSoa const& soa, utils::Range<uint32_t> range, RenderFlags renderFlags,
        math::float3 cameraPosition, math::float3 cameraForward) noexcept {

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    // compute how much maximum storage we need
    uint32_t offset = FScene::getPrimitiveCount(soa, range.first);
    // double the color pass for transparents that need to render twice
    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & (CommandTypeFlags::DEPTH | CommandTypeFlags::SHADOW));
    offset *= uint32_t(colorPass * 2 + depthPass);
    Command* const curr = commands + offset;

    /*
     *
     * The if {} below is to coerce the compiler into generating different versions of
     * "generateCommandsImpl" based on which pass we're processing.
     *
     *  We use a template function (as opposed to just inlining), so that the compiler is
     *  able to generate actual separate versions of generateCommandsImpl<>, which is much
     *  easier to debug and doesn't impact performance (it's just a predicted jump).
     *
     *  We also use a "dummy" first parameter in generateCommandsImpl<> so that the compiler
     *  doesn't have to shuffle all registers, when calling (in case it doesn't inline the call
     *  -- which, as stated above is fine and even preferable).
     *  But we actually pass "commandTypeFlags" (doesn't matter, since it's unused), which saves
     *  a few instructions for no cost to us.
     */

    switch (commandTypeFlags) {
        default: // squash IDE warning -- should never happen.
        case CommandTypeFlags::COLOR:
            generateCommandsImpl<CommandTypeFlags::COLOR>(commandTypeFlags, curr,
                    soa, range, renderFlags, cameraPosition, cameraForward);
            break;
        case CommandTypeFlags::DEPTH_AND_COLOR:
            generateCommandsImpl<CommandTypeFlags::DEPTH_AND_COLOR>(commandTypeFlags, curr,
                    soa, range, renderFlags, cameraPosition, cameraForward);
            break;
        case CommandTypeFlags::SHADOW:
            generateCommandsImpl<CommandTypeFlags::SHADOW>(commandTypeFlags, curr,
                    soa, range, renderFlags, cameraPosition, cameraForward);
            break;
    }
}

/* static */
template<uint32_t commandTypeFlags>
UTILS_NOINLINE
void RenderPass::generateCommandsImpl(uint32_t,
        Command* UTILS_RESTRICT curr,
        FScene::RenderableSoa const& UTILS_RESTRICT soa, utils::Range<uint32_t> range,
        RenderFlags renderFlags,
        float3 cameraPosition, float3 cameraForward) noexcept {

    // generateCommands() writes both the draw and depth commands simultaneously such that
    // we go throw the list of renderables just once.
    // (in principle, we could have split this method into two, at the cost of going through
    // the list twice)

    const bool colorPass  = bool(commandTypeFlags & CommandTypeFlags::COLOR);
    const bool depthPass  = bool(commandTypeFlags & (CommandTypeFlags::DEPTH | CommandTypeFlags::SHADOW));
    const bool shadowPass = bool(commandTypeFlags & CommandTypeFlags::SHADOW);

    auto const* const UTILS_RESTRICT soaWorldAABBCenter = soa.data<FScene::WORLD_AABB_CENTER>();
    auto const* const UTILS_RESTRICT soaVisibility      = soa.data<FScene::VISIBILITY_STATE>();
    auto const* const UTILS_RESTRICT soaPrimitives      = soa.data<FScene::PRIMITIVES>();
    auto const* const UTILS_RESTRICT soaBonesUbh        = soa.data<FScene::BONES_UBH>();

    const bool hasShadowing = renderFlags & HAS_SHADOWING;
    Variant materialVariant;
    materialVariant.setDirectionalLighting(renderFlags & HAS_DIRECTIONAL_LIGHT);
    materialVariant.setDynamicLighting(renderFlags & HAS_DYNAMIC_LIGHTING);
    materialVariant.setShadowReceiver(false); // this is set per Renderable

    Command cmdColor;

    Command cmdDepth;
    cmdDepth.primitive.materialVariant = { Variant::DEPTH_VARIANT };
    cmdDepth.primitive.rasterState = Driver::RasterState();
    cmdDepth.primitive.rasterState.colorWrite = false;
    cmdDepth.primitive.rasterState.depthWrite = true;
    cmdDepth.primitive.rasterState.depthFunc = Driver::RasterState::DepthFunc::L;
    cmdDepth.primitive.rasterState.alphaToCoverage = false;

    for (uint32_t i = range.first; i < range.last; ++i) {
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

        cmdColor.key = makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmdColor.primitive.index = (uint16_t)i;
        cmdColor.primitive.perRenderableBones = soaBonesUbh[i];
        materialVariant.setShadowReceiver(soaVisibility[i].receiveShadows & hasShadowing);
        materialVariant.setSkinning(soaVisibility[i].skinning);

        // we're assuming we're always doing the depth (either way, it's correct)
        // this will generate front to back rendering
        cmdDepth.key = uint64_t(Pass::DEPTH);
        cmdDepth.key |= makeField(soaVisibility[i].priority, PRIORITY_MASK, PRIORITY_SHIFT);
        cmdDepth.key |= makeField(distanceBits, DISTANCE_BITS_MASK, DISTANCE_BITS_SHIFT);
        cmdDepth.primitive.index = (uint16_t)i;
        cmdDepth.primitive.perRenderableBones = soaBonesUbh[i];
        cmdDepth.primitive.materialVariant.setSkinning(soaVisibility[i].skinning);

        const bool shadowCaster = soaVisibility[i].castShadows & hasShadowing;
        const bool writeDepthForShadows = shadowPass & shadowCaster;

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
                RenderPass::setupColorCommand(cmdColor, depthPass, mi);

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
                    // color pass, opaque objects...
                    if (!depthPass) {
                        // ...without depth pre-pass:
                        // this will bucket objects by Z, front-to-back and then sort by material
                        // in each buckets. We use the top 10 bits of the distance, which
                        // bucketizes the depth by its log2 and in 4 linear chunks in each bucket.
                        cmdColor.key &= ~Z_BUCKET_MASK;
                        cmdColor.key |= makeField(distanceBits >> 22, Z_BUCKET_MASK,
                                Z_BUCKET_SHIFT);
                    }
                    // ...with depth pre-pass, we just sort by materials
                    curr->key = uint64_t(Pass::SENTINEL);
                    ++curr;
                }

                *curr = cmdColor;
                // handle the case where this primitive is empty / no-op
                curr->key |= select(primitive.getPrimitiveType() == PrimitiveType::NONE);
                ++curr;
            }

            if (depthPass) {
                Driver::RasterState rs = mi->getMaterial()->getRasterState();

                // unconditionally write the command
                cmdDepth.primitive.primitiveHandle = primitive.getHwHandle();
                cmdDepth.primitive.mi = mi;
                cmdDepth.primitive.rasterState.culling = rs.culling;
                *curr = cmdDepth;

                // If we are drawing depth+draw we don't want to put commands using
                // alpha testing (indicated by the alpha to coverage flag) or blending in the
                // depth prepass. What we do want is put those commands in the shadow map
                // (when only drawDepth is true).
                // Also, depth-write could be disabled by the material,
                // in this case undo the command.
                bool issueDepth =
                        (rs.depthWrite & !(colorPass & (rs.alphaToCoverage | rs.hasBlending())))
                        | writeDepthForShadows;
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

// ------------------------------------------------------------------------------------------------
// FRenderer concrete implementations are defined here so that we can benefit from
// inlining and devirtualization.
// ------------------------------------------------------------------------------------------------

FRenderer::ColorPass::ColorPass(const char* name,
        JobSystem& js, JobSystem::Job* jobFroxelize, FView& view, Handle<HwRenderTarget> const rth)
        : RenderPass(name), js(js), jobFroxelize(jobFroxelize), view(view), rth(rth) {
}

void FRenderer::ColorPass::beginRenderPass(
        driver::DriverApi& driver, Viewport const& viewport, const CameraInfo& camera) noexcept {
    // wait for froxelization to finish
    // (this could even be a special command between the depth and color passes)
    js.waitAndRelease(jobFroxelize);
    view.commitFroxels(driver);

    // We won't need the depth or stencil buffers after this pass.
    RenderPassParams params = {};
    params.discardEnd = TargetBufferFlags::DEPTH_AND_STENCIL;
    params.left = viewport.left;
    params.bottom = viewport.bottom;
    params.width = viewport.width;
    params.height = viewport.height;
    params.clearColor = view.getClearColor();
    params.clearDepth = 1.0;

    if (view.hasPostProcessPass()) {
        // When using a post-process pass, composition of Views is done during the post-process
        // pass, which means it's NOT done here. For this reason, we need to clear the depth/stencil
        // buffers unconditionally. The color buffer must be cleared to what the user asked for,
        // since it's akin to a drawing command.
        // Also, all buffers can be invalidated before rendering.
        if (view.getClearTargetColor()) {
            params.clear = TargetBufferFlags::ALL;
        } else {
            params.clear = TargetBufferFlags::DEPTH_AND_STENCIL;
        }
        params.discardStart = TargetBufferFlags::ALL;
        driver.beginRenderPass(rth, params);
    } else {
        params.discardStart = view.getDiscardedTargetBuffers();
        if (view.getClearTargetColor()) {
            params.clear |= TargetBufferFlags::COLOR;
        }
        if (view.getClearTargetDepth()) {
            params.clear |= TargetBufferFlags::DEPTH;
        }
        if (view.getClearTargetStencil()) {
            params.clear |= TargetBufferFlags::STENCIL;
        }
        driver.beginRenderPass(rth, params);
    }
}

void FRenderer::ColorPass::endRenderPass(DriverApi& driver, Viewport const& viewport) noexcept {
    driver.endRenderPass();

    // and we don't need the color buffer in the areas we don't use
    if (view.hasPostProcessPass()) {
        // discard parts of the color buffer we didn't render into
        const uint32_t large = std::numeric_limits<uint16_t>::max();
        driver.discardSubRenderTargetBuffers(rth, TargetBufferFlags::COLOR,
                0, viewport.height, large, large);          // top side
        driver.discardSubRenderTargetBuffers(rth, TargetBufferFlags::COLOR,
                viewport.width, 0, large, viewport.height); // right side
    }
}

void FRenderer::ColorPass::renderColorPass(FEngine& engine, JobSystem& js,
        Handle<HwRenderTarget> const rth, FView& view, Viewport const& scaledViewport,
        GrowingSlice<Command>& commands) noexcept {

    // start the froxelization immediately, it has no dependencies
    JobSystem::Job* jobFroxelize = js.createJob(nullptr,
            [&engine, &view](JobSystem&, JobSystem::Job*) { view.froxelize(engine); });
    jobFroxelize = js.runAndRetain(jobFroxelize);

    CameraInfo const& cameraInfo = view.getCameraInfo();
    auto& soa = view.getScene()->getRenderableData();
    auto vr = view.getVisibleRenderables();

    // populate the RenderPrimitive array with the proper LOD
    view.updatePrimitivesLod(engine, cameraInfo, soa, vr);

    DriverApi& driver = engine.getDriverApi();
    view.prepareCamera(cameraInfo, scaledViewport);
    view.commitUniforms(driver);

    RenderPass::RenderFlags flags = 0;
    if (view.hasShadowing())           flags |= RenderPass::HAS_SHADOWING;
    if (view.hasDirectionalLight())    flags |= RenderPass::HAS_DIRECTIONAL_LIGHT;
    if (view.hasDynamicLighting())     flags |= RenderPass::HAS_DYNAMIC_LIGHTING;

    CommandTypeFlags commandType;
    switch (view.getDepthPrepass()) {
        case View::DepthPrepass::DEFAULT:
            // TODO: better default strategy (can even change on a per-frame basis)
#if defined(ANDROID) || defined(__EMSCRIPTEN__)
            commandType = COLOR;
#else
            commandType = DEPTH_AND_COLOR;
#endif
            break;
        case View::DepthPrepass::DISABLED:
            commandType = COLOR;
            break;
        case View::DepthPrepass::ENABLED:
            commandType = DEPTH_AND_COLOR;
            break;
    }

    ColorPass colorPass("ColorPass", js, jobFroxelize, view, rth);
    driver.pushGroupMarker("Color Pass");
    colorPass.render(engine, js, *view.getScene(), vr, commandType, flags, cameraInfo, scaledViewport, commands);
    driver.popGroupMarker();
}

// ------------------------------------------------------------------------------------------------

FRenderer::ShadowPass::ShadowPass(const char* name,
        ShadowMap const& shadowMap) noexcept
        : RenderPass(name), shadowMap(shadowMap) {
}

void FRenderer::ShadowPass::beginRenderPass(driver::DriverApi& driver, Viewport const&, const CameraInfo&) noexcept {
    shadowMap.beginRenderPass(driver);
}

void FRenderer::ShadowPass::renderShadowMap(FEngine& engine, JobSystem& js,
        FView& view, GrowingSlice<Command>& commands) noexcept {

    auto& soa = view.getScene()->getRenderableData();
    auto vr = view.getVisibleShadowCasters();
    ShadowMap const& shadowMap = view.getShadowMap();
    Viewport const& viewport = shadowMap.getViewport();
    FCamera const& camera = shadowMap.getCamera();

    CameraInfo cameraInfo = {
            .projection         = mat4f{ camera.getProjectionMatrix() },
            .cullingProjection  = mat4f{ camera.getCullingProjectionMatrix() },
            .model              = camera.getModelMatrix(),
            .view               = camera.getViewMatrix(),
            .zn                 = camera.getNear(),
            .zf                 = camera.getCullingFar(),
    };

    // populate the RenderPrimitive array with the proper LOD
    view.updatePrimitivesLod(engine, cameraInfo, soa, vr);

    driver::DriverApi& driver = engine.getDriverApi();
    view.prepareCamera(cameraInfo, viewport);
    view.commitUniforms(driver);

    RenderPass::RenderFlags flags = 0;
    if (view.hasShadowing())           flags |= RenderPass::HAS_SHADOWING;
    if (view.hasDirectionalLight())    flags |= RenderPass::HAS_DIRECTIONAL_LIGHT;
    if (view.hasDynamicLighting())     flags |= RenderPass::HAS_DYNAMIC_LIGHTING;

    ShadowPass shadowPass("ShadowPass", shadowMap);
    driver.pushGroupMarker("Shadow map Pass");
    shadowPass.render(engine, js, *view.getScene(), vr, CommandTypeFlags::SHADOW, flags, cameraInfo, viewport, commands);
    driver.popGroupMarker();
}

void FRenderer::ShadowPass::endRenderPass(DriverApi& driver, Viewport const& viewport) noexcept {
    driver.endRenderPass();
}

} // namespace details
} // namespace filament
