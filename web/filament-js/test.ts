/*
* Copyright (C) 2019 The Android Open Source Project
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

/*
 * This file exists only to test the buildability of our TypeScript annotations. To run the test,
 * invoke Filament's easy build script with "./build.sh -up wasm release", or do:
 *
 * npx tsc --noEmit \
 *     ../../third_party/gl-matrix/gl-matrix.d.ts \
 *     ./filament.d.ts \
 *     test.ts
 *
 * Note that it suffices to simply build the test, it is not meant to be executed.
 *
 * Organization: each smoke_* function exercises one subsystem (one embind class or a tightly
 * related cluster) and is registered in SMOKE_TESTS at the bottom. tsc validates each function
 * body's calls against the declarations in filament.d.ts. To cover a new binding, add a call to
 * the matching subsystem function (or write a new smoke_* beside its peers and register it) so
 * the typings can never silently drift from the bindings again.
 */

import * as Filament from "./filament";
import * as glm from "gl-matrix";

// Shared math fixtures reused across the smoke tests.
const v2 = glm.vec2.create();
const v3 = glm.vec3.create();
const v4 = glm.vec4.create();
const m3 = glm.mat3.create();
const m4 = glm.mat4.create();
const qt = glm.quat.create();

const canvas = new HTMLCanvasElement();
const engine = Filament.Engine.create(canvas);

function newEntity(): Filament.Entity {
    return Filament.EntityManager.get().create();
}

// ---------------------------------------------------------------------------
// Core engine objects
// ---------------------------------------------------------------------------

function smoke_entity_manager() {
    const em = Filament.EntityManager.get();
    const entity: Filament.Entity = em.create();
    const id: number = entity.getId();
    // getActiveEntityCount is only compiled in when FILAMENT_UTILS_TRACK_ENTITIES is set, which
    // the release wasm build does not do, so callers must feature-detect it.
    const activeEntities: number = em.getActiveEntityCount();
    const entityCount: number = em.getEntityCount();
    const maxEntities: number = Filament.EntityManager.getMaxEntityCount();
    em.advanceEpoch();
    em.destroy(entity);
    console.log(id, activeEntities, entityCount, maxEntities);
}

function smoke_engine() {
    const backend: Filament.Backend = engine.getBackend();
    const em: Filament.EntityManager = engine.getEntityManager();
    const fl: Filament.FeatureLevel = engine.getActiveFeatureLevel();
    engine.setActiveFeatureLevel(Filament.FeatureLevel.FEATURE_LEVEL_1);
    const supported: Filament.FeatureLevel = engine.getSupportedFeatureLevel();
    engine.setAutomaticInstancingEnabled(true);
    const instancing: boolean = engine.isAutomaticInstancingEnabled();
    engine.enableAccurateTranslations();
    engine.unprotected();
    const fence: Filament.Fence = engine.createFence();
    const validFence: boolean = engine.isValidFence(fence);
    const status: Filament.FenceStatus = fence.wait(Filament.Fence$Mode.FLUSH, 0);
    engine.destroyFence(fence);
    console.log(validFence, status);
    engine.flush();
    engine.flushAndWait();
    const failed: boolean = engine.hasUnrecoverableFailure();
    const maxEyes: number = Filament.Engine.getMaxStereoscopicEyes();
    const now: number = Filament.Engine.getSteadyClockTimeNano();
    const config: Filament.Engine$Config = engine.getConfig();
    // The same feature flags Engine$Builder.feature sets before construction, after it.
    const flagSet: boolean = engine.setFeatureFlag("backend.disable_parallel_shader_compile", true);
    const hasFlag: boolean = engine.hasFeatureFlag("backend.disable_parallel_shader_compile");
    const flag: boolean|undefined =
            engine.getFeatureFlag("backend.disable_parallel_shader_compile");
    console.log(flagSet, hasFlag, flag);
    engine.execute();
    console.log(backend, em, fl, supported, instancing, failed, maxEyes, now, config);
}

// Every object the Engine owns, created and then handed back to the matching destroy overload.
function smoke_engine_lifecycle() {
    const cameraEntity = newEntity();
    engine.createCamera(cameraEntity);
    const camera: Filament.Camera = engine.getCameraComponent(cameraEntity);
    const material = engine.createMaterial("nonexistent.filamat");
    const matinst = material.createInstance();
    const expensiveCheck: boolean = engine.isValidExpensiveMaterialInstance(matinst);
    const texture = Filament.Texture.Builder().width(1).height(1).build(engine);
    const rt = Filament.RenderTarget.Builder().build(engine);
    const colorGrading = Filament.ColorGrading.Builder().build(engine);
    const ibl = Filament.IndirectLight.Builder().build(engine);
    const skybox = Filament.Skybox.Builder().color([0, 0, 0, 1]).build(engine);
    const vb = Filament.VertexBuffer.Builder().vertexCount(1).bufferCount(1).build(engine);
    const ib = Filament.IndexBuffer.Builder().indexCount(1).build(engine);

    engine.destroyMaterialInstance(matinst);
    engine.destroyMaterial(material);
    engine.destroyTexture(texture);
    engine.destroyRenderTarget(rt);
    engine.destroyColorGrading(colorGrading);
    engine.destroyIndirectLight(ibl);
    engine.destroySkybox(skybox);
    engine.destroyVertexBuffer(vb);
    engine.destroyIndexBuffer(ib);
    engine.destroySwapChain(engine.createSwapChain());
    engine.destroyRenderer(engine.createRenderer());
    engine.destroyView(engine.createView());
    engine.destroyScene(engine.createScene());
    engine.destroyCameraComponent(cameraEntity);
    engine.destroyEntity(cameraEntity);
    console.log(camera, expensiveCheck);
}

// The asset-decoding entry points. Each takes a URL registered with Filament.init, or a buffer.
function smoke_engine_asset_helpers() {
    const png: Filament.Texture = engine.createTextureFromPng("albedo.png");
    const jpeg: Filament.Texture = engine.createTextureFromJpeg("albedo.jpg", { srgb: true });
    const ktx1: Filament.Texture = engine.createTextureFromKtx1("albedo.ktx");
    const ktx2: Filament.Texture = engine.createTextureFromKtx2("albedo.ktx2");
    const ibl: Filament.IndirectLight = engine.createIblFromKtx1("env_ibl.ktx");
    const sky: Filament.Skybox = engine.createSkyFromKtx1("env_skybox.ktx");
    const mesh = engine.loadFilamesh("model.filamesh");
    const renderable: Filament.Entity = mesh.renderable;
    Filament.getSupportedFormatSuffix("etc s3tc");
    console.log(png, jpeg, ktx1, ktx2, ibl, sky, renderable);
}

function smoke_engine_builder() {
    const config: Filament.Engine$Config = Filament.Engine.createDefaultConfig();
    config.commandBufferSizeMB = 3;
    config.jobSystemThreadCount = 0;
    config.disableParallelShaderCompile = true;
    config.disableHandleUseAfterFreeCheck = true;
    config.assertNativeWindowIsValid = false;
    config.forceGLES2Context = false;
    config.stereoscopicType = Filament.StereoscopicType.NONE;
    config.stereoscopicEyeCount = 2;
    config.preferredShaderLanguage = Filament.ShaderLanguage.DEFAULT;
    config.gpuContextPriority = Filament.GpuContextPriority.DEFAULT;
    config.sharedUboInitialSizeInBytes = 16384;

    const colorGrading = Filament.ColorGrading.Builder();
    const builder: Filament.Engine$Builder = new Filament.Engine$Builder();
    builder.backend(Filament.Backend.OPENGL)
        .config(config)
        .featureLevel(Filament.FeatureLevel.FEATURE_LEVEL_1)
        .feature("backend.disable_parallel_shader_compile", true)
        .colorGrading(colorGrading);

    // The same settings are reachable through Engine.create, which owns the canvas' context.
    const other: Filament.Engine = Filament.Engine.create(canvas, {
        backend: Filament.Backend.OPENGL,
        featureLevel: Filament.FeatureLevel.FEATURE_LEVEL_1,
        features: { "backend.disable_parallel_shader_compile": true },
        colorGrading: Filament.ColorGrading.Builder(),
    }, config);
    console.log(builder, other);
}

// ---------------------------------------------------------------------------
// Camera + frustum
// ---------------------------------------------------------------------------

function smoke_camera_frustum() {
    const camera: Filament.Camera = engine.createCamera(newEntity());
    camera.setProjection(Filament.Camera$Projection.ORTHO, 0, 1, 0, 1, 0, 1);
    camera.setProjectionFov(45, 1.0, 0.0, 1.0, Filament.Camera$Fov.HORIZONTAL);
    camera.setLensProjection(0, 0.33, 1, 2);
    camera.setCustomProjection(m4, 0, 1);
    const m5 = camera.getProjectionMatrix() as glm.mat4;
    const m6 = camera.getCullingProjectionMatrix() as glm.mat4;
    camera.lookAt([0, 0, 0], [0, 0, 0], [0, 0, 0]);
    camera.lookAt(v3, v3, v3);
    const frustum: Filament.Frustum = camera.getFrustum();
    frustum.setProjection(m4);
    const v5 = frustum.getNormalizedPlane(Filament.Frustum$Plane.BOTTOM) as glm.vec4;
    const b: boolean = frustum.intersectsSphere([0, 1, 2, 3]);
    const c: boolean = frustum.intersectsSphere(glm.vec4.fromValues(0, 1, 2, 3));
    const d: boolean = frustum.intersectsBox({ center: v3, halfExtent: v3 });
    const standalone = new Filament.Frustum(m4);
    console.log(m5, m6, v5, b, c, d, standalone);
}

