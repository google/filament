import {
  ValidBindableResource,
  BindableResource,
  kMaxQueryCount,
  ShaderStageKey,
} from '../../capability_info.js';
import { GPUTest, ResourceState } from '../../gpu_test.js';

/**
 * Base fixture for WebGPU validation tests.
 */
export class ValidationTest extends GPUTest {
  /**
   * Create a GPUTexture in the specified state.
   * A `descriptor` may optionally be passed, which is used when `state` is not `'invalid'`.
   */
  createTextureWithState(
    state: ResourceState,
    descriptor?: Readonly<GPUTextureDescriptor>
  ): GPUTexture {
    descriptor = descriptor ?? {
      size: { width: 1, height: 1, depthOrArrayLayers: 1 },
      format: 'rgba8unorm',
      usage:
        GPUTextureUsage.COPY_SRC |
        GPUTextureUsage.COPY_DST |
        GPUTextureUsage.TEXTURE_BINDING |
        GPUTextureUsage.STORAGE_BINDING |
        GPUTextureUsage.RENDER_ATTACHMENT,
    };

    switch (state) {
      case 'valid':
        return this.createTextureTracked(descriptor);
      case 'invalid':
        return this.getErrorTexture();
      case 'destroyed': {
        const texture = this.createTextureTracked(descriptor);
        texture.destroy();
        return texture;
      }
    }
  }

  /**
   * Create a GPUTexture in the specified state. A `descriptor` may optionally be passed;
   * if `state` is `'invalid'`, it will be modified to add an invalid combination of usages.
   */
  createBufferWithState(
    state: ResourceState,
    descriptor?: Readonly<GPUBufferDescriptor>
  ): GPUBuffer {
    descriptor = descriptor ?? {
      size: 4,
      usage: GPUBufferUsage.VERTEX,
    };

    switch (state) {
      case 'valid':
        return this.createBufferTracked(descriptor);

      case 'invalid': {
        // Make the buffer invalid because of an invalid combination of usages but keep the
        // descriptor passed as much as possible (for mappedAtCreation and friends).
        this.device.pushErrorScope('validation');
        const buffer = this.createBufferTracked({
          ...descriptor,
          usage: descriptor.usage | GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_SRC,
        });
        void this.device.popErrorScope();
        return buffer;
      }
      case 'destroyed': {
        const buffer = this.createBufferTracked(descriptor);
        buffer.destroy();
        return buffer;
      }
    }
  }

  /**
   * Create a GPUQuerySet in the specified state.
   * A `descriptor` may optionally be passed, which is used when `state` is not `'invalid'`.
   */
  createQuerySetWithState(
    state: ResourceState,
    desc?: Readonly<GPUQuerySetDescriptor>
  ): GPUQuerySet {
    const descriptor = { type: 'occlusion' as const, count: 2, ...desc };

    switch (state) {
      case 'valid':
        return this.createQuerySetTracked(descriptor);
      case 'invalid': {
        // Make the queryset invalid because of the count out of bounds.
        descriptor.count = kMaxQueryCount + 1;
        return this.expectGPUError('validation', () => this.createQuerySetTracked(descriptor));
      }
      case 'destroyed': {
        const queryset = this.createQuerySetTracked(descriptor);
        queryset.destroy();
        return queryset;
      }
    }
  }

  /** Create an arbitrarily-sized GPUBuffer with the STORAGE usage. */
  getStorageBuffer(): GPUBuffer {
    return this.createBufferTracked({ size: 1024, usage: GPUBufferUsage.STORAGE });
  }

  /** Create an arbitrarily-sized GPUBuffer with the UNIFORM usage. */
  getUniformBuffer(): GPUBuffer {
    return this.createBufferTracked({ size: 1024, usage: GPUBufferUsage.UNIFORM });
  }

  /** Return an invalid GPUBuffer. */
  getErrorBuffer(): GPUBuffer {
    return this.createBufferWithState('invalid');
  }

