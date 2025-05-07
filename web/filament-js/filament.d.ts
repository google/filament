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
 * This file declares TypeScript annotations for Filament, which is implemented with an external
 * WASM library. The annotations declared in this file must match the bindings that are defined
 * in jsbindings. Note that clients are not required to use glMatrix, but we provide annotations for
 * those that do.
 */

import * as glm from "gl-matrix";

export as namespace Filament;

export function getSupportedFormatSuffix(desired: string): void;
export function init(assets: string[], onready?: (() => void) | null): void;
export function fetch(assets: string[], onDone?: (() => void) | null, onFetched?: ((name: string) => void) | null): void;
export function clearAssetCache(): void;
export function vectorToArray<T>(vector: Vector<T>): T[];
export function fitIntoUnitCube(box: Aabb): mat4;
export function multiplyMatrices(a: mat4, b: mat4): mat4;

export const assets: {[url: string]: Uint8Array};

/**
 * May be either a string exactly containing a URL loaded with Filament.init() or Filament.fetch(),
 * OR any TypedArray such as Uint8Array, Float32Array, etc., all of which match the ArrayBufferView
 * interface.
 */
export type BufferReference = string | ArrayBufferView;

export type float2 = glm.vec2|number[];
export type float3 = glm.vec3|number[];
export type float4 = glm.vec4|number[];
export type double2 = glm.vec2|number[];
export type double3 = glm.vec3|number[];
export type double4 = glm.vec4|number[];
export type mat3 = glm.mat3|number[];
export type mat4 = glm.mat4|number[];
export type quat = glm.quat|number[];

/** A C++ std::vector. */
export interface Vector<T> {
    size(): number;
    get(i: number): T;
}

export class SwapChain {}

export interface PickingQueryResult {
    renderable: number;
    depth: number;
    fragCoords: number[];
}

export type PickCallback = (result: PickingQueryResult) => void;

export class ColorGrading {
    public static Builder(): ColorGrading$Builder;
}

export interface Box {
    center: float3;
    halfExtent: float3;
}

export interface Aabb {
    min: float3;
    max: float3;
}

export interface Renderer$ClearOptions {
    clearColor?: float4;
    clear?: boolean;
    discard?: boolean;
}

export interface LightManager$ShadowOptions {
    mapSize?: number;
    shadowCascades?: number;
    constantBias?: number;
    normalBias?: number;
    shadowFar?: number;
    shadowNearHint?: number;
    shadowFarHint?: number;
    stable?: boolean;
    polygonOffsetConstant?: number;
    polygonOffsetSlope?: number;
    screenSpaceContactShadows?: boolean;
    stepCount?: number;
    maxShadowDistance?: number;
}

// Clients should use the [PixelBuffer/CompressedPixelBuffer] helper function to contruct PixelBufferDescriptor objects.
export class driver$PixelBufferDescriptor {
    constructor(byteLength: number, format: PixelDataFormat, datatype: PixelDataType);
    constructor(byteLength: number, cdtype: CompressedPixelDataType, imageSize: number, compressed: boolean);
    getBytes(): ArrayBuffer;
}

// Clients should use createTextureFromKtx1/ImageFile helper functions if low level control is not needed
export class Texture$Builder {
    public width(width: number): Texture$Builder;
    public height(height: number): Texture$Builder;
    public depth(depth: number): Texture$Builder;
    public levels(levels: number): Texture$Builder;
    public sampler(sampler: Texture$Sampler): Texture$Builder;
    public format(format: Texture$InternalFormat): Texture$Builder;
    public usage(usage: number): Texture$Builder;
    public build(engine: Engine) : Texture;
}

export class Texture {
    public static Builder(): Texture$Builder;
    public setImage(engine: Engine, level: number, pbd: driver$PixelBufferDescriptor): void;
    public setImageCube(engine: Engine, level: number, pbd: driver$PixelBufferDescriptor) : void;
    public getWidth(engine: Engine, level?: number) : number;
    public getHeight(engine: Engine, level?: number) : number;
    public getDepth(engine: Engine, level?: number) : number;
    public getLevels(engine: Engine) : number;
    public generateMipmaps(engine: Engine) : void;
}

// TODO: Remove the entity type and just use integers for parity with Filament's Java bindings.
export class Entity {
    public getId(): number;
    public delete(): void;
}

export class Skybox {
    public setColor(color: float4): void;
    public getTexture(): Texture;
}

export class LightManager$Instance {
    public delete(): void;
}

export class RenderableManager$Instance {
    public delete(): void;
}

export class TransformManager$Instance {
    public delete(): void;
}

export class TextureSampler {
    constructor(minfilter: MinFilter, magfilter: MagFilter, wrapmode: WrapMode);
    public setAnisotropy(value: number): void;
    public setCompareMode(mode: CompareMode, func: CompareFunc): void;
}

export class MaterialInstance {
    public getName(): string;
    public setBoolParameter(name: string, value: boolean): void;
    public setFloatParameter(name: string, value: number): void;
    public setFloat2Parameter(name: string, value: float2): void;
    public setFloat3Parameter(name: string, value: float3): void;
    public setFloat4Parameter(name: string, value: float4): void;
    public setMat3Parameter(name: string, value: mat4): void;
    public setMat4Parameter(name: string, value: mat3): void;
    public setTextureParameter(name: string, value: Texture, sampler: TextureSampler): void;
    public setColor3Parameter(name: string, ctype: RgbType, value: float3): void;
    public setColor4Parameter(name: string, ctype: RgbaType, value: float4): void;
    public setPolygonOffset(scale: number, constant: number): void;
    public setMaskThreshold(threshold: number): void;
    public setDoubleSided(doubleSided: boolean): void;
    public setCullingMode(mode: CullingMode): void;
    public setColorWrite(enable: boolean): void;
    public setDepthWrite(enable: boolean): void;
    public setStencilWrite(enable: boolean): void;
    public setDepthCulling(enable: boolean): void;
    public setDepthFunc(func: CompareFunc): void;
    public setStencilCompareFunction(func: CompareFunc, face?: StencilFace): void;
    public setStencilOpStencilFail(op: StencilOperation, face?: StencilFace): void;
    public setStencilOpDepthFail(op: StencilOperation, face?: StencilFace): void;
    public setStencilOpDepthStencilPass(op: StencilOperation, face?: StencilFace): void;
    public setStencilReferenceValue(value: Number, face?: StencilFace): void;
    public setStencilReadMask(readMask: Number, face?: StencilFace): void;
    public setStencilWriteMask(writeMask: Number, face?: StencilFace): void;
}

export class EntityManager {
    public static get(): EntityManager;
    public create(): Entity;
}

export class VertexBuffer$Builder {
    public vertexCount(count: number): VertexBuffer$Builder;
    public bufferCount(count: number): VertexBuffer$Builder;
    public attribute(attrib: VertexAttribute, bufindex: number, atype: VertexBuffer$AttributeType,
            offset: number, stride: number): VertexBuffer$Builder;
    public enableBufferObjects(enabled: boolean): VertexBuffer$Builder;
    public normalized(attrib: VertexAttribute): VertexBuffer$Builder;
    public normalizedIf(attrib: VertexAttribute, normalized: boolean): VertexBuffer$Builder;
    public build(engine: Engine): VertexBuffer;
}

export class IndexBuffer$Builder {
    public indexCount(count: number): IndexBuffer$Builder;
    public bufferType(type: IndexBuffer$IndexType): IndexBuffer$Builder;
    public build(engine: Engine): IndexBuffer;
}

export class BufferObject$Builder {
    public size(byteCount: number): BufferObject$Builder;
    public bindingType(type: BufferObject$BindingType): BufferObject$Builder;
    public build(engine: Engine): BufferObject;
}

