import { keysOf } from '../common/util/data_tables.js';
import { assert, unreachable } from '../common/util/util.js';

import { align } from './util/math.js';
import { ImageCopyType } from './util/texture/layout.js';

//
// Texture format tables
//

/**
 * Defaults applied to all texture format tables automatically. Used only inside
 * `formatTableWithDefaults`. This ensures keys are never missing, always explicitly `undefined`.
 *
 * All top-level keys must be defined here, or they won't be exposed at all.
 * Documentation is also written here; this makes it propagate through to the end types.
 */
const kFormatUniversalDefaults = {
  /** Texel block width. */
  blockWidth: undefined,
  /** Texel block height. */
  blockHeight: undefined,
  color: undefined,
  depth: undefined,
  stencil: undefined,
  colorRender: undefined,
  /** Whether the format can be used in a multisample texture. */
  multisample: undefined,
  /** Optional feature required to use this format, or `undefined` if none. */
  feature: undefined,
  /** The base format for srgb formats. Specified on both srgb and equivalent non-srgb formats. */
  baseFormat: undefined,

  /** @deprecated Use `.color.bytes`, `.depth.bytes`, or `.stencil.bytes`. */
  bytesPerBlock: undefined,

  // IMPORTANT:
  // Add new top-level keys both here and in TextureFormatInfo_TypeCheck.
} as const;
/**
 * Takes `table` and applies `defaults` to every row, i.e. for each row,
 * `{ ... kUniversalDefaults, ...defaults, ...row }`.
 * This only operates at the first level; it doesn't support defaults in nested objects.
 */
function formatTableWithDefaults<Defaults extends {}, Table extends { readonly [K: string]: {} }>({
  defaults,
  table,
}: {
  defaults: Defaults;
  table: Table;
}): {
  readonly [F in keyof Table]: {
    readonly [K in keyof typeof kFormatUniversalDefaults]: K extends keyof Table[F]
      ? Table[F][K]
      : K extends keyof Defaults
      ? Defaults[K]
      : (typeof kFormatUniversalDefaults)[K];
  };
} {
  return Object.fromEntries(
    Object.entries(table).map(([k, row]) => [
      k,
      { ...kFormatUniversalDefaults, ...defaults, ...row },
    ])
    /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
  ) as any;
}

