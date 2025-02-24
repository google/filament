export const description = `
Unit tests for parameterization helpers.
`;

import { TestParams } from '../common/framework/fixture.js';
import {
  kUnitCaseParamsBuilder,
  CaseSubcaseIterable,
  ParamsBuilderBase,
  builderIterateCasesWithSubcases,
} from '../common/framework/params_builder.js';
import { makeTestGroup } from '../common/framework/test_group.js';
import {
  mergeParams,
  mergeParamsChecked,
  publicParamsEquals,
} from '../common/internal/params_utils.js';
import { assert, objectEquals } from '../common/util/util.js';

import { UnitTest } from './unit_test.js';

class ParamsTest extends UnitTest {
  expectParams<CaseP extends {}, SubcaseP extends {}>(
    act: ParamsBuilderBase<CaseP, SubcaseP>,
    exp: CaseSubcaseIterable<{}, {}>,
    caseFilter: TestParams | null = null
  ): void {
    const a = Array.from(builderIterateCasesWithSubcases(act, caseFilter)).map(
      ([caseP, subcases]) => [caseP, subcases ? Array.from(subcases) : undefined]
    );
    const e = Array.from(exp);
    this.expect(
      objectEquals(a, e),
      `
got      ${JSON.stringify(a)}
expected ${JSON.stringify(e)}`
    );
  }
}

export const g = makeTestGroup(ParamsTest);

const u = kUnitCaseParamsBuilder;

g.test('combine').fn(t => {
  t.expectParams<{ hello: number }, {}>(u.combine('hello', [1, 2, 3]), [
    [{ hello: 1 }, undefined],
    [{ hello: 2 }, undefined],
    [{ hello: 3 }, undefined],
  ]);
  t.expectParams<{ hello: number }, {}>(
    u.combine('hello', [1, 2, 3]),
    [
      [{ hello: 1 }, undefined],
      [{ hello: 2 }, undefined],
      [{ hello: 3 }, undefined],
    ],
    {}
  );
  t.expectParams<{ hello: number }, {}>(
    u.combine('hello', [1, 2, 3]),
    [[{ hello: 2 }, undefined]],
    { hello: 2 }
  );
  t.expectParams<{ hello: 1 | 2 | 3 }, {}>(u.combine('hello', [1, 2, 3] as const), [
    [{ hello: 1 }, undefined],
    [{ hello: 2 }, undefined],
    [{ hello: 3 }, undefined],
  ]);
  t.expectParams<{}, { hello: number }>(u.beginSubcases().combine('hello', [1, 2, 3]), [
    [{}, [{ hello: 1 }, { hello: 2 }, { hello: 3 }]],
  ]);
  t.expectParams<{}, { hello: number }>(
    u.beginSubcases().combine('hello', [1, 2, 3]),
    [[{}, [{ hello: 1 }, { hello: 2 }, { hello: 3 }]]],
    {}
  );
  t.expectParams<{}, { hello: number }>(u.beginSubcases().combine('hello', [1, 2, 3]), [], {
    hello: 2,
  });
  t.expectParams<{}, { hello: 1 | 2 | 3 }>(u.beginSubcases().combine('hello', [1, 2, 3] as const), [
    [{}, [{ hello: 1 }, { hello: 2 }, { hello: 3 }]],
  ]);
});

g.test('empty').fn(t => {
  t.expectParams<{}, {}>(u, [
    [{}, undefined], //
  ]);
  t.expectParams<{}, {}>(u.beginSubcases(), [
    [{}, [{}]], //
  ]);
});

