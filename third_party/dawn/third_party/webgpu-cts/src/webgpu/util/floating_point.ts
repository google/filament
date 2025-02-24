import { ROArrayArray, ROArrayArrayArray } from '../../common/util/types.js';
import { assert, unreachable } from '../../common/util/util.js';
import { Float16Array } from '../../external/petamoriken/float16/float16.js';
import { Case } from '../shader/execution/expression/case.js';
import { IntervalFilter } from '../shader/execution/expression/interval_filter.js';

import BinaryStream from './binary_stream.js';
import { anyOf } from './compare.js';
import { kValue } from './constants.js';
import {
  abstractFloat,
  f16,
  f32,
  isFloatType,
  ScalarValue,
  ScalarType,
  toMatrix,
  toVector,
  u32,
} from './conversion.js';
import {
  calculatePermutations,
  cartesianProduct,
  correctlyRoundedF16,
  correctlyRoundedF32,
  correctlyRoundedF64,
  every2DArray,
  flatten2DArray,
  FlushMode,
  flushSubnormalNumberF16,
  flushSubnormalNumberF32,
  flushSubnormalNumberF64,
  isFiniteF16,
  isFiniteF32,
  isSubnormalNumberF16,
  isSubnormalNumberF32,
  isSubnormalNumberF64,
  map2DArray,
  oneULPF16,
  oneULPF32,
  quantizeToF16,
  quantizeToF32,
  scalarF16Range,
  scalarF32Range,
  scalarF64Range,
  sparseMatrixF16Range,
  sparseMatrixF32Range,
  sparseMatrixF64Range,
  sparseScalarF16Range,
  sparseScalarF32Range,
  sparseScalarF64Range,
  sparseVectorF16Range,
  sparseVectorF32Range,
  sparseVectorF64Range,
  unflatten2DArray,
  vectorF16Range,
  vectorF32Range,
  vectorF64Range,
} from './math.js';

/** Indicate the kind of WGSL floating point numbers being operated on */
export type FPKind = 'f32' | 'f16' | 'abstract';

enum SerializedFPIntervalKind {
  Abstract,
  F32,
  F16,
}

/** serializeFPKind() serializes a FPKind to a BinaryStream */
export function serializeFPKind(s: BinaryStream, value: FPKind) {
  switch (value) {
    case 'abstract':
      s.writeU8(SerializedFPIntervalKind.Abstract);
      break;
    case 'f16':
      s.writeU8(SerializedFPIntervalKind.F16);
      break;
    case 'f32':
      s.writeU8(SerializedFPIntervalKind.F32);
      break;
  }
}

/** deserializeFPKind() deserializes a FPKind from a BinaryStream */
export function deserializeFPKind(s: BinaryStream): FPKind {
  const kind = s.readU8();
  switch (kind) {
    case SerializedFPIntervalKind.Abstract:
      return 'abstract';
    case SerializedFPIntervalKind.F16:
      return 'f16';
    case SerializedFPIntervalKind.F32:
      return 'f32';
    default:
      unreachable(`invalid deserialized FPKind: ${kind}`);
  }
}
// Containers

/**
 * Representation of endpoints for an interval as an array with either one or
 * two elements. Single element indicates that the interval is a single point.
 * For two elements, the first is the lower edges of the interval and the
 * second is the upper edge, i.e. e[0] <= e[1], where e is an IntervalEndpoints
 */
export type IntervalEndpoints = readonly [number] | readonly [number, number];

/** Represents a closed interval of floating point numbers */
export class FPInterval {
  public readonly kind: FPKind;
  public readonly begin: number;
  public readonly end: number;

  /**
   * Constructor
   *
   * `FPTraits.toInterval` is the preferred way to create FPIntervals
   *
   * @param kind the floating point number type this is an interval for
   * @param endpoints beginning and end of the interval
   */
  public constructor(kind: FPKind, ...endpoints: IntervalEndpoints) {
    this.kind = kind;

    const begin = endpoints[0];
    const end = endpoints.length === 2 ? endpoints[1] : endpoints[0];
    assert(!Number.isNaN(begin) && !Number.isNaN(end), `endpoints need to be non-NaN`);
    assert(
      begin <= end,
      `endpoints[0] (${begin}) must be less than or equal to endpoints[1]  (${end})`
    );

    this.begin = begin;
    this.end = end;
  }

  /** @returns the floating point traits for this interval */
  public traits(): FPTraits {
    return FP[this.kind];
  }

  /** @returns begin and end if non-point interval, otherwise just begin */
  public endpoints(): IntervalEndpoints {
    return this.isPoint() ? [this.begin] : [this.begin, this.end];
  }

  /** @returns if a point or interval is completely contained by this interval */
  public contains(n: number | FPInterval): boolean {
    if (Number.isNaN(n)) {
      // Being the 'any' interval indicates that accuracy is not defined for this
      // test, so the test is just checking that this input doesn't cause the
      // implementation to misbehave, so NaN is accepted.
      return this.begin === Number.NEGATIVE_INFINITY && this.end === Number.POSITIVE_INFINITY;
    }

    if (n instanceof FPInterval) {
      return this.begin <= n.begin && this.end >= n.end;
    }
    return this.begin <= n && this.end >= n;
  }

  /** @returns if any values in the interval may be flushed to zero, this
   *           includes any subnormals and zero itself.
   */
  public containsZeroOrSubnormals(): boolean {
    return !(
      this.end < this.traits().constants().negative.subnormal.min ||
      this.begin > this.traits().constants().positive.subnormal.max
    );
  }

  /** @returns if this interval contains a single point */
  public isPoint(): boolean {
    return this.begin === this.end;
  }

  /** @returns if this interval only contains finite values */
  public isFinite(): boolean {
    return this.traits().isFinite(this.begin) && this.traits().isFinite(this.end);
  }

  /** @returns a string representation for logging purposes */
  public toString(): string {
    return `{ '${this.kind}', [${this.endpoints().map(this.traits().scalarBuilder)}] }`;
  }
}

/** serializeFPInterval() serializes a FPInterval to a BinaryStream */
export function serializeFPInterval(s: BinaryStream, i: FPInterval) {
  serializeFPKind(s, i.kind);
  const traits = FP[i.kind];
  s.writeCond(i !== traits.constants().unboundedInterval, {
    if_true: () => {
      // Bounded
      switch (i.kind) {
        case 'abstract':
          s.writeF64(i.begin);
          s.writeF64(i.end);
          break;
        case 'f32':
          s.writeF32(i.begin);
          s.writeF32(i.end);
          break;
        case 'f16':
          s.writeF16(i.begin);
          s.writeF16(i.end);
          break;
        default:
          unreachable(`Unable to serialize FPInterval ${i}`);
          break;
      }
    },
    if_false: () => {
      // Unbounded
    },
  });
}

/** deserializeFPInterval() deserializes a FPInterval from a BinaryStream */
export function deserializeFPInterval(s: BinaryStream): FPInterval {
  const kind = deserializeFPKind(s);
  const traits = FP[kind];
  return s.readCond({
    if_true: () => {
      // Bounded
      switch (kind) {
        case 'abstract':
          return new FPInterval(traits.kind, s.readF64(), s.readF64());
        case 'f32':
          return new FPInterval(traits.kind, s.readF32(), s.readF32());
        case 'f16':
          return new FPInterval(traits.kind, s.readF16(), s.readF16());
      }
      unreachable(`Unable to deserialize FPInterval with kind ${kind}`);
    },
    if_false: () => {
      // Unbounded
      return traits.constants().unboundedInterval;
    },
  });
}

/**
 * Representation of a vec2/3/4 of floating point intervals as an array of
 * FPIntervals.
 */
export type FPVector =
  | [FPInterval, FPInterval]
  | [FPInterval, FPInterval, FPInterval]
  | [FPInterval, FPInterval, FPInterval, FPInterval];

/** Shorthand for an Array of Arrays that contains a column-major matrix */
type Array2D<T> = ROArrayArray<T>;

/**
 * Representation of a matCxR of floating point intervals as an array of arrays
 * of FPIntervals. This maps onto the WGSL concept of matrix. Internally
 */
export type FPMatrix =
  | readonly [readonly [FPInterval, FPInterval], readonly [FPInterval, FPInterval]]
  | readonly [
      readonly [FPInterval, FPInterval],
      readonly [FPInterval, FPInterval],
      readonly [FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval],
      readonly [FPInterval, FPInterval],
      readonly [FPInterval, FPInterval],
      readonly [FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
    ]
  | readonly [
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
      readonly [FPInterval, FPInterval, FPInterval, FPInterval],
    ];

// Utilities

/** @returns input with an appended 0, if inputs contains non-zero subnormals */
// When f16 traits is defined, this can be replaced with something like
// `FP.f16..addFlushIfNeeded`
function addFlushedIfNeededF16(values: readonly number[]): readonly number[] {
  return values.some(v => v !== 0 && isSubnormalNumberF16(v)) ? values.concat(0) : values;
}

// Operations

/**
 * A function that converts a point to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarToInterval {
  (x: number): FPInterval;
}

/** Operation used to implement a ScalarToInterval */
interface ScalarToIntervalOp {
  /** @returns acceptance interval for a function at point x */
  impl: ScalarToInterval;

  /**
   * Calculates where in the domain defined by x the min/max extrema of impl
   * occur and returns a span of those points to be used as the domain instead.
   *
   * Used by this.runScalarToIntervalOp before invoking impl.
   * If not defined, the endpoints of the existing domain are assumed to be the
   * extrema.
   *
   * This is only implemented for operations that meet all the following
   * criteria:
   *   a) non-monotonic
   *   b) used in inherited accuracy calculations
   *   c) need to take in an interval for b)
   *      i.e. fooInterval takes in x: number | FPInterval, not x: number
   */
  extrema?: (x: FPInterval) => FPInterval;

  /**
   * Restricts the inputs to operation to the given domain.
   *
   * Only defined for operations that have tighter domain requirements than 'must
   * be finite'.
   */
  domain?: () => FPInterval;
}

/**
 * A function that converts a pair of points to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarPairToInterval {
  (x: number, y: number): FPInterval;
}

/** Domain for a ScalarPairToInterval implementation */
interface ScalarPairToIntervalDomain {
  // Arrays to support discrete valid domain intervals
  x: readonly FPInterval[];
  y: readonly FPInterval[];
}

/** Operation used to implement a ScalarPairToInterval */
interface ScalarPairToIntervalOp {
  /** @returns acceptance interval for a function at point (x, y) */
  impl: ScalarPairToInterval;
  /**
   * Calculates where in domain defined by x & y the min/max extrema of impl
   * occur and returns spans of those points to be used as the domain instead.
   *
   * Used by runScalarPairToIntervalOp before invoking impl.
   * If not defined, the endpoints of the existing domain are assumed to be the
   * extrema.
   *
   * This is only implemented for functions that meet all the following
   * criteria:
   *   a) non-monotonic
   *   b) used in inherited accuracy calculations
   *   c) need to take in an interval for b)
   */
  extrema?: (x: FPInterval, y: FPInterval) => [FPInterval, FPInterval];

  /**
   * Restricts the inputs to operation to the given domain.
   *
   * Only defined for operations that have tighter domain requirements than 'must
   * be finite'.
   */
  domain?: () => ScalarPairToIntervalDomain;
}

/**
 * A function that converts a triplet of points to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarTripleToInterval {
  (x: number, y: number, z: number): FPInterval;
}

/** Operation used to implement a ScalarTripleToInterval */
interface ScalarTripleToIntervalOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns acceptance interval for a function at point (x, y, z) */
  impl: ScalarTripleToInterval;
}

// Currently ScalarToVector is not integrated with the rest of the floating point
// framework, because the only builtins that use it are actually
// u32 -> [f32, f32, f32, f32] functions, so the whole rounding and interval
// process doesn't get applied to the inputs.
// They do use the framework internally by invoking divisionInterval on segments
// of the input.
/**
 * A function that converts a point to a vector of acceptance intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarToVector {
  (n: number): FPVector;
}

/**
 * A function that converts a vector to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorToInterval {
  (x: readonly number[]): FPInterval;
}

/** Operation used to implement a VectorToInterval */
interface VectorToIntervalOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns acceptance interval for a function on vector x */
  impl: VectorToInterval;
}

/**
 * A function that converts a pair of vectors to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorPairToInterval {
  (x: readonly number[], y: readonly number[]): FPInterval;
}

/** Operation used to implement a VectorPairToInterval */
interface VectorPairToIntervalOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns acceptance interval for a function on vectors (x, y) */
  impl: VectorPairToInterval;
}

/**
 * A function that converts a vector to a vector of acceptance intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorToVector {
  (x: readonly number[]): FPVector;
}

/** Operation used to implement a VectorToVector */
interface VectorToVectorOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns a vector of acceptance intervals for a function on vector x */
  impl: VectorToVector;
}

/**
 * A function that converts a pair of vectors to a vector of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorPairToVector {
  (x: readonly number[], y: readonly number[]): FPVector;
}

/** Operation used to implement a VectorPairToVector */
interface VectorPairToVectorOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns a vector of acceptance intervals for a function on vectors (x, y) */
  impl: VectorPairToVector;
}

/**
 * A function that converts a vector and a scalar to a vector of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorScalarToVector {
  (x: readonly number[], y: number): FPVector;
}

/**
 * A function that converts a scalar and a vector  to a vector of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarVectorToVector {
  (x: number, y: readonly number[]): FPVector;
}

/**
 * A function that converts a matrix to an acceptance interval.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface MatrixToScalar {
  (m: Array2D<number>): FPInterval;
}

/** Operation used to implement a MatrixToMatrix */
interface MatrixToMatrixOp {
  // Re-using the *Op interface pattern for symmetry with the other operations.
  /** @returns a matrix of acceptance intervals for a function on matrix x */
  impl: MatrixToMatrix;
}

/**
 * A function that converts a matrix to a matrix of acceptance intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface MatrixToMatrix {
  (m: Array2D<number>): FPMatrix;
}

/**
 * A function that converts a pair of matrices to a matrix of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface MatrixPairToMatrix {
  (x: Array2D<number>, y: Array2D<number>): FPMatrix;
}

/**
 * A function that converts a matrix and a scalar to a matrix of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface MatrixScalarToMatrix {
  (x: Array2D<number>, y: number): FPMatrix;
}

/**
 * A function that converts a scalar and a matrix to a matrix of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface ScalarMatrixToMatrix {
  (x: number, y: Array2D<number>): FPMatrix;
}

/**
 * A function that converts a matrix and a vector to a vector of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface MatrixVectorToVector {
  (x: Array2D<number>, y: readonly number[]): FPVector;
}

/**
 * A function that converts a vector and a matrix to a vector of acceptance
 * intervals.
 * This is the public facing API for builtin implementations that is called
 * from tests.
 */
export interface VectorMatrixToVector {
  (x: readonly number[], y: Array2D<number>): FPVector;
}

// Traits

/**
 * Typed structure containing all the constants defined for each
 * WGSL floating point kind
 */
interface FPConstants {
  positive: {
    min: number;
    max: number;
    infinity: number;
    nearest_max: number;
    less_than_one: number;
    subnormal: {
      min: number;
      max: number;
    };
    pi: {
      whole: number;
      three_quarters: number;
      half: number;
      third: number;
      quarter: number;
      sixth: number;
    };
    e: number;
  };
  negative: {
    min: number;
    max: number;
    infinity: number;
    nearest_min: number;
    less_than_one: number;
    subnormal: {
      min: number;
      max: number;
    };
    pi: {
      whole: number;
      three_quarters: number;
      half: number;
      third: number;
      quarter: number;
      sixth: number;
    };
  };
  bias: number;
  unboundedInterval: FPInterval;
  zeroInterval: FPInterval;
  negPiToPiInterval: FPInterval;
  greaterThanZeroInterval: FPInterval;
  negOneToOneInterval: FPInterval;
  zeroVector: {
    2: FPVector;
    3: FPVector;
    4: FPVector;
  };
  unboundedVector: {
    2: FPVector;
    3: FPVector;
    4: FPVector;
  };
  unboundedMatrix: {
    2: {
      2: FPMatrix;
      3: FPMatrix;
      4: FPMatrix;
    };
    3: {
      2: FPMatrix;
      3: FPMatrix;
      4: FPMatrix;
    };
    4: {
      2: FPMatrix;
      3: FPMatrix;
      4: FPMatrix;
    };
  };
}

/** A representation of an FPInterval for a case param */
export type FPIntervalParam = {
  kind: FPKind;
  interval: number | IntervalEndpoints;
};

/** Abstract base class for all floating-point traits */
export abstract class FPTraits {
  public readonly kind: FPKind;
  protected constructor(k: FPKind) {
    this.kind = k;
  }

  public abstract constants(): FPConstants;

  // Utilities - Implemented

  /** @returns an interval containing the point or the original interval */
  public toInterval(n: number | IntervalEndpoints | FPInterval): FPInterval {
    if (n instanceof FPInterval) {
      if (n.kind === this.kind) {
        return n;
      }

      // Preserve if the original interval was unbounded or bounded
      if (!n.isFinite()) {
        return this.constants().unboundedInterval;
      }

      return new FPInterval(this.kind, ...n.endpoints());
    }

    if (n instanceof Array) {
      return new FPInterval(this.kind, ...n);
    }

    return new FPInterval(this.kind, n, n);
  }

  /**
   * Makes a param that can be turned into an interval
   */
  public toParam(n: number | IntervalEndpoints): FPIntervalParam {
    return {
      kind: this.kind,
      interval: n,
    };
  }

  /**
   * Converts p into an FPInterval if it is an FPIntervalPAram
   */
  public fromParam(
    p: number | IntervalEndpoints | FPIntervalParam
  ): number | IntervalEndpoints | FPInterval {
    const param = p as FPIntervalParam;
    if (param.interval && param.kind) {
      assert(param.kind === this.kind);
      return this.toInterval(param.interval);
    }
    return p as number | IntervalEndpoints;
  }

  /**
   * @returns an interval with the tightest endpoints that includes all provided
   *          intervals
   */
  public spanIntervals(...intervals: readonly FPInterval[]): FPInterval {
    assert(intervals.length > 0, `span of an empty list of FPIntervals is not allowed`);
    assert(
      intervals.every(i => i.kind === this.kind),
      `span is only defined for intervals with the same kind`
    );
    let begin = Number.POSITIVE_INFINITY;
    let end = Number.NEGATIVE_INFINITY;
    intervals.forEach(i => {
      begin = Math.min(i.begin, begin);
      end = Math.max(i.end, end);
    });
    return this.toInterval([begin, end]);
  }

  /** Narrow an array of values to FPVector if possible */
  public isVector(v: ReadonlyArray<number | IntervalEndpoints | FPInterval>): v is FPVector {
    if (v.every(e => e instanceof FPInterval && e.kind === this.kind)) {
      return v.length === 2 || v.length === 3 || v.length === 4;
    }
    return false;
  }

  /** @returns an FPVector representation of an array of values if possible */
  public toVector(v: ReadonlyArray<number | IntervalEndpoints | FPInterval>): FPVector {
    if (this.isVector(v) && v.every(e => e.kind === this.kind)) {
      return v;
    }

    const f = v.map(e => this.toInterval(e));
    // The return of the map above is a readonly FPInterval[], which needs to be narrowed
    // to FPVector, since FPVector is defined as fixed length tuples.
    if (this.isVector(f)) {
      return f;
    }
    unreachable(`Cannot convert [${v}] to FPVector`);
  }

  /**
   * @returns a FPVector where each element is the span for corresponding
   *          elements at the same index in the input vectors
   */
  public spanVectors(...vectors: FPVector[]): FPVector {
    assert(
      vectors.every(e => this.isVector(e)),
      'Vector span is not defined for vectors of differing floating point kinds'
    );

    const vector_length = vectors[0].length;
    assert(
      vectors.every(e => e.length === vector_length),
      `Vector span is not defined for vectors of differing lengths`
    );

    const result: FPInterval[] = new Array<FPInterval>(vector_length);

    for (let i = 0; i < vector_length; i++) {
      result[i] = this.spanIntervals(...vectors.map(v => v[i]));
    }
    return this.toVector(result);
  }

