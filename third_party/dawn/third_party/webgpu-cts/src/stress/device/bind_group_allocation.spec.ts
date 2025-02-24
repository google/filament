export const description = `
Stress tests for allocation of GPUBindGroup objects through GPUDevice.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('coexisting')
  .desc(`Tests allocation of many coexisting GPUBindGroup objects.`)
  .fn(t => {
    const kNumGroups = 1_000_000;
    const buffer = t.createBufferTracked({
      size: 64,
      usage: GPUBufferUsage.STORAGE,
    });
    const layout = t.device.createBindGroupLayout({
      entries: [
        {
          binding: 0,
          visibility: GPUShaderStage.COMPUTE,
          buffer: { type: 'storage' },
        },
      ],
    });
    const bindGroups = [];
    for (let i = 0; i < kNumGroups; ++i) {
      bindGroups.push(
        t.device.createBindGroup({
          layout,
          entries: [{ binding: 0, resource: { buffer } }],
        })
      );
    }
  });

g.test('continuous')
  .desc(
    `Tests allocation and implicit GC of many GPUBindGroup objects over time.
Objects are sequentially created and dropped for GC over a very large number of
iterations.`
  )
  .fn(t => {
    const kNumGroups = 5_000_000;
    const buffer = t.createBufferTracked({
      size: 64,
      usage: GPUBufferUsage.STORAGE,
    });
    const layout = t.device.createBindGroupLayout({
      entries: [
        {
          binding: 0,
          visibility: GPUShaderStage.COMPUTE,
          buffer: { type: 'storage' },
        },
      ],
    });
    for (let i = 0; i < kNumGroups; ++i) {
      t.device.createBindGroup({
        layout,
        entries: [{ binding: 0, resource: { buffer } }],
      });
    }
  });
