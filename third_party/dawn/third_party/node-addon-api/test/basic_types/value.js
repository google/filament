'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  const externalValue = binding.basic_types_value.createExternal();

  function isObject (value) {
    return (typeof value === 'object' && value !== externalValue) ||
           (typeof value === 'function');
  }

  function detailedTypeOf (value) {
    const type = typeof value;
    if (type !== 'object') { return type; }

    if (value === null) { return 'null'; }

    if (Array.isArray(value)) { return 'array'; }

    if (value === externalValue) { return 'external'; }

    if (!value.constructor) { return type; }

    if (value instanceof ArrayBuffer) { return 'arraybuffer'; }

    if (ArrayBuffer.isView(value)) {
      if (value instanceof DataView) {
        return 'dataview';
      } else {
        return 'typedarray';
      }
    }

    if (value instanceof Promise) { return 'promise'; }

    return 'object';
  }

  function typeCheckerTest (typeChecker, expectedType) {
    const testValueList = [
      undefined,
      null,
      true,
      10,
      'string',
      Symbol('symbol'),
      [],
      new ArrayBuffer(10),
      new Int32Array(new ArrayBuffer(12)),
      {},
      function () {},
      new Promise((resolve, reject) => {}),
      new DataView(new ArrayBuffer(12)),
      externalValue
    ];

    testValueList.forEach((testValue) => {
      if (testValue !== null && expectedType === 'object') {
        assert.strictEqual(typeChecker(testValue), isObject(testValue));
      } else {
        assert.strictEqual(typeChecker(testValue), detailedTypeOf(testValue) === expectedType);
      }
    });
  }

  function typeConverterTest (typeConverter, expectedType) {
    const testValueList = [
      true,
      false,
      0,
      10,
      'string',
      [],
      new ArrayBuffer(10),
      new Int32Array(new ArrayBuffer(12)),
      {},
      function () {},
      new Promise((resolve, reject) => {})
    ];

    testValueList.forEach((testValue) => {
      const expected = expectedType(testValue);
      const actual = typeConverter(testValue);

      if (isNaN(expected)) {
        assert(isNaN(actual));
      } else {
        assert.deepStrictEqual(typeConverter(testValue), expectedType(testValue));
      }
    });
  }

  function assertValueStrictlyEqual (value) {
    const newValue = value.createNonEmptyValue();
    assert(value.strictlyEquals(newValue, newValue));
    assert(value.strictlyEqualsOverload(newValue, newValue));
  }

  function assertValueStrictlyNonEqual (value) {
    const valueA = value.createNonEmptyValue();
    const valueB = value.createExternal();
    assert(value.strictlyNotEqualsOverload(valueA, valueB));
  }

  function assertValueReturnsCorrectEnv (value) {
    assert(value.assertValueReturnsCorrectEnv());
  }

  function assertEmptyValueNullPtrOnCast (value) {
    assert(value.assertEmptyValReturnNullPtrOnCast());
  }

  function assertNonEmptyReturnValOnCast (value) {
    assert(value.assertNonEmptyReturnValOnCast());
  }

  const value = binding.basic_types_value;

  assertValueStrictlyEqual(value);
  assertValueStrictlyNonEqual(value);
  assertValueReturnsCorrectEnv(value);
  assertEmptyValueNullPtrOnCast(value);
  assertNonEmptyReturnValOnCast(value);

  typeCheckerTest(value.isUndefined, 'undefined');
  typeCheckerTest(value.isNull, 'null');
  typeCheckerTest(value.isBoolean, 'boolean');
  typeCheckerTest(value.isNumber, 'number');
  typeCheckerTest(value.isString, 'string');
  typeCheckerTest(value.isSymbol, 'symbol');
  typeCheckerTest(value.isArray, 'array');
  typeCheckerTest(value.isArrayBuffer, 'arraybuffer');
  typeCheckerTest(value.isTypedArray, 'typedarray');
  typeCheckerTest(value.isObject, 'object');
  typeCheckerTest(value.isFunction, 'function');
  typeCheckerTest(value.isPromise, 'promise');
  typeCheckerTest(value.isDataView, 'dataview');
  typeCheckerTest(value.isExternal, 'external');

  typeConverterTest(value.toBoolean, Boolean);
  assert.strictEqual(value.toBoolean(undefined), false);
  assert.strictEqual(value.toBoolean(null), false);

  typeConverterTest(value.toNumber, Number);
  assert(isNaN(value.toNumber(undefined)));
  assert.strictEqual(value.toNumber(null), 0);

  typeConverterTest(value.toString, String);
  assert.strictEqual(value.toString(undefined), 'undefined');
  assert.strictEqual(value.toString(null), 'null');

  typeConverterTest(value.toObject, Object);
}
