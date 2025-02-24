export const description = `
Execution tests for the 'textureLoad' builtin function

Reads a single texel from a texture without sampling or filtering.

Returns the unfiltered texel data.

An out of bounds access occurs if:
 * any element of coords is outside the range [0, textureDimensions(t, level)) for the corresponding element, or
 * array_index is outside the range [0, textureNumLayers(t)), or
 * level is outside the range [0, textureNumLevels(t))

If an out of bounds access occurs, the built-in function returns one of:
 * The data for some texel within bounds of the texture
 * A vector (0,0,0,0) or (0,0,0,1) of the appropriate type for non-depth textures
 * 0.0 for depth textures

`;

import { makeTestGroup } from '../../../../../../common/framework/test_group.js';
import {
  isCompressedFloatTextureFormat,
  isDepthTextureFormat,
  isMultisampledTextureFormat,
  isStencilTextureFormat,
  kDepthStencilFormats,
  kAllTextureFormats,
  kTextureFormatInfo,
  textureDimensionAndFormatCompatible,
} from '../../../../../format_info.js';
import { GPUTest, MaxLimitsTestMixin } from '../../../../../gpu_test.js';
import { maxMipLevelCount, virtualMipSize } from '../../../../../util/texture/base.js';
import { TexelFormats } from '../../../../types.js';

import {
  TextureCall,
  checkCallResults,
  chooseTextureSize,
  createTextureWithRandomDataAndGetTexels,
  doTextureCalls,
  appendComponentTypeForFormatToTextureType,
  vec1,
  vec2,
  vec3,
  kSamplePointMethods,
  kShortShaderStages,
  generateTextureBuiltinInputs1D,
  generateTextureBuiltinInputs2D,
  generateTextureBuiltinInputs3D,
  Dimensionality,
  createVideoFrameWithRandomDataAndGetTexels,
  ShortShaderStage,
  isFillable,
} from './texture_utils.js';

export function normalizedCoordToTexelLoadTestCoord<T extends Dimensionality>(
  descriptor: GPUTextureDescriptor,
  mipLevel: number,
  coordType: 'i32' | 'u32',
  v: T
): T {
  const size = virtualMipSize(descriptor.dimension ?? '2d', descriptor.size, mipLevel);
  return v.map((v, i) => {
    const t = v * size[i];
    return coordType === 'u32' ? Math.abs(Math.round(t)) : Math.round(t);
  }) as T;
}

function skipIfStorageTexturesNotSupportedInStage(t: GPUTest, stage: ShortShaderStage) {
  if (t.isCompatibility) {
    t.skipIf(
      stage === 'f' && !(t.device.limits.maxStorageTexturesInFragmentStage! > 0),
      'device does not support storage textures in fragment shaders'
    );
    t.skipIf(
      stage === 'v' && !(t.device.limits.maxStorageTexturesInVertexStage! > 0),
      'device does not support storage textures in vertex shaders'
    );
  }
}

export const g = makeTestGroup(MaxLimitsTestMixin(GPUTest));