  /** Return an invalid GPUSampler. */
  getErrorSampler(): GPUSampler {
    this.device.pushErrorScope('validation');
    const sampler = this.device.createSampler({ lodMinClamp: -1 });
    void this.device.popErrorScope();
    return sampler;
  }

  /**
   * Return an arbitrarily-configured GPUTexture with the `TEXTURE_BINDING` usage and specified
   * sampleCount. The `RENDER_ATTACHMENT` usage will also be specified if sampleCount > 1 as is
   * required by WebGPU SPEC.
   */
  getSampledTexture(sampleCount: number = 1): GPUTexture {
    const usage =
      sampleCount > 1
        ? GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.RENDER_ATTACHMENT
        : GPUTextureUsage.TEXTURE_BINDING;
    return this.createTextureTracked({
      size: { width: 16, height: 16, depthOrArrayLayers: 1 },
      format: 'rgba8unorm',
      usage,
      sampleCount,
    });
  }

  /** Return an arbitrarily-configured GPUTexture with the `STORAGE_BINDING` usage. */
  getStorageTexture(format: GPUTextureFormat): GPUTexture {
    return this.createTextureTracked({
      size: { width: 16, height: 16, depthOrArrayLayers: 1 },
      format,
      usage: GPUTextureUsage.STORAGE_BINDING,
    });
  }

  /** Return an arbitrarily-configured GPUTexture with the `RENDER_ATTACHMENT` usage. */
  getRenderTexture(sampleCount: number = 1): GPUTexture {
    return this.createTextureTracked({
      size: { width: 16, height: 16, depthOrArrayLayers: 1 },
      format: 'rgba8unorm',
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
      sampleCount,
    });
  }

  /** Return an invalid GPUTexture. */
  getErrorTexture(): GPUTexture {
    this.device.pushErrorScope('validation');
    const texture = this.createTextureTracked({
      size: { width: 0, height: 0, depthOrArrayLayers: 0 },
      format: 'rgba8unorm',
      usage: GPUTextureUsage.TEXTURE_BINDING,
    });
    void this.device.popErrorScope();
    return texture;
  }

  /** Return an invalid GPUTextureView (created from an invalid GPUTexture). */
  getErrorTextureView(): GPUTextureView {
    this.device.pushErrorScope('validation');
    const view = this.getErrorTexture().createView();
    void this.device.popErrorScope();
    return view;
  }

  /**
   * Return an arbitrary object of the specified {@link webgpu/capability_info!BindableResource} type
   * (e.g. `'errorBuf'`, `'nonFiltSamp'`, `sampledTexMS`, etc.)
   */
  getBindingResource(bindingType: BindableResource): GPUBindingResource {
    switch (bindingType) {
      case 'errorBuf':
        return { buffer: this.getErrorBuffer() };
      case 'errorSamp':
        return this.getErrorSampler();
      case 'errorTex':
        return this.getErrorTextureView();
      case 'uniformBuf':
        return { buffer: this.getUniformBuffer() };
      case 'storageBuf':
        return { buffer: this.getStorageBuffer() };
      case 'filtSamp':
        return this.device.createSampler({ minFilter: 'linear' });
      case 'nonFiltSamp':
        return this.device.createSampler();
      case 'compareSamp':
        return this.device.createSampler({ compare: 'never' });
      case 'sampledTex':
        return this.getSampledTexture(1).createView();
      case 'sampledTexMS':
        return this.getSampledTexture(4).createView();
      case 'readonlyStorageTex':
      case 'writeonlyStorageTex':
      case 'readwriteStorageTex':
        return this.getStorageTexture('r32float').createView();
    }
  }

  /** Create an arbitrarily-sized GPUBuffer with the STORAGE usage from mismatched device. */
  getDeviceMismatchedStorageBuffer(): GPUBuffer {
    return this.trackForCleanup(
      this.mismatchedDevice.createBuffer({ size: 4, usage: GPUBufferUsage.STORAGE })
    );
  }