export class RenderableManager$Builder {
    public geometry(slot: number, ptype: RenderableManager$PrimitiveType, vb: VertexBuffer,
            ib: IndexBuffer): RenderableManager$Builder;
    public geometryOffset(slot: number, ptype: RenderableManager$PrimitiveType, vb: VertexBuffer,
            ib: IndexBuffer, offset: number, count: number): RenderableManager$Builder;
    public geometryMinMax(slot: number, ptype: RenderableManager$PrimitiveType, vb: VertexBuffer,
            ib: IndexBuffer, offset: number, minIndex: number, maxIndex: number, count: number): RenderableManager$Builder;
    public material(geo: number, minstance: MaterialInstance): RenderableManager$Builder;
    public boundingBox(box: Box): RenderableManager$Builder;
    public layerMask(select: number, values: number): RenderableManager$Builder;
    public priority(value: number): RenderableManager$Builder;
    public culling(enable: boolean): RenderableManager$Builder;
    public castShadows(enable: boolean): RenderableManager$Builder;
    public receiveShadows(enable: boolean): RenderableManager$Builder;
    public skinning(boneCount: number): RenderableManager$Builder;
    public skinningBones(transforms: RenderableManager$Bone[]): RenderableManager$Builder;
    public skinningMatrices(transforms: mat4[]): RenderableManager$Builder;
    public morphing(enable: boolean): RenderableManager$Builder;
    public blendOrder(index: number, order: number): RenderableManager$Builder;
    public build(engine: Engine, entity: Entity): void;
}

export class RenderTarget$Builder {
    public texture(attachment: RenderTarget$AttachmentPoint, texture: Texture): RenderTarget$Builder;
    public mipLevel(attachment: RenderTarget$AttachmentPoint, mipLevel: number): RenderTarget$Builder;
    public face(attachment: RenderTarget$AttachmentPoint, face: Texture$CubemapFace): RenderTarget$Builder;
    public layer(attachment: RenderTarget$AttachmentPoint, layer: number): RenderTarget$Builder;
    public build(engine: Engine): RenderTarget;
}

export class LightManager$Builder {
    public build(engine: Engine, entity: Entity): void;
    public castLight(enable: boolean): LightManager$Builder;
    public castShadows(enable: boolean): LightManager$Builder;
    public shadowOptions(options: LightManager$ShadowOptions): LightManager$Builder;
    public color(rgb: float3): LightManager$Builder;
    public direction(value: float3): LightManager$Builder;
    public intensity(value: number): LightManager$Builder;
    public falloff(value: number): LightManager$Builder;
    public position(value: float3): LightManager$Builder;
    public spotLightCone(inner: number, outer: number): LightManager$Builder;
    public sunAngularRadius(angularRadius: number): LightManager$Builder;
    public sunHaloFalloff(haloFalloff: number): LightManager$Builder;
    public sunHaloSize(haloSize: number): LightManager$Builder;
}

export class Skybox$Builder {
    public build(engine: Engine): Skybox;
    public color(rgba: float4): Skybox$Builder;
    public environment(envmap: Texture): Skybox$Builder;
    public showSun(show: boolean): Skybox$Builder;
}

export class LightManager {
    public hasComponent(entity: Entity): boolean;
    public getInstance(entity: Entity): LightManager$Instance;
    public static Builder(ltype: LightManager$Type): LightManager$Builder;
    public getType(instance: LightManager$Instance): LightManager$Type;
    public isDirectional(instance: LightManager$Instance): boolean;
    public isPointLight(instance: LightManager$Instance): boolean;
    public isSpotLight(instance: LightManager$Instance): boolean;
    public setPosition(instance: LightManager$Instance, value: float3): void;
    public getPosition(instance: LightManager$Instance): float3;
    public setDirection(instance: LightManager$Instance, value: float3): void;
    public getDirection(instance: LightManager$Instance): float3;
    public setColor(instance: LightManager$Instance, value: float3): void;
    public getColor(instance: LightManager$Instance): float3;
    public setIntensity(instance: LightManager$Instance, intensity: number): void;
    public setIntensityEnergy(instance: LightManager$Instance, watts: number, efficiency: number): void;
    public getIntensity(instance: LightManager$Instance): number;
    public setFalloff(instance: LightManager$Instance, radius: number): void;
    public getFalloff(instance: LightManager$Instance): number;
    public setShadowOptions(instance: LightManager$Instance, options: LightManager$ShadowOptions): void;
    public setSpotLightCone(instance: LightManager$Instance, inner: number, outer: number): void;
    public setSunAngularRadius(instance: LightManager$Instance, angularRadius: number): void;
    public getSunAngularRadius(instance: LightManager$Instance): number;
    public setSunHaloSize(instance: LightManager$Instance, haloSize: number): void;
    public getSunHaloSize(instance: LightManager$Instance): number;
    public setSunHaloFalloff(instance: LightManager$Instance, haloFalloff: number): void;
    public getSunHaloFalloff(instance: LightManager$Instance): number;
    public setShadowCaster(instance: LightManager$Instance, shadowCaster: boolean): number;
    public isShadowCaster(instance: LightManager$Instance): boolean;
}

export interface RenderableManager$Bone {
    unitQuaternion: quat;
    translation: float3;
}

export class RenderableManager {
    public hasComponent(entity: Entity): boolean;
    public getInstance(entity: Entity): RenderableManager$Instance;
    public static Builder(ngeos: number): RenderableManager$Builder;
    public destroy(entity: Entity): void;
    public setAxisAlignedBoundingBox(instance: RenderableManager$Instance, aabb: Box): void;
    public setLayerMask(instance: RenderableManager$Instance, select: number, values: number): void;
    public setPriority(instance: RenderableManager$Instance, priority: number): void;
    public setCastShadows(instance: RenderableManager$Instance, enable: boolean): void;
    public setReceiveShadows(inst: RenderableManager$Instance, enable: boolean): void;
    public isShadowCaster(instance: RenderableManager$Instance): boolean;
    public isShadowReceiver(instance: RenderableManager$Instance): boolean;
    public setBones(instance: RenderableManager$Instance, transforms: RenderableManager$Bone[],
            offset: number): void
    public setBonesFromMatrices(instance: RenderableManager$Instance, transforms: mat4[],
            offset: number): void
    public setMorphWeights(instance: RenderableManager$Instance, a: number, b: number, c: number,
            d: number): void;
    public getAxisAlignedBoundingBox(instance: RenderableManager$Instance): Box;
    public getPrimitiveCount(instance: RenderableManager$Instance): number;
    public setMaterialInstanceAt(instance: RenderableManager$Instance,
            primitiveIndex: number, materialInstance: MaterialInstance): void;
    public getMaterialInstanceAt(instance: RenderableManager$Instance, primitiveIndex: number):
            MaterialInstance;
    public setGeometryAt(instance: RenderableManager$Instance, primitiveIndex: number,
            type: RenderableManager$PrimitiveType, vertices: VertexBuffer, indices: IndexBuffer,
            offset: number, count: number): void;
    public setBlendOrderAt(instance: RenderableManager$Instance, primitiveIndex: number,
            order: number): void;
    public getEnabledAttributesAt(instance: RenderableManager$Instance,
            primitiveIndex: number): number;
}

export class VertexBuffer {
    public static Builder(): VertexBuffer$Builder;
    public setBufferAt(engine: Engine, bufindex: number, f32array: BufferReference,
            byteOffset?: number): void;
    public setBufferObjectAt(engine: Engine, bufindex: number, bo: BufferObject): void;
}

export class BufferObject {
    public static Builder(): BufferObject$Builder;
    public setBuffer(engine: Engine, data: BufferReference, byteOffset?: number): void;
}

export class IndexBuffer {
    public static Builder(): IndexBuffer$Builder;
    public setBuffer(engine: Engine, u16array: BufferReference, byteOffset?: number): void;
}

export class Renderer {
    public render(swapChain: SwapChain, view: View): void;
    public setClearOptions(options: Renderer$ClearOptions): void;
    public renderView(view: View): void;
    public beginFrame(swapChain: SwapChain): boolean;
    public endFrame(): void;
}

export class Material {
    public createInstance(): MaterialInstance;
    public createNamedInstance(name: string): MaterialInstance;
    public getDefaultInstance(): MaterialInstance;
    public getName(): string;
}

export class Frustum {
    constructor(pv: mat4);
    public setProjection(pv: mat4): void;
    public getNormalizedPlane(plane: Frustum$Plane): float4;
    public intersectsBox(box: Box): boolean;
    public intersectsSphere(sphere: float4): boolean;
}

