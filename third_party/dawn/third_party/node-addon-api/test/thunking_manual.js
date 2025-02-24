// Flags: --expose-gc
'use strict';

module.exports = require('./common').runTest(test);

function test (binding) {
  console.log('Thunking: Performing initial GC');
  global.gc();
  console.log('Thunking: Creating test object');
  let object = binding.thunking_manual.createTestObject();
  // eslint-disable-next-line no-unused-vars
  object = null;
  console.log('Thunking: About to GC\n--------');
  global.gc();
  console.log('--------\nThunking: GC complete');
}