  /** Create an arbitrarily-sized GPUBuffer with the UNIFORM usage from mismatched device. */
  getDeviceMismatchedUniformBuffer(): GPUBuffer {
    return this.trackForCleanup(
      this.mismatchedDevice.createBuffer({ size: 4, usage: GPUBufferUsage.UNIFORM })
    );
  }

  /** Return a GPUTexture with descriptor from mismatched device. */
  getDeviceMismatchedTexture(descriptor: GPUTextureDescriptor): GPUTexture {
    return this.trackForCleanup(this.mismatchedDevice.createTexture(descriptor));
  }

  /** Return an arbitrarily-configured GPUTexture with the `SAMPLED` usage from mismatched device. */
  getDeviceMismatchedSampledTexture(sampleCount: number = 1): GPUTexture {
    return this.getDeviceMismatchedTexture({
      size: { width: 4, height: 4, depthOrArrayLayers: 1 },
      format: 'rgba8unorm',
      usage: GPUTextureUsage.TEXTURE_BINDING,
      sampleCount,
    });
  }

  /** Return an arbitrarily-configured GPUTexture with the `STORAGE` usage from mismatched device. */
  getDeviceMismatchedStorageTexture(format: GPUTextureFormat): GPUTexture {
    return this.getDeviceMismatchedTexture({
      size: { width: 4, height: 4, depthOrArrayLayers: 1 },
      format,
      usage: GPUTextureUsage.STORAGE_BINDING,
    });
  }

  /** Return an arbitrarily-configured GPUTexture with the `RENDER_ATTACHMENT` usage from mismatched device. */
  getDeviceMismatchedRenderTexture(sampleCount: number = 1): GPUTexture {
    return this.getDeviceMismatchedTexture({
      size: { width: 4, height: 4, depthOrArrayLayers: 1 },
      format: 'rgba8unorm',
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
      sampleCount,
    });
  }

  getDeviceMismatchedBindingResource(bindingType: ValidBindableResource): GPUBindingResource {
    switch (bindingType) {
      case 'uniformBuf':
        return { buffer: this.getDeviceMismatchedUniformBuffer() };
      case 'storageBuf':
        return { buffer: this.getDeviceMismatchedStorageBuffer() };
      case 'filtSamp':
        return this.mismatchedDevice.createSampler({ minFilter: 'linear' });
      case 'nonFiltSamp':
        return this.mismatchedDevice.createSampler();
      case 'compareSamp':
        return this.mismatchedDevice.createSampler({ compare: 'never' });
      case 'sampledTex':
        return this.getDeviceMismatchedSampledTexture(1).createView();
      case 'sampledTexMS':
        return this.getDeviceMismatchedSampledTexture(4).createView();
      case 'readonlyStorageTex':
      case 'writeonlyStorageTex':
      case 'readwriteStorageTex':
        return this.getDeviceMismatchedStorageTexture('r32float').createView();
    }
  }

  /** Return a no-op shader code snippet for the specified shader stage. */
  getNoOpShaderCode(stage: ShaderStageKey): string {
    switch (stage) {
      case 'VERTEX':
        return `
          @vertex fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>();
          }
        `;
      case 'FRAGMENT':
        return `@fragment fn main() {}`;
      case 'COMPUTE':
        return `@compute @workgroup_size(1) fn main() {}`;
    }
  }

  /** Create a GPURenderPipeline in the specified state. */
  createRenderPipelineWithState(state: 'valid' | 'invalid'): GPURenderPipeline {
    return state === 'valid' ? this.createNoOpRenderPipeline() : this.createErrorRenderPipeline();
  }

