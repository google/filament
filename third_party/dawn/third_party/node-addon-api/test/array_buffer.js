'use strict';

const assert = require('assert');
const testUtil = require('./testUtil');

module.exports = require('./common').runTest(test);

function test (binding) {
  return testUtil.runGCTests([
    'Internal ArrayBuffer',
    () => {
      const test = binding.arraybuffer.createBuffer();
      binding.arraybuffer.checkBuffer(test);
      assert.ok(test instanceof ArrayBuffer);

      const test2 = test.slice(0);
      binding.arraybuffer.checkBuffer(test2);
    },

    'External ArrayBuffer',
    () => {
      const test = binding.arraybuffer.createExternalBuffer();
      binding.arraybuffer.checkBuffer(test);
      assert.ok(test instanceof ArrayBuffer);
      assert.strictEqual(0, binding.arraybuffer.getFinalizeCount());
    },

    () => assert.strictEqual(0, binding.arraybuffer.getFinalizeCount()),

    'External ArrayBuffer with finalizer',
    () => {
      const test = binding.arraybuffer.createExternalBufferWithFinalize();
      binding.arraybuffer.checkBuffer(test);
      assert.ok(test instanceof ArrayBuffer);
      assert.strictEqual(0, binding.arraybuffer.getFinalizeCount());
    },

    () => assert.strictEqual(1, binding.arraybuffer.getFinalizeCount()),

    'External ArrayBuffer with finalizer hint',
    () => {
      const test = binding.arraybuffer.createExternalBufferWithFinalizeHint();
      binding.arraybuffer.checkBuffer(test);
      assert.ok(test instanceof ArrayBuffer);
      assert.strictEqual(0, binding.arraybuffer.getFinalizeCount());
    },

    () => assert.strictEqual(1, binding.arraybuffer.getFinalizeCount()),

    'ArrayBuffer with constructor',
    () => {
      assert.strictEqual(true, binding.arraybuffer.checkEmptyBuffer());
      const test = binding.arraybuffer.createBufferWithConstructor();
      binding.arraybuffer.checkBuffer(test);
      assert.ok(test instanceof ArrayBuffer);
    },

    'ArrayBuffer updates data pointer and length when detached',
    () => {
      // Detach the ArrayBuffer in JavaScript.
      // eslint-disable-next-line no-undef
      const mem = new WebAssembly.Memory({ initial: 1 });
      binding.arraybuffer.checkDetachUpdatesData(mem.buffer, () => mem.grow(1));

      // Let C++ detach the ArrayBuffer.
      const extBuffer = binding.arraybuffer.createExternalBuffer();
      binding.arraybuffer.checkDetachUpdatesData(extBuffer);
    }
  ]);
}
