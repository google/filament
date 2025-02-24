'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  function testDeleteProperty (nativeDeleteProperty) {
    const obj = { one: 1, two: 2 };
    Object.defineProperty(obj, 'three', { configurable: false, value: 3 });
    assert.strictEqual(nativeDeleteProperty(obj, 'one'), true);
    assert.strictEqual(nativeDeleteProperty(obj, 'missing'), true);

    /* Returns true for all cases except when the property is an own non-
       configurable property, in which case, false is returned in non-strict mode. */
    assert.strictEqual(nativeDeleteProperty(obj, 'three'), false);
    assert.deepStrictEqual(obj, { two: 2 });
  }

  function testShouldThrowErrorIfKeyIsInvalid (nativeDeleteProperty) {
    assert.throws(() => {
      nativeDeleteProperty(undefined, 'test');
    }, /Cannot convert undefined or null to object/);
  }

  const testObj = { 15: 42, three: 3 };

  binding.object.deletePropertyWithUint32(testObj, 15);

  assert.strictEqual(Object.prototype.hasOwnProperty.call(testObj, 15), false);

  testDeleteProperty(binding.object.deletePropertyWithNapiValue);
  testDeleteProperty(binding.object.deletePropertyWithNapiWrapperValue);
  testDeleteProperty(binding.object.deletePropertyWithCStyleString);
  testDeleteProperty(binding.object.deletePropertyWithCppStyleString);

  testShouldThrowErrorIfKeyIsInvalid(binding.object.deletePropertyWithNapiValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.deletePropertyWithNapiWrapperValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.deletePropertyWithCStyleString);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.deletePropertyWithCppStyleString);
}
