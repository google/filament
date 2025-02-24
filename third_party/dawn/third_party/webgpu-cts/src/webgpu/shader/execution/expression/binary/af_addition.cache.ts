import { FP, FPVector } from '../../../../util/floating_point.js';
import { sparseScalarF64Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

import { getAdditionAFInterval, kSparseVectorAFValues } from './af_data.js';

const additionVectorScalarInterval = (vec: readonly number[], s: number): FPVector => {
  return FP.abstract.toVector(vec.map(v => getAdditionAFInterval(v, s)));
};

const additionScalarVectorInterval = (s: number, vec: readonly number[]): FPVector => {
  return FP.abstract.toVector(vec.map(v => getAdditionAFInterval(s, v)));
};

const vector_scalar_cases = ([2, 3, 4] as const)
  .map(dim => ({
    [`vec${dim}_scalar`]: () => {
      return FP.abstract.generateVectorScalarToVectorCases(
        kSparseVectorAFValues[dim],
        sparseScalarF64Range(),
        'finite',
        additionVectorScalarInterval
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
        additionScalarVectorInterval
      );
    },
  }))
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/af_addition', {
  ['scalar']: () => {
    return FP.abstract.generateScalarPairToIntervalCases(
      sparseScalarF64Range(),
      sparseScalarF64Range(),
      'finite',
      getAdditionAFInterval
    );
  },
  ...vector_scalar_cases,
  ...scalar_vector_cases,
});
