// eslint-disable-next-line import/no-restricted-paths
import { TestCaseRecorder } from '../framework/fixture.js';
import { globalTestConfig } from '../framework/test_config.js';

import { ErrorWithExtra, assert, objectEquals } from './util.js';

/**
 * Finds and returns the `navigator.gpu` object (or equivalent, for non-browser implementations).
 * Throws an exception if not found.
 */
function defaultGPUProvider(): GPU {
  assert(
    typeof navigator !== 'undefined' && navigator.gpu !== undefined,
    'No WebGPU implementation found'
  );
  return navigator.gpu;
}

/**
 * GPUProvider is a function that creates and returns a new GPU instance.
 * May throw an exception if a GPU cannot be created.
 */
export type GPUProvider = () => GPU;

let gpuProvider: GPUProvider = defaultGPUProvider;

/**
 * Sets the function to create and return a new GPU instance.
 */
export function setGPUProvider(provider: GPUProvider) {
  assert(impl === undefined, 'setGPUProvider() should not be after getGPU()');
  gpuProvider = provider;
}

let impl: GPU | undefined = undefined;
let s_defaultLimits: Record<string, GPUSize64> | undefined = undefined;

let defaultRequestAdapterOptions: GPURequestAdapterOptions | undefined;

export function setDefaultRequestAdapterOptions(options: GPURequestAdapterOptions) {
  // It's okay to call this if you don't change the options
  if (objectEquals(options, defaultRequestAdapterOptions)) {
    return;
  }
  if (impl) {
    throw new Error('must call setDefaultRequestAdapterOptions before getGPU');
  }
  defaultRequestAdapterOptions = { ...options };
}

export function getDefaultRequestAdapterOptions() {
  return defaultRequestAdapterOptions;
}

function copyLimits(objLike: GPUSupportedLimits) {
  const obj: Record<string, number> = {};
  for (const key in objLike) {
    obj[key] = (objLike as unknown as Record<string, number>)[key];
  }
  return obj;
}

/**
 * Finds and returns the `navigator.gpu` object (or equivalent, for non-browser implementations).
 * Throws an exception if not found.
 */
export function getGPU(recorder: TestCaseRecorder | null): GPU {
  if (impl) {
    return impl;
  }

  impl = gpuProvider();

  if (globalTestConfig.enforceDefaultLimits) {
    // eslint-disable-next-line @typescript-eslint/unbound-method
    const origRequestAdapterFn = impl.requestAdapter;
    // eslint-disable-next-line @typescript-eslint/unbound-method
    const origRequestDeviceFn = GPUAdapter.prototype.requestDevice;

    impl.requestAdapter = async function (options?: GPURequestAdapterOptions) {
      if (!s_defaultLimits) {
        const tempAdapter = await origRequestAdapterFn.call(this, {
          ...defaultRequestAdapterOptions,
          ...options,
        });
        // eslint-disable-next-line no-restricted-syntax
        const tempDevice = await tempAdapter?.requestDevice();
        s_defaultLimits = copyLimits(tempDevice!.limits);
        tempDevice?.destroy();
      }
      const adapter = await origRequestAdapterFn.call(this, {
        ...defaultRequestAdapterOptions,
        ...options,
      });
      if (adapter) {
        const limits = Object.fromEntries(
          Object.entries(s_defaultLimits).map(([key, v]) => [key, v])
        );

        Object.defineProperty(adapter, 'limits', {
          get() {
            return limits;
          },
        });
      }
      return adapter;
    };

    const enforceDefaultLimits = (adapter: GPUAdapter, desc: GPUDeviceDescriptor | undefined) => {
      if (desc?.requiredLimits) {
        for (const [key, value] of Object.entries(desc.requiredLimits)) {
          const limit = s_defaultLimits![key];
          if (limit !== undefined && value !== undefined) {
            const [beyondLimit, condition] = key.startsWith('max')
              ? [value > limit, 'greater']
              : [value < limit, 'less'];
            if (beyondLimit) {
              throw new DOMException(
                `requestedLimit ${value} for ${key} is ${condition} than adapter limit ${limit}`,
                'OperationError'
              );
            }
          }
        }
      }
    };

    GPUAdapter.prototype.requestDevice = async function (
      this: GPUAdapter,
      desc?: GPUDeviceDescriptor | undefined
    ) {
      // We need to enforce the default limits because even though we patched the adapter to
      // show defaults for adapter.limits, there are tests that test we throw when we request more than the max.
      // In other words.
      //
      //   adapter.requestDevice({ requiredLimits: {
      //     maxXXX: addapter.limits.maxXXX + 1,  // should throw
      //   });
      //
      // But unless we enforce this manually, it won't actual through if the adapter's
      // true limits are higher than we patched above.
      enforceDefaultLimits(this, desc);
      return await origRequestDeviceFn.call(this, desc);
    };
  }

  if (defaultRequestAdapterOptions) {
    // eslint-disable-next-line @typescript-eslint/unbound-method
    const oldFn = impl.requestAdapter;
    impl.requestAdapter = function (
      options?: GPURequestAdapterOptions
    ): Promise<GPUAdapter | null> {
      const promise = oldFn.call(this, { ...defaultRequestAdapterOptions, ...options });
      if (recorder) {
        void promise.then(adapter => {
          if (adapter) {
            const adapterInfo = adapter.info;
            const infoString = `Adapter: ${adapterInfo.vendor} / ${adapterInfo.architecture} / ${adapterInfo.device}`;
            recorder.debug(new ErrorWithExtra(infoString, () => ({ adapterInfo })));
          }
        });
      }
      return promise;
    };
  }

  return impl;
}
