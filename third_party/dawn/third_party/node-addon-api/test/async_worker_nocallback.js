'use strict';

const common = require('./common');

module.exports = common.runTest(test);

async function test (binding) {
  await binding.asyncworker.doWorkAsyncResNoCallback(true, {})
    .then(common.mustCall()).catch(common.mustNotCall());

  await binding.asyncworker.doWorkAsyncResNoCallback(false, {})
    .then(common.mustNotCall()).catch(common.mustCall());

  await binding.asyncworker.doWorkNoCallback(false)
    .then(common.mustNotCall()).catch(common.mustCall());

  await binding.asyncworker.doWorkNoCallback(true)
    .then(common.mustNotCall()).catch(common.mustCall());
}
