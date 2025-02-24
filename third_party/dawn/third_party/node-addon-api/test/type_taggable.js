'use strict';

const assert = require('assert');

module.exports = require('./common').runTest(test);

function testTypeTaggable ({ typeTaggedInstance, checkTypeTag }) {
  const obj1 = typeTaggedInstance(0);
  const obj2 = typeTaggedInstance(1);

  // Verify that type tags are correctly accepted.
  assert.strictEqual(checkTypeTag(0, obj1), true);
  assert.strictEqual(checkTypeTag(1, obj2), true);

  // Verify that wrongly tagged objects are rejected.
  assert.strictEqual(checkTypeTag(0, obj2), false);
  assert.strictEqual(checkTypeTag(1, obj1), false);

  // Verify that untagged objects are rejected.
  assert.strictEqual(checkTypeTag(0, {}), false);
  assert.strictEqual(checkTypeTag(1, {}), false);

  // Node v14 and v16 have an issue checking type tags if the `upper` in
  // `napi_type_tag` is 0, so these tests can only be performed on Node version
  // >=18. See:
  // - https://github.com/nodejs/node/issues/43786
  // - https://github.com/nodejs/node/pull/43788
  const nodeVersion = parseInt(process.versions.node.split('.')[0]);
  if (nodeVersion < 18) {
    return;
  }

  const obj3 = typeTaggedInstance(2);
  const obj4 = typeTaggedInstance(3);

  // Verify that untagged objects are rejected.
  assert.strictEqual(checkTypeTag(0, {}), false);
  assert.strictEqual(checkTypeTag(1, {}), false);

  // Verify that type tags are correctly accepted.
  assert.strictEqual(checkTypeTag(0, obj1), true);
  assert.strictEqual(checkTypeTag(1, obj2), true);
  assert.strictEqual(checkTypeTag(2, obj3), true);
  assert.strictEqual(checkTypeTag(3, obj4), true);

  // Verify that wrongly tagged objects are rejected.
  assert.strictEqual(checkTypeTag(0, obj2), false);
  assert.strictEqual(checkTypeTag(1, obj1), false);
  assert.strictEqual(checkTypeTag(0, obj3), false);
  assert.strictEqual(checkTypeTag(1, obj4), false);
  assert.strictEqual(checkTypeTag(2, obj4), false);
  assert.strictEqual(checkTypeTag(3, obj3), false);
  assert.strictEqual(checkTypeTag(4, obj3), false);
}

// eslint-disable-next-line camelcase
function test ({ type_taggable }) {
  testTypeTaggable(type_taggable.external);
  testTypeTaggable(type_taggable.object);
}
