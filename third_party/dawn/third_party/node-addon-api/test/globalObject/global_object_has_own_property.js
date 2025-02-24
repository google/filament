'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  const KEY_TYPE = {
    C_STR: 'KEY_AS_C_STRING',
    CPP_STR: 'KEY_AS_CPP_STRING',
    NAPI: 'KEY_AS_NAPI_VALUES',
    INT_32: 'KEY_AS_INT_32_NUM'
  };

  function assertGlobalObjectHasProperty (key, keyType) {
    switch (keyType) {
      case KEY_TYPE.NAPI:
        assert.strictEqual(binding.globalObject.hasPropertyWithNapiValue(key), true);
        break;

      case KEY_TYPE.C_STR:
        assert.strictEqual(binding.globalObject.hasPropertyWithCStyleString(key), true);
        break;

      case KEY_TYPE.CPP_STR:
        assert.strictEqual(binding.globalObject.hasPropertyWithCppStyleString(key), true);
        break;
    }
  }

  function assertErrMessageIsThrown (propertyCheckExistenceFunction, errMsg) {
    assert.throws(() => {
      propertyCheckExistenceFunction(undefined);
    }, errMsg);
  }

  binding.globalObject.createMockTestObject();
  assertGlobalObjectHasProperty('c_str_key', KEY_TYPE.C_STR);
  assertGlobalObjectHasProperty('cpp_string_key', KEY_TYPE.CPP_STR);
  assertGlobalObjectHasProperty('circular', KEY_TYPE.CPP_STR);
  assertGlobalObjectHasProperty('2', KEY_TYPE.NAPI);

  assertErrMessageIsThrown(binding.globalObject.hasPropertyWithCppStyleString, 'Error: A string was expected');
  assertErrMessageIsThrown(binding.globalObject.hasPropertyWithCStyleString, 'Error: A string was expected');
  assertErrMessageIsThrown(binding.globalObject.hasPropertyWithInt32, 'Error: A number was expected');
}
