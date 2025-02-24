export const description = `Unit tests for conversion`;

import { mergeParams } from '../common/internal/params_utils.js';
import { makeTestGroup } from '../common/internal/test_group.js';
import { keysOf } from '../common/util/data_tables.js';
import { assert, objectEquals } from '../common/util/util.js';
import { kValue } from '../webgpu/util/constants.js';
import {
  bool,
  concreteTypeOf,
  f16Bits,
  f32,
  f32Bits,
  float16BitsToFloat32,
  float32ToFloat16Bits,
  float32ToFloatBits,
  floatBitsToNormalULPFromZero,
  floatBitsToNumber,
  i32,
  kFloat16Format,
  kFloat32Format,
  MatrixValue,
  numbersApproximatelyEqual,
  pack2x16float,
  pack2x16snorm,
  pack2x16unorm,
  pack4x8snorm,
  pack4x8unorm,
  packRGB9E5UFloat,
  ScalarValue,
  toMatrix,
  u32,
  unpackRGB9E5UFloat,
  vec2,
  vec3,
  vec4,
  stringToType,
  Type,
  VectorValue,
} from '../webgpu/util/conversion.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

const kFloat16BitsToNumberCases = [
  [0b0_01111_0000000000, 1],
  [0b0_00001_0000000000, 0.00006103515625],
  [0b0_01101_0101010101, 0.33325195],
  [0b0_11110_1111111111, 65504],
  [0b0_00000_0000000000, 0],
  [0b1_00000_0000000000, -0.0], // -0.0 compares as equal to 0.0
  [0b0_01110_0000000000, 0.5],
  [0b0_01100_1001100110, 0.1999512],
  [0b0_01111_0000000001, 1.00097656],
  [0b0_10101_1001000000, 100],
  [0b1_01100_1001100110, -0.1999512],
  [0b1_10101_1001000000, -100],
  [0b0_11111_1111111111, Number.NaN],
  [0b0_11111_0000000000, Number.POSITIVE_INFINITY],
  [0b1_11111_0000000000, Number.NEGATIVE_INFINITY],
];

g.test('float16BitsToFloat32').fn(t => {
  for (const [bits, number] of [
    ...kFloat16BitsToNumberCases,
    [0b0_00000_1111111111, 0.00006104], // subnormal f16 input
    [0b1_00000_1111111111, -0.00006104],
  ]) {
    const actual = float16BitsToFloat32(bits);
    t.expect(
      // some loose check
      numbersApproximatelyEqual(actual, number, 0.00001),
      `for ${bits.toString(2)}, expected ${number}, got ${actual}`
    );
  }
});

g.test('float32ToFloat16Bits').fn(t => {
  for (const [bits, number] of [
    ...kFloat16BitsToNumberCases,
    [0b0_00000_0000000000, 0.00001], // input that becomes subnormal in f16 is rounded to 0
    [0b1_00000_0000000000, -0.00001], // and sign is preserved
  ]) {
    // some loose check
    const actual = float32ToFloat16Bits(number);
    t.expect(
      Math.abs(actual - bits) <= 1,
      `for ${number}, expected ${bits.toString(2)}, got ${actual.toString(2)}`
    );
  }
});

g.test('float32ToFloatBits_floatBitsToNumber')
  .paramsSubcasesOnly(u =>
    u
      .combine('signed', [0, 1] as const)
      .combine('exponentBits', [5, 8])
      .combine('mantissaBits', [10, 23])
  )
  .fn(t => {
    const { signed, exponentBits, mantissaBits } = t.params;
    const bias = (1 << (exponentBits - 1)) - 1;

    for (const [, value] of kFloat16BitsToNumberCases) {
      if (value < 0 && signed === 0) continue;
      const bits = float32ToFloatBits(value, signed, exponentBits, mantissaBits, bias);
      const reconstituted = floatBitsToNumber(bits, { signed, exponentBits, mantissaBits, bias });
      t.expect(
        numbersApproximatelyEqual(reconstituted, value, 0.0000001),
        `${reconstituted} vs ${value}`
      );
    }
  });

