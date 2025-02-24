import { assert } from '../../../common/util/util.js';
import { takeScreenshot, takeScreenshotDelayed } from '../../../common/util/wpt_reftest_wait.js';

interface GPURefTest {
  readonly device: GPUDevice;
  readonly queue: GPUQueue;
}

export function runRefTest(fn: (t: GPURefTest) => Promise<void> | void): void {
  void (async () => {
    assert(
      typeof navigator !== 'undefined' && navigator.gpu !== undefined,
      'No WebGPU implementation found'
    );

    const adapter = await navigator.gpu.requestAdapter();
    assert(adapter !== null);
    const device = await adapter.requestDevice();
    assert(device !== null);
    const queue = device.queue;

    await fn({ device, queue });

    takeScreenshotDelayed(50);
  })().catch(() => {
    // remove reftest-wait to mark end of test
    takeScreenshot();
  });
}
