import {
  Fixture,
  FixtureClass,
  FixtureClassInterface,
  FixtureClassWithMixin,
  SkipTestCase,
  SubcaseBatchState,
  TestCaseRecorder,
  TestParams,
} from '../common/framework/fixture.js';
import { globalTestConfig } from '../common/framework/test_config.js';
import { getGPU } from '../common/util/navigator_gpu.js';
import {
  assert,
  makeValueTestVariant,
  memcpy,
  range,
  ValueTestVariant,
  TypedArrayBufferView,
  TypedArrayBufferViewConstructor,
  unreachable,
} from '../common/util/util.js';

import { kLimits, kQueryTypeInfo, WGSLLanguageFeature } from './capability_info.js';
import { InterpolationType, InterpolationSampling } from './constants.js';
import {
  kTextureFormatInfo,
  kEncodableTextureFormats,
  resolvePerAspectFormat,
  SizedTextureFormat,
  EncodableTextureFormat,
  isCompressedTextureFormat,
  ColorTextureFormat,
  isTextureFormatUsableAsStorageFormat,
  isMultisampledTextureFormat,
} from './format_info.js';
import { checkElementsEqual, checkElementsBetween } from './util/check_contents.js';
import { CommandBufferMaker, EncoderType } from './util/command_buffer_maker.js';
import { ScalarType } from './util/conversion.js';
import {
  CanonicalDeviceDescriptor,
  DescriptorModifier,
  DevicePool,
  DeviceProvider,
  UncanonicalizedDeviceDescriptor,
} from './util/device_pool.js';
import { align, roundDown } from './util/math.js';
import { physicalMipSizeFromTexture, virtualMipSize } from './util/texture/base.js';
import {
  bytesInACompleteRow,
  getTextureCopyLayout,
  getTextureSubCopyLayout,
  LayoutOptions as TextureLayoutOptions,
} from './util/texture/layout.js';
import { PerTexelComponent, kTexelRepresentationInfo } from './util/texture/texel_data.js';
import { TexelView } from './util/texture/texel_view.js';
import {
  PerPixelComparison,
  PixelExpectation,
  TexelCompareOptions,
  textureContentIsOKByT2B,
} from './util/texture/texture_ok.js';
import { createTextureFromTexelViews } from './util/texture.js';
import { reifyExtent3D, reifyOrigin3D } from './util/unions.js';

// Declarations for WebGPU items we want tests for that are not yet officially part of the spec.
declare global {
  // MAINTENANCE_TODO: remove once added to @webgpu/types
  interface GPUSupportedLimits {
    readonly maxStorageBuffersInFragmentStage?: number;
    readonly maxStorageTexturesInFragmentStage?: number;
    readonly maxStorageBuffersInVertexStage?: number;
    readonly maxStorageTexturesInVertexStage?: number;
  }
}

const devicePool = new DevicePool();

// MAINTENANCE_TODO: When DevicePool becomes able to provide multiple devices at once, use the
// usual one instead of a new one.
const mismatchedDevicePool = new DevicePool();

const kResourceStateValues = ['valid', 'invalid', 'destroyed'] as const;
export type ResourceState = (typeof kResourceStateValues)[number];
export const kResourceStates: readonly ResourceState[] = kResourceStateValues;

/** Various "convenient" shorthands for GPUDeviceDescriptors for selectDevice functions. */
export type DeviceSelectionDescriptor =
  | UncanonicalizedDeviceDescriptor
  | GPUFeatureName
  | undefined
  | Array<GPUFeatureName | undefined>;

export function initUncanonicalizedDeviceDescriptor(
  descriptor: DeviceSelectionDescriptor
): UncanonicalizedDeviceDescriptor {
  if (typeof descriptor === 'string') {
    return { requiredFeatures: [descriptor] };
  } else if (descriptor instanceof Array) {
    return {
      requiredFeatures: descriptor.filter(f => f !== undefined) as GPUFeatureName[],
    };
  } else {
    return descriptor ?? {};
  }
}

type DeviceDescriptorSimplified = {
  requiredFeatures: GPUFeatureName[];
  requiredLimits: Record<string, number>;
  defaultQueue: GPUQueueDescriptor;
};

function mergeDeviceSelectionDescriptorIntoDeviceDescriptor(
  src: DeviceSelectionDescriptor,
  dst: DeviceDescriptorSimplified
) {
  const srcFixed = initUncanonicalizedDeviceDescriptor(src);
  if (srcFixed) {
    dst.requiredFeatures.push(...(srcFixed.requiredFeatures ?? []));
    Object.assign(dst.requiredLimits, srcFixed.requiredLimits ?? {});
  }
}

export class GPUTestSubcaseBatchState extends SubcaseBatchState {
  /** Provider for default device. */
  private provider: Promise<DeviceProvider> | undefined;
  /** Provider for mismatched device. */
  private mismatchedProvider: Promise<DeviceProvider> | undefined;
  /** The accumulated skip-if requirements for this subcase */
  private skipIfRequirements: DeviceDescriptorSimplified = {
    requiredFeatures: [],
    requiredLimits: {},
    defaultQueue: {},
  };

  override async postInit(): Promise<void> {
    // Skip all subcases if there's no device.
    await this.acquireProvider();
  }

  override async finalize(): Promise<void> {
    await super.finalize();

    // Ensure devicePool.release is called for both providers even if one rejects.
    await Promise.all([
      this.provider?.then(x => devicePool.release(x)),
      this.mismatchedProvider?.then(x => devicePool.release(x)),
    ]);
  }

  /** @internal MAINTENANCE_TODO: Make this not visible to test code? */
  acquireProvider(): Promise<DeviceProvider> {
    if (this.provider === undefined) {
      this.requestDeviceWithRequiredParametersOrSkip(this.skipIfRequirements);
    }
    assert(this.provider !== undefined);
    return this.provider;
  }

  get isCompatibility() {
    return globalTestConfig.compatibility;
  }

  /**
   * Some tests or cases need particular feature flags or limits to be enabled.
   * Call this function with a descriptor or feature name (or `undefined`) to select a
   * GPUDevice with matching capabilities. If this isn't called, a default device is provided.
   *
   * If the request isn't supported, throws a SkipTestCase exception to skip the entire test case.
   */
  requestDeviceWithRequiredParametersOrSkip(
    descriptor: DeviceSelectionDescriptor,
    descriptorModifier?: DescriptorModifier
  ): void {
    assert(this.provider === undefined, "Can't selectDeviceOrSkipTestCase() multiple times");
    this.provider = devicePool.acquire(
      this.recorder,
      initUncanonicalizedDeviceDescriptor(descriptor),
      descriptorModifier
    );
    // Suppress uncaught promise rejection (we'll catch it later).
    this.provider.catch(() => {});
  }

  /**
   * Some tests or cases need particular feature flags or limits to be enabled.
   * Call this function with a descriptor or feature name (or `undefined`) to add
   * features or limits required by the subcase. If the features or limits are not
   * available a SkipTestCase exception will be thrown to skip the entire test case.
   */
  selectDeviceOrSkipTestCase(descriptor: DeviceSelectionDescriptor): void {
    mergeDeviceSelectionDescriptorIntoDeviceDescriptor(descriptor, this.skipIfRequirements);
  }

  /**
   * Convenience function for {@link selectDeviceOrSkipTestCase}.
   * Select a device with the features required by these texture format(s).
   * If the device creation fails, then skip the test case.
   */
  selectDeviceForTextureFormatOrSkipTestCase(
    formats: GPUTextureFormat | undefined | (GPUTextureFormat | undefined)[]
  ): void {
    if (!Array.isArray(formats)) {
      formats = [formats];
    }
    const features = new Set<GPUFeatureName | undefined>();
    for (const format of formats) {
      if (format !== undefined) {
        this.skipIfTextureFormatNotSupported(format);
        features.add(kTextureFormatInfo[format].feature);
      }
    }

    this.selectDeviceOrSkipTestCase(Array.from(features));
  }

  /**
   * Convenience function for {@link selectDeviceOrSkipTestCase}.
   * Select a device with the features required by these query type(s).
   * If the device creation fails, then skip the test case.
   */
  selectDeviceForQueryTypeOrSkipTestCase(types: GPUQueryType | GPUQueryType[]): void {
    if (!Array.isArray(types)) {
      types = [types];
    }
    const features = types.map(t => kQueryTypeInfo[t].feature);
    this.selectDeviceOrSkipTestCase(features);
  }

  /** @internal MAINTENANCE_TODO: Make this not visible to test code? */
  acquireMismatchedProvider(): Promise<DeviceProvider> | undefined {
    return this.mismatchedProvider;
  }

  /**
   * Some tests need a second device which is different from the first.
   * This requests a second device so it will be available during the test. If it is not called,
   * no second device will be available.
   *
   * If the request isn't supported, throws a SkipTestCase exception to skip the entire test case.
   */
  selectMismatchedDeviceOrSkipTestCase(descriptor: DeviceSelectionDescriptor): void {
    assert(
      this.mismatchedProvider === undefined,
      "Can't selectMismatchedDeviceOrSkipTestCase() multiple times"
    );

    this.mismatchedProvider = mismatchedDevicePool.acquire(
      this.recorder,
      initUncanonicalizedDeviceDescriptor(descriptor),
      undefined
    );
    // Suppress uncaught promise rejection (we'll catch it later).
    this.mismatchedProvider.catch(() => {});
  }

