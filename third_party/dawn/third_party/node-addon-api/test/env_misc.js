'use strict';

const assert = require('assert');
const { pathToFileURL } = require('url');

module.exports = require('./common').runTest(test);

function test (binding, { bindingPath } = {}) {
  const path = binding.env_misc.get_module_file_name();
  const bindingFileUrl = pathToFileURL(bindingPath).toString();
  assert(bindingFileUrl === path);
}
