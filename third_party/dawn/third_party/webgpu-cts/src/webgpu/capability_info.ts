// MAINTENANCE_TODO: The generated Typedoc for this file is hard to navigate because it's
// alphabetized. Consider using namespaces or renames to fix this?

/* eslint-disable no-sparse-arrays */

import {
  keysOf,
  makeTable,
  makeTableRenameAndFilter,
  numericKeysOf,
  valueof,
} from '../common/util/data_tables.js';
import { assertTypeTrue, TypeEqual } from '../common/util/types.js';
import { unreachable } from '../common/util/util.js';

import { GPUConst, kMaxUnsignedLongValue, kMaxUnsignedLongLongValue } from './constants.js';

// Base device limits can be found in constants.ts.

// Queries

/** Maximum number of queries in GPUQuerySet, by spec. */
export const kMaxQueryCount = 4096;
/** Per-GPUQueryType info. */
export type QueryTypeInfo = {
  /** Optional feature required to use this GPUQueryType. */
  readonly feature: GPUFeatureName | undefined;
  // Add fields as needed
};
export const kQueryTypeInfo: {
  readonly [k in GPUQueryType]: QueryTypeInfo;
} =
  /* prettier-ignore */ {
  'occlusion': { feature:  undefined },
  'timestamp': { feature: 'timestamp-query' },
};
/** List of all GPUQueryType values. */
export const kQueryTypes = keysOf(kQueryTypeInfo);

// Buffers

/** Required alignment of a GPUBuffer size, by spec. */
export const kBufferSizeAlignment = 4;

/** Per-GPUBufferUsage copy info. */
export const kBufferUsageCopyInfo: {
  readonly [name: string]: GPUBufferUsageFlags;
} =
  /* prettier-ignore */ {
  'COPY_NONE':    0,
  'COPY_SRC':     GPUConst.BufferUsage.COPY_SRC,
  'COPY_DST':     GPUConst.BufferUsage.COPY_DST,
  'COPY_SRC_DST': GPUConst.BufferUsage.COPY_SRC | GPUConst.BufferUsage.COPY_DST,
};
/** List of all GPUBufferUsage copy values. */
export const kBufferUsageCopy = keysOf(kBufferUsageCopyInfo);

/** Per-GPUBufferUsage keys and info. */
type BufferUsageKey = keyof typeof GPUConst.BufferUsage;
export const kBufferUsageKeys = keysOf(GPUConst.BufferUsage);
export const kBufferUsageInfo: {
  readonly [k in BufferUsageKey]: GPUBufferUsageFlags;
} = {
  ...GPUConst.BufferUsage,
};

/** List of all GPUBufferUsage values. */
export const kBufferUsages = Object.values(GPUConst.BufferUsage);
export const kAllBufferUsageBits = kBufferUsages.reduce(
  (previousSet, currentUsage) => previousSet | currentUsage,
  0
);

// Errors

/** Per-GPUErrorFilter info. */
export const kErrorScopeFilterInfo: {
  readonly [k in GPUErrorFilter]: {
    generatable: boolean;
  };
} =
  /* prettier-ignore */ {
  'internal':      { generatable: false },
  'out-of-memory': { generatable: true },
  'validation':    { generatable: true },
};
/** List of all GPUErrorFilter values. */
export const kErrorScopeFilters = keysOf(kErrorScopeFilterInfo);
export const kGeneratableErrorScopeFilters = kErrorScopeFilters.filter(
  e => kErrorScopeFilterInfo[e].generatable
);

// Canvases

// The formats of GPUTextureFormat for canvas context.
export const kCanvasTextureFormats = ['bgra8unorm', 'rgba8unorm', 'rgba16float'] as const;

// The alpha mode for canvas context.
export const kCanvasAlphaModesInfo: {
  readonly [k in GPUCanvasAlphaMode]: {};
} = /* prettier-ignore */ {
  'opaque':        {},
  'premultiplied': {},
};
export const kCanvasAlphaModes = keysOf(kCanvasAlphaModesInfo);

// The color spaces for canvas context
export const kCanvasColorSpacesInfo: {
  readonly [k in PredefinedColorSpace]: {};
} = /* prettier-ignore */ {
  'srgb':       {},
  'display-p3': {},
};
export const kCanvasColorSpaces = keysOf(kCanvasColorSpacesInfo);

// Textures (except for texture format info)

/** Per-GPUTextureDimension info. */
export const kTextureDimensionInfo: {
  readonly [k in GPUTextureDimension]: {};
} = /* prettier-ignore */ {
  '1d': {},
  '2d': {},
  '3d': {},
};
/** List of all GPUTextureDimension values. */
export const kTextureDimensions = keysOf(kTextureDimensionInfo);

/** Per-GPUTextureAspect info. */
export const kTextureAspectInfo: {
  readonly [k in GPUTextureAspect]: {};
} = /* prettier-ignore */ {
  'all':          {},
  'depth-only':   {},
  'stencil-only': {},
};
/** List of all GPUTextureAspect values. */
export const kTextureAspects = keysOf(kTextureAspectInfo);

// Misc

/** Per-GPUCompareFunction info. */
export const kCompareFunctionInfo: {
  readonly [k in GPUCompareFunction]: {};
} =
  /* prettier-ignore */ {
  'never':         {},
  'less':          {},
  'equal':         {},
  'less-equal':    {},
  'greater':       {},
  'not-equal':     {},
  'greater-equal': {},
  'always':        {},
};
/** List of all GPUCompareFunction values. */
export const kCompareFunctions = keysOf(kCompareFunctionInfo);

