'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  function testProperty (obj, key, value, nativeGetProperty, nativeSetProperty) {
    nativeSetProperty(obj, key, value);
    assert.strictEqual(nativeGetProperty(obj, key), value);
  }

  testProperty({}, 'key', 'value', binding.object.subscriptGetWithCStyleString, binding.object.subscriptSetWithCStyleString);
  testProperty({ key: 'override me' }, 'key', 'value', binding.object.subscriptGetWithCppStyleString, binding.object.subscriptSetWithCppStyleString);
  testProperty({}, 0, 'value', binding.object.subscriptGetAtIndex, binding.object.subscriptSetAtIndex);
  testProperty({ key: 'override me' }, 0, 'value', binding.object.subscriptGetAtIndex, binding.object.subscriptSetAtIndex);
}
