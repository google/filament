'use strict';

const assert = require('assert');
const common = require('../common');

module.exports = {
  testCall: async binding => {
    const { testCall } = binding.threadsafe_function_exception;

    await new Promise(resolve => {
      process.once('uncaughtException', common.mustCall(err => {
        assert.strictEqual(err.message, 'test');
        resolve();
      }, 1));

      testCall(common.mustCall(() => {
        throw new Error('test');
      }, 1));
    });
  },
  testCallWithNativeCallback: async binding => {
    const { testCallWithNativeCallback } = binding.threadsafe_function_exception;

    await new Promise(resolve => {
      process.once('uncaughtException', common.mustCall(err => {
        assert.strictEqual(err.message, 'test-from-native');
        resolve();
      }, 1));

      testCallWithNativeCallback();
    });
  }
};