/** Per-GPUStencilOperation info. */
export const kStencilOperationInfo: {
  readonly [k in GPUStencilOperation]: {};
} =
  /* prettier-ignore */ {
  'keep':            {},
  'zero':            {},
  'replace':         {},
  'invert':          {},
  'increment-clamp': {},
  'decrement-clamp': {},
  'increment-wrap':  {},
  'decrement-wrap':  {},
};
/** List of all GPUStencilOperation values. */
export const kStencilOperations = keysOf(kStencilOperationInfo);

// More textures (except for texture format info)

/** Per-GPUTextureUsage type info. */
export const kTextureUsageTypeInfo: {
  readonly [name: string]: number;
} =
  /* prettier-ignore */ {
  'texture': Number(GPUConst.TextureUsage.TEXTURE_BINDING),
  'storage': Number(GPUConst.TextureUsage.STORAGE_BINDING),
  'render':  Number(GPUConst.TextureUsage.RENDER_ATTACHMENT),
};
/** List of all GPUTextureUsage type values. */
export const kTextureUsageType = keysOf(kTextureUsageTypeInfo);

/** Per-GPUTextureUsage copy info. */
export const kTextureUsageCopyInfo: {
  readonly [name: string]: number;
} =
  /* prettier-ignore */ {
  'none':     0,
  'src':      Number(GPUConst.TextureUsage.COPY_SRC),
  'dst':      Number(GPUConst.TextureUsage.COPY_DST),
  'src-dest': Number(GPUConst.TextureUsage.COPY_SRC) | Number(GPUConst.TextureUsage.COPY_DST),
};
/** List of all GPUTextureUsage copy values. */
export const kTextureUsageCopy = keysOf(kTextureUsageCopyInfo);

/** Per-GPUTextureUsage info. */
export const kTextureUsageInfo: {
  readonly [k in valueof<typeof GPUConst.TextureUsage>]: {};
} = {
  [GPUConst.TextureUsage.COPY_SRC]: {},
  [GPUConst.TextureUsage.COPY_DST]: {},
  [GPUConst.TextureUsage.TEXTURE_BINDING]: {},
  [GPUConst.TextureUsage.STORAGE_BINDING]: {},
  [GPUConst.TextureUsage.RENDER_ATTACHMENT]: {},
};
/** List of all GPUTextureUsage values. */
export const kTextureUsages = numericKeysOf<GPUTextureUsageFlags>(kTextureUsageInfo);

// Texture View

/** Per-GPUTextureViewDimension info. */
export type TextureViewDimensionInfo = {
  /** Whether a storage texture view can have this view dimension. */
  readonly storage: boolean;
  // Add fields as needed
};
/** Per-GPUTextureViewDimension info. */
export const kTextureViewDimensionInfo: {
  readonly [k in GPUTextureViewDimension]: TextureViewDimensionInfo;
} =
  /* prettier-ignore */ {
  '1d':         { storage: true  },
  '2d':         { storage: true  },
  '2d-array':   { storage: true  },
  'cube':       { storage: false },
  'cube-array': { storage: false },
  '3d':         { storage: true  },
};
/** List of all GPUTextureDimension values. */
export const kTextureViewDimensions = keysOf(kTextureViewDimensionInfo);

// Vertex formats

