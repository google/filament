export const description = `
Unittests for the pseudo random number generator
`;

import { makeTestGroup } from '../common/framework/test_group.js';
import { fullU32Range } from '../webgpu/util/math.js';
import { PRNG } from '../webgpu/util/prng.js';

import { UnitTest } from './unit_test.js';

export const g = makeTestGroup(UnitTest);

// There exist more formal tests for the quality of random number generators
// that are out of the scope for testing here (and are checked against the
// original C implementation).
// These tests are just intended to be smoke tests for implementation.

// Test against the reference u32 values from the original C implementation
// https://github.com/MersenneTwister-Lab/TinyMT/blob/master/tinymt/check32.out.txt
g.test('check').fn(t => {
  const p = new PRNG(1);
  // prettier-ignore
  const expected = [
    2545341989, 981918433,  3715302833, 2387538352, 3591001365,
    3820442102, 2114400566, 2196103051, 2783359912, 764534509,
    643179475,  1822416315, 881558334,  4207026366, 3690273640,
    3240535687, 2921447122, 3984931427, 4092394160, 44209675,
    2188315343, 2908663843, 1834519336, 3774670961, 3019990707,
    4065554902, 1239765502, 4035716197, 3412127188, 552822483,
    161364450,  353727785,  140085994,  149132008,  2547770827,
    4064042525, 4078297538, 2057335507, 622384752,  2041665899,
    2193913817, 1080849512, 33160901,  662956935,   642999063,
    3384709977, 1723175122, 3866752252, 521822317,  2292524454,
  ];
  expected.forEach((_, i) => {
    const val = p.randomU32();
    t.expect(
      val === expected[i],
      `PRNG(1) failed produced the ${i}th expected item, ${val} instead of ${expected[i]})`
    );
  });
});

// Prove that generator is deterministic for at least 1000 values with different
// seeds.
g.test('deterministic_random').fn(t => {
  fullU32Range().forEach(seed => {
    const lhs = new PRNG(seed);
    const rhs = new PRNG(seed);
    for (let i = 0; i < 1000; i++) {
      const lhs_val = lhs.random();
      const rhs_val = rhs.random();
      t.expect(
        lhs_val === rhs_val,
        `For seed ${seed}, the ${i}th item, PRNG was non-deterministic (${lhs_val} vs ${rhs_val})`
      );
    }
  });
});

g.test('deterministic_randomU32').fn(t => {
  fullU32Range().forEach(seed => {
    const lhs = new PRNG(seed);
    const rhs = new PRNG(seed);
    for (let i = 0; i < 1000; i++) {
      const lhs_val = lhs.randomU32();
      const rhs_val = rhs.randomU32();
      t.expect(
        lhs_val === rhs_val,
        `For seed ${seed}, the ${i}th item, PRNG was non-deterministic (${lhs_val} vs ${rhs_val})`
      );
    }
  });
});

// Returns 2**k, for integer k up to and including 32.
function power_of_2(k: number) {
  // The shift operator on integers returns a signed 32 bit integer.
  // So break up this calculation to avoid wraparound.
  if (k < 30) {
    return 1 << k;
  }
  return (1 << 30) * (1 << (k - 30));
}

g.test('uniformInt_range')
  .desc('Outputs of uniformInt(N) are between 0 and N-1')
  .fn(t => {
    [1, 42, 99].forEach(seed => {
      const p = new PRNG(seed);
      for (let k = 0; k < 32; k++) {
        const N = power_of_2(k);
        for (let i = 0; i < 20; i++) {
          const sample = p.uniformInt(N);
          t.expect(
            0 <= sample && sample < N,
            `Sample from [0, ${N - 1}] is out of bounds: ${sample}`
          );
          t.expect(sample === Math.trunc(sample), `Sample should be an integer: ${sample}`);
        }
      }
    });
  });

g.test('uniformInt_distribution')
  .desc('uniformInt outputs are not biased: histogram counts are close to the expected mean')
  .fn(t => {
    const p = new PRNG(42);
    const numBins = 4;
    const numSamples = 1000;
    const histogram = Array(numBins).fill(0);
    for (let i = 0; i < numSamples; i++) {
      histogram[p.uniformInt(numBins)]++;
    }
    // Each bin should have roughly the expected number of hits.
    const meanCount = numSamples / numBins;
    const toleratedMin = meanCount * 0.9;
    const toleratedMax = meanCount * 1.1;
    histogram.forEach(count => {
      t.expect(count >= toleratedMin, `count is ${count}, less than tolerated min ${toleratedMin}`);
      t.expect(count <= toleratedMax, `count is ${count}, more than tolerated min ${toleratedMax}`);
    });
  });

g.test('uniformInt_bias')
  .desc('uniformInt does not demonstrate bias expected of randInt() % N')
  .fn(t => {
    const p = new PRNG(43);
    // A bad random generator would be: randomU32() % N.
    // For N = (2**32) * 2/3, we would expect the result to be less than N/2 about 2/3 of the time.
    const badN = Math.trunc((1 << 15) * ((1 << 16) / 3));
    const halfN = badN / 2;
    let numSmall = 0;
    const numSamples = 1000;
    for (let i = 0; i < numSamples; i++) {
      const val = p.uniformInt(badN);
      if (val < halfN) {
        numSmall++;
      }
    }
    t.expect(
      numSmall > 0.45 * numSamples,
      `uniformInt is biased: too few small samples (${numSmall} / ${numSamples})`
    );
    t.expect(
      numSmall < 0.55 * numSamples,
      `uniformInt is biased: too many big samples (${numSamples - numSmall} / ${numSamples})`
    );
  });
