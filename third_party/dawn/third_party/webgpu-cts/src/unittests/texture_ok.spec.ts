export const description = `
Test for texture_ok utils.
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { typedArrayFromParam, typedArrayParam } from '../common/util/util.js';
import { TexelView } from '../webgpu/util/texture/texel_view.js';
import { findFailedPixels } from '../webgpu/util/texture/texture_ok.js';

import { UnitTest } from './unit_test.js';

class F extends UnitTest {
  test(act: string, exp: string): void {
    this.expect(act === exp, 'got: ' + act.replace('\n', '⏎'));
  }
}

export const g = makeTestGroup(F);
g.test('findFailedPixels')
  .desc(
    `
    Test findFailedPixels passes what is expected to pass and fails what is expected
    to fail. For example NaN === NaN should be true in a texture that allows NaN.
    2 different representations of the same rgb9e5ufloat should compare as equal.
    etc...
  `
  )
  .params(u =>
    u.combineWithParams([
      // Sanity Check
      {
        format: 'rgba8unorm',
        actual: typedArrayParam('Uint8Array', [0x00, 0x40, 0x80, 0xff]),
        expected: typedArrayParam('Uint8Array', [0x00, 0x40, 0x80, 0xff]),
        isSame: true,
      },
      // Slightly different values
      {
        format: 'rgba8unorm',
        actual: typedArrayParam('Uint8Array', [0x00, 0x40, 0x80, 0xff]),
        expected: typedArrayParam('Uint8Array', [0x00, 0x40, 0x81, 0xff]),
        isSame: false,
      },
      // Different representations of the same value
      {
        format: 'rgb9e5ufloat',
        actual: typedArrayParam('Uint8Array', [0x78, 0x56, 0x34, 0x12]),
        expected: typedArrayParam('Uint8Array', [0xf0, 0xac, 0x68, 0x0c]),
        isSame: true,
      },
      // Slightly different values
      {
        format: 'rgb9e5ufloat',
        actual: typedArrayParam('Uint8Array', [0x78, 0x56, 0x34, 0x12]),
        expected: typedArrayParam('Uint8Array', [0xf1, 0xac, 0x68, 0x0c]),
        isSame: false,
      },
      // Test NaN === NaN
      {
        format: 'r32float',
        actual: typedArrayParam('Float32Array', [parseFloat('abc')]),
        expected: typedArrayParam('Float32Array', [parseFloat('def')]),
        isSame: true,
      },
      // Sanity Check
      {
        format: 'r32float',
        actual: typedArrayParam('Float32Array', [1.23]),
        expected: typedArrayParam('Float32Array', [1.23]),
        isSame: true,
      },
      // Slightly different values.
      {
        format: 'r32float',
        actual: typedArrayParam('Uint32Array', [0x3f9d70a4]),
        expected: typedArrayParam('Uint32Array', [0x3f9d70a5]),
        isSame: false,
      },
      // Slightly different
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0x3ce]),
        expected: typedArrayParam('Uint32Array', [0x3cf]),
        isSame: false,
      },
      // Positive.Infinity === Positive.Infinity (red)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b11111000000]),
        expected: typedArrayParam('Uint32Array', [0b11111000000]),
        isSame: true,
      },
      // Positive.Infinity === Positive.Infinity (green)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b11111000000_00000000000]),
        expected: typedArrayParam('Uint32Array', [0b11111000000_00000000000]),
        isSame: true,
      },
      // Positive.Infinity === Positive.Infinity (blue)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b1111100000_00000000000_00000000000]),
        expected: typedArrayParam('Uint32Array', [0b1111100000_00000000000_00000000000]),
        isSame: true,
      },
      // NaN === NaN (red)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b11111000001]),
        expected: typedArrayParam('Uint32Array', [0b11111000010]),
        isSame: true,
      },
      // NaN === NaN (green)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b11111000100_00000000000]),
        expected: typedArrayParam('Uint32Array', [0b11111001000_00000000000]),
        isSame: true,
      },
      // NaN === NaN (blue)
      {
        format: 'rg11b10ufloat',
        actual: typedArrayParam('Uint32Array', [0b1111110000_00000000000_00000000000]),
        expected: typedArrayParam('Uint32Array', [0b1111101000_00000000000_00000000000]),
        isSame: true,
      },
    ] as const)
  )
  .fn(t => {
    const { format, actual, expected, isSame } = t.params;
    const actualData = new Uint8Array(typedArrayFromParam(actual).buffer);
    const expectedData = new Uint8Array(typedArrayFromParam(expected).buffer);

    const actTexelView = TexelView.fromTextureDataByReference(format, actualData, {
      bytesPerRow: actualData.byteLength,
      rowsPerImage: 1,
      subrectOrigin: [0, 0, 0],
      subrectSize: [1, 1, 1],
    });
    const expTexelView = TexelView.fromTextureDataByReference(format, expectedData, {
      bytesPerRow: expectedData.byteLength,
      rowsPerImage: 1,
      subrectOrigin: [0, 0, 0],
      subrectSize: [1, 1, 1],
    });

    const zero = { x: 0, y: 0, z: 0 };
    const failedPixelsMessage = findFailedPixels(
      format,
      zero,
      { width: 1, height: 1, depthOrArrayLayers: 1 },
      { actTexelView, expTexelView },
      {
        maxFractionalDiff: 0,
      }
    );

    t.expect(isSame === !failedPixelsMessage, failedPixelsMessage);
  });
