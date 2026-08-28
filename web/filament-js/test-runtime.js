/*
* Copyright (C) 2026 The Android Open Source Project
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
 * Runtime counterpart to test.ts.
 *
 * test.ts is only type-checked, never executed, so it proves the declarations in filament.d.ts
 * compile — not that the bindings behind them exist or marshal correctly. This script loads the
 * built module under Node and exercises the same surface against a real engine, using the NOOP
 * backend. A missing registration, a wrong embind signature, a builder setter that forgets to
 * return the builder, or a value_object field that does not round-trip fails here instead of in a
 * browser.
 *
 * The sections below follow test.ts one for one. What is missing from them is the part of the API
 * that needs a GPU: pixel readback, mipmap generation, image decoding and picking all need a real
 * backend, and live in test-browser.html instead.
 *
 * Usage:
 *   node test-runtime.js [path/to/filament.js]
 *
 * Defaults to the standard wasm release output. Exits non-zero on the first failure.
 */

'use strict';

const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert');

const ROOT = path.join(__dirname, '..', '..');

const modulePath = path.resolve(process.argv[2] ||
    path.join(ROOT, 'out', 'cmake-wasm-release', 'web', 'filament-js', 'filament.js'));

if (!fs.existsSync(modulePath)) {
    console.error(`No module at ${modulePath}. Build it with: ./build.sh -p wasm release`);
    process.exit(1);
}

// filament.js declares `var Filament = ...` at global scope, so it has to be loaded by indirect
// eval rather than require(). That scope has none of CommonJS's module-locals, and emscripten's
// Node branch uses all three to find filament.wasm, so hand them over first.
globalThis.__filename = modulePath;
globalThis.__dirname = path.dirname(modulePath);
globalThis.require = require;
// extensions.js passes the GL context to the backend through globals on `window`. The NOOP backend
// never reads them, but Engine.execute writes them unconditionally, so give it somewhere to write.
globalThis.window = globalThis;
(0, eval)(fs.readFileSync(modulePath, 'utf8'));

// Committed assets, used where a binding needs real content to chew on. test_material.filamat is
// a lit opaque material with twelve scalar and vector parameters; textured.filamat is the only
// committed material with sampler parameters; lightroom_ibl.ktx is a cubemap with SH metadata.
const MATERIAL = 'filament/test/test_material.filamat';
const TEXTURED_MATERIAL = 'docs/web/assets/suzanne/textured.filamat';
const IBL_KTX = 'libs/ktxreader/tests/lightroom_ibl.ktx';

function asset(relativePath) {
    return new Uint8Array(fs.readFileSync(path.join(ROOT, relativePath)));
}

// A single-triangle glTF with its vertex data inline, so gltfio can be exercised without pulling
// in a model from third_party.
function triangleGltf() {
    const positions = new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]);
    const base64 = Buffer.from(new Uint8Array(positions.buffer)).toString('base64');
    return new TextEncoder().encode(JSON.stringify({
        asset: { version: '2.0' },
        scene: 0,
        scenes: [{ nodes: [0] }],
        nodes: [{ mesh: 0, name: 'triangle' }],
        meshes: [{ primitives: [{ attributes: { POSITION: 0 } }] }],
        accessors: [{
            bufferView: 0, componentType: 5126, count: 3, type: 'VEC3',
            min: [0, 0, 0], max: [1, 1, 0]
        }],
        bufferViews: [{ buffer: 0, byteOffset: 0, byteLength: positions.byteLength }],
        buffers: [{
            byteLength: positions.byteLength,
            uri: 'data:application/octet-stream;base64,' + base64
        }],
    }));
}

const TESTS = [];
function test(name, fn) {
    TESTS.push([name, fn]);
}

// Most tests want an engine and nothing else. NOOP needs no canvas context, so Engine.create only
// reads the id off the object it is handed.
let Filament = null;
function withEngine(fn) {
    const engine = Filament.Engine.create({ id: 'runtime-test-canvas' },
        { backend: Filament.Backend.NOOP });
    try {
        return fn(engine);
    } finally {
        Filament.Engine.destroy(engine);
    }
}

function newEntity() {
    return Filament.EntityManager.get().create();
}

function material(engine) {
    return engine.createMaterial(asset(MATERIAL));
}

// Textures default to UPLOADABLE | SAMPLEABLE, which the Texture$Usage enum spells out one flag
// at a time on the JS side (TypeScript clients get the folded TextureUsage constants instead).
function textureUsage(...names) {
    return names.reduce((bits, name) => bits | Filament.Texture$Usage[name].value, 0);
}

// ---------------------------------------------------------------------------
// Engine$Builder
// ---------------------------------------------------------------------------

test('Engine$Builder is registered with its full setter surface', () => {
    assert.strictEqual(typeof Filament.Engine$Builder, 'function');
    for (const method of ['backend', 'config', 'featureLevel', 'feature', 'colorGrading',
        'build', '_build']) {
        assert.strictEqual(typeof Filament.Engine$Builder.prototype[method], 'function',
            `Engine$Builder.${method} is not registered`);
    }
});

test('Engine$Config round-trips through the builder', () => {
    const config = Filament.Engine.createDefaultConfig();

    // Defaults must match Engine::Config, since every binding layer mirrors them.
    assert.strictEqual(config.jobSystemThreadCount, 0);
    assert.strictEqual(config.stereoscopicEyeCount, 2);
    assert.strictEqual(config.resourceAllocatorCacheMaxAge, 1);
    assert.strictEqual(config.disableParallelShaderCompile, false);
    assert.strictEqual(config.disableHandleUseAfterFreeCheck, false);
    assert.strictEqual(config.assertNativeWindowIsValid, false);

    config.perRenderPassArenaSizeMB = 5;
    config.jobSystemThreadCount = 1;
    config.disableParallelShaderCompile = true;
    config.disableHandleUseAfterFreeCheck = true;
    config.stereoscopicEyeCount = 2;
    config.sharedUboInitialSizeInBytes = 32768;

    const builder = new Filament.Engine$Builder();
    builder.backend(Filament.Backend.NOOP);
    builder.config(config);
    const engine = builder.build();
    assert.ok(engine, 'Engine$Builder.build returned null');

    const readBack = engine.getConfig();
    assert.strictEqual(readBack.perRenderPassArenaSizeMB, 5);
    assert.strictEqual(readBack.jobSystemThreadCount, 1);
    assert.strictEqual(readBack.disableParallelShaderCompile, true);
    assert.strictEqual(readBack.disableHandleUseAfterFreeCheck, true);
    assert.strictEqual(readBack.sharedUboInitialSizeInBytes, 32768);
    assert.strictEqual(engine.getBackend(), Filament.Backend.NOOP);

    Filament.Engine.destroy(engine);
});

test('Engine$Builder accepts feature flags and a ColorGrading$Builder', () => {
    const builder = new Filament.Engine$Builder();
    builder.backend(Filament.Backend.NOOP);
    builder.featureLevel(Filament.FeatureLevel.FEATURE_LEVEL_1);
    // Unknown names are ignored by the engine; this asserts the string marshalling works.
    builder.feature('backend.disable_parallel_shader_compile', true);
    builder.colorGrading(Filament.ColorGrading.Builder());
    const engine = builder.build();
    assert.ok(engine);
    Filament.Engine.destroy(engine);
});

test('Engine.create forwards options through to the builder', () => {
    const config = Filament.Engine.createDefaultConfig();
    config.driverHandleArenaSizeMB = 4;

    // NOOP needs no WebGL context, so Engine.create skips canvas.getContext entirely and only
    // reads the id. That is the whole non-GL path of Engine.create, exercised for real.
    const engine = Filament.Engine.create({ id: 'runtime-test-canvas' }, {
        backend: Filament.Backend.NOOP,
        featureLevel: Filament.FeatureLevel.FEATURE_LEVEL_1,
        features: { 'backend.disable_parallel_shader_compile': true },
        colorGrading: Filament.ColorGrading.Builder(),
    }, config);

    assert.strictEqual(engine.getBackend(), Filament.Backend.NOOP);
    assert.strictEqual(engine.getConfig().driverHandleArenaSizeMB, 4);
    assert.strictEqual(engine.canvasId, '#runtime-test-canvas');
    Filament.Engine.destroy(engine);
});

// ---------------------------------------------------------------------------
// Engine
// ---------------------------------------------------------------------------

test('Engine exposes its feature level, clocks and instancing toggle', () => withEngine((engine) => {
    assert.strictEqual(engine.getSupportedFeatureLevel(), Filament.FeatureLevel.FEATURE_LEVEL_1);
    assert.strictEqual(engine.getActiveFeatureLevel(), Filament.FeatureLevel.FEATURE_LEVEL_1);
    engine.setActiveFeatureLevel(Filament.FeatureLevel.FEATURE_LEVEL_1);

    engine.setAutomaticInstancingEnabled(true);
    assert.strictEqual(engine.isAutomaticInstancingEnabled(), true);
    engine.setAutomaticInstancingEnabled(false);
    assert.strictEqual(engine.isAutomaticInstancingEnabled(), false);

    assert.strictEqual(engine.hasUnrecoverableFailure(), false);
    assert.ok(Filament.Engine.getMaxStereoscopicEyes() >= 1);
    assert.ok(Filament.Engine.getSteadyClockTimeNano() > 0);

    engine.enableAccurateTranslations();
    engine.unprotected();
    engine.flush();
    engine.flushAndWait();
    engine.execute();
}));

test('Engine feature flags can be read and written after construction', () => withEngine((engine) => {
    // The flag Engine$Builder.feature sets before construction is reachable afterwards too.
    const name = 'backend.disable_parallel_shader_compile';
    assert.strictEqual(engine.hasFeatureFlag(name), true, `${name} is not a known flag`);
    assert.strictEqual(typeof engine.getFeatureFlag(name), 'boolean');

    const accepted = engine.setFeatureFlag(name, true);
    assert.strictEqual(typeof accepted, 'boolean');
    if (accepted) {
        assert.strictEqual(engine.getFeatureFlag(name), true, 'setFeatureFlag did not take');
    }

    // An unknown flag is absent rather than false, which is why the getter is optional-shaped.
    assert.strictEqual(engine.hasFeatureFlag('no.such.flag'), false);
    assert.strictEqual(engine.getFeatureFlag('no.such.flag'), undefined);
    assert.strictEqual(engine.setFeatureFlag('no.such.flag', true), false);
}));

