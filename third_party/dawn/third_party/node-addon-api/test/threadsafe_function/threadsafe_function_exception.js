'use strict';

const common = require('../common');

module.exports = common.runTest(test);

const execArgv = ['--force-node-api-uncaught-exceptions-policy=true'];
async function test () {
  await common.runTestInChildProcess({
    suite: 'threadsafe_function_exception',
    testName: 'testCall',
    execArgv
  });

  await common.runTestInChildProcess({
    suite: 'threadsafe_function_exception',
    testName: 'testCallWithNativeCallback',
    execArgv
  });
}