  /** Narrow an array of an array of values to FPMatrix if possible */
  public isMatrix(m: Array2D<number | IntervalEndpoints | FPInterval> | FPVector[]): m is FPMatrix {
    if (!m.every(c => c.every(e => e instanceof FPInterval && e.kind === this.kind))) {
      return false;
    }
    // At this point m guaranteed to be a ROArrayArray<FPInterval>, but maybe typed as a
    // FPVector[].
    // Coercing the type since FPVector[] is functionally equivalent to
    // ROArrayArray<FPInterval> for .length and .every, but they are type compatible,
    // since tuples are not equivalent to arrays, so TS considers c in .every to
    // be unresolvable below, even though our usage is safe.
    m = m as ROArrayArray<FPInterval>;

    if (m.length > 4 || m.length < 2) {
      return false;
    }

    const num_rows = m[0].length;
    if (num_rows > 4 || num_rows < 2) {
      return false;
    }

    return m.every(c => c.length === num_rows);
  }

  /** @returns an FPMatrix representation of an array of an array of values if possible */
  public toMatrix(m: Array2D<number | IntervalEndpoints | FPInterval> | FPVector[]): FPMatrix {
    if (
      this.isMatrix(m) &&
      every2DArray(m, (e: FPInterval) => {
        return e.kind === this.kind;
      })
    ) {
      return m;
    }

    const result = map2DArray(m, this.toInterval.bind(this));

    // The return of the map above is a ROArrayArray<FPInterval>, which needs to be
    // narrowed to FPMatrix, since FPMatrix is defined as fixed length tuples.
    if (this.isMatrix(result)) {
      return result;
    }
    unreachable(`Cannot convert ${m} to FPMatrix`);
  }

  /**
   * @returns a FPMatrix where each element is the span for corresponding
   *          elements at the same index in the input matrices
   */
  public spanMatrices(...matrices: FPMatrix[]): FPMatrix {
    // Coercing the type of matrices, since tuples are not generally compatible
    // with Arrays, but they are functionally equivalent for the usages in this
    // function.
    const ms = matrices as Array2D<FPInterval>[];
    const num_cols = ms[0].length;
    const num_rows = ms[0][0].length;
    assert(
      ms.every(m => m.length === num_cols && m.every(r => r.length === num_rows)),
      `Matrix span is not defined for Matrices of differing dimensions`
    );

    const result: FPInterval[][] = [...Array(num_cols)].map(_ => [...Array(num_rows)]);
    for (let i = 0; i < num_cols; i++) {
      for (let j = 0; j < num_rows; j++) {
        result[i][j] = this.spanIntervals(...ms.map(m => m[i][j]));
      }
    }

    return this.toMatrix(result);
  }

  /** @returns input with an appended 0, if inputs contains non-zero subnormals */
  public addFlushedIfNeeded(values: readonly number[]): readonly number[] {
    const subnormals = values.filter(this.isSubnormal);
    const needs_zero = subnormals.length > 0 && subnormals.every(s => s !== 0);
    return needs_zero ? values.concat(0) : values;
  }