g.test('floatBitsToULPFromZero,16').fn(t => {
  const test = (bits: number, ulpFromZero: number) =>
    t.expect(floatBitsToNormalULPFromZero(bits, kFloat16Format) === ulpFromZero, bits.toString(2));
  // Zero
  test(0b0_00000_0000000000, 0);
  test(0b1_00000_0000000000, 0);
  // Subnormal
  test(0b0_00000_0000000001, 0);
  test(0b1_00000_0000000001, 0);
  test(0b0_00000_1111111111, 0);
  test(0b1_00000_1111111111, 0);
  // Normal
  test(0b0_00001_0000000000, 1); // 0 + 1ULP
  test(0b1_00001_0000000000, -1); // 0 - 1ULP
  test(0b0_00001_0000000001, 2); // 0 + 2ULP
  test(0b1_00001_0000000001, -2); // 0 - 2ULP
  test(0b0_01110_0000000000, 0b01101_0000000001); // 0.5
  test(0b1_01110_0000000000, -0b01101_0000000001); // -0.5
  test(0b0_01110_1111111110, 0b01101_1111111111); // 1.0 - 2ULP
  test(0b1_01110_1111111110, -0b01101_1111111111); // -(1.0 - 2ULP)
  test(0b0_01110_1111111111, 0b01110_0000000000); // 1.0 - 1ULP
  test(0b1_01110_1111111111, -0b01110_0000000000); // -(1.0 - 1ULP)
  test(0b0_01111_0000000000, 0b01110_0000000001); // 1.0
  test(0b1_01111_0000000000, -0b01110_0000000001); // -1.0
  test(0b0_01111_0000000001, 0b01110_0000000010); // 1.0 + 1ULP
  test(0b1_01111_0000000001, -0b01110_0000000010); // -(1.0 + 1ULP)
  test(0b0_10000_0000000000, 0b01111_0000000001); // 2.0
  test(0b1_10000_0000000000, -0b01111_0000000001); // -2.0

  const testThrows = (b: number) =>
    t.shouldThrow('Error', () => floatBitsToNormalULPFromZero(b, kFloat16Format));
  // Infinity
  testThrows(0b0_11111_0000000000);
  testThrows(0b1_11111_0000000000);
  // NaN
  testThrows(0b0_11111_1111111111);
  testThrows(0b1_11111_1111111111);
});

g.test('floatBitsToULPFromZero,32').fn(t => {
  const test = (bits: number, ulpFromZero: number) =>
    t.expect(floatBitsToNormalULPFromZero(bits, kFloat32Format) === ulpFromZero, bits.toString(2));
  // Zero
  test(0b0_00000000_00000000000000000000000, 0);
  test(0b1_00000000_00000000000000000000000, 0);
  // Subnormal
  test(0b0_00000000_00000000000000000000001, 0);
  test(0b1_00000000_00000000000000000000001, 0);
  test(0b0_00000000_11111111111111111111111, 0);
  test(0b1_00000000_11111111111111111111111, 0);
  // Normal
  test(0b0_00000001_00000000000000000000000, 1); // 0 + 1ULP
  test(0b1_00000001_00000000000000000000000, -1); // 0 - 1ULP
  test(0b0_00000001_00000000000000000000001, 2); // 0 + 2ULP
  test(0b1_00000001_00000000000000000000001, -2); // 0 - 2ULP
  test(0b0_01111110_00000000000000000000000, 0b01111101_00000000000000000000001); // 0.5
  test(0b1_01111110_00000000000000000000000, -0b01111101_00000000000000000000001); // -0.5
  test(0b0_01111110_11111111111111111111110, 0b01111101_11111111111111111111111); // 1.0 - 2ULP
  test(0b1_01111110_11111111111111111111110, -0b01111101_11111111111111111111111); // -(1.0 - 2ULP)
  test(0b0_01111110_11111111111111111111111, 0b01111110_00000000000000000000000); // 1.0 - 1ULP
  test(0b1_01111110_11111111111111111111111, -0b01111110_00000000000000000000000); // -(1.0 - 1ULP)
  test(0b0_01111111_00000000000000000000000, 0b01111110_00000000000000000000001); // 1.0
  test(0b1_01111111_00000000000000000000000, -0b01111110_00000000000000000000001); // -1.0
  test(0b0_01111111_00000000000000000000001, 0b01111110_00000000000000000000010); // 1.0 + 1ULP
  test(0b1_01111111_00000000000000000000001, -0b01111110_00000000000000000000010); // -(1.0 + 1ULP)
  test(0b0_11110000_00000000000000000000000, 0b11101111_00000000000000000000001); // 2.0
  test(0b1_11110000_00000000000000000000000, -0b11101111_00000000000000000000001); // -2.0

  const testThrows = (b: number) =>
    t.shouldThrow('Error', () => floatBitsToNormalULPFromZero(b, kFloat32Format));
  // Infinity
  testThrows(0b0_11111111_00000000000000000000000);
  testThrows(0b1_11111111_00000000000000000000000);
  // NaN
  testThrows(0b0_11111111_11111111111111111111111);
  testThrows(0b0_11111111_00000000000000000000001);
  testThrows(0b1_11111111_11111111111111111111111);
  testThrows(0b1_11111111_00000000000000000000001);
});

