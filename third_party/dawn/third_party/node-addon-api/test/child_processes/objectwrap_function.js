'use strict';

const assert = require('assert');
const testUtil = require('../testUtil');

module.exports = {
  runTest: function (binding) {
    return testUtil.runGCTests([
      'objectwrap function',
      () => {
        const { FunctionTest } = binding.objectwrap_function();
        const newConstructed = new FunctionTest();
        const functionConstructed = FunctionTest();
        assert(newConstructed instanceof FunctionTest);
        assert(functionConstructed instanceof FunctionTest);
        assert.throws(() => (FunctionTest(true)), /an exception/);
      },
      // Do one gc before returning.
      () => {}
    ]);
  }
};
