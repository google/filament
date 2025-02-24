export const description = `Unit tests for data cache serialization`;

import { getIsBuildingDataCache, setIsBuildingDataCache } from '../common/framework/data_cache.js';
import { makeTestGroup } from '../common/internal/test_group.js';
import { objectEquals } from '../common/util/util.js';
import {
  deserializeExpectation,
  serializeExpectation,
} from '../webgpu/shader/execution/expression/case_cache.js';
import BinaryStream from '../webgpu/util/binary_stream.js';
import {
  anyOf,
  deserializeComparator,
  serializeComparator,
  skipUndefined,
} from '../webgpu/util/compare.js';
import { kValue } from '../webgpu/util/constants.js';
import {
  abstractFloat,
  abstractInt,
  bool,
  deserializeValue,
  f16,
  f32,
  i16,
  i32,
  i8,
  serializeValue,
  toMatrix,
  u16,
  u32,
  u8,
  vec2,
  vec3,
  vec4,
} from '../webgpu/util/conversion.js';
import { deserializeFPInterval, FP, serializeFPInterval } from '../webgpu/util/floating_point.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

g.test('value').fn(t => {
  for (const value of [
    u32(kValue.u32.min + 0),
    u32(kValue.u32.min + 1),
    u32(kValue.u32.min + 2),
    u32(kValue.u32.max - 2),
    u32(kValue.u32.max - 1),
    u32(kValue.u32.max - 0),

    u16(kValue.u16.min + 0),
    u16(kValue.u16.min + 1),
    u16(kValue.u16.min + 2),
    u16(kValue.u16.max - 2),
    u16(kValue.u16.max - 1),
    u16(kValue.u16.max - 0),

    u8(kValue.u8.min + 0),
    u8(kValue.u8.min + 1),
    u8(kValue.u8.min + 2),
    u8(kValue.u8.max - 2),
    u8(kValue.u8.max - 1),
    u8(kValue.u8.max - 0),

    abstractInt(kValue.i64.negative.min),
    abstractInt(kValue.i64.negative.min + 1n),
    abstractInt(kValue.i64.negative.min + 2n),
    abstractInt(kValue.i64.negative.max - 2n),
    abstractInt(kValue.i64.negative.max - 1n),
    abstractInt(kValue.i64.positive.min),
    abstractInt(kValue.i64.positive.min + 1n),
    abstractInt(kValue.i64.positive.min + 2n),
    abstractInt(kValue.i64.positive.max - 2n),
    abstractInt(kValue.i64.positive.max - 1n),
    abstractInt(kValue.i64.positive.max),

    i32(kValue.i32.negative.min + 0),
    i32(kValue.i32.negative.min + 1),
    i32(kValue.i32.negative.min + 2),
    i32(kValue.i32.negative.max - 2),
    i32(kValue.i32.negative.max - 1),
    i32(kValue.i32.positive.min - 0),
    i32(kValue.i32.positive.min + 1),
    i32(kValue.i32.positive.min + 2),
    i32(kValue.i32.positive.max - 2),
    i32(kValue.i32.positive.max - 1),
    i32(kValue.i32.positive.max - 0),

    i16(kValue.i16.negative.min + 0),
    i16(kValue.i16.negative.min + 1),
    i16(kValue.i16.negative.min + 2),
    i16(kValue.i16.negative.max - 2),
    i16(kValue.i16.negative.max - 1),
    i16(kValue.i16.positive.min + 0),
    i16(kValue.i16.positive.min + 1),
    i16(kValue.i16.positive.min + 2),
    i16(kValue.i16.positive.max - 2),
    i16(kValue.i16.positive.max - 1),
    i16(kValue.i16.positive.max - 0),

    i8(kValue.i8.negative.min + 0),
    i8(kValue.i8.negative.min + 1),
    i8(kValue.i8.negative.min + 2),
    i8(kValue.i8.negative.max - 2),
    i8(kValue.i8.negative.max - 1),
    i8(kValue.i8.positive.min + 0),
    i8(kValue.i8.positive.min + 1),
    i8(kValue.i8.positive.min + 2),
    i8(kValue.i8.positive.max - 2),
    i8(kValue.i8.positive.max - 1),
    i8(kValue.i8.positive.max - 0),

    abstractFloat(0),
    abstractFloat(-0),
    abstractFloat(1),
    abstractFloat(-1),
    abstractFloat(0.5),
    abstractFloat(-0.5),
    abstractFloat(kValue.f64.positive.max),
    abstractFloat(kValue.f64.positive.min),
    abstractFloat(kValue.f64.positive.subnormal.max),
    abstractFloat(kValue.f64.positive.subnormal.min),
    abstractFloat(kValue.f64.negative.subnormal.max),
    abstractFloat(kValue.f64.negative.subnormal.min),
    abstractFloat(kValue.f64.positive.infinity),
    abstractFloat(kValue.f64.negative.infinity),

    f32(0),
    f32(-0),
    f32(1),
    f32(-1),
    f32(0.5),
    f32(-0.5),
    f32(kValue.f32.positive.max),
    f32(kValue.f32.positive.min),
    f32(kValue.f32.positive.subnormal.max),
    f32(kValue.f32.positive.subnormal.min),
    f32(kValue.f32.negative.subnormal.max),
    f32(kValue.f32.negative.subnormal.min),
    f32(kValue.f32.positive.infinity),
    f32(kValue.f32.negative.infinity),

    f16(0),
    f16(-0),
    f16(1),
    f16(-1),
    f16(0.5),
    f16(-0.5),
    f16(kValue.f16.positive.max),
    f16(kValue.f16.positive.min),
    f16(kValue.f16.positive.subnormal.max),
    f16(kValue.f16.positive.subnormal.min),
    f16(kValue.f16.negative.subnormal.max),
    f16(kValue.f16.negative.subnormal.min),
    f16(kValue.f16.positive.infinity),
    f16(kValue.f16.negative.infinity),

    bool(true),
    bool(false),

    vec2(f32(1), f32(2)),
    vec3(u32(1), u32(2), u32(3)),
    vec4(bool(false), bool(true), bool(false), bool(true)),

    toMatrix(
      [
        [0.0, 1.0],
        [2.0, 3.0],
      ],
      abstractFloat
    ),
    toMatrix(
      [
        [0.0, 1.0],
        [2.0, 3.0],
      ],
      f32
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0],
        [3.0, 4.0, 5.0],
      ],
      f16
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0, 3.0],
        [4.0, 5.0, 6.0, 7.0],
      ],
      abstractFloat
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0, 3.0],
        [4.0, 5.0, 6.0, 7.0],
      ],
      f32
    ),
    toMatrix(
      [
        [0.0, 1.0],
        [2.0, 3.0],
        [4.0, 5.0],
      ],
      f16
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0],
        [3.0, 4.0, 5.0],
        [6.0, 7.0, 8.0],
      ],
      abstractFloat
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0],
        [3.0, 4.0, 5.0],
        [6.0, 7.0, 8.0],
      ],
      f32
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0, 3.0],
        [4.0, 5.0, 6.0, 7.0],
        [8.0, 9.0, 10.0, 11.0],
      ],
      f16
    ),
    toMatrix(
      [
        [0.0, 1.0],
        [2.0, 3.0],
        [4.0, 5.0],
        [6.0, 7.0],
      ],
      abstractFloat
    ),
    toMatrix(
      [
        [0.0, 1.0],
        [2.0, 3.0],
        [4.0, 5.0],
        [6.0, 7.0],
      ],
      f32
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0],
        [3.0, 4.0, 5.0],
        [6.0, 7.0, 8.0],
        [9.0, 10.0, 11.0],
      ],
      f16
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0, 3.0],
        [4.0, 5.0, 6.0, 7.0],
        [8.0, 9.0, 10.0, 11.0],
        [12.0, 13.0, 14.0, 15.0],
      ],
      abstractFloat
    ),
    toMatrix(
      [
        [0.0, 1.0, 2.0, 3.0],
        [4.0, 5.0, 6.0, 7.0],
        [8.0, 9.0, 10.0, 11.0],
        [12.0, 13.0, 14.0, 15.0],
      ],
      f32
    ),
  ]) {
    const s = new BinaryStream(new Uint8Array(1024).buffer);
    serializeValue(s, value);
    const d = new BinaryStream(s.buffer().buffer);
    const deserialized = deserializeValue(d);
    t.expect(
      objectEquals(value, deserialized),
      `${value.type} ${value} -> serialize -> deserialize -> ${deserialized}
buffer: ${s.buffer()}`
    );
  }
});

