'use strict';
const assert = require('assert');

/**
 *
 * ThreadSafeFunction Tests: Thread Id Sums
 *
 * Every native C++ function that utilizes the TSFN will call the registered
 * callback with the thread id. Passing Array.prototype.push with a bound array
 * will push the thread id to the array. Therefore, starting `N` threads, we
 * will expect the sum of all elements in the array to be `(N-1) * (N) / 2` (as
 * thread IDs are 0-based)
 *
 * We check different methods of passing a ThreadSafeFunction around multiple
 * threads:
 *  - `testWithTSFN`: The main thread creates the TSFN. Then, it creates
 *    threads, passing the TSFN at thread construction. The number of threads is
 *    static (known at TSFN creation).
 *  - `testDelayedTSFN`: The main thread creates threads, passing a promise to a
 *    TSFN at construction. Then, it creates the TSFN, and resolves each
 *    threads' promise. The number of threads is static.
 *  - `testAcquire`: The native binding returns a function to start a new. A
 *    call to this function will return `false` once `N` calls have been made.
 *    Each thread will acquire its own use of the TSFN, call it, and then
 *    release.
 */

const THREAD_COUNT = 5;
const EXPECTED_SUM = (THREAD_COUNT - 1) * (THREAD_COUNT) / 2;

module.exports = require('../common').runTest(test);

/** @param {number[]} N */
const sum = (N) => N.reduce((sum, n) => sum + n, 0);

function test (binding) {
  async function check (bindingFunction) {
    const calls = [];
    const result = await bindingFunction(THREAD_COUNT, Array.prototype.push.bind(calls));
    assert.ok(result);
    assert.equal(sum(calls), EXPECTED_SUM);
  }

  async function checkAcquire () {
    const calls = [];
    const { promise, createThread, stopThreads } = binding.threadsafe_function_sum.testAcquire(Array.prototype.push.bind(calls));
    for (let i = 0; i < THREAD_COUNT; i++) {
      createThread();
    }
    stopThreads();
    const result = await promise;
    assert.ok(result);
    assert.equal(sum(calls), EXPECTED_SUM);
  }

  return check(binding.threadsafe_function_sum.testDelayedTSFN)
    .then(() => check(binding.threadsafe_function_sum.testWithTSFN))
    .then(() => checkAcquire());
}
