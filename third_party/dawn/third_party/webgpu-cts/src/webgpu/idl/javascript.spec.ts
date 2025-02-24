export const description = `
Validate that various GPU objects behave correctly in JavaScript.

Examples:
  - GPUTexture, GPUBuffer, GPUQuerySet, GPUDevice, GPUAdapter, GPUAdapter.limits, GPUDevice.limits
    - return nothing for Object.keys()
    - adds no properties for {...object}
    - iterate over keys for (key in object)
    - throws for (key of object)
    - return nothing from Object.getOwnPropertyDescriptors
 - GPUAdapter.limits, GPUDevice.limits
     - can not be passed to requestAdapter as requiredLimits
 - GPUAdapter.features, GPUDevice.features
    - do spread to array
    - can be copied to set
    - can be passed to requestAdapter as requiredFeatures
`;

import { TestCaseRecorder, TestParams } from '../../common/framework/fixture.js';
import { makeTestGroup } from '../../common/framework/test_group.js';
import { keysOf } from '../../common/util/data_tables.js';
import { getGPU } from '../../common/util/navigator_gpu.js';
import { assert, objectEquals, unreachable } from '../../common/util/util.js';
import { getDefaultLimitsForAdapter, kLimits } from '../capability_info.js';
import {
  DeviceSelectionDescriptor,
  GPUTest,
  GPUTestSubcaseBatchState,
  initUncanonicalizedDeviceDescriptor,
} from '../gpu_test.js';
import { CanonicalDeviceDescriptor, DescriptorModifier } from '../util/device_pool.js';

// MAINTENANCE_TODO: Remove this filter when these limits are added to the spec.
const isUnspecifiedLimit = (limit: string) =>
  /maxStorage(Buffer|Texture)sIn(Vertex|Fragment)Stage/.test(limit);

const kSpecifiedLimits = kLimits.filter(s => !isUnspecifiedLimit(s));

function addAllFeatures(adapter: GPUAdapter, desc: CanonicalDeviceDescriptor | undefined) {
  const descWithMaxLimits: CanonicalDeviceDescriptor = {
    defaultQueue: {},
    ...desc,
    requiredFeatures: [...adapter.features] as GPUFeatureName[],
    requiredLimits: { ...(desc?.requiredLimits ?? {}) },
  };
  return descWithMaxLimits;
}

/**
 * Used to request a device with all the max limits of the adapter.
 */
class AllFeaturesGPUTestSubcaseBatchState extends GPUTestSubcaseBatchState {
  override requestDeviceWithRequiredParametersOrSkip(
    descriptor: DeviceSelectionDescriptor,
    descriptorModifier?: DescriptorModifier
  ): void {
    const mod: DescriptorModifier = {
      descriptorModifier(adapter: GPUAdapter, desc: CanonicalDeviceDescriptor | undefined) {
        desc = descriptorModifier?.descriptorModifier
          ? descriptorModifier.descriptorModifier(adapter, desc)
          : desc;
        return addAllFeatures(adapter, desc);
      },
      keyModifier(baseKey: string) {
        return `${baseKey}:AllFeaturesTest`;
      },
    };
    super.requestDeviceWithRequiredParametersOrSkip(
      initUncanonicalizedDeviceDescriptor(descriptor),
      mod
    );
  }
}

/**
 * A Test that requests all the max limits from the adapter on the device.
 */
class AllFeaturesTest extends GPUTest {
  public static override MakeSharedState(
    recorder: TestCaseRecorder,
    params: TestParams
  ): GPUTestSubcaseBatchState {
    return new AllFeaturesGPUTestSubcaseBatchState(recorder, params);
  }
}

