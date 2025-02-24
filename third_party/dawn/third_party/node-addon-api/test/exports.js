'use strict';

const { strictEqual } = require('assert');
const { valid } = require('semver');

const nodeAddonApi = require('../');

module.exports = function test () {
  strictEqual(nodeAddonApi.include.startsWith('"'), true);
  strictEqual(nodeAddonApi.include.endsWith('"'), true);
  strictEqual(nodeAddonApi.include.includes('node-addon-api'), true);
  strictEqual(nodeAddonApi.include_dir, '');
  strictEqual(nodeAddonApi.gyp, 'node_api.gyp:nothing');
  strictEqual(nodeAddonApi.targets, 'node_addon_api.gyp');
  strictEqual(valid(nodeAddonApi.version), true);
  strictEqual(nodeAddonApi.version, require('../package.json').version);
  strictEqual(nodeAddonApi.isNodeApiBuiltin, true);
  strictEqual(nodeAddonApi.needsFlag, false);
};
