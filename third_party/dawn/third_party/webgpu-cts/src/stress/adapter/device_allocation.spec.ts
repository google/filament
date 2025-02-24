export const description = `
Stress tests for GPUAdapter.requestDevice.
`;

import { Fixture } from '../../common/framework/fixture.js';
import { makeTestGroup } from '../../common/framework/test_group.js';
import { attemptGarbageCollection } from '../../common/util/collect_garbage.js';
import { keysOf } from '../../common/util/data_tables.js';
import { getGPU } from '../../common/util/navigator_gpu.js';
import { assert, iterRange } from '../../common/util/util.js';
import { getDefaultLimitsForAdapter } from '../../webgpu/capability_info.js';

export const g = makeTestGroup(Fixture);

/** Adapter preference identifier to option. */
const kAdapterTypeOptions: {
  readonly [k in GPUPowerPreference | 'fallback']: GPURequestAdapterOptions;
} =
  /* prettier-ignore */ {
  'low-power':        { powerPreference:        'low-power', forceFallbackAdapter: false },
  'high-performance': { powerPreference: 'high-performance', forceFallbackAdapter: false },
  'fallback':         { powerPreference:          undefined, forceFallbackAdapter:  true },
};
/** List of all adapter hint types. */
const kAdapterTypes = keysOf(kAdapterTypeOptions);

/**
 * Creates a device, a valid compute pipeline, valid resources for the pipeline, and
 * ties them together into a set of compute commands ready to be submitted to the GPU
 * queue. Does not submit the commands in order to make sure that all resources are
 * kept alive until the device is destroyed.
 */
async function createDeviceAndComputeCommands(t: Fixture, adapter: GPUAdapter) {
  // Constants are computed such that per run, this function should allocate roughly 2G
  // worth of data. This should be sufficient as we run these creation functions many
  // times. If the data backing the created objects is not recycled we should OOM.
  const limitInfo = getDefaultLimitsForAdapter(adapter);
  const kNumPipelines = 64;
  const kNumBindgroups = 128;
  const kNumBufferElements =
    limitInfo.maxComputeWorkgroupSizeX.default * limitInfo.maxComputeWorkgroupSizeY.default;
  const kBufferSize = kNumBufferElements * 4;
  const kBufferData = new Uint32Array([...iterRange(kNumBufferElements, x => x)]);

  const device: GPUDevice = await t.requestDeviceTracked(adapter);
  const commands = [];

  for (let pipelineIndex = 0; pipelineIndex < kNumPipelines; ++pipelineIndex) {
    const pipeline = device.createComputePipeline({
      layout: 'auto',
      compute: {
        module: device.createShaderModule({
          code: `
              struct Buffer { data: array<u32>, };

              @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
              @compute @workgroup_size(1) fn main(
                  @builtin(global_invocation_id) id: vec3<u32>) {
                buffer.data[id.x * ${limitInfo.maxComputeWorkgroupSizeX.default}u + id.y] =
                  buffer.data[id.x * ${limitInfo.maxComputeWorkgroupSizeX.default}u + id.y] +
                    ${pipelineIndex}u;
              }
            `,
        }),
        entryPoint: 'main',
      },
    });
    for (let bindgroupIndex = 0; bindgroupIndex < kNumBindgroups; ++bindgroupIndex) {
      const buffer = t.trackForCleanup(
        device.createBuffer({
          size: kBufferSize,
          usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
        })
      );
      device.queue.writeBuffer(buffer, 0, kBufferData, 0, kBufferData.length);
      const bindgroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [{ binding: 0, resource: { buffer } }],
      });

      const encoder = device.createCommandEncoder();
      const pass = encoder.beginComputePass();
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindgroup);
      pass.dispatchWorkgroups(
        limitInfo.maxComputeWorkgroupSizeX.default,
        limitInfo.maxComputeWorkgroupSizeY.default
      );
      pass.end();
      commands.push(encoder.finish());
    }
  }
  return { device, objects: commands };
}

