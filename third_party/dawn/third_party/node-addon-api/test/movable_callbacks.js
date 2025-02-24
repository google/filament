'use strict';

const common = require('./common');
const testUtil = require('./testUtil');

module.exports = require('./common').runTest(binding => test(binding.movable_callbacks));

async function test (binding) {
  await testUtil.runGCTests([
    'External',
    () => {
      const fn = common.mustCall(() => {
        // noop
      }, 1);
      binding.createExternal(fn);
    },
    () => {
      // noop, wait for gc
    }
  ]);
}
