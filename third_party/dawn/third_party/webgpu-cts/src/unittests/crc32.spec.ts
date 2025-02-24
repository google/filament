export const description = `
Test for crc32 utility functions.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { crc32, toHexString } from '../common/util/crc32.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  test(content: string, expect: string): void {
    const got = toHexString(crc32(content));
    this.expect(
      expect === got,
      `
expected: ${expect}
got:      ${got}`
    );
  }
}

export const g = makeTestGroup(F);

g.test('strings').fn(t => {
  t.test('', '00000000');
  t.test('hello world', '0d4a1185');
  t.test('123456789', 'cbf43926');
});