test('Engine hands back every object type and validates it', () => withEngine((engine) => {
    const swapChain = engine.createSwapChain();
    const renderer = engine.createRenderer();
    const view = engine.createView();
    const scene = engine.createScene();
    const texture = Filament.Texture.Builder().width(1).height(1).build(engine);
    const renderTarget = Filament.RenderTarget.Builder()
        .texture(Filament.RenderTarget$AttachmentPoint.COLOR, Filament.Texture.Builder()
            .width(8).height(8)
            .sampler(Filament.Texture$Sampler.SAMPLER_2D)
            .format(Filament.Texture$InternalFormat.RGBA8)
            .usage(textureUsage('COLOR_ATTACHMENT', 'SAMPLEABLE'))
            .build(engine))
        .build(engine);
    const colorGrading = Filament.ColorGrading.Builder().build(engine);
    const indirectLight = Filament.IndirectLight.Builder().build(engine);
    const skybox = Filament.Skybox.Builder().color([0, 0, 0, 1]).build(engine);
    const vertexBuffer = Filament.VertexBuffer.Builder().vertexCount(1).bufferCount(1)
        .attribute(Filament.VertexAttribute.POSITION, 0,
            Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .build(engine);
    const indexBuffer = Filament.IndexBuffer.Builder().indexCount(1).build(engine);
    const mat = material(engine);
    const matinst = mat.createInstance();

    assert.ok(engine.isValidSwapChain(swapChain));
    assert.ok(engine.isValidRenderer(renderer));
    assert.ok(engine.isValidView(view));
    assert.ok(engine.isValidScene(scene));
    assert.ok(engine.isValidTexture(texture));
    assert.ok(engine.isValidRenderTarget(renderTarget));
    assert.ok(engine.isValidColorGrading(colorGrading));
    assert.ok(engine.isValidIndirectLight(indirectLight));
    assert.ok(engine.isValidSkybox(skybox));
    assert.ok(engine.isValidVertexBuffer(vertexBuffer));
    assert.ok(engine.isValidIndexBuffer(indexBuffer));
    assert.ok(engine.isValidMaterial(mat));
    assert.ok(engine.isValidMaterialInstance(mat, matinst));
    assert.ok(engine.isValidExpensiveMaterialInstance(matinst));

    const cameraEntity = newEntity();
    engine.createCamera(cameraEntity);
    assert.ok(engine.getCameraComponent(cameraEntity), 'getCameraComponent returned null');

    engine.destroyMaterialInstance(matinst);
    engine.destroyMaterial(mat);
    engine.destroyIndexBuffer(indexBuffer);
    engine.destroyVertexBuffer(vertexBuffer);
    engine.destroySkybox(skybox);
    engine.destroyIndirectLight(indirectLight);
    engine.destroyColorGrading(colorGrading);
    engine.destroyRenderTarget(renderTarget);
    engine.destroyTexture(texture);
    engine.destroyScene(scene);
    engine.destroyView(view);
    engine.destroyRenderer(renderer);
    engine.destroySwapChain(swapChain);
    engine.destroyCameraComponent(cameraEntity);
    engine.destroyEntity(cameraEntity);

    // The validity checks have to notice the destruction, not just the creation.
    assert.strictEqual(engine.isValidSwapChain(swapChain), false);
    assert.strictEqual(engine.isValidTexture(texture), false);
}));

test('Engine creates and waits on a fence', () => withEngine((engine) => {
    const fence = engine.createFence();
    assert.ok(engine.isValidFence(fence));
    // Any non-zero timeout aborts on a build without threads, so 0 is the only supported wait.
    const status = fence.wait(Filament.Fence$Mode.FLUSH, 0);
    assert.ok(status === Filament.FenceStatus.CONDITION_SATISFIED ||
        status === Filament.FenceStatus.TIMEOUT_EXPIRED, `unexpected status ${status}`);
    engine.destroyFence(fence);
    assert.strictEqual(engine.isValidFence(fence), false);
}));

test('Engine decodes a KTX1 environment into an IBL and a skybox', () => withEngine((engine) => {
    const ibl = engine.createIblFromKtx1(asset(IBL_KTX));
    assert.ok(ibl.getIntensity() > 0);
    // createIblFromKtx1 parses the "sh" metadata into 27 floats hanging off the IBL.
    assert.strictEqual(ibl.shfloats.length, 27);

    const skybox = engine.createSkyFromKtx1(asset(IBL_KTX));
    assert.ok(skybox.getTexture(), 'skybox has no texture');
}));

// ---------------------------------------------------------------------------
// EntityManager
// ---------------------------------------------------------------------------

test('EntityManager creates and destroys entities', () => {
    const em = Filament.EntityManager.get();
    const before = em.getEntityCount();
    const entity = em.create();
    assert.ok(entity.getId() > 0);
    assert.strictEqual(em.getEntityCount(), before + 1);
    assert.ok(Filament.EntityManager.getMaxEntityCount() > 0);
    em.advanceEpoch();
    em.destroy(entity);
    assert.strictEqual(em.getEntityCount(), before);
    // getActiveEntityCount is compiled in only when FILAMENT_UTILS_TRACK_ENTITIES is set, which
    // the release build does not do; filament.d.ts documents that it has to be feature-detected.
    assert.strictEqual(typeof em.getActiveEntityCount, 'undefined');
});

// ---------------------------------------------------------------------------
// Camera + frustum
// ---------------------------------------------------------------------------

test('Camera projections and the view basis round-trip', () => withEngine((engine) => {
    const camera = engine.createCamera(newEntity());
    camera.setProjection(Filament.Camera$Projection.PERSPECTIVE, -1, 1, -1, 1, 0.1, 100);
    assert.strictEqual(camera.getNear(), 0.1);
    assert.strictEqual(camera.getCullingFar(), 100);
    assert.strictEqual(camera.getProjectionMatrix().length, 16);
    assert.strictEqual(camera.getCullingProjectionMatrix().length, 16);
    assert.strictEqual(Filament.Camera.inverseProjection(camera.getProjectionMatrix()).length, 16);

    camera.setProjectionFov(45, 1.0, 0.1, 100, Filament.Camera$Fov.HORIZONTAL);
    assert.ok(Math.abs(camera.getFieldOfViewInDegrees(Filament.Camera$Fov.HORIZONTAL) - 45) < 1e-3);
    camera.setLensProjection(0.05, 1, 0.1, 100);
    camera.setCustomProjection([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1], 0.1, 100);
    // One projection per eye; Engine$Config.stereoscopicEyeCount defaults to two.
    const identity4 = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
    camera.setCustomEyeProjection([identity4, identity4], identity4, 0.1, 100);
    camera.setEyeModelMatrix(0, [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);

    // An identity model matrix leaves the camera at the origin looking down -Z.
    camera.setModelMatrix([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    assert.deepStrictEqual(Array.from(camera.getPosition()), [0, 0, 0]);
    assert.deepStrictEqual(Array.from(camera.getLeftVector()), [1, 0, 0]);
    assert.deepStrictEqual(Array.from(camera.getUpVector()), [0, 1, 0]);
    assert.deepStrictEqual(Array.from(camera.getForwardVector()).map((v) => v === 0 ? 0 : v), [0, 0, -1]);
    assert.strictEqual(camera.getModelMatrix().length, 16);
    assert.strictEqual(camera.getViewMatrix().length, 16);

    camera.lookAt([0, 0, 1], [0, 0, 0], [0, 1, 0]);
    camera.setScaling([2, 2]);
    assert.deepStrictEqual(Array.from(camera.getScaling()).slice(0, 2), [2, 2]);
    assert.ok(camera.getEntity().getId() > 0);
}));

test('Camera exposure controls round-trip', () => withEngine((engine) => {
    const camera = engine.createCamera(newEntity());
    camera.setExposure(16, 1 / 125, 100);
    assert.strictEqual(camera.getAperture(), 16);
    assert.ok(Math.abs(camera.getShutterSpeed() - 1 / 125) < 1e-5);
    assert.strictEqual(camera.getSensitivity(), 100);
    camera.setExposureDirect(1.0);

    camera.setFocusDistance(5);
    assert.strictEqual(camera.getFocusDistance(), 5);
    camera.setShift([0.1, 0.2]);
    const shift = camera.getShift();
    assert.ok(Math.abs(shift[0] - 0.1) < 1e-6 && Math.abs(shift[1] - 0.2) < 1e-6);
    assert.ok(camera.getFocalLength() > 0);
    assert.ok(Filament.Camera.computeEffectiveFov(45, 5) > 0);
    assert.ok(Filament.Camera.computeEffectiveFocalLength(0.05, 1) > 0);
}));

test('Frustum intersects the volumes in front of the camera', () => withEngine((engine) => {
    const camera = engine.createCamera(newEntity());
    camera.setProjection(Filament.Camera$Projection.PERSPECTIVE, -1, 1, -1, 1, 0.1, 100);
    const frustum = camera.getFrustum();
    assert.strictEqual(frustum.getNormalizedPlane(Filament.Frustum$Plane.BOTTOM).length, 4);
    // The camera looks down -Z, so a unit box one unit ahead is inside and one behind is not.
    assert.strictEqual(frustum.intersectsBox({ center: [0, 0, -1], halfExtent: [1, 1, 1] }), true);
    assert.strictEqual(frustum.intersectsBox({ center: [0, 0, 50], halfExtent: [1, 1, 1] }), false);
    assert.strictEqual(frustum.intersectsSphere([0, 0, -1, 1]), true);
    assert.strictEqual(frustum.intersectsSphere([0, 0, 50, 1]), false);

    const standalone = new Filament.Frustum(camera.getProjectionMatrix());
    standalone.setProjection(camera.getProjectionMatrix());
    assert.ok(standalone);
}));

// ---------------------------------------------------------------------------
// View / Scene / Renderer
// ---------------------------------------------------------------------------

test('View toggles and scalar getters round-trip', () => withEngine((engine) => {
    const view = engine.createView();
    view.setName('runtime view');
    assert.strictEqual(view.getName(), 'runtime view');

    view.setViewport([0, 0, 64, 32]);
    assert.deepStrictEqual(Array.from(view.getViewport()), [0, 0, 64, 32]);

    for (const [set, get, value] of [
        ['setShadowingEnabled', 'isShadowingEnabled', false],
        ['setPostProcessingEnabled', 'isPostProcessingEnabled', false],
        ['setScreenSpaceRefractionEnabled', 'isScreenSpaceRefractionEnabled', false],
        ['setFrustumCullingEnabled', 'isFrustumCullingEnabled', false],
        ['setFrontFaceWindingInverted', 'isFrontFaceWindingInverted', true],
        ['setStencilBufferEnabled', 'isStencilBufferEnabled', true],
        ['setTransparentPickingEnabled', 'isTransparentPickingEnabled', true]]) {
        view[set](value);
        assert.strictEqual(view[get](), value, `View.${get} did not return what ${set} was given`);
    }

    view.setShadowType(Filament.View$ShadowType.VSM);
    assert.strictEqual(view.getShadowType(), Filament.View$ShadowType.VSM);
    view.setBlendMode(Filament.View$BlendMode.TRANSLUCENT);
    assert.strictEqual(view.getBlendMode(), Filament.View$BlendMode.TRANSLUCENT);
    view.setDithering(Filament.View$Dithering.NONE);
    assert.strictEqual(view.getDithering(), Filament.View$Dithering.NONE);
    view.setAntiAliasing(Filament.View$AntiAliasing.NONE);
    assert.strictEqual(view.getAntiAliasing(), Filament.View$AntiAliasing.NONE);
    view.setAmbientOcclusion(Filament.View$AmbientOcclusion.SSAO);
    assert.strictEqual(view.getAmbientOcclusion(), Filament.View$AmbientOcclusion.SSAO);

    view.setVisibleLayers(0xff, 0x04);
    assert.strictEqual(view.getVisibleLayers(), 0x04);
    view.setLayerEnabled(0, true);
    assert.strictEqual(view.getVisibleLayers(), 0x05);

    view.setChannelDepthClearEnabled(0, true);
    assert.strictEqual(view.isChannelDepthClearEnabled(0), true);

    view.setMaterialGlobal(0, [1, 2, 3, 4]);
    assert.deepStrictEqual(Array.from(view.getMaterialGlobal(0)), [1, 2, 3, 4]);

    view.setGridSize(4);
    assert.strictEqual(view.getGridSize(), 4);
    assert.ok(view.getEffectiveGridSize() >= 0);
    view.setDynamicLightingOptions(5, 100);
    view.setSampleCount(1);
    assert.strictEqual(view.getSampleCount(), 1);
    assert.strictEqual(typeof view.getVisibleRenderableCount(), 'number');
    assert.strictEqual(view.getLastDynamicResolutionScale().length, 2);
    assert.ok(view.getFogEntity().getId() > 0);
    view.clearFrameHistory(engine);
}));

test('View option structs round-trip field for field', () => withEngine((engine) => {
    const view = engine.createView();

    view.setBloomOptions({ enabled: true, strength: 0.25, resolution: 256 });
    const bloom = view.getBloomOptions();
    assert.strictEqual(bloom.enabled, true);
    assert.ok(Math.abs(bloom.strength - 0.25) < 1e-6);
    assert.strictEqual(bloom.resolution, 256);

    view.setFogOptions({ enabled: true, distance: 10, density: 0.5 });
    const fog = view.getFogOptions();
    assert.strictEqual(fog.enabled, true);
    assert.strictEqual(fog.distance, 10);

    view.setVignetteOptions({ enabled: true, midPoint: 0.4, roundness: 0.6 });
    assert.strictEqual(view.getVignetteOptions().enabled, true);

    view.setDepthOfFieldOptions({ enabled: true, cocScale: 2 });
    assert.strictEqual(view.getDepthOfFieldOptions().cocScale, 2);

    view.setAmbientOcclusionOptions({ enabled: true, radius: 0.5, power: 2 });
    const ao = view.getAmbientOcclusionOptions();
    assert.strictEqual(ao.enabled, true);
    assert.ok(Math.abs(ao.radius - 0.5) < 1e-6);

    view.setGuardBandOptions({ enabled: true });
    assert.strictEqual(view.getGuardBandOptions().enabled, true);

    view.setStereoscopicOptions({ enabled: false });
    assert.strictEqual(view.getStereoscopicOptions().enabled, false);

    view.setTemporalAntiAliasingOptions({ enabled: true, feedback: 0.5, filterWidth: 1.5 });
    const taa = view.getTemporalAntiAliasingOptions();
    assert.strictEqual(taa.enabled, true);
    assert.ok(Math.abs(taa.feedback - 0.5) < 1e-6);

    view.setScreenSpaceReflectionsOptions({ enabled: true, thickness: 0.25 });
    assert.strictEqual(view.getScreenSpaceReflectionsOptions().enabled, true);

    view.setMultiSampleAntiAliasingOptions({ enabled: true, sampleCount: 4 });
    assert.strictEqual(view.getMultiSampleAntiAliasingOptions().enabled, true);

    view.setDynamicResolutionOptions({ enabled: true, minScale: [1, 1], maxScale: [1, 1] });
    assert.strictEqual(view.getDynamicResolutionOptions().enabled, true);

    view.setRenderQuality({ hdrColorBuffer: Filament.View$QualityLevel.HIGH });
    assert.strictEqual(view.getRenderQuality().hdrColorBuffer, Filament.View$QualityLevel.HIGH);

    view.setVsmShadowOptions({ anisotropy: 1, mipmapping: true, msaaSamples: 2 });
    const vsm = view.getVsmShadowOptions();
    assert.strictEqual(vsm.anisotropy, 1);
    assert.strictEqual(vsm.mipmapping, true);

    view.setSoftShadowOptions({ penumbraScale: 2, penumbraRatioScale: 3 });
    const soft = view.getSoftShadowOptions();
    assert.strictEqual(soft.penumbraScale, 2);
    assert.strictEqual(soft.penumbraRatioScale, 3);
}));

test('View holds on to the objects it is bound to', () => withEngine((engine) => {
    const view = engine.createView();
    assert.strictEqual(view.hasCamera(), false);

    const scene = engine.createScene();
    const camera = engine.createCamera(newEntity());
    const colorGrading = Filament.ColorGrading.Builder().build(engine);
    const renderTarget = Filament.RenderTarget.Builder()
        .texture(Filament.RenderTarget$AttachmentPoint.COLOR, Filament.Texture.Builder()
            .width(8).height(8)
            .sampler(Filament.Texture$Sampler.SAMPLER_2D)
            .format(Filament.Texture$InternalFormat.RGBA8)
            .usage(textureUsage('COLOR_ATTACHMENT', 'SAMPLEABLE'))
            .build(engine))
        .build(engine);
    view.setScene(scene);
    view.setCamera(camera);
    view.setColorGrading(colorGrading);
    view.setRenderTarget(renderTarget);

    assert.strictEqual(view.hasCamera(), true);
    assert.ok(view.getScene(), 'getScene returned null');
    assert.ok(view.getCamera(), 'getCamera returned null');
    assert.ok(view.getColorGrading(), 'getColorGrading returned null');
    assert.ok(view.getRenderTarget(), 'getRenderTarget returned null');
}));

test('Scene counts what it holds', () => withEngine((engine) => {
    const scene = engine.createScene();
    assert.strictEqual(scene.getEntityCount(), 0);

    const entity = newEntity();
    scene.addEntity(entity);
    assert.strictEqual(scene.getEntityCount(), 1);
    assert.strictEqual(scene.hasEntity(entity), true);

    const more = [newEntity(), newEntity()];
    scene.addEntities(more);
    assert.strictEqual(scene.getEntityCount(), 3);
    scene.removeEntities(more);
    assert.strictEqual(scene.getEntityCount(), 1);
    scene.remove(entity);
    assert.strictEqual(scene.getEntityCount(), 0);

    // A light is an entity with a light component, and is counted twice over.
    const lightEntity = newEntity();
    Filament.LightManager.Builder(Filament.LightManager$Type.POINT).build(engine, lightEntity);
    scene.addEntity(lightEntity);
    assert.strictEqual(scene.getLightCount(), 1);
    assert.strictEqual(scene.getRenderableCount(), 0);

    const skybox = Filament.Skybox.Builder().color([0, 0, 0, 1]).build(engine);
    const indirectLight = Filament.IndirectLight.Builder().build(engine);
    scene.setSkybox(skybox);
    scene.setIndirectLight(indirectLight);
    assert.ok(scene.getSkybox(), 'getSkybox returned null');
    assert.ok(scene.getIndirectLight(), 'getIndirectLight returned null');
    scene.setSkybox(null);
    scene.setIndirectLight(null);
}));

test('Renderer runs a frame and keeps its clocks', () => withEngine((engine) => {
    const renderer = engine.createRenderer();
    const swapChain = engine.createSwapChain();
    const view = engine.createView();
    view.setScene(engine.createScene());
    view.setCamera(engine.createCamera(newEntity()));
    view.setViewport([0, 0, 16, 16]);

    renderer.render(swapChain, view);
    assert.strictEqual(renderer.beginFrame(swapChain), true);
    renderer.renderView(view);
    renderer.endFrame();

    // Standalone rendering draws into the view's own render target rather than a swap chain.
    view.setRenderTarget(Filament.RenderTarget.Builder()
        .texture(Filament.RenderTarget$AttachmentPoint.COLOR,
            Filament.Texture.Builder().width(8).height(8)
                .sampler(Filament.Texture$Sampler.SAMPLER_2D)
                .format(Filament.Texture$InternalFormat.RGBA8)
                .usage(textureUsage('COLOR_ATTACHMENT', 'SAMPLEABLE'))
                .build(engine))
        .build(engine));
    renderer.renderStandaloneView(view);

    assert.ok(renderer.getUserTime() >= 0);
    assert.ok(renderer.getMaterialTime() >= 0);
    renderer.resetUserTime();
    renderer.setMaterialTimeEpoch(0);

    renderer.skipNextFrames(2);
    assert.strictEqual(renderer.getFrameToSkipCount(), 2);
    // Both of these read the same frame skipper, so they have to disagree.
    assert.strictEqual(renderer.hasGpuFallenBehind(), !renderer.shouldRenderFrame());

    const now = Filament.Engine.getSteadyClockTimeNano();
    renderer.setPresentationTime(now);
    renderer.setDesiredPresentationTime(now);
    renderer.setRenderingDeadline(now);
    renderer.setVsyncTime(now);
    renderer.skipFrame(now);
    renderer.pauseRenderThread(0);
    renderer.copyFrame(swapChain, [0, 0, 8, 8], [0, 0, 8, 8], 0);
}));

test('Renderer clear options round-trip', () => withEngine((engine) => {
    const renderer = engine.createRenderer();
    renderer.setClearOptions({ clearColor: [0.25, 0.5, 0.75, 1], clear: true, discard: false });
    const options = renderer.getClearOptions();
    assert.deepStrictEqual(Array.from(options.clearColor), [0.25, 0.5, 0.75, 1]);
    assert.strictEqual(options.clear, true);
    assert.strictEqual(options.discard, false);
}));

// ---------------------------------------------------------------------------
// Component managers
// ---------------------------------------------------------------------------

test('TransformManager composes a parent-child hierarchy', () => withEngine((engine) => {
    const tcm = engine.getTransformManager();
    const parent = newEntity();
    const child = newEntity();
    tcm.create(parent);
    tcm.create(child);
    assert.strictEqual(tcm.hasComponent(parent), true);

    const parentInstance = tcm.getInstance(parent);
    const childInstance = tcm.getInstance(child);

    // A translation of (1, 2, 3), column-major.
    const translate = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 2, 3, 1];
    tcm.setTransform(parentInstance, translate);
    assert.deepStrictEqual(Array.from(tcm.getTransform(parentInstance)), translate);

    tcm.setParent(childInstance, parentInstance);
    assert.strictEqual(tcm.getParent(childInstance).getId(), parent.getId());
    assert.strictEqual(tcm.getChildCount(parentInstance), 1);
    assert.strictEqual(tcm.getChildren(parentInstance).size(), 1);
    // The child inherits the parent's translation through the world transform.
    assert.deepStrictEqual(Array.from(tcm.getWorldTransform(childInstance)), translate);

    tcm.openLocalTransformTransaction();
    tcm.setTransform(childInstance, translate);
    tcm.commitLocalTransformTransaction();

    tcm.setAccurateTranslationsEnabled(true);
    assert.strictEqual(tcm.isAccurateTranslationsEnabled(), true);

    childInstance.delete();
    parentInstance.delete();
    tcm.destroy(child);
    tcm.destroy(parent);
    assert.strictEqual(tcm.hasComponent(child), false);
}));

test('RenderableManager$Builder options survive into the instance', () => withEngine((engine) => {
    const entity = newEntity();
    const box = { center: [0, 0, 0], halfExtent: [2, 2, 2] };
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
        .instances(2)
        .build(engine, entity);

    const rm = engine.getRenderableManager();
    assert.strictEqual(rm.hasComponent(entity), true);
    const instance = rm.getInstance(entity);

    assert.deepStrictEqual(Array.from(rm.getAxisAlignedBoundingBox(instance).halfExtent),
        [2, 2, 2]);
    assert.strictEqual(rm.getPriority(instance), 4);
    assert.strictEqual(rm.getChannel(instance), 1);
    assert.strictEqual(rm.isCullingEnabled(instance), true);
    assert.strictEqual(rm.isShadowCaster(instance), true);
    assert.strictEqual(rm.isShadowReceiver(instance), true);
    assert.strictEqual(rm.isScreenSpaceContactShadowsEnabled(instance), false);
    assert.strictEqual(rm.getFogEnabled(instance), true);
    assert.strictEqual(rm.getLightChannel(instance, 0), true);
    assert.strictEqual(rm.getBlendOrderAt(instance, 0), 1);
    assert.strictEqual(rm.isGlobalBlendOrderEnabledAt(instance, 0), true);
    assert.strictEqual(rm.getInstanceCount(instance), 2);
    assert.strictEqual(rm.getPrimitiveCount(instance), 1);

    // The same knobs, now through the instance-side setters.
    rm.setAxisAlignedBoundingBox(instance, { center: [0, 0, 0], halfExtent: [1, 1, 1] });
    assert.deepStrictEqual(Array.from(rm.getAxisAlignedBoundingBox(instance).halfExtent),
        [1, 1, 1]);
    rm.setPriority(instance, 3);
    assert.strictEqual(rm.getPriority(instance), 3);
    rm.setChannel(instance, 2);
    assert.strictEqual(rm.getChannel(instance), 2);
    rm.setCastShadows(instance, false);
    assert.strictEqual(rm.isShadowCaster(instance), false);
    rm.setReceiveShadows(instance, false);
    assert.strictEqual(rm.isShadowReceiver(instance), false);
    rm.setCulling(instance, false);
    assert.strictEqual(rm.isCullingEnabled(instance), false);
    rm.setFogEnabled(instance, false);
    assert.strictEqual(rm.getFogEnabled(instance), false);
    rm.setLightChannel(instance, 0, false);
    assert.strictEqual(rm.getLightChannel(instance, 0), false);
    rm.setScreenSpaceContactShadows(instance, true);
    assert.strictEqual(rm.isScreenSpaceContactShadowsEnabled(instance), true);
    rm.setBlendOrderAt(instance, 0, 7);
    assert.strictEqual(rm.getBlendOrderAt(instance, 0), 7);
    rm.setGlobalBlendOrderEnabledAt(instance, 0, false);
    assert.strictEqual(rm.isGlobalBlendOrderEnabledAt(instance, 0), false);
    rm.setLayerMask(instance, 0xff, 0x02);

    instance.delete();
    rm.destroy(entity);
    assert.strictEqual(rm.hasComponent(entity), false);
}));

test('RenderableManager carries geometry and material assignments', () => withEngine((engine) => {
    const vertexBuffer = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(Filament.VertexAttribute.POSITION, 0,
            Filament.VertexBuffer$AttributeType.FLOAT3, 0, 16)
        .attribute(Filament.VertexAttribute.TANGENTS, 0,
            Filament.VertexBuffer$AttributeType.SHORT4, 12, 16)
        .build(engine);
    vertexBuffer.setBufferAt(engine, 0, new Float32Array(12));
    const indexBuffer = Filament.IndexBuffer.Builder()
        .indexCount(3)
        .bufferType(Filament.IndexBuffer$IndexType.USHORT)
        .build(engine);
    indexBuffer.setBuffer(engine, new Uint16Array([0, 1, 2]));

    const mat = material(engine);
    const matinst = mat.createInstance();
    const entity = newEntity();
    // A renderable with primitives is rejected without a bounding box to cull against.
    Filament.RenderableManager.Builder(3)
        .boundingBox({ center: [0, 0, 0], halfExtent: [1, 1, 1] })
        .material(0, matinst)
        .material(1, matinst)
        .material(2, matinst)
        .geometry(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vertexBuffer, indexBuffer)
        .geometryOffset(1, Filament.RenderableManager$PrimitiveType.TRIANGLES, vertexBuffer,
            indexBuffer, 0, 3)
        .geometryMinMax(2, Filament.RenderableManager$PrimitiveType.TRIANGLES, vertexBuffer,
            indexBuffer, 0, 0, 2, 3)
        .build(engine, entity);

    const rm = engine.getRenderableManager();
    const instance = rm.getInstance(entity);
    assert.strictEqual(rm.getPrimitiveCount(instance), 3);
    // POSITION is attribute 0, so the enabled-attribute bitmask is exactly bit 0.
    assert.strictEqual(rm.getEnabledAttributesAt(instance, 0), 3);
    assert.ok(rm.getMaterialInstanceAt(instance, 0), 'primitive 0 has no material');

    rm.setMaterialInstanceAt(instance, 1, matinst);
    assert.ok(rm.getMaterialInstanceAt(instance, 1), 'setMaterialInstanceAt did not take');
    rm.setGeometryAt(instance, 0, Filament.RenderableManager$PrimitiveType.TRIANGLES,
        vertexBuffer, indexBuffer, 0, 3);
    rm.clearMaterialInstanceAt(instance, 1);
    rm.setMaterialInstanceAt(instance, 1, matinst);
    instance.delete();

    // The index-free forms, which take a vertex count instead of an index buffer.
    const noIndices = newEntity();
    Filament.RenderableManager.Builder(2)
        .boundingBox({ center: [0, 0, 0], halfExtent: [1, 1, 1] })
        .material(0, matinst)
        .material(1, matinst)
        .geometryType(Filament.RenderableManager$Builder$GeometryType.DYNAMIC)
        .geometryNoIndices(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, vertexBuffer)
        .geometryNoIndicesOffset(1, Filament.RenderableManager$PrimitiveType.TRIANGLES,
            vertexBuffer, 0, 3)
        .build(engine, noIndices);
    const noIndicesInstance = rm.getInstance(noIndices);
    rm.setGeometryNoIndicesAt(noIndicesInstance, 0,
        Filament.RenderableManager$PrimitiveType.TRIANGLES, vertexBuffer, 0, 3);
    assert.strictEqual(rm.getPrimitiveCount(noIndicesInstance), 2);
    rm.destroy(entity);
    rm.destroy(noIndices);
    engine.destroyMaterialInstance(matinst);
    engine.destroyMaterial(mat);
    engine.destroyVertexBuffer(vertexBuffer);
    engine.destroyIndexBuffer(indexBuffer);
}));

test('Skinning and morphing buffers report what they were built with', () => withEngine((engine) => {
    const bone = { unitQuaternion: [0, 0, 0, 1], translation: [0, 0, 0] };
    const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];

    // RenderableManager::setSkinningBuffer always binds CONFIG_MAX_BONE_COUNT bones, whatever
    // count it is given, so a buffer it is pointed at has to be that large.
    const skinningBuffer = Filament.SkinningBuffer.Builder()
        .boneCount(256)
        .initialize(true)
        .build(engine);
    assert.strictEqual(skinningBuffer.getBoneCount(), 256);
    skinningBuffer.setBones(engine, [bone], 0);
    skinningBuffer.setBonesFromMatrices(engine, [identity], 0);
    assert.ok(engine.isValidSkinningBuffer(skinningBuffer));

    const morphTargetBuffer = Filament.MorphTargetBuffer.Builder()
        .vertexCount(3)
        .count(2)
        .withPositions(true)
        .withTangents(true)
        .enableCustomMorphing(true)
        .build(engine);
    assert.strictEqual(morphTargetBuffer.getVertexCount(), 3);
    assert.strictEqual(morphTargetBuffer.getCount(), 2);
    assert.strictEqual(morphTargetBuffer.hasPositions(), true);
    assert.strictEqual(morphTargetBuffer.hasTangents(), true);
    assert.strictEqual(morphTargetBuffer.isCustomMorphingEnabled(), true);
    morphTargetBuffer.setPositionsAt(engine, 0, new Float32Array(9), 3, 0);
    morphTargetBuffer.setTangentsAt(engine, 0, new Int16Array(12), 3, 0);
    assert.ok(engine.isValidMorphTargetBuffer(morphTargetBuffer));

    const entity = newEntity();
    Filament.RenderableManager.Builder(1)
        .enableSkinningBuffers(true)
        .skinningBuffer(skinningBuffer, 256, 0)
        .morphingTargetCount(2)
        .morphingBuffer(morphTargetBuffer)
        .morphingBufferOffset(0, 0, 0)
        .build(engine, entity);

    const rm = engine.getRenderableManager();
    const instance = rm.getInstance(entity);
    assert.strictEqual(rm.getMorphTargetCount(instance), 2);
    rm.setSkinningBuffer(instance, skinningBuffer, 256, 0);
    rm.setMorphWeights(instance, [0.0, 1.0]);
    rm.setMorphWeightsOffset(instance, [1.0], 1);
    rm.setMorphTargetBufferOffsetAt(instance, 0, 0, 0);
    instance.delete();

    // The bone-transform forms, which write into the renderable's own skinning storage.
    const boned = newEntity();
    Filament.RenderableManager.Builder(1)
        .skinning(1)
        .skinningBones([bone])
        .build(engine, boned);
    const bonedInstance = rm.getInstance(boned);
    rm.setBones(bonedInstance, [bone], 0);
    rm.setBonesFromMatrices(bonedInstance, [identity], 0);
    bonedInstance.delete();

    Filament.RenderableManager.Builder(1).skinningMatrices([identity]).build(engine, newEntity());

    engine.destroySkinningBuffer(skinningBuffer);
    engine.destroyMorphTargetBuffer(morphTargetBuffer);
}));

test('LightManager placement and sun controls round-trip', () => withEngine((engine) => {
    const lm = engine.getLightManager();
    const entity = newEntity();
    Filament.LightManager.Builder(Filament.LightManager$Type.SUN)
        .castLight(true)
        .castShadows(true)
        .color([1, 1, 1])
        .direction([0, -1, 0])
        .position([1, 2, 3])
        .intensity(110000)
        .sunAngularRadius(1)
        .sunHaloSize(10)
        .sunHaloFalloff(80)
        .lightChannel(0, true)
        .shadowOptions({
            mapSize: 1024, shadowCascades: 4,
            cascadeSplitPositions: [0.25, 0.5, 0.75],
            vsm: { elvsm: true, blurWidth: 2.0 }
        })
        .build(engine, entity);

    assert.strictEqual(lm.hasComponent(entity), true);
    assert.strictEqual(lm.getComponentCount(), 1);
    const instance = lm.getInstance(entity);

    assert.strictEqual(lm.getType(instance), Filament.LightManager$Type.SUN);
    assert.strictEqual(lm.isDirectional(instance), true);
    assert.strictEqual(lm.isPointLight(instance), false);
    assert.strictEqual(lm.isSpotLight(instance), false);
    assert.strictEqual(lm.isShadowCaster(instance), true);
    assert.deepStrictEqual(Array.from(lm.getColor(instance)), [1, 1, 1]);
    assert.deepStrictEqual(Array.from(lm.getDirection(instance)), [0, -1, 0]);
    assert.deepStrictEqual(Array.from(lm.getPosition(instance)), [1, 2, 3]);
    assert.strictEqual(lm.getSunAngularRadius(instance), 1);
    assert.strictEqual(lm.getSunHaloSize(instance), 10);
    assert.strictEqual(lm.getSunHaloFalloff(instance), 80);
    assert.strictEqual(lm.getLightChannel(instance, 0), true);

    lm.setPosition(instance, [4, 5, 6]);
    assert.deepStrictEqual(Array.from(lm.getPosition(instance)), [4, 5, 6]);
    lm.setDirection(instance, [0, 0, -1]);
    assert.deepStrictEqual(Array.from(lm.getDirection(instance)), [0, 0, -1]);
    lm.setColor(instance, [1, 0, 0]);
    assert.deepStrictEqual(Array.from(lm.getColor(instance)), [1, 0, 0]);
    lm.setIntensity(instance, 100);
    assert.ok(Math.abs(lm.getIntensity(instance) - 100) < 1e-2);
    lm.setSunAngularRadius(instance, 0.5);
    assert.ok(Math.abs(lm.getSunAngularRadius(instance) - 0.5) < 1e-5);
    lm.setSunHaloSize(instance, 11);
    assert.strictEqual(lm.getSunHaloSize(instance), 11);
    lm.setSunHaloFalloff(instance, 60);
    assert.strictEqual(lm.getSunHaloFalloff(instance), 60);
    lm.setLightChannel(instance, 0, false);
    assert.strictEqual(lm.getLightChannel(instance, 0), false);
    lm.setShadowCaster(instance, false);
    assert.strictEqual(lm.isShadowCaster(instance), false);
    lm.setShadowOptions(instance, { mapSize: 512 });

    // The three photometric intensity setters all land on the same stored value.
    lm.setIntensityCandela(instance, 1000);
    lm.setIntensityEnergy(instance, 100, 0.5);
    assert.ok(lm.getIntensity(instance) > 0);

    instance.delete();
    lm.destroy(entity);
    assert.strictEqual(lm.hasComponent(entity), false);
}));

test('LightManager spot cone getters mirror setSpotLightCone', () => withEngine((engine) => {
    const lm = engine.getLightManager();
    const entity = newEntity();

    Filament.LightManager.Builder(Filament.LightManager$Type.SPOT)
        .castShadows(false)
        .position([0, 0, 0])
        .direction([0, -1, 0])
        .intensityCandela(1000)
        .spotLightCone(0.25, 0.5)
        .build(engine, entity);

    // The outer angle is stored as given; the inner one is recomputed from the packed cone
    // scale, which LightManager.h documents as not round-tripping exactly.
    const instance = lm.getInstance(entity);
    assert.strictEqual(lm.isSpotLight(instance), true);
    assert.ok(Math.abs(lm.getSpotLightOuterCone(instance) - 0.5) < 1e-5,
        `outer cone was ${lm.getSpotLightOuterCone(instance)}`);
    assert.ok(Math.abs(lm.getSpotLightInnerCone(instance) - 0.25) < 1e-2,
        `inner cone was ${lm.getSpotLightInnerCone(instance)}`);

    lm.setSpotLightCone(instance, 0.1, 0.2);
    assert.ok(Math.abs(lm.getSpotLightOuterCone(instance) - 0.2) < 1e-5,
        `outer cone was ${lm.getSpotLightOuterCone(instance)}`);
    assert.ok(Math.abs(lm.getSpotLightInnerCone(instance) - 0.1) < 1e-2,
        `inner cone was ${lm.getSpotLightInnerCone(instance)}`);
}));

test('LightManager$ShadowCascades computes split positions', () => {
    const uniform = Filament.LightManager$ShadowCascades.computeUniformSplits(4);
    const log = Filament.LightManager$ShadowCascades.computeLogSplits(4, 0.1, 100);
    const practical = Filament.LightManager$ShadowCascades.computePracticalSplits(4, 0.1, 100, 0.5);
    for (const splits of [uniform, log, practical]) {
        assert.strictEqual(splits.length, 3, 'four cascades have three splits between them');
        assert.ok(splits[0] < splits[1] && splits[1] < splits[2], `unsorted splits ${splits}`);
    }
});

// ---------------------------------------------------------------------------
// GPU resources: buffers, textures, materials, render targets
// ---------------------------------------------------------------------------

test('Vertex, index and buffer objects report their sizes', () => withEngine((engine) => {
    const vertexBuffer = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(Filament.VertexAttribute.POSITION, 0,
            Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .normalized(Filament.VertexAttribute.COLOR)
        .normalizedIf(Filament.VertexAttribute.COLOR, false)
        .build(engine);
    vertexBuffer.setBufferAt(engine, 0, new Float32Array(9));
    assert.strictEqual(vertexBuffer.getVertexCount(), 3);

    const indexBuffer = Filament.IndexBuffer.Builder()
        .indexCount(3)
        .bufferType(Filament.IndexBuffer$IndexType.USHORT)
        .build(engine);
    indexBuffer.setBuffer(engine, new Uint16Array([0, 1, 2]));
    assert.strictEqual(indexBuffer.getIndexCount(), 3);

    const bufferObject = Filament.BufferObject.Builder()
        .size(36)
        .bindingType(Filament.BufferObject$BindingType.VERTEX)
        .build(engine);
    bufferObject.setBuffer(engine, new Float32Array(9));
    assert.strictEqual(bufferObject.getByteCount(), 36);

    // A vertex buffer built with buffer objects sources its data from one instead of its own
    // storage, so it takes setBufferObjectAt rather than setBufferAt.
    const boBacked = Filament.VertexBuffer.Builder()
        .vertexCount(3)
        .bufferCount(1)
        .enableBufferObjects(true)
        .attribute(Filament.VertexAttribute.POSITION, 0,
            Filament.VertexBuffer$AttributeType.FLOAT3, 0, 12)
        .build(engine);
    boBacked.setBufferObjectAt(engine, 0, bufferObject);
}));

test('Texture reports the shape it was built with', () => withEngine((engine) => {
    const texture = Filament.Texture.Builder()
        .width(16)
        .height(8)
        .depth(1)
        .levels(2)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(textureUsage('UPLOADABLE', 'SAMPLEABLE'))
        .build(engine);

    assert.strictEqual(texture.getWidth(engine), 16);
    assert.strictEqual(texture.getHeight(engine, 0), 8);
    assert.strictEqual(texture.getWidth(engine, 1), 8, 'level 1 is half as wide');
    assert.strictEqual(texture.getDepth(engine, 0), 1);
    assert.strictEqual(texture.getLevels(engine), 2);
    assert.strictEqual(texture.getTarget(), Filament.Texture$Sampler.SAMPLER_2D);
    assert.strictEqual(texture.getFormat(), Filament.Texture$InternalFormat.RGBA8);

    texture.setImage(engine, 0, Filament.PixelBuffer(new Uint8Array(16 * 8 * 4),
        Filament.PixelDataFormat.RGBA, Filament.PixelDataType.UBYTE));
    texture.setImage(engine, 1, 0, 0, 8, 4, Filament.PixelBuffer(new Uint8Array(8 * 4 * 4),
        Filament.PixelDataFormat.RGBA, Filament.PixelDataType.UBYTE));

    assert.strictEqual(Filament.Texture.validatePixelFormatAndType(
        Filament.Texture$InternalFormat.RGBA8, Filament.PixelDataFormat.RGBA,
        Filament.PixelDataType.UBYTE), true);
    assert.strictEqual(Filament.Texture.isTextureFormatSupported(engine,
        Filament.Texture$InternalFormat.RGBA8), true);
    assert.strictEqual(typeof Filament.Texture.isTextureFormatMipmappable(engine,
        Filament.Texture$InternalFormat.RGBA8), 'boolean');
    assert.strictEqual(typeof Filament.Texture.isTextureSwizzleSupported(engine), 'boolean');
    assert.ok(Filament.Texture.getMaxTextureSize(engine,
        Filament.Texture$Sampler.SAMPLER_2D) > 0);
    assert.ok(Filament.Texture.getMaxArrayTextureLayers(engine) > 0);
}));

test('TextureSampler state round-trips', () => {
    const sampler = new Filament.TextureSampler(Filament.MinFilter.LINEAR,
        Filament.MagFilter.LINEAR, Filament.WrapMode.CLAMP_TO_EDGE);
    assert.strictEqual(sampler.getMinFilter(), Filament.MinFilter.LINEAR);
    assert.strictEqual(sampler.getMagFilter(), Filament.MagFilter.LINEAR);
    assert.strictEqual(sampler.getWrapModeS(), Filament.WrapMode.CLAMP_TO_EDGE);

    sampler.setAnisotropy(4);
    assert.strictEqual(sampler.getAnisotropy(), 4);
    sampler.setCompareMode(Filament.CompareMode.COMPARE_TO_TEXTURE,
        Filament.CompareFunc.LESS_EQUAL);
    assert.strictEqual(sampler.getCompareMode(), Filament.CompareMode.COMPARE_TO_TEXTURE);
    assert.strictEqual(sampler.getCompareFunc(), Filament.CompareFunc.LESS_EQUAL);
    sampler.setMinFilter(Filament.MinFilter.NEAREST);
    assert.strictEqual(sampler.getMinFilter(), Filament.MinFilter.NEAREST);
    sampler.setMagFilter(Filament.MagFilter.NEAREST);
    assert.strictEqual(sampler.getMagFilter(), Filament.MagFilter.NEAREST);
    sampler.setWrapModeS(Filament.WrapMode.REPEAT);
    assert.strictEqual(sampler.getWrapModeS(), Filament.WrapMode.REPEAT);
    sampler.setWrapModeT(Filament.WrapMode.MIRRORED_REPEAT);
    assert.strictEqual(sampler.getWrapModeT(), Filament.WrapMode.MIRRORED_REPEAT);
    sampler.setWrapModeR(Filament.WrapMode.REPEAT);
    assert.strictEqual(sampler.getWrapModeR(), Filament.WrapMode.REPEAT);
});

test('Material reflects what it was compiled with', () => withEngine((engine) => {
    const mat = material(engine);
    assert.strictEqual(mat.getName(), 'LitOpaque');
    assert.strictEqual(mat.getShading(), Filament.Shading.LIT);
    assert.strictEqual(mat.getBlendingMode(), Filament.BlendingMode.OPAQUE);
    assert.strictEqual(mat.getInterpolation(), Filament.Interpolation.SMOOTH);
    assert.strictEqual(mat.getVertexDomain(), Filament.VertexDomain.OBJECT);
    assert.strictEqual(mat.getFeatureLevel(), Filament.FeatureLevel.FEATURE_LEVEL_1);
    assert.strictEqual(mat.getRefractionMode(), Filament.RefractionMode.NONE);
    assert.strictEqual(mat.getRefractionType(), Filament.RefractionType.SOLID);
    assert.strictEqual(mat.getReflectionMode(), Filament.ReflectionMode.DEFAULT);
    assert.strictEqual(mat.getCullingMode(), Filament.CullingMode.BACK);
    assert.strictEqual(mat.getTransparencyMode(), Filament.TransparencyMode.DEFAULT);
    assert.strictEqual(mat.isDoubleSided(), false);
    assert.strictEqual(mat.isColorWriteEnabled(), true);
    assert.strictEqual(mat.isDepthWriteEnabled(), true);
    assert.strictEqual(mat.isDepthCullingEnabled(), true);
    assert.strictEqual(typeof mat.isAlphaToCoverageEnabled(), 'boolean');
    assert.ok(mat.getRequiredAttributes() > 0);
    assert.ok(mat.getMaskThreshold() >= 0);
    assert.ok(mat.getSpecularAntiAliasingVariance() >= 0);
    assert.ok(mat.getSpecularAntiAliasingThreshold() >= 0);

    // The parameter list must agree with itself and name the parameters the material declares.
    const parameters = mat.getParameters();
    assert.strictEqual(parameters.length, mat.getParameterCount());
    const names = parameters.map((p) => p.name);
    assert.ok(names.includes('baseColor'), `no baseColor in ${names}`);
    assert.strictEqual(mat.hasParameter('baseColor'), true);
    assert.strictEqual(mat.hasParameter('nonexistent'), false);
    assert.strictEqual(typeof parameters[0].isSampler, 'boolean');
    assert.strictEqual(typeof parameters[0].count, 'number');

    assert.ok(mat.getDefaultInstance(), 'no default instance');
    const createdInst = mat.createInstance();
    assert.ok(createdInst, 'createInstance returned null');
    engine.destroyMaterialInstance(createdInst);
    const named = mat.createNamedInstance('named');
    assert.strictEqual(named.getName(), 'named');
    engine.destroyMaterialInstance(named);
    engine.destroyMaterial(mat);
}));

test('MaterialInstance parameter setters accept every bound type', () => withEngine((engine) => {
    const mat = material(engine);
    const matinst = mat.createInstance();

    matinst.setFloatParameter('roughness', 0.5);
    matinst.setFloat3Parameter('baseColor', [1, 0, 0]);
    matinst.setFloat4Parameter('emissive', [1, 0, 0, 1]);
    matinst.setColor3Parameter('baseColor', Filament.RgbType.sRGB, [1, 0, 0]);
    matinst.setColor4Parameter('emissive', Filament.RgbaType.sRGB, [1, 0, 0, 1]);

    // The remaining typed setters have no matching parameter in this material, so they are
    // checked for registration rather than called; a wrong name aborts the engine.
    for (const setter of ['setBoolParameter', 'setBool2Parameter', 'setBool3Parameter',
        'setBool4Parameter', 'setIntParameter', 'setInt2Parameter', 'setInt3Parameter',
        'setInt4Parameter', 'setFloat2Parameter', 'setMat3Parameter', 'setMat4Parameter',
        'setConstantBool', 'setConstantFloat', 'setConstantInt']) {
        assert.strictEqual(typeof matinst[setter], 'function', `${setter} is not registered`);
    }

    const duplicate = matinst.duplicate();
    assert.ok(duplicate, 'duplicate returned null');
    const copy = matinst.duplicateNamed('copy');
    assert.strictEqual(copy.getName(), 'copy');
    assert.strictEqual(matinst.getMaterial().getName(), mat.getName());

    matinst.setScissor(0, 0, 16, 16);
    matinst.unsetScissor();

    engine.destroyMaterialInstance(duplicate);
    engine.destroyMaterialInstance(copy);
    engine.destroyMaterialInstance(matinst);
    engine.destroyMaterial(mat);
}));

test('MaterialInstance render state round-trips', () => withEngine((engine) => {
    const matinst = material(engine).createInstance();

    matinst.setCullingMode(Filament.CullingMode.FRONT);
    assert.strictEqual(matinst.getCullingMode(), Filament.CullingMode.FRONT);
    matinst.setCullingModeSeparate(Filament.CullingMode.BACK, Filament.CullingMode.FRONT);
    assert.strictEqual(matinst.getCullingMode(), Filament.CullingMode.BACK);
    assert.strictEqual(matinst.getShadowCullingMode(), Filament.CullingMode.FRONT);

    matinst.setColorWrite(false);
    assert.strictEqual(matinst.isColorWriteEnabled(), false);
    matinst.setDepthWrite(false);
    assert.strictEqual(matinst.isDepthWriteEnabled(), false);
    matinst.setDepthCulling(false);
    assert.strictEqual(matinst.isDepthCullingEnabled(), false);
    matinst.setDepthFunc(Filament.CompareFunc.GREATER);
    assert.strictEqual(matinst.getDepthFunc(), Filament.CompareFunc.GREATER);

    matinst.setTransparencyMode(Filament.TransparencyMode.TWO_PASSES_ONE_SIDE);
    assert.strictEqual(matinst.getTransparencyMode(),
        Filament.TransparencyMode.TWO_PASSES_ONE_SIDE);

    matinst.setStencilWrite(true);
    assert.strictEqual(matinst.isStencilWriteEnabled(), true);
    matinst.setStencilCompareFunction(Filament.CompareFunc.EQUAL);
    matinst.setStencilCompareFunction(Filament.CompareFunc.EQUAL, Filament.StencilFace.FRONT);
    matinst.setStencilOpStencilFail(Filament.StencilOperation.KEEP);
    matinst.setStencilOpDepthFail(Filament.StencilOperation.KEEP, Filament.StencilFace.BACK);
    matinst.setStencilOpDepthStencilPass(Filament.StencilOperation.REPLACE);
    matinst.setStencilReferenceValue(1);
    matinst.setStencilReadMask(0xff);
    matinst.setStencilWriteMask(0xff, Filament.StencilFace.FRONT_AND_BACK);

    matinst.setPolygonOffset(1.0, 1.0);

    // setMaskThreshold and the specular anti-aliasing setters abort unless the material was
    // compiled for them, so only their getters are safe to call on an ordinary opaque material.
    assert.ok(matinst.getMaskThreshold() >= 0);
    assert.ok(matinst.getSpecularAntiAliasingVariance() >= 0);
    assert.ok(matinst.getSpecularAntiAliasingThreshold() >= 0);
    assert.strictEqual(typeof matinst.isDoubleSided(), 'boolean');
    assert.strictEqual(typeof matinst.setDoubleSided, 'function');

    const mat = matinst.getMaterial();
    engine.destroyMaterialInstance(matinst);
    engine.destroyMaterial(mat);
}));

test('MaterialInstance binds a texture parameter', () => withEngine((engine) => {
    const mat = engine.createMaterial(asset(TEXTURED_MATERIAL));
    const samplers = mat.getParameters().filter((p) => p.isSampler).map((p) => p.name);
    assert.ok(samplers.includes('albedo'), `no albedo sampler in ${samplers}`);
    assert.strictEqual(typeof mat.getParameterTransformName('albedo'), 'string');

    const texture = Filament.Texture.Builder()
        .width(4).height(4)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(textureUsage('UPLOADABLE', 'SAMPLEABLE'))
        .build(engine);
    const matinst = mat.createInstance();
    matinst.setTextureParameter('albedo', texture,
        new Filament.TextureSampler(Filament.MinFilter.LINEAR, Filament.MagFilter.LINEAR,
            Filament.WrapMode.CLAMP_TO_EDGE));
    engine.destroyMaterialInstance(matinst);
    engine.destroyTexture(texture);
    engine.destroyMaterial(mat);
}));

test('Every tone mapper builds a ColorGrading', () => withEngine((engine) => {
    const mappers = [
        new Filament.LinearToneMapper(),
        new Filament.ACESToneMapper(),
        new Filament.ACESLegacyToneMapper(),
        new Filament.FilmicToneMapper(),
        new Filament.PBRNeutralToneMapper(),
        new Filament.GT7ToneMapper(),
        new Filament.DisplayRangeToneMapper(),
        new Filament.AgxToneMapper(Filament.AgxToneMapper$AgxLook.PUNCHY),
    ];
    for (const mapper of mappers) {
        assert.ok(Filament.ColorGrading.Builder().toneMapper(mapper).build(engine),
            'toneMapper build returned null');
        mapper.delete();
    }

    const generic = new Filament.GenericToneMapper(1.55, 0.18, 0.215, 10.0);
    assert.ok(Math.abs(generic.getContrast() - 1.55) < 1e-5);
    assert.ok(Math.abs(generic.getMidGrayIn() - 0.18) < 1e-5);
    assert.ok(Math.abs(generic.getMidGrayOut() - 0.215) < 1e-5);
    assert.ok(Math.abs(generic.getHdrMax() - 10.0) < 1e-5);
    generic.setContrast(1.6);
    generic.setMidGrayIn(0.2);
    generic.setMidGrayOut(0.22);
    generic.setHdrMax(12.0);
    assert.ok(Math.abs(generic.getContrast() - 1.6) < 1e-5);
    assert.ok(Math.abs(generic.getMidGrayIn() - 0.2) < 1e-5);
    assert.ok(Math.abs(generic.getMidGrayOut() - 0.22) < 1e-5);
    assert.ok(Math.abs(generic.getHdrMax() - 12.0) < 1e-5);
    assert.ok(Filament.ColorGrading.Builder().toneMapper(generic).build(engine));
    generic.delete();
}));

test('ColorGrading$Builder accepts its whole grading chain', () => withEngine((engine) => {
    const colorGrading = Filament.ColorGrading.Builder()
        .quality(Filament.ColorGrading$QualityLevel.HIGH)
        .format(Filament.ColorGrading$LutFormat.INTEGER)
        .dimensions(16)
        .toneMapping(Filament.ColorGrading$ToneMapping.ACES)
        .luminanceScaling(true)
        .gamutMapping(true)
        .exposure(0.0)
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
        .build(engine);
    assert.ok(engine.isValidColorGrading(colorGrading));

    // A custom LUT replaces the whole chain above; 2x2x2 is the smallest legal one.
    const lut = Filament.ColorGrading.Builder()
        .customLut(new Float32Array([
            1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1,
        ]), 2)
        .build(engine);
    assert.ok(engine.isValidColorGrading(lut));
}));

test('IndirectLight round-trips intensity, rotation and its textures', () => withEngine((engine) => {
    const sh = new Float32Array(9 * 3);
    for (let i = 0; i < sh.length; i++) {
        sh[i] = 0.1 * (i + 1);
    }
    const ibl = Filament.IndirectLight.Builder()
        .irradiance(3, sh)
        .radiance(3, sh)
        .intensity(30000)
        .build(engine);
    assert.strictEqual(ibl.getIntensity(), 30000);
    ibl.setIntensity(25000);
    assert.strictEqual(ibl.getIntensity(), 25000);

    const identity = [1, 0, 0, 0, 1, 0, 0, 0, 1];
    ibl.setRotation(identity);
    assert.deepStrictEqual(Array.from(ibl.getRotation()), identity);

    // The estimators read the SH coefficients directly, without an engine or a texture.
    const direction = Filament.IndirectLight.getDirectionEstimate(sh);
    assert.strictEqual(direction.length, 3);
    assert.ok(Number.isFinite(direction[0]), `direction estimate was ${direction}`);
    const color = Filament.IndirectLight.getColorEstimate(sh, direction);
    assert.strictEqual(color.length, 4);
    assert.ok(Number.isFinite(color[0]), `color estimate was ${color}`);

    const cubemap = Filament.Texture.Builder()
        .width(16).height(16)
        .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(textureUsage('UPLOADABLE', 'SAMPLEABLE'))
        .build(engine);
    // Both forms of irradiance are one name, which embind tells apart by argument count.
    const textured = Filament.IndirectLight.Builder()
        .reflections(cubemap)
        .irradiance(cubemap)
        .rotation(identity)
        .build(engine);
    assert.ok(textured.getReflectionsTexture(), 'reflections texture was not kept');
    assert.ok(textured.getIrradianceTexture(), 'irradiance texture was not kept');
}));

test('Skybox round-trips its layer mask and texture', () => withEngine((engine) => {
    const skybox = Filament.Skybox.Builder()
        .color([0, 0, 0, 1])
        .intensity(30000)
        .showSun(true)
        .priority(1)
        .build(engine);
    skybox.setColor([1, 1, 1, 1]);
    assert.strictEqual(skybox.getLayerMask(), 1);
    skybox.setLayerMask(0xff, 0x04);
    assert.strictEqual(skybox.getLayerMask(), 0x04);

    const envmap = Filament.Texture.Builder()
        .width(16).height(16).levels(1)
        .sampler(Filament.Texture$Sampler.SAMPLER_CUBEMAP)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(textureUsage('UPLOADABLE', 'SAMPLEABLE'))
        .build(engine);
    const textured = Filament.Skybox.Builder().environment(envmap).build(engine);
    assert.ok(textured.getTexture(), 'environment texture was not kept');
}));

test('RenderTarget round-trips its attachment', () => withEngine((engine) => {
    const color = Filament.Texture.Builder()
        .width(16).height(16).levels(2)
        .sampler(Filament.Texture$Sampler.SAMPLER_2D)
        .format(Filament.Texture$InternalFormat.RGBA8)
        .usage(textureUsage('COLOR_ATTACHMENT', 'SAMPLEABLE'))
        .build(engine);
    const renderTarget = Filament.RenderTarget.Builder()
        .texture(Filament.RenderTarget$AttachmentPoint.COLOR, color)
        .mipLevel(Filament.RenderTarget$AttachmentPoint.COLOR, 1)
        .face(Filament.RenderTarget$AttachmentPoint.COLOR,
            Filament.Texture$CubemapFace.POSITIVE_X)
        .layer(Filament.RenderTarget$AttachmentPoint.COLOR, 0)
        .build(engine);

    assert.ok(renderTarget.getTexture(Filament.RenderTarget$AttachmentPoint.COLOR),
        'attachment texture was not kept');
    assert.strictEqual(renderTarget.getMipLevel(Filament.RenderTarget$AttachmentPoint.COLOR), 1);
    assert.strictEqual(renderTarget.getFace(Filament.RenderTarget$AttachmentPoint.COLOR),
        Filament.Texture$CubemapFace.POSITIVE_X);
    assert.strictEqual(renderTarget.getLayer(Filament.RenderTarget$AttachmentPoint.COLOR), 0);
    assert.ok(renderTarget.getSupportedColorAttachmentsCount() > 0);
}));

test('SwapChain reports what the backend supports', () => withEngine((engine) => {
    const swapChain = engine.createSwapChain();
    assert.ok(engine.isValidSwapChain(swapChain));
    assert.strictEqual(typeof Filament.SwapChain.isSRGBSwapChainSupported(engine), 'boolean');
    assert.strictEqual(typeof Filament.SwapChain.isProtectedContentSupported(engine), 'boolean');
    assert.strictEqual(typeof Filament.SwapChain.isMSAASwapChainSupported(engine, 4), 'boolean');
}));

// FSwapChain tracks the flag above the driver, so it is correct under NOOP even though nothing
// fires there. Actual invocation is covered by test-browser.html.
test('SwapChain frame-scheduled callback is set, replaced and cleared', () => withEngine((engine) => {
    const swapChain = engine.createSwapChain();
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), false);

    swapChain.setFrameScheduledCallback(() => {});
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), true);

    // Replacing keeps it set; the previous val handle is released with the old std::function.
    swapChain.setFrameScheduledCallback(() => {});
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), true);

    // No argument clears it, matching the C++ default-argument overload.
    swapChain.setFrameScheduledCallback();
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), false);

    // Explicit null/undefined clear it too, so callers can pass a nullable straight through.
    swapChain.setFrameScheduledCallback(() => {});
    swapChain.setFrameScheduledCallback(null);
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), false);

    // Re-arming after a clear has to work.
    swapChain.setFrameScheduledCallback(() => {});
    assert.strictEqual(swapChain.isFrameScheduledCallbackSet(), true);
    engine.destroySwapChain(swapChain);
}));