  /** Throws an exception marking the subcase as skipped. */
  skip(msg: string): never {
    throw new SkipTestCase(msg);
  }

  /** Throws an exception making the subcase as skipped if condition is true */
  skipIf(cond: boolean, msg: string | (() => string) = '') {
    if (cond) {
      this.skip(typeof msg === 'function' ? msg() : msg);
    }
  }

  /**
   * Skips test if any format is not supported.
   */
  skipIfTextureFormatNotSupported(...formats: (GPUTextureFormat | undefined)[]) {
    if (this.isCompatibility) {
      for (const format of formats) {
        if (format === 'bgra8unorm-srgb') {
          this.skip(`texture format '${format} is not supported`);
        }
      }
    }
  }

  skipIfMultisampleNotSupportedForFormat(...formats: (GPUTextureFormat | undefined)[]) {
    for (const format of formats) {
      if (format === undefined) continue;
      if (!isMultisampledTextureFormat(format, this.isCompatibility)) {
        this.skip(`texture format '${format}' is not supported to be multisampled`);
      }
    }

    this.skipIfColorRenderableNotSupportedForFormat(...formats);
  }

  skipIfColorRenderableNotSupportedForFormat(...formats: (GPUTextureFormat | undefined)[]) {
    for (const format of formats) {
      if (format === undefined) continue;
      if (!kTextureFormatInfo[format].color) {
        this.skip(`texture format '${format} is not color renderable`);
      }
    }
  }

  skipIfCopyTextureToTextureNotSupportedForFormat(...formats: (GPUTextureFormat | undefined)[]) {
    if (this.isCompatibility) {
      for (const format of formats) {
        if (format && isCompressedTextureFormat(format)) {
          this.skip(`copyTextureToTexture with ${format} is not supported`);
        }
      }
    }
  }

  skipIfTextureViewDimensionNotSupported(...dimensions: (GPUTextureViewDimension | undefined)[]) {
    if (this.isCompatibility) {
      for (const dimension of dimensions) {
        if (dimension === 'cube-array') {
          this.skip(`texture view dimension '${dimension}' is not supported`);
        }
      }
    }
  }

  skipIfTextureFormatNotUsableAsStorageTexture(...formats: (GPUTextureFormat | undefined)[]) {
    for (const format of formats) {
      if (format && !isTextureFormatUsableAsStorageFormat(format, this.isCompatibility)) {
        this.skip(`Texture with ${format} is not usable as a storage texture`);
      }
    }
  }

  skipIfTextureLoadNotSupportedForTextureType(...types: (string | undefined | null)[]) {
    if (this.isCompatibility) {
      for (const type of types) {
        switch (type) {
          case 'texture_depth_2d':
          case 'texture_depth_2d_array':
          case 'texture_depth_multisampled_2d':
            this.skip(`${type} is not supported by textureLoad in compatibility mode`);
        }
      }
    }
  }

  /**
   * Skips test if the given interpolation type or sampling is not supported.
   */
  skipIfInterpolationTypeOrSamplingNotSupported({
    type,
    sampling,
  }: {
    type?: InterpolationType;
    sampling?: InterpolationSampling;
  }) {
    if (this.isCompatibility) {
      this.skipIf(
        type === 'linear',
        'interpolation type linear is not supported in compatibility mode'
      );
      this.skipIf(
        sampling === 'sample',
        'interpolation type linear is not supported in compatibility mode'
      );
      this.skipIf(
        type === 'flat' && (!sampling || sampling === 'first'),
        'interpolation type flat with sampling not set to either is not supported in compatibility mode'
      );
    }
  }

  /** Skips this test case if a depth texture can not be used with a non-comparison sampler. */
  skipIfDepthTextureCanNotBeUsedWithNonComparisonSampler() {
    this.skipIf(
      this.isCompatibility,
      'depth textures are not usable with non-comparison samplers in compatibility mode'
    );
  }

  /** Skips this test case if the `langFeature` is *not* supported. */
  skipIfLanguageFeatureNotSupported(langFeature: WGSLLanguageFeature) {
    if (!this.hasLanguageFeature(langFeature)) {
      this.skip(`WGSL language feature '${langFeature}' is not supported`);
    }
  }

  /** Skips this test case if the `langFeature` is supported. */
  skipIfLanguageFeatureSupported(langFeature: WGSLLanguageFeature) {
    if (this.hasLanguageFeature(langFeature)) {
      this.skip(`WGSL language feature '${langFeature}' is supported`);
    }
  }

  /** returns true iff the `langFeature` is supported  */
  hasLanguageFeature(langFeature: WGSLLanguageFeature) {
    const lf = getGPU(this.recorder).wgslLanguageFeatures;
    return lf !== undefined && lf.has(langFeature);
  }
}

/**
 * Base fixture for WebGPU tests.
 *
 * This class is a Fixture + a getter that returns a GPUDevice
 * as well as helpers that use that device.
 */
export class GPUTestBase extends Fixture<GPUTestSubcaseBatchState> {
  public static override MakeSharedState(
    recorder: TestCaseRecorder,
    params: TestParams
  ): GPUTestSubcaseBatchState {
    return new GPUTestSubcaseBatchState(recorder, params);
  }

  // This must be overridden in derived classes
  get device(): GPUDevice {
    unreachable();
    return null as unknown as GPUDevice;
  }

  /** GPUQueue for the test to use. (Same as `t.device.queue`.) */
  get queue(): GPUQueue {
    return this.device.queue;
  }

  get isCompatibility() {
    return globalTestConfig.compatibility;
  }

  makeLimitVariant(limit: (typeof kLimits)[number], variant: ValueTestVariant) {
    return makeValueTestVariant(this.device.limits[limit]!, variant);
  }

  canCallCopyTextureToBufferWithTextureFormat(format: GPUTextureFormat) {
    return !this.isCompatibility || !isCompressedTextureFormat(format);
  }

  /** Snapshot a GPUBuffer's contents, returning a new GPUBuffer with the `MAP_READ` usage. */
  private createCopyForMapRead(src: GPUBuffer, srcOffset: number, size: number): GPUBuffer {
    assert(srcOffset % 4 === 0);
    assert(size % 4 === 0);

    const dst = this.createBufferTracked({
      size,
      usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST,
    });

    const c = this.device.createCommandEncoder();
    c.copyBufferToBuffer(src, srcOffset, dst, 0, size);
    this.queue.submit([c.finish()]);

    return dst;
  }

  /**
   * Offset and size passed to createCopyForMapRead must be divisible by 4. For that
   * we might need to copy more bytes from the buffer than we want to map.
   * begin and end values represent the part of the copied buffer that stores the contents
   * we initially wanted to map.
   * The copy will not cause an OOB error because the buffer size must be 4-aligned.
   */
  private createAlignedCopyForMapRead(
    src: GPUBuffer,
    size: number,
    offset: number
  ): { mappable: GPUBuffer; subarrayByteStart: number } {
    const alignedOffset = roundDown(offset, 4);
    const subarrayByteStart = offset - alignedOffset;
    const alignedSize = align(size + subarrayByteStart, 4);
    const mappable = this.createCopyForMapRead(src, alignedOffset, alignedSize);
    return { mappable, subarrayByteStart };
  }

  /**
   * Snapshot the current contents of a range of a GPUBuffer, and return them as a TypedArray.
   * Also provides a cleanup() function to unmap and destroy the staging buffer.
   */
  async readGPUBufferRangeTyped<T extends TypedArrayBufferView>(
    src: GPUBuffer,
    {
      srcByteOffset = 0,
      method = 'copy',
      type,
      typedLength,
    }: {
      srcByteOffset?: number;
      method?: 'copy' | 'map';
      type: TypedArrayBufferViewConstructor<T>;
      typedLength: number;
    }
  ): Promise<{ data: T; cleanup(): void }> {
    assert(
      srcByteOffset % type.BYTES_PER_ELEMENT === 0,
      'srcByteOffset must be a multiple of BYTES_PER_ELEMENT'
    );

    const byteLength = typedLength * type.BYTES_PER_ELEMENT;
    let mappable: GPUBuffer;
    let mapOffset: number | undefined, mapSize: number | undefined, subarrayByteStart: number;
    if (method === 'copy') {
      ({ mappable, subarrayByteStart } = this.createAlignedCopyForMapRead(
        src,
        byteLength,
        srcByteOffset
      ));
    } else if (method === 'map') {
      mappable = src;
      mapOffset = roundDown(srcByteOffset, 8);
      mapSize = align(byteLength, 4);
      subarrayByteStart = srcByteOffset - mapOffset;
    } else {
      unreachable();
    }

    assert(subarrayByteStart % type.BYTES_PER_ELEMENT === 0);
    const subarrayStart = subarrayByteStart / type.BYTES_PER_ELEMENT;

    // 2. Map the staging buffer, and create the TypedArray from it.
    await mappable.mapAsync(GPUMapMode.READ, mapOffset, mapSize);
    const mapped = new type(mappable.getMappedRange(mapOffset, mapSize));
    const data = mapped.subarray(subarrayStart, typedLength) as T;

    return {
      data,
      cleanup() {
        mappable.unmap();
        mappable.destroy();
      },
    };
  }

