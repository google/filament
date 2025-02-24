'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  function testSetProperty (nativeSetProperty, key = 'test') {
    const obj = {};
    assert.strictEqual(nativeSetProperty(obj, key, 1), true);
    assert.strictEqual(obj[key], 1);
  }

  function testShouldThrowErrorIfKeyIsInvalid (nativeSetProperty) {
    assert.throws(() => {
      nativeSetProperty(undefined, 'test', 1);
    }, /Cannot convert undefined or null to object/);
  }

  testSetProperty(binding.object.setPropertyWithNapiValue);
  testSetProperty(binding.object.setPropertyWithNapiWrapperValue);
  testSetProperty(binding.object.setPropertyWithCStyleString);
  testSetProperty(binding.object.setPropertyWithCppStyleString);
  testSetProperty(binding.object.setPropertyWithUint32, 12);

  testShouldThrowErrorIfKeyIsInvalid(binding.object.setPropertyWithNapiValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.setPropertyWithNapiWrapperValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.setPropertyWithCStyleString);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.setPropertyWithCppStyleString);
}
