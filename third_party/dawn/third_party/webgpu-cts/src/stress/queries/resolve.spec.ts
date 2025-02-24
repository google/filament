export const description = `
Stress tests for query resolution.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('many_large_sets')
  .desc(
    `Tests a huge number of resolveQuerySet operations on a huge number of
query sets between render passes.`
  )
  .unimplemented();
