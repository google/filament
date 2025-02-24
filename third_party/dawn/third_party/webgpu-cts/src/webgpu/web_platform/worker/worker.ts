import { getGPU, setDefaultRequestAdapterOptions } from '../../../common/util/navigator_gpu.js';
import { assert, objectEquals, iterRange } from '../../../common/util/util.js';

// Should be WorkerGlobalScope, but importing lib "webworker" conflicts with lib "dom".
/* eslint-disable-next-line @typescript-eslint/no-explicit-any */
declare const self: any;

async function basicTest() {
  const adapter = await getGPU(null).requestAdapter();
  assert(adapter !== null, 'Failed to get adapter.');

  // eslint-disable-next-line no-restricted-syntax
  const device = await adapter.requestDevice();
  assert(device !== null, 'Failed to get device.');

  const kOffset = 1230000;
  const pipeline = device.createComputePipeline({
    layout: 'auto',
    compute: {
      module: device.createShaderModule({
        code: `
          struct Buffer { data: array<u32>, };

          @group(0) @binding(0) var<storage, read_write> buffer: Buffer;
          @compute @workgroup_size(1u) fn main(
              @builtin(global_invocation_id) id: vec3<u32>) {
            buffer.data[id.x] = id.x + ${kOffset}u;
          }
        `,
      }),
      entryPoint: 'main',
    },
  });

  const kNumElements = 64;
  const kBufferSize = kNumElements * 4;
  // eslint-disable-next-line no-restricted-syntax
  const buffer = device.createBuffer({
    size: kBufferSize,
    usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC,
  });

  // eslint-disable-next-line no-restricted-syntax
  const resultBuffer = device.createBuffer({
    size: kBufferSize,
    usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST,
  });

  const bindGroup = device.createBindGroup({
    layout: pipeline.getBindGroupLayout(0),
    entries: [{ binding: 0, resource: { buffer } }],
  });

  const encoder = device.createCommandEncoder();

  const pass = encoder.beginComputePass();
  pass.setPipeline(pipeline);
  pass.setBindGroup(0, bindGroup);
  pass.dispatchWorkgroups(kNumElements);
  pass.end();

  encoder.copyBufferToBuffer(buffer, 0, resultBuffer, 0, kBufferSize);

  device.queue.submit([encoder.finish()]);

  const expected = new Uint32Array([...iterRange(kNumElements, x => x + kOffset)]);

  await resultBuffer.mapAsync(GPUMapMode.READ);
  const actual = new Uint32Array(resultBuffer.getMappedRange());

  assert(objectEquals(actual, expected), 'compute pipeline ran');

  resultBuffer.destroy();
  buffer.destroy();
  device.destroy();
}

async function reportTestResults(this: MessagePort | Worker, ev: MessageEvent) {
  const defaultRequestAdapterOptions: GPURequestAdapterOptions =
    ev.data.defaultRequestAdapterOptions;
  setDefaultRequestAdapterOptions(defaultRequestAdapterOptions);

  let error = undefined;
  try {
    await basicTest();
  } catch (err: unknown) {
    error = (err as Error).toString();
  }
  this.postMessage({ error });
}

self.onmessage = (ev: MessageEvent) => {
  void reportTestResults.call(ev.source || self, ev);
};

self.onconnect = (event: MessageEvent) => {
  const port = event.ports[0];

  port.onmessage = (ev: MessageEvent) => {
    void reportTestResults.call(port, ev);
  };
};