/** Per-GPUVertexFormat info. */
// Exists just for documentation. Otherwise could be inferred by `makeTable`.
export type VertexFormatInfo = {
  /** Number of bytes in each component. */
  readonly bytesPerComponent: 1 | 2 | 4 | 'packed';
  /** The data encoding (float, normalized, or integer) for each component. */
  readonly type: 'float' | 'unorm' | 'snorm' | 'uint' | 'sint';
  /** Number of components. */
  readonly componentCount: 1 | 2 | 3 | 4;
  /** Size in bytes. */
  readonly byteSize: 1 | 2 | 4 | 8 | 12 | 16;
  /** The completely matching WGSL type for vertex format */
  readonly wgslType:
    | 'f32'
    | 'vec2<f32>'
    | 'vec3<f32>'
    | 'vec4<f32>'
    | 'u32'
    | 'vec2<u32>'
    | 'vec3<u32>'
    | 'vec4<u32>'
    | 'i32'
    | 'vec2<i32>'
    | 'vec3<i32>'
    | 'vec4<i32>';
  // Add fields as needed
};
/** Per-GPUVertexFormat info. */
export const kVertexFormatInfo: {
  readonly [k in GPUVertexFormat]: VertexFormatInfo;
} =
  /* prettier-ignore */ makeTable(
                     ['bytesPerComponent',   'type', 'componentCount', 'byteSize',  'wgslType'] as const,
                     [                   ,         ,                 ,           ,            ] as const, {
  // 8 bit components
  'uint8':           [                  1,   'uint',                1,          1,       'u32'],
  'uint8x2':         [                  1,   'uint',                2,          2, 'vec2<u32>'],
  'uint8x4':         [                  1,   'uint',                4,          4, 'vec4<u32>'],
  'sint8':           [                  1,   'sint',                1,          1,       'i32'],
  'sint8x2':         [                  1,   'sint',                2,          2, 'vec2<i32>'],
  'sint8x4':         [                  1,   'sint',                4,          4, 'vec4<i32>'],
  'unorm8':          [                  1,  'unorm',                1,          1,       'f32'],
  'unorm8x2':        [                  1,  'unorm',                2,          2, 'vec2<f32>'],
  'unorm8x4':        [                  1,  'unorm',                4,          4, 'vec4<f32>'],
  'snorm8':          [                  1,  'snorm',                1,          1,       'f32'],
  'snorm8x2':        [                  1,  'snorm',                2,          2, 'vec2<f32>'],
  'snorm8x4':        [                  1,  'snorm',                4,          4, 'vec4<f32>'],
  // 16 bit components
  'uint16':          [                  2,   'uint',                1,          2,       'u32'],
  'uint16x2':        [                  2,   'uint',                2,          4, 'vec2<u32>'],
  'uint16x4':        [                  2,   'uint',                4,          8, 'vec4<u32>'],
  'sint16':          [                  2,   'sint',                1,          2,       'i32'],
  'sint16x2':        [                  2,   'sint',                2,          4, 'vec2<i32>'],
  'sint16x4':        [                  2,   'sint',                4,          8, 'vec4<i32>'],
  'unorm16':         [                  2,  'unorm',                1,          2,       'f32'],
  'unorm16x2':       [                  2,  'unorm',                2,          4, 'vec2<f32>'],
  'unorm16x4':       [                  2,  'unorm',                4,          8, 'vec4<f32>'],
  'snorm16':         [                  2,  'snorm',                1,          2,       'f32'],
  'snorm16x2':       [                  2,  'snorm',                2,          4, 'vec2<f32>'],
  'snorm16x4':       [                  2,  'snorm',                4,          8, 'vec4<f32>'],
  'float16':         [                  2,  'float',                1,          2,      'f32'],
  'float16x2':       [                  2,  'float',                2,          4, 'vec2<f32>'],
  'float16x4':       [                  2,  'float',                4,          8, 'vec4<f32>'],
  // 32 bit components
  'float32':         [                  4,  'float',                1,          4,       'f32'],
  'float32x2':       [                  4,  'float',                2,          8, 'vec2<f32>'],
  'float32x3':       [                  4,  'float',                3,         12, 'vec3<f32>'],
  'float32x4':       [                  4,  'float',                4,         16, 'vec4<f32>'],
  'uint32':          [                  4,   'uint',                1,          4,       'u32'],
  'uint32x2':        [                  4,   'uint',                2,          8, 'vec2<u32>'],
  'uint32x3':        [                  4,   'uint',                3,         12, 'vec3<u32>'],
  'uint32x4':        [                  4,   'uint',                4,         16, 'vec4<u32>'],
  'sint32':          [                  4,   'sint',                1,          4,       'i32'],
  'sint32x2':        [                  4,   'sint',                2,          8, 'vec2<i32>'],
  'sint32x3':        [                  4,   'sint',                3,         12, 'vec3<i32>'],
  'sint32x4':        [                  4,   'sint',                4,         16, 'vec4<i32>'],
  // 32 bit packed
  'unorm10-10-10-2': [           'packed',  'unorm',                4,          4, 'vec4<f32>'],
  'unorm8x4-bgra':   [           'packed',  'unorm',                4,          4, 'vec4<f32>'],
} as const);
/** List of all GPUVertexFormat values. */
export const kVertexFormats = keysOf(kVertexFormatInfo);

// Typedefs for bindings

/**
 * Classes of `PerShaderStage` binding limits. Two bindings with the same class
 * count toward the same `PerShaderStage` limit(s) in the spec (if any).
 */
export type PerStageBindingLimitClass =
  | 'uniformBuf'
  | 'storageBuf'
  | 'sampler'
  | 'sampledTex'
  | 'readonlyStorageTex'
  | 'writeonlyStorageTex'
  | 'readwriteStorageTex';
/**
 * Classes of `PerPipelineLayout` binding limits. Two bindings with the same class
 * count toward the same `PerPipelineLayout` limit(s) in the spec (if any).
 */
export type PerPipelineBindingLimitClass = PerStageBindingLimitClass;

export type ValidBindableResource =
  | 'uniformBuf'
  | 'storageBuf'
  | 'filtSamp'
  | 'nonFiltSamp'
  | 'compareSamp'
  | 'sampledTex'
  | 'sampledTexMS'
  | 'readonlyStorageTex'
  | 'writeonlyStorageTex'
  | 'readwriteStorageTex';
type ErrorBindableResource = 'errorBuf' | 'errorSamp' | 'errorTex';

/**
 * Types of resource binding which have distinct binding rules, by spec
 * (e.g. filtering vs non-filtering sampler, multisample vs non-multisample texture).
 */
export type BindableResource = ValidBindableResource | ErrorBindableResource;
export const kBindableResources = [
  'uniformBuf',
  'storageBuf',
  'filtSamp',
  'nonFiltSamp',
  'compareSamp',
  'sampledTex',
  'sampledTexMS',
  'readonlyStorageTex',
  'writeonlyStorageTex',
  'readwriteStorageTex',
  'errorBuf',
  'errorSamp',
  'errorTex',
] as const;
assertTypeTrue<TypeEqual<BindableResource, (typeof kBindableResources)[number]>>();

// Bindings