export class Camera {
    public setProjection(proj: Camera$Projection, left: number, right: number, bottom: number,
            top: number, near: number, far: number): void;
    public setProjectionFov(fovInDegrees: number, aspect: number,
            near: number, far: number, fov: Camera$Fov): void;
    public setLensProjection(focalLength: number, aspect: number, near: number, far: number): void;
    public setCustomProjection(projection: mat4, near: number, far: number): void;
    public setScaling(scale: double2): void;
    public getProjectionMatrix(): mat4;
    public getCullingProjectionMatrix(): mat4;
    public getScaling(): double4;
    public getNear(): number;
    public getCullingFar(): number;
    public setModelMatrix(view: mat4): void;
    public lookAt(eye: float3, center: float3, up: float3): void;
    public getModelMatrix(): mat4;
    public getViewMatrix(): mat4;
    public getPosition(): float3;
    public getLeftVector(): float3;
    public getUpVector(): float3;
    public getForwardVector(): float3;
    public getFrustum(): Frustum;
    public setExposure(aperture: number, shutterSpeed: number, sensitivity: number): void;
    public setExposureDirect(exposure: number): void;
    public getAperture(): number;
    public getShutterSpeed(): number;
    public getSensitivity(): number;
    public getFocalLength(): number;
    public getFocusDistance(): number;
    public setFocusDistance(distance: number): void;
    public static inverseProjection(p: mat4): mat4;
    public static computeEffectiveFocalLength(focalLength: number, focusDistance: number) : number;
    public static computeEffectiveFov(fovInDegrees: number, focusDistance: number) : number;
}

export class ColorGrading$Builder {
    public quality(qualityLevel: ColorGrading$QualityLevel): ColorGrading$Builder;
    public format(format: ColorGrading$LutFormat): ColorGrading$Builder;
    public dimensions(dim: number): ColorGrading$Builder;
    public toneMapping(toneMapping: ColorGrading$ToneMapping): ColorGrading$Builder;
    public luminanceScaling(luminanceScaling: boolean): ColorGrading$Builder;
    public gamutMapping(gamutMapping: boolean): ColorGrading$Builder;
    public exposure(exposure: number): ColorGrading$Builder;
    public nightAdaptation(adaptation: boolean): ColorGrading$Builder;
    public whiteBalance(temperature: number, tint: number): ColorGrading$Builder;
    public channelMixer(outRed: float3, outGreen: float3, outBlue: float3): ColorGrading$Builder;
    public shadowsMidtonesHighlights(shadows: float4, midtones: float4, highlights: float4,
            ranges: float4): ColorGrading$Builder;
    public slopeOffsetPower(slope: float3, offset: float3, power: float3): ColorGrading$Builder;
    public contrast(contrast: number): ColorGrading$Builder;
    public vibrance(vibrance: number): ColorGrading$Builder;
    public saturation(saturation: number): ColorGrading$Builder;
    public curves(shadowGamma: float3, midPoint: float3,
            highlightScale: float3): ColorGrading$Builder;
    public build(engine: Engine): ColorGrading;
}

export class IndirectLight {
    public static Builder(): IndirectLight$Builder;
    public setIntensity(intensity: number): void;
    public getIntensity(): number;
    public setRotation(value: mat3): void;
    public getRotation(): mat3;
    public getReflectionsTexture(): Texture;
    public getIrradianceTexture(): Texture;
    public static getDirectionEstimate(f32array: any): float3;
    public static getColorEstimate(f32array: any, direction: float3): float4;
    shfloats: Array<number>;
}

export class IndirectLight$Builder {
    public reflections(cubemap: Texture): IndirectLight$Builder;
    public irradianceTex(cubemap: Texture): IndirectLight$Builder;
    public irradianceSh(nbands: number, f32array: any): IndirectLight$Builder;
    public intensity(value: number): IndirectLight$Builder;
    public rotation(value: mat3): IndirectLight$Builder;
    public build(engine: Engine): IndirectLight;
}

export class IcoSphere {
    constructor(nsubdivs: number);
    public subdivide(): void;
    vertices: Float32Array;
    tangents: Int16Array;
    triangles: Uint16Array;
}

export class Scene {
    public addEntity(entity: Entity): void;
    public addEntities(entities: Entity[]): void;
    public getLightCount(): number;
    public getRenderableCount(): number;
    public remove(entity: Entity): void;
    public removeEntities(entities: Entity[]): void;
    public setIndirectLight(ibl: IndirectLight|null): void;
    public setSkybox(sky: Skybox|null): void;
}

export class RenderTarget {
    public getMipLevel(): number;
    public getFace(): Texture$CubemapFace;
    public getLayer(): number;
    public static Builder() : RenderTarget$Builder;
}

export class View {
    public pick(x: number, y: number, cb: PickCallback): void;
    public setCamera(camera: Camera): void;
    public setColorGrading(colorGrading: ColorGrading): void;
    public setScene(scene: Scene): void;
    public setViewport(viewport: float4): void;
    public setVisibleLayers(select: number, values: number): void;
    public setRenderTarget(renderTarget: RenderTarget): void;
    public setAmbientOcclusionOptions(options: View$AmbientOcclusionOptions): void;
    public setDepthOfFieldOptions(options: View$DepthOfFieldOptions): void;
    public setMultiSampleAntiAliasingOptions(options: View$MultiSampleAntiAliasingOptions): void;
    public setTemporalAntiAliasingOptions(options: View$TemporalAntiAliasingOptions): void;
    public setScreenSpaceReflectionsOptions(options: View$ScreenSpaceReflectionsOptions): void;
    public setBloomOptions(options: View$BloomOptions): void;
    public setFogOptions(options: View$FogOptions): void;
    public setVignetteOptions(options: View$VignetteOptions): void;
    public setGuardBandOptions(options: View$GuardBandOptions): void;
    public setStereoscopicOptions(options: View$StereoscopicOptions): void;
    public setAmbientOcclusion(ambientOcclusion: View$AmbientOcclusion): void;
    public getAmbientOcclusion(): View$AmbientOcclusion;
    public setBlendMode(mode: View$BlendMode): void;
    public getBlendMode(): View$BlendMode;
    public setPostProcessingEnabled(enabled: boolean): void;
    public setAntiAliasing(antialiasing: View$AntiAliasing): void;
    public setStencilBufferEnabled(enabled: boolean): void;
    public isStencilBufferEnabled(): boolean;
    public setTransparentPickingEnabled(enabled: boolean): void;
    public isTransparentPickingEnabled(): boolean;
}

export class TransformManager {
    public hasComponent(entity: Entity): boolean;
    public getInstance(entity: Entity): TransformManager$Instance;
    public create(entity: Entity): void;
    public destroy(entity: Entity): void;
    public setParent(instance: TransformManager$Instance, parent: TransformManager$Instance): void;
    public setTransform(instance: TransformManager$Instance, xform: mat4): void;
    public getTransform(instance: TransformManager$Instance): mat4;
    public getWorldTransform(instance: TransformManager$Instance): mat4;
    public openLocalTransformTransaction(): void;
    public commitLocalTransformTransaction(): void;
}

interface Filamesh {
    renderable: Entity;
    vertexBuffer: VertexBuffer;
    indexBuffer: IndexBuffer;
}

export class Engine {
    public static create(canvas: HTMLCanvasElement, contextOptions?: object): Engine;
    public static destroy(engine: Engine): void;
    public execute(): void;
    public createCamera(entity: Entity): Camera;
    public createMaterial(urlOrBuffer: BufferReference): Material;
    public createRenderer(): Renderer;
    public createScene(): Scene;
    public createSwapChain(): SwapChain;
    public createTextureFromJpeg(urlOrBuffer: BufferReference, options?: object): Texture;
    public createTextureFromPng(urlOrBuffer: BufferReference, options?: object): Texture;

    public static getMaxStereoscopicEyes(): number;

    public createIblFromKtx1(urlOrBuffer: BufferReference): IndirectLight;
    public createSkyFromKtx1(urlOrBuffer: BufferReference): Skybox;
    public createTextureFromKtx1(urlOrBuffer: BufferReference, options?: object): Texture;
    public createTextureFromKtx2(urlOrBuffer: BufferReference, options?: object): Texture;

