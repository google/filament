'use strict';

const assert = require('assert');
const testUtil = require('./testUtil');

function test (binding) {
  return testUtil.runGCTests([
    'objectwrap constructor exception',
    () => {
      const { ConstructorExceptionTest } = binding.objectwrapConstructorException;
      assert.throws(() => (new ConstructorExceptionTest()), /an exception/);
    },
    // Do on gc before returning.
    () => {}
  ]);
}

module.exports = require('./common').runTest(test);
