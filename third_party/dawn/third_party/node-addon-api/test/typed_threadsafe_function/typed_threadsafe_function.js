'use strict';

const assert = require('assert');
const common = require('../common');

module.exports = common.runTest(test);

async function test (binding) {
  const expectedArray = (function (arrayLength) {
    const result = [];
    for (let index = 0; index < arrayLength; index++) {
      result.push(arrayLength - 1 - index);
    }
    return result;
  })(binding.typed_threadsafe_function.ARRAY_LENGTH);

  function testWithJSMarshaller ({
    threadStarter,
    quitAfter,
    abort,
    maxQueueSize,
    launchSecondary
  }) {
    return new Promise((resolve) => {
      const array = [];
      binding.typed_threadsafe_function[threadStarter](function testCallback (value) {
        array.push(value);
        if (array.length === quitAfter) {
          binding.typed_threadsafe_function.stopThread(common.mustCall(() => {
            resolve(array);
          }), !!abort);
        }
      }, !!abort, !!launchSecondary, maxQueueSize);
      if (threadStarter === 'startThreadNonblocking') {
        // Let's make this thread really busy for a short while to ensure that
        // the queue fills and the thread receives a napi_queue_full.
        const start = Date.now();
        while (Date.now() - start < 200);
      }
    });
  }

  await new Promise(function testWithoutJSMarshaller (resolve) {
    let callCount = 0;
    binding.typed_threadsafe_function.startThreadNoNative(function testCallback () {
      callCount++;

      // The default call-into-JS implementation passes no arguments.
      assert.strictEqual(arguments.length, 0);
      if (callCount === binding.typed_threadsafe_function.ARRAY_LENGTH) {
        setImmediate(() => {
          binding.typed_threadsafe_function.stopThread(common.mustCall(() => {
            resolve();
          }), false);
        });
      }
    }, false /* abort */, false /* launchSecondary */,
    binding.typed_threadsafe_function.MAX_QUEUE_SIZE);
  });

  // Start the thread in blocking mode, and assert that all values are passed.
  // Quit after it's done.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThread',
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      quitAfter: binding.typed_threadsafe_function.ARRAY_LENGTH
    }),
    expectedArray
  );

  // Start the thread in blocking mode with an infinite queue, and assert that
  // all values are passed. Quit after it's done.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThread',
      maxQueueSize: 0,
      quitAfter: binding.typed_threadsafe_function.ARRAY_LENGTH
    }),
    expectedArray
  );

  // Start the thread in non-blocking mode, and assert that all values are
  // passed. Quit after it's done.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThreadNonblocking',
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      quitAfter: binding.typed_threadsafe_function.ARRAY_LENGTH
    }),
    expectedArray
  );

  // Start the thread in blocking mode, and assert that all values are passed.
  // Quit early, but let the thread finish.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThread',
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      quitAfter: 1
    }),
    expectedArray
  );

  // Start the thread in blocking mode with an infinite queue, and assert that
  // all values are passed. Quit early, but let the thread finish.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThread',
      maxQueueSize: 0,
      quitAfter: 1
    }),
    expectedArray
  );

  // Start the thread in non-blocking mode, and assert that all values are
  // passed. Quit early, but let the thread finish.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThreadNonblocking',
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      quitAfter: 1
    }),
    expectedArray
  );

  // Start the thread in blocking mode, and assert that all values are passed.
  // Quit early, but let the thread finish. Launch a secondary thread to test
  // the reference counter incrementing functionality.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThread',
      quitAfter: 1,
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      launchSecondary: true
    }),
    expectedArray
  );

  // Start the thread in non-blocking mode, and assert that all values are
  // passed. Quit early, but let the thread finish. Launch a secondary thread
  // to test the reference counter incrementing functionality.
  assert.deepStrictEqual(
    await testWithJSMarshaller({
      threadStarter: 'startThreadNonblocking',
      quitAfter: 1,
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      launchSecondary: true
    }),
    expectedArray
  );

  // Start the thread in blocking mode, and assert that it could not finish.
  // Quit early by aborting.
  assert.strictEqual(
    (await testWithJSMarshaller({
      threadStarter: 'startThread',
      quitAfter: 1,
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      abort: true
    })).indexOf(0),
    -1
  );

  // Start the thread in blocking mode with an infinite queue, and assert that
  // it could not finish. Quit early by aborting.
  assert.strictEqual(
    (await testWithJSMarshaller({
      threadStarter: 'startThread',
      quitAfter: 1,
      maxQueueSize: 0,
      abort: true
    })).indexOf(0),
    -1
  );

  // Start the thread in non-blocking mode, and assert that it could not finish.
  // Quit early and aborting.
  assert.strictEqual(
    (await testWithJSMarshaller({
      threadStarter: 'startThreadNonblocking',
      quitAfter: 1,
      maxQueueSize: binding.typed_threadsafe_function.MAX_QUEUE_SIZE,
      abort: true
    })).indexOf(0),
    -1
  );
}