/** Dynamic buffer offsets require offset to be divisible by 256, by spec. */
export const kMinDynamicBufferOffsetAlignment = 256;

/** Default `PerShaderStage` binding limits, by spec. */
export const kPerStageBindingLimits: {
  readonly [k in PerStageBindingLimitClass]: {
    /** Which `PerShaderStage` binding limit class. */
    readonly class: k;
    /** Maximum number of allowed bindings in that class. */
    readonly maxLimits: { [key in ShaderStageKey]: (typeof kLimits)[number] };
    // Add fields as needed
  };
} =
  /* prettier-ignore */ {
  'uniformBuf':          { class: 'uniformBuf', maxLimits: { COMPUTE: 'maxUniformBuffersPerShaderStage', FRAGMENT: 'maxUniformBuffersPerShaderStage', VERTEX: 'maxUniformBuffersPerShaderStage' } },
  'storageBuf':          { class: 'storageBuf', maxLimits: { COMPUTE: 'maxStorageBuffersPerShaderStage', FRAGMENT: 'maxStorageBuffersInFragmentStage', VERTEX: 'maxStorageBuffersInVertexStage' } },
  'sampler':             { class: 'sampler',    maxLimits: { COMPUTE: 'maxSamplersPerShaderStage', FRAGMENT: 'maxSamplersPerShaderStage', VERTEX: 'maxSamplersPerShaderStage' } },
  'sampledTex':          { class: 'sampledTex', maxLimits: { COMPUTE: 'maxSampledTexturesPerShaderStage', FRAGMENT: 'maxSampledTexturesPerShaderStage', VERTEX: 'maxSampledTexturesPerShaderStage' } },
  'readonlyStorageTex':  { class: 'readonlyStorageTex', maxLimits: { COMPUTE: 'maxStorageTexturesPerShaderStage', FRAGMENT: 'maxStorageTexturesInFragmentStage', VERTEX: 'maxStorageTexturesInVertexStage' } },
  'writeonlyStorageTex': { class: 'writeonlyStorageTex', maxLimits: { COMPUTE: 'maxStorageTexturesPerShaderStage', FRAGMENT: 'maxStorageTexturesInFragmentStage', VERTEX: 'maxStorageTexturesInVertexStage' } },
  'readwriteStorageTex': { class: 'readwriteStorageTex', maxLimits: { COMPUTE: 'maxStorageTexturesPerShaderStage', FRAGMENT: 'maxStorageTexturesInFragmentStage', VERTEX: 'maxStorageTexturesInVertexStage'} },
};

/**
 * Default `PerPipelineLayout` binding limits, by spec.
 */
export const kPerPipelineBindingLimits: {
  readonly [k in PerPipelineBindingLimitClass]: {
    /** Which `PerPipelineLayout` binding limit class. */
    readonly class: k;
    /**
     * The name of the limit for the maximum number of allowed bindings with `hasDynamicOffset: true` in that class.
     */
    readonly maxDynamicLimit: (typeof kLimits)[number] | '';
    // Add fields as needed
  };
} =
  /* prettier-ignore */ {
  'uniformBuf':          { class: 'uniformBuf', maxDynamicLimit: 'maxDynamicUniformBuffersPerPipelineLayout', },
  'storageBuf':          { class: 'storageBuf', maxDynamicLimit: 'maxDynamicStorageBuffersPerPipelineLayout', },
  'sampler':             { class: 'sampler',    maxDynamicLimit: '', },
  'sampledTex':          { class: 'sampledTex', maxDynamicLimit: '', },
  'readonlyStorageTex':  { class: 'readonlyStorageTex', maxDynamicLimit: '', },
  'writeonlyStorageTex': { class: 'writeonlyStorageTex', maxDynamicLimit: '', },
  'readwriteStorageTex': { class: 'readwriteStorageTex', maxDynamicLimit: '', },
};

interface BindingKindInfo {
  readonly resource: ValidBindableResource;
  readonly perStageLimitClass: (typeof kPerStageBindingLimits)[PerStageBindingLimitClass];
  readonly perPipelineLimitClass: (typeof kPerPipelineBindingLimits)[PerPipelineBindingLimitClass];
  // Add fields as needed
}

const kBindingKind: {
  readonly [k in ValidBindableResource]: BindingKindInfo;
} =
  /* prettier-ignore */ {
  uniformBuf:          { resource: 'uniformBuf',   perStageLimitClass: kPerStageBindingLimits.uniformBuf, perPipelineLimitClass: kPerPipelineBindingLimits.uniformBuf, },
  storageBuf:          { resource: 'storageBuf',   perStageLimitClass: kPerStageBindingLimits.storageBuf, perPipelineLimitClass: kPerPipelineBindingLimits.storageBuf, },
  filtSamp:            { resource: 'filtSamp',     perStageLimitClass: kPerStageBindingLimits.sampler,    perPipelineLimitClass: kPerPipelineBindingLimits.sampler,    },
  nonFiltSamp:         { resource: 'nonFiltSamp',  perStageLimitClass: kPerStageBindingLimits.sampler,    perPipelineLimitClass: kPerPipelineBindingLimits.sampler,    },
  compareSamp:         { resource: 'compareSamp',  perStageLimitClass: kPerStageBindingLimits.sampler,    perPipelineLimitClass: kPerPipelineBindingLimits.sampler,    },
  sampledTex:          { resource: 'sampledTex',   perStageLimitClass: kPerStageBindingLimits.sampledTex, perPipelineLimitClass: kPerPipelineBindingLimits.sampledTex, },
  sampledTexMS:        { resource: 'sampledTexMS', perStageLimitClass: kPerStageBindingLimits.sampledTex, perPipelineLimitClass: kPerPipelineBindingLimits.sampledTex, },
  readonlyStorageTex:  { resource: 'readonlyStorageTex',   perStageLimitClass: kPerStageBindingLimits.readonlyStorageTex, perPipelineLimitClass: kPerPipelineBindingLimits.readonlyStorageTex, },
  writeonlyStorageTex: { resource: 'writeonlyStorageTex',   perStageLimitClass: kPerStageBindingLimits.writeonlyStorageTex, perPipelineLimitClass: kPerPipelineBindingLimits.writeonlyStorageTex, },
  readwriteStorageTex: { resource: 'readwriteStorageTex',   perStageLimitClass: kPerStageBindingLimits.readwriteStorageTex, perPipelineLimitClass: kPerPipelineBindingLimits.readwriteStorageTex, },
};

