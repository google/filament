'use strict';

const common = require('./common');
const assert = require('assert');

module.exports = common.runTest(test);
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

async function test ({ asyncprogressqueueworker }) {
  await success(asyncprogressqueueworker);
  await fail(asyncprogressqueueworker);

  await asyncProgressWorkerCallbackOverloads(asyncprogressqueueworker.runWorkerWithCb);
  await asyncProgressWorkerRecvOverloads(asyncprogressqueueworker.runWorkerWithRecv);
  await asyncProgressWorkerNoCbOverloads(asyncprogressqueueworker.runWorkerNoCb);
}

async function asyncProgressWorkerCallbackOverloads (bindingFunction) {
  bindingFunction(common.mustCall());
  if (!checkAsyncHooks()) {
    return;
  }

  const hooks = common.installAysncHooks('cbResources');

  const triggerAsyncId = asyncHooks.executionAsyncId();
  await new Promise((resolve, reject) => {
    bindingFunction(common.mustCall(), 'cbResources');
    hooks.then(actual => {
      assert.deepStrictEqual(actual, [
        {
          eventName: 'init',
          type: 'cbResources',
          triggerAsyncId: triggerAsyncId,
          resource: {}
        },
        { eventName: 'before' },
        { eventName: 'after' },
        { eventName: 'destroy' }
      ]);
      resolve();
    }).catch((err) => reject(err));
  });
}

async function asyncProgressWorkerRecvOverloads (bindingFunction) {
  const recvObject = {
    a: 4
  };

  function cb () {
    assert.strictEqual(this.a, recvObject.a);
  }

  bindingFunction(recvObject, common.mustCall(cb));
  if (!checkAsyncHooks()) {
    return;
  }
  const asyncResources = [
    { resName: 'cbRecvResources', resObject: {} },
    { resName: 'cbRecvResourcesObject', resObject: { foo: 'bar' } }
  ];

  for (const asyncResource of asyncResources) {
    const asyncResName = asyncResource.resName;
    const asyncResObject = asyncResource.resObject;

    const hooks = common.installAysncHooks(asyncResource.resName);
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve, reject) => {
      if (Object.keys(asyncResObject).length === 0) {
        bindingFunction(recvObject, common.mustCall(cb), asyncResName);
      } else {
        bindingFunction(recvObject, common.mustCall(cb), asyncResName, asyncResObject);
      }

      hooks.then(actual => {
        assert.deepStrictEqual(actual, [
          {
            eventName: 'init',
            type: asyncResName,
            triggerAsyncId: triggerAsyncId,
            resource: asyncResObject
          },
          { eventName: 'before' },
          { eventName: 'after' },
          { eventName: 'destroy' }
        ]);
        resolve();
      }).catch((err) => reject(err));
    });
  }
}

async function asyncProgressWorkerNoCbOverloads (bindingFunction) {
  bindingFunction(common.mustCall());
  if (!checkAsyncHooks()) {
    return;
  }
  const asyncResources = [
    { resName: 'noCbResources', resObject: {} },
    { resName: 'noCbResourcesObject', resObject: { foo: 'bar' } }
  ];

  for (const asyncResource of asyncResources) {
    const asyncResName = asyncResource.resName;
    const asyncResObject = asyncResource.resObject;

    const hooks = common.installAysncHooks(asyncResource.resName);
    const triggerAsyncId = asyncHooks.executionAsyncId();
    await new Promise((resolve, reject) => {
      if (Object.keys(asyncResObject).length === 0) {
        bindingFunction(asyncResName, common.mustCall(() => {}));
      } else {
        bindingFunction(asyncResName, asyncResObject, common.mustCall(() => {}));
      }

      hooks.then(actual => {
        assert.deepStrictEqual(actual, [
          {
            eventName: 'init',
            type: asyncResName,
            triggerAsyncId: triggerAsyncId,
            resource: asyncResObject
          },
          { eventName: 'before' },
          { eventName: 'after' },
          { eventName: 'destroy' }
        ]);
        resolve();
      }).catch((err) => reject(err));
    });
  }
}

function success (binding) {
  return new Promise((resolve, reject) => {
    const expected = [0, 1, 2, 3];
    const actual = [];
    const worker = binding.createWork(expected.length,
      common.mustCall((err) => {
        if (err) {
          reject(err);
        } else {
          // All queued items shall be invoked before complete callback.
          assert.deepEqual(actual, expected);
          resolve();
        }
      }),
      common.mustCall((_progress) => {
        actual.push(_progress);
      }, expected.length)
    );
    binding.queueWork(worker);
  });
}

function fail (binding) {
  return new Promise((resolve, reject) => {
    const worker = binding.createWork(-1,
      common.mustCall((err) => {
        assert.throws(() => { throw err; }, /test error/);
        resolve();
      }),
      common.mustNotCall()
    );
    binding.queueWork(worker);
  });
}