// The view matrix and the basis vectors derived from it, plus the two projection statics.
function smoke_camera_transform() {
    const camera = engine.createCamera(newEntity());
    camera.setModelMatrix(m4);
    const model = camera.getModelMatrix() as glm.mat4;
    const view = camera.getViewMatrix() as glm.mat4;
    const position: Filament.float3 = camera.getPosition();
    const left: Filament.float3 = camera.getLeftVector();
    const up: Filament.float3 = camera.getUpVector();
    const forward: Filament.float3 = camera.getForwardVector();
    camera.setScaling([2, 2]);
    const scaling: Filament.double4 = camera.getScaling();
    const near: number = camera.getNear();
    const cullingFar: number = camera.getCullingFar();
    const inverse = Filament.Camera.inverseProjection(m4) as glm.mat4;
    const effFocal: number = Filament.Camera.computeEffectiveFocalLength(0.05, 5);
    console.log(model, view, position, left, up, forward, scaling, near, cullingFar, inverse,
            effFocal);
}

function smoke_camera_exposure_shift() {
    const camera = engine.createCamera(newEntity());
    camera.setExposure(16, 1 / 125, 100);
    camera.setExposureDirect(1.0);
    const aperture: number = camera.getAperture();
    const focal: number = camera.getFocalLength();
    camera.setFocusDistance(5);
    const focus: number = camera.getFocusDistance();
    camera.setShift([0.1, 0.2]);
    const shift: Filament.double2 = camera.getShift();
    const shutter: number = camera.getShutterSpeed();
    const sensitivity: number = camera.getSensitivity();
    console.log(shutter, sensitivity);
    const effFov: number = Filament.Camera.computeEffectiveFov(45, 5);
    const fov: number = camera.getFieldOfViewInDegrees(Filament.Camera$Fov.VERTICAL);
    const cameraEntity: Filament.Entity = camera.getEntity();
    camera.setEyeModelMatrix(0, m4);
    camera.setCustomEyeProjection([m4, m4], m4, 0.1, 100);
    console.log(aperture, focal, focus, shift, effFov, fov, cameraEntity);
}

// ---------------------------------------------------------------------------
// View / Scene / Renderer
// ---------------------------------------------------------------------------

function smoke_view() {
    const view = engine.createView();
    const validView: boolean = engine.isValidView(view);
    const colorGrading: Filament.ColorGrading = view.getColorGrading();
    console.log(colorGrading);
    const viewport: Filament.float4 = view.getViewport();
    const hasCamera: boolean = view.hasCamera();
    const sampleCount: number = view.getSampleCount();
    view.setDithering(Filament.View$Dithering.TEMPORAL);
    view.setBlendMode(Filament.View$BlendMode.TRANSLUCENT);
    const blendMode: Filament.View$BlendMode = view.getBlendMode();
    const dro: Filament.View$DynamicResolutionOptions = view.getDynamicResolutionOptions();
    view.setDynamicResolutionOptions(dro);
    const gridSize: number = view.getGridSize();
    view.setGridSize(gridSize);
    view.setShadowingEnabled(true);
    const shadowingEnabled: boolean = view.isShadowingEnabled();
    view.setShadowType(Filament.View$ShadowType.VSM);
    const shadowType: Filament.View$ShadowType = view.getShadowType();
    view.setVsmShadowOptions({ anisotropy: 0, mipmapping: false });
    const vsmOptions: Filament.View$VsmShadowOptions = view.getVsmShadowOptions();
    view.setSoftShadowOptions({ penumbraScale: 1.0 });
    const softOptions: Filament.View$SoftShadowOptions = view.getSoftShadowOptions();
    view.setFrustumCullingEnabled(false);
    const frustumCulling: boolean = view.isFrustumCullingEnabled();
    console.log(shadowingEnabled, shadowType, vsmOptions, softOptions, frustumCulling);
    view.setFrontFaceWindingInverted(false);
    const inverted: boolean = view.isFrontFaceWindingInverted();
    const materialGlobal: Filament.float4 = view.getMaterialGlobal(0);
    view.setMaterialGlobal(0, materialGlobal);
    const fogEntity: Filament.Entity = view.getFogEntity();
    view.clearFrameHistory(engine);
    console.log(validView, viewport, hasCamera, sampleCount, blendMode, dro, gridSize, inverted,
            materialGlobal, fogEntity);

    // Post-processing option getters (each setter already had a binding; the getters are new).
    const bloom: Filament.View$BloomOptions = view.getBloomOptions();
    view.setBloomOptions(bloom);
    const fog: Filament.View$FogOptions = view.getFogOptions();
    view.setFogOptions(fog);
    const vignette: Filament.View$VignetteOptions = view.getVignetteOptions();
    view.setVignetteOptions(vignette);
    const dof: Filament.View$DepthOfFieldOptions = view.getDepthOfFieldOptions();
    view.setDepthOfFieldOptions(dof);
    const ao: Filament.View$AmbientOcclusionOptions = view.getAmbientOcclusionOptions();
    view.setAmbientOcclusionOptions(ao);
    const guardBand: Filament.View$GuardBandOptions = view.getGuardBandOptions();
    view.setGuardBandOptions(guardBand);
    const stereo: Filament.View$StereoscopicOptions = view.getStereoscopicOptions();
    view.setStereoscopicOptions(stereo);
    const taa: Filament.View$TemporalAntiAliasingOptions = view.getTemporalAntiAliasingOptions();
    view.setTemporalAntiAliasingOptions(taa);
    const ssr: Filament.View$ScreenSpaceReflectionsOptions =
            view.getScreenSpaceReflectionsOptions();
    view.setScreenSpaceReflectionsOptions(ssr);
    const msaa: Filament.View$MultiSampleAntiAliasingOptions =
            view.getMultiSampleAntiAliasingOptions();
    view.setMultiSampleAntiAliasingOptions(msaa);
    const renderQuality: Filament.View$RenderQuality = view.getRenderQuality();
    view.setRenderQuality(renderQuality);
    const dithering: Filament.View$Dithering = view.getDithering();

    // Scalar/handle getters and the layer + refraction toggles.
    const visibleLayers: number = view.getVisibleLayers();
    view.setVisibleLayers(0xff, 0x01);
    view.setLayerEnabled(1, true);
    view.setPostProcessingEnabled(true);
    const postProcessing: boolean = view.isPostProcessingEnabled();
    view.setScreenSpaceRefractionEnabled(true);
    const ssRefraction: boolean = view.isScreenSpaceRefractionEnabled();
    view.setName("smoke view");
    const viewName: string = view.getName();
    const viewScene: Filament.Scene = view.getScene();
    const viewCamera: Filament.Camera = view.getCamera();
    const viewRenderTarget: Filament.RenderTarget = view.getRenderTarget();
    console.log(bloom, fog, vignette, dof, ao, guardBand, stereo, taa, ssr, msaa, renderQuality,
            dithering, visibleLayers, postProcessing, ssRefraction, viewName, viewScene,
            viewCamera, viewRenderTarget);
}

// The setters that wire a view to the objects it draws, and the pipeline toggles that pair with
// the option structs exercised above.
function smoke_view_bindings() {
    const view = engine.createView();
    view.setScene(engine.createScene());
    view.setCamera(engine.createCamera(newEntity()));
    view.setViewport([0, 0, 64, 64]);
    view.setColorGrading(Filament.ColorGrading.Builder().build(engine));
    view.setRenderTarget(Filament.RenderTarget.Builder().build(engine));
    view.setSampleCount(4);
    view.setDynamicLightingOptions(5, 100);

    view.setAntiAliasing(Filament.View$AntiAliasing.FXAA);
    const antiAliasing: Filament.View$AntiAliasing = view.getAntiAliasing();
    view.setAmbientOcclusion(Filament.View$AmbientOcclusion.SSAO);
    const ambientOcclusion: Filament.View$AmbientOcclusion = view.getAmbientOcclusion();
    view.setStencilBufferEnabled(true);
    const stencilBuffer: boolean = view.isStencilBufferEnabled();
    view.setTransparentPickingEnabled(true);
    const transparentPicking: boolean = view.isTransparentPickingEnabled();
    view.setChannelDepthClearEnabled(0, true);
    const depthClear: boolean = view.isChannelDepthClearEnabled(0);

    const effectiveGridSize: number = view.getEffectiveGridSize();
    const visibleRenderables: number = view.getVisibleRenderableCount();
    const resolutionScale: Filament.float2 = view.getLastDynamicResolutionScale();

    view.pick(16, 16, (result: Filament.PickingQueryResult) => {
        const renderable: Filament.Entity = result.renderable;
        console.log(renderable, result.depth, result.fragCoords);
    });
    console.log(antiAliasing, ambientOcclusion, stencilBuffer, transparentPicking, depthClear,
            effectiveGridSize, visibleRenderables, resolutionScale);
}