/**
 * Creates a device, a valid render pipeline, valid resources for the pipeline, and
 * ties them together into a set of render commands ready to be submitted to the GPU
 * queue. Does not submit the commands in order to make sure that all resources are
 * kept alive until the device is destroyed.
 */
async function createDeviceAndRenderCommands(t: Fixture, adapter: GPUAdapter) {
  // Constants are computed such that per run, this function should allocate roughly 2G
  // worth of data. This should be sufficient as we run these creation functions many
  // times. If the data backing the created objects is not recycled we should OOM.
  const kNumPipelines = 128;
  const kNumBindgroups = 128;
  const kSize = 128;
  const kBufferData = new Uint32Array([...iterRange(kSize * kSize, x => x)]);

  const device: GPUDevice = await t.requestDeviceTracked(adapter);
  const commands = [];

  for (let pipelineIndex = 0; pipelineIndex < kNumPipelines; ++pipelineIndex) {
    const module = device.createShaderModule({
      code: `
          struct Buffer { data: array<vec4<u32>, ${(kSize * kSize) / 4}>, };

          @group(0) @binding(0) var<uniform> buffer: Buffer;
          @vertex fn vmain(
            @builtin(vertex_index) vertexIndex: u32
          ) -> @builtin(position) vec4<f32> {
            let index = buffer.data[vertexIndex / 4u][vertexIndex % 4u];
            let position = vec2<f32>(f32(index % ${kSize}u), f32(index / ${kSize}u));
            let r = vec2<f32>(1.0 / f32(${kSize}));
            let a = 2.0 * r;
            let b = r - vec2<f32>(1.0);
            return vec4<f32>(fma(position, a, b), 0.0, 1.0);
          }

          @fragment fn fmain() -> @location(0) vec4<f32> {
            return vec4<f32>(${pipelineIndex}.0 / ${kNumPipelines}.0, 0.0, 0.0, 1.0);
          }
        `,
    });
    const pipeline = device.createRenderPipeline({
      layout: device.createPipelineLayout({
        bindGroupLayouts: [
          device.createBindGroupLayout({
            entries: [
              {
                binding: 0,
                visibility: GPUShaderStage.VERTEX,
                buffer: { type: 'uniform' },
              },
            ],
          }),
        ],
      }),
      vertex: { module, entryPoint: 'vmain', buffers: [] },
      primitive: { topology: 'point-list' },
      fragment: {
        targets: [{ format: 'rgba8unorm' }],
        module,
        entryPoint: 'fmain',
      },
    });
    for (let bindgroupIndex = 0; bindgroupIndex < kNumBindgroups; ++bindgroupIndex) {
      const buffer = t.trackForCleanup(
        device.createBuffer({
          size: kSize * kSize * 4,
          usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
        })
      );
      device.queue.writeBuffer(buffer, 0, kBufferData, 0, kBufferData.length);
      const bindgroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [{ binding: 0, resource: { buffer } }],
      });
      const texture = t.trackForCleanup(
        device.createTexture({
          size: [kSize, kSize],
          usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC,
          format: 'rgba8unorm',
        })
      );

      const encoder = device.createCommandEncoder();
      const pass = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: texture.createView(),
            loadOp: 'load',
            storeOp: 'store',
          },
        ],
      });
      pass.setPipeline(pipeline);
      pass.setBindGroup(0, bindgroup);
      pass.draw(kSize * kSize);
      pass.end();
      commands.push(encoder.finish());
    }
  }
  return { device, objects: commands };
}

/**
 * Creates a device and a large number of buffers which are immediately written to. The
 * buffers are expected to be kept alive until they or the device are destroyed.
 */