// ---------------------------------------------------------------------------
// Geometry / image helpers
// ---------------------------------------------------------------------------

test('SurfaceOrientation returns a quaternion per vertex, in each layout', () => {
    // Three vertices whose normals all point down +Z, with matching tangents and positions.
    const normals = new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]);
    const tangents = new Float32Array([1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1]);
    const builder = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(normals, 0)
        .tangents(tangents, 0);
    // Every setter must return the builder, which is what filament.d.ts promises and what the
    // chain above depends on.
    assert.ok(builder instanceof Filament.SurfaceOrientation$Builder,
        'SurfaceOrientation$Builder setters do not chain');

    const orientation = builder.build();
    assert.strictEqual(orientation.getQuats(3).length, 12, 'short4 quats are 4 shorts each');
    assert.strictEqual(orientation.getQuatsHalf4(3).length, 12);
    assert.strictEqual(orientation.getQuatsFloat4(3).length, 12);
    orientation.delete();

    // The other inputs: uvs, positions and an index buffer, in both index widths.
    const fromTriangles16 = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(normals, 0)
        .uvs(new Float32Array([0, 0, 1, 0, 0, 1]), 0)
        .positions(new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]), 0)
        .triangleCount(1)
        .triangles16(new Uint16Array([0, 1, 2]))
        .build();
    assert.strictEqual(fromTriangles16.getQuats(3).length, 12);
    fromTriangles16.delete();

    const fromTriangles32 = new Filament.SurfaceOrientation$Builder()
        .vertexCount(3)
        .normals(normals, 0)
        .positions(new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]), 0)
        .triangleCount(1)
        .triangles32(new Uint32Array([0, 1, 2]))
        .build();
    assert.strictEqual(fromTriangles32.getQuats(3).length, 12);
    fromTriangles32.delete();
});

