'use strict';

const assert = require('assert');
const common = require('../common');

module.exports = {
  testCall: async binding => {
    const { testCall } = binding.typed_threadsafe_function_exception;

    await new Promise(resolve => {
      process.once('uncaughtException', common.mustCall(err => {
        assert.strictEqual(err.message, 'test-from-native');
        resolve();
      }, 1));

      testCall();
    });
  }
};
