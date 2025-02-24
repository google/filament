import { assert, unreachable } from '../../../../../../common/util/util.js';
import { kValue } from '../../../../../util/constants.js';
import {
  ScalarType,
  Type,
  Value,
  elementTypeOf,
  isAbstractType,
  scalarElementsOf,
  scalarTypeOf,
} from '../../../../../util/conversion.js';
import {
  scalarF16Range,
  scalarF32Range,
  scalarF64Range,
  linearRange,
  linearRangeBigInt,
  quantizeToF32,
  quantizeToF16,
  QuantizeFunc,
} from '../../../../../util/math.js';
import { ShaderValidationTest } from '../../../shader_validation_test.js';

/** @returns a function that can select between ranges depending on type */
export function rangeForType(
  number_range: readonly number[],
  bigint_range: readonly bigint[]
): (type: Type) => readonly (number | bigint)[] {
  return (type: Type): readonly (number | bigint)[] => {
    switch (scalarTypeOf(type).kind) {
      case 'abstract-float':
      case 'f32':
      case 'f16':
        return number_range;
      case 'abstract-int':
        return bigint_range;
    }
    unreachable(`Received unexpected type '${type}'`);
  };
}

/* @returns a linear sweep between -2 to 2 for type */
// prettier-ignore
export const minusTwoToTwoRangeForType = rangeForType(
  linearRange(-2, 2, 10),
  [ -2n, -1n, 0n, 1n, 2n ]
);

/* @returns array of values ranging from -3π to 3π, with a focus on multiples of π */
export const minusThreePiToThreePiRangeForType = rangeForType(
  [
    -3 * Math.PI,
    -2.999 * Math.PI,

    -2.501 * Math.PI,
    -2.5 * Math.PI,
    -2.499 * Math.PI,

    -2.001 * Math.PI,
    -2.0 * Math.PI,
    -1.999 * Math.PI,

    -1.501 * Math.PI,
    -1.5 * Math.PI,
    -1.499 * Math.PI,

    -1.001 * Math.PI,
    -1.0 * Math.PI,
    -0.999 * Math.PI,

    -0.501 * Math.PI,
    -0.5 * Math.PI,
    -0.499 * Math.PI,

    -0.001,
    0,
    0.001,

    0.499 * Math.PI,
    0.5 * Math.PI,
    0.501 * Math.PI,

    0.999 * Math.PI,
    1.0 * Math.PI,
    1.001 * Math.PI,

    1.499 * Math.PI,
    1.5 * Math.PI,
    1.501 * Math.PI,

    1.999 * Math.PI,
    2.0 * Math.PI,
    2.001 * Math.PI,

    2.499 * Math.PI,
    2.5 * Math.PI,
    2.501 * Math.PI,

    2.999 * Math.PI,
    3 * Math.PI,
  ],
  [-2n, -1n, 0n, 1n, 2n]
);

/**
 * @returns a minimal array of values ranging from -3π to 3π, with a focus on
 * multiples of π.
 *
 * Used when multiple parameters are being passed in, so the number of cases
 * becomes the square or more of this list. */
export const sparseMinusThreePiToThreePiRangeForType = rangeForType(
  [
    -3 * Math.PI,
    -2.5 * Math.PI,
    -2.0 * Math.PI,
    -1.5 * Math.PI,
    -1.0 * Math.PI,
    -0.5 * Math.PI,
    0,
    0.5 * Math.PI,
    Math.PI,
    1.5 * Math.PI,
    2.0 * Math.PI,
    2.5 * Math.PI,
    3 * Math.PI,
  ],
  [-2n, -1n, 0n, 1n, 2n]
);

/// The evaluation stages to test
export const kConstantAndOverrideStages = ['constant', 'override'] as const;

export type ConstantOrOverrideStage = 'constant' | 'override';
export type ExecutionStage = 'constant' | 'override' | 'runtime';

/**
 * @returns true if evaluation stage `stage` supports expressions of type @p.
 */
export function stageSupportsType(stage: ConstantOrOverrideStage, type: Type) {
  if (stage === 'override' && isAbstractType(elementTypeOf(type))) {
    // Abstract numerics are concretized before being used in an override expression.
    return false;
  }
  return true;
}

/**
 * Runs a validation test to check that evaluation of `builtin` either evaluates with or without
 * error at shader creation time or pipeline creation time.
 * @param t the ShaderValidationTest
 * @param builtin the name of the builtin
 * @param expectedResult false if an error is expected, true if no error is expected
 * @param args the arguments to pass to the builtin
 * @param stage the evaluation stage
 * @param returnType the explicit return type of the result variable, if provided (implicit otherwise)
 */
