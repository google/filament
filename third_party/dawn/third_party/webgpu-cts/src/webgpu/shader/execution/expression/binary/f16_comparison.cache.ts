import { anyOf } from '../../../../util/compare.js';
import { bool, f16, ScalarValue } from '../../../../util/conversion.js';
import { flushSubnormalNumberF16, vectorF16Range } from '../../../../util/math.js';
import { Case } from '../case.js';
import { makeCaseCache } from '../case_cache.js';

/**
 * @returns a test case for the provided left hand & right hand values and truth function.
 * Handles quantization and subnormals.
 */
function makeCase(
  lhs: number,
  rhs: number,
  truthFunc: (lhs: ScalarValue, rhs: ScalarValue) => boolean
): Case {
  // Subnormal float values may be flushed at any time.
  // https://www.w3.org/TR/WGSL/#floating-point-evaluation
  const f16_lhs = f16(lhs);
  const f16_rhs = f16(rhs);
  const lhs_options = new Set([f16_lhs, f16(flushSubnormalNumberF16(lhs))]);
  const rhs_options = new Set([f16_rhs, f16(flushSubnormalNumberF16(rhs))]);
  const expected: Array<ScalarValue> = [];
  lhs_options.forEach(l => {
    rhs_options.forEach(r => {
      const result = bool(truthFunc(l, r));
      if (!expected.includes(result)) {
        expected.push(result);
      }
    });
  });

  return { input: [f16_lhs, f16_rhs], expected: anyOf(...expected) };
}

export const d = makeCaseCache('binary/f16_logical', {
  equals_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) === (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  equals_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) === (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  not_equals_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) !== (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  not_equals_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) !== (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  less_than_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) < (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  less_than_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) < (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  less_equals_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) <= (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  less_equals_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) <= (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  greater_than_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) > (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  greater_than_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) > (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  greater_equals_non_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) >= (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
  greater_equals_const: () => {
    const truthFunc = (lhs: ScalarValue, rhs: ScalarValue): boolean => {
      return (lhs.value as number) >= (rhs.value as number);
    };

    return vectorF16Range(2).map(v => {
      return makeCase(v[0], v[1], truthFunc);
    });
  },
});
