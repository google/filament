'use strict';

const assert = require('assert');

// Make sure the instance data finalizer is called at process exit. If the hint
// is non-zero, it will be printed out by the child process.
const cleanupTest = (binding, hint) => {
  binding.addon_data(hint).verbose = true;
};

module.exports = {
  workingCode: binding => {
    const addonData = binding.addon_data(0);

    // Make sure it is possible to get/set instance data.
    assert.strictEqual(addonData.verbose.verbose, false);
    addonData.verbose = true;
    assert.strictEqual(addonData.verbose.verbose, true);
    addonData.verbose = false;
    assert.strictEqual(addonData.verbose.verbose, false);
  },
  cleanupWithHint: binding => cleanupTest(binding, 42),
  cleanupWithoutHint: binding => cleanupTest(binding, 0)
};