g.test('fpinterval_f32').fn(t => {
  for (const interval of [
    FP.f32.toInterval(0),
    FP.f32.toInterval(-0),
    FP.f32.toInterval(1),
    FP.f32.toInterval(-1),
    FP.f32.toInterval(0.5),
    FP.f32.toInterval(-0.5),
    FP.f32.toInterval(kValue.f32.positive.max),
    FP.f32.toInterval(kValue.f32.positive.min),
    FP.f32.toInterval(kValue.f32.positive.subnormal.max),
    FP.f32.toInterval(kValue.f32.positive.subnormal.min),
    FP.f32.toInterval(kValue.f32.negative.subnormal.max),
    FP.f32.toInterval(kValue.f32.negative.subnormal.min),
    FP.f32.toInterval(kValue.f32.positive.infinity),
    FP.f32.toInterval(kValue.f32.negative.infinity),

    FP.f32.toInterval([-0, 0]),
    FP.f32.toInterval([-1, 1]),
    FP.f32.toInterval([-0.5, 0.5]),
    FP.f32.toInterval([kValue.f32.positive.min, kValue.f32.positive.max]),
    FP.f32.toInterval([kValue.f32.positive.subnormal.min, kValue.f32.positive.subnormal.max]),
    FP.f32.toInterval([kValue.f32.negative.subnormal.min, kValue.f32.negative.subnormal.max]),
    FP.f32.toInterval([kValue.f32.negative.infinity, kValue.f32.positive.infinity]),
  ]) {
    const s = new BinaryStream(new Uint8Array(1024).buffer);
    serializeFPInterval(s, interval);
    const d = new BinaryStream(s.buffer().buffer);
    const deserialized = deserializeFPInterval(d);
    t.expect(
      objectEquals(interval, deserialized),
      `interval ${interval} -> serialize -> deserialize -> ${deserialized}`
    );
  }
});

