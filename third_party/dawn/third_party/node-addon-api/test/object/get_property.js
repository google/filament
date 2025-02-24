'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  function testGetProperty (nativeGetProperty) {
    const obj = { test: 1 };
    assert.strictEqual(nativeGetProperty(obj, 'test'), 1);
  }

  function testShouldReturnUndefinedIfKeyIsNotPresent (nativeGetProperty) {
    const obj = { };
    assert.strictEqual(nativeGetProperty(obj, 'test'), undefined);
  }

  function testShouldThrowErrorIfKeyIsInvalid (nativeGetProperty) {
    assert.throws(() => {
      nativeGetProperty(undefined, 'test');
    }, /Cannot convert undefined or null to object/);
  }

  const testObject = { 42: 100 };
  const property = binding.object.getPropertyWithUint32(testObject, 42);
  assert.strictEqual(property, 100);

  const nativeFunctions = [
    binding.object.getPropertyWithNapiValue,
    binding.object.getPropertyWithNapiWrapperValue,
    binding.object.getPropertyWithCStyleString,
    binding.object.getPropertyWithCppStyleString
  ];

  nativeFunctions.forEach((nativeFunction) => {
    testGetProperty(nativeFunction);
    testShouldReturnUndefinedIfKeyIsNotPresent(nativeFunction);
    testShouldThrowErrorIfKeyIsInvalid(nativeFunction);
  });
}
