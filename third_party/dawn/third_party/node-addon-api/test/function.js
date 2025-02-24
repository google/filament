'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(binding => {
  test(binding.function.plain);
  test(binding.function.templated);
  testLambda(binding.function.lambda);
});

function test (binding) {
  assert.strictEqual(binding.emptyConstructor(true), true);
  assert.strictEqual(binding.emptyConstructor(false), false);

  let obj = {};
  assert.deepStrictEqual(binding.voidCallback(obj), undefined);
  assert.deepStrictEqual(obj, { foo: 'bar' });

  assert.deepStrictEqual(binding.valueCallback(), { foo: 'bar' });

  /* eslint-disable-next-line no-new, new-cap */
  assert.strictEqual(new binding.newTargetCallback(), binding.newTargetCallback);
  assert.strictEqual(binding.newTargetCallback(), undefined);

  let args = null;
  let ret = null;
  let receiver = null;
  function testFunction () {
    receiver = this;
    args = [].slice.call(arguments);
    return ret;
  }
  function testConstructor () {
    args = [].slice.call(arguments);
  }

  function makeCallbackTestFunction (receiver, expectedOne, expectedTwo, expectedThree) {
    return function callback (one, two, three) {
      assert.strictEqual(this, receiver);
      assert.strictEqual(one, expectedOne);
      assert.strictEqual(two, expectedTwo);
      assert.strictEqual(three, expectedThree);
    };
  }

  ret = 4;
  assert.strictEqual(binding.callWithArgs(testFunction, 1, 2, 3), 4);
  assert.strictEqual(receiver, undefined);
  assert.deepStrictEqual(args, [1, 2, 3]);

  ret = 5;
  assert.strictEqual(binding.callWithVector(testFunction, 2, 3, 4), 5);
  assert.strictEqual(receiver, undefined);
  assert.deepStrictEqual(args, [2, 3, 4]);

  ret = 5;
  assert.strictEqual(binding.callWithVectorUsingCppWrapper(testFunction, 2, 3, 4), 5);
  assert.strictEqual(receiver, undefined);
  assert.deepStrictEqual(args, [2, 3, 4]);

  ret = 6;
  assert.strictEqual(binding.callWithReceiverAndArgs(testFunction, obj, 3, 4, 5), 6);
  assert.deepStrictEqual(receiver, obj);
  assert.deepStrictEqual(args, [3, 4, 5]);

  ret = 7;
  assert.strictEqual(binding.callWithReceiverAndVector(testFunction, obj, 4, 5, 6), 7);
  assert.deepStrictEqual(receiver, obj);
  assert.deepStrictEqual(args, [4, 5, 6]);

  ret = 7;
  assert.strictEqual(binding.callWithReceiverAndVectorUsingCppWrapper(testFunction, obj, 4, 5, 6), 7);
  assert.deepStrictEqual(receiver, obj);
  assert.deepStrictEqual(args, [4, 5, 6]);

  ret = 8;
  assert.strictEqual(binding.callWithCStyleArray(testFunction, 5, 6, 7), ret);
  assert.deepStrictEqual(receiver, undefined);
  assert.deepStrictEqual(args, [5, 6, 7]);

  ret = 9;
  assert.strictEqual(binding.callWithReceiverAndCStyleArray(testFunction, obj, 6, 7, 8), ret);
  assert.deepStrictEqual(receiver, obj);
  assert.deepStrictEqual(args, [6, 7, 8]);

  ret = 10;
  assert.strictEqual(binding.callWithFunctionOperator(testFunction, 7, 8, 9), ret);
  assert.strictEqual(receiver, undefined);
  assert.deepStrictEqual(args, [7, 8, 9]);

  assert.throws(() => {
    binding.callWithInvalidReceiver();
  }, /Invalid (pointer passed as )?argument/);

  obj = binding.callConstructorWithArgs(testConstructor, 5, 6, 7);
  assert(obj instanceof testConstructor);
  assert.deepStrictEqual(args, [5, 6, 7]);

  obj = binding.callConstructorWithVector(testConstructor, 6, 7, 8);
  assert(obj instanceof testConstructor);
  assert.deepStrictEqual(args, [6, 7, 8]);

  obj = binding.callConstructorWithCStyleArray(testConstructor, 7, 8, 9);
  assert(obj instanceof testConstructor);
  assert.deepStrictEqual(args, [7, 8, 9]);

  obj = {};
  assert.deepStrictEqual(binding.voidCallbackWithData(obj), undefined);
  assert.deepStrictEqual(obj, { foo: 'bar', data: 1 });

  assert.deepStrictEqual(binding.valueCallbackWithData(), { foo: 'bar', data: 1 });

  assert.strictEqual(binding.voidCallback.name, 'voidCallback');
  assert.strictEqual(binding.valueCallback.name, 'valueCallback');

  let testConstructCall;
  binding.isConstructCall((result) => { testConstructCall = result; });
  assert.ok(!testConstructCall);
  /* eslint-disable no-new, new-cap */
  new binding.isConstructCall((result) => { testConstructCall = result; });
  /* eslint-enable no-new, new-cap */
  assert.ok(testConstructCall);

  obj = {};
  binding.makeCallbackWithArgs(makeCallbackTestFunction(obj, '1', '2', '3'), obj, '1', '2', '3');
  binding.makeCallbackWithVector(makeCallbackTestFunction(obj, 4, 5, 6), obj, 4, 5, 6);
  binding.makeCallbackWithCStyleArray(makeCallbackTestFunction(obj, 7, 8, 9), obj, 7, 8, 9);
  assert.throws(() => {
    binding.makeCallbackWithInvalidReceiver(() => {});
  });
}

function testLambda (binding) {
  assert.ok(binding.lambdaWithNoCapture());
  assert.ok(binding.lambdaWithCapture());
  assert.ok(binding.lambdaWithMoveOnlyCapture());
}