type ResourceInfo = {
  create: (t: GPUTest) => Object;
  requiredKeys: readonly string[];
  getters: readonly string[];
  settable: readonly string[];
};
const kResourceInfo: { [key: string]: ResourceInfo } = {
  buffer: {
    create(t: GPUTest) {
      return t.createBufferTracked({ size: 16, usage: GPUBufferUsage.UNIFORM });
    },
    requiredKeys: [
      'destroy',
      'getMappedRange',
      'label',
      'mapAsync',
      'mapState',
      'size',
      'unmap',
      'usage',
    ],
    getters: ['label', 'mapState', 'size', 'usage'],
    settable: ['label'],
  },
  texture: {
    create(t: GPUTest) {
      return t.createTextureTracked({
        size: [2, 3],
        format: 'r8unorm',
        usage: GPUTextureUsage.TEXTURE_BINDING,
      });
    },
    requiredKeys: [
      'createView',
      'depthOrArrayLayers',
      'destroy',
      'dimension',
      'format',
      'height',
      'label',
      'mipLevelCount',
      'sampleCount',
      'usage',
      'width',
    ],
    getters: [
      'depthOrArrayLayers',
      'dimension',
      'format',
      'height',
      'label',
      'mipLevelCount',
      'sampleCount',
      'usage',
      'width',
    ],
    settable: ['label'],
  },
  querySet: {
    create(t: GPUTest) {
      return t.createQuerySetTracked({
        type: 'occlusion',
        count: 2,
      });
    },
    requiredKeys: ['count', 'destroy', 'label', 'type'],
    getters: ['count', 'label', 'type'],
    settable: ['label'],
  },
  adapter: {
    create(t: GPUTest) {
      return t.adapter;
    },
    requiredKeys: ['features', 'info', 'limits', 'requestDevice'],
    getters: ['features', 'info', 'limits'],
    settable: [],
  },
  device: {
    create(t: GPUTest) {
      return t.device;
    },
    requiredKeys: [
      'adapterInfo',
      'addEventListener',
      'createBindGroup',
      'createBindGroupLayout',
      'createBuffer',
      'createCommandEncoder',
      'createComputePipeline',
      'createComputePipelineAsync',
      'createPipelineLayout',
      'createQuerySet',
      'createRenderBundleEncoder',
      'createRenderPipeline',
      'createRenderPipelineAsync',
      'createSampler',
      'createShaderModule',
      'createTexture',
      'destroy',
      'dispatchEvent',
      'features',
      'importExternalTexture',
      'label',
      'limits',
      'lost',
      'onuncapturederror',
      'popErrorScope',
      'pushErrorScope',
      'queue',
      'removeEventListener',
    ],
    getters: ['adapterInfo', 'features', 'label', 'limits', 'lost', 'onuncapturederror', 'queue'],
    settable: ['label', 'onuncapturederror'],
  },
  'adapter.limits': {
    create(t: GPUTest) {
      return t.adapter.limits;
    },
    requiredKeys: kSpecifiedLimits,
    getters: kSpecifiedLimits,
    settable: [],
  },
  'device.limits': {
    create(t: GPUTest) {
      return t.device.limits;
    },
    requiredKeys: kSpecifiedLimits,
    getters: kSpecifiedLimits,
    settable: [],
  },
} as const;
const kResources = keysOf(kResourceInfo);
type ResourceName = (typeof kResources)[number];

function createResource(t: GPUTest, type: ResourceName) {
  return kResourceInfo[type].create(t);
}

type Resource = ReturnType<typeof createResource>;

function forOfIterations(obj: Resource) {
  let count = 0;
  for (const _ of obj as unknown as []) {
    ++count;
  }
  return count;
}

function hasRequiredKeys(t: GPUTest, obj: Resource, requiredKeys: readonly string[]) {
  for (const requiredKey of requiredKeys) {
    t.expect(requiredKey in obj, `${requiredKey} in ${obj.constructor.name} exists`);
  }
}

function aHasBElements(
  t: GPUTest,
  a: GPUSupportedFeatures | string[],
  b: GPUSupportedFeatures | string[]
) {
  for (const elem of b) {
    if (Array.isArray(a)) {
      t.expect(a.includes(elem), `missing ${elem}`);
    } else if (a.has) {
      t.expect(a.has(elem), `missing ${elem}`);
    } else {
      unreachable();
    }
  }
}

