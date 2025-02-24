'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function test (binding) {
  assert.strictEqual(binding.memory_management.externalAllocatedMemory(), true);
}