  /**
   * Skips test if any format is not supported.
   */
  skipIfTextureFormatNotSupported(...formats: (GPUTextureFormat | undefined)[]) {
    if (this.isCompatibility) {
      for (const format of formats) {
        if (format === 'bgra8unorm-srgb') {
          this.skip(`texture format '${format} is not supported`);
        }
      }
    }
  }

  skipIfColorRenderableNotSupportedForFormat(...formats: (GPUTextureFormat | undefined)[]) {
    for (const format of formats) {
      if (format === undefined) continue;
      if (!kTextureFormatInfo[format].color) {
        this.skip(`texture format '${format} is not color renderable`);
      }
    }
  }

  skipIfTextureViewDimensionNotSupported(...dimensions: (GPUTextureViewDimension | undefined)[]) {
    if (this.isCompatibility) {
      for (const dimension of dimensions) {
        if (dimension === 'cube-array') {
          this.skip(`texture view dimension '${dimension}' is not supported`);
        }
      }
    }
  }

  skipIfCopyTextureToTextureNotSupportedForFormat(...formats: (GPUTextureFormat | undefined)[]) {
    if (this.isCompatibility) {
      for (const format of formats) {
        if (format && isCompressedTextureFormat(format)) {
          this.skip(`copyTextureToTexture with ${format} is not supported`);
        }
      }
    }
  }

  skipIfTextureFormatNotUsableAsStorageTexture(...formats: (GPUTextureFormat | undefined)[]) {
    for (const format of formats) {
      if (format && !isTextureFormatUsableAsStorageFormat(format, this.isCompatibility)) {
        this.skip(`Texture with ${format} is not usable as a storage texture`);
      }
    }
  }

  /** Skips this test case if the `langFeature` is *not* supported. */
  skipIfLanguageFeatureNotSupported(langFeature: WGSLLanguageFeature) {
    if (!this.hasLanguageFeature(langFeature)) {
      this.skip(`WGSL language feature '${langFeature}' is not supported`);
    }
  }

  /** Skips this test case if the `langFeature` is supported. */
  skipIfLanguageFeatureSupported(langFeature: WGSLLanguageFeature) {
    if (this.hasLanguageFeature(langFeature)) {
      this.skip(`WGSL language feature '${langFeature}' is supported`);
    }
  }

  /** returns true iff the `langFeature` is supported  */
  hasLanguageFeature(langFeature: WGSLLanguageFeature) {
    const lf = getGPU(this.rec).wgslLanguageFeatures;
    return lf !== undefined && lf.has(langFeature);
  }

  /**
   * Expect a GPUBuffer's contents to pass the provided check.
   *
   * A library of checks can be found in {@link webgpu/util/check_contents}.
   */
  expectGPUBufferValuesPassCheck<T extends TypedArrayBufferView>(
    src: GPUBuffer,
    check: (actual: T) => Error | undefined,
    {
      srcByteOffset = 0,
      type,
      typedLength,
      method = 'copy',
      mode = 'fail',
    }: {
      srcByteOffset?: number;
      type: TypedArrayBufferViewConstructor<T>;
      typedLength: number;
      method?: 'copy' | 'map';
      mode?: 'fail' | 'warn';
    }
  ) {
    const readbackPromise = this.readGPUBufferRangeTyped(src, {
      srcByteOffset,
      type,
      typedLength,
      method,
    });
    this.eventualAsyncExpectation(async niceStack => {
      const readback = await readbackPromise;
      this.expectOK(check(readback.data), { mode, niceStack });
      readback.cleanup();
    });
  }

  /**
   * Expect a GPUBuffer's contents to equal the values in the provided TypedArray.
   */
  expectGPUBufferValuesEqual(
    src: GPUBuffer,
    expected: TypedArrayBufferView,
    srcByteOffset: number = 0,
    { method = 'copy', mode = 'fail' }: { method?: 'copy' | 'map'; mode?: 'fail' | 'warn' } = {}
  ): void {
    this.expectGPUBufferValuesPassCheck(src, a => checkElementsEqual(a, expected), {
      srcByteOffset,
      type: expected.constructor as TypedArrayBufferViewConstructor,
      typedLength: expected.length,
      method,
      mode,
    });
  }

