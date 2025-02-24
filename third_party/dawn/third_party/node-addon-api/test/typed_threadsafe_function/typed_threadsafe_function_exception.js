'use strict';

const common = require('../common');

module.exports = common.runTest(test);

async function test () {
  await common.runTestInChildProcess({
    suite: 'typed_threadsafe_function_exception',
    testName: 'testCall',
    execArgv: ['--force-node-api-uncaught-exceptions-policy=true']
  });
}