g.test('combine,zeroes_and_ones').fn(t => {
  t.expectParams<{}, {}>(u.combineWithParams([]).combineWithParams([]), []);
  t.expectParams<{}, {}>(u.combineWithParams([]).combineWithParams([{}]), []);
  t.expectParams<{}, {}>(u.combineWithParams([{}]).combineWithParams([]), []);
  t.expectParams<{}, {}>(u.combineWithParams([{}]).combineWithParams([{}]), [
    [{}, undefined], //
  ]);

  t.expectParams<{}, {}>(u.combine('x', []).combine('y', []), []);
  t.expectParams<{}, {}>(u.combine('x', []).combine('y', [1]), []);
  t.expectParams<{}, {}>(u.combine('x', [1]).combine('y', []), []);
  t.expectParams<{}, {}>(u.combine('x', [1]).combine('y', [1]), [
    [{ x: 1, y: 1 }, undefined], //
  ]);
});

g.test('combine,mixed').fn(t => {
  t.expectParams<{ x: number; y: string; p: number | undefined; q: number | undefined }, {}>(
    u
      .combine('x', [1, 2])
      .combine('y', ['a', 'b'])
      .combineWithParams([{ p: 4 }, { q: 5 }])
      .combineWithParams([{}]),
    [
      [{ x: 1, y: 'a', p: 4 }, undefined],
      [{ x: 1, y: 'a', q: 5 }, undefined],
      [{ x: 1, y: 'b', p: 4 }, undefined],
      [{ x: 1, y: 'b', q: 5 }, undefined],
      [{ x: 2, y: 'a', p: 4 }, undefined],
      [{ x: 2, y: 'a', q: 5 }, undefined],
      [{ x: 2, y: 'b', p: 4 }, undefined],
      [{ x: 2, y: 'b', q: 5 }, undefined],
    ]
  );
});

g.test('filter').fn(t => {
  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined }, {}>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .filter(p => p.a),
    [
      [{ a: true, x: 1 }, undefined], //
    ]
  );

  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined }, {}>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .beginSubcases()
      .filter(p => p.a),
    [
      [{ a: true, x: 1 }, [{}]], //
      // Case with no subcases is filtered out.
    ]
  );

  t.expectParams<{}, { a: boolean; x: number | undefined; y: number | undefined }>(
    u
      .beginSubcases()
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .filter(p => p.a),
    [
      [{}, [{ a: true, x: 1 }]], //
    ]
  );
});

g.test('unless').fn(t => {
  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined }, {}>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .unless(p => p.a),
    [
      [{ a: false, y: 2 }, undefined], //
    ]
  );

  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined }, {}>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .beginSubcases()
      .unless(p => p.a),
    [
      // Case with no subcases is filtered out.
      [{ a: false, y: 2 }, [{}]], //
    ]
  );

  t.expectParams<{}, { a: boolean; x: number | undefined; y: number | undefined }>(
    u
      .beginSubcases()
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .unless(p => p.a),
    [
      [{}, [{ a: false, y: 2 }]], //
    ]
  );
});