  /**
   * Expect a buffer to consist exclusively of rows of some repeated expected value. The size of
   * `expectedValue` must be 1, 2, or any multiple of 4 bytes. Rows in the buffer are expected to be
   * zero-padded out to `bytesPerRow`. `minBytesPerRow` is the number of bytes per row that contain
   * actual (non-padding) data and must be an exact multiple of the byte-length of `expectedValue`.
   */
  expectGPUBufferRepeatsSingleValue(
    buffer: GPUBuffer,
    {
      expectedValue,
      numRows,
      minBytesPerRow,
      bytesPerRow,
    }: {
      expectedValue: ArrayBuffer;
      numRows: number;
      minBytesPerRow: number;
      bytesPerRow: number;
    }
  ) {
    const valueSize = expectedValue.byteLength;
    assert(valueSize === 1 || valueSize === 2 || valueSize % 4 === 0);
    assert(minBytesPerRow % valueSize === 0);
    assert(bytesPerRow % 4 === 0);

    // If the buffer is small enough, just generate the full expected buffer contents and check
    // against them on the CPU.
    const kMaxBufferSizeToCheckOnCpu = 256 * 1024;
    const bufferSize = bytesPerRow * (numRows - 1) + minBytesPerRow;
    if (bufferSize <= kMaxBufferSizeToCheckOnCpu) {
      const valueBytes = Array.from(new Uint8Array(expectedValue));
      const rowValues = new Array(minBytesPerRow / valueSize).fill(valueBytes);
      const rowBytes = new Uint8Array([].concat(...rowValues));
      const expectedContents = new Uint8Array(bufferSize);
      range(numRows, row => expectedContents.set(rowBytes, row * bytesPerRow));
      this.expectGPUBufferValuesEqual(buffer, expectedContents);
      return;
    }

    // Copy into a buffer suitable for STORAGE usage.
    const storageBuffer = this.createBufferTracked({
      size: bufferSize,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });

    // This buffer conveys the data we expect to see for a single value read. Since we read 32 bits at
    // a time, for values smaller than 32 bits we pad this expectation with repeated value data, or
    // with zeroes if the width of a row in the buffer is less than 4 bytes. For value sizes larger
    // than 32 bits, we assume they're a multiple of 32 bits and expect to read exact matches of
    // `expectedValue` as-is.
    const expectedDataSize = Math.max(4, valueSize);
    const expectedDataBuffer = this.createBufferTracked({
      size: expectedDataSize,
      usage: GPUBufferUsage.STORAGE,
      mappedAtCreation: true,
    });
    const expectedData = new Uint32Array(expectedDataBuffer.getMappedRange());
    if (valueSize === 1) {
      const value = new Uint8Array(expectedValue)[0];
      const values = new Array(Math.min(4, minBytesPerRow)).fill(value);
      const padding = new Array(Math.max(0, 4 - values.length)).fill(0);
      const expectedBytes = new Uint8Array(expectedData.buffer);
      expectedBytes.set([...values, ...padding]);
    } else if (valueSize === 2) {
      const value = new Uint16Array(expectedValue)[0];
      const expectedWords = new Uint16Array(expectedData.buffer);
      expectedWords.set([value, minBytesPerRow > 2 ? value : 0]);
    } else {
      expectedData.set(new Uint32Array(expectedValue));
    }
    expectedDataBuffer.unmap();

    // The output buffer has one 32-bit entry per buffer row. An entry's value will be 1 if every
    // read from the corresponding row matches the expected data derived above, or 0 otherwise.
    const resultBuffer = this.createBufferTracked({
      size: numRows * 4,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC,
    });

    const readsPerRow = Math.ceil(minBytesPerRow / expectedDataSize);
    const reducer = `
    struct Buffer { data: array<u32>, };
    @group(0) @binding(0) var<storage, read> expected: Buffer;
    @group(0) @binding(1) var<storage, read> in: Buffer;
    @group(0) @binding(2) var<storage, read_write> out: Buffer;
    @compute @workgroup_size(1) fn reduce(
        @builtin(global_invocation_id) id: vec3<u32>) {
      let rowBaseIndex = id.x * ${bytesPerRow / 4}u;
      let readSize = ${expectedDataSize / 4}u;
      out.data[id.x] = 1u;
      for (var i: u32 = 0u; i < ${readsPerRow}u; i = i + 1u) {
        let elementBaseIndex = rowBaseIndex + i * readSize;
        for (var j: u32 = 0u; j < readSize; j = j + 1u) {
          if (in.data[elementBaseIndex + j] != expected.data[j]) {
            out.data[id.x] = 0u;
            return;
          }
        }
      }
    }
    `;

    const pipeline = this.device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: this.device.createShaderModule({ code: reducer }),
        entryPoint: 'reduce',
      },
    });

    const bindGroup = this.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: expectedDataBuffer } },
        { binding: 1, resource: { buffer: storageBuffer } },
        { binding: 2, resource: { buffer: resultBuffer } },
      ],
    });

    const commandEncoder = this.device.createCommandEncoder();
    commandEncoder.copyBufferToBuffer(buffer, 0, storageBuffer, 0, bufferSize);
    const pass = commandEncoder.beginComputePass();
    pass.setPipeline(pipeline);
    pass.setBindGroup(0, bindGroup);
    pass.dispatchWorkgroups(numRows);
    pass.end();
    this.device.queue.submit([commandEncoder.finish()]);

    const expectedResults = new Array(numRows).fill(1);
    this.expectGPUBufferValuesEqual(resultBuffer, new Uint32Array(expectedResults));
  }

  // MAINTENANCE_TODO: add an expectContents for textures, which logs data: uris on failure

  /**
   * Expect an entire GPUTexture to have a single color at the given mip level (defaults to 0).
   * MAINTENANCE_TODO: Remove this and/or replace it with a helper in TextureTestMixin.
   */
  expectSingleColor(
    src: GPUTexture,
    format: GPUTextureFormat,
    {
      size,
      exp,
      dimension = '2d',
      slice = 0,
      layout,
    }: {
      size: [number, number, number];
      exp: PerTexelComponent<number>;
      dimension?: GPUTextureDimension;
      slice?: number;
      layout?: TextureLayoutOptions;
    }
  ): void {
    assert(
      slice === 0 || dimension === '2d',
      'texture slices are only implemented for 2d textures'
    );

    format = resolvePerAspectFormat(format, layout?.aspect);
    const { byteLength, minBytesPerRow, bytesPerRow, rowsPerImage, mipSize } = getTextureCopyLayout(
      format,
      dimension,
      size,
      layout
    );
    // MAINTENANCE_TODO: getTextureCopyLayout does not return the proper size for array textures,
    // i.e. it will leave the z/depth value as is instead of making it 1 when dealing with 2d
    // texture arrays. Since we are passing in the dimension, we should update it to return the
    // corrected size.
    const copySize = [
      mipSize[0],
      dimension !== '1d' ? mipSize[1] : 1,
      dimension === '3d' ? mipSize[2] : 1,
    ];

    const rep = kTexelRepresentationInfo[format as EncodableTextureFormat];
    const expectedTexelData = rep.pack(rep.encode(exp));

    const buffer = this.createBufferTracked({
      size: byteLength,
      usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST,
    });

    const commandEncoder = this.device.createCommandEncoder();
    commandEncoder.copyTextureToBuffer(
      {
        texture: src,
        mipLevel: layout?.mipLevel,
        origin: { x: 0, y: 0, z: slice },
        aspect: layout?.aspect,
      },
      { buffer, bytesPerRow, rowsPerImage },
      copySize
    );
    this.queue.submit([commandEncoder.finish()]);

    this.expectGPUBufferRepeatsSingleValue(buffer, {
      expectedValue: expectedTexelData,
      numRows: rowsPerImage * copySize[2],
      minBytesPerRow,
      bytesPerRow,
    });
  }

  /**
   * Return a GPUBuffer that data are going to be written into.
   * MAINTENANCE_TODO: Remove this once expectSinglePixelBetweenTwoValuesIn2DTexture is removed.
   */
  private readSinglePixelFrom2DTexture(
    src: GPUTexture,
    format: SizedTextureFormat,
    { x, y }: { x: number; y: number },
    { slice = 0, layout }: { slice?: number; layout?: TextureLayoutOptions }
  ): GPUBuffer {
    const { byteLength, bytesPerRow, rowsPerImage } = getTextureSubCopyLayout(
      format,
      [1, 1],
      layout
    );
    const buffer = this.createBufferTracked({
      size: byteLength,
      usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST,
    });

    const commandEncoder = this.device.createCommandEncoder();
    commandEncoder.copyTextureToBuffer(
      { texture: src, mipLevel: layout?.mipLevel, origin: { x, y, z: slice } },
      { buffer, bytesPerRow, rowsPerImage },
      [1, 1]
    );
    this.queue.submit([commandEncoder.finish()]);

    return buffer;
  }

  /**
   * Take a single pixel of a 2D texture, interpret it using a TypedArray of the `expected` type,
   * and expect each value in that array to be between the corresponding "expected" values
   * (either `a[i] <= actual[i] <= b[i]` or `a[i] >= actual[i] => b[i]`).
   * MAINTENANCE_TODO: Remove this once there is a way to deal with undefined lerp-ed values.
   */
  expectSinglePixelBetweenTwoValuesIn2DTexture(
    src: GPUTexture,
    format: SizedTextureFormat,
    { x, y }: { x: number; y: number },
    {
      exp,
      slice = 0,
      layout,
      generateWarningOnly = false,
      checkElementsBetweenFn = (act, [a, b]) =>
        checkElementsBetween(act, [i => a[i] as number, i => b[i] as number]),
    }: {
      exp: [TypedArrayBufferView, TypedArrayBufferView];
      slice?: number;
      layout?: TextureLayoutOptions;
      generateWarningOnly?: boolean;
      checkElementsBetweenFn?: (
        actual: TypedArrayBufferView,
        expected: readonly [TypedArrayBufferView, TypedArrayBufferView]
      ) => Error | undefined;
    }
  ): void {
    assert(exp[0].constructor === exp[1].constructor);
    const constructor = exp[0].constructor as TypedArrayBufferViewConstructor;
    assert(exp[0].length === exp[1].length);
    const typedLength = exp[0].length;

    const buffer = this.readSinglePixelFrom2DTexture(src, format, { x, y }, { slice, layout });
    this.expectGPUBufferValuesPassCheck(buffer, a => checkElementsBetweenFn(a, exp), {
      type: constructor,
      typedLength,
      mode: generateWarningOnly ? 'warn' : 'fail',
    });
  }

  /**
   * Emulate a texture to buffer copy by using a compute shader
   * to load texture values of a subregion of a 2d texture and write to a storage buffer.
   * For sample count == 1, the buffer contains extent[0] * extent[1] of the sample.
   * For sample count > 1, the buffer contains extent[0] * extent[1] * (N = sampleCount) values sorted
   * in the order of their sample index [0, sampleCount - 1]
   *
   * This can be useful when the texture to buffer copy is not available to the texture format
   * e.g. (depth24plus), or when the texture is multisampled.
   *
   * MAINTENANCE_TODO: extend texture dimension to 1d and 3d.
   *
   * @returns storage buffer containing the copied value from the texture.
   */
  copy2DTextureToBufferUsingComputePass(
    type: ScalarType,
    componentCount: number,
    textureView: GPUTextureView,
    sampleCount: number = 1,
    extent_: GPUExtent3D = [1, 1, 1],
    origin_: GPUOrigin3D = [0, 0, 0]
  ): GPUBuffer {
    const origin = reifyOrigin3D(origin_);
    const extent = reifyExtent3D(extent_);
    const width = extent.width;
    const height = extent.height;
    const kWorkgroupSizeX = 8;
    const kWorkgroupSizeY = 8;
    const textureSrcCode =
      sampleCount === 1
        ? `@group(0) @binding(0) var src: texture_2d<${type}>;`
        : `@group(0) @binding(0) var src: texture_multisampled_2d<${type}>;`;
    const code = `
      struct Buffer {
        data: array<${type}>,
      };

      ${textureSrcCode}
      @group(0) @binding(1) var<storage, read_write> dst : Buffer;

      struct Params {
        origin: vec2u,
        extent: vec2u,
      };
      @group(0) @binding(2) var<uniform> params : Params;

      @compute @workgroup_size(${kWorkgroupSizeX}, ${kWorkgroupSizeY}, 1) fn main(@builtin(global_invocation_id) id : vec3u) {
        let boundary = params.origin + params.extent;
        let coord = params.origin + id.xy;
        if (any(coord >= boundary)) {
          return;
        }
        let offset = (id.x + id.y * params.extent.x) * ${componentCount} * ${sampleCount};
        for (var sampleIndex = 0u; sampleIndex < ${sampleCount};
          sampleIndex = sampleIndex + 1) {
          let o = offset + sampleIndex * ${componentCount};
          let v = textureLoad(src, coord.xy, sampleIndex);
          for (var component = 0u; component < ${componentCount}; component = component + 1) {
            dst.data[o + component] = v[component];
          }
        }
      }
    `;
    const computePipeline = this.device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: this.device.createShaderModule({
          code,
        }),
        entryPoint: 'main',
      },
    });

    const storageBuffer = this.createBufferTracked({
      size: sampleCount * type.size * componentCount * width * height,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
    });

    const uniformBuffer = this.makeBufferWithContents(
      new Uint32Array([origin.x, origin.y, width, height]),
      GPUBufferUsage.UNIFORM
    );

    const uniformBindGroup = this.device.createBindGroup({
      layout: computePipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: textureView,
        },
        {
          binding: 1,
          resource: {
            buffer: storageBuffer,
          },
        },
        {
          binding: 2,
          resource: {
            buffer: uniformBuffer,
          },
        },
      ],
    });

    const encoder = this.device.createCommandEncoder();
    const pass = encoder.beginComputePass();
    pass.setPipeline(computePipeline);
    pass.setBindGroup(0, uniformBindGroup);
    pass.dispatchWorkgroups(
      Math.floor((width + kWorkgroupSizeX - 1) / kWorkgroupSizeX),
      Math.floor((height + kWorkgroupSizeY - 1) / kWorkgroupSizeY),
      1
    );
    pass.end();
    this.device.queue.submit([encoder.finish()]);

    return storageBuffer;
  }

  /**
   * Expect the specified WebGPU error to be generated when running the provided function.
   */
  expectGPUError<R>(filter: GPUErrorFilter, fn: () => R, shouldError: boolean = true): R {
    // If no error is expected, we let the scope surrounding the test catch it.
    if (!shouldError) {
      return fn();
    }

    this.device.pushErrorScope(filter);
    const returnValue = fn();
    const promise = this.device.popErrorScope();

    this.eventualAsyncExpectation(async niceStack => {
      const error = await promise;

      let failed = false;
      switch (filter) {
        case 'out-of-memory':
          failed = !(error instanceof GPUOutOfMemoryError);
          break;
        case 'validation':
          failed = !(error instanceof GPUValidationError);
          break;
      }

      if (failed) {
        niceStack.message = `Expected ${filter} error`;
        this.rec.expectationFailed(niceStack);
      } else {
        niceStack.message = `Captured ${filter} error`;
        if (error instanceof GPUValidationError) {
          niceStack.message += ` - ${error.message}`;
        }
        this.rec.debug(niceStack);
      }
    });

    return returnValue;
  }

  /**
   * Expect a validation error inside the callback.
   *
   * Tests should always do just one WebGPU call in the callback, to make sure that's what's tested.
   */
  expectValidationError(fn: () => void, shouldError: boolean = true): void {
    // If no error is expected, we let the scope surrounding the test catch it.
    if (shouldError) {
      this.device.pushErrorScope('validation');
    }

    // Note: A return value is not allowed for the callback function. This is to avoid confusion
    // about what the actual behavior would be; either of the following could be reasonable:
    //   - Make expectValidationError async, and have it await on fn(). This causes an async split
    //     between pushErrorScope and popErrorScope, so if the caller doesn't `await` on
    //     expectValidationError (either accidentally or because it doesn't care to do so), then
    //     other test code will be (nondeterministically) caught by the error scope.
    //   - Make expectValidationError NOT await fn(), but just execute its first block (until the
    //     first await) and return the return value (a Promise). This would be confusing because it
    //     would look like the error scope includes the whole async function, but doesn't.
    // If we do decide we need to return a value, we should use the latter semantic.
    const returnValue = fn() as unknown;
    assert(
      returnValue === undefined,
      'expectValidationError callback should not return a value (or be async)'
    );

    if (shouldError) {
      const promise = this.device.popErrorScope();

      this.eventualAsyncExpectation(async niceStack => {
        const gpuValidationError = await promise;
        if (!gpuValidationError) {
          niceStack.message = 'Validation succeeded unexpectedly.';
          this.rec.validationFailed(niceStack);
        } else if (gpuValidationError instanceof GPUValidationError) {
          niceStack.message = `Validation failed, as expected - ${gpuValidationError.message}`;
          this.rec.debug(niceStack);
        }
      });
    }
  }

  /** Create a GPUBuffer and track it for cleanup at the end of the test. */
  createBufferTracked(descriptor: GPUBufferDescriptor): GPUBuffer {
    return this.trackForCleanup(this.device.createBuffer(descriptor));
  }

  /** Create a GPUTexture and track it for cleanup at the end of the test. */
  createTextureTracked(descriptor: GPUTextureDescriptor): GPUTexture {
    return this.trackForCleanup(this.device.createTexture(descriptor));
  }

  /** Create a GPUQuerySet and track it for cleanup at the end of the test. */
  createQuerySetTracked(descriptor: GPUQuerySetDescriptor): GPUQuerySet {
    return this.trackForCleanup(this.device.createQuerySet(descriptor));
  }

  /**
   * Creates a buffer with the contents of some TypedArray.
   * The buffer size will always be aligned to 4 as we set mappedAtCreation === true when creating the
   * buffer.
   *
   * MAINTENANCE_TODO: Several call sites would be simplified if this took ArrayBuffer as well.
   */
  makeBufferWithContents(dataArray: TypedArrayBufferView, usage: GPUBufferUsageFlags): GPUBuffer {
    const buffer = this.createBufferTracked({
      mappedAtCreation: true,
      size: align(dataArray.byteLength, 4),
      usage,
    });
    memcpy({ src: dataArray }, { dst: buffer.getMappedRange() });
    buffer.unmap();
    return buffer;
  }

  /**
   * Returns a GPUCommandEncoder, GPUComputePassEncoder, GPURenderPassEncoder, or
   * GPURenderBundleEncoder, and a `finish` method returning a GPUCommandBuffer.
   * Allows testing methods which have the same signature across multiple encoder interfaces.
   *
   * @example
   * ```
   * g.test('popDebugGroup')
   *   .params(u => u.combine('encoderType', kEncoderTypes))
   *   .fn(t => {
   *     const { encoder, finish } = t.createEncoder(t.params.encoderType);
   *     encoder.popDebugGroup();
   *   });
   *
   * g.test('writeTimestamp')
   *   .params(u => u.combine('encoderType', ['non-pass', 'compute pass', 'render pass'] as const)
   *   .fn(t => {
   *     const { encoder, finish } = t.createEncoder(t.params.encoderType);
   *     // Encoder type is inferred, so `writeTimestamp` can be used even though it doesn't exist
   *     // on GPURenderBundleEncoder.
   *     encoder.writeTimestamp(args);
   *   });
   * ```
   */
  createEncoder<T extends EncoderType>(
    encoderType: T,
    {
      attachmentInfo,
      occlusionQuerySet,
      targets,
    }: {
      attachmentInfo?: GPURenderBundleEncoderDescriptor;
      occlusionQuerySet?: GPUQuerySet;
      targets?: GPUTextureView[];
    } = {}
  ): CommandBufferMaker<T> {
    const fullAttachmentInfo = {
      // Defaults if not overridden:
      colorFormats: ['rgba8unorm'],
      sampleCount: 1,
      // Passed values take precedent.
      ...attachmentInfo,
    } as const;

    switch (encoderType) {
      case 'non-pass': {
        const encoder = this.device.createCommandEncoder();

        return new CommandBufferMaker(this, encoder, () => {
          return encoder.finish();
        });
      }
      case 'render bundle': {
        const device = this.device;
        const rbEncoder = device.createRenderBundleEncoder(fullAttachmentInfo);
        const pass = this.createEncoder('render pass', { attachmentInfo, targets });

        return new CommandBufferMaker(this, rbEncoder, () => {
          pass.encoder.executeBundles([rbEncoder.finish()]);
          return pass.finish();
        });
      }
      case 'compute pass': {
        const commandEncoder = this.device.createCommandEncoder();
        const encoder = commandEncoder.beginComputePass();

        return new CommandBufferMaker(this, encoder, () => {
          encoder.end();
          return commandEncoder.finish();
        });
      }
      case 'render pass': {
        const makeAttachmentView = (format: GPUTextureFormat) =>
          this.createTextureTracked({
            size: [16, 16, 1],
            format,
            usage: GPUTextureUsage.RENDER_ATTACHMENT,
            sampleCount: fullAttachmentInfo.sampleCount,
          }).createView();

        let depthStencilAttachment: GPURenderPassDepthStencilAttachment | undefined = undefined;
        if (fullAttachmentInfo.depthStencilFormat !== undefined) {
          depthStencilAttachment = {
            view: makeAttachmentView(fullAttachmentInfo.depthStencilFormat),
            depthReadOnly: fullAttachmentInfo.depthReadOnly,
            stencilReadOnly: fullAttachmentInfo.stencilReadOnly,
          };
          if (
            kTextureFormatInfo[fullAttachmentInfo.depthStencilFormat].depth &&
            !fullAttachmentInfo.depthReadOnly
          ) {
            depthStencilAttachment.depthClearValue = 0;
            depthStencilAttachment.depthLoadOp = 'clear';
            depthStencilAttachment.depthStoreOp = 'discard';
          }
          if (
            kTextureFormatInfo[fullAttachmentInfo.depthStencilFormat].stencil &&
            !fullAttachmentInfo.stencilReadOnly
          ) {
            depthStencilAttachment.stencilClearValue = 1;
            depthStencilAttachment.stencilLoadOp = 'clear';
            depthStencilAttachment.stencilStoreOp = 'discard';
          }
        }
        const passDesc: GPURenderPassDescriptor = {
          colorAttachments: Array.from(fullAttachmentInfo.colorFormats, (format, i) =>
            format
              ? {
                  view: targets ? targets[i] : makeAttachmentView(format),
                  clearValue: [0, 0, 0, 0],
                  loadOp: 'clear',
                  storeOp: 'store',
                }
              : null
          ),
          depthStencilAttachment,
          occlusionQuerySet,
        };

        const commandEncoder = this.device.createCommandEncoder();
        const encoder = commandEncoder.beginRenderPass(passDesc);
        return new CommandBufferMaker(this, encoder, () => {
          encoder.end();
          return commandEncoder.finish();
        });
      }
    }
    unreachable();
  }
}

