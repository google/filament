'use strict';

const assert = require('assert');
const testUtil = require('./testUtil');

module.exports = require('./common').runTest(test);

function test (binding) {
  return testUtil.runGCTests([
    'Plain C string',
    () => {
      const sum = binding.run_script.plainString();
      assert.strictEqual(sum, 1 + 2 + 3);
    },

    'std::string',
    () => {
      const sum = binding.run_script.stdString();
      assert.strictEqual(sum, 1 + 2 + 3);
    },

    'JavaScript string',
    () => {
      const sum = binding.run_script.jsString('1 + 2 + 3');
      assert.strictEqual(sum, 1 + 2 + 3);
    },

    'JavaScript, but not a string',
    () => {
      assert.throws(() => {
        binding.run_script.jsString(true);
      }, {
        name: 'TypeError',
        message: 'A string was expected'
      });
    },

    'With context',
    () => {
      const a = 1; const b = 2; const c = 3;
      const sum = binding.run_script.withContext('a + b + c', { a, b, c });
      assert.strictEqual(sum, a + b + c);
    }
  ]);
}