g.test('sampled_1d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_1d<T>, coords: C, level: C) -> vec4<T>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * level: The mip level, with level 0 containing a full size version of the texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('format', kAllTextureFormats)
      .filter(t => textureDimensionAndFormatCompatible('1d', t.format))
      // 1d textures can't have a height !== 1
      .filter(t => kTextureFormatInfo[t.format].blockHeight === 1)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('L', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    const { format } = t.params;
    t.skipIfTextureFormatNotSupported(format);
    t.selectDeviceForTextureFormatOrSkipTestCase(t.params.format);
  })
  .fn(async t => {
    const { format, stage, C, L, samplePoints } = t.params;

    // We want at least 4 blocks or something wide enough for 3 mip levels.
    const [width] = chooseTextureSize({ minSize: 8, minBlocks: 4, format });
    const size = [width, 1];

    const descriptor: GPUTextureDescriptor = {
      format,
      dimension: '1d',
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec1>[] = generateTextureBuiltinInputs1D(50, {
      method: samplePoints,
      descriptor,
      mipLevel: { num: texture.mipLevelCount, type: L },
      hashInputs: [stage, format, samplePoints, C, L],
    }).map(({ coords, mipLevel }, i) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        levelType: L === 'i32' ? 'i' : 'u',
        mipLevel,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, mipLevel, C, coords),
      };
    });

    const textureType = appendComponentTypeForFormatToTextureType('texture_1d', texture.format);
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('sampled_2d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32
L is i32 or u32

fn textureLoad(t: texture_2d<T>, coords: vec2<C>, level: L) -> vec4<T>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * level: The mip level, with level 0 containing a full size version of the texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('format', kAllTextureFormats)
      .filter(t => !isCompressedFloatTextureFormat(t.format))
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('L', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    const { format } = t.params;
    t.skipIfTextureFormatNotSupported(format);
    t.selectDeviceForTextureFormatOrSkipTestCase(t.params.format);
  })
  .fn(async t => {
    const { format, stage, samplePoints, C, L } = t.params;

    // We want at least 4 blocks or something wide enough for 3 mip levels.
    const size = chooseTextureSize({ minSize: 8, minBlocks: 4, format });

    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
      mipLevelCount: maxMipLevelCount({ size }),
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      mipLevel: { num: texture.mipLevelCount, type: L },
      hashInputs: [stage, format, samplePoints, C, L],
    }).map(({ coords, mipLevel }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        levelType: L === 'i32' ? 'i' : 'u',
        mipLevel,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, mipLevel, C, coords),
      };
    });

    const textureType = appendComponentTypeForFormatToTextureType('texture_2d', texture.format);
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('sampled_3d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_3d<T>, coords: vec3<C>, level: C) -> vec4<T>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * level: The mip level, with level 0 containing a full size version of the texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('format', kAllTextureFormats)
      .filter(t => textureDimensionAndFormatCompatible('3d', t.format))
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('L', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    const { format } = t.params;
    t.skipIfTextureFormatNotSupported(format);
    t.selectDeviceForTextureFormatOrSkipTestCase(t.params.format);
  })
  .fn(async t => {
    const { format, stage, samplePoints, C, L } = t.params;

    // We want at least 4 blocks or something wide enough for 3 mip levels.
    const size = chooseTextureSize({ minSize: 8, minBlocks: 4, format, viewDimension: '3d' });

    const descriptor: GPUTextureDescriptor = {
      format,
      dimension: '3d',
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
      mipLevelCount: maxMipLevelCount({ size }),
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec3>[] = generateTextureBuiltinInputs3D(50, {
      method: samplePoints,
      descriptor,
      mipLevel: { num: texture.mipLevelCount, type: L },
      hashInputs: [stage, format, samplePoints, C, L],
    }).map(({ coords, mipLevel }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        levelType: L === 'i32' ? 'i' : 'u',
        mipLevel,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, mipLevel, C, coords),
      };
    });

    const textureType = appendComponentTypeForFormatToTextureType('texture_3d', texture.format);
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('multisampled')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32
S is i32 or u32

fn textureLoad(t: texture_multisampled_2d<T>, coords: vec2<C>, sample_index: S)-> vec4<T>
fn textureLoad(t: texture_depth_multisampled_2d, coords: vec2<C>, sample_index: S)-> f32

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * sample_index: The 0-based sample index of the multisampled texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('texture_type', [
        'texture_multisampled_2d',
        'texture_depth_multisampled_2d',
      ] as const)
      .combine('format', kAllTextureFormats)
      .filter(t => isMultisampledTextureFormat(t.format, false))
      .filter(t => !isStencilTextureFormat(t.format))
      // Filter out texture_depth_multisampled_2d with non-depth formats
      .filter(
        t =>
          !(t.texture_type === 'texture_depth_multisampled_2d' && !isDepthTextureFormat(t.format))
      )
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('S', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    const { format, texture_type } = t.params;
    t.skipIfTextureFormatNotSupported(format);
    t.skipIfTextureLoadNotSupportedForTextureType(texture_type);
    t.skipIfMultisampleNotSupportedForFormat(format);
  })
  .fn(async t => {
    const { texture_type, format, stage, samplePoints, C, S } = t.params;

    const sampleCount = 4;
    const descriptor: GPUTextureDescriptor = {
      format,
      size: [8, 8],
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
      sampleCount,
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      sampleIndex: { num: texture.sampleCount, type: S },
      hashInputs: [stage, format, samplePoints, C, S],
    }).map(({ coords, sampleIndex }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        sampleIndexType: S === 'i32' ? 'i' : 'u',
        sampleIndex,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
      };
    });

    const textureType = appendComponentTypeForFormatToTextureType(texture_type, texture.format);
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('depth')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_depth_2d, coords: vec2<C>, level: L) -> f32

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * level: The mip level, with level 0 containing a full size version of the texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('format', kDepthStencilFormats)
      // filter out stencil only formats
      .filter(t => isDepthTextureFormat(t.format))
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('L', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    t.skipIfTextureLoadNotSupportedForTextureType('texture_depth_2d');
    t.selectDeviceForTextureFormatOrSkipTestCase(t.params.format);
  })
  .fn(async t => {
    const { format, stage, samplePoints, C, L } = t.params;

    // We want at least 4 blocks or something wide enough for 3 mip levels.
    const size = chooseTextureSize({ minSize: 8, minBlocks: 4, format });

    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
      mipLevelCount: maxMipLevelCount({ size }),
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      mipLevel: { num: texture.mipLevelCount, type: L },
      hashInputs: [stage, format, samplePoints, C, L],
    }).map(({ coords, mipLevel }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        levelType: L === 'i32' ? 'i' : 'u',
        mipLevel,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, mipLevel, C, coords),
      };
    });
    const textureType = 'texture_depth_2d';
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('external')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_external, coords: vec2<C>) -> vec4<f32>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate.
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('L', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t =>
    t.skipIf(typeof VideoFrame === 'undefined', 'VideoFrames are not supported')
  )
  .fn(async t => {
    const { stage, samplePoints, C, L } = t.params;

    const size = [8, 8, 1];

    // Note: external texture doesn't use this descriptor.
    // It's used to pass to the softwareTextureRead functions.
    const descriptor: GPUTextureDescriptor = {
      format: 'rgba8unorm',
      size,
      usage: GPUTextureUsage.COPY_DST,
    };

    const { texels, videoFrame } = createVideoFrameWithRandomDataAndGetTexels(descriptor.size);
    const texture = t.device.importExternalTexture({ source: videoFrame });

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      hashInputs: [samplePoints, C, L],
    }).map(({ coords }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
      };
    });

    const textureType = 'texture_external';
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage
    );
    t.expectOK(res);
    videoFrame.close();
  });