  /** Stub for scalar to interval generator */
  protected unimplementedScalarToInterval(name: string, _x: number | FPInterval): FPInterval {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for scalar pair to interval generator */
  protected unimplementedScalarPairToInterval(
    name: string,
    _x: number | FPInterval,
    _y: number | FPInterval
  ): FPInterval {
    unreachable(`'${name}' is yet implemented for '${this.kind}'`);
  }

  /** Stub for scalar triple to interval generator */
  protected unimplementedScalarTripleToInterval(
    name: string,
    _x: number | FPInterval,
    _y: number | FPInterval,
    _z: number | FPInterval
  ): FPInterval {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for scalar to vector generator */
  protected unimplementedScalarToVector(name: string, _x: number | FPInterval): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector to interval generator */
  protected unimplementedVectorToInterval(name: string, _x: (number | FPInterval)[]): FPInterval {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector pair to interval generator */
  protected unimplementedVectorPairToInterval(
    name: string,
    _x: readonly (number | FPInterval)[],
    _y: readonly (number | FPInterval)[]
  ): FPInterval {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector to vector generator */
  protected unimplementedVectorToVector(
    name: string,
    _x: readonly (number | FPInterval)[]
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector pair to vector generator */
  protected unimplementedVectorPairToVector(
    name: string,
    _x: readonly (number | FPInterval)[],
    _y: readonly (number | FPInterval)[]
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector-scalar to vector generator */
  protected unimplementedVectorScalarToVector(
    name: string,
    _x: readonly (number | FPInterval)[],
    _y: number | FPInterval
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for scalar-vector to vector generator */
  protected unimplementedScalarVectorToVector(
    name: string,
    _x: number | FPInterval,
    _y: (number | FPInterval)[]
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for matrix to interval generator */
  protected unimplementedMatrixToInterval(name: string, _x: Array2D<number>): FPInterval {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for matrix to matirx generator */
  protected unimplementedMatrixToMatrix(name: string, _x: Array2D<number>): FPMatrix {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for matrix pair to matrix generator */
  protected unimplementedMatrixPairToMatrix(
    name: string,
    _x: Array2D<number>,
    _y: Array2D<number>
  ): FPMatrix {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for matrix-scalar to matrix generator  */
  protected unimplementedMatrixScalarToMatrix(
    name: string,
    _x: Array2D<number>,
    _y: number | FPInterval
  ): FPMatrix {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for scalar-matrix to matrix generator  */
  protected unimplementedScalarMatrixToMatrix(
    name: string,
    _x: number | FPInterval,
    _y: Array2D<number>
  ): FPMatrix {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for matrix-vector to vector generator  */
  protected unimplementedMatrixVectorToVector(
    name: string,
    _x: Array2D<number>,
    _y: readonly (number | FPInterval)[]
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for vector-matrix to vector generator  */
  protected unimplementedVectorMatrixToVector(
    name: string,
    _x: readonly (number | FPInterval)[],
    _y: Array2D<number>
  ): FPVector {
    unreachable(`'${name}' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for distance generator */
  protected unimplementedDistance(
    _x: number | readonly number[],
    _y: number | readonly number[]
  ): FPInterval {
    unreachable(`'distance' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for faceForward */
  protected unimplementedFaceForward(
    _x: readonly number[],
    _y: readonly number[],
    _z: readonly number[]
  ): (FPVector | undefined)[] {
    unreachable(`'faceForward' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for length generator */
  protected unimplementedLength(
    _x: number | FPInterval | readonly number[] | FPVector
  ): FPInterval {
    unreachable(`'length' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for modf generator */
  protected unimplementedModf(_x: number): { fract: FPInterval; whole: FPInterval } {
    unreachable(`'modf' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for refract generator */
  protected unimplementedRefract(
    _i: readonly number[],
    _s: readonly number[],
    _r: number
  ): FPVector {
    unreachable(`'refract' is not yet implemented for '${this.kind}'`);
  }

  /** Stub for absolute errors */
  protected unimplementedAbsoluteErrorInterval(_n: number, _error_range: number): FPInterval {
    unreachable(`Absolute Error is not implement for '${this.kind}'`);
  }

  /** Stub for ULP errors */
  protected unimplementedUlpInterval(_n: number, _numULP: number): FPInterval {
    unreachable(`ULP Error is not implement for '${this.kind}'`);
  }

  // Utilities - Defined by subclass
  /**
   * @returns the nearest precise value to the input. Rounding should be IEEE
   *          'roundTiesToEven'.
   */
  public abstract readonly quantize: (n: number) => number;
  /** @returns all valid roundings of input */
  public abstract readonly correctlyRounded: (n: number) => readonly number[];
  /** @returns true if input is considered finite, otherwise false */
  public abstract readonly isFinite: (n: number) => boolean;
  /** @returns true if input is considered subnormal, otherwise false */
  public abstract readonly isSubnormal: (n: number) => boolean;
  /** @returns 0 if the provided number is subnormal, otherwise returns the proved number */
  public abstract readonly flushSubnormal: (n: number) => number;
  /** @returns 1 * ULP: (number) */
  public abstract readonly oneULP: (target: number, mode?: FlushMode) => number;
  /** @returns a builder for converting numbers to ScalarsValues */
  public abstract readonly scalarBuilder: (n: number) => ScalarValue;
  /** @returns a range of scalars for testing */
  public abstract scalarRange(): readonly number[];
  /** @returns a reduced range of scalars for testing */
  public abstract sparseScalarRange(): readonly number[];
  /** @returns a range of dim element vectors for testing */
  public abstract vectorRange(dim: number): ROArrayArray<number>;
  /** @returns a reduced range of dim element vectors for testing */
  public abstract sparseVectorRange(dim: number): ROArrayArray<number>;
  /** @returns a reduced range of cols x rows matrices for testing
   *
   * A non-sparse version of this generator is intentionally not provided due to
   * runtime issues with more dense ranges.
   */
  public abstract sparseMatrixRange(cols: number, rows: number): ROArrayArrayArray<number>;

  // Framework - Cases

  /**
   * @returns a Case for the param and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeScalarToIntervalCase(
    param: number,
    filter: IntervalFilter,
    ...ops: ScalarToInterval[]
  ): Case | undefined {
    param = this.quantize(param);

    const intervals = ops.map(o => o(param));
    if (filter === 'finite' && intervals.some(i => !i.isFinite())) {
      return undefined;
    }
    return { input: [this.scalarBuilder(param)], expected: anyOf(...intervals) };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateScalarToIntervalCases(
    params: readonly number[],
    filter: IntervalFilter,
    ...ops: ScalarToInterval[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeScalarToIntervalCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param0 the first param to pass in
   * @param param1 the second param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeScalarPairToIntervalCase(
    param0: number,
    param1: number,
    filter: IntervalFilter,
    ...ops: ScalarPairToInterval[]
  ): Case | undefined {
    param0 = this.quantize(param0);
    param1 = this.quantize(param1);

    const intervals = ops.map(o => o(param0, param1));
    if (filter === 'finite' && intervals.some(i => !i.isFinite())) {
      return undefined;
    }
    return {
      input: [this.scalarBuilder(param0), this.scalarBuilder(param1)],
      expected: anyOf(...intervals),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of inputs to try for the first input
   * @param param1s array of inputs to try for the second input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateScalarPairToIntervalCases(
    param0s: readonly number[],
    param1s: readonly number[],
    filter: IntervalFilter,
    ...ops: ScalarPairToInterval[]
  ): Case[] {
    return cartesianProduct(param0s, param1s).reduce((cases, e) => {
      const c = this.makeScalarPairToIntervalCase(e[0], e[1], filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param0 the first param to pass in
   * @param param1 the second param to pass in
   * @param param2 the third param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public makeScalarTripleToIntervalCase(
    param0: number,
    param1: number,
    param2: number,
    filter: IntervalFilter,
    ...ops: ScalarTripleToInterval[]
  ): Case | undefined {
    param0 = this.quantize(param0);
    param1 = this.quantize(param1);
    param2 = this.quantize(param2);

    const intervals = ops.map(o => o(param0, param1, param2));
    if (filter === 'finite' && intervals.some(i => !i.isFinite())) {
      return undefined;
    }
    return {
      input: [this.scalarBuilder(param0), this.scalarBuilder(param1), this.scalarBuilder(param2)],
      expected: anyOf(...intervals),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of inputs to try for the first input
   * @param param1s array of inputs to try for the second input
   * @param param2s array of inputs to try for the third input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateScalarTripleToIntervalCases(
    param0s: readonly number[],
    param1s: readonly number[],
    param2s: readonly number[],
    filter: IntervalFilter,
    ...ops: ScalarTripleToInterval[]
  ): Case[] {
    return cartesianProduct(param0s, param1s, param2s).reduce((cases, e) => {
      const c = this.makeScalarTripleToIntervalCase(e[0], e[1], e[2], filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeVectorToIntervalCase(
    param: readonly number[],
    filter: IntervalFilter,
    ...ops: VectorToInterval[]
  ): Case | undefined {
    param = param.map(this.quantize);

    const intervals = ops.map(o => o(param));
    if (filter === 'finite' && intervals.some(i => !i.isFinite())) {
      return undefined;
    }
    return {
      input: [toVector(param, this.scalarBuilder)],
      expected: anyOf(...intervals),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateVectorToIntervalCases(
    params: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorToInterval[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeVectorToIntervalCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param0 the first param to pass in
   * @param param1 the second param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeVectorPairToIntervalCase(
    param0: readonly number[],
    param1: readonly number[],
    filter: IntervalFilter,
    ...ops: VectorPairToInterval[]
  ): Case | undefined {
    param0 = param0.map(this.quantize);
    param1 = param1.map(this.quantize);

    const intervals = ops.map(o => o(param0, param1));
    if (filter === 'finite' && intervals.some(i => !i.isFinite())) {
      return undefined;
    }
    return {
      input: [toVector(param0, this.scalarBuilder), toVector(param1, this.scalarBuilder)],
      expected: anyOf(...intervals),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of inputs to try for the first input
   * @param param1s array of inputs to try for the second input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateVectorPairToIntervalCases(
    param0s: ROArrayArray<number>,
    param1s: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorPairToInterval[]
  ): Case[] {
    return cartesianProduct(param0s, param1s).reduce((cases, e) => {
      const c = this.makeVectorPairToIntervalCase(e[0], e[1], filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the param and vector of intervals generator provided
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals.
   */
  private makeVectorToVectorCase(
    param: readonly number[],
    filter: IntervalFilter,
    ...ops: VectorToVector[]
  ): Case | undefined {
    param = param.map(this.quantize);

    const vectors = ops.map(o => o(param));
    if (filter === 'finite' && vectors.some(v => v.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [toVector(param, this.scalarBuilder)],
      expected: anyOf(...vectors),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals.
   */
  public generateVectorToVectorCases(
    params: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorToVector[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeVectorToVectorCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the interval vector generator provided.
   * The Case will use an interval comparator for matching results.
   * @param scalar the scalar param to pass in
   * @param vector the vector param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance intervals
   */
  private makeScalarVectorToVectorCase(
    scalar: number,
    vector: readonly number[],
    filter: IntervalFilter,
    ...ops: ScalarVectorToVector[]
  ): Case | undefined {
    scalar = this.quantize(scalar);
    vector = vector.map(this.quantize);

    const results = ops.map(o => o(scalar, vector));
    if (filter === 'finite' && results.some(r => r.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [this.scalarBuilder(scalar), toVector(vector, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param scalars array of scalar inputs to try
   * @param vectors array of vector inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance intervals
   */
  public generateScalarVectorToVectorCases(
    scalars: readonly number[],
    vectors: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: ScalarVectorToVector[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    scalars.forEach(scalar => {
      vectors.forEach(vector => {
        const c = this.makeScalarVectorToVectorCase(scalar, vector, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the params and the interval vector generator provided.
   * The Case will use an interval comparator for matching results.
   * @param vector the vector param to pass in
   * @param scalar the scalar param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance intervals
   */
  private makeVectorScalarToVectorCase(
    vector: readonly number[],
    scalar: number,
    filter: IntervalFilter,
    ...ops: VectorScalarToVector[]
  ): Case | undefined {
    vector = vector.map(this.quantize);
    scalar = this.quantize(scalar);

    const results = ops.map(o => o(vector, scalar));
    if (filter === 'finite' && results.some(r => r.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [toVector(vector, this.scalarBuilder), this.scalarBuilder(scalar)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param vectors array of vector inputs to try
   * @param scalars array of scalar inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance intervals
   */
  public generateVectorScalarToVectorCases(
    vectors: ROArrayArray<number>,
    scalars: readonly number[],
    filter: IntervalFilter,
    ...ops: VectorScalarToVector[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    vectors.forEach(vector => {
      scalars.forEach(scalar => {
        const c = this.makeVectorScalarToVectorCase(vector, scalar, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the param and vector of intervals generator provided
   * @param param0 the first param to pass in
   * @param param1 the second param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals.
   */
  private makeVectorPairToVectorCase(
    param0: readonly number[],
    param1: readonly number[],
    filter: IntervalFilter,
    ...ops: VectorPairToVector[]
  ): Case | undefined {
    param0 = param0.map(this.quantize);
    param1 = param1.map(this.quantize);
    const vectors = ops.map(o => o(param0, param1));
    if (filter === 'finite' && vectors.some(v => v.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [toVector(param0, this.scalarBuilder), toVector(param1, this.scalarBuilder)],
      expected: anyOf(...vectors),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of inputs to try for the first input
   * @param param1s array of inputs to try for the second input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals.
   */
  public generateVectorPairToVectorCases(
    param0s: ROArrayArray<number>,
    param1s: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorPairToVector[]
  ): Case[] {
    return cartesianProduct(param0s, param1s).reduce((cases, e) => {
      const c = this.makeVectorPairToVectorCase(e[0], e[1], filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and the component-wise interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param0 the first vector param to pass in
   * @param param1 the second vector param to pass in
   * @param param2 the scalar param to pass in
   * @param filter what interval filtering to apply
   * @param componentWiseOps callbacks that implement generating a component-wise acceptance interval,
   *                         one component result at a time.
   */
  private makeVectorPairScalarToVectorComponentWiseCase(
    param0: readonly number[],
    param1: readonly number[],
    param2: number,
    filter: IntervalFilter,
    ...componentWiseOps: ScalarTripleToInterval[]
  ): Case | undefined {
    // Width of input vector
    const width = param0.length;
    assert(2 <= width && width <= 4, 'input vector width must between 2 and 4');
    assert(param1.length === width, 'two input vectors must have the same width');
    param0 = param0.map(this.quantize);
    param1 = param1.map(this.quantize);
    param2 = this.quantize(param2);

    // Call the component-wise interval generator and build the expectation FPVector
    const results = componentWiseOps.map(o => {
      return param0.map((el0, index) => o(el0, param1[index], param2)) as FPVector;
    });
    if (filter === 'finite' && results.some(r => r.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [
        toVector(param0, this.scalarBuilder),
        toVector(param1, this.scalarBuilder),
        this.scalarBuilder(param2),
      ],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of first vector inputs to try
   * @param param1s array of second vector inputs to try
   * @param param2s array of scalar inputs to try
   * @param filter what interval filtering to apply
   * @param componentWiseOpscallbacks that implement generating a component-wise acceptance interval
   */
  public generateVectorPairScalarToVectorComponentWiseCase(
    param0s: ROArrayArray<number>,
    param1s: ROArrayArray<number>,
    param2s: readonly number[],
    filter: IntervalFilter,
    ...componentWiseOps: ScalarTripleToInterval[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    param0s.forEach(param0 => {
      param1s.forEach(param1 => {
        param2s.forEach(param2 => {
          const c = this.makeVectorPairScalarToVectorComponentWiseCase(
            param0,
            param1,
            param2,
            filter,
            ...componentWiseOps
          );
          if (c !== undefined) {
            cases.push(c);
          }
        });
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the param and an array of interval generators provided
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeMatrixToScalarCase(
    param: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixToScalar[]
  ): Case | undefined {
    param = map2DArray(param, this.quantize);

    const results = ops.map(o => o(param));
    if (filter === 'finite' && results.some(e => !e.isFinite())) {
      return undefined;
    }

    return {
      input: [toMatrix(param, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateMatrixToScalarCases(
    params: ROArrayArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixToScalar[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeMatrixToScalarCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the param and an array of interval generators provided
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  private makeMatrixToMatrixCase(
    param: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixToMatrix[]
  ): Case | undefined {
    param = map2DArray(param, this.quantize);

    const results = ops.map(o => o(param));
    if (filter === 'finite' && results.some(m => m.some(c => c.some(r => !r.isFinite())))) {
      return undefined;
    }

    return {
      input: [toMatrix(param, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  public generateMatrixToMatrixCases(
    params: ROArrayArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixToMatrix[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeMatrixToMatrixCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and matrix of intervals generator provided
   * @param param0 the first param to pass in
   * @param param1 the second param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  private makeMatrixPairToMatrixCase(
    param0: ROArrayArray<number>,
    param1: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixPairToMatrix[]
  ): Case | undefined {
    param0 = map2DArray(param0, this.quantize);
    param1 = map2DArray(param1, this.quantize);
    const results = ops.map(o => o(param0, param1));
    if (filter === 'finite' && results.some(m => m.some(c => c.some(r => !r.isFinite())))) {
      return undefined;
    }
    return {
      input: [toMatrix(param0, this.scalarBuilder), toMatrix(param1, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param param0s array of inputs to try for the first input
   * @param param1s array of inputs to try for the second input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  public generateMatrixPairToMatrixCases(
    param0s: ROArrayArrayArray<number>,
    param1s: ROArrayArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixPairToMatrix[]
  ): Case[] {
    return cartesianProduct(param0s, param1s).reduce((cases, e) => {
      const c = this.makeMatrixPairToMatrixCase(e[0], e[1], filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  /**
   * @returns a Case for the params and matrix of intervals generator provided
   * @param mat the matrix param to pass in
   * @param scalar the scalar to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  private makeMatrixScalarToMatrixCase(
    mat: ROArrayArray<number>,
    scalar: number,
    filter: IntervalFilter,
    ...ops: MatrixScalarToMatrix[]
  ): Case | undefined {
    mat = map2DArray(mat, this.quantize);
    scalar = this.quantize(scalar);

    const results = ops.map(o => o(mat, scalar));
    if (filter === 'finite' && results.some(m => m.some(c => c.some(r => !r.isFinite())))) {
      return undefined;
    }
    return {
      input: [toMatrix(mat, this.scalarBuilder), this.scalarBuilder(scalar)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param mats array of inputs to try for the matrix input
   * @param scalars array of inputs to try for the scalar input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  public generateMatrixScalarToMatrixCases(
    mats: ROArrayArrayArray<number>,
    scalars: readonly number[],
    filter: IntervalFilter,
    ...ops: MatrixScalarToMatrix[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    mats.forEach(mat => {
      scalars.forEach(scalar => {
        const c = this.makeMatrixScalarToMatrixCase(mat, scalar, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the params and matrix of intervals generator provided
   * @param scalar the scalar to pass in
   * @param mat the matrix param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  private makeScalarMatrixToMatrixCase(
    scalar: number,
    mat: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: ScalarMatrixToMatrix[]
  ): Case | undefined {
    scalar = this.quantize(scalar);
    mat = map2DArray(mat, this.quantize);

    const results = ops.map(o => o(scalar, mat));
    if (filter === 'finite' && results.some(m => m.some(c => c.some(r => !r.isFinite())))) {
      return undefined;
    }
    return {
      input: [this.scalarBuilder(scalar), toMatrix(mat, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param scalars array of inputs to try for the scalar input
   * @param mats array of inputs to try for the matrix input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a matrix of acceptance
   *            intervals
   */
  public generateScalarMatrixToMatrixCases(
    scalars: readonly number[],
    mats: ROArrayArrayArray<number>,
    filter: IntervalFilter,
    ...ops: ScalarMatrixToMatrix[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    mats.forEach(mat => {
      scalars.forEach(scalar => {
        const c = this.makeScalarMatrixToMatrixCase(scalar, mat, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the params and the vector of intervals generator provided
   * @param mat the matrix param to pass in
   * @param vec the vector to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals
   */
  private makeMatrixVectorToVectorCase(
    mat: ROArrayArray<number>,
    vec: readonly number[],
    filter: IntervalFilter,
    ...ops: MatrixVectorToVector[]
  ): Case | undefined {
    mat = map2DArray(mat, this.quantize);
    vec = vec.map(this.quantize);

    const results = ops.map(o => o(mat, vec));
    if (filter === 'finite' && results.some(v => v.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [toMatrix(mat, this.scalarBuilder), toVector(vec, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param mats array of inputs to try for the matrix input
   * @param vecs array of inputs to try for the vector input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals
   */
  public generateMatrixVectorToVectorCases(
    mats: ROArrayArrayArray<number>,
    vecs: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: MatrixVectorToVector[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    mats.forEach(mat => {
      vecs.forEach(vec => {
        const c = this.makeMatrixVectorToVectorCase(mat, vec, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  /**
   * @returns a Case for the params and the vector of intervals generator provided
   * @param vec the vector to pass in
   * @param mat the matrix param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals
   */
  private makeVectorMatrixToVectorCase(
    vec: readonly number[],
    mat: ROArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorMatrixToVector[]
  ): Case | undefined {
    vec = vec.map(this.quantize);
    mat = map2DArray(mat, this.quantize);

    const results = ops.map(o => o(vec, mat));
    if (filter === 'finite' && results.some(v => v.some(e => !e.isFinite()))) {
      return undefined;
    }
    return {
      input: [toVector(vec, this.scalarBuilder), toMatrix(mat, this.scalarBuilder)],
      expected: anyOf(...results),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param vecs array of inputs to try for the vector input
   * @param mats array of inputs to try for the matrix input
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating a vector of acceptance
   *            intervals
   */
  public generateVectorMatrixToVectorCases(
    vecs: ROArrayArray<number>,
    mats: ROArrayArrayArray<number>,
    filter: IntervalFilter,
    ...ops: VectorMatrixToVector[]
  ): Case[] {
    // Cannot use cartesianProduct here, due to heterogeneous types
    const cases: Case[] = [];
    vecs.forEach(vec => {
      mats.forEach(mat => {
        const c = this.makeVectorMatrixToVectorCase(vec, mat, filter, ...ops);
        if (c !== undefined) {
          cases.push(c);
        }
      });
    });
    return cases;
  }

  // Framework - Intervals

  /**
   * Converts a point to an acceptance interval, using a specific function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   * op.extrema is invoked before this point in the call stack.
   * op.domain is tested before this point in the call stack.
   *
   * @param n value to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  private roundAndFlushScalarToInterval(n: number, op: ScalarToIntervalOp) {
    assert(!Number.isNaN(n), `flush not defined for NaN`);
    const values = this.correctlyRounded(n);
    const inputs = this.addFlushedIfNeeded(values);

    if (op.domain !== undefined) {
      // Cannot invoke op.domain() directly in the .some, because the narrowing doesn't propegate.
      const domain = op.domain();
      if (inputs.some(i => !domain.contains(i))) {
        return this.constants().unboundedInterval;
      }
    }

    const results = new Set<FPInterval>(inputs.map(op.impl));
    return this.spanIntervals(...results);
  }

  /**
   * Converts a pair to an acceptance interval, using a specific function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   * All unique combinations of x & y are run.
   * op.extrema is invoked before this point in the call stack.
   * op.domain is tested before this point in the call stack.
   *
   * @param x first param to flush & round then invoke op.impl on
   * @param y second param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  private roundAndFlushScalarPairToInterval(
    x: number,
    y: number,
    op: ScalarPairToIntervalOp
  ): FPInterval {
    assert(!Number.isNaN(x), `flush not defined for NaN`);
    assert(!Number.isNaN(y), `flush not defined for NaN`);

    const x_values = this.correctlyRounded(x);
    const y_values = this.correctlyRounded(y);
    const x_inputs = this.addFlushedIfNeeded(x_values);
    const y_inputs = this.addFlushedIfNeeded(y_values);

    if (op.domain !== undefined) {
      // Cannot invoke op.domain() directly in the .some, because the narrowing doesn't propegate.
      const domain = op.domain();

      if (x_inputs.some(i => !domain.x.some(e => e.contains(i)))) {
        return this.constants().unboundedInterval;
      }

      if (y_inputs.some(j => !domain.y.some(e => e.contains(j)))) {
        return this.constants().unboundedInterval;
      }
    }

    const intervals = new Set<FPInterval>();
    x_inputs.forEach(inner_x => {
      y_inputs.forEach(inner_y => {
        intervals.add(op.impl(inner_x, inner_y));
      });
    });
    return this.spanIntervals(...intervals);
  }

  /**
   * Converts a triplet to an acceptance interval, using a specific function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   * All unique combinations of x, y & z are run.
   *
   * @param x first param to flush & round then invoke op.impl on
   * @param y second param to flush & round then invoke op.impl on
   * @param z third param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  private roundAndFlushScalarTripleToInterval(
    x: number,
    y: number,
    z: number,
    op: ScalarTripleToIntervalOp
  ): FPInterval {
    assert(!Number.isNaN(x), `flush not defined for NaN`);
    assert(!Number.isNaN(y), `flush not defined for NaN`);
    assert(!Number.isNaN(z), `flush not defined for NaN`);
    const x_values = this.correctlyRounded(x);
    const y_values = this.correctlyRounded(y);
    const z_values = this.correctlyRounded(z);
    const x_inputs = this.addFlushedIfNeeded(x_values);
    const y_inputs = this.addFlushedIfNeeded(y_values);
    const z_inputs = this.addFlushedIfNeeded(z_values);
    const intervals = new Set<FPInterval>();
    // prettier-ignore
    x_inputs.forEach(inner_x => {
      y_inputs.forEach(inner_y => {
        z_inputs.forEach(inner_z => {
          intervals.add(op.impl(inner_x, inner_y, inner_z));
        });
      });
    });

    return this.spanIntervals(...intervals);
  }

  /**
   * Converts a vector to an acceptance interval using a specific function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   *
   * @param x param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  private roundAndFlushVectorToInterval(x: readonly number[], op: VectorToIntervalOp): FPInterval {
    assert(
      x.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );

    const x_rounded: ROArrayArray<number> = x.map(this.correctlyRounded);
    const x_flushed: ROArrayArray<number> = x_rounded.map(this.addFlushedIfNeeded.bind(this));
    const x_inputs = cartesianProduct<number>(...x_flushed);

    const intervals = new Set<FPInterval>();
    x_inputs.forEach(inner_x => {
      intervals.add(op.impl(inner_x));
    });
    return this.spanIntervals(...intervals);
  }

  /**
   * Converts a pair of vectors to an acceptance interval using a specific
   * function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   * All unique combinations of x & y are run.
   *
   * @param x first param to flush & round then invoke op.impl on
   * @param y second param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  private roundAndFlushVectorPairToInterval(
    x: readonly number[],
    y: readonly number[],
    op: VectorPairToIntervalOp
  ): FPInterval {
    assert(
      x.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );
    assert(
      y.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );

    const x_rounded: ROArrayArray<number> = x.map(this.correctlyRounded);
    const y_rounded: ROArrayArray<number> = y.map(this.correctlyRounded);
    const x_flushed: ROArrayArray<number> = x_rounded.map(this.addFlushedIfNeeded.bind(this));
    const y_flushed: ROArrayArray<number> = y_rounded.map(this.addFlushedIfNeeded.bind(this));
    const x_inputs = cartesianProduct<number>(...x_flushed);
    const y_inputs = cartesianProduct<number>(...y_flushed);

    const intervals = new Set<FPInterval>();
    x_inputs.forEach(inner_x => {
      y_inputs.forEach(inner_y => {
        intervals.add(op.impl(inner_x, inner_y));
      });
    });
    return this.spanIntervals(...intervals);
  }

  /**
   * Converts a vector to a vector of acceptance intervals using a specific
   * function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   *
   * @param x param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a vector of spans for each outputs of op.impl
   */
  private roundAndFlushVectorToVector(x: readonly number[], op: VectorToVectorOp): FPVector {
    assert(
      x.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );

    const x_rounded: ROArrayArray<number> = x.map(this.correctlyRounded);
    const x_flushed: ROArrayArray<number> = x_rounded.map(this.addFlushedIfNeeded.bind(this));
    const x_inputs = cartesianProduct<number>(...x_flushed);

    const interval_vectors = new Set<FPVector>();
    x_inputs.forEach(inner_x => {
      interval_vectors.add(op.impl(inner_x));
    });

    return this.spanVectors(...interval_vectors);
  }

  /**
   * Converts a pair of vectors to a vector of acceptance intervals using a
   * specific function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   *
   * @param x first param to flush & round then invoke op.impl on
   * @param y second param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a vector of spans for each output of op.impl
   */
  private roundAndFlushVectorPairToVector(
    x: readonly number[],
    y: readonly number[],
    op: VectorPairToVectorOp
  ): FPVector {
    assert(
      x.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );
    assert(
      y.every(e => !Number.isNaN(e)),
      `flush not defined for NaN`
    );

    const x_rounded: ROArrayArray<number> = x.map(this.correctlyRounded);
    const y_rounded: ROArrayArray<number> = y.map(this.correctlyRounded);
    const x_flushed: ROArrayArray<number> = x_rounded.map(this.addFlushedIfNeeded.bind(this));
    const y_flushed: ROArrayArray<number> = y_rounded.map(this.addFlushedIfNeeded.bind(this));
    const x_inputs = cartesianProduct<number>(...x_flushed);
    const y_inputs = cartesianProduct<number>(...y_flushed);

    const interval_vectors = new Set<FPVector>();
    x_inputs.forEach(inner_x => {
      y_inputs.forEach(inner_y => {
        interval_vectors.add(op.impl(inner_x, inner_y));
      });
    });

    return this.spanVectors(...interval_vectors);
  }

  /**
   * Converts a matrix to a matrix of acceptance intervals using a specific
   * function
   *
   * This handles correctly rounding and flushing inputs as needed.
   * Duplicate inputs are pruned before invoking op.impl.
   *
   * @param m param to flush & round then invoke op.impl on
   * @param op operation defining the function being run
   * @returns a matrix of spans for each outputs of op.impl
   */
  private roundAndFlushMatrixToMatrix(m: Array2D<number>, op: MatrixToMatrixOp): FPMatrix {
    const num_cols = m.length;
    const num_rows = m[0].length;
    assert(
      m.every(c => c.every(r => !Number.isNaN(r))),
      `flush not defined for NaN`
    );

    const m_flat = flatten2DArray(m);
    const m_rounded: ROArrayArray<number> = m_flat.map(this.correctlyRounded);
    const m_flushed: ROArrayArray<number> = m_rounded.map(this.addFlushedIfNeeded.bind(this));
    const m_options: ROArrayArray<number> = cartesianProduct<number>(...m_flushed);
    const m_inputs: ROArrayArrayArray<number> = m_options.map(e =>
      unflatten2DArray(e, num_cols, num_rows)
    );

    const interval_matrices = new Set<FPMatrix>();
    m_inputs.forEach(inner_m => {
      interval_matrices.add(op.impl(inner_m));
    });

    return this.spanMatrices(...interval_matrices);
  }

  /**
   * Calculate the acceptance interval for a unary function over an interval
   *
   * If the interval is actually a point, this just decays to
   * roundAndFlushScalarToInterval.
   *
   * The provided domain interval may be adjusted if the operation defines an
   * extrema function.
   *
   * @param x input domain interval
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  protected runScalarToIntervalOp(x: FPInterval, op: ScalarToIntervalOp): FPInterval {
    if (!x.isFinite()) {
      return this.constants().unboundedInterval;
    }

    if (op.extrema !== undefined) {
      x = op.extrema(x);
    }

    const result = this.spanIntervals(
      ...x.endpoints().map(b => this.roundAndFlushScalarToInterval(b, op))
    );
    return result.isFinite() ? result : this.constants().unboundedInterval;
  }

  /**
   * Calculate the acceptance interval for a binary function over an interval
   *
   * The provided domain intervals may be adjusted if the operation defines an
   * extrema function.
   *
   * @param x first input domain interval
   * @param y second input domain interval
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  protected runScalarPairToIntervalOp(
    x: FPInterval,
    y: FPInterval,
    op: ScalarPairToIntervalOp
  ): FPInterval {
    if (!x.isFinite() || !y.isFinite()) {
      return this.constants().unboundedInterval;
    }

    if (op.extrema !== undefined) {
      [x, y] = op.extrema(x, y);
    }

    const outputs = new Set<FPInterval>();
    x.endpoints().forEach(inner_x => {
      y.endpoints().forEach(inner_y => {
        outputs.add(this.roundAndFlushScalarPairToInterval(inner_x, inner_y, op));
      });
    });

    const result = this.spanIntervals(...outputs);
    return result.isFinite() ? result : this.constants().unboundedInterval;
  }

  /**
   * Calculate the acceptance interval for a ternary function over an interval
   *
   * @param x first input domain interval
   * @param y second input domain interval
   * @param z third input domain interval
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  protected runScalarTripleToIntervalOp(
    x: FPInterval,
    y: FPInterval,
    z: FPInterval,
    op: ScalarTripleToIntervalOp
  ): FPInterval {
    if (!x.isFinite() || !y.isFinite() || !z.isFinite()) {
      return this.constants().unboundedInterval;
    }

    const outputs = new Set<FPInterval>();
    x.endpoints().forEach(inner_x => {
      y.endpoints().forEach(inner_y => {
        z.endpoints().forEach(inner_z => {
          outputs.add(this.roundAndFlushScalarTripleToInterval(inner_x, inner_y, inner_z, op));
        });
      });
    });

    const result = this.spanIntervals(...outputs);
    return result.isFinite() ? result : this.constants().unboundedInterval;
  }

  /**
   * Calculate the acceptance interval for a vector function over given
   * intervals
   *
   * @param x input domain intervals vector
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  protected runVectorToIntervalOp(x: FPVector, op: VectorToIntervalOp): FPInterval {
    if (x.some(e => !e.isFinite())) {
      return this.constants().unboundedInterval;
    }

    const x_values = cartesianProduct<number>(...x.map(e => e.endpoints()));

    const outputs = new Set<FPInterval>();
    x_values.forEach(inner_x => {
      outputs.add(this.roundAndFlushVectorToInterval(inner_x, op));
    });

    const result = this.spanIntervals(...outputs);
    return result.isFinite() ? result : this.constants().unboundedInterval;
  }

  /**
   * Calculate the acceptance interval for a vector pair function over given
   * intervals
   *
   * @param x first input domain intervals vector
   * @param y second input domain intervals vector
   * @param op operation defining the function being run
   * @returns a span over all the outputs of op.impl
   */
  protected runVectorPairToIntervalOp(
    x: FPVector,
    y: FPVector,
    op: VectorPairToIntervalOp
  ): FPInterval {
    if (x.some(e => !e.isFinite()) || y.some(e => !e.isFinite())) {
      return this.constants().unboundedInterval;
    }

    const x_values = cartesianProduct<number>(...x.map(e => e.endpoints()));
    const y_values = cartesianProduct<number>(...y.map(e => e.endpoints()));

    const outputs = new Set<FPInterval>();
    x_values.forEach(inner_x => {
      y_values.forEach(inner_y => {
        outputs.add(this.roundAndFlushVectorPairToInterval(inner_x, inner_y, op));
      });
    });

    const result = this.spanIntervals(...outputs);
    return result.isFinite() ? result : this.constants().unboundedInterval;
  }

  /**
   * Calculate the vector of acceptance intervals for a pair of vector function
   * over given intervals
   *
   * @param x input domain intervals vector
   * @param op operation defining the function being run
   * @returns a vector of spans over all the outputs of op.impl
   */
  protected runVectorToVectorOp(x: FPVector, op: VectorToVectorOp): FPVector {
    if (x.some(e => !e.isFinite())) {
      return this.constants().unboundedVector[x.length];
    }

    const x_values = cartesianProduct<number>(...x.map(e => e.endpoints()));

    const outputs = new Set<FPVector>();
    x_values.forEach(inner_x => {
      outputs.add(this.roundAndFlushVectorToVector(inner_x, op));
    });

    const result = this.spanVectors(...outputs);
    return result.every(e => e.isFinite())
      ? result
      : this.constants().unboundedVector[result.length];
  }

  /**
   * Calculate the vector of acceptance intervals by running a scalar operation
   * component-wise over a vector.
   *
   * This is used for situations where a component-wise operation, like vector
   * negation, is needed as part of an inherited accuracy, but the top-level
   * operation test don't require an explicit vector definition of the function,
   * due to the generated 'vectorize' tests being sufficient.
   *
   * @param x input domain intervals vector
   * @param op scalar operation to be run component-wise
   * @returns a vector of intervals with the outputs of op.impl
   */
  protected runScalarToIntervalOpComponentWise(x: FPVector, op: ScalarToIntervalOp): FPVector {
    return this.toVector(x.map(e => this.runScalarToIntervalOp(e, op)));
  }

  /**
   * Calculate the vector of acceptance intervals for a vector function over
   * given intervals
   *
   * @param x first input domain intervals vector
   * @param y second input domain intervals vector
   * @param op operation defining the function being run
   * @returns a vector of spans over all the outputs of op.impl
   */
  protected runVectorPairToVectorOp(x: FPVector, y: FPVector, op: VectorPairToVectorOp): FPVector {
    if (x.some(e => !e.isFinite()) || y.some(e => !e.isFinite())) {
      return this.constants().unboundedVector[x.length];
    }

    const x_values = cartesianProduct<number>(...x.map(e => e.endpoints()));
    const y_values = cartesianProduct<number>(...y.map(e => e.endpoints()));

    const outputs = new Set<FPVector>();
    x_values.forEach(inner_x => {
      y_values.forEach(inner_y => {
        outputs.add(this.roundAndFlushVectorPairToVector(inner_x, inner_y, op));
      });
    });

    const result = this.spanVectors(...outputs);
    return result.every(e => e.isFinite())
      ? result
      : this.constants().unboundedVector[result.length];
  }

  /**
   * Calculate the vector of acceptance intervals by running a scalar operation
   * component-wise over a pair of vectors.
   *
   * This is used for situations where a component-wise operation, like vector
   * subtraction, is needed as part of an inherited accuracy, but the top-level
   * operation test don't require an explicit vector definition of the function,
   * due to the generated 'vectorize' tests being sufficient.
   *
   * @param x first input domain intervals vector
   * @param y second input domain intervals vector
   * @param op scalar operation to be run component-wise
   * @returns a vector of intervals with the outputs of op.impl
   */
  protected runScalarPairToIntervalOpVectorComponentWise(
    x: FPVector,
    y: FPVector,
    op: ScalarPairToIntervalOp
  ): FPVector {
    assert(
      x.length === y.length,
      `runScalarPairToIntervalOpVectorComponentWise requires vectors of the same dimensions`
    );

    return this.toVector(
      x.map((i, idx) => {
        return this.runScalarPairToIntervalOp(i, y[idx], op);
      })
    );
  }

  /**
   * Calculate the matrix of acceptance intervals for a pair of matrix function over
   * given intervals
   *
   * @param m input domain intervals matrix
   * @param op operation defining the function being run
   * @returns a matrix of spans over all the outputs of op.impl
   */
  protected runMatrixToMatrixOp(m: FPMatrix, op: MatrixToMatrixOp): FPMatrix {
    const num_cols = m.length;
    const num_rows = m[0].length;

    // Do not check for OOB inputs and exit early here, because the shape of
    // the output matrix may be determined by the operation being run,
    // i.e. transpose.

    const m_flat: readonly FPInterval[] = flatten2DArray(m);
    const m_values: ROArrayArray<number> = cartesianProduct<number>(
      ...m_flat.map(e => e.endpoints())
    );

    const outputs = new Set<FPMatrix>();
    m_values.forEach(inner_m => {
      const unflat_m = unflatten2DArray(inner_m, num_cols, num_rows);
      outputs.add(this.roundAndFlushMatrixToMatrix(unflat_m, op));
    });

    const result = this.spanMatrices(...outputs);
    const result_cols = result.length;
    const result_rows = result[0].length;

    // FPMatrix has to be coerced to ROArrayArray<FPInterval> to use .every. This should
    // always be safe, since FPMatrix are defined as fixed length array of
    // arrays.
    return (result as ROArrayArray<FPInterval>).every(c => c.every(r => r.isFinite()))
      ? result
      : this.constants().unboundedMatrix[result_cols][result_rows];
  }

  /**
   * Calculate the Matrix of acceptance intervals by running a scalar operation
   * component-wise over a scalar and a matrix.
   *
   * An example of this is performing constant scaling.
   *
   * @param i scalar  input
   * @param m matrix input
   * @param op scalar operation to be run component-wise
   * @returns a matrix of intervals with the outputs of op.impl
   */
  protected runScalarPairToIntervalOpScalarMatrixComponentWise(
    i: FPInterval,
    m: FPMatrix,
    op: ScalarPairToIntervalOp
  ): FPMatrix {
    const cols = m.length;
    const rows = m[0].length;
    return this.toMatrix(
      unflatten2DArray(
        flatten2DArray(m).map(e => this.runScalarPairToIntervalOp(i, e, op)),
        cols,
        rows
      )
    );
  }

  /**
   * Calculate the Matrix of acceptance intervals by running a scalar operation
   * component-wise over a pair of matrices.
   *
   * An example of this is performing matrix addition.
   *
   * @param x first input domain intervals matrix
   * @param y second input domain intervals matrix
   * @param op scalar operation to be run component-wise
   * @returns a matrix of intervals with the outputs of op.impl
   */
  protected runScalarPairToIntervalOpMatrixMatrixComponentWise(
    x: FPMatrix,
    y: FPMatrix,
    op: ScalarPairToIntervalOp
  ): FPMatrix {
    assert(
      x.length === y.length && x[0].length === y[0].length,
      `runScalarPairToIntervalOpMatrixMatrixComponentWise requires matrices of the same dimensions`
    );

    const cols = x.length;
    const rows = x[0].length;
    const flat_x = flatten2DArray(x);
    const flat_y = flatten2DArray(y);

    return this.toMatrix(
      unflatten2DArray(
        flat_x.map((i, idx) => {
          return this.runScalarPairToIntervalOp(i, flat_y[idx], op);
        }),
        cols,
        rows
      )
    );
  }

  // API - Fundamental Error Intervals

  /** @returns a ScalarToIntervalOp for [n - error_range, n + error_range] */
  private AbsoluteErrorIntervalOp(error_range: number): ScalarToIntervalOp {
    const op: ScalarToIntervalOp = {
      impl: (_: number) => {
        return this.constants().unboundedInterval;
      },
    };

    assert(
      error_range >= 0,
      `absoluteErrorInterval must have non-negative error range, get ${error_range}`
    );

    if (this.isFinite(error_range)) {
      op.impl = (n: number) => {
        assert(!Number.isNaN(n), `absolute error not defined for NaN`);
        // Return anyInterval if given center n is infinity.
        if (!this.isFinite(n)) {
          return this.constants().unboundedInterval;
        }
        return this.toInterval([n - error_range, n + error_range]);
      };
    }

    return op;
  }

  protected absoluteErrorIntervalImpl(n: number, error_range: number): FPInterval {
    error_range = Math.abs(error_range);
    return this.runScalarToIntervalOp(
      this.toInterval(n),
      this.AbsoluteErrorIntervalOp(error_range)
    );
  }

  /** @returns an interval of the absolute error around the point */
  public abstract readonly absoluteErrorInterval: (n: number, error_range: number) => FPInterval;

  /**
   * Defines a ScalarToIntervalOp for an interval of the correctly rounded values
   * around the point
   */
  private readonly CorrectlyRoundedIntervalOp: ScalarToIntervalOp = {
    impl: (n: number) => {
      assert(!Number.isNaN(n), `absolute not defined for NaN`);
      return this.toInterval(n);
    },
  };

  protected correctlyRoundedIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.CorrectlyRoundedIntervalOp);
  }

  /** @returns an interval of the correctly rounded values around the point */
  public abstract readonly correctlyRoundedInterval: (n: number | FPInterval) => FPInterval;

  protected correctlyRoundedMatrixImpl(m: Array2D<number>): FPMatrix {
    return this.toMatrix(map2DArray(m, this.correctlyRoundedInterval));
  }

  /** @returns a matrix of correctly rounded intervals for the provided matrix */
  public abstract readonly correctlyRoundedMatrix: (m: Array2D<number>) => FPMatrix;

  /** @returns a ScalarToIntervalOp for [n - numULP * ULP(n), n + numULP * ULP(n)] */
  private ULPIntervalOp(numULP: number): ScalarToIntervalOp {
    const op: ScalarToIntervalOp = {
      impl: (_: number) => {
        return this.constants().unboundedInterval;
      },
    };

    if (this.isFinite(numULP)) {
      op.impl = (n: number) => {
        assert(!Number.isNaN(n), `ULP error not defined for NaN`);

        const ulp = this.oneULP(n);
        const begin = n - numULP * ulp;
        const end = n + numULP * ulp;

        return this.toInterval([
          Math.min(begin, this.flushSubnormal(begin)),
          Math.max(end, this.flushSubnormal(end)),
        ]);
      };
    }

    return op;
  }

  protected ulpIntervalImpl(n: number, numULP: number): FPInterval {
    numULP = Math.abs(numULP);
    return this.runScalarToIntervalOp(this.toInterval(n), this.ULPIntervalOp(numULP));
  }

  /** @returns an interval of N * ULP around the point */
  public abstract readonly ulpInterval: (n: number, numULP: number) => FPInterval;

  // API - Acceptance Intervals

  private readonly AbsIntervalOp: ScalarToIntervalOp = {
    impl: (n: number) => {
      return this.correctlyRoundedInterval(Math.abs(n));
    },
  };

  protected absIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AbsIntervalOp);
  }

  /** Calculate an acceptance interval for abs(n) */
  public abstract readonly absInterval: (n: number | FPInterval) => FPInterval;

  // This op is implemented differently for f32 and f16.
  private readonly AcosIntervalOp: ScalarToIntervalOp = {
    impl: (n: number) => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      // acos(n) = atan2(sqrt(1.0 - n * n), n) or a polynomial approximation with absolute error
      const y = this.sqrtInterval(this.subtractionInterval(1, this.multiplicationInterval(n, n)));
      const approx_abs_error = this.kind === 'f32' ? 6.77e-5 : 3.91e-3;
      return this.spanIntervals(
        this.atan2Interval(y, n),
        this.absoluteErrorInterval(Math.acos(n), approx_abs_error)
      );
    },
    domain: () => {
      return this.constants().negOneToOneInterval;
    },
  };

  protected acosIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AcosIntervalOp);
  }

  /** Calculate an acceptance interval for acos(n) */
  public abstract readonly acosInterval: (n: number) => FPInterval;

  private readonly AcoshAlternativeIntervalOp: ScalarToIntervalOp = {
    impl: (x: number): FPInterval => {
      // acosh(x) = log(x + sqrt((x + 1.0f) * (x - 1.0)))
      const inner_value = this.multiplicationInterval(
        this.additionInterval(x, 1.0),
        this.subtractionInterval(x, 1.0)
      );
      const sqrt_value = this.sqrtInterval(inner_value);
      return this.logInterval(this.additionInterval(x, sqrt_value));
    },
  };

  protected acoshAlternativeIntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.AcoshAlternativeIntervalOp);
  }

  /** Calculate an acceptance interval of acosh(x) using log(x + sqrt((x + 1.0f) * (x - 1.0))) */
  public abstract readonly acoshAlternativeInterval: (x: number | FPInterval) => FPInterval;

  private readonly AcoshPrimaryIntervalOp: ScalarToIntervalOp = {
    impl: (x: number): FPInterval => {
      // acosh(x) = log(x + sqrt(x * x - 1.0))
      const inner_value = this.subtractionInterval(this.multiplicationInterval(x, x), 1.0);
      const sqrt_value = this.sqrtInterval(inner_value);
      return this.logInterval(this.additionInterval(x, sqrt_value));
    },
  };

  protected acoshPrimaryIntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.AcoshPrimaryIntervalOp);
  }

  /** Calculate an acceptance interval of acosh(x) using log(x + sqrt(x * x - 1.0)) */
  protected abstract acoshPrimaryInterval: (x: number | FPInterval) => FPInterval;

  /** All acceptance interval functions for acosh(x) */
  public abstract readonly acoshIntervals: ScalarToInterval[];

  private readonly AdditionIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      return this.correctlyRoundedInterval(x + y);
    },
  };

  protected additionIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.AdditionIntervalOp
    );
  }

  /** Calculate an acceptance interval of x + y, when x and y are both scalars */
  public abstract readonly additionInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  protected additionMatrixMatrixIntervalImpl(x: Array2D<number>, y: Array2D<number>): FPMatrix {
    return this.runScalarPairToIntervalOpMatrixMatrixComponentWise(
      this.toMatrix(x),
      this.toMatrix(y),
      this.AdditionIntervalOp
    );
  }

  /** Calculate an acceptance interval of x + y, when x and y are matrices */
  public abstract readonly additionMatrixMatrixInterval: (
    x: Array2D<number>,
    y: Array2D<number>
  ) => FPMatrix;

  // This op is implemented differently for f32 and f16.
  private readonly AsinIntervalOp: ScalarToIntervalOp = {
    impl: (n: number) => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      // asin(n) = atan2(n, sqrt(1.0 - n * n)) or a polynomial approximation with absolute error
      const x = this.sqrtInterval(this.subtractionInterval(1, this.multiplicationInterval(n, n)));
      const approx_abs_error = this.kind === 'f32' ? 6.81e-5 : 3.91e-3;
      return this.spanIntervals(
        this.atan2Interval(n, x),
        this.absoluteErrorInterval(Math.asin(n), approx_abs_error)
      );
    },
    domain: () => {
      return this.constants().negOneToOneInterval;
    },
  };

  /** Calculate an acceptance interval for asin(n) */
  protected asinIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AsinIntervalOp);
  }

  /** Calculate an acceptance interval for asin(n) */
  public abstract readonly asinInterval: (n: number) => FPInterval;

  private readonly AsinhIntervalOp: ScalarToIntervalOp = {
    impl: (x: number): FPInterval => {
      // asinh(x) = log(x + sqrt(x * x + 1.0))
      const inner_value = this.additionInterval(this.multiplicationInterval(x, x), 1.0);
      const sqrt_value = this.sqrtInterval(inner_value);
      return this.logInterval(this.additionInterval(x, sqrt_value));
    },
  };

  protected asinhIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AsinhIntervalOp);
  }

  /** Calculate an acceptance interval of asinh(x) */
  public abstract readonly asinhInterval: (n: number) => FPInterval;

  private readonly AtanIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const ulp_error = this.kind === 'f32' ? 4096 : 5;
      return this.ulpInterval(Math.atan(n), ulp_error);
    },
  };

  /** Calculate an acceptance interval of atan(x) */
  protected atanIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AtanIntervalOp);
  }

  /** Calculate an acceptance interval of atan(x) */
  public abstract readonly atanInterval: (n: number | FPInterval) => FPInterval;

  // This op is implemented differently for f32 and f16.
  private Atan2IntervalOpBuilder(): ScalarPairToIntervalOp {
    assert(this.kind === 'f32' || this.kind === 'f16');
    const constants = this.constants();
    // For atan2, the params are labelled (y, x), not (x, y), so domain.x is first parameter (y),
    // and domain.y is the second parameter (x).
    // The first param must be finite and normal.
    const domain_x = [
      this.toInterval([constants.negative.min, constants.negative.max]),
      this.toInterval([constants.positive.min, constants.positive.max]),
    ];
    // inherited from division
    const domain_y =
      this.kind === 'f32'
        ? [this.toInterval([-(2 ** 126), -(2 ** -126)]), this.toInterval([2 ** -126, 2 ** 126])]
        : [this.toInterval([-(2 ** 14), -(2 ** -14)]), this.toInterval([2 ** -14, 2 ** 14])];
    const ulp_error = this.kind === 'f32' ? 4096 : 5;
    return {
      impl: (y: number, x: number): FPInterval => {
        // Accurate result in f64
        let atan_yx = Math.atan(y / x);
        // Offset by +/-pi according to the definition. Use pi value in f64 because we are
        // handling accurate result.
        if (x < 0) {
          // x < 0, y > 0, result is atan(y/x) + π
          if (y > 0) {
            atan_yx = atan_yx + kValue.f64.positive.pi.whole;
          } else {
            // x < 0, y < 0, result is atan(y/x) - π
            atan_yx = atan_yx - kValue.f64.positive.pi.whole;
          }
        }

        return this.ulpInterval(atan_yx, ulp_error);
      },
      extrema: (y: FPInterval, x: FPInterval): [FPInterval, FPInterval] => {
        // There is discontinuity, which generates an unbounded result, at y/x = 0 that will dominate the accuracy
        if (y.contains(0)) {
          if (x.contains(0)) {
            return [this.toInterval(0), this.toInterval(0)];
          }
          return [this.toInterval(0), x];
        }
        return [y, x];
      },
      domain: () => {
        return { x: domain_x, y: domain_y };
      },
    };
  }

  protected atan2IntervalImpl(y: number | FPInterval, x: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(y),
      this.toInterval(x),
      this.Atan2IntervalOpBuilder()
    );
  }

  /** Calculate an acceptance interval of atan2(y, x) */
  public abstract readonly atan2Interval: (
    y: number | FPInterval,
    x: number | FPInterval
  ) => FPInterval;

  private readonly AtanhIntervalOp: ScalarToIntervalOp = {
    impl: (n: number) => {
      // atanh(x) = log((1.0 + x) / (1.0 - x)) * 0.5
      const numerator = this.additionInterval(1.0, n);
      const denominator = this.subtractionInterval(1.0, n);
      const log_interval = this.logInterval(this.divisionInterval(numerator, denominator));
      return this.multiplicationInterval(log_interval, 0.5);
    },
  };

  protected atanhIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.AtanhIntervalOp);
  }

  /** Calculate an acceptance interval of atanh(x) */
  public abstract readonly atanhInterval: (n: number) => FPInterval;

  private readonly CeilIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.correctlyRoundedInterval(Math.ceil(n));
    },
  };

  protected ceilIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.CeilIntervalOp);
  }

  /** Calculate an acceptance interval of ceil(x) */
  public abstract readonly ceilInterval: (n: number) => FPInterval;

  private readonly ClampMedianIntervalOp: ScalarTripleToIntervalOp = {
    impl: (x: number, y: number, z: number): FPInterval => {
      return this.correctlyRoundedInterval(
        // Default sort is string sort, so have to implement numeric comparison.
        // Cannot use the b-a one-liner, because that assumes no infinities.
        [x, y, z].sort((a, b) => {
          if (a < b) {
            return -1;
          }
          if (a > b) {
            return 1;
          }
          return 0;
        })[1]
      );
    },
  };

  protected clampMedianIntervalImpl(
    x: number | FPInterval,
    y: number | FPInterval,
    z: number | FPInterval
  ): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.toInterval(z),
      this.ClampMedianIntervalOp
    );
  }

  /** Calculate an acceptance interval of clamp(x, y, z) via median(x, y, z) */
  public abstract readonly clampMedianInterval: (
    x: number | FPInterval,
    y: number | FPInterval,
    z: number | FPInterval
  ) => FPInterval;

  private readonly ClampMinMaxIntervalOp: ScalarTripleToIntervalOp = {
    impl: (x: number, low: number, high: number): FPInterval => {
      return this.minInterval(this.maxInterval(x, low), high);
    },
  };

  protected clampMinMaxIntervalImpl(
    x: number | FPInterval,
    low: number | FPInterval,
    high: number | FPInterval
  ): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(x),
      this.toInterval(low),
      this.toInterval(high),
      this.ClampMinMaxIntervalOp
    );
  }

  /** Calculate an acceptance interval of clamp(x, high, low) via min(max(x, low), high) */
  public abstract readonly clampMinMaxInterval: (
    x: number | FPInterval,
    low: number | FPInterval,
    high: number | FPInterval
  ) => FPInterval;

  /** All acceptance interval functions for clamp(x, y, z) */
  public abstract readonly clampIntervals: ScalarTripleToInterval[];

  private readonly CosIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const abs_error = this.kind === 'f32' ? 2 ** -11 : 2 ** -7;
      return this.absoluteErrorInterval(Math.cos(n), abs_error);
    },
    domain: () => {
      return this.constants().negPiToPiInterval;
    },
  };

  protected cosIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.CosIntervalOp);
  }

  /** Calculate an acceptance interval of cos(x) */
  public abstract readonly cosInterval: (n: number) => FPInterval;

  private readonly CoshIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      // cosh(x) = (exp(x) + exp(-x)) * 0.5
      const minus_n = this.negationInterval(n);
      return this.multiplicationInterval(
        this.additionInterval(this.expInterval(n), this.expInterval(minus_n)),
        0.5
      );
    },
  };

  protected coshIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.CoshIntervalOp);
  }

  /** Calculate an acceptance interval of cosh(x) */
  public abstract readonly coshInterval: (n: number) => FPInterval;

  private readonly CrossIntervalOp: VectorPairToVectorOp = {
    impl: (x: readonly number[], y: readonly number[]): FPVector => {
      assert(x.length === 3, `CrossIntervalOp received x with ${x.length} instead of 3`);
      assert(y.length === 3, `CrossIntervalOp received y with ${y.length} instead of 3`);

      // cross(x, y) = r, where
      //   r[0] = x[1] * y[2] - x[2] * y[1]
      //   r[1] = x[2] * y[0] - x[0] * y[2]
      //   r[2] = x[0] * y[1] - x[1] * y[0]

      const r0 = this.subtractionInterval(
        this.multiplicationInterval(x[1], y[2]),
        this.multiplicationInterval(x[2], y[1])
      );
      const r1 = this.subtractionInterval(
        this.multiplicationInterval(x[2], y[0]),
        this.multiplicationInterval(x[0], y[2])
      );
      const r2 = this.subtractionInterval(
        this.multiplicationInterval(x[0], y[1]),
        this.multiplicationInterval(x[1], y[0])
      );

      if (r0.isFinite() && r1.isFinite() && r2.isFinite()) {
        return [r0, r1, r2];
      }
      return this.constants().unboundedVector[3];
    },
  };

  protected crossIntervalImpl(x: readonly number[], y: readonly number[]): FPVector {
    assert(x.length === 3, `Cross is only defined for vec3`);
    assert(y.length === 3, `Cross is only defined for vec3`);
    return this.runVectorPairToVectorOp(this.toVector(x), this.toVector(y), this.CrossIntervalOp);
  }

  /** Calculate a vector of acceptance intervals for cross(x, y) */
  public abstract readonly crossInterval: (x: readonly number[], y: readonly number[]) => FPVector;

  private readonly DegreesIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.multiplicationInterval(n, 57.295779513082322865);
    },
  };

  protected degreesIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.DegreesIntervalOp);
  }

  /** Calculate an acceptance interval of degrees(x) */
  public abstract readonly degreesInterval: (n: number) => FPInterval;

  /**
   * Calculate the minor of a NxN matrix.
   *
   * The ijth minor of a square matrix, is the N-1xN-1 matrix created by removing
   * the ith column and jth row from the original matrix.
   */
  private minorNxN(m: Array2D<number>, col: number, row: number): Array2D<number> {
    const dim = m.length;
    assert(m.length === m[0].length, `minorMatrix is only defined for square matrices`);
    assert(col >= 0 && col < dim, `col ${col} needs be in [0, # of columns '${dim}')`);
    assert(row >= 0 && row < dim, `row ${row} needs be in [0, # of rows '${dim}')`);

    const result: number[][] = [...Array(dim - 1)].map(_ => [...Array(dim - 1)]);

    const col_indices: readonly number[] = [...Array(dim).keys()].filter(e => e !== col);
    const row_indices: readonly number[] = [...Array(dim).keys()].filter(e => e !== row);

    col_indices.forEach((c, i) => {
      row_indices.forEach((r, j) => {
        result[i][j] = m[c][r];
      });
    });
    return result;
  }

  /** Calculate an acceptance interval for determinant(m), where m is a 2x2 matrix */
  private determinant2x2Interval(m: Array2D<number>): FPInterval {
    assert(
      m.length === m[0].length && m.length === 2,
      `determinant2x2Interval called on non-2x2 matrix`
    );
    return this.subtractionInterval(
      this.multiplicationInterval(m[0][0], m[1][1]),
      this.multiplicationInterval(m[0][1], m[1][0])
    );
  }

  /** Calculate an acceptance interval for determinant(m), where m is a 3x3 matrix */
  private determinant3x3Interval(m: Array2D<number>): FPInterval {
    assert(
      m.length === m[0].length && m.length === 3,
      `determinant3x3Interval called on non-3x3 matrix`
    );

    // M is a 3x3 matrix
    // det(M) is A + B + C, where A, B, C are three elements in a row/column times
    // their own co-factor.
    // (The co-factor is the determinant of the minor of that position with the
    // appropriate +/-)
    // For simplicity sake A, B, C are calculated as the elements of the first
    // column
    const A = this.multiplicationInterval(
      m[0][0],
      this.determinant2x2Interval(this.minorNxN(m, 0, 0))
    );
    const B = this.multiplicationInterval(
      -m[0][1],
      this.determinant2x2Interval(this.minorNxN(m, 0, 1))
    );
    const C = this.multiplicationInterval(
      m[0][2],
      this.determinant2x2Interval(this.minorNxN(m, 0, 2))
    );

    // Need to calculate permutations, since for fp addition is not associative,
    // so A + B + C is not guaranteed to equal B + C + A, etc.
    const permutations: ROArrayArray<FPInterval> = calculatePermutations([A, B, C]);
    return this.spanIntervals(
      ...permutations.map(p =>
        p.reduce((prev: FPInterval, cur: FPInterval) => this.additionInterval(prev, cur))
      )
    );
  }

  /** Calculate an acceptance interval for determinant(m), where m is a 4x4 matrix */
  private determinant4x4Interval(m: Array2D<number>): FPInterval {
    assert(
      m.length === m[0].length && m.length === 4,
      `determinant3x3Interval called on non-4x4 matrix`
    );

    // M is a 4x4 matrix
    // det(M) is A + B + C + D, where A, B, C, D are four elements in a row/column
    // times their own co-factor.
    // (The co-factor is the determinant of the minor of that position with the
    // appropriate +/-)
    // For simplicity sake A, B, C, D are calculated as the elements of the
    // first column
    const A = this.multiplicationInterval(
      m[0][0],
      this.determinant3x3Interval(this.minorNxN(m, 0, 0))
    );
    const B = this.multiplicationInterval(
      -m[0][1],
      this.determinant3x3Interval(this.minorNxN(m, 0, 1))
    );
    const C = this.multiplicationInterval(
      m[0][2],
      this.determinant3x3Interval(this.minorNxN(m, 0, 2))
    );
    const D = this.multiplicationInterval(
      -m[0][3],
      this.determinant3x3Interval(this.minorNxN(m, 0, 3))
    );

    // Need to calculate permutations, since for fp addition is not associative
    // so A + B + C + D is not guaranteed to equal B + C + A + D, etc.
    const permutations: ROArrayArray<FPInterval> = calculatePermutations([A, B, C, D]);
    return this.spanIntervals(
      ...permutations.map(p =>
        p.reduce((prev: FPInterval, cur: FPInterval) => this.additionInterval(prev, cur))
      )
    );
  }

  /**
   * This code calculates 3x3 and 4x4 determinants using the textbook co-factor
   * method, using the first column for the co-factor selection.
   *
   * For matrices composed of integer elements, e, with |e|^4 < 2**21, this
   * should be fine.
   *
   * For e, where e is subnormal or 4*(e^4) might not be precisely expressible as
   * a f32 values, this approach breaks down, because the rule of all co-factor
   * definitions of determinant being equal doesn't hold in these cases.
   *
   * The general solution for this is to calculate all the permutations of the
   * operations in the worked out formula for determinant.
   * For 3x3 this is tractable, but for 4x4 this works out to ~23! permutations
   * that need to be calculated.
   * Thus, CTS testing and the spec definition of accuracy is restricted to the
   * space that the simple implementation is valid.
   */
  protected determinantIntervalImpl(x: Array2D<number>): FPInterval {
    const dim = x.length;
    assert(
      x[0].length === dim && (dim === 2 || dim === 3 || dim === 4),
      `determinantInterval only defined for 2x2, 3x3 and 4x4 matrices`
    );
    switch (dim) {
      case 2:
        return this.determinant2x2Interval(x);
      case 3:
        return this.determinant3x3Interval(x);
      case 4:
        return this.determinant4x4Interval(x);
    }
    unreachable(
      "determinantInterval called on x, where which has an unexpected dimension of '${dim}'"
    );
  }

  /** Calculate an acceptance interval for determinant(x) */
  public abstract readonly determinantInterval: (x: Array2D<number>) => FPInterval;

  private readonly DistanceIntervalScalarOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      return this.lengthInterval(this.subtractionInterval(x, y));
    },
  };

  private readonly DistanceIntervalVectorOp: VectorPairToIntervalOp = {
    impl: (x: readonly number[], y: readonly number[]): FPInterval => {
      return this.lengthInterval(
        this.runScalarPairToIntervalOpVectorComponentWise(
          this.toVector(x),
          this.toVector(y),
          this.SubtractionIntervalOp
        )
      );
    },
  };

  protected distanceIntervalImpl(
    x: number | readonly number[],
    y: number | readonly number[]
  ): FPInterval {
    if (x instanceof Array && y instanceof Array) {
      assert(
        x.length === y.length,
        `distanceInterval requires both params to have the same number of elements`
      );
      return this.runVectorPairToIntervalOp(
        this.toVector(x),
        this.toVector(y),
        this.DistanceIntervalVectorOp
      );
    } else if (!(x instanceof Array) && !(y instanceof Array)) {
      return this.runScalarPairToIntervalOp(
        this.toInterval(x),
        this.toInterval(y),
        this.DistanceIntervalScalarOp
      );
    }
    unreachable(
      `distanceInterval requires both params to both the same type, either scalars or vectors`
    );
  }

  /** Calculate an acceptance interval of distance(x, y) */
  public abstract readonly distanceInterval: (
    x: number | readonly number[],
    y: number | readonly number[]
  ) => FPInterval;

  // This op is implemented differently for f32 and f16.
  private DivisionIntervalOpBuilder(): ScalarPairToIntervalOp {
    const constants = this.constants();
    const domain_x = [this.toInterval([constants.negative.min, constants.positive.max])];
    const domain_y =
      this.kind === 'f32' || this.kind === 'abstract'
        ? [this.toInterval([-(2 ** 126), -(2 ** -126)]), this.toInterval([2 ** -126, 2 ** 126])]
        : [this.toInterval([-(2 ** 14), -(2 ** -14)]), this.toInterval([2 ** -14, 2 ** 14])];
    return {
      impl: (x: number, y: number): FPInterval => {
        if (y === 0) {
          return constants.unboundedInterval;
        }
        return this.ulpInterval(x / y, 2.5);
      },
      extrema: (x: FPInterval, y: FPInterval): [FPInterval, FPInterval] => {
        // division has a discontinuity at y = 0.
        if (y.contains(0)) {
          y = this.toInterval(0);
        }
        return [x, y];
      },
      domain: () => {
        return { x: domain_x, y: domain_y };
      },
    };
  }

  protected divisionIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.DivisionIntervalOpBuilder()
    );
  }

  /** Calculate an acceptance interval of x / y */
  public abstract readonly divisionInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  private readonly DotIntervalOp: VectorPairToIntervalOp = {
    impl: (x: readonly number[], y: readonly number[]): FPInterval => {
      // dot(x, y) = sum of x[i] * y[i]
      const multiplications = this.runScalarPairToIntervalOpVectorComponentWise(
        this.toVector(x),
        this.toVector(y),
        this.MultiplicationIntervalOp
      );

      // vec2 doesn't require permutations, since a + b = b + a for floats
      if (multiplications.length === 2) {
        return this.additionInterval(multiplications[0], multiplications[1]);
      }

      // The spec does not state the ordering of summation, so all the
      // permutations are calculated and their results spanned, since addition
      // of more than two floats is not transitive, i.e. a + b + c is not
      // guaranteed to equal b + a + c
      const permutations: ROArrayArray<FPInterval> = calculatePermutations(multiplications);
      return this.spanIntervals(
        ...permutations.map(p => p.reduce((prev, cur) => this.additionInterval(prev, cur)))
      );
    },
  };

  protected dotIntervalImpl(
    x: readonly number[] | readonly FPInterval[],
    y: readonly number[] | readonly FPInterval[]
  ): FPInterval {
    assert(
      x.length === y.length,
      `dot not defined for vectors with different lengths, x = ${x}, y = ${y}`
    );
    return this.runVectorPairToIntervalOp(this.toVector(x), this.toVector(y), this.DotIntervalOp);
  }

  /** Calculated the acceptance interval for dot(x, y) */
  public abstract readonly dotInterval: (
    x: readonly number[] | readonly FPInterval[],
    y: readonly number[] | readonly FPInterval[]
  ) => FPInterval;

  private readonly ExpIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const ulp_error = this.kind === 'f32' ? 3 + 2 * Math.abs(n) : 1 + 2 * Math.abs(n);
      return this.ulpInterval(Math.exp(n), ulp_error);
    },
  };

  protected expIntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.ExpIntervalOp);
  }

  /** Calculate an acceptance interval for exp(x) */
  public abstract readonly expInterval: (x: number | FPInterval) => FPInterval;

  private readonly Exp2IntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const ulp_error = this.kind === 'f32' ? 3 + 2 * Math.abs(n) : 1 + 2 * Math.abs(n);
      return this.ulpInterval(Math.pow(2, n), ulp_error);
    },
  };

  protected exp2IntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.Exp2IntervalOp);
  }

  /** Calculate an acceptance interval for exp2(x) */
  public abstract readonly exp2Interval: (x: number | FPInterval) => FPInterval;

  /**
   * faceForward(x, y, z) = select(-x, x, dot(z, y) < 0.0)
   *
   * This builtin selects from two discrete results (delta rounding/flushing),
   * so the majority of the framework code is not appropriate, since the
   * framework attempts to span results.
   *
   * Thus, a bespoke implementation is used instead of
   * defining an Op and running that through the framework.
   */
  protected faceForwardIntervalsImpl(
    x: readonly number[],
    y: readonly number[],
    z: readonly number[]
  ): (FPVector | undefined)[] {
    const x_vec = this.toVector(x);
    // Running vector through this.runScalarToIntervalOpComponentWise to make
    // sure that flushing/rounding is handled, since toVector does not perform
    // those operations.
    const positive_x = this.runScalarToIntervalOpComponentWise(x_vec, {
      impl: (i: number): FPInterval => {
        return this.toInterval(i);
      },
    });
    const negative_x = this.runScalarToIntervalOpComponentWise(x_vec, this.NegationIntervalOp);

    const dot_interval = this.dotInterval(z, y);

    const results: (FPVector | undefined)[] = [];

    if (!dot_interval.isFinite()) {
      // dot calculation went out of bounds
      // Inserting undefined in the result, so that the test running framework
      // is aware of this potential OOB.
      // For const-eval tests, it means that the test case should be skipped,
      // since the shader will fail to compile.
      // For non-const-eval the undefined should be stripped out of the possible
      // results.

      results.push(undefined);
    }

    // Because the result of dot can be an interval, it might span across 0, thus
    // it is possible that both -x and x are valid responses.
    if (dot_interval.begin < 0 || dot_interval.end < 0) {
      results.push(positive_x);
    }

    if (dot_interval.begin >= 0 || dot_interval.end >= 0) {
      results.push(negative_x);
    }

    assert(
      results.length > 0 || results.every(r => r === undefined),
      `faceForwardInterval selected neither positive x or negative x for the result, this shouldn't be possible`
    );
    return results;
  }

  /** Calculate the acceptance intervals for faceForward(x, y, z) */
  public abstract readonly faceForwardIntervals: (
    x: readonly number[],
    y: readonly number[],
    z: readonly number[]
  ) => (FPVector | undefined)[];

  private readonly FloorIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.correctlyRoundedInterval(Math.floor(n));
    },
  };

  protected floorIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.FloorIntervalOp);
  }

  /** Calculate an acceptance interval of floor(x) */
  public abstract readonly floorInterval: (n: number) => FPInterval;

  private readonly FmaIntervalOp: ScalarTripleToIntervalOp = {
    impl: (x: number, y: number, z: number): FPInterval => {
      return this.additionInterval(this.multiplicationInterval(x, y), z);
    },
  };

  protected fmaIntervalImpl(x: number, y: number, z: number): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.toInterval(z),
      this.FmaIntervalOp
    );
  }

  /** Calculate an acceptance interval for fma(x, y, z) */
  public abstract readonly fmaInterval: (x: number, y: number, z: number) => FPInterval;

  private readonly FractIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      // fract(x) = x - floor(x) is defined in the spec.
      // For people coming from a non-graphics background this will cause some
      // unintuitive results. For example,
      // fract(-1.1) is not 0.1 or -0.1, but instead 0.9.
      // This is how other shading languages operate and allows for a desirable
      // wrap around in graphics programming.
      const result = this.subtractionInterval(n, this.floorInterval(n));
      assert(
        // negative.subnormal.min instead of 0, because FTZ can occur
        // selectively during the calculation
        this.toInterval([this.constants().negative.subnormal.min, 1.0]).contains(result),
        `fract(${n}) interval [${result}] unexpectedly extends beyond [~0.0, 1.0]`
      );
      if (result.contains(1)) {
        // Very small negative numbers can lead to catastrophic cancellation,
        // thus calculating a fract of 1.0, which is technically not a
        // fractional part, so some implementations clamp the result to next
        // nearest number.
        return this.spanIntervals(result, this.toInterval(this.constants().positive.less_than_one));
      }
      return result;
    },
  };

