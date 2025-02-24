'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  if (!('object_deprecated' in binding)) {
    return;
  }

  function assertPropertyIsNot (obj, key, attribute) {
    const propDesc = Object.getOwnPropertyDescriptor(obj, key);
    assert.ok(propDesc);
    assert.ok(!propDesc[attribute]);
  }

  function testDefineProperties (nameType) {
    const obj = {};
    binding.object.defineProperties(obj, nameType);

    assertPropertyIsNot(obj, 'readonlyAccessor', 'enumerable');
    assertPropertyIsNot(obj, 'readonlyAccessor', 'configurable');
    assert.strictEqual(obj.readonlyAccessor, true);

    assertPropertyIsNot(obj, 'readwriteAccessor', 'enumerable');
    assertPropertyIsNot(obj, 'readwriteAccessor', 'configurable');
    obj.readwriteAccessor = false;
    assert.strictEqual(obj.readwriteAccessor, false);
    obj.readwriteAccessor = true;
    assert.strictEqual(obj.readwriteAccessor, true);

    assertPropertyIsNot(obj, 'function', 'writable');
    assertPropertyIsNot(obj, 'function', 'enumerable');
    assertPropertyIsNot(obj, 'function', 'configurable');
    assert.strictEqual(obj.function(), true);
  }

  testDefineProperties('literal');
  testDefineProperties('string');
  testDefineProperties('value');
}
