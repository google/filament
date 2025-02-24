export const description = `Unit tests for check_contents`;

import { Fixture } from '../common/framework/fixture.js';
import { makeTestGroup } from '../common/internal/test_group.js';
import { ErrorWithExtra } from '../common/util/util.js';
import { checkElementsEqual } from '../webgpu/util/check_contents.js';

class F extends Fixture {
  test(substr: undefined | string, result: undefined | ErrorWithExtra) {
    if (substr === undefined) {
      this.expect(result === undefined, result?.message);
    } else {
      this.expect(result !== undefined && result.message.indexOf(substr) !== -1, result?.message);
    }
  }
}

export const g = makeTestGroup(F);

g.test('checkElementsEqual').fn(t => {
  t.shouldThrow('Error', () => checkElementsEqual(new Uint8Array(), new Uint16Array()));
  t.shouldThrow('Error', () => checkElementsEqual(new Uint32Array(), new Float32Array()));
  t.shouldThrow('Error', () => checkElementsEqual(new Uint8Array([]), new Uint8Array([0])));
  t.shouldThrow('Error', () => checkElementsEqual(new Uint8Array([0]), new Uint8Array([])));
  {
    t.test(undefined, checkElementsEqual(new Uint8Array([]), new Uint8Array([])));
    t.test(undefined, checkElementsEqual(new Uint8Array([0]), new Uint8Array([0])));
    t.test(undefined, checkElementsEqual(new Uint8Array([1]), new Uint8Array([1])));
    t.test(
      `
 Starting at index 0:
   actual == 0x: 00
   failed ->     xx
 expected ==     01`,
      checkElementsEqual(new Uint8Array([0]), new Uint8Array([1]))
    );
    t.test(
      'expected ==     01 02 01',
      checkElementsEqual(new Uint8Array([1, 1, 1]), new Uint8Array([1, 2, 1]))
    );
  }
  {
    const actual = new Uint8Array(280);
    const exp = new Uint8Array(280);
    for (let i = 2; i < 20; ++i) actual[i] = i - 4;
    t.test(
      '00 fe ff 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 00',
      checkElementsEqual(actual, exp)
    );
    for (let i = 2; i < 280; ++i) actual[i] = i - 4;
    t.test('Starting at index 1:', checkElementsEqual(actual, exp));
    for (let i = 0; i < 2; ++i) actual[i] = i - 4;
    t.test('Starting at index 0:', checkElementsEqual(actual, exp));
  }
  {
    const actual = new Int32Array(30);
    const exp = new Int32Array(30);
    for (let i = 2; i < 7; ++i) actual[i] = i - 3;
    t.test('00000002 00000003 00000000\n', checkElementsEqual(actual, exp));
    for (let i = 2; i < 30; ++i) actual[i] = i - 3;
    t.test('00000000 00000000 ...', checkElementsEqual(actual, exp));
  }
  {
    const actual = new Float64Array(30);
    const exp = new Float64Array(30);
    for (let i = 2; i < 7; ++i) actual[i] = (i - 4) * 1e100;
    t.test('2.000e+100 0.000\n', checkElementsEqual(actual, exp));
    for (let i = 2; i < 280; ++i) actual[i] = (i - 4) * 1e100;
    t.test('6.000e+100 7.000e+100 ...', checkElementsEqual(actual, exp));
  }
});