  protected fractIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.FractIntervalOp);
  }

  /** Calculate an acceptance interval of fract(x) */
  public abstract readonly fractInterval: (n: number) => FPInterval;

  private readonly InverseSqrtIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.ulpInterval(1 / Math.sqrt(n), 2);
    },
    domain: () => {
      return this.constants().greaterThanZeroInterval;
    },
  };

  protected inverseSqrtIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.InverseSqrtIntervalOp);
  }

  /** Calculate an acceptance interval of inverseSqrt(x) */
  public abstract readonly inverseSqrtInterval: (n: number | FPInterval) => FPInterval;

  private readonly LdexpIntervalOp: ScalarPairToIntervalOp = {
    impl: (e1: number, e2: number) => {
      assert(Number.isInteger(e2), 'the second param of ldexp must be an integer');
      // Spec explicitly calls indeterminate value if e2 > bias + 1
      if (e2 > this.constants().bias + 1) {
        return this.constants().unboundedInterval;
      }
      // The spec says the result of ldexp(e1, e2) = e1 * 2 ^ e2, and the
      // accuracy is correctly rounded to the true value, so the inheritance
      // framework does not need to be invoked to determine endpoints.
      // Instead, the value at a higher precision is calculated and passed to
      // correctlyRoundedInterval.
      const result = e1 * 2 ** e2;
      if (!Number.isFinite(result)) {
        // Overflowed TS's number type, so definitely out of bounds
        return this.constants().unboundedInterval;
      }
      // The result may be zero if e2 + bias <= 0, but we can't simply span the interval to 0.0.
      // For example, for f32 input e1 = 2**120 and e2 = -130, e2 + bias = -3 <= 0, but
      // e1 * 2 ** e2 = 2**-10, so the valid result is 2**-10 or 0.0, instead of [0.0, 2**-10].
      // Always return the correctly-rounded interval, and special examination should be taken when
      // using the result.
      return this.correctlyRoundedInterval(result);
    },
  };

  protected ldexpIntervalImpl(e1: number, e2: number): FPInterval {
    // Only round and flush e1, as e2 is of integer type (i32 or abstract integer) and should be
    // precise.
    return this.roundAndFlushScalarToInterval(e1, {
      impl: (e1: number) => this.LdexpIntervalOp.impl(e1, e2),
    });
  }

  /**
   * Calculate an acceptance interval of ldexp(e1, e2), where e2 is integer
   *
   * Spec indicate that the result may be zero if e2 + bias <= 0, no matter how large
   * was e1 * 2 ** e2, i.e. the actual valid result is correctlyRounded(e1 * 2 ** e2) or 0.0, if
   * e2 + bias <= 0. Such discontinious flush-to-zero behavior is hard to be expressed using
   * FPInterval, therefore in the situation of e2 + bias <= 0 the returned interval would be just
   * correctlyRounded(e1 * 2 ** e2), and special examination should be taken when using the result.
   *
   */
  public abstract readonly ldexpInterval: (e1: number, e2: number) => FPInterval;

  private readonly LengthIntervalScalarOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.sqrtInterval(this.multiplicationInterval(n, n));
    },
  };

  private readonly LengthIntervalVectorOp: VectorToIntervalOp = {
    impl: (n: readonly number[]): FPInterval => {
      return this.sqrtInterval(this.dotInterval(n, n));
    },
  };

  protected lengthIntervalImpl(n: number | FPInterval | readonly number[] | FPVector): FPInterval {
    if (n instanceof Array) {
      return this.runVectorToIntervalOp(this.toVector(n), this.LengthIntervalVectorOp);
    } else {
      return this.runScalarToIntervalOp(this.toInterval(n), this.LengthIntervalScalarOp);
    }
  }

  /** Calculate an acceptance interval of length(x) */
  public abstract readonly lengthInterval: (
    n: number | FPInterval | readonly number[] | FPVector
  ) => FPInterval;

  private readonly LogIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const abs_error = this.kind === 'f32' ? 2 ** -21 : 2 ** -7;
      if (n >= 0.5 && n <= 2.0) {
        return this.absoluteErrorInterval(Math.log(n), abs_error);
      }
      return this.ulpInterval(Math.log(n), 3);
    },
    domain: () => {
      return this.constants().greaterThanZeroInterval;
    },
  };

  protected logIntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.LogIntervalOp);
  }

  /** Calculate an acceptance interval of log(x) */
  public abstract readonly logInterval: (x: number | FPInterval) => FPInterval;

  private readonly Log2IntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const abs_error = this.kind === 'f32' ? 2 ** -21 : 2 ** -7;
      if (n >= 0.5 && n <= 2.0) {
        return this.absoluteErrorInterval(Math.log2(n), abs_error);
      }
      return this.ulpInterval(Math.log2(n), 3);
    },
    domain: () => {
      return this.constants().greaterThanZeroInterval;
    },
  };

  protected log2IntervalImpl(x: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(x), this.Log2IntervalOp);
  }

  /** Calculate an acceptance interval of log2(x) */
  public abstract readonly log2Interval: (x: number | FPInterval) => FPInterval;

  private readonly MaxIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      // If both of the inputs are subnormal, then either of the inputs can be returned
      if (this.isSubnormal(x) && this.isSubnormal(y)) {
        return this.correctlyRoundedInterval(
          this.spanIntervals(this.toInterval(x), this.toInterval(y))
        );
      }

      return this.correctlyRoundedInterval(Math.max(x, y));
    },
  };

  protected maxIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.MaxIntervalOp
    );
  }

  /** Calculate an acceptance interval of max(x, y) */
  public abstract readonly maxInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  private readonly MinIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      // If both of the inputs are subnormal, then either of the inputs can be returned
      if (this.isSubnormal(x) && this.isSubnormal(y)) {
        return this.correctlyRoundedInterval(
          this.spanIntervals(this.toInterval(x), this.toInterval(y))
        );
      }

      return this.correctlyRoundedInterval(Math.min(x, y));
    },
  };

  protected minIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.MinIntervalOp
    );
  }

  /** Calculate an acceptance interval of min(x, y) */
  public abstract readonly minInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  private readonly MixImpreciseIntervalOp: ScalarTripleToIntervalOp = {
    impl: (x: number, y: number, z: number): FPInterval => {
      // x + (y - x) * z =
      //  x + t, where t = (y - x) * z
      const t = this.multiplicationInterval(this.subtractionInterval(y, x), z);
      return this.additionInterval(x, t);
    },
  };

  protected mixImpreciseIntervalImpl(x: number, y: number, z: number): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.toInterval(z),
      this.MixImpreciseIntervalOp
    );
  }

  /** Calculate an acceptance interval of mix(x, y, z) using x + (y - x) * z */
  public abstract readonly mixImpreciseInterval: (x: number, y: number, z: number) => FPInterval;

  private readonly MixPreciseIntervalOp: ScalarTripleToIntervalOp = {
    impl: (x: number, y: number, z: number): FPInterval => {
      // x * (1.0 - z) + y * z =
      //   t + s, where t = x * (1.0 - z), s = y * z
      const t = this.multiplicationInterval(x, this.subtractionInterval(1.0, z));
      const s = this.multiplicationInterval(y, z);
      return this.additionInterval(t, s);
    },
  };

  protected mixPreciseIntervalImpl(x: number, y: number, z: number): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.toInterval(z),
      this.MixPreciseIntervalOp
    );
  }

  /** Calculate an acceptance interval of mix(x, y, z) using x * (1.0 - z) + y * z */
  public abstract readonly mixPreciseInterval: (x: number, y: number, z: number) => FPInterval;

  /** All acceptance interval functions for mix(x, y, z) */
  public abstract readonly mixIntervals: ScalarTripleToInterval[];

  protected modfIntervalImpl(n: number): { fract: FPInterval; whole: FPInterval } {
    const fract = this.correctlyRoundedInterval(n % 1.0);
    const whole = this.correctlyRoundedInterval(n - (n % 1.0));
    return { fract, whole };
  }

  /** Calculate an acceptance interval of modf(x) */
  public abstract readonly modfInterval: (n: number) => { fract: FPInterval; whole: FPInterval };

  private readonly MultiplicationInnerOp = {
    impl: (x: number, y: number): FPInterval => {
      return this.correctlyRoundedInterval(x * y);
    },
  };

  private readonly MultiplicationIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      return this.roundAndFlushScalarPairToInterval(x, y, this.MultiplicationInnerOp);
    },
  };

  protected multiplicationIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.MultiplicationIntervalOp
    );
  }

  /** Calculate an acceptance interval of x * y */
  public abstract readonly multiplicationInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  /**
   * @returns the vector result of multiplying the given vector by the given
   *          scalar
   */
  private multiplyVectorByScalar(v: readonly number[], c: number | FPInterval): FPVector {
    return this.toVector(v.map(x => this.multiplicationInterval(x, c)));
  }

  protected multiplicationMatrixScalarIntervalImpl(mat: Array2D<number>, scalar: number): FPMatrix {
    return this.runScalarPairToIntervalOpScalarMatrixComponentWise(
      this.toInterval(scalar),
      this.toMatrix(mat),
      this.MultiplicationIntervalOp
    );
  }

  /** Calculate an acceptance interval of x * y, when x is a matrix and y is a scalar */
  public abstract readonly multiplicationMatrixScalarInterval: (
    mat: Array2D<number>,
    scalar: number
  ) => FPMatrix;

  protected multiplicationScalarMatrixIntervalImpl(scalar: number, mat: Array2D<number>): FPMatrix {
    return this.multiplicationMatrixScalarInterval(mat, scalar);
  }

  /** Calculate an acceptance interval of x * y, when x is a scalar and y is a matrix */
  public abstract readonly multiplicationScalarMatrixInterval: (
    scalar: number,
    mat: Array2D<number>
  ) => FPMatrix;

  protected multiplicationMatrixMatrixIntervalImpl(
    mat_x: Array2D<number>,
    mat_y: Array2D<number>
  ): FPMatrix {
    const x_cols = mat_x.length;
    const x_rows = mat_x[0].length;
    const y_cols = mat_y.length;
    const y_rows = mat_y[0].length;
    assert(x_cols === y_rows, `'mat${x_cols}x${x_rows} * mat${y_cols}x${y_rows}' is not defined`);

    const x_transposed = this.transposeInterval(mat_x);

    let oob_result: boolean = false;
    const result: FPInterval[][] = [...Array(y_cols)].map(_ => [...Array(x_rows)]);
    mat_y.forEach((y, i) => {
      x_transposed.forEach((x, j) => {
        result[i][j] = this.dotInterval(x, y);
        if (!oob_result && !result[i][j].isFinite()) {
          oob_result = true;
        }
      });
    });

    if (oob_result) {
      return this.constants().unboundedMatrix[result.length as 2 | 3 | 4][
        result[0].length as 2 | 3 | 4
      ];
    }
    return result as ROArrayArray<FPInterval> as FPMatrix;
  }

  /** Calculate an acceptance interval of x * y, when x is a matrix and y is a matrix */
  public abstract readonly multiplicationMatrixMatrixInterval: (
    mat_x: Array2D<number>,
    mat_y: Array2D<number>
  ) => FPMatrix;

  protected multiplicationMatrixVectorIntervalImpl(
    x: Array2D<number>,
    y: readonly number[]
  ): FPVector {
    const cols = x.length;
    const rows = x[0].length;
    assert(y.length === cols, `'mat${cols}x${rows} * vec${y.length}' is not defined`);

    return this.transposeInterval(x).map(e => this.dotInterval(e, y)) as FPVector;
  }

  /** Calculate an acceptance interval of x * y, when x is a matrix and y is a vector */
  public abstract readonly multiplicationMatrixVectorInterval: (
    x: Array2D<number>,
    y: readonly number[]
  ) => FPVector;

  protected multiplicationVectorMatrixIntervalImpl(
    x: readonly number[],
    y: Array2D<number>
  ): FPVector {
    const cols = y.length;
    const rows = y[0].length;
    assert(x.length === rows, `'vec${x.length} * mat${cols}x${rows}' is not defined`);

    return y.map(e => this.dotInterval(x, e)) as FPVector;
  }

  /** Calculate an acceptance interval of x * y, when x is a vector and y is a matrix */
  public abstract readonly multiplicationVectorMatrixInterval: (
    x: readonly number[],
    y: Array2D<number>
  ) => FPVector;

  private readonly NegationIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.correctlyRoundedInterval(-n);
    },
  };

  protected negationIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.NegationIntervalOp);
  }

  /** Calculate an acceptance interval of -x */
  public abstract readonly negationInterval: (n: number) => FPInterval;

  private readonly NormalizeIntervalOp: VectorToVectorOp = {
    impl: (n: readonly number[]): FPVector => {
      const length = this.lengthInterval(n);
      const result = this.toVector(n.map(e => this.divisionInterval(e, length)));
      if (result.some(r => !r.isFinite())) {
        return this.constants().unboundedVector[result.length];
      }
      return result;
    },
  };

  protected normalizeIntervalImpl(n: readonly number[]): FPVector {
    return this.runVectorToVectorOp(this.toVector(n), this.NormalizeIntervalOp);
  }

  public abstract readonly normalizeInterval: (n: readonly number[]) => FPVector;

  private readonly PowIntervalOp: ScalarPairToIntervalOp = {
    // pow(x, y) has no explicit domain restrictions, but inherits the x <= 0
    // domain restriction from log2(x). Invoking log2Interval(x) in impl will
    // enforce this, so there is no need to wrap the impl call here.
    impl: (x: number, y: number): FPInterval => {
      return this.exp2Interval(this.multiplicationInterval(y, this.log2Interval(x)));
    },
  };

  protected powIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.PowIntervalOp
    );
  }

  /** Calculate an acceptance interval of pow(x, y) */
  public abstract readonly powInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  private readonly RadiansIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.multiplicationInterval(n, 0.017453292519943295474);
    },
  };

  protected radiansIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.RadiansIntervalOp);
  }

  /** Calculate an acceptance interval of radians(x) */
  public abstract readonly radiansInterval: (n: number) => FPInterval;

  private readonly ReflectIntervalOp: VectorPairToVectorOp = {
    impl: (x: readonly number[], y: readonly number[]): FPVector => {
      assert(
        x.length === y.length,
        `ReflectIntervalOp received x (${x}) and y (${y}) with different numbers of elements`
      );

      // reflect(x, y) = x - 2.0 * dot(x, y) * y
      //               = x - t * y, t = 2.0 * dot(x, y)
      // x = incident vector
      // y = normal of reflecting surface
      const t = this.multiplicationInterval(2.0, this.dotInterval(x, y));
      const rhs = this.multiplyVectorByScalar(y, t);
      const result = this.runScalarPairToIntervalOpVectorComponentWise(
        this.toVector(x),
        rhs,
        this.SubtractionIntervalOp
      );

      if (result.some(r => !r.isFinite())) {
        return this.constants().unboundedVector[result.length];
      }
      return result;
    },
  };

  protected reflectIntervalImpl(x: readonly number[], y: readonly number[]): FPVector {
    assert(
      x.length === y.length,
      `reflect is only defined for vectors with the same number of elements`
    );
    return this.runVectorPairToVectorOp(this.toVector(x), this.toVector(y), this.ReflectIntervalOp);
  }

  /** Calculate an acceptance interval of reflect(x, y) */
  public abstract readonly reflectInterval: (
    x: readonly number[],
    y: readonly number[]
  ) => FPVector;

  /**
   * refract is a singular function in the sense that it is the only builtin that
   * takes in (FPVector, FPVector, F32/F16) and returns FPVector and is basically
   * defined in terms of other functions.
   *
   * Instead of implementing all the framework code to integrate it with its
   * own operation type, etc, it instead has a bespoke implementation that is a
   * composition of other builtin functions that use the framework.
   */
  protected refractIntervalImpl(i: readonly number[], s: readonly number[], r: number): FPVector {
    assert(
      i.length === s.length,
      `refract is only defined for vectors with the same number of elements`
    );

    const r_squared = this.multiplicationInterval(r, r);
    const dot = this.dotInterval(s, i);
    const dot_squared = this.multiplicationInterval(dot, dot);
    const one_minus_dot_squared = this.subtractionInterval(1, dot_squared);
    const k = this.subtractionInterval(
      1.0,
      this.multiplicationInterval(r_squared, one_minus_dot_squared)
    );

    if (!k.isFinite() || k.containsZeroOrSubnormals()) {
      // There is a discontinuity at k == 0, due to sqrt(k) being calculated, so exiting early
      return this.constants().unboundedVector[this.toVector(i).length];
    }

    if (k.end < 0.0) {
      // if k is negative, then the zero vector is the valid response
      return this.constants().zeroVector[this.toVector(i).length];
    }

    const dot_times_r = this.multiplicationInterval(dot, r);
    const k_sqrt = this.sqrtInterval(k);
    const t = this.additionInterval(dot_times_r, k_sqrt); // t = r * dot(i, s) + sqrt(k)

    const result = this.runScalarPairToIntervalOpVectorComponentWise(
      this.multiplyVectorByScalar(i, r),
      this.multiplyVectorByScalar(s, t),
      this.SubtractionIntervalOp
    ); // (i * r) - (s * t)

    if (result.some(r => !r.isFinite())) {
      return this.constants().unboundedVector[result.length];
    }
    return result;
  }

  /** Calculate acceptance interval vectors of reflect(i, s, r) */
  public abstract readonly refractInterval: (
    i: readonly number[],
    s: readonly number[],
    r: number
  ) => FPVector;

  private readonly RemainderIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      // x % y = x - y * trunc(x/y)
      return this.subtractionInterval(
        x,
        this.multiplicationInterval(y, this.truncInterval(this.divisionInterval(x, y)))
      );
    },
  };

  /** Calculate an acceptance interval for x % y */
  protected remainderIntervalImpl(x: number, y: number): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.RemainderIntervalOp
    );
  }

  /** Calculate an acceptance interval for x % y */
  public abstract readonly remainderInterval: (x: number, y: number) => FPInterval;

  private readonly RoundIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      const k = Math.floor(n);
      const diff_before = n - k;
      const diff_after = k + 1 - n;
      if (diff_before < diff_after) {
        return this.correctlyRoundedInterval(k);
      } else if (diff_before > diff_after) {
        return this.correctlyRoundedInterval(k + 1);
      }

      // n is in the middle of two integers.
      // The tie breaking rule is 'k if k is even, k + 1 if k is odd'
      if (k % 2 === 0) {
        return this.correctlyRoundedInterval(k);
      }
      return this.correctlyRoundedInterval(k + 1);
    },
  };

  protected roundIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.RoundIntervalOp);
  }

  /** Calculate an acceptance interval of round(x) */
  public abstract readonly roundInterval: (n: number) => FPInterval;

  /**
   * The definition of saturate does not specify which version of clamp to use.
   * Using min-max here, since it has wider acceptance intervals, that include
   * all of median's.
   */
  protected saturateIntervalImpl(n: number): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(n),
      this.toInterval(0.0),
      this.toInterval(1.0),
      this.ClampMinMaxIntervalOp
    );
  }

  /*** Calculate an acceptance interval of saturate(n) as clamp(n, 0.0, 1.0) */
  public abstract readonly saturateInterval: (n: number) => FPInterval;

  private readonly SignIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      if (n > 0.0) {
        return this.correctlyRoundedInterval(1.0);
      }
      if (n < 0.0) {
        return this.correctlyRoundedInterval(-1.0);
      }

      return this.correctlyRoundedInterval(0.0);
    },
  };

  protected signIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.SignIntervalOp);
  }

  /** Calculate an acceptance interval of sign(x) */
  public abstract readonly signInterval: (n: number) => FPInterval;

  private readonly SinIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      assert(this.kind === 'f32' || this.kind === 'f16');
      const abs_error = this.kind === 'f32' ? 2 ** -11 : 2 ** -7;
      return this.absoluteErrorInterval(Math.sin(n), abs_error);
    },
    domain: () => {
      return this.constants().negPiToPiInterval;
    },
  };

  protected sinIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.SinIntervalOp);
  }

  /** Calculate an acceptance interval of sin(x) */
  public abstract readonly sinInterval: (n: number) => FPInterval;

  private readonly SinhIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      // sinh(x) = (exp(x) - exp(-x)) * 0.5
      const minus_n = this.negationInterval(n);
      return this.multiplicationInterval(
        this.subtractionInterval(this.expInterval(n), this.expInterval(minus_n)),
        0.5
      );
    },
  };

  protected sinhIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.SinhIntervalOp);
  }

  /** Calculate an acceptance interval of sinh(x) */
  public abstract readonly sinhInterval: (n: number) => FPInterval;

  private readonly SmoothStepOp: ScalarTripleToIntervalOp = {
    impl: (low: number, high: number, x: number): FPInterval => {
      // For clamp(foo, 0.0, 1.0) the different implementations of clamp provide
      // the same value, so arbitrarily picking the minmax version to use.
      // t = clamp((x - low) / (high - low), 0.0, 1.0)
      // prettier-ignore
      const t = this.clampMedianInterval(
        this.divisionInterval(
          this.subtractionInterval(x, low),
          this.subtractionInterval(high, low)),
        0.0,
        1.0);
      // Inherited from t * t * (3.0 - 2.0 * t)
      // prettier-ignore
      return this.multiplicationInterval(
        t,
        this.multiplicationInterval(t,
          this.subtractionInterval(3.0,
            this.multiplicationInterval(2.0, t))));
    },
  };

  protected smoothStepIntervalImpl(low: number, high: number, x: number): FPInterval {
    return this.runScalarTripleToIntervalOp(
      this.toInterval(low),
      this.toInterval(high),
      this.toInterval(x),
      this.SmoothStepOp
    );
  }

  /** Calculate an acceptance interval of smoothStep(low, high, x) */
  public abstract readonly smoothStepInterval: (low: number, high: number, x: number) => FPInterval;

  private readonly SqrtIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.divisionInterval(1.0, this.inverseSqrtInterval(n));
    },
  };

  protected sqrtIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.SqrtIntervalOp);
  }

  /** Calculate an acceptance interval of sqrt(x) */
  public abstract readonly sqrtInterval: (n: number | FPInterval) => FPInterval;

  private readonly StepIntervalOp: ScalarPairToIntervalOp = {
    impl: (edge: number, x: number): FPInterval => {
      if (edge <= x) {
        return this.correctlyRoundedInterval(1.0);
      }
      return this.correctlyRoundedInterval(0.0);
    },
  };

  protected stepIntervalImpl(edge: number, x: number): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(edge),
      this.toInterval(x),
      this.StepIntervalOp
    );
  }

  /**
   * Calculate an acceptance 'interval' for step(edge, x)
   *
   * step only returns two possible values, so its interval requires special
   * interpretation in CTS tests.
   * This interval will be one of four values: [0, 0], [0, 1], [1, 1] & [-∞, +∞].
   * [0, 0] and [1, 1] indicate that the correct answer in point they encapsulate.
   * [0, 1] should not be treated as a span, i.e. 0.1 is acceptable, but instead
   * indicate either 0.0 or 1.0 are acceptable answers.
   * [-∞, +∞] is treated as unbounded interval, since an unbounded or
   * infinite value was passed in.
   */
  public abstract readonly stepInterval: (edge: number, x: number) => FPInterval;

  private readonly SubtractionIntervalOp: ScalarPairToIntervalOp = {
    impl: (x: number, y: number): FPInterval => {
      return this.correctlyRoundedInterval(x - y);
    },
  };

  protected subtractionIntervalImpl(x: number | FPInterval, y: number | FPInterval): FPInterval {
    return this.runScalarPairToIntervalOp(
      this.toInterval(x),
      this.toInterval(y),
      this.SubtractionIntervalOp
    );
  }

  /** Calculate an acceptance interval of x - y */
  public abstract readonly subtractionInterval: (
    x: number | FPInterval,
    y: number | FPInterval
  ) => FPInterval;

  protected subtractionMatrixMatrixIntervalImpl(x: Array2D<number>, y: Array2D<number>): FPMatrix {
    return this.runScalarPairToIntervalOpMatrixMatrixComponentWise(
      this.toMatrix(x),
      this.toMatrix(y),
      this.SubtractionIntervalOp
    );
  }

  /** Calculate an acceptance interval of x - y, when x and y are matrices */
  public abstract readonly subtractionMatrixMatrixInterval: (
    x: Array2D<number>,
    y: Array2D<number>
  ) => FPMatrix;

  private readonly TanIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.divisionInterval(this.sinInterval(n), this.cosInterval(n));
    },
  };

  protected tanIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.TanIntervalOp);
  }

  /** Calculate an acceptance interval of tan(x) */
  public abstract readonly tanInterval: (n: number) => FPInterval;

  private readonly TanhIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.divisionInterval(this.sinhInterval(n), this.coshInterval(n));
    },
  };

  protected tanhIntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.TanhIntervalOp);
  }

  /** Calculate an acceptance interval of tanh(x) */
  public abstract readonly tanhInterval: (n: number) => FPInterval;

  private readonly TransposeIntervalOp: MatrixToMatrixOp = {
    impl: (m: Array2D<number>): FPMatrix => {
      const num_cols = m.length;
      const num_rows = m[0].length;
      const result: FPInterval[][] = [...Array(num_rows)].map(_ => [...Array(num_cols)]);

      for (let i = 0; i < num_cols; i++) {
        for (let j = 0; j < num_rows; j++) {
          result[j][i] = this.correctlyRoundedInterval(m[i][j]);
        }
      }
      return this.toMatrix(result);
    },
  };

  protected transposeIntervalImpl(m: Array2D<number>): FPMatrix {
    return this.runMatrixToMatrixOp(this.toMatrix(m), this.TransposeIntervalOp);
  }

  /** Calculate an acceptance interval of transpose(m) */
  public abstract readonly transposeInterval: (m: Array2D<number>) => FPMatrix;

  private readonly TruncIntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      return this.correctlyRoundedInterval(Math.trunc(n));
    },
  };

  protected truncIntervalImpl(n: number | FPInterval): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.TruncIntervalOp);
  }

  /** Calculate an acceptance interval of trunc(x) */
  public abstract readonly truncInterval: (n: number | FPInterval) => FPInterval;
}