g.test('scalarWGSL').fn(t => {
  const cases: Array<[ScalarValue, string]> = [
    [f32(0.0), '0.0f'],
    // The number -0.0 can be remapped to 0.0 when stored in a Scalar
    // object. It is not possible to guarantee that '-0.0f' will
    // be emitted. So the WGSL scalar value printing does not try
    // to handle this case.
    [f32(-0.0), '0.0f'], // -0.0 can be remapped to 0.0
    [f32(1.0), '1.0f'],
    [f32(-1.0), '-1.0f'],
    [f32Bits(0x70000000), '1.5845632502852868e+29f'],
    [f32Bits(0xf0000000), '-1.5845632502852868e+29f'],
    [f16Bits(0), '0.0h'],
    [f16Bits(0x3c00), '1.0h'],
    [f16Bits(0xbc00), '-1.0h'],
    [u32(0), '0u'],
    [u32(1), '1u'],
    [u32(2000000000), '2000000000u'],
    [u32(-1), '4294967295u'],
    [i32(0), 'i32(0)'],
    [i32(1), 'i32(1)'],
    [i32(-1), 'i32(-1)'],
    [bool(true), 'true'],
    [bool(false), 'false'],
  ];
  for (const [value, expect] of cases) {
    const got = value.wgsl();
    t.expect(
      got === expect,
      `[value: ${value.value}, type: ${value.type}]
got:    ${got}
expect: ${expect}`
    );
  }
});