    public createView(): View;

    public createAssetLoader(): gltfio$AssetLoader;

    public destroySwapChain(swapChain: SwapChain): void;
    public destroyRenderer(renderer: Renderer): void;
    public destroyView(view: View): void;
    public destroyScene(scene: Scene): void;
    public destroyCameraComponent(camera: Entity): void;
    public destroyMaterial(material: Material): void;
    public destroyEntity(entity: Entity): void;
    public destroyIndexBuffer(indexBuffer: IndexBuffer): void;
    public destroyIndirectLight(indirectLight: IndirectLight): void;
    public destroyMaterialInstance(materialInstance: MaterialInstance): void;
    public destroyRenderTarget(renderTarget: RenderTarget): void;
    public destroySkybox(skybox: Skybox): void;
    public destroyTexture(texture: Texture): void;
    public destroyColorGrading(colorGrading: ColorGrading): void;

    public getCameraComponent(entity: Entity): Camera;
    public getLightManager(): LightManager;
    public destroyVertexBuffer(vertexBuffer: VertexBuffer): void;
    public getRenderableManager(): RenderableManager;
    public getSupportedFormatSuffix(suffix: string): void;
    public getTransformManager(): TransformManager;
    public init(assets: string[], onready: () => void): void;
    public loadFilamesh(urlOrBuffer: BufferReference, definstance?: MaterialInstance, matinstances?: object): Filamesh;
}

export class Ktx2Reader {
    constructor(engine: Engine, quiet: boolean)
    public requestFormat(format: Texture$InternalFormat): void;
    public unrequestFormat(format: Texture$InternalFormat): void;
    public load(urlOrBuffer: BufferReference, transfer: TransferFunction): Texture|null;
}

export class gltfio$AssetLoader {
    public createAsset(urlOrBuffer: BufferReference): gltfio$FilamentAsset;
    public createInstancedAsset(urlOrBuffer: BufferReference,
            instances: (gltfio$FilamentInstance | null)[]): gltfio$FilamentAsset;
    public destroyAsset(asset: gltfio$FilamentAsset): void;
    public createInstance(asset: gltfio$FilamentAsset): (gltfio$FilamentInstance | null);
    public delete(): void;
}

export class gltfio$FilamentAsset {
    public loadResources(onDone: () => void|null, onFetched: (s: string) => void|null,
            basePath: string|null, asyncInterval: number|null, options?: object): void;
    public getEntities(): Entity[];
    public getEntitiesByName(name: string): Entity[];
    public getEntityByName(name: string): Entity;
    public getEntitiesByPrefix(name: string): Entity[];
    public getLightEntities(): Entity[];
    public getRenderableEntities(): Entity[];
    public getCameraEntities(): Entity[];
    public getRoot(): Entity;
    public popRenderable(): Entity;
    public getInstance(): gltfio$FilamentInstance;
    public geAssetInstances(): gltfio$FilamentInstance[];
    public getResourceUris(): string[];
    public getBoundingBox(): Aabb;
    public getName(entity: Entity): string;
    public getExtras(entity: Entity): string;
    public getWireframe(): Entity;
    public getEngine(): Engine;
    public releaseSourceData(): void;
}

export class gltfio$FilamentInstance {
    public getAsset(): gltfio$FilamentAsset;
    public getEntities(): Vector<Entity>;
    public getRoot(): Entity;
    public getAnimator(): gltfio$Animator;
    public getSkinNames(): Vector<string>;
    public attachSkin(skinIndex: number, entity: Entity): void;
    public detachSkin(skinIndex: number, entity: Entity): void;
    public getMaterialInstances(): Vector<MaterialInstance>;
    public detachMaterialInstances(): void;
    public getMaterialVariantNames(): string[];
    public applyMaterialVariant(index: number): void;
}

export class gltfio$Animator {
    public applyAnimation(index: number): void;
    public applyCrossFade(previousAnimIndex: number, previousAnimTime: number, alpha: number): void;
    public updateBoneMatrices(): void;
    public resetBoneMatrices(): void;
    public getAnimationCount(): number;
    public getAnimationDuration(index: number): number;
    public getAnimationName(index: number): string;
}

export class SurfaceOrientation$Builder {
    public constructor();
    public vertexCount(count: number): SurfaceOrientation$Builder;
    public normals(vec3array: Float32Array, stride: number): SurfaceOrientation$Builder;
    public uvs(vec2array: Float32Array, stride: number): SurfaceOrientation$Builder;
    public positions(vec3array: Float32Array, stride: number): SurfaceOrientation$Builder;
    public triangleCount(count: number): SurfaceOrientation$Builder;
    public triangles16(indices: Uint16Array): SurfaceOrientation$Builder;
    public triangles32(indices: Uint32Array): SurfaceOrientation$Builder;
    public build(): SurfaceOrientation;
}

export class SurfaceOrientation {
    public getQuats(quatCount: number): Int16Array;
    public getQuatsHalf4(quatCount: number): Uint16Array;
    public getQuatsFloat4(quatCount: number): Float32Array;
    public delete(): void;
}

export enum Frustum$Plane {
    LEFT,
    RIGHT,
    BOTTOM,
    TOP,
    FAR,
    NEAR,
}

export enum Camera$Fov {
    VERTICAL,
    HORIZONTAL,
}

export enum Camera$Projection {
    PERSPECTIVE,
    ORTHO,
}

export enum ColorGrading$QualityLevel {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA,
}

export enum ColorGrading$LutFormat {
    INTEGER,
    FLOAT,
}

export enum ColorGrading$ToneMapping {
    LINEAR,
    ACES_LEGACY,
    ACES,
    FILMIC,
    EVILS,
    REINHARD,
    DISPLAY_RANGE,
}

export enum CompressedPixelDataType {
    EAC_R11,
    EAC_R11_SIGNED,
    EAC_RG11,
    EAC_RG11_SIGNED,
    ETC2_RGB8,
    ETC2_SRGB8,
    ETC2_RGB8_A1,
    ETC2_SRGB8_A1,
    ETC2_EAC_RGBA8,
    ETC2_EAC_SRGBA8,
    DXT1_RGB,
    DXT1_RGBA,
    DXT3_RGBA,
    DXT5_RGBA,
    DXT1_SRGB,
    DXT1_SRGBA,
    DXT3_SRGBA,
    DXT5_SRGBA,
    RGBA_ASTC_4x4,
    RGBA_ASTC_5x4,
    RGBA_ASTC_5x5,
    RGBA_ASTC_6x5,
    RGBA_ASTC_6x6,
    RGBA_ASTC_8x5,
    RGBA_ASTC_8x6,
    RGBA_ASTC_8x8,
    RGBA_ASTC_10x5,
    RGBA_ASTC_10x6,
    RGBA_ASTC_10x8,
    RGBA_ASTC_10x10,
    RGBA_ASTC_12x10,
    RGBA_ASTC_12x12,
    SRGB8_ALPHA8_ASTC_4x4,
    SRGB8_ALPHA8_ASTC_5x4,
    SRGB8_ALPHA8_ASTC_5x5,
    SRGB8_ALPHA8_ASTC_6x5,
    SRGB8_ALPHA8_ASTC_6x6,
    SRGB8_ALPHA8_ASTC_8x5,
    SRGB8_ALPHA8_ASTC_8x6,
    SRGB8_ALPHA8_ASTC_8x8,
    SRGB8_ALPHA8_ASTC_10x5,
    SRGB8_ALPHA8_ASTC_10x6,
    SRGB8_ALPHA8_ASTC_10x8,
    SRGB8_ALPHA8_ASTC_10x10,
    SRGB8_ALPHA8_ASTC_12x10,
    SRGB8_ALPHA8_ASTC_12x12,
}

export enum IndexBuffer$IndexType {
    USHORT,
    UINT,
}

export enum BufferObject$BindingType {
    VERTEX,
}

export enum LightManager$Type {
    SUN,
    DIRECTIONAL,
    POINT,
    FOCUSED_SPOT,
    SPOT,
}