async function createDeviceAndBuffers(t: Fixture, adapter: GPUAdapter) {
  // Currently we just allocate 2G of memory using 512MB blocks. We may be able to
  // increase this to hit OOM instead, but on integrated GPUs on Metal, this can cause
  // kernel panics at the moment, and it can greatly increase the time needed.
  const kTotalMemorySize = 2 * 1024 * 1024 * 1024;
  const kMemoryBlockSize = 512 * 1024 * 1024;
  const kMemoryBlockData = new Uint8Array(kMemoryBlockSize);

  const device: GPUDevice = await t.requestDeviceTracked(adapter);
  const buffers = [];
  for (let memory = 0; memory < kTotalMemorySize; memory += kMemoryBlockSize) {
    const buffer = t.trackForCleanup(
      device.createBuffer({
        size: kMemoryBlockSize,
        usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
      })
    );

    // Write out to the buffer to make sure that it has backing memory.
    device.queue.writeBuffer(buffer, 0, kMemoryBlockData, 0, kMemoryBlockData.length);
    buffers.push(buffer);
  }
  return { device, objects: buffers };
}

g.test('coexisting')
  .desc(`Tests allocation of many coexisting GPUDevice objects.`)
  .params(u => u.combine('adapterType', kAdapterTypes))
  .fn(async t => {
    const { adapterType } = t.params;
    const adapter = await getGPU(t.rec).requestAdapter(kAdapterTypeOptions[adapterType]);
    assert(adapter !== null, 'Failed to get adapter.');

    // Based on Vulkan conformance test requirement to be able to create multiple devices.
    const kNumDevices = 5;

    const devices = [];
    for (let i = 0; i < kNumDevices; ++i) {
      const device = await t.requestDeviceTracked(adapter);
      devices.push(device);
    }
  });

g.test('continuous,with_destroy')
  .desc(
    `Tests allocation and destruction of many GPUDevice objects over time. Device objects
are sequentially requested with a series of device allocated objects created on each
device. The devices are then destroyed to verify that the device and the device allocated
objects are recycled over a very large number of iterations.`
  )
  .params(u => u.combine('adapterType', kAdapterTypes))
  .fn(async t => {
    const { adapterType } = t.params;
    const adapter = await getGPU(t.rec).requestAdapter(kAdapterTypeOptions[adapterType]);
    assert(adapter !== null, 'Failed to get adapter.');

    // Since devices are being destroyed, we should be able to create many devices.
    const kNumDevices = 100;
    const kFunctions = [
      createDeviceAndBuffers,
      createDeviceAndComputeCommands,
      createDeviceAndRenderCommands,
    ];

    const deviceList = [];
    const objectLists = [];
    for (let i = 0; i < kNumDevices; ++i) {
      const { device, objects } = await kFunctions[i % kFunctions.length](t, adapter);
      t.expect(objects.length > 0, 'unable to allocate any objects');
      deviceList.push(device);
      objectLists.push(objects);
      device.destroy();
    }
  });

g.test('continuous,no_destroy')
  .desc(
    `Tests allocation and implicit GC of many GPUDevice objects over time. Objects are
sequentially requested and dropped for GC over a very large number of iterations. Note
that without destroy, we do not create device allocated objects because that will
implicitly keep the device in scope.`
  )
  .params(u => u.combine('adapterType', kAdapterTypes))
  .fn(async t => {
    const { adapterType } = t.params;
    const adapter = await getGPU(t.rec).requestAdapter(kAdapterTypeOptions[adapterType]);
    assert(adapter !== null, 'Failed to get adapter.');

    const kNumDevices = 10_000;
    for (let i = 1; i <= kNumDevices; ++i) {
      await (async () => {
        // No trackForCleanup because it would prevent the GPUDevice from being GCed.
        // eslint-disable-next-line no-restricted-syntax
        t.expect((await adapter.requestDevice()) !== null, 'unexpected null device');
      })();
      if (i % 10 === 0) {
        // We need to occasionally wait for GC to clear out stale devices.
        await attemptGarbageCollection();
      }
    }
  });
