'use strict';

const common = require('./common');

module.exports = common.runTest(test);

async function test (binding) {
  binding.callbackInfo.testCbSetData();
}
