'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function test (binding) {
  const {
    CreateDate,
    IsDate,
    ValueOf,
    OperatorValue
  } = binding.date;
  assert.deepStrictEqual(CreateDate(0), new Date(0));
  assert.strictEqual(IsDate(new Date(0)), true);
  assert.strictEqual(ValueOf(new Date(42)), 42);
  assert.strictEqual(OperatorValue(new Date(42)), true);
}
