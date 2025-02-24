'use strict';

module.exports = require('./common').runTestInChildProcess({
  suite: 'addon',
  testName: 'workingCode',
  expectedStderr: ['TestAddon::~TestAddon']
});
