'use strict';

const assert = require('assert');
const asyncHook = require('async_hooks');

module.exports = require('./common').runTest(async (binding) => {
  await test(binding.functionreference);
});

function installAsyncHook () {
  let id;
  let destroyed;
  let hook;
  const events = [];
  return new Promise((resolve, reject) => {
    const interval = setInterval(() => {
      if (destroyed) {
        hook.disable();
        clearInterval(interval);
        resolve(events);
      }
    }, 10);

    hook = asyncHook
      .createHook({
        init (asyncId, type, triggerAsyncId, resource) {
          if (id === undefined && type === 'func_ref_resources') {
            id = asyncId;
            events.push({ eventName: 'init', type, triggerAsyncId, resource });
          }
        },
        before (asyncId) {
          if (asyncId === id) {
            events.push({ eventName: 'before' });
          }
        },
        after (asyncId) {
          if (asyncId === id) {
            events.push({ eventName: 'after' });
          }
        },
        destroy (asyncId) {
          if (asyncId === id) {
            events.push({ eventName: 'destroy' });
            destroyed = true;
          }
        }
      })
      .enable();
  });
}

function canConstructRefFromExistingRef (binding) {
  const testFunc = () => 240;
  assert(binding.ConstructWithMove(testFunc) === 240);
}

function canCallFunctionWithDifferentOverloads (binding) {
  let outsideRef = {};
  const testFunc = (a, b) => a * a - b * b;
  const testFuncB = (a, b, c) => a + b - c * c;
  const testFuncC = (a, b, c) => {
    outsideRef.a = a;
    outsideRef.b = b;
    outsideRef.c = c;
  };
  const testFuncD = (a, b, c, d) => {
    outsideRef.result = a + b * c - d;
    return outsideRef.result;
  };

  assert(binding.CallWithVec(testFunc, 5, 4) === testFunc(5, 4));
  assert(binding.CallWithInitList(testFuncB, 2, 4, 5) === testFuncB(2, 4, 5));

  binding.CallWithRecvVector(testFuncC, outsideRef, 1, 2, 4);
  assert(outsideRef.a === 1 && outsideRef.b === 2 && outsideRef.c === 4);

  outsideRef = {};
  binding.CallWithRecvInitList(testFuncC, outsideRef, 1, 2, 4);
  assert(outsideRef.a === 1 && outsideRef.b === 2 && outsideRef.c === 4);

  outsideRef = {};
  binding.CallWithRecvArgc(testFuncD, outsideRef, 2, 4, 5, 6);
  assert(outsideRef.result === testFuncD(2, 4, 5, 6));
}

async function canCallAsyncFunctionWithDifferentOverloads (binding) {
  const testFunc = () => 2100;
  const testFuncB = (a, b, c, d) => a + b + c + d;
  let hook = installAsyncHook();
  binding.AsyncCallWithInitList(testFunc);
  let triggerAsyncId = asyncHook.executionAsyncId();
  let res = await hook;
  assert.deepStrictEqual(res, [
    {
      eventName: 'init',
      type: 'func_ref_resources',
      triggerAsyncId: triggerAsyncId,
      resource: {}
    },
    { eventName: 'before' },
    { eventName: 'after' },
    { eventName: 'destroy' }
  ]);

  hook = installAsyncHook();
  triggerAsyncId = asyncHook.executionAsyncId();
  assert(
    binding.AsyncCallWithVector(testFuncB, 2, 4, 5, 6) === testFuncB(2, 4, 5, 6)
  );
  res = await hook;
  assert.deepStrictEqual(res, [
    {
      eventName: 'init',
      type: 'func_ref_resources',
      triggerAsyncId: triggerAsyncId,
      resource: {}
    },
    { eventName: 'before' },
    { eventName: 'after' },
    { eventName: 'destroy' }
  ]);

  hook = installAsyncHook();
  triggerAsyncId = asyncHook.executionAsyncId();
  assert(
    binding.AsyncCallWithArgv(testFuncB, 2, 4, 5, 6) === testFuncB(2, 4, 5, 6)
  );
}
async function test (binding) {
  const e = new Error('foobar');
  const functionMayThrow = () => {
    throw e;
  };
  const classMayThrow = class {
    constructor () {
      throw e;
    }
  };

  const newRef = binding.CreateFuncRefWithNew(120);
  assert(newRef.getValue() === 120);

  const newRefWithVecArg = binding.CreateFuncRefWithNewVec(80);
  assert(newRefWithVecArg.getValue() === 80);

  assert.throws(() => {
    binding.call(functionMayThrow);
  }, /foobar/);
  assert.throws(() => {
    binding.construct(classMayThrow);
  }, /foobar/);

  canConstructRefFromExistingRef(binding);
  canCallFunctionWithDifferentOverloads(binding);
  await canCallAsyncFunctionWithDifferentOverloads(binding);
}
