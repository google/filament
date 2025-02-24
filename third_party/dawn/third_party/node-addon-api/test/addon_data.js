'use strict';

const common = require('./common');

module.exports = common.runTest(test);

async function test () {
  await common.runTestInChildProcess({
    suite: 'addon_data',
    testName: 'workingCode'
  });

  await common.runTestInChildProcess({
    suite: 'addon_data',
    testName: 'cleanupWithoutHint',
    expectedStderr: ['addon_data: Addon::~Addon']
  });

  await common.runTestInChildProcess({
    suite: 'addon_data',
    testName: 'cleanupWithHint',
    expectedStderr: ['addon_data: Addon::~Addon', 'hint: 42']
  });
}
