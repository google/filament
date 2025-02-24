'use strict';

const assert = require('assert');

const isMainProcess = process.argv[1] !== __filename;

/**
 * In order to test that the event loop exits even with an active TSFN, we need
 * to spawn a new process for the test.
 *   - Main process: spawns new node instance, executing this script
 *   - Child process: creates TSFN. Native module Unref's via setTimeout after some time but does NOT call Release.
 *
 * Main process should expect child process to exit.
 *
 * We also added a new test case for `Ref`. The idea being, if a TSFN is active, the event loop that it belongs to should not exit
 * Our setup is similar to the test for the `Unref` case, with the difference being now we are expecting the child process to hang
 */

if (isMainProcess) {
  module.exports = require('../common').runTestWithBindingPath(test);
} else {
  const isTestingRef = (process.argv[3] === 'true');

  if (isTestingRef) {
    execTSFNRefTest(process.argv[2]);
  } else {
    execTSFNUnrefTest(process.argv[2]);
  }
}

function testUnRefCallback (resolve, reject, bindingFile) {
  const child = require('../napi_child').spawn(process.argv[0], [
    '--expose-gc', __filename, bindingFile, false
  ], { stdio: 'inherit' });

  let timeout = setTimeout(function () {
    child.kill();
    timeout = 0;
    reject(new Error('Expected child to die'));
  }, 5000);

  child.on('error', (err) => {
    clearTimeout(timeout);
    timeout = 0;
    reject(new Error(err));
  });

  child.on('close', (code) => {
    if (timeout) clearTimeout(timeout);
    assert.strictEqual(code, 0, 'Expected return value 0');
    resolve();
  });
}

function testRefCallback (resolve, reject, bindingFile) {
  const child = require('../napi_child').spawn(process.argv[0], [
    '--expose-gc', __filename, bindingFile, true
  ], { stdio: 'inherit' });

  let timeout = setTimeout(function () {
    child.kill();
    timeout = 0;
    resolve();
  }, 1000);

  child.on('error', (err) => {
    clearTimeout(timeout);
    timeout = 0;
    reject(new Error(err));
  });

  child.on('close', (code) => {
    if (timeout) clearTimeout(timeout);

    reject(new Error('We expected Child to hang'));
  });
}

function test (bindingFile) {
  // Main process
  return new Promise((resolve, reject) => {
    testUnRefCallback(resolve, reject, bindingFile);
  }).then(() => {
    return new Promise((resolve, reject) => {
      testRefCallback(resolve, reject, bindingFile);
    });
  });
}

function execTSFNUnrefTest (bindingFile) {
  const binding = require(bindingFile);
  binding.typed_threadsafe_function_unref.testUnref({}, () => { });
}

function execTSFNRefTest (bindingFile) {
  const binding = require(bindingFile);
  binding.typed_threadsafe_function_unref.testRef({}, () => { });
}