g.test('expandP').fn(t => {
  // simple
  t.expectParams<{}, {}>(
    u.expandWithParams(function* () {}),
    []
  );
  t.expectParams<{}, {}>(
    u.expandWithParams(function* () {
      yield {};
    }),
    [[{}, undefined]]
  );
  t.expectParams<{ z: number | undefined; w: number | undefined }, {}>(
    u.expandWithParams(function* () {
      yield* kUnitCaseParamsBuilder.combine('z', [3, 4]);
      yield { w: 5 };
    }),
    [
      [{ z: 3 }, undefined],
      [{ z: 4 }, undefined],
      [{ w: 5 }, undefined],
    ]
  );
  t.expectParams<{ z: number | undefined; w: number | undefined }, {}>(
    u.expandWithParams(function* () {
      yield* kUnitCaseParamsBuilder.combine('z', [3, 4]);
      yield { w: 5 };
    }),
    [
      [{ z: 3 }, undefined],
      [{ z: 4 }, undefined],
      [{ w: 5 }, undefined],
    ],
    {}
  );
  t.expectParams<{ z: number | undefined; w: number | undefined }, {}>(
    u.expandWithParams(function* () {
      yield* kUnitCaseParamsBuilder.combine('z', [3, 4]);
      yield { w: 5 };
    }),
    [[{ z: 4 }, undefined]],
    { z: 4 }
  );
  t.expectParams<{ z: number | undefined; w: number | undefined }, {}>(
    u.expandWithParams(function* () {
      yield* kUnitCaseParamsBuilder.combine('z', [3, 4]);
      yield { w: 5 };
    }),
    [[{ z: 3 }, undefined]],
    { z: 3 }
  );
  t.expectParams<{}, { z: number | undefined; w: number | undefined }>(
    u.beginSubcases().expandWithParams(function* () {
      yield* kUnitCaseParamsBuilder.combine('z', [3, 4]);
      yield { w: 5 };
    }),
    [[{}, [{ z: 3 }, { z: 4 }, { w: 5 }]]]
  );

  t.expectParams<{ x: [] | {} }, {}>(
    u.expand('x', () => [[], {}] as const),
    [
      [{ x: [] }, undefined],
      [{ x: {} }, undefined],
    ]
  );
  t.expectParams<{ x: [] | {} }, {}>(
    u.expand('x', () => [[], {}] as const),
    [[{ x: [] }, undefined]],
    { x: [] }
  );
  t.expectParams<{ x: [] | {} }, {}>(
    u.expand('x', () => [[], {}] as const),
    [[{ x: {} }, undefined]],
    { x: {} }
  );

  // more complex
  {
    const p = u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .expandWithParams(function* (p) {
        if (p.a) {
          yield { z: 3 };
          yield { z: 4 };
        } else {
          yield { w: 5 };
        }
      });
    type T = {
      a: boolean;
      x: number | undefined;
      y: number | undefined;
      z: number | undefined;
      w: number | undefined;
    };
    t.expectParams<T, {}>(p, [
      [{ a: true, x: 1, z: 3 }, undefined],
      [{ a: true, x: 1, z: 4 }, undefined],
      [{ a: false, y: 2, w: 5 }, undefined],
    ]);
    t.expectParams<T, {}>(
      p,
      [
        [{ a: true, x: 1, z: 3 }, undefined],
        [{ a: true, x: 1, z: 4 }, undefined],
        [{ a: false, y: 2, w: 5 }, undefined],
      ],
      {}
    );
    t.expectParams<T, {}>(
      p,
      [
        [{ a: true, x: 1, z: 3 }, undefined],
        [{ a: true, x: 1, z: 4 }, undefined],
      ],
      { a: true }
    );
    t.expectParams<T, {}>(p, [[{ a: false, y: 2, w: 5 }, undefined]], { a: false });
  }

  t.expectParams<
    { a: boolean; x: number | undefined; y: number | undefined },
    { z: number | undefined; w: number | undefined }
  >(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .beginSubcases()
      .expandWithParams(function* (p) {
        if (p.a) {
          yield { z: 3 };
          yield { z: 4 };
        } else {
          yield { w: 5 };
        }
      }),
    [
      [{ a: true, x: 1 }, [{ z: 3 }, { z: 4 }]],
      [{ a: false, y: 2 }, [{ w: 5 }]],
    ]
  );
});

g.test('expand').fn(t => {
  // simple
  t.expectParams<{}, {}>(
    u.expand('x', function* () {}),
    []
  );
  t.expectParams<{ z: number }, {}>(
    u.expand('z', function* () {
      yield 3;
      yield 4;
    }),
    [
      [{ z: 3 }, undefined],
      [{ z: 4 }, undefined],
    ]
  );
  t.expectParams<{ z: number }, {}>(
    u.expand('z', function* () {
      yield 3;
      yield 4;
    }),
    [
      [{ z: 3 }, undefined],
      [{ z: 4 }, undefined],
    ],
    {}
  );
  t.expectParams<{ z: number }, {}>(
    u.expand('z', function* () {
      yield 3;
      yield 4;
    }),
    [[{ z: 3 }, undefined]],
    { z: 3 }
  );
  t.expectParams<{}, { z: number }>(
    u.beginSubcases().expand('z', function* () {
      yield 3;
      yield 4;
    }),
    [[{}, [{ z: 3 }, { z: 4 }]]]
  );

  // more complex
  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined; z: number }, {}>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .expand('z', function* (p) {
        if (p.a) {
          yield 3;
        } else {
          yield 5;
        }
      }),
    [
      [{ a: true, x: 1, z: 3 }, undefined],
      [{ a: false, y: 2, z: 5 }, undefined],
    ]
  );
  t.expectParams<{ a: boolean; x: number | undefined; y: number | undefined }, { z: number }>(
    u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, y: 2 },
      ])
      .beginSubcases()
      .expand('z', function* (p) {
        if (p.a) {
          yield 3;
        } else {
          yield 5;
        }
      }),
    [
      [{ a: true, x: 1 }, [{ z: 3 }]],
      [{ a: false, y: 2 }, [{ z: 5 }]],
    ]
  );
});

