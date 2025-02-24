import { FP, FPVector } from '../../../../util/floating_point.js';
import { sparseScalarF64Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

import { getMultiplicationAFInterval, kSparseVectorAFValues } from './af_data.js';

const multiplicationVectorScalarInterval = (v: readonly number[], s: number): FPVector => {
  return FP.abstract.toVector(v.map(e => getMultiplicationAFInterval(e, s)));
};

const multiplicationScalarVectorInterval = (s: number, v: readonly number[]): FPVector => {
  return FP.abstract.toVector(v.map(e => getMultiplicationAFInterval(s, e)));
};

const scalar_cases = {
  ['scalar']: () => {
    return FP.abstract.generateScalarPairToIntervalCases(
      sparseScalarF64Range(),
      sparseScalarF64Range(),
      'finite',
      getMultiplicationAFInterval
    );
  },
};

const vector_scalar_cases = ([2, 3, 4] as const)
  .map(dim => ({
    [`vec${dim}_scalar`]: () => {
      return FP.abstract.generateVectorScalarToVectorCases(
        kSparseVectorAFValues[dim],
        sparseScalarF64Range(),
        'finite',
        multiplicationVectorScalarInterval
      );
    },
  }))
  .reduce((a, b) => ({ ...a, ...b }), {});

const scalar_vector_cases = ([2, 3, 4] as const)
  .map(dim => ({
    [`scalar_vec${dim}`]: () => {
      return FP.abstract.generateScalarVectorToVectorCases(
        sparseScalarF64Range(),
        kSparseVectorAFValues[dim],
        'finite',
        multiplicationScalarVectorInterval
      );
    },
  }))
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/af_multiplication', {
  ...scalar_cases,
  ...vector_scalar_cases,
  ...scalar_vector_cases,
});
