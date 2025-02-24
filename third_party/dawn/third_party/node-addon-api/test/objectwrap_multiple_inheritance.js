'use strict';

const assert = require('assert');

const test = bindingName => {
  const binding = require(bindingName);
  const TestMI = binding.objectwrap_multiple_inheritance.TestMI;
  const testmi = new TestMI();

  assert.strictEqual(testmi.test, 0);
};

module.exports = require('./common').runTestWithBindingPath(test);
