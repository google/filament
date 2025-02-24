export const description = `
Stress tests covering robustness in the presence of non-halting shaders.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('compute')
  .desc(
    `Tests execution of compute passes with non-halting dispatch operations.

This is expected to hang for a bit, but it should ultimately result in graceful
device loss.`
  )
  .fn(async t => {
    const data = new Uint32Array([0]);
    const buffer = t.makeBufferWithContents(data, GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC);
    const module = t.device.createShaderModule({
      code: `
        struct Buffer { data: u32, };
        @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
        @compute @workgroup_size(1) fn main() {
          loop {
            if (buffer.data == 1u) {
              break;
            }
            buffer.data = buffer.data + 2u;
          }
        }
      `,
    });
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
    pass.dispatchWorkgroups(1);
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    await t.device.lost;
  });

g.test('vertex')
  .desc(
    `Tests execution of render passes with a non-halting vertex stage.

This is expected to hang for a bit, but it should ultimately result in graceful
device loss.`
  )
  .fn(async t => {
    const module = t.device.createShaderModule({
      code: `
        struct Data { counter: u32, increment: u32, };
        @group(0) @binding(0) var<uniform> data: Data;
        @vertex fn vmain() -> @builtin(position) vec4<f32> {
          var counter: u32 = data.counter;
          loop {
            if (counter % 2u == 1u) {
              break;
            }
            counter = counter + data.increment;
          }
          return vec4<f32>(1.0, 1.0, 0.0, f32(counter));
        }
        @fragment fn fmain() -> @location(0) vec4<f32> {
          return vec4<f32>(1.0);
        }
      `,
    });

    const pipeline = t.device.createRenderPipeline({
      layout: 'auto',
      vertex: { module, entryPoint: 'vmain', buffers: [] },
      primitive: { topology: 'point-list' },
      fragment: {
        targets: [{ format: 'rgba8unorm' }],
        module,
        entryPoint: 'fmain',
      },
    });
    const uniforms = t.makeBufferWithContents(new Uint32Array([0, 2]), GPUBufferUsage.UNIFORM);
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: { buffer: uniforms },
        },
      ],
    });
    const renderTarget = t.createTextureTracked({
      size: [1, 1],
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
      format: 'rgba8unorm',
    });
    const encoder = t.device.createCommandEncoder();
    const pass = encoder.beginRenderPass({
      colorAttachments: [
        {
          view: renderTarget.createView(),
          clearValue: [0, 0, 0, 0],
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
    });
    pass.setPipeline(pipeline);
    pass.setBindGroup(0, bindGroup);
    pass.draw(1);
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    await t.device.lost;
  });

g.test('fragment')
  .desc(
    `Tests execution of render passes with a non-halting fragment stage.

This is expected to hang for a bit, but it should ultimately result in graceful
device loss.`
  )
  .fn(async t => {
    const module = t.device.createShaderModule({
      code: `
        struct Data { counter: u32, increment: u32, };
        @group(0) @binding(0) var<uniform> data: Data;
        @vertex fn vmain() -> @builtin(position) vec4<f32> {
          return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }
        @fragment fn fmain() -> @location(0) vec4<f32> {
          var counter: u32 = data.counter;
          loop {
            if (counter % 2u == 1u) {
              break;
            }
            counter = counter + data.increment;
          }
          return vec4<f32>(1.0 / f32(counter), 0.0, 0.0, 1.0);
        }
      `,
    });

    const pipeline = t.device.createRenderPipeline({
      layout: 'auto',
      vertex: { module, entryPoint: 'vmain', buffers: [] },
      primitive: { topology: 'point-list' },
      fragment: {
        targets: [{ format: 'rgba8unorm' }],
        module,
        entryPoint: 'fmain',
      },
    });
    const uniforms = t.makeBufferWithContents(new Uint32Array([0, 2]), GPUBufferUsage.UNIFORM);
    const bindGroup = t.device.createBindGroup({
      layout: pipeline.getBindGroupLayout(0),
      entries: [
        {
          binding: 0,
          resource: { buffer: uniforms },
        },
      ],
    });
    const renderTarget = t.createTextureTracked({
      size: [1, 1],
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
      format: 'rgba8unorm',
    });
    const encoder = t.device.createCommandEncoder();
    const pass = encoder.beginRenderPass({
      colorAttachments: [
        {
          view: renderTarget.createView(),
          clearValue: [0, 0, 0, 0],
          loadOp: 'clear',
          storeOp: 'store',
        },
      ],
    });
    pass.setPipeline(pipeline);
    pass.setBindGroup(0, bindGroup);
    pass.draw(1);
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    await t.device.lost;
  });
