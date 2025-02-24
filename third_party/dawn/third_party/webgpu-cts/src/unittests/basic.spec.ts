export const description = `
Basic unit tests for test framework.
`;

import { makeTestGroup } from '../common/framework/test_group.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('test,sync').fn(_t => {});

g.test('test,async').fn(async _t => {});

g.test('test_with_params,sync')
  .paramsSimple([{}])
  .fn(t => {
    t.debug(JSON.stringify(t.params));
  });

g.test('test_with_params,async')
  .paramsSimple([{}])
  .fn(t => {
    t.debug(JSON.stringify(t.params));
  });

g.test('test_with_params,private_params')
  .paramsSimple([
    { a: 1, b: 2, _result: 3 }, //
    { a: 4, b: -3, _result: 1 },
  ])
  .fn(t => {
    const { a, b, _result } = t.params;
    t.expect(a + b === _result);
  });
