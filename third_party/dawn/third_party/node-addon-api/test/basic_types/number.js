/* eslint-disable no-lone-blocks */
'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

function test (binding) {
  const MIN_INT32 = -2147483648;
  const MAX_INT32 = 2147483647;
  const MIN_UINT32 = 0;
  const MAX_UINT32 = 4294967295;
  const MIN_INT64 = Number.MIN_SAFE_INTEGER;
  const MAX_INT64 = Number.MAX_SAFE_INTEGER;
  const MIN_FLOAT = binding.basic_types_number.minFloat();
  const MAX_FLOAT = binding.basic_types_number.maxFloat();
  const MIN_DOUBLE = binding.basic_types_number.minDouble();
  const MAX_DOUBLE = binding.basic_types_number.maxDouble();

  function randomRangeTestForInteger (min, max, converter) {
    for (let i = min; i < max; i += Math.floor(Math.random() * max / 100)) {
      assert.strictEqual(i, converter(i));
    }
  }

  // Test for 32bit signed integer [-2147483648, 2147483647]
  {
    // Range tests
    randomRangeTestForInteger(MIN_INT32, MAX_INT32, binding.basic_types_number.toInt32);
    assert.strictEqual(MIN_INT32, binding.basic_types_number.toInt32(MIN_INT32));
    assert.strictEqual(MAX_INT32, binding.basic_types_number.toInt32(MAX_INT32));

    // Overflow tests
    assert.notStrictEqual(MAX_INT32 + 1, binding.basic_types_number.toInt32(MAX_INT32 + 1));
    assert.notStrictEqual(MIN_INT32 - 1, binding.basic_types_number.toInt32(MIN_INT32 - 1));
  }

  // Test for 32bit unsigned integer [0, 4294967295]
  {
    // Range tests
    randomRangeTestForInteger(MIN_UINT32, MAX_UINT32, binding.basic_types_number.toUint32);
    assert.strictEqual(MIN_UINT32, binding.basic_types_number.toUint32(MIN_UINT32));
    assert.strictEqual(MAX_UINT32, binding.basic_types_number.toUint32(MAX_UINT32));

    // Overflow tests
    assert.notStrictEqual(MAX_UINT32 + 1, binding.basic_types_number.toUint32(MAX_UINT32 + 1));
    assert.notStrictEqual(MIN_UINT32 - 1, binding.basic_types_number.toUint32(MIN_UINT32 - 1));
  }

  // Test for 64bit signed integer
  {
    // Range tests
    randomRangeTestForInteger(MIN_INT64, MAX_INT64, binding.basic_types_number.toInt64);
    assert.strictEqual(MIN_INT64, binding.basic_types_number.toInt64(MIN_INT64));
    assert.strictEqual(MAX_INT64, binding.basic_types_number.toInt64(MAX_INT64));

    // The int64 type can't be represented with full precision in JavaScript.
    // So, we are not able to do overflow test here.
    // Please see https://tc39.github.io/ecma262/#sec-ecmascript-language-types-number-type.
  }

  // Test for float type (might be single-precision 32bit IEEE 754 floating point number)
  {
    // Range test
    assert.strictEqual(MIN_FLOAT, binding.basic_types_number.toFloat(MIN_FLOAT));
    assert.strictEqual(MAX_FLOAT, binding.basic_types_number.toFloat(MAX_FLOAT));

    // Overflow test
    assert.strictEqual(0, binding.basic_types_number.toFloat(MIN_FLOAT * MIN_FLOAT));
    assert.strictEqual(Infinity, binding.basic_types_number.toFloat(MAX_FLOAT * MAX_FLOAT));
  }

  // Test for double type (is double-precision 64 bit IEEE 754 floating point number)
  {
    assert.strictEqual(MIN_DOUBLE, binding.basic_types_number.toDouble(MIN_DOUBLE));
    assert.strictEqual(MAX_DOUBLE, binding.basic_types_number.toDouble(MAX_DOUBLE));

    // Overflow test
    assert.strictEqual(0, binding.basic_types_number.toDouble(MIN_DOUBLE * MIN_DOUBLE));
    assert.strictEqual(Infinity, binding.basic_types_number.toDouble(MAX_DOUBLE * MAX_DOUBLE));
  }

  // Test for operator overloading
  {
    assert.strictEqual(binding.basic_types_number.operatorInt32(MIN_INT32), true);
    assert.strictEqual(binding.basic_types_number.operatorInt32(MAX_INT32), true);
    assert.strictEqual(binding.basic_types_number.operatorUint32(MIN_UINT32), true);
    assert.strictEqual(binding.basic_types_number.operatorUint32(MAX_UINT32), true);
    assert.strictEqual(binding.basic_types_number.operatorInt64(MIN_INT64), true);
    assert.strictEqual(binding.basic_types_number.operatorInt64(MAX_INT64), true);
    assert.strictEqual(binding.basic_types_number.operatorFloat(MIN_FLOAT), true);
    assert.strictEqual(binding.basic_types_number.operatorFloat(MAX_FLOAT), true);
    assert.strictEqual(binding.basic_types_number.operatorFloat(MAX_DOUBLE), true);
    assert.strictEqual(binding.basic_types_number.operatorDouble(MIN_DOUBLE), true);
    assert.strictEqual(binding.basic_types_number.operatorDouble(MAX_DOUBLE), true);
  }

  // Construction test
  {
    assert.strictEqual(binding.basic_types_number.createEmptyNumber(), true);
    randomRangeTestForInteger(MIN_INT32, MAX_INT32, binding.basic_types_number.createNumberFromExistingValue);
    assert.strictEqual(MIN_INT32, binding.basic_types_number.createNumberFromExistingValue(MIN_INT32));
    assert.strictEqual(MAX_INT32, binding.basic_types_number.createNumberFromExistingValue(MAX_INT32));
    randomRangeTestForInteger(MIN_UINT32, MAX_UINT32, binding.basic_types_number.createNumberFromExistingValue);
    assert.strictEqual(MIN_UINT32, binding.basic_types_number.createNumberFromExistingValue(MIN_UINT32));
    assert.strictEqual(MAX_UINT32, binding.basic_types_number.createNumberFromExistingValue(MAX_UINT32));
    randomRangeTestForInteger(MIN_INT64, MAX_INT64, binding.basic_types_number.createNumberFromExistingValue);
    assert.strictEqual(MIN_INT64, binding.basic_types_number.createNumberFromExistingValue(MIN_INT64));
    assert.strictEqual(MAX_INT64, binding.basic_types_number.createNumberFromExistingValue(MAX_INT64));
    assert.strictEqual(MIN_FLOAT, binding.basic_types_number.createNumberFromExistingValue(MIN_FLOAT));
    assert.strictEqual(MAX_FLOAT, binding.basic_types_number.createNumberFromExistingValue(MAX_FLOAT));
    assert.strictEqual(MIN_DOUBLE, binding.basic_types_number.createNumberFromExistingValue(MIN_DOUBLE));
    assert.strictEqual(MAX_DOUBLE, binding.basic_types_number.createNumberFromExistingValue(MAX_DOUBLE));
  }
}