export const g = makeTestGroup(AllFeaturesTest);
g.test('obj,Object_keys')
  .desc('tests returns nothing for Object.keys()')
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const obj = createResource(t, type);
    t.expect(objectEquals([...Object.keys(obj)], []), `[...Object.keys(${type})] === []`);
  });

g.test('obj,spread')
  .desc('does not spread')
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const obj = createResource(t, type);
    t.expect(objectEquals({ ...obj }, {}), `{ ...${type} ] === {}`);
  });

g.test('obj,for_in')
  .desc('iterates over keys - for (key in object)')
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const obj = createResource(t, type);
    hasRequiredKeys(t, obj, kResourceInfo[type].requiredKeys);
  });

g.test('obj,for_of')
  .desc('throws TypeError - for (key of object')
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const obj = createResource(t, type);
    t.shouldThrow('TypeError', () => forOfIterations(obj), {
      message: `for (const key of ${type} } throws TypeError`,
    });
  });

g.test('obj,getOwnPropertyDescriptors')
  .desc('Object.getOwnPropertyDescriptors returns {}')
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const obj = createResource(t, type);
    t.expect(
      objectEquals(Object.getOwnPropertyDescriptors(obj), {}),
      `Object.getOwnPropertyDescriptors(${type}} === {}`
    );
  });

g.test('setlike,spread')
  .desc('obj spreads')
  .params(u => u.combine('type', ['adapter', 'device'] as const))
  .fn(t => {
    const { type } = t.params;
    const obj = type === 'adapter' ? t.adapter : t.device;
    const copy = [...obj.features];
    aHasBElements(t, copy, obj.features);
    aHasBElements(t, obj.features, copy);
  });

g.test('setlike,set')
  .desc('obj copies to set')
  .params(u => u.combine('type', ['adapter', 'device'] as const))
  .fn(t => {
    const { type } = t.params;
    const obj = type === 'adapter' ? t.adapter : t.device;
    const copy = new Set(obj.features);
    aHasBElements(t, copy, obj.features);
    aHasBElements(t, obj.features, copy);
  });

g.test('setlike,requiredFeatures')
  .desc('can be passed as required features')
  .params(u => u.combine('type', ['adapter', 'device'] as const))
  .fn(async t => {
    const { type } = t.params;
    const obj = type === 'adapter' ? t.adapter : t.device;

    const gpu = getGPU(null);
    const adapter = await gpu.requestAdapter();
    const device = await t.requestDeviceTracked(adapter!, {
      requiredFeatures: obj.features as Iterable<GPUFeatureName>,
    });
    aHasBElements(t, device.features, obj.features);
    aHasBElements(t, obj.features, device.features);
  });

g.test('limits')
  .desc('adapter/device.limits can not be passed as requiredLimits')
  .params(u => u.combine('type', ['adapter', 'device'] as const))
  .fn(async t => {
    const { type } = t.params;
    const obj = type === 'adapter' ? t.adapter : t.device;

    const gpu = getGPU(null);
    const adapter = await gpu.requestAdapter();
    assert(!!adapter);
    const device = await t.requestDeviceTracked(adapter, {
      requiredLimits: obj.limits as unknown as Record<string, number>,
    });
    const defaultLimits = getDefaultLimitsForAdapter(adapter);
    for (const [key, { default: defaultLimit }] of Object.entries(defaultLimits)) {
      if (isUnspecifiedLimit(key)) {
        continue;
      }
      const actual = (device.limits as unknown as Record<string, number>)[key];
      t.expect(
        actual === defaultLimit,
        `expected device.limits.${key}(${actual}) === ${defaultLimit}`
      );
    }
  });

g.test('readonly_properties')
  .desc(
    `
    Test that setting a property with no setter throws.
    `
  )
  .params(u => u.combine('type', kResources))
  .fn(t => {
    'use strict'; // This makes setting a readonly property produce a TypeError.
    const { type } = t.params;
    const { getters, settable } = kResourceInfo[type];
    const obj = createResource(t, type);
    for (const getter of getters) {
      const origValue = (obj as unknown as Record<string, unknown>)[getter];

      // try setting it.
      const isSettable = settable.includes(getter);
      t.shouldThrow(
        isSettable ? false : 'TypeError',
        () => {
          (obj as unknown as Record<string, string>)[getter] = 'test value';
          // If we were able to set it, restore it.
          (obj as unknown as Record<string, unknown>)[getter] = origValue;
        },
        {
          message: `instanceof ${type}.${getter} = value ${
            isSettable ? 'does not throw' : 'throws'
          }`,
        }
      );
    }
  });

