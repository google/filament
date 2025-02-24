export const description = `
Stress tests for timestamp queries.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('command_encoder_one_query_set')
  .desc(
    `Tests a huge number of timestamp queries over a single query set between render
passes on a single command encoder.`
  )
  .unimplemented();

g.test('command_encoder_many_query_sets')
  .desc(
    `Tests a huge number of timestamp queries over many query sets between render
passes on a single command encoder.`
  )
  .unimplemented();

g.test('render_pass_one_query_set')
  .desc(
    `Tests a huge number of timestamp queries over a single query set in a single
render pass.`
  )
  .unimplemented();

g.test('render_pass_many_query_sets')
  .desc(
    `Tests a huge number of timestamp queries over a huge number of query sets in a
single render pass.`
  )
  .unimplemented();

g.test('compute_pass_one_query_set')
  .desc(
    `Tests a huge number of timestamp queries over a single query set in a single
compute pass.`
  )
  .unimplemented();

g.test('compute_pass_many_query_sets')
  .desc(
    `Tests a huge number of timestamp queries over a huge number of query sets in a
single compute pass.`
  )
  .unimplemented();
