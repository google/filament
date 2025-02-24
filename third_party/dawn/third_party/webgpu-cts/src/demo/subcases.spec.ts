export const description = 'Tests with subcases';

import { makeTestGroup } from '../common/framework/test_group.js';
import { UnitTest } from '../unittests/unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('pass_warn_fail')
  .params(u =>
    u
      .combine('x', [1, 2, 3]) //
      .beginSubcases()
      .combine('y', [1, 2, 3])
  )
  .fn(t => {
    const { x, y } = t.params;
    if (x + y > 5) {
      t.fail();
    } else if (x + y > 4) {
      t.warn();
    }
  });

g.test('DOMException,cases')
  .params(u => u.combine('fail', [false, true]))
  .fn(t => {
    if (t.params.fail) {
      throw new DOMException('Message!', 'Name!');
    }
  });

g.test('DOMException,subcases')
  .paramsSubcasesOnly(u => u.combine('fail', [false, true]))
  .fn(t => {
    if (t.params.fail) {
      throw new DOMException('Message!', 'Name!');
    }
  });
