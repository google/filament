export const description = `
Tests for TestQuery comparison
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { compareQueries, Ordering } from '../common/internal/query/compare.js';
import {
  TestQuery,
  TestQuerySingleCase,
  TestQueryMultiFile,
  TestQueryMultiTest,
  TestQueryMultiCase,
} from '../common/internal/query/query.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  expectQ(a: TestQuery, exp: '<' | '=' | '>' | '!', b: TestQuery) {
    const [expOrdering, expInvOrdering] =
      exp === '<'
        ? [Ordering.StrictSubset, Ordering.StrictSuperset]
        : exp === '='
        ? [Ordering.Equal, Ordering.Equal]
        : exp === '>'
        ? [Ordering.StrictSuperset, Ordering.StrictSubset]
        : [Ordering.Unordered, Ordering.Unordered];
    {
      const act = compareQueries(a, b);
      this.expect(act === expOrdering, `${a} ${b}  got ${act}, exp ${expOrdering}`);
    }
    {
      const act = compareQueries(a, b);
      this.expect(act === expOrdering, `${b} ${a}  got ${act}, exp ${expInvOrdering}`);
    }
  }

  expectWellOrdered(...qs: TestQuery[]) {
    for (let i = 0; i < qs.length; ++i) {
      this.expectQ(qs[i], '=', qs[i]);
      for (let j = i + 1; j < qs.length; ++j) {
        this.expectQ(qs[i], '>', qs[j]);
      }
    }
  }

  expectUnordered(...qs: TestQuery[]) {
    for (let i = 0; i < qs.length; ++i) {
      this.expectQ(qs[i], '=', qs[i]);
      for (let j = i + 1; j < qs.length; ++j) {
        this.expectQ(qs[i], '!', qs[j]);
      }
    }
  }
}

export const g = makeTestGroup(F);

// suite:*  >  suite:a,*  >  suite:a,b,*   >  suite:a,b:*
// suite:a,b:*  >  suite:a,b:c,*  >  suite:a,b:c,d,*  >  suite:a,b:c,d:*
// suite:a,b:c,d:*  >  suite:a,b:c,d:x=1;*  >  suite:a,b:c,d:x=1;y=2;*  >  suite:a,b:c,d:x=1;y=2
// suite:a;* (unordered) suite:b;*
g.test('well_ordered').fn(t => {
  t.expectWellOrdered(
    new TestQueryMultiFile('suite', []),
    new TestQueryMultiFile('suite', ['a']),
    new TestQueryMultiFile('suite', ['a', 'b']),
    new TestQueryMultiTest('suite', ['a', 'b'], []),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c']),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'd']),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], {}),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 1 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 })
  );
  t.expectWellOrdered(
    new TestQueryMultiFile('suite', []),
    new TestQueryMultiFile('suite', ['a']),
    new TestQueryMultiFile('suite', ['a', 'b']),
    new TestQueryMultiTest('suite', ['a', 'b'], []),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c']),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'd']),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], {}),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], {})
  );
});

g.test('unordered').fn(t => {
  t.expectUnordered(
    new TestQueryMultiFile('suite', ['a']), //
    new TestQueryMultiFile('suite', ['x'])
  );
  t.expectUnordered(
    new TestQueryMultiFile('suite', ['a', 'b']),
    new TestQueryMultiFile('suite', ['a', 'x'])
  );
  t.expectUnordered(
    new TestQueryMultiTest('suite', ['a', 'b'], ['c']),
    new TestQueryMultiTest('suite', ['a', 'b'], ['x']),
    new TestQueryMultiTest('suite', ['a'], []),
    new TestQueryMultiTest('suite', ['a', 'x'], [])
  );
  t.expectUnordered(
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'd']),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'x']),
    new TestQueryMultiTest('suite', ['a'], []),
    new TestQueryMultiTest('suite', ['a', 'x'], [])
  );
  t.expectUnordered(
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'd']),
    new TestQueryMultiTest('suite', ['a', 'b'], ['c', 'x']),
    new TestQueryMultiTest('suite', ['a'], []),
    new TestQueryMultiTest('suite', ['a', 'x'], ['c'])
  );
  t.expectUnordered(
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 1 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 9 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c'], { x: 9 })
  );
  t.expectUnordered(
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 8 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c'], { x: 1, y: 8 })
  );
  t.expectUnordered(
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: 1, y: 8 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c'], { x: 1, y: 8 })
  );
  t.expectUnordered(
    new TestQuerySingleCase('suite1', ['bar', 'buzz', 'buzz'], ['zap'], {}),
    new TestQueryMultiTest('suite1', ['bar'], [])
  );
  // Expect that 0.0 and -0.0 are treated as different queries
  t.expectUnordered(
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: 0.0 }),
    new TestQueryMultiCase('suite', ['a', 'b'], ['c', 'd'], { x: -0.0 })
  );
  t.expectUnordered(
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: 0.0, y: 0.0 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: 0.0, y: -0.0 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: -0.0, y: 0.0 }),
    new TestQuerySingleCase('suite', ['a', 'b'], ['c', 'd'], { x: -0.0, y: -0.0 })
  );
});
