'use strict';

const assert = require('assert');

function test (binding, succeed) {
  return new Promise((resolve) =>
    // Can't pass an arrow function to doWork because that results in an
    // undefined context inside its body when the function gets called.
    binding.doWork(succeed, function (e) {
      setImmediate(() => {
        // If the work is supposed to fail, make sure there's an error.
        assert.strictEqual(succeed || e.message === 'test error', true);
        assert.strictEqual(binding.workerGone, false);
        binding.deleteWorker();
        assert.strictEqual(binding.workerGone, true);
        resolve();
      });
    }));
}

module.exports = require('./common').runTest(async binding => {
  await test(binding.persistentasyncworker, false);
  await test(binding.persistentasyncworker, true);
});
