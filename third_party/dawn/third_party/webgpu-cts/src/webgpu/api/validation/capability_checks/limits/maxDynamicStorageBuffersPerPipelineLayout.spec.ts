import { assert, range } from '../../../../../common/util/util.js';
import { kShaderStageCombinationsWithStage } from '../../../../capability_info.js';
import { GPUConst } from '../../../../constants.js';

import { kMaximumLimitBaseParams, LimitsRequest, makeLimitTestGroup } from './limit_utils.js';

const kExtraLimits: LimitsRequest = {
  maxBindingsPerBindGroup: 'adapterLimit',
  maxBindGroups: 'adapterLimit',
  maxStorageBuffersPerShaderStage: 'adapterLimit',
  maxStorageBuffersInFragmentStage: 'adapterLimit',
  maxStorageBuffersInVertexStage: 'adapterLimit',
};

const limit = 'maxDynamicStorageBuffersPerPipelineLayout';
export const { g, description } = makeLimitTestGroup(limit);

g.test('createBindGroupLayout,at_over')
  .desc(`Test using createBindGroupLayout at and over ${limit} limit`)
  .params(
    kMaximumLimitBaseParams
      .combine('visibility', kShaderStageCombinationsWithStage)
      .combine('type', ['storage', 'read-only-storage'] as GPUBufferBindingType[])
      .filter(
        ({ visibility, type }) =>
          (visibility & GPUConst.ShaderStage.VERTEX) === 0 || type !== 'storage'
      )
  )
  .fn(async t => {
    const { limitTest, testValueName, visibility, type } = t.params;
    await t.testDeviceWithRequestedMaximumLimits(
      limitTest,
      testValueName,
      async ({ device, testValue, shouldError }) => {
        t.skipIfNotEnoughStorageBuffersInStage(visibility, testValue);
        shouldError ||= testValue > t.device.limits.maxStorageBuffersPerShaderStage;
        await t.expectValidationError(() => {
          device.createBindGroupLayout({
            entries: range(testValue, i => ({
              binding: i,
              visibility,
              buffer: {
                type,
                hasDynamicOffset: true,
              },
            })),
          });
        }, shouldError);
      },
      kExtraLimits
    );
  });

g.test('createPipelineLayout,at_over')
  .desc(`Test using at and over ${limit} limit in createPipelineLayout`)
  .params(
    kMaximumLimitBaseParams
      .combine('visibility', kShaderStageCombinationsWithStage)
      .combine('type', ['storage', 'read-only-storage'] as GPUBufferBindingType[])
      .filter(
        ({ visibility, type }) =>
          (visibility & GPUConst.ShaderStage.VERTEX) === 0 || type !== 'storage'
      )
  )
  .fn(async t => {
    const { limitTest, testValueName, visibility, type } = t.params;

    await t.testDeviceWithRequestedMaximumLimits(
      limitTest,
      testValueName,
      async ({ device, testValue, shouldError, actualLimit }) => {
        t.skipIfNotEnoughStorageBuffersInStage(visibility, testValue);

        const maxBindingsPerBindGroup = Math.min(
          t.device.limits.maxBindingsPerBindGroup,
          actualLimit
        );

        const kNumGroups = Math.ceil(testValue / maxBindingsPerBindGroup);

        // Not sure what to do in this case but best we get notified if it happens.
        assert(kNumGroups <= t.device.limits.maxBindGroups);

        const bindGroupLayouts = range(kNumGroups, i => {
          const numInGroup = Math.min(
            testValue - i * maxBindingsPerBindGroup,
            maxBindingsPerBindGroup
          );
          return device.createBindGroupLayout({
            entries: range(numInGroup, i => ({
              binding: i,
              visibility,
              buffer: {
                type,
                hasDynamicOffset: true,
              },
            })),
          });
        });

        await t.expectValidationError(
          () => device.createPipelineLayout({ bindGroupLayouts }),
          shouldError
        );
      },
      kExtraLimits
    );
  });
