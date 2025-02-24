export const description = `
Stress tests covering robustness when available VRAM is exhausted.
`;

import { makeTestGroup } from '../../common/framework/test_group.js';
import { unreachable } from '../../common/util/util.js';
import { GPUConst } from '../../webgpu/constants.js';
import { GPUTest } from '../../webgpu/gpu_test.js';
import { exhaustVramUntilUnder64MB } from '../../webgpu/util/memory.js';

export const g = makeTestGroup(GPUTest);

function createBufferWithMapState(
  t: GPUTest,
  size: number,
  mapState: GPUBufferMapState,
  mode: GPUMapModeFlags,
  mappedAtCreation: boolean
) {
  const mappable = mapState === 'unmapped';
  if (!mappable && !mappedAtCreation) {
    return t.createBufferTracked({
      size,
      usage: GPUBufferUsage.UNIFORM,
      mappedAtCreation,
    });
  }
  let buffer: GPUBuffer;
  switch (mode) {
    case GPUMapMode.READ:
      buffer = t.createBufferTracked({
        size,
        usage: GPUBufferUsage.MAP_READ,
        mappedAtCreation,
      });
      break;
    case GPUMapMode.WRITE:
      buffer = t.createBufferTracked({
        size,
        usage: GPUBufferUsage.MAP_WRITE,
        mappedAtCreation,
      });
      break;
    default:
      unreachable();
  }
  // If we want the buffer to be mappable and also mappedAtCreation, we call unmap on it now.
  if (mappable && mappedAtCreation) {
    buffer.unmap();
  }
  return buffer;
}

g.test('vram_oom')
  .desc(`Tests that we can allocate buffers until we run out of VRAM.`)
  .fn(async t => {
    await exhaustVramUntilUnder64MB(t);
  });

g.test('map_after_vram_oom')
  .desc(
    `Allocates tons of buffers and textures with varying mapping states (unmappable,
mappable, mapAtCreation, mapAtCreation-then-unmapped) until OOM; then attempts
to mapAsync all the mappable objects. The last buffer should be an error buffer so
mapAsync on it should reject and produce a validation error. `
  )
  .params(u =>
    u
      .combine('mapState', ['mapped', 'unmapped'] as GPUBufferMapState[])
      .combine('mode', [GPUConst.MapMode.READ, GPUConst.MapMode.WRITE])
      .combine('mappedAtCreation', [true, false])
      .combine('unmapBeforeResolve', [true, false])
  )
  .fn(async t => {
    // Use a relatively large size to quickly hit OOM.
    const kSize = 512 * 1024 * 1024;

    const { mapState, mode, mappedAtCreation, unmapBeforeResolve } = t.params;
    const mappable = mapState === 'unmapped';
    const buffers: GPUBuffer[] = [];
    // Closure to call map and verify results on all of the buffers.
    const finish = async () => {
      if (mappable) {
        await Promise.all(buffers.map(value => value.mapAsync(mode)));
      } else {
        buffers.forEach(value => {
          t.expectValidationError(() => {
            void value.mapAsync(mode);
          });
        });
      }
      // Finally, destroy all the buffers to free the resources.
      buffers.forEach(buffer => buffer.destroy());
    };

    let errorBuffer: GPUBuffer;
    for (;;) {
      if (mappedAtCreation) {
        // When mappedAtCreation is true, OOM can happen on the client which throws a RangeError. In
        // this case, we don't do any validations on the OOM buffer.
        try {
          t.device.pushErrorScope('out-of-memory');
          const buffer = createBufferWithMapState(t, kSize, mapState, mode, mappedAtCreation);
          if (await t.device.popErrorScope()) {
            errorBuffer = buffer;
            break;
          }
          buffers.push(buffer);
        } catch (ex) {
          t.expect(ex instanceof RangeError);
          await finish();
          return;
        }
      } else {
        t.device.pushErrorScope('out-of-memory');
        const buffer = createBufferWithMapState(t, kSize, mapState, mode, mappedAtCreation);
        if (await t.device.popErrorScope()) {
          errorBuffer = buffer;
          break;
        }
        buffers.push(buffer);
      }
    }

    // Do some validation on the OOM buffer.
    let promise: Promise<void>;
    t.expectValidationError(() => {
      promise = errorBuffer.mapAsync(mode);
    });
    if (unmapBeforeResolve) {
      // Should reject with abort error because buffer will be unmapped
      // before validation check finishes.
      t.shouldReject('AbortError', promise!);
    } else {
      // Should also reject in addition to the validation error.
      t.shouldReject('OperationError', promise!);

      // Wait for validation error before unmap to ensure validation check
      // ends before unmap.
      try {
        await promise!;
        throw new Error('The promise should be rejected.');
      } catch {
        // Should cause an exception because the promise should be rejected.
      }
    }

    // Should throw an OperationError because the buffer is not mapped.
    // Note: not a RangeError because the state of the buffer is checked first.
    t.shouldThrow('OperationError', () => {
      errorBuffer.getMappedRange();
    });

    // Should't be a validation error even if the buffer failed to be mapped.
    errorBuffer.unmap();
    errorBuffer.destroy();

    // Finish the rest of the test w.r.t the mappable buffers.
    await finish();
  });

g.test('validation_vs_oom')
  .desc(
    `Tests that calls affected by both OOM and validation errors expose the
validation error with precedence.`
  )
  .unimplemented();

g.test('recovery')
  .desc(
    `Tests that after going VRAM-OOM, destroying allocated resources eventually
allows new resources to be allocated.`
  )
  .unimplemented();
