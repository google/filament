'use strict';

const assert = require('assert');
const testUtil = require('./testUtil');

module.exports = require('./common').runTest(test);

function test (binding) {
  return testUtil.runGCTests([
    'test reference',
    () => binding.reference.createWeakArray(),
    () => assert.strictEqual(true, binding.reference.accessWeakArrayEmpty()),
    'test reference move op',
    () => binding.reference.refMoveAssignTest(),
    'test reference ref',
    () => binding.reference.referenceRefTest(),
    'test reference reset',
    () => binding.reference.refResetTest()
  ]);
}