function smoke_scene() {
    const scene = engine.createScene();
    const entity = newEntity();
    scene.addEntity(entity);
    scene.addEntities([entity]);
    const entityCount: number = scene.getEntityCount();
    const renderableCount: number = scene.getRenderableCount();
    const lightCount: number = scene.getLightCount();
    const hasEntity: boolean = scene.hasEntity(entity);
    scene.setSkybox(Filament.Skybox.Builder().color([0, 0, 0, 1]).build(engine));
    scene.setIndirectLight(Filament.IndirectLight.Builder().build(engine));
    const skybox: Filament.Skybox = scene.getSkybox();
    const ibl: Filament.IndirectLight = scene.getIndirectLight();
    const validScene: boolean = engine.isValidScene(scene);
    scene.remove(entity);
    scene.removeEntities([entity]);
    // Both accept null, which detaches whatever the scene was holding.
    scene.setSkybox(null);
    scene.setIndirectLight(null);
    console.log(entityCount, renderableCount, lightCount, hasEntity, skybox, ibl, validScene);
}

function smoke_renderer() {
    const renderer = engine.createRenderer();
    const userTime: number = renderer.getUserTime();
    renderer.resetUserTime();
    renderer.skipNextFrames(1);
    const skipCount: number = renderer.getFrameToSkipCount();
    const shouldRender: boolean = renderer.shouldRenderFrame();
    const gpuBehind: boolean = renderer.hasGpuFallenBehind();
    console.log(gpuBehind);
    renderer.setClearOptions({ clearColor: [0, 0, 0, 1], clear: true });
    const clearOptions: Filament.Renderer$ClearOptions = renderer.getClearOptions();
    const validRenderer: boolean = engine.isValidRenderer(renderer);
    const dstSwapChain = engine.createSwapChain();
    renderer.copyFrame(dstSwapChain, [0, 0, 16, 16], [0, 0, 16, 16], 0);
    renderer.readPixels(0, 0, 16, 16, Filament.PixelDataFormat.RGBA,
            Filament.PixelDataType.UBYTE, (pixels: Uint8Array) => {
        console.log(pixels.length);
    });
    const rt = Filament.RenderTarget.Builder().build(engine);
    renderer.readPixels(rt, 0, 0, 16, 16, Filament.PixelDataFormat.RGBA,
            Filament.PixelDataType.UBYTE, (pixels: Uint8Array) => {
        console.log(pixels.length);
    });
    console.log(userTime, skipCount, shouldRender, clearOptions, validRenderer);
}

// The frame loop itself: the begin/end pair, the three ways to submit a view, and the clocks that
// drive pacing.
function smoke_renderer_frame() {
    const renderer = engine.createRenderer();
    const swapChain = engine.createSwapChain();
    const view = engine.createView();
    view.setScene(engine.createScene());
    view.setCamera(engine.createCamera(newEntity()));
    view.setViewport([0, 0, 16, 16]);

    renderer.render(swapChain, view);
    if (renderer.beginFrame(swapChain)) {
        renderer.renderView(view);
        renderer.endFrame();
    }
    // Standalone rendering targets the view's own RenderTarget rather than a swap chain.
    view.setRenderTarget(Filament.RenderTarget.Builder().build(engine));
    renderer.renderStandaloneView(view);

    const materialTime: number = renderer.getMaterialTime();
    renderer.setMaterialTimeEpoch(0);
    const now: number = Filament.Engine.getSteadyClockTimeNano();
    renderer.setPresentationTime(now);
    renderer.setDesiredPresentationTime(now);
    renderer.setRenderingDeadline(now);
    renderer.setVsyncTime(now);
    renderer.skipFrame(now);
    renderer.pauseRenderThread(0);
    console.log(materialTime);
}

// ---------------------------------------------------------------------------
// Component managers
// ---------------------------------------------------------------------------

function smoke_transforms() {
    const tcm = engine.getTransformManager();
    const entity = newEntity();
    tcm.create(entity);
    const hasComponent: boolean = tcm.hasComponent(entity);
    console.log(hasComponent);

    const child = newEntity();
    tcm.create(child);
    const childInst = tcm.getInstance(child);
    tcm.setParent(childInst, tcm.getInstance(entity));
    childInst.delete();
    tcm.destroy(child);

    const inst: Filament.TransformManager$Instance = tcm.getInstance(entity);
    tcm.setTransform(inst, m4);
    const m5 = tcm.getTransform(inst) as glm.mat4;
    const m6 = tcm.getWorldTransform(inst) as glm.mat4;
    tcm.openLocalTransformTransaction();
    tcm.commitLocalTransformTransaction();
    const parent: Filament.Entity = tcm.getParent(inst);
    const children: Filament.Vector<Filament.Entity> = tcm.getChildren(inst);
    const childCount: number = tcm.getChildCount(inst);
    tcm.setAccurateTranslationsEnabled(true);
    const accurate: boolean = tcm.isAccurateTranslationsEnabled();
    console.log(m5, m6, parent, children, childCount, accurate);
    inst.delete();
}

function smoke_renderables() {
    const rm = engine.getRenderableManager();
    const entity = newEntity();
    const inst: Filament.RenderableManager$Instance = rm.getInstance(entity);
    const bone: Filament.RenderableManager$Bone = {
        unitQuaternion: qt,
        translation: v3
    };
    rm.setCastShadows(inst, true);
    rm.setBones(inst, [bone], 0);
    rm.setBonesFromMatrices(inst, [m4], 0);
    Filament.RenderableManager.Builder(1)
        .skinning(1)
        .skinningBones([bone])
        .skinningMatrices([m4])
        .build(engine, entity);
    const hasComponent: boolean = rm.hasComponent(entity);
    console.log(hasComponent);
    inst.delete();
    rm.destroy(entity);
}

// The Builder options that describe how a renderable is drawn, and their instance-side setters.
function smoke_renderable_appearance() {
    const rm = engine.getRenderableManager();
    const entity = newEntity();
    const box: Filament.Box = { center: v3, halfExtent: [1, 1, 1] };
    Filament.RenderableManager.Builder(1)
        .boundingBox(box)
        .layerMask(0xff, 0x01)
        .priority(4)
        .channel(1)
        .culling(true)
        .castShadows(true)
        .receiveShadows(true)
        .screenSpaceContactShadows(false)
        .fog(true)
        .lightChannel(0, true)
        .blendOrder(0, 1)
        .globalBlendOrderEnabled(0, true)
        .morphing(false)
        .instances(2)
        .build(engine, entity);

    const inst = rm.getInstance(entity);
    rm.setAxisAlignedBoundingBox(inst, box);
    rm.setLayerMask(inst, 0xff, 0x01);
    rm.setPriority(inst, 3);
    rm.setCulling(inst, true);
    rm.setReceiveShadows(inst, true);
    rm.setScreenSpaceContactShadows(inst, false);
    rm.setBlendOrderAt(inst, 0, 2);
    rm.setGlobalBlendOrderEnabledAt(inst, 0, true);

    const aabb: Filament.Box = rm.getAxisAlignedBoundingBox(inst);
    const culling: boolean = rm.isCullingEnabled(inst);
    const receiver: boolean = rm.isShadowReceiver(inst);
    const contactShadows: boolean = rm.isScreenSpaceContactShadowsEnabled(inst);
    const blendOrder: number = rm.getBlendOrderAt(inst, 0);
    const globalBlendOrder: boolean = rm.isGlobalBlendOrderEnabledAt(inst, 0);
    const attributes: number = rm.getEnabledAttributesAt(inst, 0);
    const instanceCount: number = rm.getInstanceCount(inst);
    inst.delete();
    console.log(aabb, culling, receiver, contactShadows, blendOrder, globalBlendOrder, attributes,
            instanceCount);
}

// Material assignment, which is the one part of a renderable that outlives the Builder.
function smoke_renderable_materials() {
    const rm = engine.getRenderableManager();
    const entity = newEntity();
    const material = engine.createMaterial("nonexistent.filamat");
    const matinst = material.createInstance();
    const vb = Filament.VertexBuffer.Builder().vertexCount(3).bufferCount(1).build(engine);
    const ib = Filament.IndexBuffer.Builder().indexCount(3).build(engine);

    Filament.RenderableManager.Builder(1)
        .material(0, matinst)
        .geometry(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb, ib)
        .build(engine, entity);

    const inst = rm.getInstance(entity);
    rm.setMaterialInstanceAt(inst, 0, matinst);
    const assigned: Filament.MaterialInstance = rm.getMaterialInstanceAt(inst, 0);
    rm.setGeometryAt(inst, 0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb, ib, 0, 3);
    rm.clearMaterialInstanceAt(inst, 0);
    inst.delete();
    console.log(assigned);
}

