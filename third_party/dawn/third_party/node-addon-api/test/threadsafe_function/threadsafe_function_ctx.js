'use strict';

const assert = require('assert');

module.exports = require('../common').runTest(test);

async function test (binding) {
  const ctx = { };
  const tsfn = new binding.threadsafe_function_ctx.TSFNWrap(ctx);
  assert(tsfn.getContext() === ctx);
  await tsfn.release();
  binding.threadsafe_function_ctx.AssertFnReturnCorrectCxt();
}
