'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

async function test (binding) {
  const testCall = binding.threadsafe_function_existing_tsfn.testCall;

  assert.strictEqual(typeof await testCall({ blocking: true, data: true }), 'number');
  assert.strictEqual(typeof await testCall({ blocking: true, data: false }), 'undefined');
  assert.strictEqual(typeof await testCall({ blocking: false, data: true }), 'number');
  assert.strictEqual(typeof await testCall({ blocking: false, data: false }), 'undefined');
}
