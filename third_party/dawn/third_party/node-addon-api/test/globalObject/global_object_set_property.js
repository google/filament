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

  function setGlobalObjectKeyValue (key, value, keyType) {
    switch (keyType) {
      case KEY_TYPE.CPP_STR:
        binding.globalObject.setPropertyWithCppStyleString(key, value);
        break;

      case KEY_TYPE.C_STR:
        binding.globalObject.setPropertyWithCStyleString(key, value);
        break;

      case KEY_TYPE.INT_32:
        binding.globalObject.setPropertyWithInt32(key, value);
        break;

      case KEY_TYPE.NAPI:
        binding.globalObject.setPropertyWithNapiValue(key, value);
        break;
    }
  }

  function assertErrMessageIsThrown (nativeObjectSetFunction, errMsg) {
    assert.throws(() => {
      nativeObjectSetFunction(undefined, 1);
    }, errMsg);
  }

  setGlobalObjectKeyValue('cKey', 'cValue', KEY_TYPE.CPP_STR);
  setGlobalObjectKeyValue(1, 10, KEY_TYPE.INT_32);
  setGlobalObjectKeyValue('napi_key', 'napi_value', KEY_TYPE.NAPI);
  setGlobalObjectKeyValue('cppKey', 'cppValue', KEY_TYPE.CPP_STR);
  setGlobalObjectKeyValue('circular', global, KEY_TYPE.NAPI);

  assert.deepStrictEqual(global.circular, global);
  assert.deepStrictEqual(global.cppKey, 'cppValue');
  assert.deepStrictEqual(global.napi_key, 'napi_value');
  assert.deepStrictEqual(global[1], 10);
  assert.deepStrictEqual(global.cKey, 'cValue');

  assertErrMessageIsThrown(binding.globalObject.setPropertyWithCppStyleString, 'Error: A string was expected');
  assertErrMessageIsThrown(binding.globalObject.setPropertyWithCStyleString, 'Error: A string was expected');
  assertErrMessageIsThrown(binding.globalObject.setPropertyWithInt32, 'Error: A number was expected');
}
