'use strict';

const assert = require('assert');
const common = require('./common');

// we only check async hooks on 8.x an higher were
// they are closer to working properly
const nodeVersion = process.versions.node.split('.')[0];
let asyncHooks;
function checkAsyncHooks () {
  if (nodeVersion >= 8) {
    if (asyncHooks === undefined) {
      asyncHooks = require('async_hooks');
    }
    return true;
  }
  return false;
}

module.exports = common.runTest(test);

function installAsyncHooksForTest (resName) {
  return new Promise((resolve, reject) => {
    let id;
    const events = [];
    /**
     * TODO(legendecas): investigate why resolving & disabling hooks in
     * destroy callback causing crash with case 'callbackscope.js'.
     */
    let destroyed = false;
    const hook = asyncHooks.createHook({
      init (asyncId, type, triggerAsyncId, resource) {
        if (id === undefined && type === resName) {
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
    }).enable();

    const interval = setInterval(() => {
      if (destroyed) {
        hook.disable();
        clearInterval(interval);
        resolve(events);
      }
    }, 10);
  });
}

async function makeCallbackWithResource (binding) {
  const hooks = installAsyncHooksForTest('async_context_test');
  const triggerAsyncId = asyncHooks.executionAsyncId();
  await new Promise((resolve, reject) => {
    binding.asynccontext.makeCallback(common.mustCall(), { foo: 'foo' });
    hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'async_context_test',
          triggerAsyncId: triggerAsyncId,
          resource: { foo: 'foo' }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
    resolve();
  });
}

async function makeCallbackWithoutResource (binding) {
  const hooks = installAsyncHooksForTest('async_context_no_res_test');
  const triggerAsyncId = asyncHooks.executionAsyncId();
  await new Promise((resolve, reject) => {
    binding.asynccontext.makeCallbackNoResource(common.mustCall());
    hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'async_context_no_res_test',
          triggerAsyncId: triggerAsyncId,
          resource: { }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
    resolve();
  });
}

function assertAsyncContextReturnsCorrectEnv (binding) {
  assert.strictEqual(binding.asynccontext.asyncCxtReturnCorrectEnv(), true);
}

async function test (binding) {
  if (!checkAsyncHooks()) {
    return;
  }

  await makeCallbackWithResource(binding);
  await makeCallbackWithoutResource(binding);
  assertAsyncContextReturnsCorrectEnv(binding);
}
