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

function installAsyncHooksForTest () {
  return new Promise((resolve, reject) => {
    let id;
    const events = [];
    /**
     * TODO(legendecas): investigate why resolving & disabling hooks in
     * destroy callback causing crash with case 'callbackscope.js'.
     */
    let destroyed = false;
    const interval = setInterval(() => {
      if (destroyed) {
        hook.disable();
        clearInterval(interval);
        resolve(events);
      }
    }, 10);

    const hook = asyncHooks.createHook({
      init (asyncId, type, triggerAsyncId, resource) {
        if (id === undefined && type === 'TestResource') {
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
  });
}

async function test (binding) {
  const libUvThreadCount = Number(process.env.UV_THREADPOOL_SIZE || 4);
  binding.asyncworker.tryCancelQueuedWork(() => {}, 'echoString', libUvThreadCount);

  let taskFailed = false;
  try {
    binding.asyncworker.expectCancelToFail(() => {});
  } catch (e) {
    taskFailed = true;
  }

  assert.equal(taskFailed, true, 'We expect task cancellation to fail');

  if (!checkAsyncHooks()) {
    binding.asyncworker.expectCustomAllocWorkerToDealloc(() => {});

    await new Promise((resolve) => {
      const obj = { data: 'test data' };
      binding.asyncworker.doWorkRecv(obj, function (e) {
        assert.strictEqual(typeof e, 'undefined');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      });
    });

    await new Promise((resolve) => {
      binding.asyncworker.doWork(true, null, function (e) {
        assert.strictEqual(typeof e, 'undefined');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, 'test data');
    });

    await new Promise((resolve) => {
      binding.asyncworker.doWork(false, {}, function (e) {
        assert.ok(e instanceof Error);
        assert.strictEqual(e.message, 'test error');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, 'test data');
    });

    await new Promise((resolve) => {
      binding.asyncworker.doWorkWithResult(true, {}, function (succeed, succeedString) {
        assert(arguments.length === 2);
        assert(succeed);
        assert(succeedString === 'ok');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, 'test data');
    });

    return;
  }

  {
    const hooks = installAsyncHooksForTest();
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve) => {
      const recvObj = { data: 'test data' };
      binding.asyncworker.doWithRecvAsyncRes(recvObj, function (e) {
        assert.strictEqual(typeof e, 'undefined');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, { foo: 'fooBar' });
    });

    await hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'TestResource',
          triggerAsyncId: triggerAsyncId,
          resource: { foo: 'fooBar' }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
  }

  {
    const hooks = installAsyncHooksForTest();
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve) => {
      const recvObj = { data: 'test data' };
      binding.asyncworker.doWithRecvAsyncRes(recvObj, function (e) {
        assert.strictEqual(typeof e, 'undefined');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, null);
    });

    await hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'TestResource',
          triggerAsyncId: triggerAsyncId,
          resource: { }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
  }

  {
    const hooks = installAsyncHooksForTest();
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve) => {
      binding.asyncworker.doWork(true, { foo: 'foo' }, function (e) {
        assert.strictEqual(typeof e, 'undefined');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, 'test data');
    });

    await hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'TestResource',
          triggerAsyncId: triggerAsyncId,
          resource: { foo: 'foo' }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
  }

  {
    const hooks = installAsyncHooksForTest();
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve) => {
      binding.asyncworker.doWorkWithResult(true, { foo: 'foo' },
        function (succeed, succeedString) {
          assert(arguments.length === 2);
          assert(succeed);
          assert(succeedString === 'ok');
          assert.strictEqual(typeof this, 'object');
          assert.strictEqual(this.data, 'test data');
          resolve();
        }, 'test data');
    });

    await hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'TestResource',
          triggerAsyncId: triggerAsyncId,
          resource: { foo: 'foo' }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
  }

  {
    const hooks = installAsyncHooksForTest();
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve) => {
      binding.asyncworker.doWork(false, { foo: 'foo' }, function (e) {
        assert.ok(e instanceof Error);
        assert.strictEqual(e.message, 'test error');
        assert.strictEqual(typeof this, 'object');
        assert.strictEqual(this.data, 'test data');
        resolve();
      }, 'test data');
    });

    await hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'TestResource',
          triggerAsyncId: triggerAsyncId,
          resource: { foo: 'foo' }
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
    }).catch(common.mustNotCall());
  }
}
