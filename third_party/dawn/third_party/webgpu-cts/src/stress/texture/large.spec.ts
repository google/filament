export const description = `
Stress tests covering usage of very large textures.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('loading,2d')
  .desc(
    `Tests execution of shaders loading values from very large (up to at least
8192x8192) 2D textures. The texture size is selected according to the limit
supported by the GPUDevice.`
  )
  .unimplemented();

g.test('loading,2d_array')
  .desc(
    `Tests execution of shaders loading values from very large (up to at least
8192x8192x2048) arrays of 2D textures. The texture and array size is selected
according to limits supported by the GPUDevice.`
  )
  .unimplemented();

g.test('loading,3d')
  .desc(
    `Tests execution of shaders loading values from very large (up to at least
2048x2048x2048) textures. The texture size is selected according to the limit
supported by the GPUDevice.`
  )
  .unimplemented();

g.test('sampling,2d')
  .desc(
    `Tests execution of shaders sampling values from very large (up to at least
8192x8192) 2D textures. The texture size is selected according to the limit
supported by the GPUDevice.`
  )
  .unimplemented();

g.test('sampling,2d_array')
  .desc(
    `Tests execution of shaders sampling values from very large (up to at least
8192x8192x2048) arrays of 2D textures. The texture and array size is selected
according to limits supported by the GPUDevice.`
  )
  .unimplemented();

g.test('sampling,3d')
  .desc(
    `Tests execution of shaders sampling values from very large (up to at least
2048x2048x2048) textures. The texture size is selected according to the limit
supported by the GPUDevice.`
  )
  .unimplemented();
