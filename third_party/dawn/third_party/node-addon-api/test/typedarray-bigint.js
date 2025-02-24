'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function test (binding) {
  [
    ['bigint64', BigInt64Array],
    ['biguint64', BigUint64Array]
  ].forEach(([type, Constructor]) => {
    try {
      const length = 4;
      const t = binding.typedarray.createTypedArray(type, length);
      assert.ok(t instanceof Constructor);
      assert.strictEqual(binding.typedarray.getTypedArrayType(t), type);
      assert.strictEqual(binding.typedarray.getTypedArrayLength(t), length);

      t[3] = 11n;
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 11n);
      binding.typedarray.setTypedArrayElement(t, 3, 22n);
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 22n);
      assert.strictEqual(t[3], 22n);

      const b = binding.typedarray.getTypedArrayBuffer(t);
      assert.ok(b instanceof ArrayBuffer);
    } catch (e) {
      console.log(type, Constructor);
      throw e;
    }

    try {
      const length = 4;
      const offset = 8;
      const b = new ArrayBuffer(offset + 64 * 4);

      const t = binding.typedarray.createTypedArray(type, length, b, offset);
      assert.ok(t instanceof Constructor);
      assert.strictEqual(binding.typedarray.getTypedArrayType(t), type);
      assert.strictEqual(binding.typedarray.getTypedArrayLength(t), length);

      t[3] = 11n;
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 11n);
      binding.typedarray.setTypedArrayElement(t, 3, 22n);
      assert.strictEqual(binding.typedarray.getTypedArrayElement(t, 3), 22n);
      assert.strictEqual(t[3], 22n);

      assert.strictEqual(binding.typedarray.getTypedArrayBuffer(t), b);
    } catch (e) {
      console.log(type, Constructor);
      throw e;
    }
  });

  assert.throws(() => {
    binding.typedarray.createInvalidTypedArray();
  }, /Invalid (pointer passed as )?argument/);
}
