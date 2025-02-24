import { FP } from '../../../../../util/floating_point.js';
import { makeCaseCache } from '../../case_cache.js';

// Cases: [f32|f16|abstract]_matCxR_[non_]const
// abstract_matCxR_non_const is empty and not used
const cases = (['f32', 'f16', 'abstract'] as const)
  .flatMap(trait =>
    ([2, 3, 4] as const).flatMap(cols =>
      ([2, 3, 4] as const).flatMap(rows =>
        ([true, false] as const).map(nonConst => ({
          [`${trait}_mat${cols}x${rows}_${nonConst ? 'non_const' : 'const'}`]: () => {
            if (trait === 'abstract' && nonConst) {
              return [];
            }
            return FP[trait].generateMatrixToMatrixCases(
              FP[trait].sparseMatrixRange(cols, rows),
              nonConst ? 'unfiltered' : 'finite',
              FP[trait].transposeInterval
            );
          },
        }))
      )
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('transpose', cases);
