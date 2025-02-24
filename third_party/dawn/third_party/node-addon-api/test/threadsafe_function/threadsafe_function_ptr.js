'use strict';

module.exports = require('../common').runTest(test);

function test (binding) {
  binding.threadsafe_function_ptr.test({}, () => {});
}
