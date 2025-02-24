'use strict';
const assert = require('assert');
module.exports = require('../common').runTest(test);

function test (binding) {
  assert(binding.typed_threadsafe_function_ptr.test({}, () => {}) === undefined);
  assert(binding.typed_threadsafe_function_ptr.null() === null);
}
