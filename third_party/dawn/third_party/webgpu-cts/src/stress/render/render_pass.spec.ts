export const description = `
Stress tests covering GPURenderPassEncoder usage.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { range } from '../../common/util/util.js';
import { GPUTest } from '../../webgpu/gpu_test.js';

export const g = makeTestGroup(GPUTest);

g.test('many')
  .desc(
    `Tests execution of a huge number of render passes using the same GPURenderPipeline. This uses
a single render pass for every output fragment, with each pass executing a one-vertex draw call.`
  )
  .fn(t => {
    const kSize = 1024;
    const module = t.device.createShaderModule({
      code: `
    @vertex fn vmain(@builtin(vertex_index) index: u32)
        -> @builtin(position) vec4<f32> {
      let position = vec2<f32>(f32(index % ${kSize}u), f32(index / ${kSize}u));
      let r = vec2<f32>(1.0 / f32(${kSize}));
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(position, a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
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
    range(kSize * kSize, i => {
      const pass = encoder.beginRenderPass(renderPassDescriptor);
      pass.setPipeline(pipeline);
      pass.draw(1, 1, i);
      pass.end();
    });
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kSize, kSize, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });

g.test('pipeline_churn')
  .desc(
    `Tests execution of a large number of render pipelines, each within its own render pass. Each
pass does a single draw call, with one pass per output fragment.`
  )
  .fn(t => {
    const kWidth = 64;
    const kHeight = 8;
    const module = t.device.createShaderModule({
      code: `
    @vertex fn vmain(@builtin(vertex_index) index: u32)
        -> @builtin(position) vec4<f32> {
      let position = vec2<f32>(f32(index % ${kWidth}u), f32(index / ${kWidth}u));
      let size = vec2<f32>(f32(${kWidth}), f32(${kHeight}));
      let r = vec2<f32>(1.0) / size;
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(position, a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
    }
    `,
    });
    const renderTarget = t.createTextureTracked({
      size: [kWidth, kHeight],
      usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
      format: 'rgba8unorm',
    });
    const depthTarget = t.createTextureTracked({
      size: [kWidth, kHeight],
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
      format: 'depth24plus-stencil8',
    });
    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: renderTarget.createView(),
          loadOp: 'load',
          storeOp: 'store',
        },
      ],
      depthStencilAttachment: {
        view: depthTarget.createView(),
        depthLoadOp: 'load',
        depthStoreOp: 'store',
        stencilLoadOp: 'load',
        stencilStoreOp: 'discard',
      },
    };
    const encoder = t.device.createCommandEncoder();
    range(kWidth * kHeight, i => {
      const pipeline = t.device.createRenderPipeline({
        layout: 'auto',
        vertex: { module, entryPoint: 'vmain', buffers: [] },
        primitive: { topology: 'point-list' },
        depthStencil: {
          format: 'depth24plus-stencil8',
          depthCompare: 'always',
          depthWriteEnabled: false,
          // Not really used, but it ensures that each pipeline is unique.
          depthBias: i,
        },
        fragment: {
          targets: [{ format: 'rgba8unorm' }],
          module,
          entryPoint: 'fmain',
        },
      });
      const pass = encoder.beginRenderPass(renderPassDescriptor);
      pass.setPipeline(pipeline);
      pass.draw(1, 1, i);
      pass.end();
    });
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kWidth, kHeight, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });

g.test('bind_group_churn')
  .desc(
    `Tests execution of render passes which switch between a huge number of bind groups. This uses
a single render pass with a single pipeline, and one draw call per fragment of the output texture.
Each draw call is made with a unique bind group 0, with binding 0 referencing a unique uniform
buffer.`
  )
  .fn(t => {
    const kSize = 128;
    const module = t.device.createShaderModule({
      code: `
    struct Uniforms { index: u32, };
    @group(0) @binding(0) var<uniform> uniforms: Uniforms;
    @vertex fn vmain() -> @builtin(position) vec4<f32> {
      let index = uniforms.index;
      let position = vec2<f32>(f32(index % ${kSize}u), f32(index / ${kSize}u));
      let r = vec2<f32>(1.0 / f32(${kSize}));
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(position, a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
    }
    `,
    });
    const layout = t.device.createBindGroupLayout({
      entries: [
        {
          binding: 0,
          visibility: GPUShaderStage.VERTEX,
          buffer: { type: 'uniform' },
        },
      ],
    });
    const pipeline = t.device.createRenderPipeline({
      layout: t.device.createPipelineLayout({ bindGroupLayouts: [layout] }),
      vertex: { module, entryPoint: 'vmain', buffers: [] },
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
    range(kSize * kSize, i => {
      const buffer = t.createBufferTracked({
        size: 4,
        usage: GPUBufferUsage.UNIFORM,
        mappedAtCreation: true,
      });
      new Uint32Array(buffer.getMappedRange())[0] = i;
      buffer.unmap();
      pass.setBindGroup(
        0,
        t.device.createBindGroup({ layout, entries: [{ binding: 0, resource: { buffer } }] })
      );
      pass.draw(1, 1);
    });
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kSize, kSize, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });

g.test('many_draws')
  .desc(
    `Tests execution of render passes with a huge number of draw calls. This uses a single
render pass with a single pipeline, and one draw call per fragment of the output texture.`
  )
  .fn(t => {
    const kSize = 4096;
    const module = t.device.createShaderModule({
      code: `
    @vertex fn vmain(@builtin(vertex_index) index: u32)
        -> @builtin(position) vec4<f32> {
      let position = vec2<f32>(f32(index % ${kSize}u), f32(index / ${kSize}u));
      let r = vec2<f32>(1.0 / f32(${kSize}));
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(position, a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
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
    range(kSize * kSize, i => pass.draw(1, 1, i));
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kSize, kSize, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });

g.test('huge_draws')
  .desc(
    `Tests execution of several render passes with huge draw calls. Each pass uses a single draw
call which draws multiple vertices for each fragment of a large output texture.`
  )
  .fn(t => {
    const kSize = 32768;
    const kTextureSize = 4096;
    const kVertsPerFragment = (kSize * kSize) / (kTextureSize * kTextureSize);
    const module = t.device.createShaderModule({
      code: `
    @vertex fn vmain(@builtin(vertex_index) vert_index: u32)
        -> @builtin(position) vec4<f32> {
      let index = vert_index / ${kVertsPerFragment}u;
      let position = vec2<f32>(f32(index % ${kTextureSize}u), f32(index / ${kTextureSize}u));
      let r = vec2<f32>(1.0 / f32(${kTextureSize}));
      let a = 2.0 * r;
      let b = r - vec2<f32>(1.0);
      return vec4<f32>(fma(position, a, b), 0.0, 1.0);
    }
    @fragment fn fmain() -> @location(0) vec4<f32> {
      return vec4<f32>(1.0, 0.0, 1.0, 1.0);
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
    const renderTarget = t.createTextureTracked({
      size: [kTextureSize, kTextureSize],
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
    pass.draw(kSize * kSize);
    pass.end();
    t.device.queue.submit([encoder.finish()]);
    t.expectSingleColor(renderTarget, 'rgba8unorm', {
      size: [kTextureSize, kTextureSize, 1],
      exp: { R: 1, G: 0, B: 1, A: 1 },
    });
  });
