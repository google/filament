export const description = `
Stress tests for command submission to GPUQueue objects.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { iterRange } from '../../common/util/util.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('huge_command_buffer')
  .desc(
    `Tests submission of huge command buffers to a GPUQueue. Huge buffers are
encoded by chaining together long sequences of compute passes, with expected
results verified at the end of the test.`
  )
  .fn(t => {
    const kNumElements = 64;
    const data = new Uint32Array([...iterRange(kNumElements, x => x)]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);
    const pipeline = t.device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: t.device.createShaderModule({
          code: `
            struct Buffer { data: array<u32>, };
            @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
            @compute @workgroup_size(1) fn main(
                @builtin(global_invocation_id) id: vec3<u32>) {
              buffer.data[id.x] = buffer.data[id.x] + 1u;
            }
          `,
        }),
        entryPoint: 'main',
      },
    });
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [{ binding: 0, resource: { buffer } }],
    });
    const encoder = t.device.createCommandEncoder();
    const kNumIterations = 500_000;
    for (let i = 0; i < kNumIterations; ++i) {
      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(kNumElements);
      pass.end();
    }
    t.device.queue.submit([encoder.finish()]);
    t.expectGPUBufferValuesEqual(
      buffer,
      new Uint32Array([...iterRange(kNumElements, x => x + kNumIterations)])
    );
  });

g.test('many_command_buffers')
  .desc(
    `Tests submission of a huge number of command buffers to a GPUQueue by a single
submit() call.`
  )
  .fn(t => {
    const kNumElements = 64;
    const data = new Uint32Array([...iterRange(kNumElements, x => x)]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);
    const pipeline = t.device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: t.device.createShaderModule({
          code: `
            struct Buffer { data: array<u32>, };
            @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
            @compute @workgroup_size(1) fn main(
                @builtin(global_invocation_id) id: vec3<u32>) {
              buffer.data[id.x] = buffer.data[id.x] + 1u;
            }
          `,
        }),
        entryPoint: 'main',
      },
    });
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [{ binding: 0, resource: { buffer } }],
    });
    const kNumIterations = 500_000;
    const buffers = [];
    for (let i = 0; i < kNumIterations; ++i) {
      const encoder = t.device.createCommandEncoder();
      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(kNumElements);
      pass.end();
      buffers.push(encoder.finish());
    }
    t.device.queue.submit(buffers);
    t.expectGPUBufferValuesEqual(
      buffer,
      new Uint32Array([...iterRange(kNumElements, x => x + kNumIterations)])
    );
  });
