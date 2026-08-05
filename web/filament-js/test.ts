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
    const activeEntities: number = em.getActiveEntityCount();
    console.log(id, activeEntities);
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
    console.log(backend, em, fl, supported, instancing, failed, maxEyes, now);
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
    console.log(m5, m6, v5, b, c);
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

function smoke_scene() {
    const scene = engine.createScene();
    const entity = newEntity();
    scene.addEntity(entity);
    scene.addEntities([entity]);
    const entityCount: number = scene.getEntityCount();
    const renderableCount: number = scene.getRenderableCount();
    const lightCount: number = scene.getLightCount();
    const hasEntity: boolean = scene.hasEntity(entity);
    const skybox: Filament.Skybox = scene.getSkybox();
    const ibl: Filament.IndirectLight = scene.getIndirectLight();
    const validScene: boolean = engine.isValidScene(scene);
    scene.remove(entity);
    console.log(entityCount, renderableCount, lightCount, hasEntity, skybox, ibl, validScene);
}

function smoke_renderer() {
    const renderer = engine.createRenderer();
    const userTime: number = renderer.getUserTime();
    renderer.resetUserTime();
    renderer.skipNextFrames(1);
    const skipCount: number = renderer.getFrameToSkipCount();
    const shouldRender: boolean = renderer.shouldRenderFrame();
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

// ---------------------------------------------------------------------------
// Component managers
// ---------------------------------------------------------------------------

function smoke_transforms() {
    const tcm = engine.getTransformManager();
    const entity = newEntity();
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
    Filament.RenderableManager.Builder(1).skinningBones([bone]).build(engine, entity);
    inst.delete();
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

    Filament.LightManager.Builder(Filament.LightManager$Type.SPOT)
        .intensityCandela(1000)
        .build(engine, newEntity());

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
        .build(engine);
    vb.setBufferAt(engine, 0, new Float32Array(9));
    const validVb: boolean = engine.isValidVertexBuffer(vb);

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
    const levels: number = texture.getLevels(engine);
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
        .samples(1)
        .swizzle(Filament.Texture$Swizzle.CHANNEL_0, Filament.Texture$Swizzle.CHANNEL_1,
                Filament.Texture$Swizzle.CHANNEL_2, Filament.Texture$Swizzle.SUBSTITUTE_ONE)
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
    matinst.setColor3Parameter("baseColor", Filament.RgbType.sRGB, v3);
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
        .toneMapping(Filament.ColorGrading$ToneMapping.ACES)
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
        .irradianceSh(3, sh)
        .radianceSh(3, sh)
        .intensity(30000)
        .build(engine);
    ibl.setIntensity(25000);
    const intensity: number = ibl.getIntensity();
    ibl.setRotation(m3);
    const rotation: Filament.mat3 = ibl.getRotation();
    const valid: boolean = engine.isValidIndirectLight(ibl);
    console.log(intensity, rotation, valid);
}

function smoke_skybox() {
    const sky = Filament.Skybox.Builder()
        .color([0, 0, 0, 1])
        .intensity(30000)
        .showSun(true)
        .build(engine);
    sky.setColor([1, 1, 1, 1]);
    const mask: number = sky.getLayerMask();
    sky.setLayerMask(0xff, 0x01);
    const valid: boolean = engine.isValidSkybox(sky);
    console.log(mask, valid);
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
        .build(engine);
    const tex: Filament.Texture = rt.getTexture(Filament.RenderTarget$AttachmentPoint.COLOR);
    const mip: number = rt.getMipLevel(Filament.RenderTarget$AttachmentPoint.COLOR);
    const supported: number = rt.getSupportedColorAttachmentsCount();
    const valid: boolean = engine.isValidRenderTarget(rt);
    console.log(tex, mip, supported, valid);
}

function smoke_swap_chain() {
    const sc = engine.createSwapChain();
    const valid: boolean = engine.isValidSwapChain(sc);
    const srgb: boolean = Filament.SwapChain.isSRGBSwapChainSupported(engine);
    console.log(valid, srgb);
}

// ---------------------------------------------------------------------------
// Geometry / image helpers
// ---------------------------------------------------------------------------

function smoke_surface_orientation() {
    const so = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(new Float32Array(9), 0)
        .tangents(new Float32Array(12), 0)
        .build();
    const quats: Int16Array = so.getQuats(3);
    so.delete();
    console.log(quats);
}

function smoke_ktx2_reader() {
    const reader = new Filament.Ktx2Reader(engine, true);
    reader.requestFormat(Filament.Texture$InternalFormat.RGBA8);
    reader.unrequestFormat(Filament.Texture$InternalFormat.RGBA8);
    console.log(reader);
}

function smoke_ktx1_bundle() {
    const bd = new Filament.driver$BufferDescriptor(4);
    const bundle = new Filament.Ktx1Bundle(bd);
    const mips: number = bundle.getNumMipLevels();
    const arrayLength: number = bundle.getArrayLength();
    const format: Filament.Texture$InternalFormat = bundle.getInternalFormat(true);
    const compressed: boolean = bundle.isCompressed();
    const cubemap: boolean = bundle.isCubemap();
    const info: Filament.KtxInfo = bundle.info();
    const width: number = info.pixelWidth;
    const blob: ArrayBuffer = bundle.getBlob([0, 0, 0]);
    const metadata: string = bundle.getMetadata("key");
    console.log(mips, arrayLength, format, compressed, cubemap, width, blob, metadata);
}

function smoke_mesh_reader() {
    const registry = new Filament.MeshReader$MaterialRegistry();
    const size: number = registry.size();
    const keys: Filament.Vector<string> = registry.keys();
    const bd = new Filament.driver$BufferDescriptor(4);
    const mesh = Filament.MeshReader.loadMeshFromBuffer(engine, bd, registry);
    const renderable: Filament.Entity = mesh.renderable();
    const vb: Filament.VertexBuffer = mesh.vertexBuffer();
    const ib: Filament.IndexBuffer = mesh.indexBuffer();
    console.log(size, keys, renderable, vb, ib);
}

// ---------------------------------------------------------------------------
// Camera manipulator (camutils)
// ---------------------------------------------------------------------------

function smoke_camutils() {
    const manip = new Filament.Camutils$Manipulator$Builder()
        .viewport(1024, 768)
        .orbitHomePosition(0, 0, 1)
        .targetPosition(0, 0, 0)
        .upVector(0, 1, 0)
        .build(Filament.Camutils$Mode.ORBIT);
    manip.attach(canvas);
    manip.update(0.016);
    const camera: Filament.Camera = engine.createCamera(newEntity());
    camera.setLookAt(manip);
    manip.detach(canvas);
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
    animator.updateBoneMatrices();
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
    Filament.gltfio$AssetLoader.destroy(loader);
    console.log(entities, root, box, skinNames, variants, animCount);
}

function smoke_gltfio_resource_loader() {
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    const stb = new Filament.gltfio$StbProvider(engine);
    const ktx2 = new Filament.gltfio$Ktx2Provider(engine);
    const webp = new Filament.gltfio$WebpProvider(engine);
    const ubershader = new Filament.gltfio$UbershaderProvider(engine);
    resourceLoader.addStbProvider("image/png", stb);
    resourceLoader.addKtx2Provider("image/ktx2", ktx2);
    resourceLoader.addWebpProvider("image/webp", webp);
    const bd = new Filament.driver$BufferDescriptor(4);
    resourceLoader.addResourceData("tex.png", bd);
    const has: boolean = resourceLoader.hasResourceData("tex.png");
    const progress: number = resourceLoader.asyncGetLoadProgress();
    const webpSupported: boolean = Filament.gltfio$WebpProvider.isWebpSupported();
    resourceLoader.asyncCancelLoad();
    resourceLoader.evictResourceData();
    ubershader.destroyMaterials();
    console.log(has, progress, webpSupported);
}

// ---------------------------------------------------------------------------
// Viewer / automation harness (gltf_viewer tooling)
// ---------------------------------------------------------------------------

function smoke_viewer() {
    const spec = Filament.AutomationSpec.generateDefaultTestCases();
    const specSize: number = spec.size();
    const auto = Filament.AutomationEngine.createDefault();
    const settings = auto.getSettings();
    const json: string = new Filament.JsonSerializer().writeJson(settings);
    const running: boolean = auto.isRunning();
    const testCount: number = auto.testCount();
    const gui = new Filament.ViewerGui(engine, engine.createScene(), engine.createView(), 300);
    gui.keyPressEvent(65);
    gui.delete();
    spec.delete();
    auto.delete();
    console.log(specSize, json, running, testCount);
}

// Registry of every smoke test. tsc validates each function body's signatures against the
// embind bindings; add new coverage by appending here (and writing a focused function above).
const SMOKE_TESTS: Array<() => void> = [
    smoke_entity_manager,
    smoke_engine,
    smoke_camera_frustum,
    smoke_camera_exposure_shift,
    smoke_view,
    smoke_scene,
    smoke_renderer,
    smoke_transforms,
    smoke_renderables,
    smoke_renderable_geometry,
    smoke_skinning_morphing_buffers,
    smoke_renderable_instance,
    smoke_lights,
    smoke_buffers,
    smoke_texture,
    smoke_material,
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
    smoke_gltfio_resource_loader,
    smoke_viewer,
];

for (const test of SMOKE_TESTS) {
    test();
}
