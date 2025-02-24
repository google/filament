'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function test (binding) {
  const expected = '123456789';

  assert.throws(binding.name.nullStringShouldThrow, {
    name: 'Error',
    message: 'Error in native callback'
  });
  assert.ok(binding.name.checkString(expected, 'utf8'));
  assert.ok(binding.name.checkString(expected, 'utf16'));
  assert.ok(binding.name.checkString(expected.substr(0, 3), 'utf8', 3));
  assert.ok(binding.name.checkString(expected.substr(0, 3), 'utf16', 3));

  const str1 = binding.name.createString('utf8');
  assert.strictEqual(str1, expected);
  assert.ok(binding.name.checkString(str1, 'utf8'));
  assert.ok(binding.name.checkString(str1, 'utf16'));

  const substr1 = binding.name.createString('utf8', 3);
  assert.strictEqual(substr1, expected.substr(0, 3));
  assert.ok(binding.name.checkString(substr1, 'utf8', 3));
  assert.ok(binding.name.checkString(substr1, 'utf16', 3));

  const str2 = binding.name.createString('utf16');
  assert.strictEqual(str1, expected);
  assert.ok(binding.name.checkString(str2, 'utf8'));
  assert.ok(binding.name.checkString(str2, 'utf16'));

  const substr2 = binding.name.createString('utf16', 3);
  assert.strictEqual(substr1, expected.substr(0, 3));
  assert.ok(binding.name.checkString(substr2, 'utf8', 3));
  assert.ok(binding.name.checkString(substr2, 'utf16', 3));

  // eslint-disable-next-line symbol-description
  assert.ok(binding.name.checkSymbol(Symbol()));
  assert.ok(binding.name.checkSymbol(Symbol('test')));

  const sym1 = binding.name.createSymbol();
  assert.strictEqual(typeof sym1, 'symbol');
  assert.ok(binding.name.checkSymbol(sym1));

  const sym2 = binding.name.createSymbol('test');
  assert.strictEqual(typeof sym2, 'symbol');
  assert.ok(binding.name.checkSymbol(sym1));

  // Check for off-by-one errors which might only appear for strings of certain sizes,
  // due to how std::string increments its capacity in chunks.
  const longString = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  for (let i = 10; i <= longString.length; i++) {
    const str = longString.substr(0, i);
    assert.strictEqual(binding.name.echoString(str, 'utf8'), str);
    assert.strictEqual(binding.name.echoString(str, 'utf16'), str);
  }
}