// Pre-defined values that get used multiple times in _constants' initializers. Cannot use FPTraits members, since this
// executes before they are defined.
const kF32UnboundedInterval = new FPInterval(
  'f32',
  Number.NEGATIVE_INFINITY,
  Number.POSITIVE_INFINITY
);
const kF32ZeroInterval = new FPInterval('f32', 0);

class F32Traits extends FPTraits {
  private static _constants: FPConstants = {
    positive: {
      min: kValue.f32.positive.min,
      max: kValue.f32.positive.max,
      infinity: kValue.f32.positive.infinity,
      nearest_max: kValue.f32.positive.nearest_max,
      less_than_one: kValue.f32.positive.less_than_one,
      subnormal: {
        min: kValue.f32.positive.subnormal.min,
        max: kValue.f32.positive.subnormal.max,
      },
      pi: {
        whole: kValue.f32.positive.pi.whole,
        three_quarters: kValue.f32.positive.pi.three_quarters,
        half: kValue.f32.positive.pi.half,
        third: kValue.f32.positive.pi.third,
        quarter: kValue.f32.positive.pi.quarter,
        sixth: kValue.f32.positive.pi.sixth,
      },
      e: kValue.f32.positive.e,
    },
    negative: {
      min: kValue.f32.negative.min,
      max: kValue.f32.negative.max,
      infinity: kValue.f32.negative.infinity,
      nearest_min: kValue.f32.negative.nearest_min,
      less_than_one: kValue.f32.negative.less_than_one,
      subnormal: {
        min: kValue.f32.negative.subnormal.min,
        max: kValue.f32.negative.subnormal.max,
      },
      pi: {
        whole: kValue.f32.negative.pi.whole,
        three_quarters: kValue.f32.negative.pi.three_quarters,
        half: kValue.f32.negative.pi.half,
        third: kValue.f32.negative.pi.third,
        quarter: kValue.f32.negative.pi.quarter,
        sixth: kValue.f32.negative.pi.sixth,
      },
    },
    bias: 127,
    unboundedInterval: kF32UnboundedInterval,
    zeroInterval: kF32ZeroInterval,
    // Have to use the constants.ts values here, because values defined in the
    // initializer cannot be referenced in the initializer
    negPiToPiInterval: new FPInterval(
      'f32',
      kValue.f32.negative.pi.whole,
      kValue.f32.positive.pi.whole
    ),
    greaterThanZeroInterval: new FPInterval(
      'f32',
      kValue.f32.positive.subnormal.min,
      kValue.f32.positive.max
    ),
    negOneToOneInterval: new FPInterval('f32', -1, 1),
    zeroVector: {
      2: [kF32ZeroInterval, kF32ZeroInterval],
      3: [kF32ZeroInterval, kF32ZeroInterval, kF32ZeroInterval],
      4: [kF32ZeroInterval, kF32ZeroInterval, kF32ZeroInterval, kF32ZeroInterval],
    },
    unboundedVector: {
      2: [kF32UnboundedInterval, kF32UnboundedInterval],
      3: [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
      4: [
        kF32UnboundedInterval,
        kF32UnboundedInterval,
        kF32UnboundedInterval,
        kF32UnboundedInterval,
      ],
    },
    unboundedMatrix: {
      2: {
        2: [
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        3: [
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        4: [
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
        ],
      },
      3: {
        2: [
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        3: [
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        4: [
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
        ],
      },
      4: {
        2: [
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        3: [
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
          [kF32UnboundedInterval, kF32UnboundedInterval, kF32UnboundedInterval],
        ],
        4: [
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
          [
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
            kF32UnboundedInterval,
          ],
        ],
      },
    },
  };

  public constructor() {
    super('f32');
  }

  public constants(): FPConstants {
    return F32Traits._constants;
  }

  // Utilities - Overrides
  public readonly quantize = quantizeToF32;
  public readonly correctlyRounded = correctlyRoundedF32;
  public readonly isFinite = isFiniteF32;
  public readonly isSubnormal = isSubnormalNumberF32;
  public readonly flushSubnormal = flushSubnormalNumberF32;
  public readonly oneULP = oneULPF32;
  public readonly scalarBuilder = f32;
  public readonly scalarRange = scalarF32Range;
  public readonly sparseScalarRange = sparseScalarF32Range;
  public readonly vectorRange = vectorF32Range;
  public readonly sparseVectorRange = sparseVectorF32Range;
  public readonly sparseMatrixRange = sparseMatrixF32Range;

  // Framework - Fundamental Error Intervals - Overrides
  public readonly absoluteErrorInterval = this.absoluteErrorIntervalImpl.bind(this);
  public readonly correctlyRoundedInterval = this.correctlyRoundedIntervalImpl.bind(this);
  public readonly correctlyRoundedMatrix = this.correctlyRoundedMatrixImpl.bind(this);
  public readonly ulpInterval = this.ulpIntervalImpl.bind(this);

  // Framework - API - Overrides
  public readonly absInterval = this.absIntervalImpl.bind(this);
  public readonly acosInterval = this.acosIntervalImpl.bind(this);
  public readonly acoshAlternativeInterval = this.acoshAlternativeIntervalImpl.bind(this);
  public readonly acoshPrimaryInterval = this.acoshPrimaryIntervalImpl.bind(this);
  public readonly acoshIntervals = [this.acoshAlternativeInterval, this.acoshPrimaryInterval];
  public readonly additionInterval = this.additionIntervalImpl.bind(this);
  public readonly additionMatrixMatrixInterval = this.additionMatrixMatrixIntervalImpl.bind(this);
  public readonly asinInterval = this.asinIntervalImpl.bind(this);
  public readonly asinhInterval = this.asinhIntervalImpl.bind(this);
  public readonly atanInterval = this.atanIntervalImpl.bind(this);
  public readonly atan2Interval = this.atan2IntervalImpl.bind(this);
  public readonly atanhInterval = this.atanhIntervalImpl.bind(this);
  public readonly ceilInterval = this.ceilIntervalImpl.bind(this);
  public readonly clampMedianInterval = this.clampMedianIntervalImpl.bind(this);
  public readonly clampMinMaxInterval = this.clampMinMaxIntervalImpl.bind(this);
  public readonly clampIntervals = [this.clampMedianInterval, this.clampMinMaxInterval];
  public readonly cosInterval = this.cosIntervalImpl.bind(this);
  public readonly coshInterval = this.coshIntervalImpl.bind(this);
  public readonly crossInterval = this.crossIntervalImpl.bind(this);
  public readonly degreesInterval = this.degreesIntervalImpl.bind(this);
  public readonly determinantInterval = this.determinantIntervalImpl.bind(this);
  public readonly distanceInterval = this.distanceIntervalImpl.bind(this);
  public readonly divisionInterval = this.divisionIntervalImpl.bind(this);
  public readonly dotInterval = this.dotIntervalImpl.bind(this);
  public readonly expInterval = this.expIntervalImpl.bind(this);
  public readonly exp2Interval = this.exp2IntervalImpl.bind(this);
  public readonly faceForwardIntervals = this.faceForwardIntervalsImpl.bind(this);
  public readonly floorInterval = this.floorIntervalImpl.bind(this);
  public readonly fmaInterval = this.fmaIntervalImpl.bind(this);
  public readonly fractInterval = this.fractIntervalImpl.bind(this);
  public readonly inverseSqrtInterval = this.inverseSqrtIntervalImpl.bind(this);
  public readonly ldexpInterval = this.ldexpIntervalImpl.bind(this);
  public readonly lengthInterval = this.lengthIntervalImpl.bind(this);
  public readonly logInterval = this.logIntervalImpl.bind(this);
  public readonly log2Interval = this.log2IntervalImpl.bind(this);
  public readonly maxInterval = this.maxIntervalImpl.bind(this);
  public readonly minInterval = this.minIntervalImpl.bind(this);
  public readonly mixImpreciseInterval = this.mixImpreciseIntervalImpl.bind(this);
  public readonly mixPreciseInterval = this.mixPreciseIntervalImpl.bind(this);
  public readonly mixIntervals = [this.mixImpreciseInterval, this.mixPreciseInterval];
  public readonly modfInterval = this.modfIntervalImpl.bind(this);
  public readonly multiplicationInterval = this.multiplicationIntervalImpl.bind(this);
  public readonly multiplicationMatrixMatrixInterval =
    this.multiplicationMatrixMatrixIntervalImpl.bind(this);
  public readonly multiplicationMatrixScalarInterval =
    this.multiplicationMatrixScalarIntervalImpl.bind(this);
  public readonly multiplicationScalarMatrixInterval =
    this.multiplicationScalarMatrixIntervalImpl.bind(this);
  public readonly multiplicationMatrixVectorInterval =
    this.multiplicationMatrixVectorIntervalImpl.bind(this);
  public readonly multiplicationVectorMatrixInterval =
    this.multiplicationVectorMatrixIntervalImpl.bind(this);
  public readonly negationInterval = this.negationIntervalImpl.bind(this);
  public readonly normalizeInterval = this.normalizeIntervalImpl.bind(this);
  public readonly powInterval = this.powIntervalImpl.bind(this);
  public readonly radiansInterval = this.radiansIntervalImpl.bind(this);
  public readonly reflectInterval = this.reflectIntervalImpl.bind(this);
  public readonly refractInterval = this.refractIntervalImpl.bind(this);
  public readonly remainderInterval = this.remainderIntervalImpl.bind(this);
  public readonly roundInterval = this.roundIntervalImpl.bind(this);
  public readonly saturateInterval = this.saturateIntervalImpl.bind(this);
  public readonly signInterval = this.signIntervalImpl.bind(this);
  public readonly sinInterval = this.sinIntervalImpl.bind(this);
  public readonly sinhInterval = this.sinhIntervalImpl.bind(this);
  public readonly smoothStepInterval = this.smoothStepIntervalImpl.bind(this);
  public readonly sqrtInterval = this.sqrtIntervalImpl.bind(this);
  public readonly stepInterval = this.stepIntervalImpl.bind(this);
  public readonly subtractionInterval = this.subtractionIntervalImpl.bind(this);
  public readonly subtractionMatrixMatrixInterval =
    this.subtractionMatrixMatrixIntervalImpl.bind(this);
  public readonly tanInterval = this.tanIntervalImpl.bind(this);
  public readonly tanhInterval = this.tanhIntervalImpl.bind(this);
  public readonly transposeInterval = this.transposeIntervalImpl.bind(this);
  public readonly truncInterval = this.truncIntervalImpl.bind(this);

  // Framework - Cases

  // U32 -> Interval is used for testing f32 specific unpack* functions
  /**
   * @returns a Case for the param and the interval generator provided.
   * The Case will use an interval comparator for matching results.
   * @param param the param to pass in
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  private makeU32ToVectorCase(
    param: number,
    filter: IntervalFilter,
    ...ops: ScalarToVector[]
  ): Case | undefined {
    param = Math.trunc(param);

    const vectors = ops.map(o => o(param));
    if (filter === 'finite' && vectors.some(v => !v.every(e => e.isFinite()))) {
      return undefined;
    }
    return {
      input: u32(param),
      expected: anyOf(...vectors),
    };
  }

  /**
   * @returns an array of Cases for operations over a range of inputs
   * @param params array of inputs to try
   * @param filter what interval filtering to apply
   * @param ops callbacks that implement generating an acceptance interval
   */
  public generateU32ToIntervalCases(
    params: readonly number[],
    filter: IntervalFilter,
    ...ops: ScalarToVector[]
  ): Case[] {
    return params.reduce((cases, e) => {
      const c = this.makeU32ToVectorCase(e, filter, ...ops);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  }

  // Framework - API

  private readonly QuantizeToF16IntervalOp: ScalarToIntervalOp = {
    impl: (n: number): FPInterval => {
      const rounded = correctlyRoundedF16(n);
      const flushed = addFlushedIfNeededF16(rounded);
      return this.spanIntervals(...flushed.map(f => this.toInterval(f)));
    },
  };

  protected quantizeToF16IntervalImpl(n: number): FPInterval {
    return this.runScalarToIntervalOp(this.toInterval(n), this.QuantizeToF16IntervalOp);
  }

  /** Calculate an acceptance interval of quantizeToF16(x) */
  public readonly quantizeToF16Interval = this.quantizeToF16IntervalImpl.bind(this);

  /**
   * Once-allocated ArrayBuffer/views to avoid overhead of allocation when
   * converting between numeric formats
   *
   * unpackData* is shared between all the unpack*Interval functions, so to
   * avoid re-entrancy problems, they should not call each other or themselves
   * directly or indirectly.
   */
  private readonly unpackData = new ArrayBuffer(4);
  private readonly unpackDataU32 = new Uint32Array(this.unpackData);
  private readonly unpackDataU16 = new Uint16Array(this.unpackData);
  private readonly unpackDataU8 = new Uint8Array(this.unpackData);
  private readonly unpackDataI16 = new Int16Array(this.unpackData);
  private readonly unpackDataI8 = new Int8Array(this.unpackData);
  private readonly unpackDataF16 = new Float16Array(this.unpackData);

  private unpack2x16floatIntervalImpl(n: number): FPVector {
    assert(
      n >= kValue.u32.min && n <= kValue.u32.max,
      'unpack2x16floatInterval only accepts valid u32 values'
    );
    this.unpackDataU32[0] = n;
    if (this.unpackDataF16.some(f => !isFiniteF16(f))) {
      return [this.constants().unboundedInterval, this.constants().unboundedInterval];
    }

    const result: FPVector = [
      this.quantizeToF16Interval(this.unpackDataF16[0]),
      this.quantizeToF16Interval(this.unpackDataF16[1]),
    ];

    if (result.some(r => !r.isFinite())) {
      return [this.constants().unboundedInterval, this.constants().unboundedInterval];
    }
    return result;
  }

  /** Calculate an acceptance interval vector for unpack2x16float(x) */
  public readonly unpack2x16floatInterval = this.unpack2x16floatIntervalImpl.bind(this);

  private unpack2x16snormIntervalImpl(n: number): FPVector {
    assert(
      n >= kValue.u32.min && n <= kValue.u32.max,
      'unpack2x16snormInterval only accepts valid u32 values'
    );
    const op = (n: number): FPInterval => {
      return this.ulpInterval(Math.max(n / 32767, -1), 3);
    };

    this.unpackDataU32[0] = n;
    return [op(this.unpackDataI16[0]), op(this.unpackDataI16[1])];
  }

  /** Calculate an acceptance interval vector for unpack2x16snorm(x) */
  public readonly unpack2x16snormInterval = this.unpack2x16snormIntervalImpl.bind(this);

  private unpack2x16unormIntervalImpl(n: number): FPVector {
    assert(
      n >= kValue.u32.min && n <= kValue.u32.max,
      'unpack2x16unormInterval only accepts valid u32 values'
    );
    const op = (n: number): FPInterval => {
      return this.ulpInterval(n / 65535, 3);
    };

    this.unpackDataU32[0] = n;
    return [op(this.unpackDataU16[0]), op(this.unpackDataU16[1])];
  }

  /** Calculate an acceptance interval vector for unpack2x16unorm(x) */
  public readonly unpack2x16unormInterval = this.unpack2x16unormIntervalImpl.bind(this);

  private unpack4x8snormIntervalImpl(n: number): FPVector {
    assert(
      n >= kValue.u32.min && n <= kValue.u32.max,
      'unpack4x8snormInterval only accepts valid u32 values'
    );
    const op = (n: number): FPInterval => {
      return this.ulpInterval(Math.max(n / 127, -1), 3);
    };
    this.unpackDataU32[0] = n;
    return [
      op(this.unpackDataI8[0]),
      op(this.unpackDataI8[1]),
      op(this.unpackDataI8[2]),
      op(this.unpackDataI8[3]),
    ];
  }

  /** Calculate an acceptance interval vector for unpack4x8snorm(x) */
  public readonly unpack4x8snormInterval = this.unpack4x8snormIntervalImpl.bind(this);

  private unpack4x8unormIntervalImpl(n: number): FPVector {
    assert(
      n >= kValue.u32.min && n <= kValue.u32.max,
      'unpack4x8unormInterval only accepts valid u32 values'
    );
    const op = (n: number): FPInterval => {
      return this.ulpInterval(n / 255, 3);
    };

    this.unpackDataU32[0] = n;
    return [
      op(this.unpackDataU8[0]),
      op(this.unpackDataU8[1]),
      op(this.unpackDataU8[2]),
      op(this.unpackDataU8[3]),
    ];
  }

  /** Calculate an acceptance interval vector for unpack4x8unorm(x) */
  public readonly unpack4x8unormInterval = this.unpack4x8unormIntervalImpl.bind(this);
}

// Need to separately allocate f32 traits, so they can be referenced by
// FPAbstractTraits for forwarding.
const kF32Traits = new F32Traits();

// Pre-defined values that get used multiple times in _constants' initializers. Cannot use FPTraits members, since this
// executes before they are defined.
const kAbstractUnboundedInterval = new FPInterval(
  'abstract',
  Number.NEGATIVE_INFINITY,
  Number.POSITIVE_INFINITY
);
const kAbstractZeroInterval = new FPInterval('abstract', 0);

// This is implementation is incomplete
class FPAbstractTraits extends FPTraits {
  private static _constants: FPConstants = {
    positive: {
      min: kValue.f64.positive.min,
      max: kValue.f64.positive.max,
      infinity: kValue.f64.positive.infinity,
      nearest_max: kValue.f64.positive.nearest_max,
      less_than_one: kValue.f64.positive.less_than_one,
      subnormal: {
        min: kValue.f64.positive.subnormal.min,
        max: kValue.f64.positive.subnormal.max,
      },
      pi: {
        whole: kValue.f64.positive.pi.whole,
        three_quarters: kValue.f64.positive.pi.three_quarters,
        half: kValue.f64.positive.pi.half,
        third: kValue.f64.positive.pi.third,
        quarter: kValue.f64.positive.pi.quarter,
        sixth: kValue.f64.positive.pi.sixth,
      },
      e: kValue.f64.positive.e,
    },
    negative: {
      min: kValue.f64.negative.min,
      max: kValue.f64.negative.max,
      infinity: kValue.f64.negative.infinity,
      nearest_min: kValue.f64.negative.nearest_min,
      less_than_one: kValue.f64.negative.less_than_one,
      subnormal: {
        min: kValue.f64.negative.subnormal.min,
        max: kValue.f64.negative.subnormal.max,
      },
      pi: {
        whole: kValue.f64.negative.pi.whole,
        three_quarters: kValue.f64.negative.pi.three_quarters,
        half: kValue.f64.negative.pi.half,
        third: kValue.f64.negative.pi.third,
        quarter: kValue.f64.negative.pi.quarter,
        sixth: kValue.f64.negative.pi.sixth,
      },
    },
    bias: 1023,
    unboundedInterval: kAbstractUnboundedInterval,
    zeroInterval: kAbstractZeroInterval,
    // Have to use the constants.ts values here, because values defined in the
    // initializer cannot be referenced in the initializer
    negPiToPiInterval: new FPInterval(
      'abstract',
      kValue.f64.negative.pi.whole,
      kValue.f64.positive.pi.whole
    ),
    greaterThanZeroInterval: new FPInterval(
      'abstract',
      kValue.f64.positive.subnormal.min,
      kValue.f64.positive.max
    ),
    negOneToOneInterval: new FPInterval('abstract', -1, 1),

    zeroVector: {
      2: [kAbstractZeroInterval, kAbstractZeroInterval],
      3: [kAbstractZeroInterval, kAbstractZeroInterval, kAbstractZeroInterval],
      4: [
        kAbstractZeroInterval,
        kAbstractZeroInterval,
        kAbstractZeroInterval,
        kAbstractZeroInterval,
      ],
    },
    unboundedVector: {
      2: [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
      3: [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
      4: [
        kAbstractUnboundedInterval,
        kAbstractUnboundedInterval,
        kAbstractUnboundedInterval,
        kAbstractUnboundedInterval,
      ],
    },
    unboundedMatrix: {
      2: {
        2: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        3: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        4: [
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
        ],
      },
      3: {
        2: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        3: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        4: [
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
        ],
      },
      4: {
        2: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        3: [
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
          [kAbstractUnboundedInterval, kAbstractUnboundedInterval, kAbstractUnboundedInterval],
        ],
        4: [
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
          [
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
            kAbstractUnboundedInterval,
          ],
        ],
      },
    },
  };

  public constructor() {
    super('abstract');
  }

  public constants(): FPConstants {
    return FPAbstractTraits._constants;
  }

  // Utilities - Overrides
  // number is represented as a f64 internally, so all number values are already
  // quantized to f64
  public readonly quantize = (n: number) => {
    return n;
  };
  public readonly correctlyRounded = correctlyRoundedF64;
  public readonly isFinite = Number.isFinite;
  public readonly isSubnormal = isSubnormalNumberF64;
  public readonly flushSubnormal = flushSubnormalNumberF64;
  public readonly oneULP = (_target: number, _mode: FlushMode = 'flush'): number => {
    unreachable(`'FPAbstractTraits.oneULP should never be called`);
  };
  public readonly scalarBuilder = abstractFloat;
  public readonly scalarRange = scalarF64Range;
  public readonly sparseScalarRange = sparseScalarF64Range;
  public readonly vectorRange = vectorF64Range;
  public readonly sparseVectorRange = sparseVectorF64Range;
  public readonly sparseMatrixRange = sparseMatrixF64Range;

  // Framework - Fundamental Error Intervals - Overrides
  public readonly absoluteErrorInterval = this.unimplementedAbsoluteErrorInterval.bind(this); // Should use FP.f32 instead
  public readonly correctlyRoundedInterval = this.correctlyRoundedIntervalImpl.bind(this);
  public readonly correctlyRoundedMatrix = this.correctlyRoundedMatrixImpl.bind(this);
  public readonly ulpInterval = this.unimplementedUlpInterval.bind(this); // Should use FP.f32 instead

  // Framework - API - Overrides
  public readonly absInterval = this.absIntervalImpl.bind(this);
  public readonly acosInterval = this.unimplementedScalarToInterval.bind(this, 'acosInterval');
  public readonly acoshAlternativeInterval = this.unimplementedScalarToInterval.bind(
    this,
    'acoshAlternativeInterval'
  );
  public readonly acoshPrimaryInterval = this.unimplementedScalarToInterval.bind(
    this,
    'acoshPrimaryInterval'
  );
  public readonly acoshIntervals = [this.acoshAlternativeInterval, this.acoshPrimaryInterval];
  public readonly additionInterval = this.unimplementedScalarPairToInterval.bind(
    this,
    'additionInterval'
  );
  public readonly additionMatrixMatrixInterval = this.unimplementedMatrixPairToMatrix.bind(
    this,
    'additionMatrixMatrixInterval'
  );
  public readonly asinInterval = this.unimplementedScalarToInterval.bind(this, 'asinInterval');
  public readonly asinhInterval = this.unimplementedScalarToInterval.bind(this, 'asinhInterval');
  public readonly atanInterval = this.unimplementedScalarToInterval.bind(this, 'atanInterval');
  public readonly atan2Interval = this.unimplementedScalarPairToInterval.bind(
    this,
    'atan2Interval'
  );
  public readonly atanhInterval = this.unimplementedScalarToInterval.bind(this, 'atanhInterval');
  public readonly ceilInterval = this.ceilIntervalImpl.bind(this);
  public readonly clampMedianInterval = this.clampMedianIntervalImpl.bind(this);
  public readonly clampMinMaxInterval = this.clampMinMaxIntervalImpl.bind(this);
  public readonly clampIntervals = [this.clampMedianInterval, this.clampMinMaxInterval];
  public readonly cosInterval = this.unimplementedScalarToInterval.bind(this, 'cosInterval');
  public readonly coshInterval = this.unimplementedScalarToInterval.bind(this, 'coshInterval');
  public readonly crossInterval = this.unimplementedVectorPairToVector.bind(this, 'crossInterval');
  public readonly degreesInterval = this.unimplementedScalarToInterval.bind(
    this,
    'degreesInterval'
  );
  public readonly determinantInterval = this.unimplementedMatrixToInterval.bind(
    this,
    'determinant'
  );
  public readonly distanceInterval = this.unimplementedDistance.bind(this);
  public readonly divisionInterval = this.unimplementedScalarPairToInterval.bind(
    this,
    'divisionInterval'
  );
  public readonly dotInterval = this.unimplementedVectorPairToInterval.bind(this, 'dotInterval');
  public readonly expInterval = this.unimplementedScalarToInterval.bind(this, 'expInterval');
  public readonly exp2Interval = this.unimplementedScalarToInterval.bind(this, 'exp2Interval');
  public readonly faceForwardIntervals = this.unimplementedFaceForward.bind(this);
  public readonly floorInterval = this.floorIntervalImpl.bind(this);
  public readonly fmaInterval = this.unimplementedScalarTripleToInterval.bind(this, 'fmaInterval');
  public readonly fractInterval = this.unimplementedScalarToInterval.bind(this, 'fractInterval');
  public readonly inverseSqrtInterval = this.unimplementedScalarToInterval.bind(
    this,
    'inverseSqrtInterval'
  );
  public readonly ldexpInterval = this.ldexpIntervalImpl.bind(this);
  public readonly lengthInterval = this.unimplementedLength.bind(this);
  public readonly logInterval = this.unimplementedScalarToInterval.bind(this, 'logInterval');
  public readonly log2Interval = this.unimplementedScalarToInterval.bind(this, 'log2Interval');
  public readonly maxInterval = this.maxIntervalImpl.bind(this);
  public readonly minInterval = this.minIntervalImpl.bind(this);
  public readonly mixImpreciseInterval = this.unimplementedScalarTripleToInterval.bind(
    this,
    'mixImpreciseInterval'
  );
  public readonly mixPreciseInterval = this.unimplementedScalarTripleToInterval.bind(
    this,
    'mixPreciseInterval'
  );
  public readonly mixIntervals = [this.mixImpreciseInterval, this.mixPreciseInterval];
  public readonly modfInterval = this.modfIntervalImpl.bind(this);
  public readonly multiplicationInterval = this.unimplementedScalarPairToInterval.bind(
    this,
    'multiplicationInterval'
  );
  public readonly multiplicationMatrixMatrixInterval = this.unimplementedMatrixPairToMatrix.bind(
    this,
    'multiplicationMatrixMatrixInterval'
  );
  public readonly multiplicationMatrixScalarInterval = this.unimplementedMatrixScalarToMatrix.bind(
    this,
    'multiplicationMatrixScalarInterval'
  );
  public readonly multiplicationScalarMatrixInterval = this.unimplementedScalarMatrixToMatrix.bind(
    this,
    'multiplicationScalarMatrixInterval'
  );
  public readonly multiplicationMatrixVectorInterval = this.unimplementedMatrixVectorToVector.bind(
    this,
    'multiplicationMatrixVectorInterval'
  );
  public readonly multiplicationVectorMatrixInterval = this.unimplementedVectorMatrixToVector.bind(
    this,
    'multiplicationVectorMatrixInterval'
  );
  public readonly negationInterval = this.negationIntervalImpl.bind(this);
  public readonly normalizeInterval = this.unimplementedVectorToVector.bind(
    this,
    'normalizeInterval'
  );
  public readonly powInterval = this.unimplementedScalarPairToInterval.bind(this, 'powInterval');
  public readonly radiansInterval = this.unimplementedScalarToInterval.bind(this, 'radiansImpl');
  public readonly reflectInterval = this.unimplementedVectorPairToVector.bind(
    this,
    'reflectInterval'
  );
  public readonly refractInterval = this.unimplementedRefract.bind(this);
  public readonly remainderInterval = this.unimplementedScalarPairToInterval.bind(
    this,
    'remainderInterval'
  );
  public readonly roundInterval = this.roundIntervalImpl.bind(this);
  public readonly saturateInterval = this.saturateIntervalImpl.bind(this);
  public readonly signInterval = this.signIntervalImpl.bind(this);
  public readonly sinInterval = this.unimplementedScalarToInterval.bind(this, 'sinInterval');
  public readonly sinhInterval = this.unimplementedScalarToInterval.bind(this, 'sinhInterval');
  public readonly smoothStepInterval = this.unimplementedScalarTripleToInterval.bind(
    this,
    'smoothStepInterval'
  );
  public readonly sqrtInterval = this.unimplementedScalarToInterval.bind(this, 'sqrtInterval');
  public readonly stepInterval = this.stepIntervalImpl.bind(this);
  public readonly subtractionInterval = this.unimplementedScalarPairToInterval.bind(
    this,
    'subtractionInterval'
  );
  public readonly subtractionMatrixMatrixInterval = this.unimplementedMatrixPairToMatrix.bind(
    this,
    'subtractionMatrixMatrixInterval'
  );
  public readonly tanInterval = this.unimplementedScalarToInterval.bind(this, 'tanInterval');
  public readonly tanhInterval = this.unimplementedScalarToInterval.bind(this, 'tanhInterval');
  public readonly transposeInterval = this.transposeIntervalImpl.bind(this);
  public readonly truncInterval = this.truncIntervalImpl.bind(this);
}

// Pre-defined values that get used multiple times in _constants' initializers. Cannot use FPTraits members, since this
// executes before they are defined.
const kF16UnboundedInterval = new FPInterval(
  'f16',
  Number.NEGATIVE_INFINITY,
  Number.POSITIVE_INFINITY
);
const kF16ZeroInterval = new FPInterval('f16', 0);

// This is implementation is incomplete
class F16Traits extends FPTraits {
  private static _constants: FPConstants = {
    positive: {
      min: kValue.f16.positive.min,
      max: kValue.f16.positive.max,
      infinity: kValue.f16.positive.infinity,
      nearest_max: kValue.f16.positive.nearest_max,
      less_than_one: kValue.f16.positive.less_than_one,
      subnormal: {
        min: kValue.f16.positive.subnormal.min,
        max: kValue.f16.positive.subnormal.max,
      },
      pi: {
        whole: kValue.f16.positive.pi.whole,
        three_quarters: kValue.f16.positive.pi.three_quarters,
        half: kValue.f16.positive.pi.half,
        third: kValue.f16.positive.pi.third,
        quarter: kValue.f16.positive.pi.quarter,
        sixth: kValue.f16.positive.pi.sixth,
      },
      e: kValue.f16.positive.e,
    },
    negative: {
      min: kValue.f16.negative.min,
      max: kValue.f16.negative.max,
      infinity: kValue.f16.negative.infinity,
      nearest_min: kValue.f16.negative.nearest_min,
      less_than_one: kValue.f16.negative.less_than_one,
      subnormal: {
        min: kValue.f16.negative.subnormal.min,
        max: kValue.f16.negative.subnormal.max,
      },
      pi: {
        whole: kValue.f16.negative.pi.whole,
        three_quarters: kValue.f16.negative.pi.three_quarters,
        half: kValue.f16.negative.pi.half,
        third: kValue.f16.negative.pi.third,
        quarter: kValue.f16.negative.pi.quarter,
        sixth: kValue.f16.negative.pi.sixth,
      },
    },
    bias: 15,
    unboundedInterval: kF16UnboundedInterval,
    zeroInterval: kF16ZeroInterval,
    // Have to use the constants.ts values here, because values defined in the
    // initializer cannot be referenced in the initializer
    negPiToPiInterval: new FPInterval(
      'f16',
      kValue.f16.negative.pi.whole,
      kValue.f16.positive.pi.whole
    ),
    greaterThanZeroInterval: new FPInterval(
      'f16',
      kValue.f16.positive.subnormal.min,
      kValue.f16.positive.max
    ),
    negOneToOneInterval: new FPInterval('f16', -1, 1),

    zeroVector: {
      2: [kF16ZeroInterval, kF16ZeroInterval],
      3: [kF16ZeroInterval, kF16ZeroInterval, kF16ZeroInterval],
      4: [kF16ZeroInterval, kF16ZeroInterval, kF16ZeroInterval, kF16ZeroInterval],
    },
    unboundedVector: {
      2: [kF16UnboundedInterval, kF16UnboundedInterval],
      3: [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
      4: [
        kF16UnboundedInterval,
        kF16UnboundedInterval,
        kF16UnboundedInterval,
        kF16UnboundedInterval,
      ],
    },
    unboundedMatrix: {
      2: {
        2: [
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        3: [
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        4: [
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
        ],
      },
      3: {
        2: [
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        3: [
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        4: [
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
        ],
      },
      4: {
        2: [
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        3: [
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
          [kF16UnboundedInterval, kF16UnboundedInterval, kF16UnboundedInterval],
        ],
        4: [
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
          [
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
            kF16UnboundedInterval,
          ],
        ],
      },
    },
  };

  public constructor() {
    super('f16');
  }

  public constants(): FPConstants {
    return F16Traits._constants;
  }

  // Utilities - Overrides
  public readonly quantize = quantizeToF16;
  public readonly correctlyRounded = correctlyRoundedF16;
  public readonly isFinite = isFiniteF16;
  public readonly isSubnormal = isSubnormalNumberF16;
  public readonly flushSubnormal = flushSubnormalNumberF16;
  public readonly oneULP = oneULPF16;
  public readonly scalarBuilder = f16;
  public readonly scalarRange = scalarF16Range;
  public readonly sparseScalarRange = sparseScalarF16Range;
  public readonly vectorRange = vectorF16Range;
  public readonly sparseVectorRange = sparseVectorF16Range;
  public readonly sparseMatrixRange = sparseMatrixF16Range;

  // Framework - Fundamental Error Intervals - Overrides
  public readonly absoluteErrorInterval = this.absoluteErrorIntervalImpl.bind(this);
  public readonly correctlyRoundedInterval = this.correctlyRoundedIntervalImpl.bind(this);
  public readonly correctlyRoundedMatrix = this.correctlyRoundedMatrixImpl.bind(this);
  public readonly ulpInterval = this.ulpIntervalImpl.bind(this);

  // Framework - API - Overrides
  public readonly absInterval = this.absIntervalImpl.bind(this);
  public readonly acosInterval = this.acosIntervalImpl.bind(this);
  public readonly acoshAlternativeInterval = this.acoshAlternativeIntervalImpl.bind(this);
  public readonly acoshPrimaryInterval = this.acoshPrimaryIntervalImpl.bind(this);
  public readonly acoshIntervals = [this.acoshAlternativeInterval, this.acoshPrimaryInterval];
  public readonly additionInterval = this.additionIntervalImpl.bind(this);
  public readonly additionMatrixMatrixInterval = this.additionMatrixMatrixIntervalImpl.bind(this);
  public readonly asinInterval = this.asinIntervalImpl.bind(this);
  public readonly asinhInterval = this.asinhIntervalImpl.bind(this);
  public readonly atanInterval = this.atanIntervalImpl.bind(this);
  public readonly atan2Interval = this.atan2IntervalImpl.bind(this);
  public readonly atanhInterval = this.atanhIntervalImpl.bind(this);
  public readonly ceilInterval = this.ceilIntervalImpl.bind(this);
  public readonly clampMedianInterval = this.clampMedianIntervalImpl.bind(this);
  public readonly clampMinMaxInterval = this.clampMinMaxIntervalImpl.bind(this);
  public readonly clampIntervals = [this.clampMedianInterval, this.clampMinMaxInterval];
  public readonly cosInterval = this.cosIntervalImpl.bind(this);
  public readonly coshInterval = this.coshIntervalImpl.bind(this);
  public readonly crossInterval = this.crossIntervalImpl.bind(this);
  public readonly degreesInterval = this.degreesIntervalImpl.bind(this);
  public readonly determinantInterval = this.determinantIntervalImpl.bind(this);
  public readonly distanceInterval = this.distanceIntervalImpl.bind(this);
  public readonly divisionInterval = this.divisionIntervalImpl.bind(this);
  public readonly dotInterval = this.dotIntervalImpl.bind(this);
  public readonly expInterval = this.expIntervalImpl.bind(this);
  public readonly exp2Interval = this.exp2IntervalImpl.bind(this);
  public readonly faceForwardIntervals = this.faceForwardIntervalsImpl.bind(this);
  public readonly floorInterval = this.floorIntervalImpl.bind(this);
  public readonly fmaInterval = this.fmaIntervalImpl.bind(this);
  public readonly fractInterval = this.fractIntervalImpl.bind(this);
  public readonly inverseSqrtInterval = this.inverseSqrtIntervalImpl.bind(this);
  public readonly ldexpInterval = this.ldexpIntervalImpl.bind(this);
  public readonly lengthInterval = this.lengthIntervalImpl.bind(this);
  public readonly logInterval = this.logIntervalImpl.bind(this);
  public readonly log2Interval = this.log2IntervalImpl.bind(this);
  public readonly maxInterval = this.maxIntervalImpl.bind(this);
  public readonly minInterval = this.minIntervalImpl.bind(this);
  public readonly mixImpreciseInterval = this.mixImpreciseIntervalImpl.bind(this);
  public readonly mixPreciseInterval = this.mixPreciseIntervalImpl.bind(this);
  public readonly mixIntervals = [this.mixImpreciseInterval, this.mixPreciseInterval];
  public readonly modfInterval = this.modfIntervalImpl.bind(this);
  public readonly multiplicationInterval = this.multiplicationIntervalImpl.bind(this);
  public readonly multiplicationMatrixMatrixInterval =
    this.multiplicationMatrixMatrixIntervalImpl.bind(this);
  public readonly multiplicationMatrixScalarInterval =
    this.multiplicationMatrixScalarIntervalImpl.bind(this);
  public readonly multiplicationScalarMatrixInterval =
    this.multiplicationScalarMatrixIntervalImpl.bind(this);
  public readonly multiplicationMatrixVectorInterval =
    this.multiplicationMatrixVectorIntervalImpl.bind(this);
  public readonly multiplicationVectorMatrixInterval =
    this.multiplicationVectorMatrixIntervalImpl.bind(this);
  public readonly negationInterval = this.negationIntervalImpl.bind(this);
  public readonly normalizeInterval = this.normalizeIntervalImpl.bind(this);
  public readonly powInterval = this.powIntervalImpl.bind(this);
  public readonly radiansInterval = this.radiansIntervalImpl.bind(this);
  public readonly reflectInterval = this.reflectIntervalImpl.bind(this);
  public readonly refractInterval = this.refractIntervalImpl.bind(this);
  public readonly remainderInterval = this.remainderIntervalImpl.bind(this);
  public readonly roundInterval = this.roundIntervalImpl.bind(this);
  public readonly saturateInterval = this.saturateIntervalImpl.bind(this);
  public readonly signInterval = this.signIntervalImpl.bind(this);
  public readonly sinInterval = this.sinIntervalImpl.bind(this);
  public readonly sinhInterval = this.sinhIntervalImpl.bind(this);
  public readonly smoothStepInterval = this.smoothStepIntervalImpl.bind(this);
  public readonly sqrtInterval = this.sqrtIntervalImpl.bind(this);
  public readonly stepInterval = this.stepIntervalImpl.bind(this);
  public readonly subtractionInterval = this.subtractionIntervalImpl.bind(this);
  public readonly subtractionMatrixMatrixInterval =
    this.subtractionMatrixMatrixIntervalImpl.bind(this);
  public readonly tanInterval = this.tanIntervalImpl.bind(this);
  public readonly tanhInterval = this.tanhIntervalImpl.bind(this);
  public readonly transposeInterval = this.transposeIntervalImpl.bind(this);
  public readonly truncInterval = this.truncIntervalImpl.bind(this);
}

export const FP = {
  f32: kF32Traits,
  f16: new F16Traits(),
  abstract: new FPAbstractTraits(),
};

/** @returns the floating-point traits for `type` */
export function fpTraitsFor(type: ScalarType): FPTraits {
  switch (type.kind) {
    case 'abstract-float':
      return FP.abstract;
    case 'f32':
      return FP.f32;
    case 'f16':
      return FP.f16;
    default:
      unreachable(`unsupported type: ${type}`);
  }
}

/** @returns true if the value `value` is representable with `type` */
export function isRepresentable(value: number, type: ScalarType) {
  if (!Number.isFinite(value)) {
    return false;
  }
  if (isFloatType(type)) {
    const constants = fpTraitsFor(type).constants();
    return value >= constants.negative.min && value <= constants.positive.max;
  }

  assert(false, `isRepresentable() is not yet implemented for type ${type}`);
}