g.test('vectorWGSL').fn(t => {
  const cases: Array<[VectorValue, string]> = [
    [vec2(f32(42.0), f32(24.0)), 'vec2(42.0f, 24.0f)'],
    [vec2(f16Bits(0x5140), f16Bits(0x4e00)), 'vec2(42.0h, 24.0h)'],
    [vec2(u32(42), u32(24)), 'vec2(42u, 24u)'],
    [vec2(i32(42), i32(24)), 'vec2(i32(42), i32(24))'],
    [vec2(bool(false), bool(true)), 'vec2(false, true)'],

    [vec3(f32(0.0), f32(1.0), f32(-1.0)), 'vec3(0.0f, 1.0f, -1.0f)'],
    [vec3(f16Bits(0), f16Bits(0x3c00), f16Bits(0xbc00)), 'vec3(0.0h, 1.0h, -1.0h)'],
    [vec3(u32(0), u32(1), u32(-1)), 'vec3(0u, 1u, 4294967295u)'],
    [vec3(i32(0), i32(1), i32(-1)), 'vec3(i32(0), i32(1), i32(-1))'],
    [vec3(bool(true), bool(false), bool(true)), 'vec3(true, false, true)'],

    [vec4(f32(1.0), f32(-2.0), f32(4.0), f32(-8.0)), 'vec4(1.0f, -2.0f, 4.0f, -8.0f)'],
    [
      vec4(f16Bits(0xbc00), f16Bits(0x4000), f16Bits(0xc400), f16Bits(0x4800)),
      'vec4(-1.0h, 2.0h, -4.0h, 8.0h)',
    ],
    [vec4(u32(1), u32(-2), u32(4), u32(-8)), 'vec4(1u, 4294967294u, 4u, 4294967288u)'],
    [vec4(i32(1), i32(-2), i32(4), i32(-8)), 'vec4(i32(1), i32(-2), i32(4), i32(-8))'],
    [vec4(bool(false), bool(true), bool(true), bool(false)), 'vec4(false, true, true, false)'],
  ];
  for (const [value, expect] of cases) {
    const got = value.wgsl();
    t.expect(
      got === expect,
      `[values: ${value.elements}, type: ${value.type}]
got:    ${got}
expect: ${expect}`
    );
  }
});

g.test('matrixWGSL').fn(t => {
  const cases: Array<[MatrixValue, string]> = [
    [
      toMatrix(
        [
          [0.0, 1.0],
          [2.0, 3.0],
        ],
        f32
      ),
      'mat2x2(0.0f, 1.0f, 2.0f, 3.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0],
          [3.0, 4.0, 5.0],
        ],
        f32
      ),
      'mat2x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0, 7.0],
        ],
        f32
      ),
      'mat2x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0],
          [2.0, 3.0],
          [4.0, 5.0],
        ],
        f32
      ),
      'mat3x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0],
          [3.0, 4.0, 5.0],
          [6.0, 7.0, 8.0],
        ],
        f32
      ),
      'mat3x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0, 7.0],
          [8.0, 9.0, 10.0, 11.0],
        ],
        f32
      ),
      'mat3x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0],
          [2.0, 3.0],
          [4.0, 5.0],
          [6.0, 7.0],
        ],
        f32
      ),
      'mat4x2(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0],
          [3.0, 4.0, 5.0],
          [6.0, 7.0, 8.0],
          [9.0, 10.0, 11.0],
        ],
        f32
      ),
      'mat4x3(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f)',
    ],
    [
      toMatrix(
        [
          [0.0, 1.0, 2.0, 3.0],
          [4.0, 5.0, 6.0, 7.0],
          [8.0, 9.0, 10.0, 11.0],
          [12.0, 13.0, 14.0, 15.0],
        ],
        f32
      ),
      'mat4x4(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f)',
    ],
  ];
  for (const [value, expect] of cases) {
    const got = value.wgsl();
    t.expect(
      got === expect,
      `[values: ${value.elements}, type: ${value.type}]
got:    ${got}
expect: ${expect}`
    );
  }
});

g.test('constructorMatrix')
  .params(u =>
    u
      .combine('cols', [2, 3, 4] as const)
      .combine('rows', [2, 3, 4] as const)
      .combine('type', ['f32'] as const)
  )
  .fn(t => {
    const cols = t.params.cols;
    const rows = t.params.rows;
    const type = t.params.type;
    const scalar_builder = type === 'f32' ? f32 : undefined;
    assert(scalar_builder !== undefined, `Unexpected type param '${type}' provided`);

    const elements = [...Array(cols).keys()].map(c => {
      return [...Array(rows).keys()].map(r => scalar_builder(c * cols + r));
    });

    const got = new MatrixValue(elements);
    const got_type = got.type;
    t.expect(
      got_type.cols === cols,
      `expected Matrix to have ${cols} columns, received ${got_type.cols} instead`
    );
    t.expect(
      got_type.rows === rows,
      `expected Matrix to have ${rows} columns, received ${got_type.rows} instead`
    );
    t.expect(
      got_type.elementType.kind === type,
      `expected Matrix to have ${type} elements, received ${got_type.elementType.kind} instead`
    );
    t.expect(
      objectEquals(got.elements, elements),
      `Matrix did not have expected elements (${JSON.stringify(elements)}), instead had (${
        got.elements
      })`
    );
  });

