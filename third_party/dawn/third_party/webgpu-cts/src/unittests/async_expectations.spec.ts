/* eslint-disable @typescript-eslint/require-await */
export const description = `
Tests for eventualAsyncExpectation and immediateAsyncExpectation.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { makeTestGroupForUnitTesting } from '../common/internal/test_group.js';
import { assert, objectEquals, rejectOnTimeout, resolveOnTimeout } from '../common/util/util.js';

import { TestGroupTest } from './test_group_test.js';
import { UnitTest } from './unit_test.js';

class FixtureToTest extends UnitTest {
  public override immediateAsyncExpectation<T>(fn: () => Promise<T>): Promise<T> {
    return super.immediateAsyncExpectation(fn);
  }
  public override eventualAsyncExpectation<T>(fn: (niceStack: Error) => Promise<T>): void {
    super.eventualAsyncExpectation(fn);
  }
}

export const g = makeTestGroup(TestGroupTest);

g.test('eventual').fn(async t0 => {
  const g = makeTestGroupForUnitTesting(FixtureToTest);

  const runState = [0, 0, 0, 0];
  let runStateIndex = 0;

  // Should pass in state 3
  g.test('noawait,resolve').fn(t => {
    const idx = runStateIndex++;

    runState[idx] = 1;
    t.eventualAsyncExpectation(async () => {
      runState[idx] = 2;
      await resolveOnTimeout(50);
      runState[idx] = 3;
    });
    runState[idx] = 4;
  });

  // Should fail in state 4
  g.test('noawait,reject').fn(t => {
    const idx = runStateIndex++;

    runState[idx] = 1;
    t.eventualAsyncExpectation(async () => {
      runState[idx] = 2;
      await rejectOnTimeout(50, 'rejected 1');
      runState[idx] = 3;
    });
    runState[idx] = 4;
  });

  // Should fail in state 3
  g.test('nested,2').fn(t => {
    const idx = runStateIndex++;

    runState[idx] = 1;
    t.eventualAsyncExpectation(async () => {
      runState[idx] = 2;
      await resolveOnTimeout(50); // Wait a bit before adding a new eventualAsyncExpectation
      t.eventualAsyncExpectation(() => rejectOnTimeout(100, 'inner rejected 1'));
      runState[idx] = 3;
    });
    runState[idx] = 4;
  });

  // Should fail in state 3
  g.test('nested,4').fn(t => {
    const idx = runStateIndex++;

    runState[idx] = 1;
    t.eventualAsyncExpectation(async () => {
      t.eventualAsyncExpectation(async () => {
        t.eventualAsyncExpectation(async () => {
          runState[idx] = 2;
          await resolveOnTimeout(50); // Wait a bit before adding a new eventualAsyncExpectation
          t.eventualAsyncExpectation(() => rejectOnTimeout(100, 'inner rejected 2'));
          runState[idx] = 3;
        });
      });
    });
    runState[idx] = 4;
  });

  const resultsPromise = t0.run(g);
  assert(objectEquals(runState, [0, 0, 0, 0]));

  const statuses = Array.from(await resultsPromise).map(([, v]) => v.status);
  assert(objectEquals(runState, [3, 4, 3, 3]), () => runState.toString());
  assert(objectEquals(statuses, ['pass', 'fail', 'fail', 'fail']), () => statuses.toString());
});

g.test('immediate').fn(async t0 => {
  const g = makeTestGroupForUnitTesting(FixtureToTest);

  const runState = [0, 0, 0, 0, 0];

  g.test('noawait,resolve').fn(t => {
    runState[0] = 1;
    void t.immediateAsyncExpectation(async () => {
      runState[0] = 2;
      await resolveOnTimeout(50);
      runState[0] = 3;
    });
    runState[0] = 4;
  });

  // (Can't g.test('noawait,reject') because it causes a top-level Promise
  // rejection which crashes Node.)

  g.test('await,resolve').fn(async t => {
    runState[1] = 1;
    await t.immediateAsyncExpectation(async () => {
      runState[1] = 2;
      await resolveOnTimeout(50);
      runState[1] = 3;
    });
  });

  g.test('await,reject').fn(async t => {
    runState[2] = 1;
    await t.immediateAsyncExpectation(async () => {
      runState[2] = 2;
      await rejectOnTimeout(50, 'rejected 3');
      runState[2] = 3;
    });
  });

  // (Similarly can't test 'nested,noawait'.)

  g.test('nested,await,2').fn(t => {
    runState[3] = 1;
    t.eventualAsyncExpectation(async () => {
      runState[3] = 2;
      await resolveOnTimeout(50); // Wait a bit before adding a new immediateAsyncExpectation
      runState[3] = 3;
      await t.immediateAsyncExpectation(() => rejectOnTimeout(100, 'inner rejected 3'));
      runState[3] = 5;
    });
    runState[3] = 4;
  });

  g.test('nested,await,4').fn(t => {
    runState[4] = 1;
    t.eventualAsyncExpectation(async () => {
      t.eventualAsyncExpectation(async () => {
        t.eventualAsyncExpectation(async () => {
          runState[4] = 2;
          await resolveOnTimeout(50); // Wait a bit before adding a new immediateAsyncExpectation
          runState[4] = 3;
          await t.immediateAsyncExpectation(() => rejectOnTimeout(100, 'inner rejected 3'));
          runState[4] = 5;
        });
      });
    });
    runState[4] = 4;
  });

  const resultsPromise = t0.run(g);
  assert(objectEquals(runState, [0, 0, 0, 0, 0]));

  const statuses = Array.from(await resultsPromise).map(([, v]) => v.status);
  assert(objectEquals(runState, [3, 3, 2, 3, 3]));
  assert(objectEquals(statuses, ['fail', 'pass', 'fail', 'fail', 'fail']));
});