g.test('arrayed')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_2d_array<T>, coords: vec2<C>, array_index: A, level: L) -> vec4<T>
fn textureLoad(t: texture_depth_2d_array, coords: vec2<C>, array_index: A, level: L) -> f32

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * array_index: The 0-based texture array index
 * level: The mip level, with level 0 containing a full size version of the texture
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combine('format', kAllTextureFormats)
      .filter(t => isFillable(t.format))
      .combine('texture_type', ['texture_2d_array', 'texture_depth_2d_array'] as const)
      .filter(
        t => !(t.texture_type === 'texture_depth_2d_array' && !isDepthTextureFormat(t.format))
      )
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combineWithParams([
        { C: 'i32', A: 'u32', L: 'u32' },
        { C: 'u32', A: 'u32', L: 'u32' },
        { C: 'u32', A: 'i32', L: 'u32' },
        { C: 'u32', A: 'u32', L: 'i32' },
      ] as const)
      .combine('depthOrArrayLayers', [1, 8] as const)
  )
  .beforeAllSubcases(t => {
    const { format, texture_type } = t.params;
    t.skipIfTextureFormatNotSupported(format);
    t.skipIfTextureLoadNotSupportedForTextureType(texture_type);
    t.selectDeviceForTextureFormatOrSkipTestCase(format);
  })
  .fn(async t => {
    const { texture_type, format, stage, samplePoints, C, A, L, depthOrArrayLayers } = t.params;

    // We want at least 4 blocks or something wide enough for 3 mip levels.
    const [width, height] = chooseTextureSize({ minSize: 8, minBlocks: 4, format });
    const size = { width, height, depthOrArrayLayers };
    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.TEXTURE_BINDING,
      mipLevelCount: maxMipLevelCount({ size }),
      ...(t.isCompatibility && { textureBindingViewDimension: '2d-array' }),
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      mipLevel: { num: texture.mipLevelCount, type: L },
      arrayIndex: { num: texture.depthOrArrayLayers, type: A },
      hashInputs: [stage, format, samplePoints, C, L, A],
    }).map(({ coords, mipLevel, arrayIndex }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        levelType: L === 'i32' ? 'i' : 'u',
        arrayIndexType: A === 'i32' ? 'i' : 'u',
        arrayIndex,
        mipLevel,
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, mipLevel, C, coords),
      };
    });
    const textureType = appendComponentTypeForFormatToTextureType(texture_type, texture.format);
    const viewDescriptor: GPUTextureViewDescriptor = { dimension: '2d-array' };
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('storage_textures_1d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_storage_1d<format, read>, coords: C) -> vec4<f32>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combineWithParams([...TexelFormats, { format: 'bgra8unorm' }] as const)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    t.skipIf(!t.hasLanguageFeature('readonly_and_readwrite_storage_textures'));
    if (t.params.format === 'bgra8unorm') {
      t.selectDeviceOrSkipTestCase('bgra8unorm-storage');
    } else {
      t.skipIfTextureFormatNotUsableAsStorageTexture(t.params.format as GPUTextureFormat);
    }
  })
  .fn(async t => {
    const { format, stage, samplePoints, C } = t.params;

    skipIfStorageTexturesNotSupportedInStage(t, stage);

    // We want at least 3 blocks or something wide enough for 3 mip levels.
    const [width] = chooseTextureSize({ minSize: 8, minBlocks: 4, format });
    const size = [width, 1];
    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.STORAGE_BINDING,
      dimension: '1d',
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec1>[] = generateTextureBuiltinInputs1D(50, {
      method: samplePoints,
      descriptor,
      hashInputs: [stage, format, samplePoints, C],
    }).map(({ coords }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
      };
    });
    const textureType = `texture_storage_1d<${format}, read>`;
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('storage_textures_2d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_storage_2d<format, read>, coords: vec2<C>) -> vec4<f32>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combineWithParams([...TexelFormats, { format: 'bgra8unorm' }] as const)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    t.skipIf(!t.hasLanguageFeature('readonly_and_readwrite_storage_textures'));
    if (t.params.format === 'bgra8unorm') {
      t.selectDeviceOrSkipTestCase('bgra8unorm-storage');
    } else {
      t.skipIfTextureFormatNotUsableAsStorageTexture(t.params.format as GPUTextureFormat);
    }
  })
  .fn(async t => {
    const { format, stage, samplePoints, C } = t.params;

    skipIfStorageTexturesNotSupportedInStage(t, stage);

    // We want at least 3 blocks or something wide enough for 3 mip levels.
    const size = chooseTextureSize({ minSize: 8, minBlocks: 3, format });
    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.STORAGE_BINDING,
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      hashInputs: [stage, format, samplePoints, C],
    }).map(({ coords }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
      };
    });
    const textureType = `texture_storage_2d<${format}, read>`;
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('storage_textures_2d_array')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32
A is i32 or u32

