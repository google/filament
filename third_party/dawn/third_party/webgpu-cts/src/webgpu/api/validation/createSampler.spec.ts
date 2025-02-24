export const description = `
createSampler validation tests.
`;

import { makeTestGroup } from '../../../common/framework/test_group.js';

import { ValidationTest } from './validation_test.js';

export const g = makeTestGroup(ValidationTest);

g.test('lodMinAndMaxClamp')
  .desc('test different combinations of min and max clamp values')
  .paramsSubcasesOnly(u =>
    u //
      .combine('lodMinClamp', [-4e-30, -1, 0, 0.5, 1, 10, 4e30])
      .combine('lodMaxClamp', [-4e-30, -1, 0, 0.5, 1, 10, 4e30])
  )
  .fn(t => {
    const shouldError =
      t.params.lodMinClamp > t.params.lodMaxClamp ||
      t.params.lodMinClamp < 0 ||
      t.params.lodMaxClamp < 0;
    t.expectValidationError(() => {
      t.device.createSampler({
        lodMinClamp: t.params.lodMinClamp,
        lodMaxClamp: t.params.lodMaxClamp,
      });
    }, shouldError);
  });

g.test('maxAnisotropy')
  .desc('test different maxAnisotropy values and combinations with min/mag/mipmapFilter')
  .params(u =>
    u //
      .beginSubcases()
      .combineWithParams([
        ...u.combine('maxAnisotropy', [-1, undefined, 0, 1, 2, 4, 7, 16, 32, 33, 1024]),
        { minFilter: 'nearest' as const },
        { magFilter: 'nearest' as const },
        { mipmapFilter: 'nearest' as const },
      ])
  )
  .fn(t => {
    const {
      maxAnisotropy = 1,
      minFilter = 'linear',
      magFilter = 'linear',
      mipmapFilter = 'linear',
    } = t.params as {
      maxAnisotropy?: number;
      minFilter?: GPUFilterMode;
      magFilter?: GPUFilterMode;
      mipmapFilter?: GPUFilterMode;
    };

    const shouldError =
      maxAnisotropy < 1 ||
      (maxAnisotropy > 1 &&
        !(minFilter === 'linear' && magFilter === 'linear' && mipmapFilter === 'linear'));
    t.expectValidationError(() => {
      t.device.createSampler({
        minFilter,
        magFilter,
        mipmapFilter,
        maxAnisotropy,
      });
    }, shouldError);
  });
