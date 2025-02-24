export const description = `
Test for "parseImports" utility.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { parseImports } from '../common/util/parse_imports.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  test(content: string, expect: string[]): void {
    const got = parseImports('a/b/c.js', content);
    const expectJoined = expect.join('\n');
    const gotJoined = got.join('\n');
    this.expect(
      expectJoined === gotJoined,
      `
expected: ${expectJoined}
got:      ${gotJoined}`
    );
  }
}

export const g = makeTestGroup(F);

g.test('empty').fn(t => {
  t.test(``, []);
  t.test(`\n`, []);
  t.test(`\n\n`, []);
});

g.test('simple').fn(t => {
  t.test(`import 'x/y/z.js';`, ['a/b/x/y/z.js']);
  t.test(`import * as blah from 'x/y/z.js';`, ['a/b/x/y/z.js']);
  t.test(`import { blah } from 'x/y/z.js';`, ['a/b/x/y/z.js']);
});

g.test('multiple').fn(t => {
  t.test(
    `
blah blah blah
import 'x/y/z.js';
more blah
import * as blah from 'm/n/o.js';
extra blah
import { blah } from '../h.js';
ending with blah
`,
    ['a/b/x/y/z.js', 'a/b/m/n/o.js', 'a/h.js']
  );
});

g.test('multiline').fn(t => {
  t.test(
    `import {
  blah
} from 'x/y/z.js';`,
    ['a/b/x/y/z.js']
  );
  t.test(
    `import {
  blahA,
  blahB,
} from 'x/y/z.js';`,
    ['a/b/x/y/z.js']
  );
});

g.test('file_characters').fn(t => {
  t.test(`import '01234_56789.js';`, ['a/b/01234_56789.js']);
});

g.test('relative_paths').fn(t => {
  t.test(`import '../x.js';`, ['a/x.js']);
  t.test(`import '../x/y.js';`, ['a/x/y.js']);
  t.test(`import '../../x.js';`, ['x.js']);
  t.test(`import '../../../x.js';`, ['../x.js']);
  t.test(`import '../../../../x.js';`, ['../../x.js']);
});
