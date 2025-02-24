import { bool, i32 } from '../../../../util/conversion.js';
import { vectorI32Range } from '../../../../util/math.js';
import { Case } from '../case.js';
import { makeCaseCache } from '../case_cache.js';

/**
 * @returns a test case for the provided left hand & right hand values and
 * expected boolean result.
 */
function makeCase(lhs: number, rhs: number, expected_answer: boolean): Case {
  return { input: [i32(lhs), i32(rhs)], expected: bool(expected_answer) };
}

export const d = makeCaseCache('binary/i32_comparison', {
  equals: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] === v[1])),
  not_equals: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] !== v[1])),
  less_than: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] < v[1])),
  less_equal: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] <= v[1])),
  greater_than: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] > v[1])),
  greater_equal: () => vectorI32Range(2).map(v => makeCase(v[0], v[1], v[0] >= v[1])),
});