test('Ktx1Bundle parses a cubemap and its metadata', () => {
    const bundle = new Filament.Ktx1Bundle(Filament.Buffer(asset(IBL_KTX)));
    assert.ok(bundle.getNumMipLevels() > 0);
    assert.strictEqual(bundle.getArrayLength(), 1);
    assert.strictEqual(bundle.isCubemap(), true);
    assert.strictEqual(bundle.isCompressed(), false);
    assert.ok(bundle.getInternalFormat(true) !== undefined);
    assert.ok(bundle.getPixelDataFormat() !== undefined);
    assert.ok(bundle.getPixelDataType() !== undefined);

    const info = bundle.info();
    assert.ok(info.pixelWidth > 0);
    assert.strictEqual(info.pixelHeight, info.pixelWidth, 'a cubemap face is square');
    assert.strictEqual(info.pixelDepth, 0);
    assert.ok(info.glInternalFormat > 0);
    assert.ok(info.glFormat >= 0);
    assert.ok(info.glType >= 0);
    assert.ok(info.glTypeSize > 0);
    assert.ok(info.endianness >= 0);
    assert.ok(info.glBaseInternalFormat > 0);

    // One face of the top mip, then all six of them.
    const face = bundle.getBlob([0, 0, 0]);
    assert.ok(face.byteLength > 0);
    assert.strictEqual(bundle.getCubeBlob(0).byteLength, face.byteLength * 6);
    // This environment carries its spherical harmonics as a metadata string.
    assert.ok(bundle.getMetadata('sh').length > 0, 'no sh metadata');
});