g.test('invalid,shadowing').fn(t => {
  // Existing CaseP is shadowed by a new CaseP.
  {
    const p = u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, x: 2 },
      ])
      .expandWithParams(function* (p) {
        if (p.a) {
          yield { x: 3 };
        } else {
          yield { w: 5 };
        }
      });
    // Iterating causes merging e.g. ({x:1}, {x:3}), which fails.
    t.shouldThrow('Error', () => {
      Array.from(p.iterateCasesWithSubcases(null));
    });
  }
  // Existing SubcaseP is shadowed by a new SubcaseP.
  {
    const p = u
      .beginSubcases()
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, x: 2 },
      ])
      .expandWithParams(function* (p) {
        if (p.a) {
          yield { x: 3 };
        } else {
          yield { w: 5 };
        }
      });
    // Iterating causes merging e.g. ({x:1}, {x:3}), which fails.
    t.shouldThrow('Error', () => {
      Array.from(p.iterateCasesWithSubcases(null));
    });
  }
  // Existing CaseP is shadowed by a new SubcaseP.
  {
    const p = u
      .combineWithParams([
        { a: true, x: 1 },
        { a: false, x: 2 },
      ])
      .beginSubcases()
      .expandWithParams(function* (p) {
        if (p.a) {
          yield { x: 3 };
        } else {
          yield { w: 5 };
        }
      });
    const cases = Array.from(p.iterateCasesWithSubcases(null));
    // Iterating cases is fine...
    for (const [caseP, subcases] of cases) {
      assert(subcases !== undefined);
      // Iterating subcases is fine...
      for (const subcaseP of subcases) {
        if (caseP.a) {
          assert(subcases !== undefined);

          // Only errors once we try to merge e.g. ({x:1}, {x:3}).
          mergeParams(caseP, subcaseP);
          t.shouldThrow('Error', () => {
            mergeParamsChecked(caseP, subcaseP);
          });
        }
      }
    }
  }
});

g.test('undefined').fn(t => {
  t.expect(!publicParamsEquals({ a: undefined }, {}));
  t.expect(!publicParamsEquals({}, { a: undefined }));
});

g.test('private').fn(t => {
  t.expect(publicParamsEquals({ _a: 0 }, {}));
  t.expect(publicParamsEquals({}, { _a: 0 }));
});

g.test('value,array').fn(t => {
  t.expectParams<{ a: number[] }, {}>(u.combineWithParams([{ a: [1, 2] }]), [
    [{ a: [1, 2] }, undefined], //
  ]);
  t.expectParams<{}, { a: number[] }>(u.beginSubcases().combineWithParams([{ a: [1, 2] }]), [
    [{}, [{ a: [1, 2] }]], //
  ]);
});

g.test('value,object').fn(t => {
  t.expectParams<{ a: { [k: string]: number } }, {}>(u.combineWithParams([{ a: { x: 1 } }]), [
    [{ a: { x: 1 } }, undefined], //
  ]);
  t.expectParams<{}, { a: { [k: string]: number } }>(
    u.beginSubcases().combineWithParams([{ a: { x: 1 } }]),
    [
      [{}, [{ a: { x: 1 } }]], //
    ]
  );
});
