'use strict';

const { create, coverage, globals } = require('./dawn.node');

Object.assign(globalThis, globals);

module.exports = { create, coverage };