test('MeshReader$MaterialRegistry maps names to material instances', () => withEngine((engine) => {
    const registry = new Filament.MeshReader$MaterialRegistry();
    assert.strictEqual(registry.size(), 0);
    const mat = material(engine);
    registry.set('DefaultMaterial', mat.getDefaultInstance());
    assert.strictEqual(registry.size(), 1);
    assert.ok(registry.get('DefaultMaterial'), 'registry lost the instance it was given');
    assert.strictEqual(registry.keys().size(), 1);
    registry.delete();
    engine.destroyMaterial(mat);
}));

test('driver buffer descriptors expose their bytes', () => {
    const buffer = new Filament.driver$BufferDescriptor(16);
    assert.strictEqual(buffer.getBytes().byteLength, 16);
    const pixels = new Filament.driver$PixelBufferDescriptor(16, Filament.PixelDataFormat.RGBA,
        Filament.PixelDataType.UBYTE);
    assert.strictEqual(pixels.getBytes().byteLength, 16);
    buffer.delete();
    pixels.delete();
});

// ---------------------------------------------------------------------------
// Camera manipulator (camutils)
// ---------------------------------------------------------------------------

test('Camutils orbit manipulator tracks its home position', () => withEngine((engine) => {
    const builder = new Filament.Camutils$Manipulator$Builder()
        .viewport(1024, 768)
        .orbitHomePosition(0, 0, 4)
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
        .panning(true);
    const manipulator = builder.build(Filament.Camutils$Mode.ORBIT);
    builder.delete();

    const eye = [0, 0, 0];
    const target = [0, 0, 0];
    const up = [0, 0, 0];
    manipulator.getLookAt(eye, target, up);
    assert.deepStrictEqual(eye, [0, 0, 4], 'getLookAt did not write back the home position');
    assert.deepStrictEqual(target, [0, 0, 0]);

    manipulator.setViewport(800, 600);
    manipulator.grabBegin(10, 10, false);
    manipulator.grabUpdate(20, 20);
    manipulator.grabEnd();
    manipulator.scroll(10, 10, 1);
    manipulator.update(0.016);

    const origin = [0, 0, 0];
    const direction = [0, 0, 0];
    manipulator.getRay(512, 384, origin, direction);
    assert.ok(direction.some((c) => c !== 0), 'getRay did not write a direction');
    const hit = [0, 0, 0];
    assert.strictEqual(typeof manipulator.raycast(512, 384, hit), 'boolean');

    const camEntity = newEntity();
    const camera = engine.createCamera(camEntity);
    camera.setLookAt(manipulator);

    const current = manipulator.getCurrentBookmark();
    const home = manipulator.getHomeBookmark();
    const midpoint = Filament.Camutils$Bookmark.interpolate(current, home, 0.5);
    assert.ok(Filament.Camutils$Bookmark.duration(current, home) >= 0);
    engine.destroyCameraComponent(camEntity);
    manipulator.delete();
}));

