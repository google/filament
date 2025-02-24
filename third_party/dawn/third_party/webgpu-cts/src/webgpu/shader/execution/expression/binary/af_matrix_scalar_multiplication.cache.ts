import { ROArrayArray } from '../../../../../common/util/types.js';
import { FP, FPMatrix } from '../../../../util/floating_point.js';
import { flatten2DArray, sparseScalarF64Range, unflatten2DArray } from '../../../../util/math.js';
import { selectNCases } from '../case.js';
import { makeCaseCache } from '../case_cache.js';

import { getMultiplicationAFInterval, kSparseMatrixAFValues } from './af_data.js';

const multiplicationMatrixScalarInterval = (mat: ROArrayArray<number>, s: number): FPMatrix => {
  const cols = mat.length;
  const rows = mat[0].length;
  return FP.abstract.toMatrix(
    unflatten2DArray(
      flatten2DArray(mat).map(e => getMultiplicationAFInterval(e, s)),
      cols,
      rows
    )
  );
};

const multiplicationScalarMatrixInterval = (s: number, mat: ROArrayArray<number>): FPMatrix => {
  const cols = mat.length;
  const rows = mat[0].length;
  return FP.abstract.toMatrix(
    unflatten2DArray(
      flatten2DArray(mat).map(e => getMultiplicationAFInterval(s, e)),
      cols,
      rows
    )
  );
};

// Cases: matCxR_scalar
const mat_scalar_cases = ([2, 3, 4] as const)
  .flatMap(cols =>
    ([2, 3, 4] as const).map(rows => ({
      [`mat${cols}x${rows}_scalar`]: () => {
        return selectNCases(
          'binary/af_matrix_scalar_multiplication_mat_scalar',
          50,
          FP.abstract.generateMatrixScalarToMatrixCases(
            kSparseMatrixAFValues[cols][rows],
            sparseScalarF64Range(),
            'finite',
            multiplicationMatrixScalarInterval
          )
        );
      },
    }))
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

// Cases: scalar_matCxR
const scalar_mat_cases = ([2, 3, 4] as const)
  .flatMap(cols =>
    ([2, 3, 4] as const).map(rows => ({
      [`scalar_mat${cols}x${rows}`]: () => {
        return selectNCases(
          'binary/af_matrix_scalar_multiplication_scalar_mat',
          50,
          FP.abstract.generateScalarMatrixToMatrixCases(
            sparseScalarF64Range(),
            kSparseMatrixAFValues[cols][rows],
            'finite',
            multiplicationScalarMatrixInterval
          )
        );
      },
    }))
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/af_matrix_scalar_multiplication', {
  ...mat_scalar_cases,
  ...scalar_mat_cases,
});
