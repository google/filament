import { assert } from '../../../../common/util/util.js';
import { GPUTest } from '../../../gpu_test.js';

export class MappingTest extends GPUTest {
  checkMapWrite(
    buffer: GPUBuffer,
    offset: number,
    mappedContents: ArrayBuffer,
    size: number
  ): void {
    this.checkMapWriteZeroed(mappedContents, size);

    const mappedView = new Uint32Array(mappedContents);
    const expected = new Uint32Array(new ArrayBuffer(size));
    assert(mappedView.byteLength === size);
    for (let i = 0; i < mappedView.length; ++i) {
      mappedView[i] = expected[i] = i + 1;
    }
    buffer.unmap();

    this.expectGPUBufferValuesEqual(buffer, expected, offset);
  }

  checkMapWriteZeroed(arrayBuffer: ArrayBuffer, expectedSize: number): void {
    this.expect(arrayBuffer.byteLength === expectedSize);
    const view = new Uint8Array(arrayBuffer);
    this.expectZero(view);
  }

  expectZero(actual: Uint8Array): void {
    const size = actual.byteLength;
    for (let i = 0; i < size; ++i) {
      if (actual[i] !== 0) {
        this.fail(`at [${i}], expected zero, got ${actual[i]}`);
        break;
      }
    }
  }
}
