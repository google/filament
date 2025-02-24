export const description = `
Stress tests covering GPUComputePassEncoder usage.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { assert, iterRange } from '../../common/util/util.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('many')
  .desc(
    `Tests execution of a huge number of compute passes using the same
GPUComputePipeline.`
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
    const kNumIterations = 250_000;
    for (let i = 0; i < kNumIterations; ++i) {
      const encoder = t.device.createCommandEncoder();
      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(kNumElements);
      pass.end();
      t.device.queue.submit([encoder.finish()]);
    }
    t.expectGPUBufferValuesEqual(
      buffer,
      new Uint32Array([...iterRange(kNumElements, x => x + kNumIterations)])
    );
  });

g.test('pipeline_churn')
  .desc(
    `Tests execution of a huge number of compute passes which each use a different
GPUComputePipeline.`
  )
  .fn(t => {
    const buffer = t.makeBufferWithContents(
      new Uint32Array([0]),
      GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC
    );
    const kNumIterations = 10_000;
    const stages = iterRange(kNumIterations, i => ({
      module: t.device.createShaderModule({
        code: `
        struct Buffer { data: u32, };
        @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
        @compute @workgroup_size(1) fn main${i}() {
          buffer.data = buffer.data + 1u;
        }
        `,
      }),
      entryPoint: `main${i}`,
    }));
    for (const compute of stages) {
      const encoder = t.device.createCommandEncoder();
      const pipeline = t.device.createComputePipeline({ layout: 'auto', compute });
      const bindGroup = t.device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [{ binding: 0, resource: { buffer } }],
      });
      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(1);
      pass.end();
      t.device.queue.submit([encoder.finish()]);
    }
    t.expectGPUBufferValuesEqual(buffer, new Uint32Array([kNumIterations]));
  });

g.test('bind_group_churn')
  .desc(
    `Tests execution of compute passes which switch between a huge number of bind
groups.`
  )
  .fn(t => {
    const kNumElements = 64;
    const data = new Uint32Array([...iterRange(kNumElements, x => x)]);
    const buffer1 = t.makeBufferWithContents(
      data,
      GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC
    );
    const buffer2 = t.makeBufferWithContents(
      data,
      GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC
    );
    const module = t.device.createShaderModule({
      code: `
        struct Buffer { data: array<u32>, };
        @group(0) @binding(0) var<storage, read_write> buffer1: Buffer;
        @group(0) @binding(1) var<storage, read_write> buffer2: Buffer;
        @compute @workgroup_size(1) fn main(
            @builtin(global_invocation_id) id: vec3<u32>) {
          buffer1.data[id.x] = buffer1.data[id.x] + 1u;
          buffer2.data[id.x] = buffer2.data[id.x] + 2u;
        }
      `,
    });
    const kNumIterations = 250_000;
    const pipeline = t.device.createComputePipeline({
      layout: 'auto',
      compute: { module, entryPoint: 'main' },
    });
    const encoder = t.device.createCommandEncoder();
    const pass = encoder.beginComputePass();
    pass.setPipeline(pipeline);
    for (let i = 0; i < kNumIterations; ++i) {
      const buffer1Binding = i % 2;
      const buffer2Binding = buffer1Binding ^ 1;
      const bindGroup = t.device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
          { binding: buffer1Binding, resource: { buffer: buffer1 } },
          { binding: buffer2Binding, resource: { buffer: buffer2 } },
        ],
      });
      pass.setBindGroup(0, bindGroup);
      pass.dispatchWorkgroups(kNumElements);
    }
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    const kTotalAddition = (kNumIterations / 2) * 3;
    t.expectGPUBufferValuesEqual(
      buffer1,
      new Uint32Array([...iterRange(kNumElements, x => x + kTotalAddition)])
    );
    t.expectGPUBufferValuesEqual(
      buffer2,
      new Uint32Array([...iterRange(kNumElements, x => x + kTotalAddition)])
    );
  });

g.test('many_dispatches')
  .desc(`Tests execution of compute passes with a huge number of dispatch calls`)
  .fn(t => {
    const kNumElements = 64;
    const data = new Uint32Array([...iterRange(kNumElements, x => x)]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);
    const module = t.device.createShaderModule({
      code: `
        struct Buffer { data: array<u32>, };
        @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
        @compute @workgroup_size(1) fn main(
            @builtin(global_invocation_id) id: vec3<u32>) {
          buffer.data[id.x] = buffer.data[id.x] + 1u;
        }
      `,
    });
    const kNumIterations = 1_000_000;
    const pipeline = t.device.createComputePipeline({
      layout: 'auto',
      compute: { module, entryPoint: 'main' },
    });
    const encoder = t.device.createCommandEncoder();
    const pass = encoder.beginComputePass();
    pass.setPipeline(pipeline);
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [{ binding: 0, resource: { buffer } }],
    });
    pass.setBindGroup(0, bindGroup);
    for (let i = 0; i < kNumIterations; ++i) {
      pass.dispatchWorkgroups(kNumElements);
    }
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    t.expectGPUBufferValuesEqual(
      buffer,
      new Uint32Array([...iterRange(kNumElements, x => x + kNumIterations)])
    );
  });

g.test('huge_dispatches')
  .desc(`Tests execution of compute passes with huge dispatch calls`)
  .fn(async t => {
    const kDimensions = [512, 512, 128];
    kDimensions.forEach(x => {
      assert(x <= t.device.limits.maxComputeWorkgroupsPerDimension);
    });

    const kNumElements = kDimensions[0] * kDimensions[1] * kDimensions[2];
    const data = new Uint32Array([...iterRange(kNumElements, x => x)]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);
    const module = t.device.createShaderModule({
      code: `
        struct Buffer { data: array<u32>, };
        @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
        @compute @workgroup_size(1) fn main(
            @builtin(global_invocation_id) id: vec3<u32>) {
          let index = (id.z * 512u + id.y) * 512u + id.x;
          buffer.data[index] = buffer.data[index] + 1u;
        }
      `,
    });
    const kNumIterations = 16;
    const pipeline = t.device.createComputePipeline({
      layout: 'auto',
      compute: { module, entryPoint: 'main' },
    });
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [{ binding: 0, resource: { buffer } }],
    });
    for (let i = 0; i < kNumIterations; ++i) {
      const encoder = t.device.createCommandEncoder();
      const pass = encoder.beginComputePass();
      pass.setBindGroup(0, bindGroup);
      pass.setPipeline(pipeline);
      pass.dispatchWorkgroups(kDimensions[0], kDimensions[1], kDimensions[2]);
      pass.end();
      t.device.queue.submit([encoder.finish()]);
      await t.device.queue.onSubmittedWorkDone();
    }
    t.expectGPUBufferValuesEqual(
      buffer,
      new Uint32Array([...iterRange(kNumElements, x => x + kNumIterations)])
    );
  });
