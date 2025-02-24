export const description = `
Stress tests for allocation of GPUQuerySet objects through GPUDevice.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('coexisting')
  .desc(`Tests allocation of many coexisting GPUQuerySet objects.`)
  .unimplemented();

g.test('continuous,with_destroy')
  .desc(
    `Tests allocation and destruction of many GPUQuerySet objects over time. Objects
are sequentially created and destroyed over a very large number of iterations.`
  )
  .unimplemented();

g.test('continuous,no_destroy')
  .desc(
    `Tests allocation and implicit GC of many GPUQuerySet objects over time. Objects
are sequentially created and dropped for GC over a very large number of
iterations.`
  )
  .unimplemented();
