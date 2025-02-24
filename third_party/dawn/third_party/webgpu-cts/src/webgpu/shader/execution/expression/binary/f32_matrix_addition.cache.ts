import { FP } from '../../../../util/floating_point.js';
import { sparseMatrixF32Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

// Cases: matCxR_[non_]const
const mat_cases = ([2, 3, 4] as const)
  .flatMap(cols =>
    ([2, 3, 4] as const).flatMap(rows =>
      ([true, false] as const).map(nonConst => ({
        [`mat${cols}x${rows}_${nonConst ? 'non_const' : 'const'}`]: () => {
          return FP.f32.generateMatrixPairToMatrixCases(
            sparseMatrixF32Range(cols, rows),
            sparseMatrixF32Range(cols, rows),
            nonConst ? 'unfiltered' : 'finite',
            FP.f32.additionMatrixMatrixInterval
          );
        },
      }))
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/f32_matrix_addition', mat_cases);