fn textureLoad(t: texture_storage_2d<format, read>, coords: vec2<C>, array_index: A) -> vec4<f32>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
 * array_index: The 0-based texture array index
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combineWithParams([...TexelFormats, { format: 'bgra8unorm' }] as const)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
      .combine('A', ['i32', 'u32'] as const)
      .combine('depthOrArrayLayers', [1, 8] as const)
  )
  .beforeAllSubcases(t => {
    t.skipIf(!t.hasLanguageFeature('readonly_and_readwrite_storage_textures'));
    if (t.params.format === 'bgra8unorm') {
      t.selectDeviceOrSkipTestCase('bgra8unorm-storage');
    } else {
      t.skipIfTextureFormatNotUsableAsStorageTexture(t.params.format as GPUTextureFormat);
    }
  })
  .fn(async t => {
    const { format, stage, samplePoints, C, A, depthOrArrayLayers } = t.params;

    skipIfStorageTexturesNotSupportedInStage(t, stage);

    // We want at least 3 blocks or something wide enough for 3 mip levels.
    const [width, height] = chooseTextureSize({ minSize: 8, minBlocks: 4, format });
    const size = { width, height, depthOrArrayLayers };
    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.STORAGE_BINDING,
      ...(t.isCompatibility && { textureBindingViewDimension: '2d-array' }),
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec2>[] = generateTextureBuiltinInputs2D(50, {
      method: samplePoints,
      descriptor,
      arrayIndex: { num: texture.depthOrArrayLayers, type: A },
      hashInputs: [stage, format, samplePoints, C, A],
    }).map(({ coords, arrayIndex }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
        arrayIndexType: A === 'i32' ? 'i' : 'u',
        arrayIndex,
      };
    });
    const textureType = `texture_storage_2d_array<${format}, read>`;
    const viewDescriptor: GPUTextureViewDescriptor = {
      dimension: '2d-array',
    };
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });

