'use strict';

const assert = require('assert');
const { spawnSync } = require('child_process');
const testUtil = require('./testUtil');

if (process.argv.length === 3) {
  let interval;

  // Running as the child process, hook up an `uncaughtException` handler to
  // examine the error thrown by the finalizer.
  process.on('uncaughtException', (error) => {
    // TODO (gabrielschulhof): Use assert.matches() when we drop support for
    // Node.js v10.x.
    assert(!!error.message.match(/Finalizer exception/));
    if (interval) {
      clearInterval(interval);
    }
    process.exit(0);
  });

  // Create an external whose finalizer throws.
  (() =>
    require(process.argv[2]).external.createExternalWithFinalizeException())();

  // gc until the external's finalizer throws or until we give up. Since the
  // exception is thrown from a native `SetImmediate()` we cannot catch it
  // anywhere except in the process' `uncaughtException` handler.
  let maxGCTries = 10;
  (function gcInterval () {
    global.gc();
    if (!interval) {
      interval = setInterval(gcInterval, 100);
    } else if (--maxGCTries === 0) {
      throw new Error('Timed out waiting for the gc to throw');
    }
  })();
} else {
  module.exports = require('./common').runTestWithBindingPath(test);

  function test (bindingPath) {
    const binding = require(bindingPath);

    const child = spawnSync(process.execPath, [
      '--expose-gc', __filename, bindingPath
    ], { stdio: 'inherit' });
    assert.strictEqual(child.status, 0);
    assert.strictEqual(child.signal, null);

    return testUtil.runGCTests([
      'External without finalizer',
      () => {
        const test = binding.external.createExternal();
        assert.strictEqual(typeof test, 'object');
        binding.external.checkExternal(test);
        assert.strictEqual(0, binding.external.getFinalizeCount());
      },
      () => {
        assert.strictEqual(0, binding.external.getFinalizeCount());
      },

      'External with finalizer',
      () => {
        const test = binding.external.createExternalWithFinalize();
        assert.strictEqual(typeof test, 'object');
        binding.external.checkExternal(test);
        assert.strictEqual(0, binding.external.getFinalizeCount());
      },
      () => {
        assert.strictEqual(1, binding.external.getFinalizeCount());
      },

      'External with finalizer hint',
      () => {
        const test = binding.external.createExternalWithFinalizeHint();
        assert.strictEqual(typeof test, 'object');
        binding.external.checkExternal(test);
        assert.strictEqual(0, binding.external.getFinalizeCount());
      },
      () => {
        assert.strictEqual(1, binding.external.getFinalizeCount());
      }
    ]);
  }
}