g.test('pack2x16float')
  .paramsSimple([
    // f16 normals
    { inputs: [0, 0], result: [0x00000000, 0x80000000, 0x00008000, 0x80008000] },
    { inputs: [1, 0], result: [0x00003c00, 0x80003c00] },
    { inputs: [1, 1], result: [0x3c003c00] },
    { inputs: [-1, -1], result: [0xbc00bc00] },
    { inputs: [10, 1], result: [0x3c004900] },
    { inputs: [-10, 1], result: [0x3c00c900] },

    // f32 normal, but not f16 precise
    { inputs: [1.00000011920928955078125, 1], result: [0x3c003c00, 0x3c003c01] },

    // f32 subnormals
    // prettier-ignore
    { inputs: [kValue.f32.positive.subnormal.max, 1], result: [0x3c000000, 0x3c008000, 0x3c000001] },
    // prettier-ignore
    { inputs: [kValue.f32.negative.subnormal.min, 1], result: [0x3c008001, 0x3c000000, 0x3c008000] },

    // f16 subnormals
    // prettier-ignore
    { inputs: [kValue.f16.positive.subnormal.max, 1], result: [0x3c0003ff, 0x3c000000, 0x3c008000] },
    // prettier-ignore
    { inputs: [kValue.f16.negative.subnormal.min, 1], result: [0x03c0083ff, 0x3c000000, 0x3c008000] },

    // f16 out of bounds
    { inputs: [kValue.f16.positive.max + 1, 1], result: [undefined] },
    { inputs: [kValue.f16.negative.min - 1, 1], result: [undefined] },
    { inputs: [1, kValue.f16.positive.max + 1], result: [undefined] },
    { inputs: [1, kValue.f16.negative.min - 1], result: [undefined] },
  ] as const)
  .fn(test => {
    const toString = (data: readonly (undefined | number)[]): String[] => {
      return data.map(d => (d !== undefined ? u32(d).toString() : 'undefined'));
    };

    const inputs = test.params.inputs;
    const got = pack2x16float(inputs[0], inputs[1]);
    const expect = test.params.result;

    const got_str = toString(got);
    const expect_str = toString(expect);

    // Using strings of the outputs, so they can be easily sorted, since order of the results doesn't matter.
    test.expect(
      objectEquals(got_str.sort(), expect_str.sort()),
      `pack2x16float(${inputs}) returned [${got_str}]. Expected [${expect_str}]`
    );
  });

g.test('pack2x16snorm')
  .paramsSimple([
    // Normals
    { inputs: [0, 0], result: 0x00000000 },
    { inputs: [1, 0], result: 0x00007fff },
    { inputs: [0, 1], result: 0x7fff0000 },
    { inputs: [1, 1], result: 0x7fff7fff },
    { inputs: [-1, -1], result: 0x80018001 },
    { inputs: [10, 10], result: 0x7fff7fff },
    { inputs: [-10, -10], result: 0x80018001 },
    { inputs: [0.1, 0.1], result: 0x0ccd0ccd },
    { inputs: [-0.1, -0.1], result: 0xf333f333 },
    { inputs: [0.5, 0.5], result: 0x40004000 },
    { inputs: [-0.5, -0.5], result: 0xc001c001 },
    { inputs: [0.1, 0.5], result: 0x40000ccd },
    { inputs: [-0.1, -0.5], result: 0xc001f333 },

    // Subnormals
    { inputs: [kValue.f32.positive.subnormal.max, 1], result: 0x7fff0000 },
    { inputs: [kValue.f32.negative.subnormal.min, 1], result: 0x7fff0000 },
  ] as const)
  .fn(test => {
    const inputs = test.params.inputs;
    const got = pack2x16snorm(inputs[0], inputs[1]);
    const expect = test.params.result;

    test.expect(got === expect, `pack2x16snorm(${inputs}) returned ${got}. Expected ${expect}`);
  });