test('Camutils free-flight manipulator accepts key events', () => {
    const builder = new Filament.Camutils$Manipulator$Builder()
        .viewport(1024, 768)
        .flightStartPosition(0, 0, 5)
        .flightStartOrientation(0, 0)
        .flightMaxMoveSpeed(10)
        .flightSpeedSteps(80)
        .flightPanSpeed(0.01, 0.01)
        .flightMoveDamping(15);
    const manipulator = builder.build(Filament.Camutils$Mode.FREE_FLIGHT);
    builder.delete();

    const eye = [0, 0, 0];
    const target = [0, 0, 0];
    const up = [0, 0, 0];
    manipulator.getLookAt(eye, target, up);
    assert.deepStrictEqual(eye, [0, 0, 5]);

    // Holding forward for a second has to move the camera along its view direction.
    manipulator.keyDown(Filament.Camutils$Key.FORWARD);
    for (let i = 0; i < 60; i++) {
        manipulator.update(1 / 60);
    }
    manipulator.keyUp(Filament.Camutils$Key.FORWARD);
    manipulator.getLookAt(eye, target, up);
    assert.ok(eye[2] < 5, `free flight did not move forward, eye is at ${eye}`);
    manipulator.delete();
});

// ---------------------------------------------------------------------------
// glTF (gltfio)
// ---------------------------------------------------------------------------