/** "plain color formats", plus rgb9e5ufloat. */
const kRegularTextureFormatInfo = formatTableWithDefaults({
  defaults: { blockWidth: 1, blockHeight: 1 },
  table: {
    // plain, 8 bits per component

    r8unorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      colorRender: { blend: true, resolve: true, byteCost: 1, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r8snorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r8uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      colorRender: { blend: false, resolve: false, byteCost: 1, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r8sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      colorRender: { blend: false, resolve: false, byteCost: 1, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rg8unorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: true, resolve: true, byteCost: 2, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg8snorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg8uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: false, resolve: false, byteCost: 2, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg8sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: false, resolve: false, byteCost: 2, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rgba8unorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 1 },
      multisample: true,
      baseFormat: 'rgba8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'rgba8unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 1 },
      multisample: true,
      baseFormat: 'rgba8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba8snorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 4,
      },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba8uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba8sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 1 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    bgra8unorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 1 },
      multisample: true,
      baseFormat: 'bgra8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bgra8unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 1 },
      multisample: true,
      baseFormat: 'bgra8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    // plain, 16 bits per component

    r16uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: false, resolve: false, byteCost: 2, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r16sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: false, resolve: false, byteCost: 2, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r16float: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      colorRender: { blend: true, resolve: true, byteCost: 2, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rg16uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg16sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg16float: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 4, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rgba16uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba16sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba16float: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 2 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    // plain, 32 bits per component

    r32uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: true,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r32sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: true,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    r32float: {
      color: {
        type: 'unfilterable-float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: true,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 4, alignment: 4 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rg32uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg32sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg32float: {
      color: {
        type: 'unfilterable-float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 8,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    rgba32uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 16,
      },
      colorRender: { blend: false, resolve: false, byteCost: 16, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba32sint: {
      color: {
        type: 'sint',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 16,
      },
      colorRender: { blend: false, resolve: false, byteCost: 16, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgba32float: {
      color: {
        type: 'unfilterable-float',
        copySrc: true,
        copyDst: true,
        storage: true,
        readWriteStorage: false,
        bytes: 16,
      },
      colorRender: { blend: false, resolve: false, byteCost: 16, alignment: 4 },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    // plain, mixed component width, 32 bits per texel

    rgb10a2uint: {
      color: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: false, resolve: false, byteCost: 8, alignment: 4 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rgb10a2unorm: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      colorRender: { blend: true, resolve: true, byteCost: 8, alignment: 4 },
      multisample: true,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    rg11b10ufloat: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    // packed

    rgb9e5ufloat: {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      multisample: false,
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
  },
} as const);

// MAINTENANCE_TODO: Distinguishing "sized" and "unsized" depth stencil formats doesn't make sense
// because one aspect can be sized and one can be unsized. This should be cleaned up, but is kept
// this way during a migration phase.
const kSizedDepthStencilFormatInfo = formatTableWithDefaults({
  defaults: { blockWidth: 1, blockHeight: 1, multisample: true },
  table: {
    stencil8: {
      stencil: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      bytesPerBlock: 1,
    },
    depth16unorm: {
      depth: {
        type: 'depth',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 2,
      },
      bytesPerBlock: 2,
    },
    depth32float: {
      depth: {
        type: 'depth',
        copySrc: true,
        copyDst: false,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      bytesPerBlock: 4,
    },
  },
} as const);
const kUnsizedDepthStencilFormatInfo = formatTableWithDefaults({
  defaults: { blockWidth: 1, blockHeight: 1, multisample: true },
  table: {
    depth24plus: {
      depth: {
        type: 'depth',
        copySrc: false,
        copyDst: false,
        storage: false,
        readWriteStorage: false,
        bytes: undefined,
      },
    },
    'depth24plus-stencil8': {
      depth: {
        type: 'depth',
        copySrc: false,
        copyDst: false,
        storage: false,
        readWriteStorage: false,
        bytes: undefined,
      },
      stencil: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
    },
    'depth32float-stencil8': {
      depth: {
        type: 'depth',
        copySrc: true,
        copyDst: false,
        storage: false,
        readWriteStorage: false,
        bytes: 4,
      },
      stencil: {
        type: 'uint',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 1,
      },
      feature: 'depth32float-stencil8',
    },
  },
} as const);

const kBCTextureFormatInfo = formatTableWithDefaults({
  defaults: {
    blockWidth: 4,
    blockHeight: 4,
    multisample: false,
    feature: 'texture-compression-bc',
  },
  table: {
    'bc1-rgba-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'bc1-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc1-rgba-unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'bc1-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc2-rgba-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc2-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc2-rgba-unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc2-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc3-rgba-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc3-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc3-rgba-unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc3-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc4-r-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc4-r-snorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc5-rg-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc5-rg-snorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc6h-rgb-ufloat': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc6h-rgb-float': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'bc7-rgba-unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc7-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'bc7-rgba-unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'bc7-rgba-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
  },
} as const);

const kETC2TextureFormatInfo = formatTableWithDefaults({
  defaults: {
    blockWidth: 4,
    blockHeight: 4,
    multisample: false,
    feature: 'texture-compression-etc2',
  },
  table: {
    'etc2-rgb8unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'etc2-rgb8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'etc2-rgb8unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'etc2-rgb8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'etc2-rgb8a1unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'etc2-rgb8a1unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'etc2-rgb8a1unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      baseFormat: 'etc2-rgb8a1unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'etc2-rgba8unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'etc2-rgba8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'etc2-rgba8unorm-srgb': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'etc2-rgba8unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'eac-r11unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'eac-r11snorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 8,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'eac-rg11unorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'eac-rg11snorm': {
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
  },
} as const);

const kASTCTextureFormatInfo = formatTableWithDefaults({
  defaults: {
    multisample: false,
    feature: 'texture-compression-astc',
  },
  table: {
    'astc-4x4-unorm': {
      blockWidth: 4,
      blockHeight: 4,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-4x4-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-4x4-unorm-srgb': {
      blockWidth: 4,
      blockHeight: 4,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-4x4-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-5x4-unorm': {
      blockWidth: 5,
      blockHeight: 4,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-5x4-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-5x4-unorm-srgb': {
      blockWidth: 5,
      blockHeight: 4,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-5x4-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-5x5-unorm': {
      blockWidth: 5,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-5x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-5x5-unorm-srgb': {
      blockWidth: 5,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-5x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-6x5-unorm': {
      blockWidth: 6,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-6x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-6x5-unorm-srgb': {
      blockWidth: 6,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-6x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-6x6-unorm': {
      blockWidth: 6,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-6x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-6x6-unorm-srgb': {
      blockWidth: 6,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-6x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-8x5-unorm': {
      blockWidth: 8,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-8x5-unorm-srgb': {
      blockWidth: 8,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-8x6-unorm': {
      blockWidth: 8,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-8x6-unorm-srgb': {
      blockWidth: 8,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-8x8-unorm': {
      blockWidth: 8,
      blockHeight: 8,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x8-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-8x8-unorm-srgb': {
      blockWidth: 8,
      blockHeight: 8,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-8x8-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-10x5-unorm': {
      blockWidth: 10,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-10x5-unorm-srgb': {
      blockWidth: 10,
      blockHeight: 5,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x5-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-10x6-unorm': {
      blockWidth: 10,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-10x6-unorm-srgb': {
      blockWidth: 10,
      blockHeight: 6,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x6-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-10x8-unorm': {
      blockWidth: 10,
      blockHeight: 8,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x8-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-10x8-unorm-srgb': {
      blockWidth: 10,
      blockHeight: 8,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x8-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-10x10-unorm': {
      blockWidth: 10,
      blockHeight: 10,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x10-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-10x10-unorm-srgb': {
      blockWidth: 10,
      blockHeight: 10,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-10x10-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-12x10-unorm': {
      blockWidth: 12,
      blockHeight: 10,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-12x10-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-12x10-unorm-srgb': {
      blockWidth: 12,
      blockHeight: 10,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-12x10-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },

    'astc-12x12-unorm': {
      blockWidth: 12,
      blockHeight: 12,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-12x12-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
    'astc-12x12-unorm-srgb': {
      blockWidth: 12,
      blockHeight: 12,
      color: {
        type: 'float',
        copySrc: true,
        copyDst: true,
        storage: false,
        readWriteStorage: false,
        bytes: 16,
      },
      baseFormat: 'astc-12x12-unorm',
      /*prettier-ignore*/ get bytesPerBlock() { return this.color.bytes; },
    },
  },
} as const);

// Definitions for use locally. To access the table entries, use `kTextureFormatInfo`.

// MAINTENANCE_TODO: Consider generating the exports below programmatically by filtering the big list, instead
// of using these local constants? Requires some type magic though.
/* prettier-ignore */ const   kCompressedTextureFormatInfo = { ...kBCTextureFormatInfo, ...kETC2TextureFormatInfo, ...kASTCTextureFormatInfo } as const;
/* prettier-ignore */ const        kColorTextureFormatInfo = { ...kRegularTextureFormatInfo, ...kCompressedTextureFormatInfo } as const;
/* prettier-ignore */ const    kEncodableTextureFormatInfo = { ...kRegularTextureFormatInfo, ...kSizedDepthStencilFormatInfo } as const;
/* prettier-ignore */ const        kSizedTextureFormatInfo = { ...kRegularTextureFormatInfo, ...kSizedDepthStencilFormatInfo, ...kCompressedTextureFormatInfo } as const;
/* prettier-ignore */ const        kDepthStencilFormatInfo = { ...kSizedDepthStencilFormatInfo, ...kUnsizedDepthStencilFormatInfo } as const;
/* prettier-ignore */ const kUncompressedTextureFormatInfo = { ...kRegularTextureFormatInfo, ...kSizedDepthStencilFormatInfo, ...kUnsizedDepthStencilFormatInfo } as const;
/* prettier-ignore */ const          kAllTextureFormatInfo = { ...kUncompressedTextureFormatInfo, ...kCompressedTextureFormatInfo } as const;

/** A "regular" texture format (uncompressed, sized, single-plane color formats). */
/* prettier-ignore */ export type      RegularTextureFormat = keyof typeof kRegularTextureFormatInfo;
/** A sized depth/stencil texture format. */
/* prettier-ignore */ export type   SizedDepthStencilFormat = keyof typeof kSizedDepthStencilFormatInfo;
/** An unsized depth/stencil texture format. */
/* prettier-ignore */ export type UnsizedDepthStencilFormat = keyof typeof kUnsizedDepthStencilFormatInfo;
/** A compressed (block) texture format. */
/* prettier-ignore */ export type   CompressedTextureFormat = keyof typeof kCompressedTextureFormatInfo;

/** A color texture format (regular | compressed). */
/* prettier-ignore */ export type        ColorTextureFormat = keyof typeof kColorTextureFormatInfo;
/** An encodable texture format (regular | sized depth/stencil). */
/* prettier-ignore */ export type    EncodableTextureFormat = keyof typeof kEncodableTextureFormatInfo;
/** A sized texture format (regular | sized depth/stencil | compressed). */
/* prettier-ignore */ export type        SizedTextureFormat = keyof typeof kSizedTextureFormatInfo;
/** A depth/stencil format (sized | unsized). */
/* prettier-ignore */ export type        DepthStencilFormat = keyof typeof kDepthStencilFormatInfo;
/** An uncompressed (block size 1x1) format (regular | depth/stencil). */
/* prettier-ignore */ export type UncompressedTextureFormat = keyof typeof kUncompressedTextureFormatInfo;

/* prettier-ignore */ export const      kRegularTextureFormats: readonly      RegularTextureFormat[] = keysOf(     kRegularTextureFormatInfo);
/* prettier-ignore */ export const   kSizedDepthStencilFormats: readonly   SizedDepthStencilFormat[] = keysOf(  kSizedDepthStencilFormatInfo);
/* prettier-ignore */ export const kUnsizedDepthStencilFormats: readonly UnsizedDepthStencilFormat[] = keysOf(kUnsizedDepthStencilFormatInfo);
/* prettier-ignore */ export const   kCompressedTextureFormats: readonly   CompressedTextureFormat[] = keysOf(  kCompressedTextureFormatInfo);

/* prettier-ignore */ export const        kColorTextureFormats: readonly        ColorTextureFormat[] = keysOf(       kColorTextureFormatInfo);
/* prettier-ignore */ export const    kEncodableTextureFormats: readonly    EncodableTextureFormat[] = keysOf(   kEncodableTextureFormatInfo);
/* prettier-ignore */ export const        kSizedTextureFormats: readonly        SizedTextureFormat[] = keysOf(       kSizedTextureFormatInfo);
/* prettier-ignore */ export const        kDepthStencilFormats: readonly        DepthStencilFormat[] = keysOf(       kDepthStencilFormatInfo);
/* prettier-ignore */ export const kUncompressedTextureFormats: readonly UncompressedTextureFormat[] = keysOf(kUncompressedTextureFormatInfo);
/* prettier-ignore */ export const          kAllTextureFormats: readonly          GPUTextureFormat[] = keysOf(         kAllTextureFormatInfo);

// CompressedTextureFormat are unrenderable so filter from RegularTextureFormats for color targets is enough
export const kRenderableColorTextureFormats = kRegularTextureFormats.filter(
  v => kColorTextureFormatInfo[v].colorRender
);

/** Per-GPUTextureFormat-per-aspect info. */
interface TextureFormatAspectInfo {
  /** Whether the aspect can be used as `COPY_SRC`. */
  copySrc: boolean;
  /** Whether the aspect can be used as `COPY_DST`. */
  copyDst: boolean;
  /** Whether the aspect can be used as `STORAGE`. */
  storage: boolean;
  /** Whether the aspect can be used as `STORAGE` with `read-write` storage texture access. */
  readWriteStorage: boolean;
  /** The "texel block copy footprint" of one texel block; `undefined` if the aspect is unsized. */
  bytes: number | undefined;
}
/** Per GPUTextureFormat-per-aspect info for color aspects. */
interface TextureFormatColorAspectInfo extends TextureFormatAspectInfo {
  bytes: number;
  /** "Best" sample type of the format. "float" also implies "unfilterable-float". */
  type: 'float' | 'uint' | 'sint' | 'unfilterable-float';
}
/** Per GPUTextureFormat-per-aspect info for depth aspects. */
interface TextureFormatDepthAspectInfo extends TextureFormatAspectInfo {
  /** "depth" also implies "unfilterable-float". */
  type: 'depth';
}
/** Per GPUTextureFormat-per-aspect info for stencil aspects. */
interface TextureFormatStencilAspectInfo extends TextureFormatAspectInfo {
  bytes: 1;
  type: 'uint';
}

/**
 * Per-GPUTextureFormat info.
 * This is not actually the type of values in kTextureFormatInfo; that type is fully const
 * so that it can be narrowed very precisely at usage sites by the compiler.
 * This type exists only as a type check on the inferred type of kTextureFormatInfo.
 */
type TextureFormatInfo_TypeCheck = {
  blockWidth: number;
  blockHeight: number;
  multisample: boolean;
  baseFormat: GPUTextureFormat | undefined;
  feature: GPUFeatureName | undefined;

  bytesPerBlock: number | undefined;

  // IMPORTANT:
  // Add new top-level keys both here and in kUniversalDefaults.
} & (
  | {
      /** Color aspect info. */
      color: TextureFormatColorAspectInfo;
      /** Defined if the format is a color format that can be used as `RENDER_ATTACHMENT`. */
      colorRender:
        | undefined
        | {
            /** Whether the format is blendable. */
            blend: boolean;
            /** Whether the format can be a multisample resolve target. */
            resolve: boolean;
            /** The "render target pixel byte cost" of the format. */
            byteCost: number;
            /** The "render target component alignment" of the format. */
            alignment: number;
          };
    }
  | (
      | {
          /** Depth aspect info. */
          depth: TextureFormatDepthAspectInfo;
          /** Stencil aspect info. */
          stencil: undefined | TextureFormatStencilAspectInfo;
          multisample: true;
        }
      | {
          /** Stencil aspect info. */
          stencil: TextureFormatStencilAspectInfo;
          multisample: true;
        }
    )
);

/** Per-GPUTextureFormat info. */
export const kTextureFormatInfo = {
  ...kRegularTextureFormatInfo,
  ...kSizedDepthStencilFormatInfo,
  ...kUnsizedDepthStencilFormatInfo,
  ...kBCTextureFormatInfo,
  ...kETC2TextureFormatInfo,
  ...kASTCTextureFormatInfo,
} as const;

/** Defining this variable verifies the type of kTextureFormatInfo2. It is not used. */
// eslint-disable-next-line @typescript-eslint/no-unused-vars
const kTextureFormatInfo_TypeCheck: {
  readonly [F in GPUTextureFormat]: TextureFormatInfo_TypeCheck;
} = kTextureFormatInfo;

/** Valid GPUTextureFormats for `copyExternalImageToTexture`, by spec. */
export const kValidTextureFormatsForCopyE2T = [
  'r8unorm',
  'r16float',
  'r32float',
  'rg8unorm',
  'rg16float',
  'rg32float',
  'rgba8unorm',
  'rgba8unorm-srgb',
  'bgra8unorm',
  'bgra8unorm-srgb',
  'rgb10a2unorm',
  'rgba16float',
  'rgba32float',
] as const;

//
// Other related stuff
//

const kDepthStencilFormatCapabilityInBufferTextureCopy = {
  // kUnsizedDepthStencilFormats
  depth24plus: {
    CopyB2T: [],
    CopyT2B: [],
    texelAspectSize: { 'depth-only': -1, 'stencil-only': -1 },
  },
  'depth24plus-stencil8': {
    CopyB2T: ['stencil-only'],
    CopyT2B: ['stencil-only'],
    texelAspectSize: { 'depth-only': -1, 'stencil-only': 1 },
  },

  // kSizedDepthStencilFormats
  depth16unorm: {
    CopyB2T: ['all', 'depth-only'],
    CopyT2B: ['all', 'depth-only'],
    texelAspectSize: { 'depth-only': 2, 'stencil-only': -1 },
  },
  depth32float: {
    CopyB2T: [],
    CopyT2B: ['all', 'depth-only'],
    texelAspectSize: { 'depth-only': 4, 'stencil-only': -1 },
  },
  'depth32float-stencil8': {
    CopyB2T: ['stencil-only'],
    CopyT2B: ['depth-only', 'stencil-only'],
    texelAspectSize: { 'depth-only': 4, 'stencil-only': 1 },
  },
  stencil8: {
    CopyB2T: ['all', 'stencil-only'],
    CopyT2B: ['all', 'stencil-only'],
    texelAspectSize: { 'depth-only': -1, 'stencil-only': 1 },
  },
} as const;

/** `kDepthStencilFormatResolvedAspect[format][aspect]` returns the aspect-specific format for a
 *  depth-stencil format, or `undefined` if the format doesn't have the aspect.
 */
export const kDepthStencilFormatResolvedAspect: {
  readonly [k in DepthStencilFormat]: {
    readonly [a in GPUTextureAspect]: DepthStencilFormat | undefined;
  };
} = {
  // kUnsizedDepthStencilFormats
  depth24plus: {
    all: 'depth24plus',
    'depth-only': 'depth24plus',
    'stencil-only': undefined,
  },
  'depth24plus-stencil8': {
    all: 'depth24plus-stencil8',
    'depth-only': 'depth24plus',
    'stencil-only': 'stencil8',
  },

  // kSizedDepthStencilFormats
  depth16unorm: {
    all: 'depth16unorm',
    'depth-only': 'depth16unorm',
    'stencil-only': undefined,
  },
  depth32float: {
    all: 'depth32float',
    'depth-only': 'depth32float',
    'stencil-only': undefined,
  },
  'depth32float-stencil8': {
    all: 'depth32float-stencil8',
    'depth-only': 'depth32float',
    'stencil-only': 'stencil8',
  },
  stencil8: {
    all: 'stencil8',
    'depth-only': undefined,
    'stencil-only': 'stencil8',
  },
} as const;

/**
 * @returns the GPUTextureFormat corresponding to the @param aspect of @param format.
 * This allows choosing the correct format for depth-stencil aspects when creating pipelines that
 * will have to match the resolved format of views, or to get per-aspect information like the
 * `blockByteSize`.
 *
 * Many helpers use an `undefined` `aspect` to means `'all'` so this is also the default for this
 * function.
 */
export function resolvePerAspectFormat(
  format: GPUTextureFormat,
  aspect?: GPUTextureAspect
): GPUTextureFormat {
  if (aspect === 'all' || aspect === undefined) {
    return format;
  }
  assert(!!kTextureFormatInfo[format].depth || !!kTextureFormatInfo[format].stencil);
  const resolved = kDepthStencilFormatResolvedAspect[format as DepthStencilFormat][aspect ?? 'all'];
  assert(resolved !== undefined);
  return resolved;
}

/**
 * @returns the sample type of the specified aspect of the specified format.
 */
export function sampleTypeForFormatAndAspect(
  format: GPUTextureFormat,
  aspect: GPUTextureAspect
): 'uint' | 'depth' | 'float' | 'sint' | 'unfilterable-float' {
  const info = kTextureFormatInfo[format];
  if (info.color) {
    assert(aspect === 'all', `color format ${format} used with aspect ${aspect}`);
    return info.color.type;
  } else if (info.depth && info.stencil) {
    if (aspect === 'depth-only') {
      return info.depth.type;
    } else if (aspect === 'stencil-only') {
      return info.stencil.type;
    } else {
      unreachable(`depth-stencil format ${format} used with aspect ${aspect}`);
    }
  } else if (info.depth) {
    assert(aspect !== 'stencil-only', `depth-only format ${format} used with aspect ${aspect}`);
    return info.depth.type;
  } else if (info.stencil) {
    assert(aspect !== 'depth-only', `stencil-only format ${format} used with aspect ${aspect}`);
    return info.stencil.type;
  }
  unreachable();
}

/**
 * Gets all copyable aspects for copies between texture and buffer for specified depth/stencil format and copy type, by spec.
 */
export function depthStencilFormatCopyableAspects(
  type: ImageCopyType,
  format: DepthStencilFormat
): readonly GPUTextureAspect[] {
  const appliedType = type === 'WriteTexture' ? 'CopyB2T' : type;
  return kDepthStencilFormatCapabilityInBufferTextureCopy[format][appliedType];
}

/**
 * Computes whether a copy between a depth/stencil texture aspect and a buffer is supported, by spec.
 */
export function depthStencilBufferTextureCopySupported(
  type: ImageCopyType,
  format: DepthStencilFormat,
  aspect: GPUTextureAspect
): boolean {
  const supportedAspects: readonly GPUTextureAspect[] = depthStencilFormatCopyableAspects(
    type,
    format
  );
  return supportedAspects.includes(aspect);
}

/**
 * Returns the byte size of the depth or stencil aspect of the specified depth/stencil format,
 * or -1 if none.
 */
export function depthStencilFormatAspectSize(
  format: DepthStencilFormat,
  aspect: 'depth-only' | 'stencil-only'
) {
  const texelAspectSize =
    kDepthStencilFormatCapabilityInBufferTextureCopy[format].texelAspectSize[aspect];
  assert(texelAspectSize > 0);
  return texelAspectSize;
}

/**
 * Returns true iff a texture can be created with the provided GPUTextureDimension
 * (defaulting to 2d) and GPUTextureFormat, by spec.
 */
export function textureDimensionAndFormatCompatible(
  dimension: undefined | GPUTextureDimension,
  format: GPUTextureFormat
): boolean {
  const info = kAllTextureFormatInfo[format];
  return !(
    (dimension === '1d' || dimension === '3d') &&
    (info.blockWidth > 1 || info.depth || info.stencil)
  );
}

/**
 * Check if two formats are view format compatible.
 *
 * This function may need to be generalized to use `baseFormat` from `kTextureFormatInfo`.
 */
export function viewCompatible(
  compatibilityMode: boolean,
  a: GPUTextureFormat,
  b: GPUTextureFormat
): boolean {
  return compatibilityMode ? a === b : a === b || a + '-srgb' === b || b + '-srgb' === a;
}

export function getFeaturesForFormats<T>(
  formats: readonly (T & (GPUTextureFormat | undefined))[]
): readonly (GPUFeatureName | undefined)[] {
  return Array.from(new Set(formats.map(f => (f ? kTextureFormatInfo[f].feature : undefined))));
}

export function filterFormatsByFeature<T>(
  feature: GPUFeatureName | undefined,
  formats: readonly (T & (GPUTextureFormat | undefined))[]
): readonly (T & (GPUTextureFormat | undefined))[] {
  return formats.filter(f => f === undefined || kTextureFormatInfo[f].feature === feature);
}

export function isCompressedTextureFormat(format: GPUTextureFormat) {
  return format in kCompressedTextureFormatInfo;
}

export function isDepthTextureFormat(format: GPUTextureFormat) {
  return !!kTextureFormatInfo[format].depth;
}

export function isStencilTextureFormat(format: GPUTextureFormat) {
  return !!kTextureFormatInfo[format].stencil;
}

export function isDepthOrStencilTextureFormat(format: GPUTextureFormat) {
  return isDepthTextureFormat(format) || isStencilTextureFormat(format);
}

export function isEncodableTextureFormat(format: GPUTextureFormat) {
  return kEncodableTextureFormats.includes(format as EncodableTextureFormat);
}

export function canUseAsRenderTarget(format: GPUTextureFormat) {
  return kTextureFormatInfo[format].colorRender || isDepthOrStencilTextureFormat(format);
}

export function is16Float(format: GPUTextureFormat) {
  return format === 'r16float' || format === 'rg16float' || format === 'rgba16float';
}

export function is32Float(format: GPUTextureFormat) {
  return format === 'r32float' || format === 'rg32float' || format === 'rgba32float';
}

/**
 * Returns true if texture is filterable as `texture_xxx<f32>`
 *
 * examples:
 * * 'rgba8unorm' -> true
 * * 'depth16unorm' -> false
 * * 'rgba32float' -> true (you need to enable feature 'float32-filterable')
 */
export function isFilterableAsTextureF32(format: GPUTextureFormat) {
  const info = kTextureFormatInfo[format];
  return info.color?.type === 'float' || is32Float(format);
}

export const kCompatModeUnsupportedStorageTextureFormats: readonly GPUTextureFormat[] = [
  'rg32float',
  'rg32sint',
  'rg32uint',
] as const;

export function isTextureFormatUsableAsStorageFormat(
  format: GPUTextureFormat,
  isCompatibilityMode: boolean
): boolean {
  if (isCompatibilityMode) {
    if (kCompatModeUnsupportedStorageTextureFormats.indexOf(format) >= 0) {
      return false;
    }
  }
  const info = kTextureFormatInfo[format];
  return !!(info.color?.storage || info.depth?.storage || info.stencil?.storage);
}

export function isRegularTextureFormat(format: GPUTextureFormat) {
  return format in kRegularTextureFormatInfo;
}

/**
 * Returns true if format is both compressed and a float format, for example 'bc6h-rgb-ufloat'.
 */
export function isCompressedFloatTextureFormat(format: GPUTextureFormat) {
  return isCompressedTextureFormat(format) && format.includes('float');
}

/**
 * Returns true if format is sint or uint
 */
export function isSintOrUintFormat(format: GPUTextureFormat) {
  const info = kTextureFormatInfo[format];
  const type = info.color?.type ?? info.depth?.type ?? info.stencil?.type;
  return type === 'sint' || type === 'uint';
}

/**
 * Returns true of format can be multisampled.
 */
export const kCompatModeUnsupportedMultisampledTextureFormats: readonly GPUTextureFormat[] = [
  'rgba16float',
  'r32float',
] as const;

export function isMultisampledTextureFormat(
  format: GPUTextureFormat,
  isCompatibilityMode: boolean
): boolean {
  if (isCompatibilityMode) {
    if (kCompatModeUnsupportedMultisampledTextureFormats.indexOf(format) >= 0) {
      return false;
    }
  }
  return kAllTextureFormatInfo[format].multisample;
}

export const kFeaturesForFormats = getFeaturesForFormats(kAllTextureFormats);

/**
 * Given an array of texture formats return the number of bytes per sample.
 */
export function computeBytesPerSampleFromFormats(formats: readonly GPUTextureFormat[]) {
  let bytesPerSample = 0;
  for (const format of formats) {
    const info = kTextureFormatInfo[format];
    const alignedBytesPerSample = align(bytesPerSample, info.colorRender!.alignment);
    bytesPerSample = alignedBytesPerSample + info.colorRender!.byteCost;
  }
  return bytesPerSample;
}

/**
 * Given an array of GPUColorTargetState return the number of bytes per sample
 */
export function computeBytesPerSample(targets: GPUColorTargetState[]) {
  return computeBytesPerSampleFromFormats(targets.map(({ format }) => format));
}
