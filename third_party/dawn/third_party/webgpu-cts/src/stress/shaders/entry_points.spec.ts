export const description = `
Stress tests covering behavior around shader entry points.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { range } from '../../common/util/util.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

const makeCode = (numEntryPoints: number) => {
  const kBaseCode = `
      struct Buffer { data: u32, };
      @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
      fn main() { buffer.data = buffer.data + 1u;  }
      `;
  const makeEntryPoint = (i: number) => `
      @compute @workgroup_size(1) fn computeMain${i}() { main(); }
      `;
  return kBaseCode + range(numEntryPoints, makeEntryPoint).join('');
};

g.test('many')
  .desc(
    `Tests compilation and usage of shaders with a huge number of entry points.

TODO: There may be a normative limit to the number of entry points allowed in
a shader, in which case this would become a validation test instead.`
  )
  .fn(t => {
    const data = new Uint32Array([0]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);

    // NOTE: Initial shader compilation time seems to scale exponentially with
    // this value in Chrome.
    const kNumEntryPoints = 200;

    const shader = t.device.createShaderModule({
      code: makeCode(kNumEntryPoints),
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
    const pipelineLayout = t.device.createPipelineLayout({
      bindGroupLayouts: [layout],
    });
    const bindGroup = t.device.createBindGroup({
      layout,
      entries: [{ binding: 0, resource: { buffer } }],
    });

    const encoder = t.device.createCommandEncoder();
    range(kNumEntryPoints, i => {
      const pipeline = t.device.createComputePipeline({
        layout: pipelineLayout,
        compute: {
          module: shader,
          entryPoint: `computeMain${i}`,
        },
      });

      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(1);
      pass.end();
    });

    t.device.queue.submit([encoder.finish()]);
    t.expectGPUBufferValuesEqual(buffer, new Uint32Array([kNumEntryPoints]));
  });