g.test('fpinterval_f16').fn(t => {
  for (const interval of [
    FP.f16.toInterval(0),
    FP.f16.toInterval(-0),
    FP.f16.toInterval(1),
    FP.f16.toInterval(-1),
    FP.f16.toInterval(0.5),
    FP.f16.toInterval(-0.5),
    FP.f16.toInterval(kValue.f16.positive.max),
    FP.f16.toInterval(kValue.f16.positive.min),
    FP.f16.toInterval(kValue.f16.positive.subnormal.max),
    FP.f16.toInterval(kValue.f16.positive.subnormal.min),
    FP.f16.toInterval(kValue.f16.negative.subnormal.max),
    FP.f16.toInterval(kValue.f16.negative.subnormal.min),
    FP.f16.toInterval(kValue.f16.positive.infinity),
    FP.f16.toInterval(kValue.f16.negative.infinity),

    FP.f16.toInterval([-0, 0]),
    FP.f16.toInterval([-1, 1]),
    FP.f16.toInterval([-0.5, 0.5]),
    FP.f16.toInterval([kValue.f16.positive.min, kValue.f16.positive.max]),
    FP.f16.toInterval([kValue.f16.positive.subnormal.min, kValue.f16.positive.subnormal.max]),
    FP.f16.toInterval([kValue.f16.negative.subnormal.min, kValue.f16.negative.subnormal.max]),
    FP.f16.toInterval([kValue.f16.negative.infinity, kValue.f16.positive.infinity]),
  ]) {
    const s = new BinaryStream(new Uint8Array(1024).buffer);
    serializeFPInterval(s, interval);
    const d = new BinaryStream(s.buffer().buffer);
    const deserialized = deserializeFPInterval(d);
    t.expect(
      objectEquals(interval, deserialized),
      `interval ${interval} -> serialize -> deserialize -> ${deserialized}`
    );
  }
});

