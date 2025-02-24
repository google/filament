import { FP } from '../../../../../util/floating_point.js';
import { makeCaseCache } from '../../case_cache.js';

// Cases: [f32|f16|abstract]
const cases = (['f32', 'f16', 'abstract'] as const)
  .map(trait => ({
    [`${trait}`]: () => {
      return FP[trait].generateScalarToIntervalCases(
        FP[trait].scalarRange(),
        trait !== 'abstract' ? 'unfiltered' : 'finite',
        // radians has an inherited accuracy, so abstract is only expected to be as accurate as f32
        FP[trait !== 'abstract' ? trait : 'f32'].radiansInterval
      );
    },
  }))
  .reduce((a, b) => ({ ...a, ...b }), {});

export const d = makeCaseCache('radians', cases);
