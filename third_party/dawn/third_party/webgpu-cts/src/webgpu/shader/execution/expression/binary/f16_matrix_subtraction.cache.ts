import { FP } from '../../../../util/floating_point.js';
import { sparseMatrixF16Range } from '../../../../util/math.js';
import { makeCaseCache } from '../case_cache.js';

// Cases: matCxR_[non_]const
const mat_cases = ([2, 3, 4] as const)
  .flatMap(cols =>
    ([2, 3, 4] as const).flatMap(rows =>
      ([true, false] as const).map(nonConst => ({
        [`mat${cols}x${rows}_${nonConst ? 'non_const' : 'const'}`]: () => {
          return FP.f16.generateMatrixPairToMatrixCases(
            sparseMatrixF16Range(cols, rows),
            sparseMatrixF16Range(cols, rows),
            nonConst ? 'unfiltered' : 'finite',
            FP.f16.subtractionMatrixMatrixInterval
          );
        },
      }))
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('binary/f16_matrix_subtraction', mat_cases);
