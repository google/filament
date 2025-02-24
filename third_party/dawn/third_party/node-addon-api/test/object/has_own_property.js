'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  function testHasOwnProperty (nativeHasOwnProperty) {
    const obj = { one: 1 };

    Object.defineProperty(obj, 'two', { value: 2 });

    assert.strictEqual(nativeHasOwnProperty(obj, 'one'), true);
    assert.strictEqual(nativeHasOwnProperty(obj, 'two'), true);
    assert.strictEqual('toString' in obj, true);
    assert.strictEqual(nativeHasOwnProperty(obj, 'toString'), false);
  }

  function testShouldThrowErrorIfKeyIsInvalid (nativeHasOwnProperty) {
    assert.throws(() => {
      nativeHasOwnProperty(undefined, 'test');
    }, /Cannot convert undefined or null to object/);
  }

  testHasOwnProperty(binding.object.hasOwnPropertyWithNapiValue);
  testHasOwnProperty(binding.object.hasOwnPropertyWithNapiWrapperValue);
  testHasOwnProperty(binding.object.hasOwnPropertyWithCStyleString);
  testHasOwnProperty(binding.object.hasOwnPropertyWithCppStyleString);

  testShouldThrowErrorIfKeyIsInvalid(binding.object.hasOwnPropertyWithNapiValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.hasOwnPropertyWithNapiWrapperValue);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.hasOwnPropertyWithCStyleString);
  testShouldThrowErrorIfKeyIsInvalid(binding.object.hasOwnPropertyWithCppStyleString);
}