function smoke_renderable_geometry() {
    const rm = engine.getRenderableManager();
    const entity = newEntity();
    const vb = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(Filament.VertexAttribute.POSITION, 0,
                Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .build(engine);
    Filament.RenderableManager.Builder(2)
        .geometryType(Filament.RenderableManager$Builder$GeometryType.DYNAMIC)
        .geometryNoIndices(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb)
        .geometryNoIndicesOffset(1, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb, 0, 3)
        .build(engine, entity);

    // The indexed forms, which differ in how much of the index buffer each primitive covers.
    const ib = Filament.IndexBuffer.Builder().indexCount(3).build(engine);
    Filament.RenderableManager.Builder(2)
        .geometryOffset(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb, ib, 0, 3)
        .geometryMinMax(1, Filament.RenderableManager$PrimitiveType.TRIANGLES, vb, ib, 0, 0, 2, 3)
        .build(engine, newEntity());
    const inst = rm.getInstance(entity);
    rm.setGeometryNoIndicesAt(inst, 0, Filament.RenderableManager$PrimitiveType.TRIANGLES,
            vb, 0, 3);
    inst.delete();
}

function smoke_skinning_morphing_buffers() {
    const bone: Filament.RenderableManager$Bone = {
        unitQuaternion: qt,
        translation: v3
    };
    const sb: Filament.SkinningBuffer = Filament.SkinningBuffer.Builder()
        .boneCount(4)
        .initialize(true)
        .build(engine);
    const boneCount: number = sb.getBoneCount();
    sb.setBones(engine, [bone], 0);
    sb.setBonesFromMatrices(engine, [m4], 0);
    const validSb: boolean = engine.isValidSkinningBuffer(sb);

    const mtb: Filament.MorphTargetBuffer = Filament.MorphTargetBuffer.Builder()
        .vertexCount(3)
        .count(2)
        .withPositions(true)
        .withTangents(true)
        .enableCustomMorphing(true)
        .build(engine);
    const vertexCount: number = mtb.getVertexCount();
    const targetCount: number = mtb.getCount();
    const hasPositions: boolean = mtb.hasPositions();
    const hasTangents: boolean = mtb.hasTangents();
    const customMorphing: boolean = mtb.isCustomMorphingEnabled();
    mtb.setPositionsAt(engine, 0, new Float32Array(9), 3, 0);
    mtb.setTangentsAt(engine, 0, new Int16Array(12), 3, 0);
    const validMtb: boolean = engine.isValidMorphTargetBuffer(mtb);

    const entity = newEntity();
    Filament.RenderableManager.Builder(1)
        .enableSkinningBuffers(true)
        .skinningBuffer(sb, 4, 0)
        .morphingTargetCount(2)
        .morphingBuffer(mtb)
        .morphingBufferOffset(0, 0, 0)
        .build(engine, entity);

    // Instance-side counterpart of Builder::skinningBuffer.
    const rm = engine.getRenderableManager();
    const skinInst = rm.getInstance(entity);
    rm.setSkinningBuffer(skinInst, sb, 4, 0);
    skinInst.delete();

    engine.destroySkinningBuffer(sb);
    engine.destroyMorphTargetBuffer(mtb);
    console.log(boneCount, validSb, vertexCount, targetCount, hasPositions, hasTangents,
            customMorphing, validMtb);
}

function smoke_renderable_instance() {
    const rm = engine.getRenderableManager();
    const rinst = rm.getInstance(newEntity());
    const priority: number = rm.getPriority(rinst);
    rm.setChannel(rinst, 1);
    const channel: number = rm.getChannel(rinst);
    rm.setFogEnabled(rinst, true);
    const fogEnabled: boolean = rm.getFogEnabled(rinst);
    rm.setLightChannel(rinst, 0, true);
    const lightChannel: boolean = rm.getLightChannel(rinst, 0);
    const shadowCaster: boolean = rm.isShadowCaster(rinst);
    const primitiveCount: number = rm.getPrimitiveCount(rinst);
    const morphTargetCount: number = rm.getMorphTargetCount(rinst);
    rm.setMorphWeights(rinst, [0.0, 1.0]);
    rm.setMorphWeightsOffset(rinst, [0.0, 1.0], 2);
    rm.setMorphTargetBufferOffsetAt(rinst, 0, 0, 0);
    console.log(priority, channel, fogEnabled, lightChannel, shadowCaster, primitiveCount,
            morphTargetCount);
}

function smoke_lights() {
    const entity = newEntity();
    Filament.LightManager.Builder(Filament.LightManager$Type.SUN)
        .color([1, 1, 1])
        .intensity(110000)
        .direction([0, -1, 0])
        .castShadows(true)
        .shadowOptions({
            mapSize: 1024,
            shadowCascades: 4,
            cascadeSplitPositions: [0.25, 0.50, 0.75],
            vsm: { elvsm: true, blurWidth: 2.0 },
        })
        .build(engine, entity);
    const lm = engine.getLightManager();
    const has: boolean = lm.hasComponent(entity);
    const componentCount: number = lm.getComponentCount();
    console.log(componentCount);
    const inst: Filament.LightManager$Instance = lm.getInstance(entity);
    const type: Filament.LightManager$Type = lm.getType(inst);
    lm.setIntensity(inst, 100);
    const intensity: number = lm.getIntensity(inst);
    const color: Filament.float3 = lm.getColor(inst);
    lm.setLightChannel(inst, 0, true);
    const lightChannel: boolean = lm.getLightChannel(inst, 0);
    lm.setIntensityCandela(inst, 1000);
    inst.delete();
    lm.destroy(entity);
    console.log(has, type, intensity, color, lightChannel);

    const spotEntity = newEntity();
    Filament.LightManager.Builder(Filament.LightManager$Type.SPOT)
        .intensityCandela(1000)
        .spotLightCone(0.25, 0.5)
        .build(engine, spotEntity);
    const spot: Filament.LightManager$Instance = lm.getInstance(spotEntity);
    lm.setSpotLightCone(spot, 0.1, 0.2);
    const innerCone: number = lm.getSpotLightInnerCone(spot);
    const outerCone: number = lm.getSpotLightOuterCone(spot);
    console.log(innerCone, outerCone);

    // The point/spot placement and sun-disc controls, on the Builder and on the instance.
    const sunEntity = newEntity();
    Filament.LightManager.Builder(Filament.LightManager$Type.SUN)
        .castLight(true)
        .position([1, 2, 3])
        .direction([0, -1, 0])
        .falloff(10)
        .intensityEnergy(100, 0.8)
        .sunAngularRadius(1)
        .sunHaloSize(10)
        .sunHaloFalloff(80)
        .lightChannel(0, true)
        .build(engine, sunEntity);
    const sun: Filament.LightManager$Instance = lm.getInstance(sunEntity);
    lm.setPosition(sun, [4, 5, 6]);
    lm.setDirection(sun, [0, 0, -1]);
    lm.setFalloff(sun, 20);
    lm.setIntensityEnergy(sun, 100, 0.5);
    lm.setSunAngularRadius(sun, 0.5);
    lm.setSunHaloSize(sun, 11);
    lm.setSunHaloFalloff(sun, 60);
    lm.setShadowCaster(sun, true);
    lm.setShadowOptions(sun, { mapSize: 512, shadowCascades: 1 });
    const position: Filament.float3 = lm.getPosition(sun);
    const direction: Filament.float3 = lm.getDirection(sun);
    const falloff: number = lm.getFalloff(sun);
    const angularRadius: number = lm.getSunAngularRadius(sun);
    const haloSize: number = lm.getSunHaloSize(sun);
    const haloFalloff: number = lm.getSunHaloFalloff(sun);
    const caster: boolean = lm.isShadowCaster(sun);
    const directional: boolean = lm.isDirectional(sun);
    const point: boolean = lm.isPointLight(sun);
    const isSpot: boolean = lm.isSpotLight(sun);
    sun.delete();
    console.log(position, direction, falloff, angularRadius, haloSize, haloFalloff, caster,
            directional, point, isSpot);

    const uniform: number[] = Filament.LightManager$ShadowCascades.computeUniformSplits(4);
    const logSplits: number[] = Filament.LightManager$ShadowCascades.computeLogSplits(4, 0.1, 100);
    const practical: number[] =
            Filament.LightManager$ShadowCascades.computePracticalSplits(4, 0.1, 100, 0.5);
    console.log(uniform, logSplits, practical);
}

// ---------------------------------------------------------------------------
// GPU resources: buffers, textures, materials, render targets
// ---------------------------------------------------------------------------

function smoke_buffers() {
    const vb = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(Filament.VertexAttribute.POSITION, 0,
                Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .normalized(Filament.VertexAttribute.COLOR)
        .normalizedIf(Filament.VertexAttribute.COLOR, false)
        .build(engine);
    vb.setBufferAt(engine, 0, new Float32Array(9));
    const validVb: boolean = engine.isValidVertexBuffer(vb);

    // A vertex buffer can source its data from a BufferObject instead of its own storage.
    const boVb = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .enableBufferObjects(true)
        .attribute(Filament.VertexAttribute.POSITION, 0,
                Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .build(engine);
    const vbo = Filament.BufferObject.Builder()
        .size(36)
        .bindingType(Filament.BufferObject$BindingType.VERTEX)
        .build(engine);
    vbo.setBuffer(engine, new Float32Array(9));
    boVb.setBufferObjectAt(engine, 0, vbo);

    const ib = Filament.IndexBuffer.Builder()
        .indexCount(3)
        .bufferType(Filament.IndexBuffer$IndexType.USHORT)
        .build(engine);
    ib.setBuffer(engine, new Uint16Array([0, 1, 2]));
    const validIb: boolean = engine.isValidIndexBuffer(ib);

    const bo = Filament.BufferObject.Builder()
        .size(36)
        .bindingType(Filament.BufferObject$BindingType.VERTEX)
        .build(engine);
    bo.setBuffer(engine, new Float32Array(9));
    const byteCount: number = bo.getByteCount();
    const vertexCount: number = vb.getVertexCount();
    const indexCount: number = ib.getIndexCount();
    console.log(validVb, validIb, byteCount, vertexCount, indexCount);
}

function smoke_texture() {
    const texture = Filament.Texture.Builder()
        .width(16)
        .height(16)
        .levels(1)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(Filament.TextureUsage.DEFAULT)
        .build(engine);
    const w: number = texture.getWidth(engine);
    const h: number = texture.getHeight(engine, 0);
    const d: number = texture.getDepth(engine, 0);
    const levels: number = texture.getLevels(engine);
    const validFormat: boolean = Filament.Texture.validatePixelFormatAndType(
            Filament.Texture$InternalFormat.RGBA8, Filament.PixelDataFormat.RGBA,
            Filament.PixelDataType.UBYTE);
    console.log(d, validFormat);
    texture.generateMipmaps(engine);
    const mipmappable: boolean = Filament.Texture.isTextureFormatMipmappable(engine,
            Filament.Texture$InternalFormat.RGBA8);
    const swizzleSupported: boolean = Filament.Texture.isTextureSwizzleSupported(engine);
    console.log(swizzleSupported);
    const validTexture: boolean = engine.isValidTexture(texture);

    const target: Filament.Texture$Sampler = texture.getTarget();
    const format: Filament.Texture$InternalFormat = texture.getFormat();
    const formatSupported: boolean = Filament.Texture.isTextureFormatSupported(engine,
            Filament.Texture$InternalFormat.RGBA8);
    const maxSize: number = Filament.Texture.getMaxTextureSize(engine,
            Filament.Texture$Sampler.SAMPLER_2D);
    const maxLayers: number = Filament.Texture.getMaxArrayTextureLayers(engine);
    console.log(target, format, formatSupported, maxSize, maxLayers);

    // swizzle() is reachable, but build() throws on WebGL, which has no texture swizzle. This
    // file is only ever type-checked, never executed, so the call is safe to express here.
    Filament.Texture.Builder()
        .width(16)
        .height(16)
        .depth(1)
        .samples(1)
        .swizzle(Filament.Texture$Swizzle.CHANNEL_0, Filament.Texture$Swizzle.CHANNEL_1,
                Filament.Texture$Swizzle.CHANNEL_2, Filament.Texture$Swizzle.SUBSTITUTE_ONE)
        .build(engine);

    // Likewise external(): the web has no stream or image to attach, so the texture stays empty.
    Filament.Texture.Builder()
        .width(16)
        .height(16)
        .sampler(Filament.Texture$Sampler.SAMPLER_EXTERNAL)
        .external()
        .build(engine);

    const sampler = new Filament.TextureSampler(Filament.MinFilter.LINEAR,
            Filament.MagFilter.LINEAR, Filament.WrapMode.CLAMP_TO_EDGE);
    sampler.setAnisotropy(4);
    sampler.setCompareMode(Filament.CompareMode.NONE, Filament.CompareFunc.LESS_EQUAL);
    sampler.setMinFilter(Filament.MinFilter.NEAREST);
    sampler.setMagFilter(Filament.MagFilter.NEAREST);
    sampler.setWrapModeS(Filament.WrapMode.REPEAT);
    sampler.setWrapModeT(Filament.WrapMode.REPEAT);
    sampler.setWrapModeR(Filament.WrapMode.REPEAT);
    const anisotropy: number = sampler.getAnisotropy();
    const compareMode: Filament.CompareMode = sampler.getCompareMode();
    const compareFunc: Filament.CompareFunc = sampler.getCompareFunc();
    const minFilter: Filament.MinFilter = sampler.getMinFilter();
    const magFilter: Filament.MagFilter = sampler.getMagFilter();
    const wrapS: Filament.WrapMode = sampler.getWrapModeS();
    const wrapT: Filament.WrapMode = sampler.getWrapModeT();
    const wrapR: Filament.WrapMode = sampler.getWrapModeR();
    console.log(anisotropy, compareMode, compareFunc, minFilter, magFilter, wrapS, wrapT, wrapR);

    const pbd = new Filament.driver$PixelBufferDescriptor(16 * 16 * 4,
            Filament.PixelDataFormat.RGBA, Filament.PixelDataType.UBYTE);
    texture.setImage(engine, 0, pbd);
    console.log(w, h, levels, mipmappable, validTexture);
}

function smoke_material() {
    const material = engine.createMaterial("nonexistent.filamat");
    const matinst: Filament.MaterialInstance = material.createInstance();
    const named: Filament.MaterialInstance = material.createNamedInstance("named");
    const def: Filament.MaterialInstance = material.getDefaultInstance();
    const name: string = material.getName();
    const validMaterial: boolean = engine.isValidMaterial(material);
    const validInstance: boolean = engine.isValidMaterialInstance(material, matinst);

    const source: Filament.Material = matinst.getMaterial();
    const dup: Filament.MaterialInstance = matinst.duplicate();
    const dupNamed: Filament.MaterialInstance = matinst.duplicateNamed("copy");
    matinst.setScissor(0, 0, 16, 16);
    matinst.unsetScissor();
    console.log(source, dup, dupNamed);

    matinst.setFloatParameter("alpha", 1.0);
    matinst.setBoolParameter("flag", true);
    matinst.setBool2Parameter("flag2", [true, false]);
    matinst.setBool3Parameter("flag3", [true, false, true]);
    matinst.setBool4Parameter("flag4", [true, false, true, false]);
    matinst.setIntParameter("count", 1);
    matinst.setInt2Parameter("count2", [1, 2]);
    matinst.setInt3Parameter("count3", [1, 2, 3]);
    matinst.setInt4Parameter("count4", [1, 2, 3, 4]);
    matinst.setFloat2Parameter("uvScale", [1, 1]);
    matinst.setFloat3Parameter("tint", v3);
    matinst.setFloat4Parameter("rect", v4);
    matinst.setMat3Parameter("uvTransform", m4);
    matinst.setMat4Parameter("transform", m3);
    matinst.setColor3Parameter("baseColor", Filament.RgbType.sRGB, v3);
    matinst.setColor4Parameter("emissive", Filament.RgbaType.sRGB, v4);
    matinst.setTextureParameter("albedo",
            Filament.Texture.Builder().width(1).height(1).build(engine),
            new Filament.TextureSampler(Filament.MinFilter.LINEAR, Filament.MagFilter.LINEAR,
                    Filament.WrapMode.REPEAT));
    matinst.setCullingMode(Filament.CullingMode.BACK);
    matinst.setTransparencyMode(Filament.TransparencyMode.DEFAULT);
    const transparency: Filament.TransparencyMode = matinst.getTransparencyMode();
    const threshold: number = matinst.getMaskThreshold();
    console.log(named, def, name, validMaterial, validInstance, transparency, threshold);

    matinst.setConstantBool("test_bool", true);
    matinst.setConstantFloat("test_float", 1.0);
    matinst.setConstantInt("test_int", 1);
    const stencilWrite: boolean = matinst.isStencilWriteEnabled();
    console.log(stencilWrite);

    // Reflection surface: parameter enumeration plus the compiled-in material properties.
    const paramCount: number = material.getParameterCount();
    const params: Filament.Material$ParameterInfo[] = material.getParameters();
    const hasParam: boolean = material.hasParameter("baseColor");
    const shading: Filament.Shading = material.getShading();
    const interpolation: Filament.Interpolation = material.getInterpolation();
    const blending: Filament.BlendingMode = material.getBlendingMode();
    const refractionMode: Filament.RefractionMode = material.getRefractionMode();
    const refractionType: Filament.RefractionType = material.getRefractionType();
    const reflectionMode: Filament.ReflectionMode = material.getReflectionMode();
    const vertexDomain: Filament.VertexDomain = material.getVertexDomain();
    const culling: Filament.CullingMode = material.getCullingMode();
    const matTransparency: Filament.TransparencyMode = material.getTransparencyMode();
    console.log(matTransparency);
    const featureLevel: Filament.FeatureLevel = material.getFeatureLevel();
    const maskThreshold: number = material.getMaskThreshold();
    const saaVariance: number = material.getSpecularAntiAliasingVariance();
    const saaThreshold: number = material.getSpecularAntiAliasingThreshold();
    const requiredAttributes: number = material.getRequiredAttributes();
    const doubleSided: boolean = material.isDoubleSided();
    const alphaToCoverage: boolean = material.isAlphaToCoverageEnabled();
    const colorWrite: boolean = material.isColorWriteEnabled();
    const depthWrite: boolean = material.isDepthWriteEnabled();
    const depthCulling: boolean = material.isDepthCullingEnabled();
    console.log(paramCount, params, hasParam, shading, interpolation, blending, refractionMode,
            refractionType, reflectionMode, vertexDomain, culling, featureLevel, maskThreshold,
            saaVariance, saaThreshold, requiredAttributes, doubleSided, alphaToCoverage,
            colorWrite, depthWrite, depthCulling);

    try {
        matinst.getConstantBool("test_bool");
        matinst.getConstantFloat("test_float");
        matinst.getConstantInt("test_int");
    } catch (e) {
        // constants might not exist in nonexistent.filamat, which is fine for smoke test
    }
}

// The per-instance render state: rasterizer, depth and stencil. Each setter overrides what the
// material was compiled with, so every one of them has a matching getter to read back.
function smoke_material_instance_state() {
    const material = engine.createMaterial("nonexistent.filamat");
    const matinst = material.createInstance();

    matinst.setDoubleSided(true);
    matinst.setCullingModeSeparate(Filament.CullingMode.BACK, Filament.CullingMode.FRONT);
    matinst.setPolygonOffset(1.0, 1.0);
    matinst.setColorWrite(true);
    matinst.setDepthWrite(true);
    matinst.setDepthCulling(true);
    matinst.setDepthFunc(Filament.CompareFunc.LESS);
    // Only legal on a MASKED material; the mask threshold is ignored otherwise.
    matinst.setMaskThreshold(0.4);
    matinst.setSpecularAntiAliasingVariance(0.2);
    matinst.setSpecularAntiAliasingThreshold(0.1);

    const doubleSided: boolean = matinst.isDoubleSided();
    const cullingMode: Filament.CullingMode = matinst.getCullingMode();
    const shadowCulling: Filament.CullingMode = matinst.getShadowCullingMode();
    const colorWrite: boolean = matinst.isColorWriteEnabled();
    const depthWrite: boolean = matinst.isDepthWriteEnabled();
    const depthCulling: boolean = matinst.isDepthCullingEnabled();
    const depthFunc: Filament.CompareFunc = matinst.getDepthFunc();
    const saaVariance: number = matinst.getSpecularAntiAliasingVariance();
    const saaThreshold: number = matinst.getSpecularAntiAliasingThreshold();

    matinst.setStencilWrite(true);
    matinst.setStencilCompareFunction(Filament.CompareFunc.EQUAL);
    matinst.setStencilCompareFunction(Filament.CompareFunc.EQUAL, Filament.StencilFace.FRONT);
    matinst.setStencilOpStencilFail(Filament.StencilOperation.KEEP);
    matinst.setStencilOpDepthFail(Filament.StencilOperation.KEEP, Filament.StencilFace.BACK);
    matinst.setStencilOpDepthStencilPass(Filament.StencilOperation.REPLACE);
    matinst.setStencilReferenceValue(1);
    matinst.setStencilReadMask(0xff);
    matinst.setStencilWriteMask(0xff, Filament.StencilFace.FRONT_AND_BACK);

    const transformName: string = material.getParameterTransformName("albedo");
    console.log(doubleSided, cullingMode, shadowCulling, colorWrite, depthWrite, depthCulling,
            depthFunc, saaVariance, saaThreshold, transformName);
}

function smoke_tone_mappers() {
    // Each operator reachable through ColorGrading$Builder.toneMapper. The four below the
    // deprecated ToneMapping enum cannot express are PBRNeutral, GT7, AgX and Generic.
    const mappers: Filament.ToneMapper[] = [
        new Filament.LinearToneMapper(),
        new Filament.ACESToneMapper(),
        new Filament.ACESLegacyToneMapper(),
        new Filament.FilmicToneMapper(),
        new Filament.PBRNeutralToneMapper(),
        new Filament.GT7ToneMapper(),
        new Filament.DisplayRangeToneMapper(),
        new Filament.AgxToneMapper(Filament.AgxToneMapper$AgxLook.PUNCHY),
    ];

    const generic = new Filament.GenericToneMapper(1.55, 0.18, 0.215, 10.0);
    generic.setContrast(1.6);
    generic.setMidGrayIn(0.2);
    generic.setMidGrayOut(0.22);
    generic.setHdrMax(12.0);
    const contrast: number = generic.getContrast();
    const midGrayIn: number = generic.getMidGrayIn();
    const midGrayOut: number = generic.getMidGrayOut();
    const hdrMax: number = generic.getHdrMax();
    console.log(contrast, midGrayIn, midGrayOut, hdrMax);

    for (const mapper of mappers) {
        Filament.ColorGrading.Builder().toneMapper(mapper).build(engine);
        mapper.delete();
    }
    Filament.ColorGrading.Builder().toneMapper(generic).build(engine);
    generic.delete();
}

function smoke_color_grading() {
    const cg = Filament.ColorGrading.Builder()
        .quality(Filament.ColorGrading$QualityLevel.HIGH)
        .format(Filament.ColorGrading$LutFormat.INTEGER)
        .dimensions(16)
        .toneMapping(Filament.ColorGrading$ToneMapping.ACES)
        .luminanceScaling(true)
        .gamutMapping(true)
        .nightAdaptation(0.5)
        .whiteBalance(0.1, -0.1)
        .channelMixer([1, 0, 0], [0, 1, 0], [0, 0, 1])
        .shadowsMidtonesHighlights([1, 1, 1, 0], [1, 1, 1, 0], [1, 1, 1, 0], [0, 0.33, 0.66, 1])
        .slopeOffsetPower([1, 1, 1], [0, 0, 0], [1, 1, 1])
        .contrast(1.1)
        .vibrance(1.1)
        .saturation(1.1)
        .curves([1, 1, 1], [1, 1, 1], [1, 1, 1])
        .fastMath(true)
        .exposure(0.0)
        .customLut(new Float32Array([
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
            1.0, 1.0, 1.0,
            0.0, 0.0, 0.0,
            1.0, 1.0, 0.0,
            0.0, 1.0, 1.0,
            1.0, 0.0, 1.0
        ]), 2)
        .build(engine);
    const valid: boolean = engine.isValidColorGrading(cg);
    console.log(valid);
}

function smoke_indirect_light() {
    const sh = new Float32Array(9 * 3);
    const ibl = Filament.IndirectLight.Builder()
        .irradiance(3, sh)
        .radiance(3, sh)
        .intensity(30000)
        .rotation(m3)
        .build(engine);
    ibl.setIntensity(25000);
    const intensity: number = ibl.getIntensity();
    ibl.setRotation(m3);
    const rotation: Filament.mat3 = ibl.getRotation();
    const valid: boolean = engine.isValidIndirectLight(ibl);
    console.log(intensity, rotation, valid);

    // The cubemap-backed form, plus the two estimators that read the SH coefficients directly.
    const cubemap = Filament.Texture.Builder()
        .width(16)
        .height(16)
        .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .build(engine);
    // The cubemap form of irradiance, which embind tells apart from the SH form by arity.
    const textured = Filament.IndirectLight.Builder()
        .reflections(cubemap)
        .irradiance(cubemap)
        .build(engine);
    const reflections: Filament.Texture = textured.getReflectionsTexture();
    const irradiance: Filament.Texture = textured.getIrradianceTexture();
    const direction: Filament.float3 = Filament.IndirectLight.getDirectionEstimate(sh);
    const color: Filament.float4 = Filament.IndirectLight.getColorEstimate(sh, direction);
    console.log(reflections, irradiance, direction, color);
}

function smoke_skybox() {
    const sky = Filament.Skybox.Builder()
        .color([0, 0, 0, 1])
        .intensity(30000)
        .showSun(true)
        .priority(1)
        .build(engine);
    sky.setColor([1, 1, 1, 1]);
    const mask: number = sky.getLayerMask();
    sky.setLayerMask(0xff, 0x01);
    const valid: boolean = engine.isValidSkybox(sky);

    // The textured form, whose cubemap is readable back off the skybox.
    const envmap = Filament.Texture.Builder()
        .width(16)
        .height(16)
        .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .build(engine);
    const textured = Filament.Skybox.Builder().environment(envmap).build(engine);
    const texture: Filament.Texture = textured.getTexture();
    console.log(mask, valid, texture);
}

function smoke_render_target() {
    const color = Filament.Texture.Builder()
        .width(16)
        .height(16)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(Filament.TextureUsage.COLOR_ATTACHMENT)
        .build(engine);
    const rt = Filament.RenderTarget.Builder()
        .texture(Filament.RenderTarget$AttachmentPoint.COLOR, color)
        .mipLevel(Filament.RenderTarget$AttachmentPoint.COLOR, 0)
        .face(Filament.RenderTarget$AttachmentPoint.COLOR, Filament.Texture$CubemapFace.POSITIVE_X)
        .layer(Filament.RenderTarget$AttachmentPoint.COLOR, 0)
        .build(engine);
    const tex: Filament.Texture = rt.getTexture(Filament.RenderTarget$AttachmentPoint.COLOR);
    const mip: number = rt.getMipLevel(Filament.RenderTarget$AttachmentPoint.COLOR);
    const face: Filament.Texture$CubemapFace =
            rt.getFace(Filament.RenderTarget$AttachmentPoint.COLOR);
    const layer: number = rt.getLayer(Filament.RenderTarget$AttachmentPoint.COLOR);
    const supported: number = rt.getSupportedColorAttachmentsCount();
    const valid: boolean = engine.isValidRenderTarget(rt);
    console.log(tex, mip, face, layer, supported, valid);
}

function smoke_swap_chain() {
    const sc = engine.createSwapChain();
    const valid: boolean = engine.isValidSwapChain(sc);
    const srgb: boolean = Filament.SwapChain.isSRGBSwapChainSupported(engine);
    const protectedContent: boolean = Filament.SwapChain.isProtectedContentSupported(engine);
    const msaa: boolean = Filament.SwapChain.isMSAASwapChainSupported(engine, 4);
    sc.setFrameScheduledCallback(() => console.log("frame scheduled"));
    const callbackSet: boolean = sc.isFrameScheduledCallbackSet();
    sc.setFrameScheduledCallback();
    console.log(valid, srgb, protectedContent, msaa, callbackSet);
}

// ---------------------------------------------------------------------------
// Geometry / image helpers
// ---------------------------------------------------------------------------

function smoke_surface_orientation() {
    // Every input the builder accepts. Normals alone are enough, but tangents, uvs, positions and
    // an index buffer each feed a different code path in the tangent-frame solver.
    const so = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(new Float32Array(9), 0)
        .tangents(new Float32Array(12), 0)
        .uvs(new Float32Array(6), 0)
        .positions(new Float32Array(9), 0)
        .triangleCount(1)
        .triangles16(new Uint16Array([0, 1, 2]))
        .build();
    const quats: Int16Array = so.getQuats(3);
    const half4: Uint16Array = so.getQuatsHalf4(3);
    const float4: Float32Array = so.getQuatsFloat4(3);
    so.delete();

    const indexed32 = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(new Float32Array(9), 0)
        .triangleCount(1)
        .triangles32(new Uint32Array([0, 1, 2]))
        .build();
    indexed32.delete();
    console.log(quats, half4, float4);
}

function smoke_ktx2_reader() {
    const reader = new Filament.Ktx2Reader(engine, true);
    reader.requestFormat(Filament.Texture$InternalFormat.RGBA8);
    reader.unrequestFormat(Filament.Texture$InternalFormat.RGBA8);
    const texture: Filament.Texture|null = reader.load("image.ktx2",
            Filament.Ktx2Reader$TransferFunction.LINEAR);
    console.log(reader, texture);
}

function smoke_ktx1_bundle() {
    const bd = new Filament.driver$BufferDescriptor(4);
    const bytes: ArrayBuffer = bd.getBytes();
    const bundle = new Filament.Ktx1Bundle(bd);
    const mips: number = bundle.getNumMipLevels();
    const arrayLength: number = bundle.getArrayLength();
    const format: Filament.Texture$InternalFormat = bundle.getInternalFormat(true);
    const pixelFormat: Filament.PixelDataFormat = bundle.getPixelDataFormat();
    const pixelType: Filament.PixelDataType = bundle.getPixelDataType();
    const compressedType: Filament.CompressedPixelDataType = bundle.getCompressedPixelDataType();
    const compressed: boolean = bundle.isCompressed();
    const cubemap: boolean = bundle.isCubemap();
    const info: Filament.KtxInfo = bundle.info();
    const width: number = info.pixelWidth;
    const height: number = info.pixelHeight;
    const depth: number = info.pixelDepth;
    const endianness: number = info.endianness;
    const glType: number = info.glType;
    const glTypeSize: number = info.glTypeSize;
    const glFormat: number = info.glFormat;
    const glInternalFormat: number = info.glInternalFormat;
    const glBaseInternalFormat: number = info.glBaseInternalFormat;
    console.log(height, depth, endianness, glType, glTypeSize, glFormat, glInternalFormat,
            glBaseInternalFormat);
    const blob: ArrayBuffer = bundle.getBlob([0, 0, 0]);
    const cubeBlob: ArrayBuffer = bundle.getCubeBlob(0);
    const metadata: string = bundle.getMetadata("key");
    console.log(mips, arrayLength, format, pixelFormat, pixelType, compressedType, compressed,
            cubemap, width, blob, cubeBlob, metadata, bytes);
}

function smoke_mesh_reader() {
    const registry = new Filament.MeshReader$MaterialRegistry();
    const material = engine.createMaterial("nonexistent.filamat");
    registry.set("DefaultMaterial", material.getDefaultInstance());
    const stored: Filament.MaterialInstance = registry.get("DefaultMaterial");
    const size: number = registry.size();
    const keys: Filament.Vector<string> = registry.keys();
    const bd = new Filament.driver$BufferDescriptor(4);
    const mesh = Filament.MeshReader.loadMeshFromBuffer(engine, bd, registry);
    const renderable: Filament.Entity = mesh.renderable();
    const vb: Filament.VertexBuffer = mesh.vertexBuffer();
    const ib: Filament.IndexBuffer = mesh.indexBuffer();
    console.log(size, keys, stored, renderable, vb, ib);
}

// ---------------------------------------------------------------------------
// Camera manipulator (camutils)
// ---------------------------------------------------------------------------

function smoke_camutils() {
    const manip = new Filament.Camutils$Manipulator$Builder()
        .viewport(1024, 768)
        .orbitHomePosition(0, 0, 1)
        .orbitSpeed(0.01, 0.01)
        .targetPosition(0, 0, 0)
        .upVector(0, 1, 0)
        .zoomSpeed(0.01)
        .fovDirection(Filament.Camutils$Fov.VERTICAL)
        .fovDegrees(45)
        .farPlane(1000)
        .mapExtent(512, 512)
        .mapMinDistance(1)
        .groundPlane(0, 1, 0, 0)
        .panning(true)
        .build(Filament.Camutils$Mode.ORBIT);
    manip.attach(canvas);
    manip.update(0.016);
    const camera: Filament.Camera = engine.createCamera(newEntity());
    camera.setLookAt(manip);
    manip.detach(canvas);

    // Free-flight adds a keyboard-driven mode, so it exercises the flight* options and keyDown/Up.
    const flight = new Filament.Camutils$Manipulator$Builder()
        .viewport(1024, 768)
        .flightStartPosition(0, 0, 5)
        .flightStartOrientation(0, 0)
        .flightMaxMoveSpeed(10)
        .flightSpeedSteps(80)
        .flightPanSpeed(0.01, 0.01)
        .flightMoveDamping(15)
        .build(Filament.Camutils$Mode.FREE_FLIGHT);
    flight.setViewport(800, 600);
    flight.keyDown(Filament.Camutils$Key.FORWARD);
    flight.keyUp(Filament.Camutils$Key.FORWARD);
    flight.grabBegin(10, 10, false);
    flight.grabUpdate(20, 20);
    flight.grabEnd();
    flight.scroll(10, 10, 1);
    flight.update(0.016);

    // getLookAt, getRay and raycast write into the vectors they are handed.
    const eye = v3, target = v3, up = v3;
    flight.getLookAt(eye, target, up);
    flight.getRay(10, 10, v3, v3);
    const hit: boolean = flight.raycast(10, 10, v3);

    const current: Filament.Camutils$Bookmark = flight.getCurrentBookmark();
    const home: Filament.Camutils$Bookmark = flight.getHomeBookmark();
    const midpoint: Filament.Camutils$Bookmark =
            Filament.Camutils$Bookmark.interpolate(current, home, 0.5);
    const duration: number = Filament.Camutils$Bookmark.duration(current, home);
    flight.jumpToBookmark(midpoint);
    console.log(hit, duration);
}

// ---------------------------------------------------------------------------
// glTF (gltfio)
// ---------------------------------------------------------------------------

function smoke_gltfio() {
    const loader = engine.createAssetLoader();
    const asset = loader.createAsset("model.glb");
    const entities: Filament.Entity[] = asset.getEntities();
    const root: Filament.Entity = asset.getRoot();
    const box: Filament.Aabb = asset.getBoundingBox();
    const instance = asset.getInstance();
    const morphTargetNames: string[] = asset.getMorphTargetNames(root);
    const skinNames: Filament.Vector<string> = instance.getSkinNames();
    const skinCount: number = instance.getSkinCount();
    const jointCount: number = instance.getJointCountAt(0);
    const joints: Filament.Entity[] = instance.getJointsAt(0);
    console.log(morphTargetNames, skinCount, jointCount, joints);
    const variants: string[] = instance.getMaterialVariantNames();
    const animator = instance.getAnimator();
    const animCount: number = animator.getAnimationCount();
    animator.applyAnimation(0, 0.0);
    animator.applyCrossFade(0, 0.0, 0.5);
    animator.updateBoneMatrices();
    animator.resetBoneMatrices();
    const animDuration: number = animator.getAnimationDuration(0);
    const animName: string = animator.getAnimationName(0);
    console.log(animDuration, animName);
    // Count accessors, so callers can size a buffer without materializing the entity vector.
    const entityCount: number = asset.getEntityCount();
    const lightCount: number = asset.getLightEntityCount();
    const renderableCount: number = asset.getRenderableEntityCount();
    const cameraCount: number = asset.getCameraEntityCount();
    const resourceUriCount: number = asset.getResourceUriCount();
    const assetMorphCount: number = asset.getMorphTargetCountAt(root);
    const sceneCount: number = asset.getSceneCount();
    const assetInstanceCount: number = asset.getAssetInstanceCount();
    const popped: Filament.Vector<Filament.Entity> = asset.popRenderables(4);
    const instEntityCount: number = instance.getEntityCount();
    const variantCount: number = instance.getMaterialVariantCount();
    const matInstCount: number = instance.getMaterialInstanceCount();
    loader.enableDiagnostics(true);
    console.log(entityCount, lightCount, renderableCount, cameraCount, resourceUriCount,
            assetMorphCount, sceneCount, assetInstanceCount, popped, instEntityCount,
            variantCount, matInstCount);

    loader.destroyAsset(asset);
    loader.gc();
    Filament.gltfio$AssetLoader.destroy(loader);
    console.log(entities, root, box, skinNames, variants, animCount);
}

// The entity queries an asset answers, and the instance-side skin and variant switching.
function smoke_gltfio_queries() {
    const loader = engine.createAssetLoader();
    const asset = loader.createAsset("model.glb");
    const root = asset.getRoot();

    const named: Filament.Entity[] = asset.getEntitiesByName("Cube");
    const first: Filament.Entity = asset.getFirstEntityByName("Cube");
    const prefixed: Filament.Entity[] = asset.getEntitiesByPrefix("Cu");
    const lights: Filament.Entity[] = asset.getLightEntities();
    const renderables: Filament.Entity[] = asset.getRenderableEntities();
    const cameras: Filament.Entity[] = asset.getCameraEntities();
    const wireframe: Filament.Entity = asset.getWireframe();
    const popped: Filament.Entity = asset.popRenderable();
    const uris: string[] = asset.getResourceUris();
    const name: string = asset.getName(root);
    const extras: string = asset.getExtras(root);
    const owner: Filament.Engine = asset.getEngine();
    const instances: Filament.gltfio$FilamentInstance[] = asset.getAssetInstances();

    // Resources can be pulled in by the asset itself or by an explicit ResourceLoader.
    asset.loadResources(() => console.log("done"), (uri: string) => console.log(uri), null, null);
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    const loaded: boolean = resourceLoader.loadResources(asset);
    const begun: boolean = resourceLoader.asyncBeginLoad(asset);
    resourceLoader.asyncUpdateLoad();
    asset.releaseSourceData();

    const instanced = loader.createInstancedAsset("model.glb", [null, null]);
    const instance = instanced.getInstance();
    instance.applyMaterialVariant(0);
    instance.attachSkin(0, root);
    instance.detachSkin(0, root);
    const owningAsset: Filament.gltfio$FilamentAsset = instance.getAsset();
    const materialInstances: Filament.Vector<Filament.MaterialInstance> =
            instance.getMaterialInstances();
    instance.detachMaterialInstances();
    const instanceEntities: Filament.Vector<Filament.Entity> = instance.getEntities();
    const instanceRoot: Filament.Entity = instance.getRoot();

    console.log(named, first, prefixed, lights, renderables, cameras, wireframe, popped,
            uris, name, extras, owner, instances, loaded, begun, owningAsset, materialInstances,
            instanceEntities, instanceRoot);
}

function smoke_gltfio_resource_loader() {
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    const stb: Filament.gltfio$TextureProvider =
            Filament.gltfio$TextureProvider.createStbProvider(engine);
    const ktx2: Filament.gltfio$TextureProvider =
            Filament.gltfio$TextureProvider.createKtx2Provider(engine);
    const webp: Filament.gltfio$TextureProvider =
            Filament.gltfio$TextureProvider.createWebpProvider(engine);
    const ubershader = new Filament.gltfio$UbershaderProvider(engine);
    resourceLoader.addTextureProvider("image/png", stb);
    resourceLoader.addTextureProvider("image/ktx2", ktx2);
    resourceLoader.addTextureProvider("image/webp", webp);
    const bd = new Filament.driver$BufferDescriptor(4);
    resourceLoader.addResourceData("tex.png", bd);
    const has: boolean = resourceLoader.hasResourceData("tex.png");
    const progress: number = resourceLoader.asyncGetLoadProgress();
    const webpSupported: boolean = Filament.gltfio$TextureProvider.isWebpSupported();
    resourceLoader.asyncCancelLoad();
    resourceLoader.evictResourceData();
    const materialsCount: number = ubershader.getMaterialsCount();
    const materials: Filament.Vector<Filament.Material> = ubershader.getMaterials();
    const needsDummyUvs: boolean = ubershader.needsDummyData(Filament.VertexAttribute.UV0);
    console.log(materialsCount, materials, needsDummyUvs);
    ubershader.destroyMaterials();
    console.log(has, progress, webpSupported);
}

// ---------------------------------------------------------------------------
// Viewer / automation harness (gltf_viewer tooling)
// ---------------------------------------------------------------------------

function smoke_viewer() {
    const spec = Filament.AutomationSpec.generateDefaultTestCases();
    const specSize: number = spec.size();
    const specName: string = spec.getName(0);
    const generated: Filament.AutomationSpec|null = Filament.AutomationSpec.generate("[]");
    const auto = Filament.AutomationEngine.createDefault();
    const settings = auto.getSettings();
    const fetched: boolean = spec.get(0, settings);
    const serializer = new Filament.JsonSerializer();
    const json: string = serializer.writeJson(settings);
    const parsed: boolean = serializer.readJson(json, settings);
    const running: boolean = auto.isRunning();
    const testCount: number = auto.testCount();
    const gui = new Filament.ViewerGui(engine, engine.createScene(), engine.createView(), 300);
    gui.keyPressEvent(65);
    gui.delete();
    spec.delete();
    auto.delete();
    console.log(specSize, specName, generated, fetched, json, parsed, running, testCount);
}

// The batch-mode driver: an AutomationEngine steps through its spec one frame at a time, applying
// each test case to the content it is handed.
function smoke_automation_engine() {
    const view = engine.createView();
    const content: Filament.ViewerContent = {
        view,
        renderer: engine.createRenderer(),
        materials: [],
        lightManager: engine.getLightManager(),
        scene: engine.createScene(),
        indirectLight: null,
        sunlight: newEntity(),
        assetLights: [],
    };

    const auto = Filament.AutomationEngine.createDefault();
    const fromJson: Filament.AutomationEngine|null =
            Filament.AutomationEngine.createFromJSON("[]");
    const options: Filament.AutomationEngine$Options = auto.getOptions();
    options.sleepDuration = 0.1;
    options.minFrameCount = 2;
    options.verbose = false;
    options.exportScreenshots = false;
    options.exportSettings = false;
    options.exportFormat = Filament.AutomationEngine$ExportFormat.PPM;
    auto.setOptions(options);

    auto.startRunning();
    auto.startBatchMode();
    auto.tick(engine, content, 0.016);
    auto.applySettings(engine, '{"view":{"bloom":{"enabled":true}}}', content);
    const colorGrading: Filament.ColorGrading = auto.getColorGrading(engine);
    const viewerOptions: Filament.ViewerOptions = auto.getViewerOptions();
    const current: number = auto.currentTest();
    const status: string = auto.getStatusMessage();
    const shouldClose: boolean = auto.shouldClose();
    auto.signalBatchMode();
    auto.stopRunning();
    auto.terminate();
    auto.delete();

    const gui = new Filament.ViewerGui(engine, engine.createScene(), engine.createView(), 300);
    gui.mouseEvent(10, 10, true, 0, false);
    gui.keyDownEvent(65);
    gui.keyUpEvent(65);
    gui.renderUserInterface(0.016, engine.createView(), 1.0);
    const guiSettings: Filament.viewer$Settings = gui.getSettings();
    gui.delete();
    console.log(fromJson, colorGrading, viewerOptions, current, status, shouldClose, guiSettings);
}

// Registry of every smoke test. tsc validates each function body's signatures against the
// embind bindings; add new coverage by appending here (and writing a focused function above).
const SMOKE_TESTS: Array<() => void> = [
    smoke_entity_manager,
    smoke_engine,
    smoke_engine_builder,
    smoke_engine_lifecycle,
    smoke_engine_asset_helpers,
    smoke_camera_frustum,
    smoke_camera_transform,
    smoke_camera_exposure_shift,
    smoke_view,
    smoke_view_bindings,
    smoke_scene,
    smoke_renderer,
    smoke_renderer_frame,
    smoke_transforms,
    smoke_renderables,
    smoke_renderable_appearance,
    smoke_renderable_materials,
    smoke_renderable_geometry,
    smoke_skinning_morphing_buffers,
    smoke_renderable_instance,
    smoke_lights,
    smoke_buffers,
    smoke_texture,
    smoke_material,
    smoke_material_instance_state,
    smoke_tone_mappers,
    smoke_color_grading,
    smoke_indirect_light,
    smoke_skybox,
    smoke_render_target,
    smoke_swap_chain,
    smoke_surface_orientation,
    smoke_ktx2_reader,
    smoke_ktx1_bundle,
    smoke_mesh_reader,
    smoke_camutils,
    smoke_gltfio,
    smoke_gltfio_queries,
    smoke_gltfio_resource_loader,
    smoke_viewer,
    smoke_automation_engine,
];

for (const test of SMOKE_TESTS) {
    test();
}
