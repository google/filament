export const description = `
Unit tests for TestQuery strings.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { compareQueries, Ordering } from '../common/internal/query/compare.js';
import {
  TestQuery,
  TestQuerySingleCase,
  TestQueryMultiCase,
  TestQueryMultiTest,
  TestQueryMultiFile,
  relativeQueryString,
} from '../common/internal/query/query.js';

import { UnitTest } from './unit_test.js';

class T extends UnitTest {
  expectQueryString(q: TestQuery, exp: string): void {
    const s = q.toString();
    this.expect(s === exp, `got ${s} expected ${exp}`);
  }

  expectRelativeQueryString(parent: TestQuery, child: TestQuery, exp: string): void {
    const s = relativeQueryString(parent, child);
    this.expect(s === exp, `got ${s} expected ${exp}`);

    if (compareQueries(parent, child) !== Ordering.Equal) {
      // Test in reverse
      this.shouldThrow('Error', () => {
        relativeQueryString(child, parent);
      });
    }
  }
}

export const g = makeTestGroup(T);

g.test('stringifyQuery,single_case').fn(t => {
  t.expectQueryString(
    new TestQuerySingleCase('a', ['b_1', '2_c'], ['d_3', '4_e'], {
      f: 'g',
      _pri1: 0,
      x: 3,
      _pri2: 1,
    }),
    'a:b_1,2_c:d_3,4_e:f="g";x=3'
  );
});

g.test('stringifyQuery,single_case,json').fn(t => {
  t.expectQueryString(
    new TestQuerySingleCase('a', ['b_1', '2_c'], ['d_3', '4_e'], {
      f: 'g',
      x: { p: 2, q: 'Q' },
    }),
    'a:b_1,2_c:d_3,4_e:f="g";x={"p":2,"q":"Q"}'
  );
});

g.test('stringifyQuery,multi_case').fn(t => {
  t.expectQueryString(
    new TestQueryMultiCase('a', ['b_1', '2_c'], ['d_3', '4_e'], {
      f: 'g',
      _pri1: 0,
      a: 3,
      _pri2: 1,
    }),
    'a:b_1,2_c:d_3,4_e:f="g";a=3;*'
  );

  t.expectQueryString(
    new TestQueryMultiCase('a', ['b_1', '2_c'], ['d_3', '4_e'], {}),
    'a:b_1,2_c:d_3,4_e:*'
  );
});

g.test('stringifyQuery,multi_test').fn(t => {
  t.expectQueryString(
    new TestQueryMultiTest('a', ['b_1', '2_c'], ['d_3', '4_e']),
    'a:b_1,2_c:d_3,4_e,*'
  );

  t.expectQueryString(
    new TestQueryMultiTest('a', ['b_1', '2_c'], []), //
    'a:b_1,2_c:*'
  );
});

g.test('stringifyQuery,multi_file').fn(t => {
  t.expectQueryString(
    new TestQueryMultiFile('a', ['b_1', '2_c']), //
    'a:b_1,2_c,*'
  );

  t.expectQueryString(
    new TestQueryMultiFile('a', []), //
    'a:*'
  );
});

g.test('relativeQueryString,equal_or_child').fn(t => {
  // Depth difference = 0
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', []), //
    new TestQueryMultiFile('a', []), //
    ''
  );
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b', 'c']), //
    new TestQueryMultiFile('a', ['b', 'c']), //
    ''
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    ''
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    ''
  );
  t.expectRelativeQueryString(
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ''
  );

  // Depth difference = 1
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', []), //
    new TestQueryMultiFile('a', ['b']), //
    ':b,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b']), //
    new TestQueryMultiFile('a', ['b', 'c']), //
    ',c,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b', 'c']), //
    new TestQueryMultiTest('a', ['b', 'c'], []), //
    ':*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], []), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    ':d,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    ',e,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], {}), //
    ':*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], {}), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    ':f=0;*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ';g=1;*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ''
  );

  // Depth difference = 2
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', []), //
    new TestQueryMultiFile('a', ['b', 'c']), //
    ':b,c,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b', 'c']), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    ':d,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], {}), //
    ',e:*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], {}), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ':f=0;g=1;*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1, h: 2 }), //
    ';h=2'
  );
  // Depth difference = 2
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b']), //
    new TestQueryMultiTest('a', ['b', 'c'], []), //
    ',c:*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], []), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    ':d,e,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    ':f=0;*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ';g=1'
  );

  // Depth difference = 4
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', []), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    ':b,c:d,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d']), //
    new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ',e:f=0;g=1;*'
  );
  // Depth difference = 4
  t.expectRelativeQueryString(
    new TestQueryMultiFile('a', ['b']), //
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    ',c:d,e,*'
  );
  t.expectRelativeQueryString(
    new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']), //
    new TestQuerySingleCase('a', ['b', 'c'], ['d', 'e'], { f: 0, g: 1 }), //
    ':f=0;g=1'
  );
});

g.test('relativeQueryString,unrelated').fn(t => {
  t.shouldThrow('Error', () => {
    relativeQueryString(
      new TestQueryMultiFile('a', ['b', 'x']), //
      new TestQueryMultiFile('a', ['b', 'c']) //
    );
  });
  t.shouldThrow('Error', () => {
    relativeQueryString(
      new TestQueryMultiTest('a', ['b', 'c'], ['d', 'x']), //
      new TestQueryMultiTest('a', ['b', 'c'], ['d', 'e']) //
    );
  });
  t.shouldThrow('Error', () => {
    relativeQueryString(
      new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 0 }), //
      new TestQueryMultiCase('a', ['b', 'c'], ['d', 'e'], { f: 1 }) //
    );
  });
});
