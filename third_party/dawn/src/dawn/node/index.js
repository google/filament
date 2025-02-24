'use strict';

const { create, coverage, globals } = require('./dawn.node');

module.exports = {
  ...globals,
  coverage,
  gpu: create(process.env.DAWN_FLAGS?.split(',') || []),
};
