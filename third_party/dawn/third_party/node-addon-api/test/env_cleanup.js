'use strict';

const assert = require('assert');

if (process.argv[2] === 'runInChildProcess') {
  const bindingPath = process.argv[3];
  const removeHooks = process.argv[4] === 'true';

  const binding = require(bindingPath);
  const actualAdded = binding.env_cleanup.addHooks(removeHooks);
  const expectedAdded = removeHooks === true ? 0 : 8;
  assert(actualAdded === expectedAdded, 'Incorrect number of hooks added');
} else {
  module.exports = require('./common').runTestWithBindingPath(test);
}

function test (bindingPath) {
  for (const removeHooks of [false, true]) {
    const { status, output } = require('./napi_child').spawnSync(
      process.execPath,
      [
        __filename,
        'runInChildProcess',
        bindingPath,
        removeHooks
      ],
      { encoding: 'utf8' }
    );

    const stdout = output[1].trim();
    /**
         * There is no need to sort the lines, as per Node-API documentation:
         *  > The hooks will be called in reverse order, i.e. the most recently
         *  > added one will be called first.
        */
    const lines = stdout.split(/[\r\n]+/);

    assert(status === 0, `Process aborted with status ${status}`);

    if (removeHooks) {
      assert.deepStrictEqual(lines, [''], 'Child process had console output when none expected');
    } else {
      assert.deepStrictEqual(lines, [
        'lambda cleanup()',
        'lambda cleanup(void)',
        'lambda cleanup(42)',
        'static cleanup()',
        'static cleanup()',
        'static cleanup(43)',
        'static cleanup(42)',
        'static cleanup(42)'
      ], 'Child process console output mismisatch');
    }
  }
}