export enum MagFilter {
    NEAREST,
    LINEAR,
}

export enum MinFilter {
    NEAREST,
    LINEAR,
    NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_NEAREST,
    NEAREST_MIPMAP_LINEAR,
    LINEAR_MIPMAP_LINEAR,
}

export enum CompareMode {
    NONE,
    COMPARE_TO_TEXTURE,
}

export enum CompareFunc {
    LESS_EQUAL,
    GREATER_EQUAL,
    LESS,
    GREATER,
    EQUAL,
    NOT_EQUAL,
    ALWAYS,
    NEVER,
}

export enum CullingMode {
    NONE,
    FRONT,
    BACK,
    FRONT_AND_BACK,
}

export enum StencilOperation {
    KEEP,
    ZERO,
    REPLACE,
    INCR_CLAMP,
    INCR_WRAP,
    DECR_CLAMP,
    DECR_WRAP,
    INVERT,
}

export enum StencilFace {
    FRONT,
    BACK,
    FRONT_AND_BACK,
}

export enum PixelDataFormat {
    R,
    R_INTEGER,
    RG,
    RG_INTEGER,
    RGB,
    RGB_INTEGER,
    RGBA,
    RGBA_INTEGER,
    UNUSED,
    DEPTH_COMPONENT,
    DEPTH_STENCIL,
    ALPHA,
}

export enum PixelDataType {
    UBYTE,
    BYTE,
    USHORT,
    SHORT,
    UINT,
    INT,
    HALF,
    FLOAT,
    UINT_10F_11F_11F_REV,
    USHORT_565,
}

export enum RenderableManager$PrimitiveType {
    POINTS,
    LINES,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    NONE,
}

export enum RgbType {
    sRGB,
    LINEAR,
}

export enum RgbaType {
    sRGB,
    LINEAR,
    PREMULTIPLIED_sRGB,
    PREMULTIPLIED_LINEAR,
}

export enum Texture$InternalFormat {
    R8,
    R8_SNORM,
    R8UI,
    R8I,
    STENCIL8,
    R16F,
    R16UI,
    R16I,
    RG8,
    RG8_SNORM,
    RG8UI,
    RG8I,
    RGB565,
    RGB9_E5,
    RGB5_A1,
    RGBA4,
    DEPTH16,
    RGB8,
    SRGB8,
    RGB8_SNORM,
    RGB8UI,
    RGB8I,
    DEPTH24,
    R32F,
    R32UI,
    R32I,
    RG16F,
    RG16UI,
    RG16I,
    R11F_G11F_B10F,
    RGBA8,
    SRGB8_A8,
    RGBA8_SNORM,
    UNUSED,
    RGB10_A2,
    RGBA8UI,
    RGBA8I,
    DEPTH32F,
    DEPTH24_STENCIL8,
    DEPTH32F_STENCIL8,
    RGB16F,
    RGB16UI,
    RGB16I,
    RG32F,
    RG32UI,
    RG32I,
    RGBA16F,
    RGBA16UI,
    RGBA16I,
    RGB32F,
    RGB32UI,
    RGB32I,
    RGBA32F,
    RGBA32UI,
    RGBA32I,
    EAC_R11,
    EAC_R11_SIGNED,
    EAC_RG11,
    EAC_RG11_SIGNED,
    ETC2_RGB8,
    ETC2_SRGB8,
    ETC2_RGB8_A1,
    ETC2_SRGB8_A1,
    ETC2_EAC_RGBA8,
    ETC2_EAC_SRGBA8,
    DXT1_RGB,
    DXT1_RGBA,
    DXT3_RGBA,
    DXT5_RGBA,
    DXT1_SRGB,
    DXT1_SRGBA,
    DXT3_SRGBA,
    DXT5_SRGBA,
    RGBA_ASTC_4x4,
    RGBA_ASTC_5x4,
    RGBA_ASTC_5x5,
    RGBA_ASTC_6x5,
    RGBA_ASTC_6x6,
    RGBA_ASTC_8x5,
    RGBA_ASTC_8x6,
    RGBA_ASTC_8x8,
    RGBA_ASTC_10x5,
    RGBA_ASTC_10x6,
    RGBA_ASTC_10x8,
    RGBA_ASTC_10x10,
    RGBA_ASTC_12x10,
    RGBA_ASTC_12x12,
    SRGB8_ALPHA8_ASTC_4x4,
    SRGB8_ALPHA8_ASTC_5x4,
    SRGB8_ALPHA8_ASTC_5x5,
    SRGB8_ALPHA8_ASTC_6x5,
    SRGB8_ALPHA8_ASTC_6x6,
    SRGB8_ALPHA8_ASTC_8x5,
    SRGB8_ALPHA8_ASTC_8x6,
    SRGB8_ALPHA8_ASTC_8x8,
    SRGB8_ALPHA8_ASTC_10x5,
    SRGB8_ALPHA8_ASTC_10x6,
    SRGB8_ALPHA8_ASTC_10x8,
    SRGB8_ALPHA8_ASTC_10x10,
    SRGB8_ALPHA8_ASTC_12x10,
    SRGB8_ALPHA8_ASTC_12x12,
}

export enum Texture$Sampler {
    SAMPLER_2D,
    SAMPLER_CUBEMAP,
    SAMPLER_EXTERNAL,
}

// This enum is a bit different the others because it can be used in a bitfield.
// It is a "const enum" which means TypeScript will simply create a constant for each member.
// It does not contain the $ delimiter to avoid interference with the embind class.
export const enum TextureUsage {
    COLOR_ATTACHMENT = 1,
    DEPTH_ATTACHMENT = 2,
    STENCIL_ATTACHMENT = 4,
    UPLOADABLE = 8,
    SAMPLEABLE = 16,
    SUBPASS_INPUT = 32,
    DEFAULT = UPLOADABLE | SAMPLEABLE,
}

export enum Texture$CubemapFace {
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z,
}

export enum RenderTarget$AttachmentPoint {
    COLOR,
    DEPTH,
}

export enum View$AmbientOcclusion {
    NONE,
    SSAO,
}

export enum VertexAttribute {
    POSITION = 0,
    TANGENTS = 1,
    COLOR = 2,
    UV0 = 3,
    UV1 = 4,
    BONE_INDICES = 5,
    BONE_WEIGHTS = 6,
    UNUSED = 7,
    CUSTOM0 = 8,
    CUSTOM1 = 9,
    CUSTOM2 = 10,
    CUSTOM3 = 11,
    CUSTOM4 = 12,
    CUSTOM5 = 13,
    CUSTOM6 = 14,
    CUSTOM7 = 15,
    MORPH_POSITION_0 = CUSTOM0,
    MORPH_POSITION_1 = CUSTOM1,
    MORPH_POSITION_2 = CUSTOM2,
    MORPH_POSITION_3 = CUSTOM3,
    MORPH_TANGENTS_0 = CUSTOM4,
    MORPH_TANGENTS_1 = CUSTOM5,
    MORPH_TANGENTS_2 = CUSTOM6,
    MORPH_TANGENTS_3 = CUSTOM7,
}

export enum VertexBuffer$AttributeType {
    BYTE,
    BYTE2,
    BYTE3,
    BYTE4,
    UBYTE,
    UBYTE2,
    UBYTE3,
    UBYTE4,
    SHORT,
    SHORT2,
    SHORT3,
    SHORT4,
    USHORT,
    USHORT2,
    USHORT3,
    USHORT4,
    INT,
    UINT,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    HALF,
    HALF2,
    HALF3,
    HALF4,
}

export enum WrapMode {
    CLAMP_TO_EDGE,
    REPEAT,
    MIRRORED_REPEAT,
}

export enum Ktx2Reader$TransferFunction {
    LINEAR,
    sRGB,
}

export enum Ktx2Reader$Result {
    SUCCESS,
    COMPRESSED_TRANSCODE_FAILURE,
    UNCOMPRESSED_TRANSCODE_FAILURE,
    FORMAT_UNSUPPORTED,
    FORMAT_ALREADY_REQUESTED,
}

export function _malloc(size: number): number;
export function _free(size: number): void;

