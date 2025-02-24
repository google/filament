'use strict';
const assert = require('assert');

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

module.exports = require('./common').runTest(test);

function test (binding) {
  if (!checkAsyncHooks()) { return; }

  let id;
  let insideHook = false;
  const hook = asyncHooks.createHook({
    init (asyncId, type, triggerAsyncId, resource) {
      if (id === undefined && (type === 'callback_scope_test' || type === 'existing_callback_scope_test')) {
        id = asyncId;
      }
    },
    before (asyncId) {
      if (asyncId === id) { insideHook = true; }
    },
    after (asyncId) {
      if (asyncId === id) { insideHook = false; }
    }
  }).enable();

  return new Promise(resolve => {
    binding.callbackscope.runInCallbackScope(function () {
      assert(insideHook);
      binding.callbackscope.runInPreExistingCbScope(function () {
        assert(insideHook);
        hook.disable();
        resolve();
      });
    });
  });
}
