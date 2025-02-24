'use strict';

const except = require('bindings')('addon');
const noexcept = require('bindings')('addon_noexcept');

module.exports = {
  except,
  noexcept
};