// Binding type info

const kValidStagesAll = {
  validStages:
    GPUConst.ShaderStage.VERTEX | GPUConst.ShaderStage.FRAGMENT | GPUConst.ShaderStage.COMPUTE,
} as const;
const kValidStagesStorageWrite = {
  validStages: GPUConst.ShaderStage.FRAGMENT | GPUConst.ShaderStage.COMPUTE,
} as const;

/** Binding type info (including class limits) for the specified GPUBufferBindingLayout. */
export function bufferBindingTypeInfo(d: GPUBufferBindingLayout) {
  /* prettier-ignore */
  switch (d.type ?? 'uniform') {
    case 'uniform':           return { usage: GPUConst.BufferUsage.UNIFORM, ...kBindingKind.uniformBuf,  ...kValidStagesAll,          };
    case 'storage':           return { usage: GPUConst.BufferUsage.STORAGE, ...kBindingKind.storageBuf,  ...kValidStagesStorageWrite, };
    case 'read-only-storage': return { usage: GPUConst.BufferUsage.STORAGE, ...kBindingKind.storageBuf,  ...kValidStagesAll,          };
  }
}
/** List of all GPUBufferBindingType values. */
export const kBufferBindingTypes = ['uniform', 'storage', 'read-only-storage'] as const;
assertTypeTrue<TypeEqual<GPUBufferBindingType, (typeof kBufferBindingTypes)[number]>>();

/** Binding type info (including class limits) for the specified GPUSamplerBindingLayout. */
export function samplerBindingTypeInfo(d: GPUSamplerBindingLayout) {
  /* prettier-ignore */
  switch (d.type ?? 'filtering') {
    case 'filtering':     return { ...kBindingKind.filtSamp,    ...kValidStagesAll, };
    case 'non-filtering': return { ...kBindingKind.nonFiltSamp, ...kValidStagesAll, };
    case 'comparison':    return { ...kBindingKind.compareSamp, ...kValidStagesAll, };
  }
}
/** List of all GPUSamplerBindingType values. */
export const kSamplerBindingTypes = ['filtering', 'non-filtering', 'comparison'] as const;
assertTypeTrue<TypeEqual<GPUSamplerBindingType, (typeof kSamplerBindingTypes)[number]>>();

/** Binding type info (including class limits) for the specified GPUTextureBindingLayout. */
export function sampledTextureBindingTypeInfo(d: GPUTextureBindingLayout) {
  /* prettier-ignore */
  if (d.multisampled) {
    return { usage: GPUConst.TextureUsage.TEXTURE_BINDING, ...kBindingKind.sampledTexMS, ...kValidStagesAll, };
  } else {
    return { usage: GPUConst.TextureUsage.TEXTURE_BINDING, ...kBindingKind.sampledTex,   ...kValidStagesAll, };
  }
}
/** List of all GPUTextureSampleType values. */
export const kTextureSampleTypes = [
  'float',
  'unfilterable-float',
  'depth',
  'sint',
  'uint',
] as const;
assertTypeTrue<TypeEqual<GPUTextureSampleType, (typeof kTextureSampleTypes)[number]>>();

/** Binding type info (including class limits) for the specified GPUStorageTextureAccess. */
export function storageTextureBindingTypeInfo(d: { access?: GPUStorageTextureAccess | undefined }) {
  switch (d.access) {
    case undefined:
    case 'write-only':
      return {
        wgslAccess: 'write',
        usage: GPUConst.TextureUsage.STORAGE_BINDING,
        ...kBindingKind.writeonlyStorageTex,
        ...kValidStagesStorageWrite,
      };
    case 'read-only':
      return {
        wgslAccess: 'read',
        usage: GPUConst.TextureUsage.STORAGE_BINDING,
        ...kBindingKind.readonlyStorageTex,
        ...kValidStagesAll,
      };
    case 'read-write':
      return {
        wgslAccess: 'read_write',
        usage: GPUConst.TextureUsage.STORAGE_BINDING,
        ...kBindingKind.readwriteStorageTex,
        ...kValidStagesStorageWrite,
      };
  }
}
/** List of all GPUStorageTextureAccess values. */
export const kStorageTextureAccessValues = ['read-only', 'read-write', 'write-only'] as const;
assertTypeTrue<TypeEqual<GPUStorageTextureAccess, (typeof kStorageTextureAccessValues)[number]>>();

