// -------------------------------------------------------------------------------------------------
// Declares TypeScript annotations for Filament, which is implemented with an external WASM library.
// We will eventually autogenerate this file, but for now it is incomplete and hand-authored.
// -------------------------------------------------------------------------------------------------

export as namespace Filament;

export function getSupportedFormatSuffix(desired: string): void;
export function init(assets: string[], onready: () => void): void;
export function fetch(assets: string[], onready: () => void): void;

export const assets: {[url: string]: Uint8Array};

export type Box = number[][];
export type float2 = number[];
export type float3 = number[];
export type float4 = number[];
export type mat3 = number[];
export type mat4 = number[];

export class Entity {}
export class Skybox {}
export class Texture {}
export class SwapChain {}

export class TransformManager$Instance {
    public delete(): void;
}

export class TextureSampler {
    constructor(minfilter: MinFilter, magfilter: MagFilter, wrapmode: WrapMode);
}

export class MaterialInstance {
    public setFloatParameter(name: string, value: number): void;
    public setFloat2Parameter(name: string, value: float2): void;
    public setFloat3Parameter(name: string, value: float3): void;
    public setFloat4Parameter(name: string, value: float4): void;
    public setTextureParameter(name: string, value: Texture, sampler: TextureSampler): void;
    public setColorParameter(name: string, ctype: RgbType, value: float3): void;
    public setPolygonOffset(scale: number, constant: number): void;
}

export class EntityManager {
    public static get(): EntityManager;
    public create(): Entity;
}

export class VertexBuffer$Builder {
    public vertexCount(count: number): VertexBuffer$Builder;
    public bufferCount(count: number): VertexBuffer$Builder;
    public attribute(attrib: VertexAttribute,
                        bufindex: number,
                        atype: VertexBuffer$AttributeType,
                        offset: number, stride: number): VertexBuffer$Builder;
    public normalized(attrib: VertexAttribute): VertexBuffer$Builder;
    public build(engine: Engine): VertexBuffer;
}

export class IndexBuffer$Builder {
    public indexCount(count: number): IndexBuffer$Builder;
    public bufferType(IndexBuffer$IndexType): IndexBuffer$Builder;
    public build(engine: Engine): IndexBuffer;
}

export class RenderableManager$Builder {
    public boundingBox(box: Box): RenderableManager$Builder;
    public culling(enable: boolean): RenderableManager$Builder;
    public material(geo: number, minstance: MaterialInstance): RenderableManager$Builder;
    public geometry(slot: number,
                    ptype: RenderableManager$PrimitiveType,
                    vb: VertexBuffer,
                    ib: IndexBuffer): RenderableManager$Builder;
    public build(engine: Engine, entity: Entity): void;
}

export class LightManager$Builder {
    public color(rgb: float3): LightManager$Builder;
    public intensity(value: number): LightManager$Builder;
    public position(value: float3): LightManager$Builder;
    public direction(value: float3): LightManager$Builder;
    public build(engine: Engine, entity: Entity): void;
}

export class LightManager {
    public static Builder(ltype: LightManager$Type): LightManager$Builder;
}

export class RenderableManager {
    public static Builder(ngeos: number): RenderableManager$Builder;
}

export class VertexBuffer {
    public static Builder(): VertexBuffer$Builder;
    public setBufferAt(engine: Engine, bufindex: number, f32array: any): void;
}

export class IndexBuffer {
    public static Builder(): IndexBuffer$Builder;
    public setBuffer(engine: Engine, u16array: any): void;
}

export class Renderer {
    public render(swapChain: SwapChain, view: View): void;
}

export class Material {
    public createInstance(): MaterialInstance;
}

export class Camera {
    public lookAt(eye: number[], center: number[], up: number[]): void;
    public setProjectionFov(fovInDegrees: number, aspect: number,
                            near: number, far: number, fov: Camera$Fov): void;
}

export class IndirectLight {
    public setIntensity(intensity: number);
}

export class Scene {
    public addEntity(entity: Entity);
    public getLightCount(): number;
    public getRenderableCount(): number;
    public remove(entity: Entity);
    public setIndirectLight(ibl: IndirectLight);
    public setSkybox(sky: Skybox);
}

export class View {
    public setCamera(camera: Camera);
    public setClearColor(color: number[]);
    public setScene(scene: Scene);
    public setViewport(viewport: number[]);
}

export class TransformManager {
    public getInstance(entity: Entity): TransformManager$Instance;
    public setTransform(instance: TransformManager$Instance, xform: mat4): void;
}

interface Filamesh {
    renderable: Entity;
    vertexBuffer: VertexBuffer;
    indexBuffer: IndexBuffer;
}

export class Engine {
    public static create(HTMLCanvasElement): Engine;
    public createCamera(): Camera;
    public createIblFromKtx(url: string): IndirectLight;
    public createMaterial(url: string): Material;
    public createRenderer(): Renderer;
    public createScene(): Scene;
    public createSkyFromKtx(url: string): Skybox;
    public createSwapChain(): SwapChain;
    public createTextureFromJpeg(url: string): Texture;
    public createTextureFromPng(url: string): Texture;
    public createView(): View;
    public destroySkybox(skybox: Skybox): void;
    public getSupportedFormatSuffix(suffix: string): void;
    public getTransformManager(): TransformManager;
    public init(assets: string[], onready: () => void): void;
    public loadFilamesh(url: string,
                        definstance: MaterialInstance,
                        matinstances: object): Filamesh;
}

export enum Camera$Fov {
    VERTICAL,
    HORIZONTAL,
}

export enum Camera$Projection {
    PERSPECTIVE,
    ORTHO,
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

export enum PixelDataFormat {
    R,
    R_INTEGER,
    RG,
    RG_INTEGER,
    RGB,
    RGB_INTEGER,
    RGBA,
    RGBA_INTEGER,
    RGBM,
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
}

export enum RenderableManager$PrimitiveType {
    POINTS,
    LINES,
    TRIANGLES,
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

export enum Texture$Usage {
    DEFAULT,
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT,
}

export enum VertexAttribute {
    POSITION,
    TANGENTS,
    COLOR,
    UV0,
    UV1,
    BONE_INDICES,
    BONE_WEIGHTS,
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

export enum View$AntiAliasing {
    NONE,
    FXAA,
}

export enum View$DepthPrepass {
    DEFAULT,
    DISABLED,
    ENABLED,
}

export enum WrapMode {
    CLAMP_TO_EDGE,
    REPEAT,
    MIRRORED_REPEAT,
}
