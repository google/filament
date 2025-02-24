export const description = `
Unit tests for parameterization.
`;

import { TestParams } from '../common/framework/fixture.js';
import { kUnitCaseParamsBuilder } from '../common/framework/params_builder.js';
import { makeTestGroup } from '../common/framework/test_group.js';
import { makeTestGroupForUnitTesting } from '../common/internal/test_group.js';

import { TestGroupTest } from './test_group_test.js';
import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(TestGroupTest);

g.test('combine_none,arg_unit')
  .params(u => u.combineWithParams([]))
  .fn(t => {
    t.fail("this test shouldn't run");
  });

g.test('combine_none,arg_ignored')
  .params(() => kUnitCaseParamsBuilder.combineWithParams([]))
  .fn(t => {
    t.fail("this test shouldn't run");
  });

g.test('combine_none,plain_builder')
  .params(kUnitCaseParamsBuilder.combineWithParams([]))
  .fn(t => {
    t.fail("this test shouldn't run");
  });

g.test('combine_none,plain_array')
  .paramsSimple([])
  .fn(t => {
    t.fail("this test shouldn't run");
  });

g.test('combine_one,case')
  .params(u =>
    u //
      .combineWithParams([{ x: 1 }])
  )
  .fn(t => {
    t.expect(t.params.x === 1);
  });

g.test('combine_one,subcase')
  .paramsSubcasesOnly(u =>
    u //
      .combineWithParams([{ x: 1 }])
  )
  .fn(t => {
    t.expect(t.params.x === 1);
  });

g.test('filter')
  .params(u =>
    u
      .combineWithParams([
        { a: true, x: 1 }, //
        { a: false, y: 2 },
      ])
      .filter(p => p.a)
  )
  .fn(t => {
    t.expect(t.params.a);
  });

g.test('unless')
  .params(u =>
    u
      .combineWithParams([
        { a: true, x: 1 }, //
        { a: false, y: 2 },
      ])
      .unless(p => p.a)
  )
  .fn(t => {
    t.expect(!t.params.a);
  });

g.test('generator').fn(t0 => {
  const g = makeTestGroupForUnitTesting(UnitTest);

  const ran: TestParams[] = [];

  g.test('generator')
    .params(u =>
      u.combineWithParams({
        *[Symbol.iterator]() {
          for (let x = 0; x < 3; ++x) {
            for (let y = 0; y < 2; ++y) {
              yield { x, y };
            }
          }
        },
      })
    )
    .fn(t => {
      ran.push(t.params);
    });

  t0.expectCases(g, [
    { test: ['generator'], params: { x: 0, y: 0 } },
    { test: ['generator'], params: { x: 0, y: 1 } },
    { test: ['generator'], params: { x: 1, y: 0 } },
    { test: ['generator'], params: { x: 1, y: 1 } },
    { test: ['generator'], params: { x: 2, y: 0 } },
    { test: ['generator'], params: { x: 2, y: 1 } },
  ]);
});