/** GPUBindGroupLayoutEntry, but only the "union" fields, not the common fields. */
export type BGLEntry = Omit<GPUBindGroupLayoutEntry, 'binding' | 'visibility'>;
/** Binding type info (including class limits) for the specified BGLEntry. */
export function texBindingTypeInfo(e: BGLEntry) {
  if (e.texture !== undefined) return sampledTextureBindingTypeInfo(e.texture);
  if (e.storageTexture !== undefined) return storageTextureBindingTypeInfo(e.storageTexture);
  unreachable();
}
/** BindingTypeInfo (including class limits) for the specified BGLEntry. */
export function bindingTypeInfo(e: BGLEntry) {
  if (e.buffer !== undefined) return bufferBindingTypeInfo(e.buffer);
  if (e.texture !== undefined) return sampledTextureBindingTypeInfo(e.texture);
  if (e.sampler !== undefined) return samplerBindingTypeInfo(e.sampler);
  if (e.storageTexture !== undefined) return storageTextureBindingTypeInfo(e.storageTexture);
  unreachable('GPUBindGroupLayoutEntry has no BindingLayout');
}

/**
 * Generate a list of possible buffer-typed BGLEntry values.
 *
 * Note: Generates different `type` options, but not `hasDynamicOffset` options.
 */
export function bufferBindingEntries(includeUndefined: boolean): readonly BGLEntry[] {
  return [
    ...(includeUndefined ? [{ buffer: { type: undefined } }] : []),
    { buffer: { type: 'uniform' } },
    { buffer: { type: 'storage' } },
    { buffer: { type: 'read-only-storage' } },
  ] as const;
}
/** Generate a list of possible sampler-typed BGLEntry values. */
export function samplerBindingEntries(includeUndefined: boolean): readonly BGLEntry[] {
  return [
    ...(includeUndefined ? [{ sampler: { type: undefined } }] : []),
    { sampler: { type: 'comparison' } },
    { sampler: { type: 'filtering' } },
    { sampler: { type: 'non-filtering' } },
  ] as const;
}
/**
 * Generate a list of possible texture-typed BGLEntry values.
 *
 * Note: Generates different `multisampled` options, but not `sampleType` or `viewDimension` options.
 */
export function textureBindingEntries(includeUndefined: boolean): readonly BGLEntry[] {
  return [
    ...(includeUndefined
      ? [{ texture: { multisampled: undefined, sampleType: 'unfilterable-float' } } as const]
      : []),
    { texture: { multisampled: false, sampleType: 'unfilterable-float' } },
    { texture: { multisampled: true, sampleType: 'unfilterable-float' } },
  ] as const;
}
/**
 * Generate a list of possible storageTexture-typed BGLEntry values.
 *
 * Note: Generates different `access` options, but not `format` or `viewDimension` options.
 */
export function storageTextureBindingEntries(format: GPUTextureFormat): readonly BGLEntry[] {
  return [
    { storageTexture: { access: 'write-only', format } },
    { storageTexture: { access: 'read-only', format } },
    { storageTexture: { access: 'read-write', format } },
  ] as const;
}
/** Generate a list of possible texture-or-storageTexture-typed BGLEntry values. */
export function sampledAndStorageBindingEntries(
  includeUndefined: boolean,
  format: GPUTextureFormat = 'r32float'
): readonly BGLEntry[] {
  return [
    ...textureBindingEntries(includeUndefined),
    ...storageTextureBindingEntries(format),
  ] as const;
}
/**
 * Generate a list of possible BGLEntry values of every type, but not variants with different:
 * - buffer.hasDynamicOffset
 * - texture.sampleType
 * - texture.viewDimension
 * - storageTexture.viewDimension
 */
export function allBindingEntries(
  includeUndefined: boolean,
  format: GPUTextureFormat = 'r32float'
): readonly BGLEntry[] {
  return [
    ...bufferBindingEntries(includeUndefined),
    ...samplerBindingEntries(includeUndefined),
    ...sampledAndStorageBindingEntries(includeUndefined, format),
  ] as const;
}

// Shader stages

/** List of all GPUShaderStage values. */
export type ShaderStageKey = keyof typeof GPUConst.ShaderStage;
export const kShaderStageKeys = Object.keys(GPUConst.ShaderStage) as ShaderStageKey[];
export const kShaderStages: readonly GPUShaderStageFlags[] = [
  GPUConst.ShaderStage.VERTEX,
  GPUConst.ShaderStage.FRAGMENT,
  GPUConst.ShaderStage.COMPUTE,
];
/** List of all possible combinations of GPUShaderStage values. */
export const kShaderStageCombinations: readonly GPUShaderStageFlags[] = [0, 1, 2, 3, 4, 5, 6, 7];
export const kShaderStageCombinationsWithStage: readonly GPUShaderStageFlags[] = [
  1, 2, 3, 4, 5, 6, 7,
];

/**
 * List of all possible texture sampleCount values.
 *
 * MAINTENANCE_TODO: Switch existing tests to use kTextureSampleCounts
 */
export const kTextureSampleCounts = [1, 4] as const;

// Sampler info

/** List of all mipmap filter modes. */
export const kMipmapFilterModes: readonly GPUMipmapFilterMode[] = ['nearest', 'linear'];
assertTypeTrue<TypeEqual<GPUMipmapFilterMode, (typeof kMipmapFilterModes)[number]>>();