interface HeapInterface {
    set(buffer: any, pointer: number): any;
    subarray(buffer: any, offset: number): any;
}

export const HEAPU8 : HeapInterface;

// The remainder of this file is generated by beamsplitter

/**
 * Generic quality level.
 */
export enum View$QualityLevel {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA,
}

export enum View$BlendMode {
    OPAQUE,
    TRANSLUCENT,
}

/**
 * Dynamic resolution can be used to either reach a desired target frame rate
 * by lowering the resolution of a View, or to increase the quality when the
 * rendering is faster than the target frame rate.
 *
 * This structure can be used to specify the minimum scale factor used when
 * lowering the resolution of a View, and the maximum scale factor used when
 * increasing the resolution for higher quality rendering. The scale factors
 * can be controlled on each X and Y axis independently. By default, all scale
 * factors are set to 1.0.
 *
 * enabled:   enable or disables dynamic resolution on a View
 *
 * homogeneousScaling: by default the system scales the major axis first. Set this to true
 *                     to force homogeneous scaling.
 *
 * minScale:  the minimum scale in X and Y this View should use
 *
 * maxScale:  the maximum scale in X and Y this View should use
 *
 * quality:   upscaling quality.
 *            LOW: 1 bilinear tap, Medium: 4 bilinear taps, High: 9 bilinear taps (tent)
 *
 * \note
 * Dynamic resolution is only supported on platforms where the time to render
 * a frame can be measured accurately. Dynamic resolution is currently only
 * supported on Android.
 *
 * @see Renderer::FrameRateOptions
 *
 */
export interface View$DynamicResolutionOptions {
    /**
     * minimum scale factors in x and y
     */
    minScale?: float2;
    /**
     * maximum scale factors in x and y
     */
    maxScale?: float2;
    /**
     * sharpness when QualityLevel::MEDIUM or higher is used [0 (disabled), 1 (sharpest)]
     */
    sharpness?: number;
    /**
     * enable or disable dynamic resolution
     */
    enabled?: boolean;
    /**
     * set to true to force homogeneous scaling
     */
    homogeneousScaling?: boolean;
    /**
     * Upscaling quality
     * LOW:    bilinear filtered blit. Fastest, poor quality
     * MEDIUM: Qualcomm Snapdragon Game Super Resolution (SGSR) 1.0
     * HIGH:   AMD FidelityFX FSR1 w/ mobile optimizations
     * ULTRA:  AMD FidelityFX FSR1
     *      FSR1 and SGSR require a well anti-aliased (MSAA or TAA), noise free scene. Avoid FXAA and dithering.
     *
     * The default upscaling quality is set to LOW.
     */
    quality?: View$QualityLevel;
}

export enum View$BloomOptions$BlendMode {
    ADD, // Bloom is modulated by the strength parameter and added to the scene
    INTERPOLATE, // Bloom is interpolated with the scene using the strength parameter
}

/**
 * Options to control the bloom effect
 *
 * enabled:     Enable or disable the bloom post-processing effect. Disabled by default.
 *
 * levels:      Number of successive blurs to achieve the blur effect, the minimum is 3 and the
 *              maximum is 12. This value together with resolution influences the spread of the
 *              blur effect. This value can be silently reduced to accommodate the original
 *              image size.
 *
 * resolution:  Resolution of bloom's minor axis. The minimum value is 2^levels and the
 *              the maximum is lower of the original resolution and 4096. This parameter is
 *              silently clamped to the minimum and maximum.
 *              It is highly recommended that this value be smaller than the target resolution
 *              after dynamic resolution is applied (horizontally and vertically).
 *
 * strength:    how much of the bloom is added to the original image. Between 0 and 1.
 *
 * blendMode:   Whether the bloom effect is purely additive (false) or mixed with the original
 *              image (true).
 *
 * threshold:   When enabled, a threshold at 1.0 is applied on the source image, this is
 *              useful for artistic reasons and is usually needed when a dirt texture is used.
 *
 * dirt:        A dirt/scratch/smudges texture (that can be RGB), which gets added to the
 *              bloom effect. Smudges are visible where bloom occurs. Threshold must be
 *              enabled for the dirt effect to work properly.
 *
 * dirtStrength: Strength of the dirt texture.
 */
export interface View$BloomOptions {
    // JavaScript binding for dirt is not yet supported, must use default value.
    // JavaScript binding for dirtStrength is not yet supported, must use default value.
    /**
     * bloom's strength between 0.0 and 1.0
     */
    strength?: number;
    /**
     * resolution of vertical axis (2^levels to 2048)
     */
    resolution?: number;
    /**
     * number of blur levels (1 to 11)
     */
    levels?: number;
    /**
     * how the bloom effect is applied
     */
    blendMode?: View$BloomOptions$BlendMode;
    /**
     * whether to threshold the source
     */
    threshold?: boolean;
    /**
     * enable or disable bloom
     */
    enabled?: boolean;
    /**
     * limit highlights to this value before bloom [10, +inf]
     */
    highlight?: number;
    /**
     * Bloom quality level.
     * LOW (default): use a more optimized down-sampling filter, however there can be artifacts
     *      with dynamic resolution, this can be alleviated by using the homogenous mode.
     * MEDIUM: Good balance between quality and performance.
     * HIGH: In this mode the bloom resolution is automatically increased to avoid artifacts.
     *      This mode can be significantly slower on mobile, especially at high resolution.
     *      This mode greatly improves the anamorphic bloom.
     */
    quality?: View$QualityLevel;
    /**
     * enable screen-space lens flare
     */
    lensFlare?: boolean;
    /**
     * enable starburst effect on lens flare
     */
    starburst?: boolean;
    /**
     * amount of chromatic aberration
     */
    chromaticAberration?: number;
    /**
     * number of flare "ghosts"
     */
    ghostCount?: number;
    /**
     * spacing of the ghost in screen units [0, 1[
     */
    ghostSpacing?: number;
    /**
     * hdr threshold for the ghosts
     */
    ghostThreshold?: number;
    /**
     * thickness of halo in vertical screen units, 0 to disable
     */
    haloThickness?: number;
    /**
     * radius of halo in vertical screen units [0, 0.5]
     */
    haloRadius?: number;
    /**
     * hdr threshold for the halo
     */
    haloThreshold?: number;
}

/**
 * Options to control large-scale fog in the scene
 */
export interface View$FogOptions {
    /**
     * Distance in world units [m] from the camera to where the fog starts ( >= 0.0 )
     */
    distance?: number;
    /**
     * Distance in world units [m] after which the fog calculation is disabled.
     * This can be used to exclude the skybox, which is desirable if it already contains clouds or
     * fog. The default value is +infinity which applies the fog to everything.
     *
     * Note: The SkyBox is typically at a distance of 1e19 in world space (depending on the near
     * plane distance and projection used though).
     */
    cutOffDistance?: number;
    /**
     * fog's maximum opacity between 0 and 1
     */
    maximumOpacity?: number;
    /**
     * Fog's floor in world units [m]. This sets the "sea level".
     */
    height?: number;
    /**
     * How fast the fog dissipates with altitude. heightFalloff has a unit of [1/m].
     * It can be expressed as 1/H, where H is the altitude change in world units [m] that causes a
     * factor 2.78 (e) change in fog density.
     *
     * A falloff of 0 means the fog density is constant everywhere and may result is slightly
     * faster computations.
     */
    heightFalloff?: number;
    /**
     *  Fog's color is used for ambient light in-scattering, a good value is
     *  to use the average of the ambient light, possibly tinted towards blue
     *  for outdoors environments. Color component's values should be between 0 and 1, values
     *  above one are allowed but could create a non energy-conservative fog (this is dependant
     *  on the IBL's intensity as well).
     *
     *  We assume that our fog has no absorption and therefore all the light it scatters out
     *  becomes ambient light in-scattering and has lost all directionality, i.e.: scattering is
     *  isotropic. This somewhat simulates Rayleigh scattering.
     *
     *  This value is used as a tint instead, when fogColorFromIbl is enabled.
     *
     *  @see fogColorFromIbl
     */
    color?: float3;
    /**
     * Extinction factor in [1/m] at altitude 'height'. The extinction factor controls how much
     * light is absorbed and out-scattered per unit of distance. Each unit of extinction reduces
     * the incoming light to 37% of its original value.
     *
     * Note: The extinction factor is related to the fog density, it's usually some constant K times
     * the density at sea level (more specifically at fog height). The constant K depends on
     * the composition of the fog/atmosphere.
     *
     * For historical reason this parameter is called `density`.
     */
    density?: number;
    /**
     * Distance in world units [m] from the camera where the Sun in-scattering starts.
     */
    inScatteringStart?: number;
    /**
     * Very inaccurately simulates the Sun's in-scattering. That is, the light from the sun that
     * is scattered (by the fog) towards the camera.
     * Size of the Sun in-scattering (>0 to activate). Good values are >> 1 (e.g. ~10 - 100).
     * Smaller values result is a larger scattering size.
     */
    inScatteringSize?: number;
    /**
     * The fog color will be sampled from the IBL in the view direction and tinted by `color`.
     * Depending on the scene this can produce very convincing results.
     *
     * This simulates a more anisotropic phase-function.
     *
     * `fogColorFromIbl` is ignored when skyTexture is specified.
     *
     * @see skyColor
     */
    fogColorFromIbl?: boolean;
    // JavaScript binding for skyColor is not yet supported, must use default value.
    /**
     * Enable or disable large-scale fog
     */
    enabled?: boolean;
}

