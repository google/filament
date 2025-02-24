'use strict';
const assert = require('assert');

module.exports = {
  workingCode: binding => {
    const addon = binding.addon();
    assert.strictEqual(addon.increment(), 43);
    assert.strictEqual(addon.increment(), 44);
    assert.strictEqual(addon.subObject.decrement(), 43);
  }
};
