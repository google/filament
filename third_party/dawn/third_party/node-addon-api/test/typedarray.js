'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function test (binding) {
  const testData = [
    ['int8', Int8Array, 1, new Int8Array([0, 124, 24, 44])],
    ['uint8', Uint8Array, 1, new Uint8Array([0, 255, 2, 14])],
    ['uint8_clamped', Uint8ClampedArray, 1, new Uint8ClampedArray([0, 256, 0, 255])],
    ['int16', Int16Array, 2, new Int16Array([-32768, 32767, 1234, 42])],
    ['uint16', Uint16Array, 2, new Uint16Array([0, 65535, 4, 12])],
    ['int32', Int32Array, 4, new Int32Array([Math.pow(2, 31), Math.pow(-2, 31), 255, 4])],
    ['uint32', Uint32Array, 4, new Uint32Array([0, Math.pow(2, 32), 24, 125])],
    ['float32', Float32Array, 4, new Float32Array([0, 21, 34, 45])],
    ['float64', Float64Array, 8, new Float64Array([0, 4124, 45, 90])]
  ];

  const bigIntTests = [
    ['bigint64', BigInt64Array, 8, new BigInt64Array([9007199254740991n, 9007199254740991n, 24n, 125n])],
    ['biguint64', BigUint64Array, 8, new BigUint64Array([9007199254740991n, 9007199254740991n, 2345n, 345n])]
  ];

  bigIntTests.forEach(data => {
    const length = 4;
    const t = binding.typedarray.createTypedArray(data[0], length);
    assert.ok(t instanceof data[1]);
    assert.strictEqual(binding.typedarray.getTypedArrayType(t), data[0]);
    assert.strictEqual(binding.typedarray.getTypedArrayLength(t), length);
    assert.strictEqual(binding.typedarray.getTypedArraySize(t), data[2]);
    assert.strictEqual(binding.typedarray.getTypedArrayByteOffset(t), 0);
    assert.strictEqual(binding.typedarray.getTypedArrayByteLength(t), data[2] * length);

    t[3] = 11n;
    assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 11n);
    binding.typedarray.setTypedArrayElement(t, 3, 22n);
    assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 22n);
    assert.strictEqual(t[3], 22n);

    const nonEmptyTypedArray = binding.typedarray.createTypedArray(data[0], length, data[3].buffer);
    binding.typedarray.checkBufferContent(nonEmptyTypedArray, data[3]);
  });

  testData.forEach(data => {
    try {
      const length = 4;
      const t = binding.typedarray.createTypedArray(data[0], length);
      assert.ok(t instanceof data[1]);
      assert.strictEqual(binding.typedarray.getTypedArrayType(t), data[0]);
      assert.strictEqual(binding.typedarray.getTypedArrayLength(t), length);
      assert.strictEqual(binding.typedarray.getTypedArraySize(t), data[2]);
      assert.strictEqual(binding.typedarray.getTypedArrayByteOffset(t), 0);
      assert.strictEqual(binding.typedarray.getTypedArrayByteLength(t), data[2] * length);

      t[3] = 11;
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 11);
      binding.typedarray.setTypedArrayElement(t, 3, 22);
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 22);
      assert.strictEqual(t[3], 22);

      const b = binding.typedarray.getTypedArrayBuffer(t);
      assert.ok(b instanceof ArrayBuffer);
    } catch (e) {
      console.log(data);
      throw e;
    }
  });

  testData.forEach(data => {
    try {
      const length = 4;
      const offset = 8;
      const b = new ArrayBuffer(offset + 64 * 4);

      const t = binding.typedarray.createTypedArray(data[0], length, b, offset);
      assert.ok(t instanceof data[1]);
      assert.strictEqual(binding.typedarray.getTypedArrayType(t), data[0]);
      assert.strictEqual(binding.typedarray.getTypedArrayLength(t), length);
      assert.strictEqual(binding.typedarray.getTypedArraySize(t), data[2]);
      assert.strictEqual(binding.typedarray.getTypedArrayByteOffset(t), offset);
      assert.strictEqual(binding.typedarray.getTypedArrayByteLength(t), data[2] * length);

      t[3] = 11;
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 11);
      binding.typedarray.setTypedArrayElement(t, 3, 22);
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 22);
      assert.strictEqual(t[3], 22);

      assert.strictEqual(binding.typedarray.getTypedArrayBuffer(t), b);

      const nonEmptyTypedArray = binding.typedarray.createTypedArray(data[0], length, data[3].buffer);
      assert.strictEqual(binding.typedarray.checkBufferContent(nonEmptyTypedArray, data[3]), true);
    } catch (e) {
      console.log(data);
      throw e;
    }
  });

  assert.throws(() => {
    binding.typedarray.createInvalidTypedArray();
  }, /Invalid (pointer passed as )?argument/);
}