g.test('getter_replacement')
  .desc(
    `
    Test that replacing getters on class prototypes works

    This is a common pattern for shims and debugging libraries so make sure this pattern works.
    `
  )
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const { getters } = kResourceInfo[type];

    const obj = createResource(t, type);
    for (const getter of getters) {
      // Check it's not 'ownProperty`
      const properties = Object.getOwnPropertyDescriptor(obj, getter);
      t.expect(
        properties === undefined,
        `Object.getOwnPropertyDescriptor(instance of ${type}, '${getter}') === undefined`
      );

      // Check it's actually a getter that returns a non-function value.
      const origValue = (obj as unknown as Record<string, () => unknown>)[getter];
      t.expect(typeof origValue !== 'function', `instance of ${type}.${getter} !== 'function'`);

      // check replacing the getter on constructor works.
      const ctorPrototype = obj.constructor.prototype;
      const origProperties = Object.getOwnPropertyDescriptor(ctorPrototype, getter);
      assert(
        !!origProperties,
        `Object.getOwnPropertyDescriptor(${type}, '${getter}') !== undefined`
      );
      try {
        Object.defineProperty(ctorPrototype, getter, {
          get() {
            return 'testGetterValue';
          },
        });
        const value = (obj as unknown as Record<string, string>)[getter];
        t.expect(
          value === 'testGetterValue',
          `replacing getter: '${getter}' on ${type} returns test value`
        );
      } finally {
        Object.defineProperty(ctorPrototype, getter, origProperties);
      }

      // Check it turns the same value after restoring as before restoring.
      const afterValue = (obj as unknown as Record<string, () => unknown>)[getter];
      assert(afterValue === origValue, `able to restore getter for instance of ${type}.${getter}`);

      // Check getOwnProperty also returns the value we got before.
      assert(
        Object.getOwnPropertyDescriptor(ctorPrototype, getter)!.get === origProperties.get,
        `getOwnPropertyDescriptor(${type}, '${getter}').get is original function`
      );
    }
  });

g.test('method_replacement')
  .desc(
    `
    Test that replacing methods on class prototypes works

    This is a common pattern for shims and debugging libraries so make sure this pattern works.
    `
  )
  .params(u => u.combine('type', kResources))
  .fn(t => {
    const { type } = t.params;
    const { requiredKeys, getters } = kResourceInfo[type];
    const gettersSet = new Set<string>(getters);
    const methods = requiredKeys.filter(k => !gettersSet.has(k));

    const obj = createResource(t, type);
    for (const method of methods) {
      const ctorPrototype = obj.constructor.prototype;
      const origFunc = ctorPrototype[method];

      t.expect(typeof origFunc === 'function', `${type}.prototype.${method} is a function`);

      // Check the function the prototype and the one on the object are the same
      t.expect(
        (obj as unknown as Record<string, unknown>)[method] === origFunc,
        `instance of ${type}.${method} === ${type}.prototype.${method}`
      );

      // Check replacing the method on constructor works.
      try {
        (ctorPrototype as unknown as Record<string, unknown>)[method] = function () {
          return 'testMethodValue';
        };
        const value = (obj as unknown as Record<string, () => string>)[method]();
        t.expect(
          value === 'testMethodValue',
          `replacing method: '${method}' on ${type} returns test value`
        );
      } finally {
        (ctorPrototype as unknown as Record<string, unknown>)[method] = origFunc;
      }

      // Check the function the prototype and the one on the object are the same after restoring.
      assert(
        (obj as unknown as Record<string, unknown>)[method] === origFunc,
        `instance of ${type}.${method} === ${type}.prototype.${method}`
      );
    }
  });