g.test('storage_textures_3d')
  .specURL('https://www.w3.org/TR/WGSL/#textureload')
  .desc(
    `
C is i32 or u32

fn textureLoad(t: texture_storage_2d<format, read>, coords: vec3<C>) -> vec4<f32>

Parameters:
 * t: The sampled texture to read from
 * coords: The 0-based texel coordinate
`
  )
  .params(u =>
    u
      .combine('stage', kShortShaderStages)
      .combineWithParams([...TexelFormats, { format: 'bgra8unorm' }] as const)
      .beginSubcases()
      .combine('samplePoints', kSamplePointMethods)
      .combine('C', ['i32', 'u32'] as const)
  )
  .beforeAllSubcases(t => {
    t.skipIf(!t.hasLanguageFeature('readonly_and_readwrite_storage_textures'));
    if (t.params.format === 'bgra8unorm') {
      t.selectDeviceOrSkipTestCase('bgra8unorm-storage');
    } else {
      t.skipIfTextureFormatNotUsableAsStorageTexture(t.params.format as GPUTextureFormat);
    }
  })
  .fn(async t => {
    const { format, stage, samplePoints, C } = t.params;

    skipIfStorageTexturesNotSupportedInStage(t, stage);

    // We want at least 3 blocks or something wide enough for 3 mip levels.
    const size = chooseTextureSize({ minSize: 8, minBlocks: 4, format, viewDimension: '3d' });
    const descriptor: GPUTextureDescriptor = {
      format,
      size,
      usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.STORAGE_BINDING,
      dimension: '3d',
    };
    const { texels, texture } = await createTextureWithRandomDataAndGetTexels(t, descriptor);

    const calls: TextureCall<vec3>[] = generateTextureBuiltinInputs3D(50, {
      method: samplePoints,
      descriptor,
      hashInputs: [stage, format, samplePoints, C],
    }).map(({ coords }) => {
      return {
        builtin: 'textureLoad',
        coordType: C === 'i32' ? 'i' : 'u',
        coords: normalizedCoordToTexelLoadTestCoord(descriptor, 0, C, coords),
      };
    });
    const textureType = `texture_storage_3d<${format}, read>`;
    const viewDescriptor = {};
    const sampler = undefined;
    const results = await doTextureCalls(
      t,
      texture,
      viewDescriptor,
      textureType,
      sampler,
      calls,
      stage
    );
    const res = await checkCallResults(
      t,
      { texels, descriptor, viewDescriptor },
      textureType,
      sampler,
      calls,
      results,
      stage,
      texture
    );
    t.expectOK(res);
  });
