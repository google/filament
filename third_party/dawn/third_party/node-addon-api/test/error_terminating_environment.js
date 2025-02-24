'use strict';

const assert = require('assert');
const { whichBuildType } = require('./common');

// These tests ensure that Error types can be used in a terminating
// environment without triggering any fatal errors.

if (process.argv[2] === 'runInChildProcess') {
  const bindingPath = process.argv[3];
  const indexForTestCase = Number(process.argv[4]);

  const binding = require(bindingPath);

  // Use C++ promises to ensure the worker thread is terminated right
  // before running the testable code in the binding.

  binding.error.resetPromises();

  const { Worker } = require('worker_threads');

  const worker = new Worker(
    __filename,
    {
      argv: [
        'runInWorkerThread',
        bindingPath,
        indexForTestCase
      ]
    }
  );

  binding.error.waitForWorkerThread();

  worker.terminate();

  binding.error.releaseWorkerThread();
} else {
  if (process.argv[2] === 'runInWorkerThread') {
    const bindingPath = process.argv[3];
    const indexForTestCase = Number(process.argv[4]);

    const binding = require(bindingPath);

    switch (indexForTestCase) {
      case 0:
        binding.error.throwJSError('test', true);
        break;
      case 1:
        binding.error.throwTypeError('test', true);
        break;
      case 2:
        binding.error.throwRangeError('test', true);
        break;
      case 3:
        binding.error.throwDefaultError(false, true);
        break;
      case 4:
        binding.error.throwDefaultError(true, true);
        break;
      default: assert.fail('Invalid index');
    }

    assert.fail('This should not be reachable');
  }

  wrapTest();

  async function wrapTest () {
    const buildType = await whichBuildType();
    test(`./build/${buildType}/binding.node`, true);
    test(`./build/${buildType}/binding_noexcept.node`, true);
    test(`./build/${buildType}/binding_swallowexcept.node`, false);
    test(`./build/${buildType}/binding_swallowexcept_noexcept.node`, false);
    test(`./build/${buildType}/binding_custom_namespace.node`, true);
  }

  function test (bindingPath, processShouldAbort) {
    const numberOfTestCases = 5;

    for (let i = 0; i < numberOfTestCases; ++i) {
      const childProcess = require('./napi_child').spawnSync(
        process.execPath,
        [
          __filename,
          'runInChildProcess',
          bindingPath,
          i
        ]
      );

      if (processShouldAbort) {
        assert(childProcess.status !== 0, `Test case ${bindingPath} ${i} failed: Process exited with status code 0.`);
      } else {
        assert(childProcess.status === 0, `Test case ${bindingPath} ${i} failed: Process status ${childProcess.status} is non-zero`);
      }
    }
  }
}
