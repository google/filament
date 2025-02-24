export const description = `
Tests for TestQuery
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { parseQuery } from '../common/internal/query/parseQuery.js';
import {
  TestQueryMultiFile,
  TestQueryMultiTest,
  TestQueryMultiCase,
  TestQuerySingleCase,
  TestQuery,
} from '../common/internal/query/query.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  expectToString(q: TestQuery, exp: string) {
    this.expect(q.toString() === exp);
  }

  expectQueriesEqual(q1: TestQuery, q2: TestQuery) {
    this.expect(q1.level === q2.level);

    if (q1.level >= 1) {
      this.expect(q1.isMultiFile === q2.isMultiFile);
      this.expect(q1.suite === q2.suite);
      this.expect(q1.filePathParts.length === q2.filePathParts.length);
      for (let i = 0; i < q1.filePathParts.length; i++) {
        this.expect(q1.filePathParts[i] === q2.filePathParts[i]);
      }
    }

    if (q1.level >= 2) {
      const p1 = q1 as TestQueryMultiTest;
      const p2 = q2 as TestQueryMultiTest;

      this.expect(p1.isMultiTest === p2.isMultiTest);
      this.expect(p1.testPathParts.length === p2.testPathParts.length);
      for (let i = 0; i < p1.testPathParts.length; i++) {
        this.expect(p1.testPathParts[i] === p2.testPathParts[i]);
      }
    }

    if (q1.level >= 3) {
      const p1 = q1 as TestQueryMultiCase;
      const p2 = q2 as TestQueryMultiCase;

      this.expect(p1.isMultiCase === p2.isMultiCase);
      this.expect(Object.keys(p1.params).length === Object.keys(p2.params).length);
      for (const key of Object.keys(p1.params)) {
        this.expect(key in p2.params);
        const v1 = p1.params[key];
        const v2 = p2.params[key];
        this.expect(
          v1 === v2 ||
            (typeof v1 === 'number' && isNaN(v1)) === (typeof v2 === 'number' && isNaN(v2))
        );
        this.expect(Object.is(v1, -0) === Object.is(v2, -0));
      }
    }
  }

  expectQueryParse(s: string, q: TestQuery) {
    this.expectQueriesEqual(q, parseQuery(s));
  }
}

export const g = makeTestGroup(F);

g.test('constructor').fn(t => {
  t.shouldThrow('Error', () => new TestQueryMultiTest('suite', [], []));

  t.shouldThrow('Error', () => new TestQueryMultiCase('suite', ['a'], [], {}));
  t.shouldThrow('Error', () => new TestQueryMultiCase('suite', [], ['c'], {}));
  t.shouldThrow('Error', () => new TestQueryMultiCase('suite', [], [], {}));

  t.shouldThrow('Error', () => new TestQuerySingleCase('suite', ['a'], [], {}));
  t.shouldThrow('Error', () => new TestQuerySingleCase('suite', [], ['c'], {}));
  t.shouldThrow('Error', () => new TestQuerySingleCase('suite', [], [], {}));
});

g.test('toString').fn(t => {
  t.expectToString(new TestQueryMultiFile('s', []), 's:*');
  t.expectToString(new TestQueryMultiFile('s', ['a']), 's:a,*');
  t.expectToString(new TestQueryMultiFile('s', ['a', 'b']), 's:a,b,*');
  t.expectToString(new TestQueryMultiTest('s', ['a', 'b'], []), 's:a,b:*');
  t.expectToString(new TestQueryMultiTest('s', ['a', 'b'], ['c']), 's:a,b:c,*');
  t.expectToString(new TestQueryMultiTest('s', ['a', 'b'], ['c', 'd']), 's:a,b:c,d,*');
  t.expectToString(new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], {}), 's:a,b:c,d:*');
  t.expectToString(
    new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], { x: 1 }),
    's:a,b:c,d:x=1;*'
  );
  t.expectToString(
    new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 }),
    's:a,b:c,d:x=1;y=2;*'
  );
  t.expectToString(
    new TestQuerySingleCase('s', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 }),
    's:a,b:c,d:x=1;y=2'
  );
  t.expectToString(new TestQuerySingleCase('s', ['a', 'b'], ['c', 'd'], {}), 's:a,b:c,d:');

  // Test handling of magic param value that convert to NaN/undefined/Infinity/etc.
  t.expectToString(new TestQuerySingleCase('s', ['a'], ['b'], { c: NaN }), 's:a:b:c="_nan_"');
  t.expectToString(
    new TestQuerySingleCase('s', ['a'], ['b'], { c: undefined }),
    's:a:b:c="_undef_"'
  );
  t.expectToString(new TestQuerySingleCase('s', ['a'], ['b'], { c: -0 }), 's:a:b:c="_negzero_"');
});

g.test('parseQuery').fn(t => {
  t.expectQueryParse('s:*', new TestQueryMultiFile('s', []));
  t.expectQueryParse('s:a,*', new TestQueryMultiFile('s', ['a']));
  t.expectQueryParse('s:a,b,*', new TestQueryMultiFile('s', ['a', 'b']));
  t.expectQueryParse('s:a,b:*', new TestQueryMultiTest('s', ['a', 'b'], []));
  t.expectQueryParse('s:a,b:c,*', new TestQueryMultiTest('s', ['a', 'b'], ['c']));
  t.expectQueryParse('s:a,b:c,d,*', new TestQueryMultiTest('s', ['a', 'b'], ['c', 'd']));
  t.expectQueryParse('s:a,b:c,d:*', new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], {}));
  t.expectQueryParse(
    's:a,b:c,d:x=1;*',
    new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], { x: 1 })
  );
  t.expectQueryParse(
    's:a,b:c,d:x=1;y=2;*',
    new TestQueryMultiCase('s', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 })
  );
  t.expectQueryParse(
    's:a,b:c,d:x=1;y=2',
    new TestQuerySingleCase('s', ['a', 'b'], ['c', 'd'], { x: 1, y: 2 })
  );
  t.expectQueryParse('s:a,b:c,d:', new TestQuerySingleCase('s', ['a', 'b'], ['c', 'd'], {}));

  // Test handling of magic param value that convert to NaN/undefined/Infinity/etc.
  t.expectQueryParse('s:a:b:c="_nan_"', new TestQuerySingleCase('s', ['a'], ['b'], { c: NaN }));
  t.expectQueryParse(
    's:a:b:c="_undef_"',
    new TestQuerySingleCase('s', ['a'], ['b'], { c: undefined })
  );
  t.expectQueryParse('s:a:b:c="_negzero_"', new TestQuerySingleCase('s', ['a'], ['b'], { c: -0 }));
});