g.test('fpinterval_abstract').fn(t => {
  for (const interval of [
    FP.abstract.toInterval(0),
    FP.abstract.toInterval(-0),
    FP.abstract.toInterval(1),
    FP.abstract.toInterval(-1),
    FP.abstract.toInterval(0.5),
    FP.abstract.toInterval(-0.5),
    FP.abstract.toInterval(kValue.f64.positive.max),
    FP.abstract.toInterval(kValue.f64.positive.min),
    FP.abstract.toInterval(kValue.f64.positive.subnormal.max),
    FP.abstract.toInterval(kValue.f64.positive.subnormal.min),
    FP.abstract.toInterval(kValue.f64.negative.subnormal.max),
    FP.abstract.toInterval(kValue.f64.negative.subnormal.min),
    FP.abstract.toInterval(kValue.f64.positive.infinity),
    FP.abstract.toInterval(kValue.f64.negative.infinity),

    FP.abstract.toInterval([-0, 0]),
    FP.abstract.toInterval([-1, 1]),
    FP.abstract.toInterval([-0.5, 0.5]),
    FP.abstract.toInterval([kValue.f64.positive.min, kValue.f64.positive.max]),
    FP.abstract.toInterval([kValue.f64.positive.subnormal.min, kValue.f64.positive.subnormal.max]),
    FP.abstract.toInterval([kValue.f64.negative.subnormal.min, kValue.f64.negative.subnormal.max]),
    FP.abstract.toInterval([kValue.f64.negative.infinity, kValue.f64.positive.infinity]),
  ]) {
    const s = new BinaryStream(new Uint8Array(1024).buffer);
    serializeFPInterval(s, interval);
    const d = new BinaryStream(s.buffer().buffer);
    const deserialized = deserializeFPInterval(d);
    t.expect(
      objectEquals(interval, deserialized),
      `interval ${interval} -> serialize -> deserialize -> ${deserialized}`
    );
  }
});

g.test('expression_expectation').fn(t => {
  for (const expectation of [
    // Value
    f32(123),
    vec2(f32(1), f32(2)),
    // Interval
    FP.f32.toInterval([-0.5, 0.5]),
    FP.f32.toInterval([kValue.f32.positive.min, kValue.f32.positive.max]),
    // Intervals
    [FP.f32.toInterval([-8.0, 0.5]), FP.f32.toInterval([2.0, 4.0])],
  ]) {
    const s = new BinaryStream(new Uint8Array(1024).buffer);
    serializeExpectation(s, expectation);
    const d = new BinaryStream(s.buffer().buffer);
    const deserialized = deserializeExpectation(d);
    t.expect(
      objectEquals(expectation, deserialized),
      `expectation ${expectation} -> serialize -> deserialize -> ${deserialized}`
    );
  }
});

/**
 * Temporarily enabled building of the data cache.
 * Required for Comparators to serialize.
 */
function enableBuildingDataCache(f: () => void) {
  const wasBuildingDataCache = getIsBuildingDataCache();
  setIsBuildingDataCache(true);
  f();
  setIsBuildingDataCache(wasBuildingDataCache);
}

g.test('anyOf').fn(t => {
  enableBuildingDataCache(() => {
    for (const c of [
      {
        comparator: anyOf(i32(123)),
        testCases: [f32(0), f32(10), f32(122), f32(123), f32(124), f32(200)],
      },
    ]) {
      const s = new BinaryStream(new Uint8Array(1024).buffer);
      serializeComparator(s, c.comparator);
      const d = new BinaryStream(s.buffer().buffer);
      const deserialized = deserializeComparator(d);
      for (const val of c.testCases) {
        const got = deserialized.compare(val);
        const expect = c.comparator.compare(val);
        t.expect(
          got.matched === expect.matched,
          `comparator(${val}): got: ${expect.matched}, expect: ${got.matched}`
        );
      }
    }
  });
});

g.test('skipUndefined').fn(t => {
  enableBuildingDataCache(() => {
    for (const c of [
      {
        comparator: skipUndefined(i32(123)),
        testCases: [f32(0), f32(10), f32(122), f32(123), f32(124), f32(200)],
      },
      {
        comparator: skipUndefined(undefined),
        testCases: [f32(0), f32(10), f32(122), f32(123), f32(124), f32(200)],
      },
    ]) {
      const s = new BinaryStream(new Uint8Array(1024).buffer);
      serializeComparator(s, c.comparator);
      const d = new BinaryStream(s.buffer().buffer);
      const deserialized = deserializeComparator(d);
      for (const val of c.testCases) {
        const got = deserialized.compare(val);
        const expect = c.comparator.compare(val);
        t.expect(
          got.matched === expect.matched,
          `comparator(${val}): got: ${expect.matched}, expect: ${got.matched}`
        );
      }
    }
  });
});
