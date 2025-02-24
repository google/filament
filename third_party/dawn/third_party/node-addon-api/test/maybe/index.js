'use strict';

const assert = require('assert');
const { whichBuildType } = require('../common');

const napiChild = require('../napi_child');

module.exports = async function wrapTest () {
  const buildType = await whichBuildType();
  test(require(`../build/${buildType}/binding_noexcept_maybe.node`).maybe_check);
};

function test (binding) {
  if (process.argv.includes('child')) {
    child(binding);
    return;
  }
  const cp = napiChild.spawn(process.execPath, [__filename, 'child'], {
    stdio: ['ignore', 'inherit', 'pipe']
  });
  cp.stderr.setEncoding('utf8');
  let stderr = '';
  cp.stderr.on('data', chunk => {
    stderr += chunk;
  });
  cp.on('exit', (code, signal) => {
    if (process.platform === 'win32') {
      assert.strictEqual(code, 128 + 6 /* SIGABRT */);
    } else {
      assert.strictEqual(signal, 'SIGABRT');
    }
    assert.ok(stderr.match(/FATAL ERROR: Napi::Maybe::Check Maybe value is Nothing./));
  });
}

function child (binding) {
  const MAGIC_NUMBER = 12459062;
  binding.normalJsCallback(() => {
    return MAGIC_NUMBER;
  }, MAGIC_NUMBER);

  binding.testMaybeOverloadOp(
    () => { return MAGIC_NUMBER; },
    () => { throw Error('Foobar'); }
  );

  binding.voidCallback(() => {
    throw new Error('foobar');
  });
}