/** List of address modes. */
export const kAddressModes: readonly GPUAddressMode[] = [
  'clamp-to-edge',
  'repeat',
  'mirror-repeat',
];
assertTypeTrue<TypeEqual<GPUAddressMode, (typeof kAddressModes)[number]>>();

// Blend factors and Blend components

/** List of all GPUBlendFactor values. */
export const kBlendFactors: readonly GPUBlendFactor[] = [
  'zero',
  'one',
  'src',
  'one-minus-src',
  'src-alpha',
  'one-minus-src-alpha',
  'dst',
  'one-minus-dst',
  'dst-alpha',
  'one-minus-dst-alpha',
  'src-alpha-saturated',
  'constant',
  'one-minus-constant',
  'src1',
  'one-minus-src1',
  'src1-alpha',
  'one-minus-src1-alpha',
];

/** Check if `blendFactor` belongs to the blend factors in the extension "dual-source-blending". */
export function IsDualSourceBlendingFactor(blendFactor?: GPUBlendFactor): boolean {
  switch (blendFactor) {
    case 'src1':
    case 'one-minus-src1':
    case 'src1-alpha':
    case 'one-minus-src1-alpha':
      return true;
    default:
      return false;
  }
}

/** List of all GPUBlendOperation values. */
export const kBlendOperations: readonly GPUBlendOperation[] = [
  'add',
  'subtract',
  'reverse-subtract',
  'min',
  'max',
];

// Primitive topologies
export const kPrimitiveTopology: readonly GPUPrimitiveTopology[] = [
  'point-list',
  'line-list',
  'line-strip',
  'triangle-list',
  'triangle-strip',
];
assertTypeTrue<TypeEqual<GPUPrimitiveTopology, (typeof kPrimitiveTopology)[number]>>();

export const kIndexFormat: readonly GPUIndexFormat[] = ['uint16', 'uint32'];
assertTypeTrue<TypeEqual<GPUIndexFormat, (typeof kIndexFormat)[number]>>();

/** Info for each entry of GPUSupportedLimits */
const [kLimitInfoKeys, kLimitInfoDefaults, kLimitInfoData] =
  /* prettier-ignore */ [
                                               [    'class',    'core', 'compatibility',            'maximumValue'] as const,
                                               [  'maximum',          ,                ,     kMaxUnsignedLongValue] as const, {
  'maxTextureDimension1D':                     [           ,      8192,            4096,                          ],
  'maxTextureDimension2D':                     [           ,      8192,            4096,                          ],
  'maxTextureDimension3D':                     [           ,      2048,            1024,                          ],
  'maxTextureArrayLayers':                     [           ,       256,             256,                          ],

  'maxBindGroups':                             [           ,         4,               4,                          ],
  'maxBindGroupsPlusVertexBuffers':            [           ,        24,              24,                          ],
  'maxBindingsPerBindGroup':                   [           ,      1000,            1000,                          ],
  'maxDynamicUniformBuffersPerPipelineLayout': [           ,         8,               8,                          ],
  'maxDynamicStorageBuffersPerPipelineLayout': [           ,         4,               4,                          ],
  'maxSampledTexturesPerShaderStage':          [           ,        16,              16,                          ],
  'maxSamplersPerShaderStage':                 [           ,        16,              16,                          ],
  'maxStorageBuffersInFragmentStage':          [           ,         8,               0,                          ],
  'maxStorageBuffersInVertexStage':            [           ,         8,               0,                          ],
  'maxStorageBuffersPerShaderStage':           [           ,         8,               4,                          ],
  'maxStorageTexturesInFragmentStage':         [           ,         4,               0,                          ],
  'maxStorageTexturesInVertexStage':           [           ,         4,               0,                          ],
  'maxStorageTexturesPerShaderStage':          [           ,         4,               4,                          ],
  'maxUniformBuffersPerShaderStage':           [           ,        12,              12,                          ],

  'maxUniformBufferBindingSize':               [           ,     65536,           16384, kMaxUnsignedLongLongValue],
  'maxStorageBufferBindingSize':               [           , 134217728,       134217728, kMaxUnsignedLongLongValue],
  'minUniformBufferOffsetAlignment':           ['alignment',       256,             256,                          ],
  'minStorageBufferOffsetAlignment':           ['alignment',       256,             256,                          ],

  'maxVertexBuffers':                          [           ,         8,               8,                          ],
  'maxBufferSize':                             [           , 268435456,       268435456, kMaxUnsignedLongLongValue],
  'maxVertexAttributes':                       [           ,        16,              16,                          ],
  'maxVertexBufferArrayStride':                [           ,      2048,            2048,                          ],
  'maxInterStageShaderVariables':              [           ,        16,              15,                          ],

  'maxColorAttachments':                       [           ,         8,               4,                          ],
  'maxColorAttachmentBytesPerSample':          [           ,        32,              32,                          ],

  'maxComputeWorkgroupStorageSize':            [           ,     16384,           16384,                          ],
  'maxComputeInvocationsPerWorkgroup':         [           ,       256,             128,                          ],
  'maxComputeWorkgroupSizeX':                  [           ,       256,             128,                          ],
  'maxComputeWorkgroupSizeY':                  [           ,       256,             128,                          ],
  'maxComputeWorkgroupSizeZ':                  [           ,        64,              64,                          ],
  'maxComputeWorkgroupsPerDimension':          [           ,     65535,           65535,                          ],
} as const];