g.test('pack2x16unorm')
  .paramsSimple([
    // Normals
    { inputs: [0, 0], result: 0x00000000 },
    { inputs: [1, 0], result: 0x0000ffff },
    { inputs: [0, 1], result: 0xffff0000 },
    { inputs: [1, 1], result: 0xffffffff },
    { inputs: [-1, -1], result: 0x00000000 },
    { inputs: [0.1, 0.1], result: 0x199a199a },
    { inputs: [0.5, 0.5], result: 0x80008000 },
    { inputs: [0.1, 0.5], result: 0x8000199a },
    { inputs: [10, 10], result: 0xffffffff },

    // Subnormals
    { inputs: [kValue.f32.positive.subnormal.max, 1], result: 0xffff0000 },
  ] as const)
  .fn(test => {
    const inputs = test.params.inputs;
    const got = pack2x16unorm(inputs[0], inputs[1]);
    const expect = test.params.result;

    test.expect(got === expect, `pack2x16unorm(${inputs}) returned ${got}. Expected ${expect}`);
  });

g.test('pack4x8snorm')
  .paramsSimple([
    // Normals
    { inputs: [0, 0, 0, 0], result: 0x00000000 },
    { inputs: [1, 0, 0, 0], result: 0x0000007f },
    { inputs: [0, 1, 0, 0], result: 0x00007f00 },
    { inputs: [0, 0, 1, 0], result: 0x007f0000 },
    { inputs: [0, 0, 0, 1], result: 0x7f000000 },
    { inputs: [1, 1, 1, 1], result: 0x7f7f7f7f },
    { inputs: [10, 10, 10, 10], result: 0x7f7f7f7f },
    { inputs: [-1, 0, 0, 0], result: 0x00000081 },
    { inputs: [0, -1, 0, 0], result: 0x00008100 },
    { inputs: [0, 0, -1, 0], result: 0x00810000 },
    { inputs: [0, 0, 0, -1], result: 0x81000000 },
    { inputs: [-1, -1, -1, -1], result: 0x81818181 },
    { inputs: [-10, -10, -10, -10], result: 0x81818181 },
    { inputs: [0.1, 0.1, 0.1, 0.1], result: 0x0d0d0d0d },
    { inputs: [-0.1, -0.1, -0.1, -0.1], result: 0xf3f3f3f3 },
    { inputs: [0.1, -0.1, 0.1, -0.1], result: 0xf30df30d },
    { inputs: [0.5, 0.5, 0.5, 0.5], result: 0x40404040 },
    { inputs: [-0.5, -0.5, -0.5, -0.5], result: 0xc1c1c1c1 },
    { inputs: [-0.5, 0.5, -0.5, 0.5], result: 0x40c140c1 },
    { inputs: [0.1, 0.5, 0.1, 0.5], result: 0x400d400d },
    { inputs: [-0.1, -0.5, -0.1, -0.5], result: 0xc1f3c1f3 },

    // Subnormals
    { inputs: [kValue.f32.positive.subnormal.max, 1, 1, 1], result: 0x7f7f7f00 },
    { inputs: [kValue.f32.negative.subnormal.min, 1, 1, 1], result: 0x7f7f7f00 },
  ] as const)
  .fn(test => {
    const inputs = test.params.inputs;
    const got = pack4x8snorm(inputs[0], inputs[1], inputs[2], inputs[3]);
    const expect = test.params.result;

    test.expect(got === expect, `pack4x8snorm(${inputs}) returned ${u32(got)}. Expected ${expect}`);
  });