test('gltfio loads an asset and reports its contents', () => withEngine((engine) => {
    const loader = engine.createAssetLoader();
    const asset_ = loader.createAsset(triangleGltf());
    assert.ok(asset_, 'createAsset returned null');

    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    assert.strictEqual(resourceLoader.loadResources(asset_), true);

    assert.strictEqual(asset_.getEntityCount(), 1);
    assert.strictEqual(asset_.getRenderableEntityCount(), 1);
    assert.strictEqual(asset_.getLightEntityCount(), 0);
    assert.strictEqual(asset_.getCameraEntityCount(), 0);
    assert.strictEqual(asset_.getSceneCount(), 1);
    assert.strictEqual(asset_.getResourceUriCount(), 1);
    assert.strictEqual(asset_.getEntities().length, 1);
    assert.strictEqual(asset_.getRenderableEntities().length, 1);
    assert.strictEqual(asset_.getLightEntities().length, 0);
    assert.strictEqual(asset_.getCameraEntities().length, 0);
    assert.strictEqual(asset_.getResourceUris().length, 1);
    assert.ok(asset_.getRoot().getId() > 0);
    assert.ok(asset_.getWireframe().getId() > 0);
    assert.ok(asset_.getEngine(), 'getEngine returned null');

    // The node is named "triangle", so every by-name query has to find it and no other.
    assert.strictEqual(asset_.getEntitiesByName('triangle').length, 1);
    assert.strictEqual(asset_.getEntitiesByPrefix('tri').length, 1);
    assert.strictEqual(asset_.getEntitiesByName('nonexistent').length, 0);
    assert.ok(asset_.getFirstEntityByName('triangle').getId() > 0);
    assert.strictEqual(asset_.getName(asset_.getFirstEntityByName('triangle')), 'triangle');
    assert.strictEqual(typeof asset_.getExtras(asset_.getRoot()), 'string');
    assert.strictEqual(asset_.getMorphTargetCountAt(asset_.getRoot()), 0);
    assert.strictEqual(asset_.getMorphTargetNames(asset_.getRoot()).length, 0);

    const box = asset_.getBoundingBox();
    assert.strictEqual(box.min.length, 3);
    assert.strictEqual(box.max.length, 3);

    // popRenderable(s) drains the queue of renderables that are ready to be added to a scene.
    assert.ok(asset_.popRenderables(4).size() >= 0);
    assert.ok(asset_.popRenderable());

    asset_.releaseSourceData();
    loader.enableDiagnostics(true);
    resourceLoader.evictResourceData();
    resourceLoader.delete();
    loader.destroyAsset(asset_);
    loader.gc();
    Filament.gltfio$AssetLoader.destroy(loader);
}));

