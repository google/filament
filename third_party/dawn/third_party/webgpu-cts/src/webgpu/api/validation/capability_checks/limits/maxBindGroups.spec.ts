import { assert } from '../../../../../common/util/util.js';

import {
  kCreatePipelineTypes,
  kEncoderTypes,
  kMaximumLimitBaseParams,
  makeLimitTestGroup,
} from './limit_utils.js';

const limit = 'maxBindGroups';
export const { g, description } = makeLimitTestGroup(limit);

type BindingLayout = {
  buffer?: GPUBufferBindingLayout;
  sampler?: GPUSamplerBindingLayout;
  texture?: GPUTextureBindingLayout;
  storageTexture?: GPUStorageTextureBindingLayout;
  externalTexture?: GPUExternalTextureBindingLayout;
};

type LimitToBindingLayout = {
  name: keyof GPUSupportedLimits;
  entry: BindingLayout;
};

const kLimitToBindingLayout: readonly LimitToBindingLayout[] = [
  {
    name: 'maxSampledTexturesPerShaderStage',
    entry: {
      texture: {},
    },
  },
  {
    name: 'maxSamplersPerShaderStage',
    entry: {
      sampler: {},
    },
  },
  {
    name: 'maxUniformBuffersPerShaderStage',
    entry: {
      buffer: {},
    },
  },
  {
    name: 'maxStorageBuffersPerShaderStage',
    entry: {
      buffer: {
        type: 'read-only-storage',
      },
    },
  },
  {
    name: 'maxStorageTexturesPerShaderStage',
    entry: {
      storageTexture: {
        access: 'write-only',
        format: 'rgba8unorm',
        viewDimension: '2d',
      },
    },
  },
] as const;

/**
 * Yields all possible binding layout entries for a stage.
 */
function* getBindingLayoutEntriesForStage(device: GPUDevice) {
  for (const { name, entry } of kLimitToBindingLayout) {
    const limit = device.limits[name] as number;
    for (let i = 0; i < limit; ++i) {
      yield entry;
    }
  }
}

/**
 * Yields all of the possible BindingLayoutEntryAndVisibility entries for a render pipeline
 */
function* getBindingLayoutEntriesForRenderPipeline(
  device: GPUDevice
): Generator<GPUBindGroupLayoutEntry> {
  const visibilities = [GPUShaderStage.VERTEX, GPUShaderStage.FRAGMENT];
  for (const visibility of visibilities) {
    for (const bindEntryResourceType of getBindingLayoutEntriesForStage(device)) {
      const entry: GPUBindGroupLayoutEntry = {
        binding: 0,
        visibility,
        ...bindEntryResourceType,
      };
      yield entry;
    }
  }
}

/**
 * Returns the total possible bindings per render pipeline
 */
function getTotalPossibleBindingsPerRenderPipeline(device: GPUDevice) {
  const totalPossibleBindingsPerStage =
    device.limits.maxSampledTexturesPerShaderStage +
    device.limits.maxSamplersPerShaderStage +
    device.limits.maxUniformBuffersPerShaderStage +
    device.limits.maxStorageBuffersPerShaderStage +
    device.limits.maxStorageTexturesPerShaderStage;
  return totalPossibleBindingsPerStage * 2;
}

/**
 * Yields count GPUBindGroupLayoutEntries
 */
function* getBindingLayoutEntries(
  device: GPUDevice,
  count: number
): Generator<GPUBindGroupLayoutEntry> {
  assert(count < getTotalPossibleBindingsPerRenderPipeline(device));
  const iter = getBindingLayoutEntriesForRenderPipeline(device);
  for (; count > 0; --count) {
    yield iter.next().value;
  }
}

g.test('createPipelineLayout,at_over')
  .desc(`Test using createPipelineLayout at and over ${limit} limit`)
  .params(kMaximumLimitBaseParams)
  .fn(async t => {
    const { limitTest, testValueName } = t.params;

    await t.testDeviceWithRequestedMaximumLimits(
      limitTest,
      testValueName,
      async ({ device, testValue, shouldError, actualLimit }) => {
        const totalPossibleBindingsPerPipeline = getTotalPossibleBindingsPerRenderPipeline(device);
        // Not sure what to do if we ever hit this but I think it's better to assert than silently skip.
        assert(
          testValue < totalPossibleBindingsPerPipeline,
          `not enough possible bindings(${totalPossibleBindingsPerPipeline}) to test ${testValue} bindGroups`
        );

        const bindingDescriptions: string[] = [];
        const bindGroupLayouts = [...getBindingLayoutEntries(device, testValue)].map(entry => {
          bindingDescriptions.push(
            `${JSON.stringify(entry)} // group(${bindingDescriptions.length})`
          );
          return device.createBindGroupLayout({
            entries: [entry],
          });
        });

        await t.expectValidationError(
          () => {
            device.createPipelineLayout({ bindGroupLayouts });
          },
          shouldError,
          `testing ${testValue} bindGroups on maxBindGroups = ${actualLimit} with \n${bindingDescriptions.join(
            '\n'
          )}`
        );
      }
    );
  });

g.test('createPipeline,at_over')
  .desc(
    `Test using createRenderPipeline(Async) and createComputePipeline(Async) at and over ${limit} limit`
  )
  .params(
    kMaximumLimitBaseParams
      .combine('createPipelineType', kCreatePipelineTypes)
      .combine('async', [false, true] as const)
  )
  .fn(async t => {
    const { limitTest, testValueName, createPipelineType, async } = t.params;

    await t.testDeviceWithRequestedMaximumLimits(
      limitTest,
      testValueName,
      async ({ device, testValue, shouldError }) => {
        const lastIndex = testValue - 1;

        const code = t.getGroupIndexWGSLForPipelineType(createPipelineType, lastIndex);
        const module = device.createShaderModule({ code });

        await t.testCreatePipeline(createPipelineType, async, module, shouldError);
      }
    );
  });

g.test('setBindGroup,at_over')
  .desc(`Test using setBindGroup at and over ${limit} limit`)
  .params(kMaximumLimitBaseParams.combine('encoderType', kEncoderTypes))
  .fn(async t => {
    const { limitTest, testValueName, encoderType } = t.params;
    await t.testDeviceWithRequestedMaximumLimits(
      limitTest,
      testValueName,
      async ({ testValue, actualLimit, shouldError }) => {
        const lastIndex = testValue - 1;
        await t.testGPUBindingCommandsMixin(
          encoderType,
          ({ passEncoder, bindGroup }) => {
            passEncoder.setBindGroup(lastIndex, bindGroup);
          },
          shouldError,
          `shouldError: ${shouldError}, actualLimit: ${actualLimit}, testValue: ${lastIndex}`
        );
      }
    );
  });

g.test('validate,maxBindGroupsPlusVertexBuffers')
  .desc(`Test that ${limit} <= maxBindGroupsPlusVertexBuffers`)
  .fn(t => {
    const { adapter, defaultLimit, adapterLimit } = t;
    t.expect(defaultLimit <= t.getDefaultLimit('maxBindGroupsPlusVertexBuffers'));
    t.expect(adapterLimit <= adapter.limits.maxBindGroupsPlusVertexBuffers);
  });