export function validateConstOrOverrideBuiltinEval(
  t: ShaderValidationTest,
  builtin: string,
  expectedResult: boolean,
  args: Value[],
  stage: ConstantOrOverrideStage,
  returnType?: Type
) {
  const elTys = args.map(arg => elementTypeOf(arg.type));
  const enables = elTys.some(ty => ty === Type.f16) ? 'enable f16;' : '';
  const optionalVarType = returnType ? `: ${returnType.toString()}` : '';

  switch (stage) {
    case 'constant': {
      t.expectCompileResult(
        expectedResult,
        `${enables}
const v ${optionalVarType} = ${builtin}(${args.map(arg => arg.wgsl()).join(', ')});`
      );
      break;
    }
    case 'override': {
      assert(!elTys.some(ty => isAbstractType(ty)));
      const constants: Record<string, number> = {};
      const overrideDecls: string[] = [];
      const callArgs: string[] = [];
      let numOverrides = 0;
      for (const arg of args) {
        const argOverrides: string[] = [];
        for (const el of scalarElementsOf(arg)) {
          const name = `o${numOverrides++}`;
          overrideDecls.push(`override ${name} : ${el.type};`);
          argOverrides.push(name);
          constants[name] = Number(el.value);
        }
        callArgs.push(`${arg.type}(${argOverrides.join(', ')})`);
      }
      t.expectPipelineResult({
        expectedResult,
        code: `${enables}
${overrideDecls.join('\n')}
var<private> v ${optionalVarType} = ${builtin}(${callArgs.join(', ')});`,
        constants,
        reference: ['v'],
      });
      break;
    }
  }
}

/**
 * Runs a validation test to check that evaluation of `binaryOp` either evaluates with or without
 * error at shader creation time or pipeline creation time.
 * @param t the ShaderValidationTest
 * @param binaryOp the symbol of the binary operator
 * @param expectedResult false if an error is expected, true if no error is expected
 * @param leftStage the evaluation stage for the left argument
 * @param left the left-hand side of the binary operation
 * @param rightStage the evaluation stage for the right argument
 * @param right the right-hand side of the binary operation
 */
export function validateConstOrOverrideBinaryOpEval(
  t: ShaderValidationTest,
  binaryOp: string,
  expectedResult: boolean,
  leftStage: ExecutionStage,
  left: Value,
  rightStage: ExecutionStage,
  right: Value
) {
  const allArgs = [left, right];
  const elTys = allArgs.map(arg => elementTypeOf(arg.type));
  const enables = elTys.some(ty => ty === Type.f16) ? 'enable f16;' : '';

  const codeLines = [enables];
  const constants: Record<string, number> = {};
  let numOverrides = 0;

  function addOperand(name: string, stage: ExecutionStage, value: Value) {
    switch (stage) {
      case 'runtime':
        assert(!isAbstractType(value.type));
        codeLines.push(`var<private> ${name} = ${value.wgsl()};`);
        return name;

      case 'constant':
        codeLines.push(`const ${name} = ${value.wgsl()};`);
        return name;

      case 'override': {
        assert(!isAbstractType(value.type));
        const argOverrides: string[] = [];
        for (const el of scalarElementsOf(value)) {
          const elName = `o${numOverrides++}`;
          codeLines.push(`override ${elName} : ${el.type};`);
          constants[elName] = Number(el.value);
          argOverrides.push(elName);
        }
        return `${value.type}(${argOverrides.join(', ')})`;
      }
    }
  }

  const leftOperand = addOperand('left', leftStage, left);
  const rightOperand = addOperand('right', rightStage, right);

  if (leftStage === 'override' || rightStage === 'override') {
    t.expectPipelineResult({
      expectedResult,
      code: codeLines.join('\n'),
      constants,
      reference: [`${leftOperand} ${binaryOp} ${rightOperand}`],
    });
  } else {
    codeLines.push(`fn f() { _ = ${leftOperand} ${binaryOp} ${rightOperand}; }`);
    t.expectCompileResult(expectedResult, codeLines.join('\n'));
  }
}
/** @returns a sweep of the representable values for element type of `type` */
export function fullRangeForType(type: Type, count?: number): readonly (number | bigint)[] {
  if (count === undefined) {
    count = 25;
  }
  switch (scalarTypeOf(type)?.kind) {
    case 'abstract-float':
      return scalarF64Range({
        pos_sub: Math.ceil((count * 1) / 5),
        pos_norm: Math.ceil((count * 4) / 5),
      });
    case 'f32':
      return scalarF32Range({
        pos_sub: Math.ceil((count * 1) / 5),
        pos_norm: Math.ceil((count * 4) / 5),
      });
    case 'f16':
      return scalarF16Range({
        pos_sub: Math.ceil((count * 1) / 5),
        pos_norm: Math.ceil((count * 4) / 5),
      });
    case 'i32':
      return linearRange(kValue.i32.negative.min, kValue.i32.positive.max, count).map(f =>
        Math.floor(f)
      );
    case 'u32':
      return linearRange(0, kValue.u32.max, count).map(f => Math.floor(f));
    case 'abstract-int':
      // Returned values are already ints, so don't need to be floored.
      return linearRangeBigInt(kValue.i64.negative.min, kValue.i64.positive.max, count);
  }
  unreachable();
}

