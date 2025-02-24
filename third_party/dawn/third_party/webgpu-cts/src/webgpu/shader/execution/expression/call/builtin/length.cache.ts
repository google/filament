import { FP } from '../../../../../util/floating_point.js';
import { makeCaseCache } from '../../case_cache.js';

// Cases: [f32|f16|abstract]_[non_]const
const scalar_cases = (['f32', 'f16', 'abstract'] as const)
  .map(trait => ({
    [`${trait}`]: () => {
      return FP[trait].generateScalarToIntervalCases(
        FP[trait].scalarRange(),
        trait !== 'abstract' ? 'unfiltered' : 'finite',
        // length has an inherited accuracy, so is only expected to be as accurate as f32
        FP[trait !== 'abstract' ? trait : 'f32'].lengthInterval
      );
    },
  }))
  .reduce((a, b) => ({ ...a, ...b }), {});

// Cases: [f32|f16|abstract]_vecN_[non_]const
const vec_cases = (['f32', 'f16', 'abstract'] as const)
  .flatMap(trait =>
    ([2, 3, 4] as const).flatMap(dim =>
      ([true, false] as const).map(nonConst => ({
        [`${trait}_vec${dim}_${nonConst ? 'non_const' : 'const'}`]: () => {
          if (trait === 'abstract' && nonConst) {
            return [];
          }
          return FP[trait].generateVectorToIntervalCases(
            FP[trait].vectorRange(dim),
            nonConst ? 'unfiltered' : 'finite',
            // length has an inherited accuracy, so is only expected to be as accurate as f32
            FP[trait !== 'abstract' ? trait : 'f32'].lengthInterval
          );
        },
      }))
    )
  )
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('length', {
  ...scalar_cases,
  ...vec_cases,
});
