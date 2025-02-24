'use strict';

const assert = require('assert');

module.exports = require('./common').runTest((binding) => {
  test(binding.errorHandlingPrim);
});

function canThrow (binding, errorMessage, errorType) {
  try {
    binding.errorHandlingPrim(() => {
      throw errorMessage;
    });
  } catch (e) {
    // eslint-disable-next-line valid-typeof
    assert(typeof e === errorType);
    assert(e === errorMessage);
  }
}

function test (binding) {
  canThrow(binding, '404 server not found!', 'string');
  canThrow(binding, 42, 'number');
  canThrow(binding, Symbol.for('newSym'), 'symbol');
  canThrow(binding, false, 'boolean');
  canThrow(binding, BigInt(123), 'bigint');
  canThrow(binding, () => { console.log('Logger shutdown incorrectly'); }, 'function');
  canThrow(binding, { status: 403, errorMsg: 'Not authenticated' }, 'object');
}