/**
 * Fixture for WebGPU tests that uses a DeviceProvider
 */
export class GPUTest extends GPUTestBase {
  // Should never be undefined in a test. If it is, init() must not have run/finished.
  private provider: DeviceProvider | undefined;
  private mismatchedProvider: DeviceProvider | undefined;

  override async init() {
    await super.init();

    this.provider = await this.sharedState.acquireProvider();
    this.mismatchedProvider = await this.sharedState.acquireMismatchedProvider();
  }

  /** GPUAdapter that the device was created from. */
  get adapter(): GPUAdapter {
    assert(this.provider !== undefined, 'internal error: DeviceProvider missing');
    return this.provider.adapter;
  }

  /**
   * GPUDevice for the test to use.
   */
  override get device(): GPUDevice {
    assert(this.provider !== undefined, 'internal error: DeviceProvider missing');
    return this.provider.device;
  }

  /**
   * GPUDevice for tests requiring a second device different from the default one,
   * e.g. for creating objects for by device_mismatch validation tests.
   */
  get mismatchedDevice(): GPUDevice {
    assert(
      this.mismatchedProvider !== undefined,
      'selectMismatchedDeviceOrSkipTestCase was not called in beforeAllSubcases'
    );
    return this.mismatchedProvider.device;
  }

  /**
   * Expects that the device should be lost for a particular reason at the teardown of the test.
   */
  expectDeviceLost(reason: GPUDeviceLostReason): void {
    assert(this.provider !== undefined, 'internal error: GPUDevice missing?');
    this.provider.expectDeviceLost(reason);
  }
}