  /** Return a GPURenderPipeline with default options and no-op vertex and fragment shaders. */
  createNoOpRenderPipeline(
    layout: GPUPipelineLayout | GPUAutoLayoutMode = 'auto',
    colorFormat: GPUTextureFormat = 'rgba8unorm'
  ): GPURenderPipeline {
    return this.device.createRenderPipeline({
      layout,
      vertex: {
        module: this.device.createShaderModule({
          code: this.getNoOpShaderCode('VERTEX'),
        }),
        entryPoint: 'main',
      },
      fragment: {
        module: this.device.createShaderModule({
          code: this.getNoOpShaderCode('FRAGMENT'),
        }),
        entryPoint: 'main',
        targets: [{ format: colorFormat, writeMask: 0 }],
      },
      primitive: { topology: 'triangle-list' },
    });
  }

  /** Return an invalid GPURenderPipeline. */
  createErrorRenderPipeline(): GPURenderPipeline {
    this.device.pushErrorScope('validation');
    const pipeline = this.device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        module: this.device.createShaderModule({
          code: '',
        }),
        entryPoint: '',
      },
    });
    void this.device.popErrorScope();
    return pipeline;
  }

  /** Return a GPUComputePipeline with a no-op shader. */
  createNoOpComputePipeline(
    layout: GPUPipelineLayout | GPUAutoLayoutMode = 'auto'
  ): GPUComputePipeline {
    return this.device.createComputePipeline({
      layout,
      compute: {
        module: this.device.createShaderModule({
          code: this.getNoOpShaderCode('COMPUTE'),
        }),
        entryPoint: 'main',
      },
    });
  }

  /** Return an invalid GPUComputePipeline. */
  createErrorComputePipeline(): GPUComputePipeline {
    this.device.pushErrorScope('validation');
    const pipeline = this.device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: this.device.createShaderModule({
          code: '',
        }),
        entryPoint: '',
      },
    });
    void this.device.popErrorScope();
    return pipeline;
  }

  /** Return an invalid GPUShaderModule. */
  createInvalidShaderModule(): GPUShaderModule {
    this.device.pushErrorScope('validation');
    const code = 'deadbeaf'; // Something make no sense
    const shaderModule = this.device.createShaderModule({ code });
    void this.device.popErrorScope();
    return shaderModule;
  }

  /** Helper for testing createRenderPipeline(Async) validation */
  doCreateRenderPipelineTest(
    isAsync: boolean,
    _success: boolean,
    descriptor: GPURenderPipelineDescriptor,
    errorTypeName: 'GPUPipelineError' | 'TypeError' = 'GPUPipelineError'
  ) {
    if (isAsync) {
      if (_success) {
        this.shouldResolve(this.device.createRenderPipelineAsync(descriptor));
      } else {
        this.shouldReject(errorTypeName, this.device.createRenderPipelineAsync(descriptor));
      }
    } else {
      if (errorTypeName === 'GPUPipelineError') {
        this.expectValidationError(() => {
          this.device.createRenderPipeline(descriptor);
        }, !_success);
      } else {
        this.shouldThrow(_success ? false : errorTypeName, () => {
          this.device.createRenderPipeline(descriptor);
        });
      }
    }
  }

  /** Helper for testing createComputePipeline(Async) validation */
  doCreateComputePipelineTest(
    isAsync: boolean,
    _success: boolean,
    descriptor: GPUComputePipelineDescriptor,
    errorTypeName: 'GPUPipelineError' | 'TypeError' = 'GPUPipelineError'
  ) {
    if (isAsync) {
      if (_success) {
        this.shouldResolve(this.device.createComputePipelineAsync(descriptor));
      } else {
        this.shouldReject(errorTypeName, this.device.createComputePipelineAsync(descriptor));
      }
    } else {
      if (errorTypeName === 'GPUPipelineError') {
        this.expectValidationError(() => {
          this.device.createComputePipeline(descriptor);
        }, !_success);
      } else {
        this.shouldThrow(_success ? false : errorTypeName, () => {
          this.device.createComputePipeline(descriptor);
        });
      }
    }
  }
}