g.test('pack4x8unorm')
  .paramsSimple([
    // Normals
    { inputs: [0, 0, 0, 0], result: 0x00000000 },
    { inputs: [1, 0, 0, 0], result: 0x000000ff },
    { inputs: [0, 1, 0, 0], result: 0x0000ff00 },
    { inputs: [0, 0, 1, 0], result: 0x00ff0000 },
    { inputs: [0, 0, 0, 1], result: 0xff000000 },
    { inputs: [1, 1, 1, 1], result: 0xffffffff },
    { inputs: [10, 10, 10, 10], result: 0xffffffff },
    { inputs: [-1, -1, -1, -1], result: 0x00000000 },
    { inputs: [-10, -10, -10, -10], result: 0x00000000 },
    { inputs: [0.1, 0.1, 0.1, 0.1], result: 0x1a1a1a1a },
    { inputs: [0.5, 0.5, 0.5, 0.5], result: 0x80808080 },
    { inputs: [0.1, 0.5, 0.1, 0.5], result: 0x801a801a },

    // Subnormals
    { inputs: [kValue.f32.positive.subnormal.max, 1, 1, 1], result: 0xffffff00 },
  ] as const)
  .fn(test => {
    const inputs = test.params.inputs;
    const got = pack4x8unorm(inputs[0], inputs[1], inputs[2], inputs[3]);
    const expect = test.params.result;

    test.expect(got === expect, `pack4x8unorm(${inputs}) returned ${got}. Expected ${expect}`);
  });

const kRGB9E5UFloatCommonData = {
  zero: /*        */ { encoded: 0b00000_000000000_000000000_000000000, rgb: [0, 0, 0] },
  max: /*         */ { encoded: 0b11111_111111111_111111111_111111111, rgb: [65408, 65408, 65408] },
  r1: /*          */ { encoded: 0b10000_000000000_000000000_100000000, rgb: [1, 0, 0] },
  r2: /*          */ { encoded: 0b10001_000000000_000000000_100000000, rgb: [2, 0, 0] },
  g1: /*          */ { encoded: 0b10000_000000000_100000000_000000000, rgb: [0, 1, 0] },
  g2: /*          */ { encoded: 0b10001_000000000_100000000_000000000, rgb: [0, 2, 0] },
  b1: /*          */ { encoded: 0b10000_100000000_000000000_000000000, rgb: [0, 0, 1] },
  b2: /*          */ { encoded: 0b10001_100000000_000000000_000000000, rgb: [0, 0, 2] },
  r1_g1_b1: /*    */ { encoded: 0b10000_100000000_100000000_100000000, rgb: [1, 1, 1] },
  r1_g2_b1: /*    */ { encoded: 0b10001_010000000_100000000_010000000, rgb: [1, 2, 1] },
  r4_g8_b2: /*    */ { encoded: 0b10011_001000000_100000000_010000000, rgb: [4, 8, 2] },
  r1_g2_b3: /*    */ { encoded: 0b10001_110000000_100000000_010000000, rgb: [1, 2, 3] },
  r128_g3968_b65408: { encoded: 0b11111_111111111_000011111_000000001, rgb: [128, 3968, 65408] },
  r128_g1984_b30016: { encoded: 0b11110_111010101_000011111_000000010, rgb: [128, 1984, 30016] },
  r_5_g_25_b_8: /**/ { encoded: 0b10011_100000000_000001000_000010000, rgb: [0.5, 0.25, 8] },
};

const kPackRGB9E5UFloatData = mergeParams(kRGB9E5UFloatCommonData, {
  clamp_max: /*   */ { encoded: 0b11111_111111111_111111111_111111111, rgb: [1e7, 1e10, 1e50] },
  subnormals: /*  */ { encoded: 0b00000_000000000_000000000_000000000, rgb: [1e-10, 1e-20, 1e-30] },
  r57423_g54_b3478: { encoded: 0b11111_000011011_000000000_111000001, rgb: [57423, 54, 3478] },
  r6852_g3571_b2356: { encoded: 0b11100_010010011_011011111_110101100, rgb: [6852, 3571, 2356] },
  r68312_g12_b8123: { encoded: 0b11111_000111111_000000000_111111111, rgb: [68312, 12, 8123] },
  r7321_g846_b32: { encoded: 0b11100_000000010_000110101_111001010, rgb: [7321, 846, 32] },
});