/**
 * Gets the adapter limits as a standard JavaScript object.
 */
function getAdapterLimitsAsDeviceRequiredLimits(adapter: GPUAdapter) {
  const requiredLimits: Record<string, GPUSize64> = {};
  const adapterLimits = adapter.limits as unknown as Record<string, GPUSize64>;
  for (const key in adapter.limits) {
    // MAINTENANCE_TODO: Remove this once minSubgroupSize is removed from
    // chromium.
    if (key === 'maxSubgroupSize' || key === 'minSubgroupSize') {
      continue;
    }
    requiredLimits[key] = adapterLimits[key];
  }
  return requiredLimits;
}

/**
 * Removes limits that don't exist on the adapter.
 * A test might request a new limit that not all implementions support. The test itself
 * should check the requested limit using code that expects undefined.
 *
 * ```ts
 *    t.skipIf(limit < 2);     // BAD! Doesn't skip if unsupported beause undefined is never less than 2.
 *    t.skipIf(!(limit >= 2)); // Good. Skips if limits is not >= 2. undefined is not >= 2.
 * ```
 */
function removeNonExistentLimits(adapter: GPUAdapter, limits: Record<string, GPUSize64>) {
  const filteredLimits: Record<string, GPUSize64> = {};
  const adapterLimits = adapter.limits as unknown as Record<string, GPUSize64>;
  for (const [limit, value] of Object.entries(limits)) {
    if (adapterLimits[limit] !== undefined) {
      filteredLimits[limit] = value;
    }
  }
  return filteredLimits;
}

function applyLimitsToDescriptor(
  adapter: GPUAdapter,
  desc: CanonicalDeviceDescriptor | undefined,
  getRequiredLimits: (adapter: GPUAdapter) => Record<string, number>
) {
  const descWithMaxLimits: CanonicalDeviceDescriptor = {
    requiredFeatures: [],
    defaultQueue: {},
    ...desc,
    requiredLimits: removeNonExistentLimits(adapter, getRequiredLimits(adapter)),
  };
  return descWithMaxLimits;
}

/**
 * Used by RequiredLimitsTestMixin to allow you to request specific limits
 *
 * Supply a `getRequiredLimits` function that given a GPUAdapter, turns the limits
 * you want.
 *
 * Also supply a key function that returns a device key. You should generally return
 * the name of each limit you request and any math you did on the limit. For example
 *
 * ```js
 * {
 *   getRequiredLimits(adapter) {
 *     return {
 *       maxBindGroups: adapter.limits.maxBindGroups / 2,
 *       maxTextureDimensions2D: Math.max(adapter.limits.maxTextureDimensions2D, 8192),
 *     },
 *   },
 *   key() {
 *     return `
 *       maxBindGroups / 2,
 *       max(maxTextureDimension2D, 8192),
 *     `;
 *   },
 * }
 * ```
 *
 * Its important to note, the key is used BEFORE knowing the adapter limits to get a device
 * that was already created with the same key.
 */
interface RequiredLimitsHelper {
  getRequiredLimits: (adapter: GPUAdapter) => Record<string, number>;
  key(): string;
}

/**
 * Used by RequiredLimitsTest to request a device with all requested limits of the adapter.
 */
export class RequiredLimitsGPUTestSubcaseBatchState extends GPUTestSubcaseBatchState {
  private requiredLimitsHelper: RequiredLimitsHelper;
  constructor(
    protected override readonly recorder: TestCaseRecorder,
    public override readonly params: TestParams,
    requiredLimitsHelper: RequiredLimitsHelper
  ) {
    super(recorder, params);
    this.requiredLimitsHelper = requiredLimitsHelper;
  }
  override requestDeviceWithRequiredParametersOrSkip(
    descriptor: DeviceSelectionDescriptor,
    descriptorModifier?: DescriptorModifier
  ): void {
    const requiredLimitsHelper = this.requiredLimitsHelper;
    const mod: DescriptorModifier = {
      descriptorModifier(adapter: GPUAdapter, desc: CanonicalDeviceDescriptor | undefined) {
        desc = descriptorModifier?.descriptorModifier
          ? descriptorModifier.descriptorModifier(adapter, desc)
          : desc;
        return applyLimitsToDescriptor(adapter, desc, requiredLimitsHelper.getRequiredLimits);
      },
      keyModifier(baseKey: string) {
        return `${baseKey}:${requiredLimitsHelper.key()}`;
      },
    };
    super.requestDeviceWithRequiredParametersOrSkip(
      initUncanonicalizedDeviceDescriptor(descriptor),
      mod
    );
  }
}

export type RequiredLimitsTestMixinType = {
  // placeholder. Change to an interface if we need MaxLimits specific methods.
};

/**
 * A text mixin to make it relatively easy to request specific limits.
 */
export function RequiredLimitsTestMixin<F extends FixtureClass<GPUTestBase>>(
  Base: F,
  requiredLimitsHelper: RequiredLimitsHelper
): FixtureClassWithMixin<F, RequiredLimitsTestMixinType> {
  class RequiredLimitsImpl
    extends (Base as FixtureClassInterface<GPUTestBase>)
    implements RequiredLimitsTestMixinType
  {
    //
    public static override MakeSharedState(
      recorder: TestCaseRecorder,
      params: TestParams
    ): GPUTestSubcaseBatchState {
      return new RequiredLimitsGPUTestSubcaseBatchState(recorder, params, requiredLimitsHelper);
    }
  }

  return RequiredLimitsImpl as unknown as FixtureClassWithMixin<F, RequiredLimitsTestMixinType>;
}

/**
 * Requests all the max limits from the adapter.
 */
export function MaxLimitsTestMixin<F extends FixtureClass<GPUTestBase>>(Base: F) {
  return RequiredLimitsTestMixin(Base, {
    getRequiredLimits: getAdapterLimitsAsDeviceRequiredLimits,
    key() {
      return 'AllLimits';
    },
  });
}

/**
 * Texture expectation mixin can be applied on top of GPUTest to add texture
 * related expectation helpers.
 */
export interface TextureTestMixinType {
  /**
   * Creates a 1 mip level texture with the contents of a TexelView and tracks
   * it for destruction for the test case.
   */
  createTextureFromTexelView(
    texelView: TexelView,
    desc: Omit<GPUTextureDescriptor, 'format'>
  ): GPUTexture;

  /**
   * Creates a mipmapped texture where each mipmap level's (`i`) content is
   * from `texelViews[i]` and tracks it for destruction for the test case.
   */
  createTextureFromTexelViewsMultipleMipmaps(
    texelViews: TexelView[],
    desc: Omit<GPUTextureDescriptor, 'format'>
  ): GPUTexture;

  /**
   * Expects that comparing the subrect (defined via `size`) of a GPUTexture
   * to the expected TexelView passes without error.
   */
  expectTexelViewComparisonIsOkInTexture(
    src: GPUTexelCopyTextureInfo,
    exp: TexelView,
    size: GPUExtent3D,
    comparisonOptions?: TexelCompareOptions
  ): void;

  /**
   * Expects that a sparse set of pixels in the GPUTexture passes comparison against
   * their expected colors without error.
   */
  expectSinglePixelComparisonsAreOkInTexture<E extends PixelExpectation>(
    src: GPUTexelCopyTextureInfo,
    exp: PerPixelComparison<E>[],
    comparisonOptions?: TexelCompareOptions
  ): void;

  /**
   * Renders the 2 given textures to an rgba8unorm texture at the size of the
   * specified mipLevel, each time reading the contents of the result.
   * Expects contents of both renders to match. Also expects contents described
   * by origin and size to not be a constant value so as to make sure something
   * interesting was actually compared.
   *
   * The point of this function is to compare compressed texture contents in
   * compatibility mode. `copyTextureToBuffer` does not work for compressed
   * textures in compatibility mode so instead, we pass 2 compressed texture
   * to this function. Each one will be rendered to an `rgba8unorm` texture,
   * the results of that `rgba8unorm` texture read via `copyTextureToBuffer`,
   * and then results compared. This indirectly lets us compare the contents
   * of the 2 compressed textures.
   *
   * Code calling this function would generate the textures where the
   * `actualTexture` is generated calling `writeTexture`, `copyBufferToTexture`
   * or `copyTextureToTexture` and `expectedTexture`'s data is generated entirely
   * on the CPU in such a way that its content should match whatever process
   * was used to generate `actualTexture`. Often this involves calling
   * `updateLinearTextureDataSubBox`
   */
  expectTexturesToMatchByRendering(
    actualTexture: GPUTexture,
    expectedTexture: GPUTexture,
    mipLevel: number,
    origin: Required<GPUOrigin3DDict>,
    size: Required<GPUExtent3DDict>
  ): void;