export enum View$DepthOfFieldOptions$Filter {
    NONE,
    UNUSED,
    MEDIAN,
}

/**
 * Options to control Depth of Field (DoF) effect in the scene.
 *
 * cocScale can be used to set the depth of field blur independently from the camera
 * aperture, e.g. for artistic reasons. This can be achieved by setting:
 *      cocScale = cameraAperture / desiredDoFAperture
 *
 * @see Camera
 */
export interface View$DepthOfFieldOptions {
    /**
     * circle of confusion scale factor (amount of blur)
     */
    cocScale?: number;
    /**
     * width/height aspect ratio of the circle of confusion (simulate anamorphic lenses)
     */
    cocAspectRatio?: number;
    /**
     * maximum aperture diameter in meters (zero to disable rotation)
     */
    maxApertureDiameter?: number;
    /**
     * enable or disable depth of field effect
     */
    enabled?: boolean;
    /**
     * filter to use for filling gaps in the kernel
     */
    filter?: View$DepthOfFieldOptions$Filter;
    /**
     * perform DoF processing at native resolution
     */
    nativeResolution?: boolean;
    /**
     * Number of of rings used by the gather kernels. The number of rings affects quality
     * and performance. The actual number of sample per pixel is defined
     * as (ringCount * 2 - 1)^2. Here are a few commonly used values:
     *       3 rings :   25 ( 5x 5 grid)
     *       4 rings :   49 ( 7x 7 grid)
     *       5 rings :   81 ( 9x 9 grid)
     *      17 rings : 1089 (33x33 grid)
     *
     * With a maximum circle-of-confusion of 32, it is never necessary to use more than 17 rings.
     *
     * Usually all three settings below are set to the same value, however, it is often
     * acceptable to use a lower ring count for the "fast tiles", which improves performance.
     * Fast tiles are regions of the screen where every pixels have a similar
     * circle-of-confusion radius.
     *
     * A value of 0 means default, which is 5 on desktop and 3 on mobile.
     *
     * @{
     */
    foregroundRingCount?: number;
    /**
     * number of kernel rings for background tiles
     */
    backgroundRingCount?: number;
    /**
     * number of kernel rings for fast tiles
     */
    fastGatherRingCount?: number;
    /**
     * maximum circle-of-confusion in pixels for the foreground, must be in [0, 32] range.
     * A value of 0 means default, which is 32 on desktop and 24 on mobile.
     */
    maxForegroundCOC?: number;
    /**
     * maximum circle-of-confusion in pixels for the background, must be in [0, 32] range.
     * A value of 0 means default, which is 32 on desktop and 24 on mobile.
     */
    maxBackgroundCOC?: number;
}

/**
 * Options to control the vignetting effect.
 */
export interface View$VignetteOptions {
    /**
     * high values restrict the vignette closer to the corners, between 0 and 1
     */
    midPoint?: number;
    /**
     * controls the shape of the vignette, from a rounded rectangle (0.0), to an oval (0.5), to a circle (1.0)
     */
    roundness?: number;
    /**
     * softening amount of the vignette effect, between 0 and 1
     */
    feather?: number;
    /**
     * color of the vignette effect, alpha is currently ignored
     */
    color?: float4;
    /**
     * enables or disables the vignette effect
     */
    enabled?: boolean;
}

/**
 * Structure used to set the precision of the color buffer and related quality settings.
 *
 * @see setRenderQuality, getRenderQuality
 */
export interface View$RenderQuality {
    /**
     * Sets the quality of the HDR color buffer.
     *
     * A quality of HIGH or ULTRA means using an RGB16F or RGBA16F color buffer. This means
     * colors in the LDR range (0..1) have a 10 bit precision. A quality of LOW or MEDIUM means
     * using an R11G11B10F opaque color buffer or an RGBA16F transparent color buffer. With
     * R11G11B10F colors in the LDR range have a precision of either 6 bits (red and green
     * channels) or 5 bits (blue channel).
     */
    hdrColorBuffer?: View$QualityLevel;
}

export enum View$AmbientOcclusionOptions$AmbientOcclusionType {
    SAO, // use Scalable Ambient Occlusion
    GTAO, // use Ground Truth-Based Ambient Occlusion
}

/**
 * Screen Space Cone Tracing (SSCT) options
 * Ambient shadows from dominant light
 */
export interface View$AmbientOcclusionOptions$Ssct {
    /**
     * full cone angle in radian, between 0 and pi/2
     */
    lightConeRad?: number;
    /**
     * how far shadows can be cast
     */
    shadowDistance?: number;
    /**
     * max distance for contact
     */
    contactDistanceMax?: number;
    /**
     * intensity
     */
    intensity?: number;
    /**
     * light direction
     */
    lightDirection?: float3;
    /**
     * depth bias in world units (mitigate self shadowing)
     */
    depthBias?: number;
    /**
     * depth slope bias (mitigate self shadowing)
     */
    depthSlopeBias?: number;
    /**
     * tracing sample count, between 1 and 255
     */
    sampleCount?: number;
    /**
     * # of rays to trace, between 1 and 255
     */
    rayCount?: number;
    /**
     * enables or disables SSCT
     */
    enabled?: boolean;
}

/**
 * Ground Truth-base Ambient Occlusion (GTAO) options
 */
export interface View$AmbientOcclusionOptions$Gtao {
    /**
     * # of slices. Higher value makes less noise.
     */
    sampleSliceCount?: number;
    /**
     * # of steps the radius is divided into for integration. Higher value makes less bias.
     */
    sampleStepsPerSlice?: number;
    /**
     * thickness heuristic, should be closed to 0
     */
    thicknessHeuristic?: number;
}

/**
 * Options for screen space Ambient Occlusion (SSAO) and Screen Space Cone Tracing (SSCT)
 * @see setAmbientOcclusionOptions()
 */
export interface View$AmbientOcclusionOptions {
    /**
     * Type of ambient occlusion algorithm.
     */
    aoType?: View$AmbientOcclusionOptions$AmbientOcclusionType;
    /**
     * Ambient Occlusion radius in meters, between 0 and ~10.
     */
    radius?: number;
    /**
     * Controls ambient occlusion's contrast. Must be positive.
     */
    power?: number;
    /**
     * Self-occlusion bias in meters. Use to avoid self-occlusion.
     * Between 0 and a few mm. No effect when aoType set to GTAO
     */
    bias?: number;
    /**
     * How each dimension of the AO buffer is scaled. Must be either 0.5 or 1.0.
     */
    resolution?: number;
    /**
     * Strength of the Ambient Occlusion effect.
     */
    intensity?: number;
    /**
     * depth distance that constitute an edge for filtering
     */
    bilateralThreshold?: number;
    /**
     * affects # of samples used for AO and params for filtering
     */
    quality?: View$QualityLevel;
    /**
     * affects AO smoothness. Recommend setting to HIGH when aoType set to GTAO.
     */
    lowPassFilter?: View$QualityLevel;
    /**
     * affects AO buffer upsampling quality
     */
    upsampling?: View$QualityLevel;
    /**
     * enables or disables screen-space ambient occlusion
     */
    enabled?: boolean;
    /**
     * enables bent normals computation from AO, and specular AO
     */
    bentNormals?: boolean;
    /**
     * min angle in radian to consider. No effect when aoType set to GTAO.
     */
    minHorizonAngleRad?: number;
    // JavaScript binding for ssct is not yet supported, must use default value.
    // JavaScript binding for gtao is not yet supported, must use default value.
}