/**
 * Feature levels corresponding to core WebGPU and WebGPU
 * in compatibility mode. They can be passed to
 * getDefaultLimits though if you have access to an adapter
 * it's preferred to use getDefaultLimitsForAdapter.
 */
export const kFeatureLevels = ['core', 'compatibility'] as const;
export type FeatureLevel = (typeof kFeatureLevels)[number];

const kLimitKeys = ['class', 'default', 'maximumValue'] as const;

const kLimitInfoCore = makeTableRenameAndFilter(
  { default: 'core' },
  kLimitKeys,
  kLimitInfoKeys,
  kLimitInfoDefaults,
  kLimitInfoData
);

const kLimitInfoCompatibility = makeTableRenameAndFilter(
  { default: 'compatibility' },
  kLimitKeys,
  kLimitInfoKeys,
  kLimitInfoDefaults,
  kLimitInfoData
);

const kLimitInfos = {
  core: kLimitInfoCore,
  compatibility: kLimitInfoCompatibility,
} as const;

export const kLimitClasses = Object.fromEntries(
  Object.entries(kLimitInfoCore).map(([k, { class: c }]) => [k, c])
);

export function getDefaultLimits(featureLevel: FeatureLevel) {
  return kLimitInfos[featureLevel];
}

export function getDefaultLimitsForAdapter(adapter: GPUAdapter) {
  // MAINTENANCE_TODO: Remove casts once we have a standardized way to do this
  // (see https://github.com/gpuweb/gpuweb/pull/5037#issuecomment-2576110161).
  const adapterExtensions = adapter as unknown as {
    isCompatibilityMode?: boolean;
    featureLevel?: string;
  };
  const featureLevel =
    adapterExtensions.featureLevel === 'compatibility' || adapterExtensions.isCompatibilityMode
      ? 'compatibility'
      : 'core';
  return getDefaultLimits(featureLevel);
}

const kEachStage = [
  GPUConst.ShaderStage.COMPUTE,
  GPUConst.ShaderStage.FRAGMENT,
  GPUConst.ShaderStage.VERTEX,
];
function shaderStageFlagToStageName(stage: GPUShaderStageFlags) {
  switch (stage) {
    case GPUConst.ShaderStage.COMPUTE:
      return 'COMPUTE';
    case GPUConst.ShaderStage.FRAGMENT:
      return 'FRAGMENT';
    case GPUConst.ShaderStage.VERTEX:
      return 'VERTEX';
    default:
      unreachable();
  }
}

/**
 * Get the limit of the number of things you can bind for
 * a given BGLEntry given the specified visibility. This is
 * the minimum across stages for the given visibility.
 */
export function getBindingLimitForBindingType(
  device: GPUDevice,
  visibility: GPUShaderStageFlags,
  e: BGLEntry
) {
  const info = bindingTypeInfo(e);
  const maxLimits = info.perStageLimitClass.maxLimits;
  const limits = kEachStage
    .filter(stage => stage & visibility)
    .map(stage => device.limits[maxLimits[shaderStageFlagToStageName(stage)]]!);
  return limits.length > 0 ? Math.min(...limits) : 0;
}

/** List of all entries of GPUSupportedLimits. */
export const kLimits = keysOf(kLimitInfoCore);

/**
 * The number of color attachments to test.
 * The CTS needs to generate a consistent list of tests.
 * We can't use any default limits since they different from core to compat mode
 * So, tests should use this value and filter out any values that are out of
 * range for the current device.
 *
 * The test in maxColorAttachments.spec.ts tests that kMaxColorAttachmentsToTest
 * is large enough to cover all devices tested.
 */
export const kMaxColorAttachmentsToTest = 32;

/** The size of indirect draw parameters in the indirectBuffer of drawIndirect */
export const kDrawIndirectParametersSize = 4;
/** The size of indirect drawIndexed parameters in the indirectBuffer of drawIndexedIndirect */
export const kDrawIndexedIndirectParametersSize = 5;

/** Per-GPUFeatureName info. */
export const kFeatureNameInfo: {
  readonly [k in GPUFeatureName]: {};
} =
  /* prettier-ignore */ {
  'bgra8unorm-storage':                 {},
  'depth-clip-control':                 {},
  'depth32float-stencil8':              {},
  'texture-compression-bc':             {},
  'texture-compression-bc-sliced-3d':   {},
  'texture-compression-etc2':           {},
  'texture-compression-astc':           {},
  'texture-compression-astc-sliced-3d': {},
  'timestamp-query':                    {},
  'indirect-first-instance':            {},
  'shader-f16':                         {},
  'rg11b10ufloat-renderable':           {},
  'float32-filterable':                 {},
  'float32-blendable':                  {},
  'clip-distances':                     {},
  'dual-source-blending':               {},
};
/** List of all GPUFeatureName values. */
export const kFeatureNames = keysOf(kFeatureNameInfo);

/** List of all known WGSL language features */
export const kKnownWGSLLanguageFeatures = [
  'readonly_and_readwrite_storage_textures',
  'packed_4x8_integer_dot_product',
  'unrestricted_pointer_parameters',
  'pointer_composite_access',
] as const;

export type WGSLLanguageFeature = (typeof kKnownWGSLLanguageFeatures)[number];