/** @returns all the values in the provided arrays with duplicates removed */
export function unique<T>(...arrays: Array<readonly T[]>): T[] {
  const set = new Set<T>();
  for (const arr of arrays) {
    for (const item of arr) {
      set.add(item);
    }
  }
  return [...set];
}

interface FloatLimits {
  positive: {
    max: number;
    min: number;
  };
  negative: {
    max: number;
    min: number;
  };
  emax: number;
}

/**
 * Provides an easy way to validate steps in an equation that will trigger a validation error with
 * constant or override values due to overflow/underflow. Typical call pattern is:
 *
 * const vCheck = new ConstantOrOverrideValueChecker(t, Type.f32);
 * const c = vCheck.checkedResult(a + b);
 * const d = vCheck.checkedResult(c * c);
 * const expectedResult = vCheck.allChecksPassed();
 */
export class ConstantOrOverrideValueChecker {
  #allChecksPassed = true;
  #floatLimits?: FloatLimits;
  #quantizeFn: QuantizeFunc<number>;

  constructor(
    private t: ShaderValidationTest,
    type: ScalarType
  ) {
    switch (type) {
      case Type.f32:
        this.#quantizeFn = quantizeToF32;
        this.#floatLimits = kValue.f32;
        break;
      case Type.f16:
        this.#quantizeFn = quantizeToF16;
        this.#floatLimits = kValue.f16;
        break;
      default:
        this.#quantizeFn = (v: number) => v;
        break;
    }
  }

  quantize(value: number): number {
    return this.#quantizeFn(value);
  }

  // Some overflow floating point values may fall into an abiguously rounded scenario, where they
  // can either round up to Infinity or down to the maximum representable value. In these cases the
  // test should be skipped, because it's valid for implementations to differ.
  // See: https://www.w3.org/TR/WGSL/#floating-point-overflow
  isAmbiguousOverflow(value: number): boolean {
    // Non-finite values are not ambiguous, and can still be validated.
    if (!Number.isFinite(value)) {
      return false;
    }

    // Values within the min/max range for the given type are not ambiguous.
    if (
      !this.#floatLimits ||
      (value <= this.#floatLimits.positive.max && value >= this.#floatLimits.negative.min)
    ) {
      return false;
    }

    // If a value falls outside the min/max range, check to see if it is under
    // 2^(EMAX(T)+1). If so, the rounding behavior is implementation specific,
    // and should not be validated.
    return Math.abs(value) < Math.pow(2, this.#floatLimits.emax + 1);
  }

  // Returns true if the value may be quantized to zero with the given type.
  isNearZero(value: number): boolean {
    if (!Number.isFinite(value)) {
      return false;
    }
    if (!this.#floatLimits) {
      return value === 0;
    }

    return value < this.#floatLimits.positive.min && value > this.#floatLimits.negative.max;
  }

  checkedResult(value: number): number {
    if (this.isAmbiguousOverflow(value)) {
      this.t.skip(`Checked value, ${value}, was within the ambiguous overflow rounding range.`);
    }

    const quantizedValue = this.quantize(value);
    if (!Number.isFinite(quantizedValue)) {
      this.#allChecksPassed = false;
    }
    return quantizedValue;
  }

  checkedResultBigInt(value: bigint): bigint {
    if (kValue.i64.isOOB(value)) {
      this.#allChecksPassed = false;
    }
    return value;
  }

  skipIfCheckFails(value: number): number {
    if (this.isAmbiguousOverflow(value)) {
      this.t.skip(`Checked value, ${value}, was within the ambiguous overflow rounding range.`);
    }

    const quantizedValue = this.quantize(value);
    if (!Number.isFinite(quantizedValue)) {
      this.t.skip(`Checked value, ${value}, was not finite after quantization.`);
    }
    return value;
  }

  allChecksPassed(): boolean {
    return this.#allChecksPassed;
  }
}