/**
 * Options for Multi-Sample Anti-aliasing (MSAA)
 * @see setMultiSampleAntiAliasingOptions()
 */
export interface View$MultiSampleAntiAliasingOptions {
    /**
     * enables or disables msaa
     */
    enabled?: boolean;
    /**
     * sampleCount number of samples to use for multi-sampled anti-aliasing.\n
     *              0: treated as 1
     *              1: no anti-aliasing
     *              n: sample count. Effective sample could be different depending on the
     *                 GPU capabilities.
     */
    sampleCount?: number;
    /**
     * custom resolve improves quality for HDR scenes, but may impact performance.
     */
    customResolve?: boolean;
}

export enum View$TemporalAntiAliasingOptions$BoxType {
    AABB, // use an AABB neighborhood
    VARIANCE, // use the variance of the neighborhood (not recommended)
    AABB_VARIANCE, // use both AABB and variance
}

export enum View$TemporalAntiAliasingOptions$BoxClipping {
    ACCURATE, // Accurate box clipping
    CLAMP, // clamping
    NONE, // no rejections (use for debugging)
}

export enum View$TemporalAntiAliasingOptions$JitterPattern {
    RGSS_X4,
    UNIFORM_HELIX_X4,
    HALTON_23_X8,
    HALTON_23_X16,
    HALTON_23_X32,
}

/**
 * Options for Temporal Anti-aliasing (TAA)
 * Most TAA parameters are extremely costly to change, as they will trigger the TAA post-process
 * shaders to be recompiled. These options should be changed or set during initialization.
 * `filterWidth`, `feedback` and `jitterPattern`, however, can be changed at any time.
 *
 * `feedback` of 0.1 effectively accumulates a maximum of 19 samples in steady state.
 * see "A Survey of Temporal Antialiasing Techniques" by Lei Yang and all for more information.
 *
 * @see setTemporalAntiAliasingOptions()
 */
export interface View$TemporalAntiAliasingOptions {
    /**
     * reconstruction filter width typically between 1 (sharper) and 2 (smoother)
     */
    filterWidth?: number;
    /**
     * history feedback, between 0 (maximum temporal AA) and 1 (no temporal AA).
     */
    feedback?: number;
    /**
     * texturing lod bias (typically -1 or -2)
     */
    lodBias?: number;
    /**
     * post-TAA sharpen, especially useful when upscaling is true.
     */
    sharpness?: number;
    /**
     * enables or disables temporal anti-aliasing
     */
    enabled?: boolean;
    /**
     * 4x TAA upscaling. Disables Dynamic Resolution. [BETA]
     */
    upscaling?: boolean;
    /**
     * whether to filter the history buffer
     */
    filterHistory?: boolean;
    /**
     * whether to apply the reconstruction filter to the input
     */
    filterInput?: boolean;
    /**
     * whether to use the YcoCg color-space for history rejection
     */
    useYCoCg?: boolean;
    /**
     * type of color gamut box
     */
    boxType?: View$TemporalAntiAliasingOptions$BoxType;
    /**
     * clipping algorithm
     */
    boxClipping?: View$TemporalAntiAliasingOptions$BoxClipping;
    jitterPattern?: View$TemporalAntiAliasingOptions$JitterPattern;
    varianceGamma?: number;
    /**
     * adjust the feedback dynamically to reduce flickering
     */
    preventFlickering?: boolean;
    /**
     * whether to apply history reprojection (debug option)
     */
    historyReprojection?: boolean;
}

/**
 * Options for Screen-space Reflections.
 * @see setScreenSpaceReflectionsOptions()
 */
export interface View$ScreenSpaceReflectionsOptions {
    /**
     * ray thickness, in world units
     */
    thickness?: number;
    /**
     * bias, in world units, to prevent self-intersections
     */
    bias?: number;
    /**
     * maximum distance, in world units, to raycast
     */
    maxDistance?: number;
    /**
     * stride, in texels, for samples along the ray.
     */
    stride?: number;
    enabled?: boolean;
}

/**
 * Options for the  screen-space guard band.
 * A guard band can be enabled to avoid some artifacts towards the edge of the screen when
 * using screen-space effects such as SSAO. Enabling the guard band reduces performance slightly.
 * Currently the guard band can only be enabled or disabled.
 */
export interface View$GuardBandOptions {
    enabled?: boolean;
}

/**
 * List of available post-processing anti-aliasing techniques.
 * @see setAntiAliasing, getAntiAliasing, setSampleCount
 */
export enum View$AntiAliasing {
    NONE, // no anti aliasing performed as part of post-processing
    FXAA, // FXAA is a low-quality but very efficient type of anti-aliasing. (default).
}

/**
 * List of available post-processing dithering techniques.
 */
export enum View$Dithering {
    NONE, // No dithering
    TEMPORAL, // Temporal dithering (default)
}

/**
 * List of available shadow mapping techniques.
 * @see setShadowType
 */
export enum View$ShadowType {
    PCF, // percentage-closer filtered shadows (default)
    VSM, // variance shadows
    DPCF, // PCF with contact hardening simulation
    PCSS, // PCF with soft shadows and contact hardening
    PCFd,
}

/**
 * View-level options for VSM Shadowing.
 * @see setVsmShadowOptions()
 * @warning This API is still experimental and subject to change.
 */
export interface View$VsmShadowOptions {
    /**
     * Sets the number of anisotropic samples to use when sampling a VSM shadow map. If greater
     * than 0, mipmaps will automatically be generated each frame for all lights.
     *
     * The number of anisotropic samples = 2 ^ vsmAnisotropy.
     */
    anisotropy?: number;
    /**
     * Whether to generate mipmaps for all VSM shadow maps.
     */
    mipmapping?: boolean;
    /**
     * The number of MSAA samples to use when rendering VSM shadow maps.
     * Must be a power-of-two and greater than or equal to 1. A value of 1 effectively turns
     * off MSAA.
     * Higher values may not be available depending on the underlying hardware.
     */
    msaaSamples?: number;
    /**
     * Whether to use a 32-bits or 16-bits texture format for VSM shadow maps. 32-bits
     * precision is rarely needed, but it does reduces light leaks as well as "fading"
     * of the shadows in some situations. Setting highPrecision to true for a single
     * shadow map will double the memory usage of all shadow maps.
     */
    highPrecision?: boolean;
    /**
     * VSM minimum variance scale, must be positive.
     */
    minVarianceScale?: number;
    /**
     * VSM light bleeding reduction amount, between 0 and 1.
     */
    lightBleedReduction?: number;
}

/**
 * View-level options for DPCF and PCSS Shadowing.
 * @see setSoftShadowOptions()
 * @warning This API is still experimental and subject to change.
 */
export interface View$SoftShadowOptions {
    /**
     * Globally scales the penumbra of all DPCF and PCSS shadows
     * Acceptable values are greater than 0
     */
    penumbraScale?: number;
    /**
     * Globally scales the computed penumbra ratio of all DPCF and PCSS shadows.
     * This effectively controls the strength of contact hardening effect and is useful for
     * artistic purposes. Higher values make the shadows become softer faster.
     * Acceptable values are equal to or greater than 1.
     */
    penumbraRatioScale?: number;
}

/**
 * Options for stereoscopic (multi-eye) rendering.
 */
export interface View$StereoscopicOptions {
    enabled?: boolean;
}
