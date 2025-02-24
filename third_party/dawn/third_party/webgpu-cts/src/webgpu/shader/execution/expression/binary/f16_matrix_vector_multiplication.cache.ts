import { FP } from '../../../../util/floating_point.js';
import { sparseMatrixF16Range, sparseVectorF16Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

// Cases: matCxR_vecC_[non_]const
const mat_vec_cases = ([2, 3, 4] as const)
  .flatMap(cols =>
    ([2, 3, 4] as const).flatMap(rows =>
      ([true, false] as const).map(nonConst => ({
        [`mat${cols}x${rows}_vec${cols}_${nonConst ? 'non_const' : 'const'}`]: () => {
          return FP.f16.generateMatrixVectorToVectorCases(
            sparseMatrixF16Range(cols, rows),
            sparseVectorF16Range(cols),
            nonConst ? 'unfiltered' : 'finite',
            FP.f16.multiplicationMatrixVectorInterval
          );
        },
      }))
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

// Cases: vecR_matCxR_[non_]const
const vec_mat_cases = ([2, 3, 4] as const)
  .flatMap(rows =>
    ([2, 3, 4] as const).flatMap(cols =>
      ([true, false] as const).map(nonConst => ({
        [`vec${rows}_mat${cols}x${rows}_${nonConst ? 'non_const' : 'const'}`]: () => {
          return FP.f16.generateVectorMatrixToVectorCases(
            sparseVectorF16Range(rows),
            sparseMatrixF16Range(cols, rows),
            nonConst ? 'unfiltered' : 'finite',
            FP.f16.multiplicationVectorMatrixInterval
          );
        },
      }))
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/f16_matrix_vector_multiplication', {
  ...mat_vec_cases,
  ...vec_mat_cases,
});
