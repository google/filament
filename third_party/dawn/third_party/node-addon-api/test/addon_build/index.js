'use strict';

const { promisify } = require('util');
const exec = promisify(require('child_process').exec);
const { copy, remove } = require('fs-extra');
const path = require('path');
const assert = require('assert');

const ADDONS_FOLDER = path.join(__dirname, 'addons');

const addons = [
  'echo addon',
  'echo-addon'
];

async function beforeAll (addons) {
  console.log('   >Preparing native addons to build');
  for (const addon of addons) {
    await remove(path.join(ADDONS_FOLDER, addon));
    await copy(path.join(__dirname, 'tpl'), path.join(ADDONS_FOLDER, addon));
  }
}

async function test (addon) {
  console.log(`   >Building addon: '${addon}'`);
  await exec('npm install', {
    cwd: path.join(ADDONS_FOLDER, addon)
  });
  console.log(`   >Running test for: '${addon}'`);
  // Disabled the checks on stderr and stdout because of this issue on npm:
  // Stop using process.umask(): https://github.com/npm/cli/issues/1103
  // We should enable the following checks again after the resolution of
  // the reported issue.
  // assert.strictEqual(stderr, '');
  // assert.ok(stderr.length === 0);
  // assert.ok(stdout.length > 0);
  const binding = require(`${ADDONS_FOLDER}/${addon}`);
  assert.strictEqual(binding.except.echo('except'), 'except');
  assert.strictEqual(binding.except.echo(101), 101);
  assert.strictEqual(binding.noexcept.echo('noexcept'), 'noexcept');
  assert.strictEqual(binding.noexcept.echo(103), 103);
}

module.exports = (async function () {
  await beforeAll(addons);
  for (const addon of addons) {
    await test(addon);
  }
})();
