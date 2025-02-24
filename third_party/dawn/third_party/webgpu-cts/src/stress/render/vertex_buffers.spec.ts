export const description = `
Stress tests covering vertex buffer usage.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

function createHugeVertexBuffer(t: GPUTest, size: number) {
  const kBufferSize = size * size * 8;
  const buffer = t.createBufferTracked({
    size: kBufferSize,
    usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC,
  });
  const pipeline = t.device.createComputePipeline({
    layout: 'auto',
    compute: {
      module: t.device.createShaderModule({
        code: `
        struct Buffer { data: array<vec2<u32>>, };
        @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
        @compute @workgroup_size(1) fn main(
            @builtin(global_invocation_id) id: vec3<u32>) {
          let base = id.x * ${size}u;
          for (var x: u32 = 0u; x < ${size}u; x = x + 1u) {
            buffer.data[base + x] = vec2<u32>(x, id.x);
          }
        }
        `,
      }),
      entryPoint: 'main',
    },
  });
  const bindGroup = t.device.createBindGroup({
    layout: pipeline.getBindGroupLayout(0),
    entries: [
      {
        binding: 0,
        resource: { buffer },
      },
    ],
  });
  const encoder = t.device.createCommandEncoder();
  const pass = encoder.beginComputePass();
  pass.setPipeline(pipeline);
  pass.setBindGroup(0, bindGroup);
  pass.dispatchWorkgroups(size);
  pass.end();

  const vertexBuffer = t.createBufferTracked({
    size: kBufferSize,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST,
  });
  encoder.copyBufferToBuffer(buffer, 0, vertexBuffer, 0, kBufferSize);
  t.device.queue.submit([encoder.finish()]);
  return vertexBuffer;
}

g.test('many')
  .desc(`Tests execution of draw calls using a huge vertex buffer.`)
  .fn(t => {
    const kSize = 4096;
    const buffer = createHugeVertexBuffer(t, kSize);
    const module = t.device.createShaderModule({
      code: `
    @vertex fn vmain(@location(0) position: vec2<u32>)
        -> @builtin(position) vec4<f32> {
      let r = vec2<f32>(1.0 / f32(${kSize}));
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(vec2<f32>(position), a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
    }
    `,
    });
    const pipeline = t.device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        module,
        entryPoint: 'vmain',
        buffers: [
          {
            arrayStride: 8,
            attributes: [
              {
                format: 'uint32x2',
                offset: 0,
                shaderLocation: 0,
              },
            ],
          },
        ],
      },
      primitive: { topology: 'point-list' },
      fragment: {
        targets: [{ format: 'rgba8unorm' }],
        module,
        entryPoint: 'fmain',
      },
    });
    const renderTarget = t.createTextureTracked({
      size: [kSize, kSize],
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
      format: 'rgba8unorm',
    });
    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: renderTarget.createView(),
          loadOp: 'load',
          storeOp: 'store',
        },
      ],
    };

    const encoder = t.device.createCommandEncoder();
    const pass = encoder.beginRenderPass(renderPassDescriptor);
    pass.setPipeline(pipeline);
    pass.setVertexBuffer(0, buffer);
    pass.draw(kSize * kSize);
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kSize, kSize, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });
