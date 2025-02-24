export const description = 'Description for c.spec.ts';

import { makeTestGroup } from '../../../common/framework/test_group.js';
import { unreachable } from '../../../common/util/util.js';
import { UnitTest } from '../../../unittests/unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('f')
  .desc(
    `Test plan for f
    - Test stuff
    - Test some more stuff`
  )
  .fn(() => {});

g.test('f,g').fn(() => {});

g.test('f,g,h')
  .paramsSimple([{}, { x: 0 }, { x: 0, y: 0 }])
  .fn(() => {});

g.test('case_depth_2_in_single_child_test')
  .paramsSimple([{ x: 0, y: 0 }])
  .fn(() => {});

g.test('deep_case_tree')
  .params(u =>
    u //
      .combine('x', [1, 2])
      .combine('y', [1, 2])
      .combine('z', [1, 2])
  )
  .fn(() => {});

g.test('statuses,debug').fn(t => {
  t.debug('debug');
});

g.test('statuses,skip').fn(t => {
  t.skip('skip');
});

g.test('statuses,warn').fn(t => {
  t.warn('warn');
});

g.test('statuses,fail').fn(t => {
  t.fail('fail');
});

g.test('statuses,throw').fn(() => {
  unreachable('unreachable');
});

g.test('multiple_same_stack').fn(t => {
  for (let i = 0; i < 3; ++i) {
    t.fail(
      i === 2
        ? 'this should appear after deduplicated line'
        : 'this should be "seen 2 times with identical stack"'
    );
  }
});

g.test('multiple_same_level').fn(t => {
  t.fail('this should print a stack');
  t.fail('this should print a stack');
  t.fail('this should not print a stack');
});

g.test('lower_levels_hidden,before').fn(t => {
  t.warn('warn - this should not print a stack');
  t.fail('fail');
});

g.test('lower_levels_hidden,after').fn(t => {
  t.fail('fail');
  t.warn('warn - this should not print a stack');
});