  /**
   * Copies an entire texture's mipLevel to a buffer
   */
  copyWholeTextureToNewBufferSimple(texture: GPUTexture, mipLevel: number): GPUBuffer;

  /**
   * Copies an texture's mipLevel to a buffer
   * The size of the buffer is specified by `byteLength`
   */
  copyWholeTextureToNewBuffer(
    { texture, mipLevel }: { texture: GPUTexture; mipLevel: number | undefined },
    resultDataLayout: {
      bytesPerBlock: number;
      byteLength: number;
      bytesPerRow: number;
      rowsPerImage: number;
      mipSize: [number, number, number];
    }
  ): GPUBuffer;

  /**
   * Updates a Uint8Array with a cubic portion of data from another Uint8Array.
   * Effectively it's a Uint8Array to Uint8Array copy that
   * does the same thing as `writeTexture` but because the
   * destination is a buffer you have to provide the parameters
   * of the destination buffer similarly to how you'd provide them
   * to `copyTextureToBuffer`
   */
  updateLinearTextureDataSubBox(
    format: ColorTextureFormat,
    copySize: Required<GPUExtent3DDict>,
    copyParams: {
      dest: LinearCopyParameters;
      src: LinearCopyParameters;
    }
  ): void;

  /**
   * Gets a byte offset to a texel
   */
  getTexelOffsetInBytes(
    textureDataLayout: Required<GPUTexelCopyBufferLayout>,
    format: ColorTextureFormat,
    texel: Required<GPUOrigin3DDict>,
    origin?: Required<GPUOrigin3DDict>
  ): number;

  iterateBlockRows(
    size: Required<GPUExtent3DDict>,
    format: ColorTextureFormat
  ): Generator<Required<GPUOrigin3DDict>>;
}

type PipelineType = '2d' | '2d-array';

type ImageCopyTestResources = {
  pipelineByPipelineType: Map<PipelineType, GPURenderPipeline>;
};

const s_deviceToResourcesMap = new WeakMap<GPUDevice, ImageCopyTestResources>();

/**
 * Gets a (cached) pipeline to render a texture to an rgba8unorm texture
 */
function getPipelineToRenderTextureToRGB8UnormTexture(
  device: GPUDevice,
  texture: GPUTexture,
  isCompatibility: boolean
) {
  if (!s_deviceToResourcesMap.has(device)) {
    s_deviceToResourcesMap.set(device, {
      pipelineByPipelineType: new Map<PipelineType, GPURenderPipeline>(),
    });
  }

  const { pipelineByPipelineType } = s_deviceToResourcesMap.get(device)!;
  const pipelineType: PipelineType =
    isCompatibility && texture.depthOrArrayLayers > 1 ? '2d-array' : '2d';
  if (!pipelineByPipelineType.get(pipelineType)) {
    const [textureType, layerCode] =
      pipelineType === '2d' ? ['texture_2d', ''] : ['texture_2d_array', ', uni.baseArrayLayer'];
    const module = device.createShaderModule({
      code: `
        struct VSOutput {
          @builtin(position) position: vec4f,
          @location(0) texcoord: vec2f,
        };

        struct Uniforms {
          baseArrayLayer: u32,
        };

        @vertex fn vs(
          @builtin(vertex_index) vertexIndex : u32
        ) -> VSOutput {
            let pos = array(
               vec2f(-1, -1),
               vec2f(-1,  3),
               vec2f( 3, -1),
            );

            var vsOutput: VSOutput;

            let xy = pos[vertexIndex];

            vsOutput.position = vec4f(xy, 0.0, 1.0);
            vsOutput.texcoord = xy * vec2f(0.5, -0.5) + vec2f(0.5);

            return vsOutput;
         }

         @group(0) @binding(0) var ourSampler: sampler;
         @group(0) @binding(1) var ourTexture: ${textureType}<f32>;
         @group(0) @binding(2) var<uniform> uni: Uniforms;

         @fragment fn fs(fsInput: VSOutput) -> @location(0) vec4f {
            return textureSample(ourTexture, ourSampler, fsInput.texcoord${layerCode});
         }
      `,
    });
    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        module,
        entryPoint: 'vs',
      },
      fragment: {
        module,
        entryPoint: 'fs',
        targets: [{ format: 'rgba8unorm' }],
      },
    });
    pipelineByPipelineType.set(pipelineType, pipeline);
  }
  const pipeline = pipelineByPipelineType.get(pipelineType)!;
  return { pipelineType, pipeline };
}

type LinearCopyParameters = {
  dataLayout: Required<GPUTexelCopyBufferLayout>;
  origin: Required<GPUOrigin3DDict>;
  data: Uint8Array;
};

