export const description = `
Test for "pp" preprocessor.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { pp } from '../common/util/preprocessor.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  test(act: string, exp: string): void {
    this.expect(act === exp, 'got: ' + act.replace('\n', '⏎'));
  }
}

export const g = makeTestGroup(F);

g.test('empty').fn(t => {
  t.test(pp``, '');
  t.test(pp`\n`, '\n');
  t.test(pp`\n\n`, '\n\n');
});

g.test('plain').fn(t => {
  t.test(pp`a`, 'a');
  t.test(pp`\na`, '\na');
  t.test(pp`\n\na`, '\n\na');
  t.test(pp`\na\n`, '\na\n');
  t.test(pp`a\n\n`, 'a\n\n');
});

g.test('substitutions,1').fn(t => {
  const act = pp`a ${3} b`;
  const exp = 'a 3 b';
  t.test(act, exp);
});

g.test('substitutions,2').fn(t => {
  const act = pp`a ${'x'}`;
  const exp = 'a x';
  t.test(act, exp);
});

g.test('substitutions,3').fn(t => {
  const act = pp`a ${'x'} b`;
  const exp = 'a x b';
  t.test(act, exp);
});

g.test('substitutions,4').fn(t => {
  const act = pp`
a
${pp._if(false)}
${'x'}
${pp._endif}
b`;
  const exp = '\na\n\nb';
  t.test(act, exp);
});

g.test('if,true').fn(t => {
  const act = pp`
a
${pp._if(true)}c${pp._endif}
d
`;
  const exp = '\na\nc\nd\n';
  t.test(act, exp);
});

g.test('if,false').fn(t => {
  const act = pp`
a
${pp._if(false)}c${pp._endif}
d
`;
  const exp = '\na\n\nd\n';
  t.test(act, exp);
});

g.test('else,1').fn(t => {
  const act = pp`
a
${pp._if(true)}
b
${pp._else}
c
${pp._endif}
d
`;
  const exp = '\na\n\nb\n\nd\n';
  t.test(act, exp);
});

g.test('else,2').fn(t => {
  const act = pp`
a
${pp._if(false)}
b
${pp._else}
c
${pp._endif}
d
`;
  const exp = '\na\n\nc\n\nd\n';
  t.test(act, exp);
});

g.test('elif,1').fn(t => {
  const act = pp`
a
${pp._if(false)}
b
${pp._elif(true)}
e
${pp._else}
c
${pp._endif}
d
`;
  const exp = '\na\n\ne\n\nd\n';
  t.test(act, exp);
});

g.test('elif,2').fn(t => {
  const act = pp`
a
${pp._if(true)}
b
${pp._elif(true)}
e
${pp._else}
c
${pp._endif}
d
`;
  const exp = '\na\n\nb\n\nd\n';
  t.test(act, exp);
});

g.test('nested,1').fn(t => {
  const act = pp`
a
${pp._if(false)}
b
${pp.__if(true)}
e
${pp.__endif}
c
${pp._endif}
d
`;
  const exp = '\na\n\nd\n';
  t.test(act, exp);
});

g.test('nested,2').fn(t => {
  const act = pp`
a
${pp._if(false)}
b
${pp._else}
h
${pp.__if(false)}
e
${pp.__elif(true)}
f
${pp.__else}
g
${pp.__endif}
c
${pp._endif}
d
`;
  const exp = '\na\n\nh\n\nf\n\nc\n\nd\n';
  t.test(act, exp);
});

g.test('errors,pass').fn(() => {
  pp`${pp._if(true)}${pp._endif}`;
  pp`${pp._if(true)}${pp._else}${pp._endif}`;
  pp`${pp._if(true)}${pp.__if(true)}${pp.__endif}${pp._endif}`;
});

g.test('errors,fail').fn(t => {
  const e = (fn: () => void) => t.shouldThrow('Error', fn);
  e(() => pp`${pp._if(true)}`);
  e(() => pp`${pp._elif(true)}`);
  e(() => pp`${pp._else}`);
  e(() => pp`${pp._endif}`);
  e(() => pp`${pp.__if(true)}`);
  e(() => pp`${pp.__elif(true)}`);
  e(() => pp`${pp.__else}`);
  e(() => pp`${pp.__endif}`);

  e(() => pp`${pp._if(true)}${pp._elif(true)}`);
  e(() => pp`${pp._if(true)}${pp._elif(true)}${pp._else}`);
  e(() => pp`${pp._if(true)}${pp._else}`);
  e(() => pp`${pp._else}${pp._endif}`);

  e(() => pp`${pp._if(true)}${pp.__endif}`);
  e(() => pp`${pp.__if(true)}${pp.__endif}`);
  e(() => pp`${pp.__if(true)}${pp._endif}`);

  e(() => pp`${pp._if(true)}${pp._else}${pp._else}${pp._endif}`);
  e(() => pp`${pp._if(true)}${pp.__if(true)}${pp.__else}${pp.__else}${pp.__endif}${pp._endif}`);
});