function bits5_9_9_9(x: number) {
  const s = (x >>> 0).toString(2).padStart(32, '0');
  return `${s.slice(0, 5)}_${s.slice(5, 14)}_${s.slice(14, 23)}_${s.slice(23, 32)}`;
}

g.test('packRGB9E5UFloat')
  .params(u => u.combine('case', keysOf(kPackRGB9E5UFloatData)))
  .fn(test => {
    const c = kPackRGB9E5UFloatData[test.params.case];
    const got = packRGB9E5UFloat(c.rgb[0], c.rgb[1], c.rgb[2]);
    const expect = c.encoded;

    test.expect(
      got === expect,
      `packRGB9E5UFloat(${c.rgb}) returned ${bits5_9_9_9(got)}. Expected ${bits5_9_9_9(expect)}`
    );
  });

g.test('unpackRGB9E5UFloat')
  .params(u => u.combine('case', keysOf(kRGB9E5UFloatCommonData)))
  .fn(test => {
    const c = kRGB9E5UFloatCommonData[test.params.case];
    const got = unpackRGB9E5UFloat(c.encoded);
    const expect = c.rgb;

    test.expect(
      got.R === expect[0] && got.G === expect[1] && got.B === expect[2],
      `unpackRGB9E5UFloat(${bits5_9_9_9(c.encoded)} ` +
        `returned ${got.R},${got.G},${got.B}. Expected ${expect}`
    );
  });

const kConcreteTypeOfNoAllowedListCases = {
  bool: Type.bool,
  i32: Type.i32,
  u32: Type.u32,
  f32: Type.f32,
  f16: Type.f16,
  abstractInt: Type.i32,
  abstractFloat: Type.f32,
  vec2b: Type.vec2b,
  vec2i: Type.vec2i,
  vec3u: Type.vec3u,
  vec4f: Type.vec4f,
  vec2h: Type.vec2h,
  vec3ai: Type.vec3i,
  vec4af: Type.vec4f,
  mat2x2f: Type.mat2x2f,
  mat3x4h: Type.mat3x4h,
} as const;

g.test('concreteTypeOf_noAllowedLiist')
  .params(u => u.combine('src', keysOf(kConcreteTypeOfNoAllowedListCases)))
  .fn(test => {
    const src = test.params.src;
    const dest = kConcreteTypeOfNoAllowedListCases[src];
    const got = concreteTypeOf(stringToType(src));
    test.expect(got === dest);
  });

const kConcreteTypeOfAllowListCases = {
  af_f32: { type: Type.abstractFloat, allowed: [Type.f32], expect: Type.f32 },
  af_f16: { type: Type.abstractFloat, allowed: [Type.f16], expect: Type.f16 },
  af_all: {
    type: Type.abstractFloat,
    allowed: [Type.f16, Type.f32, Type.i32, Type.u32],
    expect: Type.f32,
  },
  ai_i32: { type: Type.abstractInt, allowed: [Type.i32], expect: Type.i32 },
  ai_u32: { type: Type.abstractInt, allowed: [Type.u32], expect: Type.u32 },
  ai_f32: { type: Type.abstractInt, allowed: [Type.f32], expect: Type.f32 },
  ai_f16: { type: Type.abstractInt, allowed: [Type.f16], expect: Type.f16 },
  ai_all: {
    type: Type.abstractInt,
    allowed: [Type.f16, Type.f32, Type.i32, Type.u32],
    expect: Type.i32,
  },
} as const;

g.test('concreteTypeOf_AllowedLiist')
  .params(u => u.combine('k', keysOf(kConcreteTypeOfAllowListCases)))
  .fn(test => {
    const { type, allowed, expect } = kConcreteTypeOfAllowListCases[test.params.k];
    const got = concreteTypeOf(type, allowed as [Type]);
    test.expect(got === expect);
  });
