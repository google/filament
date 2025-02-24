export const description = `Test you can request an device with all features and limits`;

import { TestCaseRecorder, TestParams } from '../../../../common/framework/fixture.js';
import { makeTestGroup } from '../../../../common/framework/test_group.js';
import {
  DeviceSelectionDescriptor,
  GPUTest,
  GPUTestSubcaseBatchState,
  initUncanonicalizedDeviceDescriptor,
} from '../../../gpu_test.js';
import { CanonicalDeviceDescriptor, DescriptorModifier } from '../../../util/device_pool.js';

/**
 * Gets the adapter limits as a standard JavaScript object.
 * MAINTENANCE_TODO: Remove this and use the same function from gpu_test.ts once minSubgroupSize is removed
 * The reason this is separate now is we want this test to fail. `mnSubgroupSize` should never have
 * be added and this test exists to see that the same mistake doesn't happen in the future.
 */
function getAdapterLimitsAsDeviceRequiredLimits(adapter: GPUAdapter) {
  const requiredLimits: Record<string, GPUSize64> = {};
  const adapterLimits = adapter.limits as unknown as Record<string, GPUSize64>;
  for (const key in adapter.limits) {
    requiredLimits[key] = adapterLimits[key];
  }
  return requiredLimits;
}

function setAllLimitsToAdapterLimitsAndAddAllFeatures(
  adapter: GPUAdapter,
  desc: CanonicalDeviceDescriptor | undefined
) {
  const descWithMaxLimits: CanonicalDeviceDescriptor = {
    defaultQueue: {},
    ...desc,
    requiredFeatures: [...adapter.features] as GPUFeatureName[],
    requiredLimits: getAdapterLimitsAsDeviceRequiredLimits(adapter),
  };
  return descWithMaxLimits;
}

/**
 * Used to request a device with all the max limits of the adapter.
 */
export class AllLimitsAndFeaturesGPUTestSubcaseBatchState extends GPUTestSubcaseBatchState {
  override requestDeviceWithRequiredParametersOrSkip(
    descriptor: DeviceSelectionDescriptor,
    descriptorModifier?: DescriptorModifier
  ): void {
    const mod: DescriptorModifier = {
      descriptorModifier(adapter: GPUAdapter, desc: CanonicalDeviceDescriptor | undefined) {
        desc = descriptorModifier?.descriptorModifier
          ? descriptorModifier.descriptorModifier(adapter, desc)
          : desc;
        return setAllLimitsToAdapterLimitsAndAddAllFeatures(adapter, desc);
      },
      keyModifier(baseKey: string) {
        return `${baseKey}:AllLimitsAndFeaturesTest`;
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
export class AllLimitsAndFeaturesLimitsTest extends GPUTest {
  public static override MakeSharedState(
    recorder: TestCaseRecorder,
    params: TestParams
  ): GPUTestSubcaseBatchState {
    return new AllLimitsAndFeaturesGPUTestSubcaseBatchState(recorder, params);
  }
}

export const g = makeTestGroup(AllLimitsAndFeaturesLimitsTest);

g.test('everything')
  .desc(
    `
Test we can request all features and limits.

It is expected that, even though this is generally not recommended, because
it is possible, make sure it works and continues to work going forward so that
changes to WebGPU do not break sites requesting everything.
`
  )
  .fn(t => {
    // Test that all the limits on the device match the adapter.
    const adapterLimits = t.adapter.limits as unknown as Record<string, GPUSize64>;
    const deviceLimits = t.device.limits as unknown as Record<string, GPUSize64>;
    for (const key in t.adapter.limits) {
      const deviceLimit = deviceLimits[key];
      const adapterLimit = adapterLimits[key];
      t.expect(
        deviceLimit === adapterLimit,
        `device.limits.${key} (${deviceLimit}) === adapter.limits.${key} (${adapterLimit})`
      );
    }

    // Test that all the adapter features are on the device.
    for (const feature of t.adapter.features) {
      t.expect(t.device.features.has(feature), `device has feature: ${feature}`);
    }
  });
