'use strict';

if (process.argv[2] === 'child') {
  // Create a single wrapped instance then exit.
  // eslint-disable-next-line no-new
  new (require(process.argv[3]).objectwrap.Test)();
} else {
  const assert = require('assert');
  const testUtil = require('./testUtil');

  module.exports = require('./common').runTestWithBindingPath(test);

  function test (bindingName) {
    return testUtil.runGCTests([
      'objectwrap removewrap test',
      () => {
        const binding = require(bindingName);
        const Test = binding.objectwrap_removewrap.Test;
        const getDtorCalled = binding.objectwrap_removewrap.getDtorCalled;

        assert.strictEqual(getDtorCalled(), 0);
        assert.throws(() => {
          // eslint-disable-next-line no-new
          new Test();
        });
        assert.strictEqual(getDtorCalled(), 1);
      },
      // Test that gc does not crash.
      () => {}
    ]);
  }
}