test('gltfio instances share one asset', () => withEngine((engine) => {
    const loader = engine.createAssetLoader();
    const asset_ = loader.createInstancedAsset(triangleGltf(), [null, null]);
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    resourceLoader.loadResources(asset_);

    assert.strictEqual(asset_.getAssetInstanceCount(), 2);
    assert.strictEqual(asset_.getAssetInstances().length, 2);

    const instance = asset_.getInstance();
    assert.ok(instance, 'instanced asset has no instance');
    assert.ok(instance.getAsset(), 'instance does not point back at its asset');
    assert.strictEqual(instance.getEntityCount(), 1);
    assert.strictEqual(instance.getEntities().size(), 1);
    assert.ok(instance.getRoot().getId() > 0);
    assert.strictEqual(instance.getSkinCount(), 0);
    assert.strictEqual(instance.getSkinNames().size(), 0);
    assert.strictEqual(instance.getMaterialVariantCount(), 0);
    assert.strictEqual(instance.getMaterialVariantNames().length, 0);
    assert.strictEqual(instance.getMaterialInstanceCount(), 1);
    assert.strictEqual(instance.getMaterialInstances().size(), 1);
    assert.strictEqual(typeof instance.detachMaterialInstances, 'function');
    const animator = instance.getAnimator();
    assert.strictEqual(animator.getAnimationCount(), 0);
    animator.updateBoneMatrices();
    animator.resetBoneMatrices();

    resourceLoader.evictResourceData();
    resourceLoader.delete();
    loader.destroyAsset(asset_);
    Filament.gltfio$AssetLoader.destroy(loader);
}));

test('gltfio resource loader holds and evicts resource data', () => withEngine((engine) => {
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    // One provider type for all three decoders, so one entry point takes any of them.
    const TextureProvider = Filament.gltfio$TextureProvider;
    resourceLoader.addTextureProvider('image/png', TextureProvider.createStbProvider(engine));
    resourceLoader.addTextureProvider('image/ktx2', TextureProvider.createKtx2Provider(engine));
    resourceLoader.addTextureProvider('image/webp', TextureProvider.createWebpProvider(engine));

    resourceLoader.addResourceData('tex.png', Filament.Buffer(new Uint8Array(4)));
    assert.strictEqual(resourceLoader.hasResourceData('tex.png'), true);
    assert.strictEqual(resourceLoader.hasResourceData('other.png'), false);
    assert.strictEqual(resourceLoader.asyncGetLoadProgress(), 0);
    resourceLoader.asyncCancelLoad();
    resourceLoader.evictResourceData();
    assert.strictEqual(resourceLoader.hasResourceData('tex.png'), false);

    assert.strictEqual(typeof TextureProvider.isWebpSupported(), 'boolean');
    const ubershader = new Filament.gltfio$UbershaderProvider(engine);
    ubershader.destroyMaterials();
    ubershader.delete();
    resourceLoader.delete();
}));

test('gltfio loads asynchronously', () => withEngine((engine) => {
    try {
    const loader = engine.createAssetLoader();
    const asset_ = loader.createAsset(triangleGltf());
    const resourceLoader = new Filament.gltfio$ResourceLoader(engine, true);
    const stb = Filament.gltfio$TextureProvider.createStbProvider(engine);
    resourceLoader.addTextureProvider('image/png', stb);
    assert.strictEqual(resourceLoader.asyncBeginLoad(asset_), true);
    resourceLoader.asyncUpdateLoad();
    assert.strictEqual(resourceLoader.asyncGetLoadProgress(), 1);
    resourceLoader.asyncCancelLoad();
    resourceLoader.evictResourceData();
    stb.delete();
    resourceLoader.delete();
    loader.destroyAsset(asset_);
    Filament.gltfio$AssetLoader.destroy(loader);
    } catch(err) {
        console.error('TEST 52 ERROR:', err && err.stack ? err.stack : err);
        throw err;
    }
}));

// ---------------------------------------------------------------------------
// Viewer / automation harness (gltf_viewer tooling)
// ---------------------------------------------------------------------------

test('AutomationSpec generates and reads test cases', () => {
    const spec = Filament.AutomationSpec.generateDefaultTestCases();
    assert.ok(spec.size() > 0);
    assert.ok(spec.getName(0).length > 0);

    const settings = Filament.AutomationEngine.createDefault().getSettings();
    assert.strictEqual(spec.get(0, settings), true);
    assert.strictEqual(spec.get(spec.size(), settings), false, 'out of range get must fail');

    assert.ok(Filament.AutomationSpec.generate('[{"name":"t","base":{}}]'),
        'generate rejected a valid spec');
    spec.delete();
});

test('JsonSerializer round-trips a settings blob', () => {
    const auto = Filament.AutomationEngine.createDefault();
    const settings = auto.getSettings();
    const serializer = new Filament.JsonSerializer();
    const json = serializer.writeJson(settings);
    assert.ok(json.length > 0);
    assert.strictEqual(serializer.readJson(json, settings), true);
    assert.strictEqual(serializer.readJson('not json', settings), false);
    auto.delete();
});

test('AutomationEngine steps through a batch', () => withEngine((engine) => {
    const view = engine.createView();
    view.setScene(engine.createScene());
    view.setCamera(engine.createCamera(newEntity()));
    view.setViewport([0, 0, 16, 16]);
    const content = {
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
    assert.ok(Filament.AutomationEngine.createFromJSON('[{"name":"t","base":{}}]'),
        'createFromJSON rejected a valid spec');
    assert.ok(auto.testCount() > 0);
    assert.strictEqual(auto.isRunning(), false);

    const options = auto.getOptions();
    options.sleepDuration = 0;
    options.minFrameCount = 1;
    options.verbose = false;
    options.exportScreenshots = false;
    options.exportSettings = false;
    options.exportFormat = Filament.AutomationEngine$ExportFormat.PPM;
    auto.setOptions(options);
    assert.strictEqual(auto.getOptions().minFrameCount, 1);

    auto.startRunning();
    auto.tick(engine, content, 0.016);
    assert.strictEqual(auto.currentTest(), 0);
    auto.applySettings(engine, '{"view":{"bloom":{"enabled":true}}}', content);
    assert.strictEqual(view.getBloomOptions().enabled, true, 'applySettings did not reach the view');
    assert.strictEqual(typeof auto.getStatusMessage(), 'string');
    assert.strictEqual(auto.shouldClose(), false);
    assert.ok(auto.getViewerOptions(), 'getViewerOptions returned null');
    // The color grading is only built once a test case has been applied.
    auto.getColorGrading(engine);

    auto.startBatchMode();
    auto.signalBatchMode();
    auto.stopRunning();
    assert.strictEqual(auto.isRunning(), false);

    auto.terminate();
    auto.delete();
}));

test('ViewerGui builds its user interface', () => withEngine((engine) => {
    // The viewer pushes its settings into the view's camera, so the view needs one; a view
    // without a camera cannot be rendered anyway.
    const viewWithCamera = () => {
        const view = engine.createView();
        view.setScene(engine.createScene());
        view.setCamera(engine.createCamera(newEntity()));
        view.setViewport([0, 0, 300, 600]);
        return view;
    };

    const gui = new Filament.ViewerGui(engine, engine.createScene(), viewWithCamera(), 300);
    gui.mouseEvent(10, 10, true, 0, false);
    gui.keyDownEvent(65);
    gui.keyUpEvent(65);
    gui.keyPressEvent(65);
    assert.ok(gui.getSettings(), 'ViewerGui has no settings');

    // Twice, because the first call is the one that builds the ImGui context and its atlas.
    const guiView = viewWithCamera();
    gui.renderUserInterface(0.016, guiView, 1.0);
    gui.renderUserInterface(0.016, guiView, 1.0);
    gui.delete();
}));

// ---------------------------------------------------------------------------

globalThis.Filament.init([], () => {
    Filament = globalThis.Filament;
    let failed = 0;
    for (const [name, fn] of TESTS) {
        try {
            fn();
            console.log(`  ok   ${name}`);
        } catch (e) {
            failed++;
            console.log(`  FAIL ${name}\n       ${e && e.stack ? e.stack : e}`);
        }
    }
    console.log(`\n${TESTS.length - failed}/${TESTS.length} passed`);
    process.exit(failed === 0 ? 0 : 1);
});