export function TextureTestMixin<F extends FixtureClass<GPUTestBase>>(
  Base: F
): FixtureClassWithMixin<F, TextureTestMixinType> {
  class TextureExpectations
    extends (Base as FixtureClassInterface<GPUTestBase>)
    implements TextureTestMixinType
  {
    /**
     * Creates a 1 mip level texture with the contents of a TexelView.
     */
    createTextureFromTexelView(
      texelView: TexelView,
      desc: Omit<GPUTextureDescriptor, 'format'>
    ): GPUTexture {
      return createTextureFromTexelViews(this, [texelView], desc);
    }

    createTextureFromTexelViewsMultipleMipmaps(
      texelViews: TexelView[],
      desc: Omit<GPUTextureDescriptor, 'format'>
    ): GPUTexture {
      return createTextureFromTexelViews(this, texelViews, desc);
    }

    expectTexelViewComparisonIsOkInTexture(
      src: GPUTexelCopyTextureInfo,
      exp: TexelView,
      size: GPUExtent3D,
      comparisonOptions = {
        maxIntDiff: 0,
        maxDiffULPsForNormFormat: 1,
        maxDiffULPsForFloatFormat: 1,
      }
    ): void {
      this.eventualExpectOK(
        textureContentIsOKByT2B(this, src, size, { expTexelView: exp }, comparisonOptions)
      );
    }

    expectSinglePixelComparisonsAreOkInTexture<E extends PixelExpectation>(
      src: GPUTexelCopyTextureInfo,
      exp: PerPixelComparison<E>[],
      comparisonOptions = {
        maxIntDiff: 0,
        maxDiffULPsForNormFormat: 1,
        maxDiffULPsForFloatFormat: 1,
      }
    ): void {
      assert(exp.length > 0, 'must specify at least one pixel comparison');
      assert(
        (kEncodableTextureFormats as GPUTextureFormat[]).includes(src.texture.format),
        () => `${src.texture.format} is not an encodable format`
      );
      const lowerCorner = [src.texture.width, src.texture.height, src.texture.depthOrArrayLayers];
      const upperCorner = [0, 0, 0];
      const expMap = new Map<string, E>();
      const coords: Required<GPUOrigin3DDict>[] = [];
      for (const e of exp) {
        const coord = reifyOrigin3D(e.coord);
        const coordKey = JSON.stringify(coord);
        coords.push(coord);

        // Compute the minimum sub-rect that encompasses all the pixel comparisons. The
        // `lowerCorner` will become the origin, and the `upperCorner` will be used to compute the
        // size.
        lowerCorner[0] = Math.min(lowerCorner[0], coord.x);
        lowerCorner[1] = Math.min(lowerCorner[1], coord.y);
        lowerCorner[2] = Math.min(lowerCorner[2], coord.z);
        upperCorner[0] = Math.max(upperCorner[0], coord.x);
        upperCorner[1] = Math.max(upperCorner[1], coord.y);
        upperCorner[2] = Math.max(upperCorner[2], coord.z);

        // Build a sparse map of the coordinates to the expected colors for the texel view.
        assert(
          !expMap.has(coordKey),
          () => `duplicate pixel expectation at coordinate (${coord.x},${coord.y},${coord.z})`
        );
        expMap.set(coordKey, e.exp);
      }
      const size: GPUExtent3D = [
        upperCorner[0] - lowerCorner[0] + 1,
        upperCorner[1] - lowerCorner[1] + 1,
        upperCorner[2] - lowerCorner[2] + 1,
      ];
      let expTexelView: TexelView;
      if (Symbol.iterator in exp[0].exp) {
        expTexelView = TexelView.fromTexelsAsBytes(
          src.texture.format as EncodableTextureFormat,
          coord => {
            const res = expMap.get(JSON.stringify(coord));
            assert(
              res !== undefined,
              () => `invalid coordinate (${coord.x},${coord.y},${coord.z}) in sparse texel view`
            );
            return res as Uint8Array;
          }
        );
      } else {
        expTexelView = TexelView.fromTexelsAsColors(
          src.texture.format as EncodableTextureFormat,
          coord => {
            const res = expMap.get(JSON.stringify(coord));
            assert(
              res !== undefined,
              () => `invalid coordinate (${coord.x},${coord.y},${coord.z}) in sparse texel view`
            );
            return res as PerTexelComponent<number>;
          }
        );
      }
      const coordsF = (function* () {
        for (const coord of coords) {
          yield coord;
        }
      })();

      this.eventualExpectOK(
        textureContentIsOKByT2B(
          this,
          { ...src, origin: reifyOrigin3D(lowerCorner) },
          size,
          { expTexelView },
          comparisonOptions,
          coordsF
        )
      );
    }

    expectTexturesToMatchByRendering(
      actualTexture: GPUTexture,
      expectedTexture: GPUTexture,
      mipLevel: number,
      origin: Required<GPUOrigin3DDict>,
      size: Required<GPUExtent3DDict>
    ): void {
      // Render every layer of both textures at mipLevel to an rgba8unorm texture
      // that matches the size of the mipLevel. After each render, copy the
      // result to a buffer and expect the results from both textures to match.
      const { pipelineType, pipeline } = getPipelineToRenderTextureToRGB8UnormTexture(
        this.device,
        actualTexture,
        this.isCompatibility
      );
      const readbackPromisesPerTexturePerLayer = [actualTexture, expectedTexture].map(
        (texture, ndx) => {
          const attachmentSize = virtualMipSize('2d', [texture.width, texture.height, 1], mipLevel);
          const attachment = this.createTextureTracked({
            label: `readback${ndx}`,
            size: attachmentSize,
            format: 'rgba8unorm',
            usage: GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT,
          });

          const sampler = this.device.createSampler();

          const numLayers = texture.depthOrArrayLayers;
          const readbackPromisesPerLayer = [];

          const uniformBuffer = this.createBufferTracked({
            size: 4,
            usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
          });

          for (let layer = 0; layer < numLayers; ++layer) {
            const viewDescriptor: GPUTextureViewDescriptor = {
              baseMipLevel: mipLevel,
              mipLevelCount: 1,
              ...(!this.isCompatibility && {
                baseArrayLayer: layer,
                arrayLayerCount: 1,
              }),
              dimension: pipelineType,
            };

            const bindGroup = this.device.createBindGroup({
              layout: pipeline.getBindGroupLayout(0),
              entries: [
                { binding: 0, resource: sampler },
                {
                  binding: 1,
                  resource: texture.createView(viewDescriptor),
                },
                ...(pipelineType === '2d-array'
                  ? [
                      {
                        binding: 2,
                        resource: { buffer: uniformBuffer },
                      },
                    ]
                  : []),
              ],
            });

            this.device.queue.writeBuffer(uniformBuffer, 0, new Uint32Array([layer]));

            const encoder = this.device.createCommandEncoder();
            const pass = encoder.beginRenderPass({
              colorAttachments: [
                {
                  view: attachment.createView(),
                  clearValue: [0.5, 0.5, 0.5, 0.5],
                  loadOp: 'clear',
                  storeOp: 'store',
                },
              ],
            });
            pass.setPipeline(pipeline);
            pass.setBindGroup(0, bindGroup);
            pass.draw(3);
            pass.end();
            this.queue.submit([encoder.finish()]);

            const buffer = this.copyWholeTextureToNewBufferSimple(attachment, 0);

            readbackPromisesPerLayer.push(
              this.readGPUBufferRangeTyped(buffer, {
                type: Uint8Array,
                typedLength: buffer.size,
              })
            );
          }
          return readbackPromisesPerLayer;
        }
      );

      this.eventualAsyncExpectation(async niceStack => {
        const readbacksPerTexturePerLayer = [];

        // Wait for all buffers to be ready
        for (const readbackPromises of readbackPromisesPerTexturePerLayer) {
          readbacksPerTexturePerLayer.push(await Promise.all(readbackPromises));
        }

        function arrayNotAllTheSameValue(arr: TypedArrayBufferView | number[], msg?: string) {
          const first = arr[0];
          return arr.length <= 1 || arr.findIndex(v => v !== first) >= 0
            ? undefined
            : Error(`array is entirely ${first} so likely nothing was tested: ${msg || ''}`);
        }

        // Compare each layer of each texture as read from buffer.
        const [actualReadbacksPerLayer, expectedReadbacksPerLayer] = readbacksPerTexturePerLayer;
        for (let layer = 0; layer < actualReadbacksPerLayer.length; ++layer) {
          const actualReadback = actualReadbacksPerLayer[layer];
          const expectedReadback = expectedReadbacksPerLayer[layer];
          const sameOk =
            size.width === 0 ||
            size.height === 0 ||
            layer < origin.z ||
            layer >= origin.z + size.depthOrArrayLayers;
          this.expectOK(
            sameOk ? undefined : arrayNotAllTheSameValue(actualReadback.data, 'actualTexture')
          );
          this.expectOK(
            sameOk ? undefined : arrayNotAllTheSameValue(expectedReadback.data, 'expectedTexture')
          );
          this.expectOK(checkElementsEqual(actualReadback.data, expectedReadback.data), {
            mode: 'fail',
            niceStack,
          });
          actualReadback.cleanup();
          expectedReadback.cleanup();
        }
      });
    }

    copyWholeTextureToNewBufferSimple(texture: GPUTexture, mipLevel: number) {
      const { blockWidth, blockHeight, bytesPerBlock } = kTextureFormatInfo[texture.format];
      const mipSize = physicalMipSizeFromTexture(texture, mipLevel);
      assert(bytesPerBlock !== undefined);

      const blocksPerRow = mipSize[0] / blockWidth;
      const blocksPerColumn = mipSize[1] / blockHeight;

      assert(blocksPerRow % 1 === 0);
      assert(blocksPerColumn % 1 === 0);

      const bytesPerRow = align(blocksPerRow * bytesPerBlock, 256);
      const byteLength = bytesPerRow * blocksPerColumn * mipSize[2];

      return this.copyWholeTextureToNewBuffer(
        { texture, mipLevel },
        {
          bytesPerBlock,
          bytesPerRow,
          rowsPerImage: blocksPerColumn,
          byteLength,
        }
      );
    }

    copyWholeTextureToNewBuffer(
      { texture, mipLevel }: { texture: GPUTexture; mipLevel: number | undefined },
      resultDataLayout: {
        bytesPerBlock: number;
        byteLength: number;
        bytesPerRow: number;
        rowsPerImage: number;
      }
    ): GPUBuffer {
      const { byteLength, bytesPerRow, rowsPerImage } = resultDataLayout;
      const buffer = this.createBufferTracked({
        size: align(byteLength, 4), // this is necessary because we need to copy and map data from this buffer
        usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST,
      });

      const mipSize = physicalMipSizeFromTexture(texture, mipLevel || 0);
      const encoder = this.device.createCommandEncoder();
      encoder.copyTextureToBuffer(
        { texture, mipLevel },
        { buffer, bytesPerRow, rowsPerImage },
        mipSize
      );
      this.device.queue.submit([encoder.finish()]);

      return buffer;
    }

    updateLinearTextureDataSubBox(
      format: ColorTextureFormat,
      copySize: Required<GPUExtent3DDict>,
      copyParams: {
        dest: LinearCopyParameters;
        src: LinearCopyParameters;
      }
    ): void {
      const { src, dest } = copyParams;
      const rowLength = bytesInACompleteRow(copySize.width, format);
      for (const texel of this.iterateBlockRows(copySize, format)) {
        const srcOffsetElements = this.getTexelOffsetInBytes(
          src.dataLayout,
          format,
          texel,
          src.origin
        );
        const dstOffsetElements = this.getTexelOffsetInBytes(
          dest.dataLayout,
          format,
          texel,
          dest.origin
        );
        memcpy(
          { src: src.data, start: srcOffsetElements, length: rowLength },
          { dst: dest.data, start: dstOffsetElements }
        );
      }
    }

    /** Offset for a particular texel in the linear texture data */
    getTexelOffsetInBytes(
      textureDataLayout: Required<GPUTexelCopyBufferLayout>,
      format: ColorTextureFormat,
      texel: Required<GPUOrigin3DDict>,
      origin: Required<GPUOrigin3DDict> = { x: 0, y: 0, z: 0 }
    ): number {
      const { offset, bytesPerRow, rowsPerImage } = textureDataLayout;
      const info = kTextureFormatInfo[format];

      assert(texel.x % info.blockWidth === 0);
      assert(texel.y % info.blockHeight === 0);
      assert(origin.x % info.blockWidth === 0);
      assert(origin.y % info.blockHeight === 0);

      const bytesPerImage = rowsPerImage * bytesPerRow;

      return (
        offset +
        (texel.z + origin.z) * bytesPerImage +
        ((texel.y + origin.y) / info.blockHeight) * bytesPerRow +
        ((texel.x + origin.x) / info.blockWidth) * info.color.bytes
      );
    }

    *iterateBlockRows(
      size: Required<GPUExtent3DDict>,
      format: ColorTextureFormat
    ): Generator<Required<GPUOrigin3DDict>> {
      if (size.width === 0 || size.height === 0 || size.depthOrArrayLayers === 0) {
        // do not iterate anything for an empty region
        return;
      }
      const info = kTextureFormatInfo[format];
      assert(size.height % info.blockHeight === 0);
      // Note: it's important that the order is in increasing memory address order.
      for (let z = 0; z < size.depthOrArrayLayers; ++z) {
        for (let y = 0; y < size.height; y += info.blockHeight) {
          yield {
            x: 0,
            y,
            z,
          };
        }
      }
    }
  }

  return TextureExpectations as unknown as FixtureClassWithMixin<F, TextureTestMixinType>;
}
