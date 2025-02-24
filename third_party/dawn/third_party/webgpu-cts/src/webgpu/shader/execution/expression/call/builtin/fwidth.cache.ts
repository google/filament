import { f32 } from '../../../../../util/conversion.js';
import { FP, FPInterval } from '../../../../../util/floating_point.js';
import { cartesianProduct, sparseScalarF32Range } from '../../../../../util/math.js';
import { Case } from '../../case.js';
import { makeCaseCache } from '../../case_cache.js';

function makeCase(v0: number, v1: number, v2: number, v3: number): Case {
  const expected: FPInterval[] = [
    (x: number, y: number, z: number, w: number) =>
      FP.f32.additionInterval(
        FP.f32.absInterval(FP.f32.subtractionInterval(x, y)),
        FP.f32.absInterval(FP.f32.subtractionInterval(x, z))
      ),
    (x: number, y: number, z: number, w: number) =>
      FP.f32.additionInterval(
        FP.f32.absInterval(FP.f32.subtractionInterval(x, y)),
        FP.f32.absInterval(FP.f32.subtractionInterval(y, w))
      ),
    (x: number, y: number, z: number, w: number) =>
      FP.f32.additionInterval(
        FP.f32.absInterval(FP.f32.subtractionInterval(z, w)),
        FP.f32.absInterval(FP.f32.subtractionInterval(x, z))
      ),
    (x: number, y: number, z: number, w: number) =>
      FP.f32.additionInterval(
        FP.f32.absInterval(FP.f32.subtractionInterval(z, w)),
        FP.f32.absInterval(FP.f32.subtractionInterval(y, w))
      ),
  ].map(o => o(v0, v1, v2, v3));

  return {
    input: [f32(v0), f32(v1), f32(v2), f32(v3)],
    expected,
  };
}

const cases = {
  scalar: () => {
    return cartesianProduct(
      sparseScalarF32Range(),
      sparseScalarF32Range(),
      sparseScalarF32Range(),
      sparseScalarF32Range()
    ).reduce((cases, e) => {
      const c = makeCase(e[0], e[1], e[2], e[3]);
      if (c !== undefined) {
        cases.push(c);
      }
      return cases;
    }, new Array<Case>());
  },
};

export const d = makeCaseCache('fwidth', cases);
