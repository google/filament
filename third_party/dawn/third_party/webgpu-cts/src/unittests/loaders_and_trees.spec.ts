export const description = `
Tests for queries/filtering, loading, and running.
`;

import { Fixture } from '../common/framework/fixture.js';
import { makeTestGroup } from '../common/framework/test_group.js';
import { TestFileLoader, SpecFile } from '../common/internal/file_loader.js';
import { Logger } from '../common/internal/logging/logger.js';
import { Status } from '../common/internal/logging/result.js';
import { parseQuery } from '../common/internal/query/parseQuery.js';
import {
  TestQuery,
  TestQuerySingleCase,
  TestQueryMultiCase,
  TestQueryMultiTest,
  TestQueryMultiFile,
  TestQueryWithExpectation,
} from '../common/internal/query/query.js';
import { makeTestGroupForUnitTesting } from '../common/internal/test_group.js';
import { TestSuiteListing, TestSuiteListingEntry } from '../common/internal/test_suite_listing.js';
import { ExpandThroughLevel, TestTreeLeaf } from '../common/internal/tree.js';
import { assert, objectEquals } from '../common/util/util.js';

import { UnitTest } from './unit_test.js';

const listingData: { [k: string]: TestSuiteListingEntry[] } = {
  suite1: [
    { file: [], readme: 'desc 1a' },
    { file: ['foo'] },
    { file: ['bar'], readme: 'desc 1h' },
    { file: ['bar', 'biz'] },
    { file: ['bar', 'buzz', 'buzz'] },
    { file: ['baz'] },
    { file: ['empty'], readme: 'desc 1z' }, // directory with no files
  ],
  suite2: [{ file: [], readme: 'desc 2a' }, { file: ['foof'] }],
};

const specsData: { [k: string]: SpecFile } = {
  'suite1/foo.spec.js': {
    description: 'desc 1b',
    g: (() => {
      const g = makeTestGroupForUnitTesting(UnitTest);
      g.test('hello').fn(() => {});
      g.test('bonjour').fn(() => {});
      g.test('hola')
        .desc('TODO TODO')
        .fn(() => {});
      return g;
    })(),
  },
  'suite1/bar/biz.spec.js': {
    description: 'desc 1f TODO TODO',
    g: makeTestGroupForUnitTesting(UnitTest), // file with no tests
  },
  'suite1/bar/buzz/buzz.spec.js': {
    description: 'desc 1d TODO',
    g: (() => {
      const g = makeTestGroupForUnitTesting(UnitTest);
      g.test('zap').fn(() => {});
      return g;
    })(),
  },
  'suite1/baz.spec.js': {
    description: 'desc 1e',
    g: (() => {
      const g = makeTestGroupForUnitTesting(UnitTest);
      g.test('wye')
        .paramsSimple([{}, { x: 1 }])
        .fn(() => {});
      g.test('zed')
        .paramsSimple([
          { a: 1, b: 2, _c: 0 },
          { b: 3, a: 1, _c: 0 },
        ])
        .fn(() => {});
      g.test('batched')
        // creates two cases: one for subcases 1,2 and one for subcase 3
        .paramsSubcasesOnly(u => u.combine('x', [1, 2, 3]))
        .batch(2)
        .fn(() => {});
      return g;
    })(),
  },
  'suite2/foof.spec.js': {
    description: 'desc 2b',
    g: (() => {
      const g = makeTestGroupForUnitTesting(UnitTest);
      g.test('blah').fn(t => {
        t.debug('OK');
      });
      g.test('bleh')
        .paramsSimple([{ a: 1 }])
        .fn(t => {
          t.debug('OK');
          t.debug('OK');
        });
      g.test('bluh,a').fn(t => {
        t.fail('goodbye');
      });
      return g;
    })(),
  },
};

class FakeTestFileLoader extends TestFileLoader {
  listing(suite: string): Promise<TestSuiteListing> {
    return Promise.resolve(listingData[suite]);
  }

  import(path: string): Promise<SpecFile> {
    assert(path in specsData, '[test] mock file ' + path + ' does not exist');
    return Promise.resolve(specsData[path]);
  }
}

class LoadingTest extends UnitTest {
  loader: FakeTestFileLoader = new FakeTestFileLoader();
  events: (string | null)[] = [];
  private isListenersAdded = false;

  collectEvents(): void {
    this.events = [];
    if (!this.isListenersAdded) {
      this.isListenersAdded = true;
      this.loader.addEventListener('import', ev => this.events.push(ev.data.url));
      this.loader.addEventListener('finish', _ev => this.events.push(null));
    }
  }

  async load(query: string): Promise<TestTreeLeaf[]> {
    return Array.from(await this.loader.loadCases(parseQuery(query)));
  }

  async loadNames(query: string): Promise<string[]> {
    return (await this.load(query)).map(c => c.query.toString());
  }
}

export const g = makeTestGroup(LoadingTest);

g.test('suite').fn(t => {
  t.shouldReject('Error', t.load('suite1'));
  t.shouldReject('Error', t.load('suite1:'));
});

g.test('group').fn(async t => {
  t.collectEvents();
  t.expect((await t.load('suite1:*')).length === 10);
  t.expect(
    objectEquals(t.events, [
      'suite1/foo.spec.js',
      'suite1/bar/biz.spec.js',
      'suite1/bar/buzz/buzz.spec.js',
      'suite1/baz.spec.js',
      null,
    ])
  );

  t.collectEvents();
  t.expect((await t.load('suite1:foo,*')).length === 3); // x:foo,* matches x:foo:
  t.expect(objectEquals(t.events, ['suite1/foo.spec.js', null]));

  t.collectEvents();
  t.expect((await t.load('suite1:bar,*')).length === 1);
  t.expect(
    objectEquals(t.events, ['suite1/bar/biz.spec.js', 'suite1/bar/buzz/buzz.spec.js', null])
  );

  t.collectEvents();
  t.expect((await t.load('suite1:bar,buzz,buzz,*')).length === 1);
  t.expect(objectEquals(t.events, ['suite1/bar/buzz/buzz.spec.js', null]));

  t.shouldReject('Error', t.load('suite1:f*'));

  {
    const s = new TestQueryMultiFile('suite1', ['bar', 'buzz']).toString();
    t.collectEvents();
    t.expect((await t.load(s)).length === 1);
    t.expect(objectEquals(t.events, ['suite1/bar/buzz/buzz.spec.js', null]));
  }
});

g.test('test').fn(async t => {
  t.shouldReject('Error', t.load('suite1::'));
  t.shouldReject('Error', t.load('suite1:bar:'));
  t.shouldReject('Error', t.load('suite1:bar,:'));

  t.shouldReject('Error', t.load('suite1::*'));
  t.shouldReject('Error', t.load('suite1:bar,:*'));
  t.shouldReject('Error', t.load('suite1:bar:*'));

  t.expect((await t.load('suite1:foo:*')).length === 3);
  t.expect((await t.load('suite1:bar,buzz,buzz:*')).length === 1);
  t.expect((await t.load('suite1:baz:*')).length === 6);

  t.expect((await t.load('suite2:foof:bluh,*')).length === 1);
  t.expect((await t.load('suite2:foof:bluh,a,*')).length === 1);

  {
    const s = new TestQueryMultiTest('suite2', ['foof'], ['bluh']).toString();
    t.expect((await t.load(s)).length === 1);
  }
});

g.test('case').fn(async t => {
  t.shouldReject('Error', t.load('suite1:foo::'));
  t.shouldReject('Error', t.load('suite1:bar:zed,:'));

  t.shouldReject('Error', t.load('suite1:foo:h*'));

  t.shouldReject('Error', t.load('suite1:foo::*'));
  t.shouldReject('Error', t.load('suite1:baz::*'));
  t.shouldReject('Error', t.load('suite1:baz:zed,:*'));

  t.shouldReject('Error', t.load('suite1:baz:zed:'));
  t.shouldReject('Error', t.load('suite1:baz:zed:a=1'));
  t.shouldReject('Error', t.load('suite1:baz:zed:a=1;b=2*'));
  t.shouldReject('Error', t.load('suite1:baz:zed:a=1;b=2;'));
  t.shouldReject('SyntaxError', t.load('suite1:baz:zed:a=1;b=2,')); // tries to parse '2,' as JSON
  t.shouldReject('Error', t.load('suite1:baz:zed:a=1,b=2')); // '=' not allowed in value '1,b=2'
  t.shouldReject('Error', t.load('suite1:baz:zed:b=2*'));
  t.shouldReject('Error', t.load('suite1:baz:zed:b=2;a=1;_c=0'));
  t.shouldReject('Error', t.load('suite1:baz:zed:a=1,*'));

  t.expect((await t.load('suite1:baz:zed:*')).length === 2);
  t.expect((await t.load('suite1:baz:zed:a=1;*')).length === 2);
  t.expect((await t.load('suite1:baz:zed:a=1;b=2')).length === 1);
  t.expect((await t.load('suite1:baz:zed:a=1;b=2;*')).length === 1);
  t.expect((await t.load('suite1:baz:zed:b=2;*')).length === 1);
  t.expect((await t.load('suite1:baz:zed:b=2;a=1')).length === 1);
  t.expect((await t.load('suite1:baz:zed:b=2;a=1;*')).length === 1);
  t.expect((await t.load('suite1:baz:zed:b=3;a=1')).length === 1);
  t.expect((await t.load('suite1:baz:zed:a=1;b=3')).length === 1);
  t.expect((await t.load('suite1:foo:hello:')).length === 1);

  {
    const s = new TestQueryMultiCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }).toString();
    t.expect((await t.load(s)).length === 1);
  }
  {
    const s = new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }).toString();
    t.expect((await t.load(s)).length === 1);
  }
});

g.test('batching').fn(async t => {
  t.expect((await t.load('suite1:baz:batched,*')).length === 2);
  t.expect((await t.load('suite1:baz:batched:*')).length === 2);
  t.expect((await t.load('suite1:baz:batched:batch__=1;*')).length === 1);
  t.expect((await t.load('suite1:baz:batched:batch__=1')).length === 1);
});

async function runTestcase(
  t: Fixture,
  log: Logger,
  testcases: TestTreeLeaf[],
  i: number,
  query: TestQuery,
  expectations: TestQueryWithExpectation[],
  status: Status,
  logs: (s: string[]) => boolean
) {
  t.expect(objectEquals(testcases[i].query, query));
  const name = testcases[i].query.toString();
  const [rec, res] = log.record(name);
  await testcases[i].run(rec, expectations);

  t.expect(log.results.get(name) === res);
  t.expect(res.status === status);
  t.expect(res.timems >= 0);
  assert(res.logs !== undefined); // only undefined while pending
  t.expect(logs(res.logs.map(l => JSON.stringify(l))));
}

g.test('end2end').fn(async t => {
  const l = await t.load('suite2:foof:*');
  assert(l.length === 3, 'listing length');

  const log = new Logger({ overrideDebugMode: true });

  await runTestcase(
    t,
    log,
    l,
    0,
    new TestQuerySingleCase('suite2', ['foof'], ['blah'], {}),
    [],
    'pass',
    logs => objectEquals(logs, ['"DEBUG: OK"'])
  );
  await runTestcase(
    t,
    log,
    l,
    1,
    new TestQuerySingleCase('suite2', ['foof'], ['bleh'], { a: 1 }),
    [],
    'pass',
    logs => objectEquals(logs, ['"DEBUG: OK"', '"DEBUG: OK"'])
  );
  await runTestcase(
    t,
    log,
    l,
    2,
    new TestQuerySingleCase('suite2', ['foof'], ['bluh', 'a'], {}),
    [],
    'fail',
    logs =>
      logs.length === 1 &&
      logs[0].startsWith('"EXPECTATION FAILED: goodbye\\n') &&
      logs[0].indexOf('loaders_and_trees.spec.') !== -1
  );
});

g.test('expectations,single_case').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const zedCases = await t.load('suite1:baz:zed:*');

  // Single-case. Covers one case.
  const zedExpectationsSkipA1B2 = [
    {
      query: new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    zedCases,
    0,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    zedExpectationsSkipA1B2,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    zedCases,
    1,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 3 }),
    zedExpectationsSkipA1B2,
    'pass',
    logs => logs.length === 0
  );
});

g.test('expectations,single_case,none').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const zedCases = await t.load('suite1:baz:zed:*');
  // Single-case. Doesn't cover any cases.
  const zedExpectationsSkipA1B0 = [
    {
      query: new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 0 }),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    zedCases,
    0,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    zedExpectationsSkipA1B0,
    'pass',
    logs => logs.length === 0
  );

  await runTestcase(
    t,
    log,
    zedCases,
    1,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 3 }),
    zedExpectationsSkipA1B0,
    'pass',
    logs => logs.length === 0
  );
});

g.test('expectations,multi_case').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const zedCases = await t.load('suite1:baz:zed:*');
  // Multi-case, not all cases covered.
  const zedExpectationsSkipB3 = [
    {
      query: new TestQueryMultiCase('suite1', ['baz'], ['zed'], { b: 3 }),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    zedCases,
    0,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    zedExpectationsSkipB3,
    'pass',
    logs => logs.length === 0
  );

  await runTestcase(
    t,
    log,
    zedCases,
    1,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 3 }),
    zedExpectationsSkipB3,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,multi_case_all').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const zedCases = await t.load('suite1:baz:zed:*');
  // Multi-case, all cases covered.
  const zedExpectationsSkipA1 = [
    {
      query: new TestQueryMultiCase('suite1', ['baz'], ['zed'], { a: 1 }),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    zedCases,
    0,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    zedExpectationsSkipA1,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    zedCases,
    1,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 3 }),
    zedExpectationsSkipA1,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,multi_case_none').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const zedCases = await t.load('suite1:baz:zed:*');
  // Multi-case, no params, all cases covered.
  const zedExpectationsSkipZed = [
    {
      query: new TestQueryMultiCase('suite1', ['baz'], ['zed'], {}),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    zedCases,
    0,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    zedExpectationsSkipZed,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    zedCases,
    1,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 3 }),
    zedExpectationsSkipZed,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,multi_test').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite1Cases = await t.load('suite1:*');

  // Multi-test, all cases covered.
  const expectationsSkipAllInBaz = [
    {
      query: new TestQueryMultiTest('suite1', ['baz'], []),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite1Cases,
    4,
    new TestQuerySingleCase('suite1', ['baz'], ['wye'], {}),
    expectationsSkipAllInBaz,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    suite1Cases,
    6,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    expectationsSkipAllInBaz,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,multi_test,none').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite1Cases = await t.load('suite1:*');

  // Multi-test, no cases covered.
  const expectationsSkipAllInFoo = [
    {
      query: new TestQueryMultiTest('suite1', ['foo'], []),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite1Cases,
    4,
    new TestQuerySingleCase('suite1', ['baz'], ['wye'], {}),
    expectationsSkipAllInFoo,
    'pass',
    logs => logs.length === 0
  );

  await runTestcase(
    t,
    log,
    suite1Cases,
    6,
    new TestQuerySingleCase('suite1', ['baz'], ['zed'], { a: 1, b: 2 }),
    expectationsSkipAllInFoo,
    'pass',
    logs => logs.length === 0
  );
});

g.test('expectations,multi_file').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite1Cases = await t.load('suite1:*');

  // Multi-file
  const expectationsSkipAll = [
    {
      query: new TestQueryMultiFile('suite1', []),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite1Cases,
    0,
    new TestQuerySingleCase('suite1', ['foo'], ['hello'], {}),
    expectationsSkipAll,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    suite1Cases,
    3,
    new TestQuerySingleCase('suite1', ['bar', 'buzz', 'buzz'], ['zap'], {}),
    expectationsSkipAll,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,catches_failure').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite2Cases = await t.load('suite2:*');

  // Catches failure
  const expectedFailures = [
    {
      query: new TestQueryMultiCase('suite2', ['foof'], ['bluh', 'a'], {}),
      expectation: 'fail' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite2Cases,
    0,
    new TestQuerySingleCase('suite2', ['foof'], ['blah'], {}),
    expectedFailures,
    'pass',
    logs => objectEquals(logs, ['"DEBUG: OK"'])
  );

  // Status is passed, but failure is logged.
  await runTestcase(
    t,
    log,
    suite2Cases,
    2,
    new TestQuerySingleCase('suite2', ['foof'], ['bluh', 'a'], {}),
    expectedFailures,
    'pass',
    logs => logs.length === 1 && logs[0].startsWith('"EXPECTATION FAILED: goodbye\\n')
  );
});

g.test('expectations,skip_dominates_failure').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite2Cases = await t.load('suite2:*');

  const expectedFailures = [
    {
      query: new TestQueryMultiCase('suite2', ['foof'], ['bluh', 'a'], {}),
      expectation: 'fail' as const,
    },
    {
      query: new TestQueryMultiCase('suite2', ['foof'], ['bluh', 'a'], {}),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite2Cases,
    2,
    new TestQuerySingleCase('suite2', ['foof'], ['bluh', 'a'], {}),
    expectedFailures,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );
});

g.test('expectations,skip_inside_failure').fn(async t => {
  const log = new Logger({ overrideDebugMode: true });
  const suite2Cases = await t.load('suite2:*');

  const expectedFailures = [
    {
      query: new TestQueryMultiFile('suite2', []),
      expectation: 'fail' as const,
    },
    {
      query: new TestQueryMultiCase('suite2', ['foof'], ['blah'], {}),
      expectation: 'skip' as const,
    },
  ];

  await runTestcase(
    t,
    log,
    suite2Cases,
    0,
    new TestQuerySingleCase('suite2', ['foof'], ['blah'], {}),
    expectedFailures,
    'skip',
    logs => logs.length === 1 && logs[0].startsWith('"SKIP: Skipped by expectations"')
  );

  await runTestcase(
    t,
    log,
    suite2Cases,
    2,
    new TestQuerySingleCase('suite2', ['foof'], ['bluh', 'a'], {}),
    expectedFailures,
    'pass',
    logs => logs.length === 1 && logs[0].startsWith('"EXPECTATION FAILED: goodbye\\n')
  );
});

async function testIterateCollapsed(
  t: LoadingTest,
  alwaysExpandThroughLevel: ExpandThroughLevel,
  expectations: string[],
  expectedResult: 'throws' | string[] | [string, number | undefined][],
  includeEmptySubtrees = false
) {
  t.debug(`expandThrough=${alwaysExpandThroughLevel} expectations=${expectations}`);
  const treePromise = t.loader.loadTree(new TestQueryMultiFile('suite1', []), {
    subqueriesToExpand: expectations,
  });
  if (expectedResult === 'throws') {
    t.shouldReject('Error', treePromise, {
      // Some errors here use StacklessError to print nicer command line outputs.
      allowMissingStack: true,
    });
    return;
  }
  const tree = await treePromise;
  const actualIter = tree.iterateCollapsedNodes({
    includeEmptySubtrees,
    alwaysExpandThroughLevel,
  });
  const testingTODOs = expectedResult.length > 0 && expectedResult[0] instanceof Array;
  const actual = Array.from(actualIter, ({ query, subtreeCounts }) =>
    testingTODOs ? [query.toString(), subtreeCounts?.nodesWithTODO] : query.toString()
  );
  if (!objectEquals(actual, expectedResult)) {
    t.fail(
      `iterateCollapsed failed:
  got ${JSON.stringify(actual)}
  exp ${JSON.stringify(expectedResult)}
${tree.toString()}`
    );
  }
}

g.test('print').fn(async t => {
  const tree = await t.loader.loadTree(new TestQueryMultiFile('suite1', []));
  tree.toString();
});

g.test('iterateCollapsed').fn(async t => {
  await testIterateCollapsed(
    t,
    1,
    [],
    [
      ['suite1:foo:*', 1], // to-do propagated up from foo:hola
      ['suite1:bar,buzz,buzz:*', 1], // to-do in file description
      ['suite1:baz:*', 0],
    ]
  );
  await testIterateCollapsed(
    t,
    2,
    [],
    [
      ['suite1:foo:hello:*', 0],
      ['suite1:foo:bonjour:*', 0],
      ['suite1:foo:hola:*', 1], // to-do in test description
      ['suite1:bar,buzz,buzz:zap:*', 0],
      ['suite1:baz:wye:*', 0],
      ['suite1:baz:zed:*', 0],
      ['suite1:baz:batched:*', 0],
    ]
  );
  await testIterateCollapsed(
    t,
    3,
    [],
    [
      ['suite1:foo:hello:', undefined],
      ['suite1:foo:bonjour:', undefined],
      ['suite1:foo:hola:', undefined],
      ['suite1:bar,buzz,buzz:zap:', undefined],
      ['suite1:baz:wye:', undefined],
      ['suite1:baz:wye:x=1', undefined],
      ['suite1:baz:zed:a=1;b=2', undefined],
      ['suite1:baz:zed:b=3;a=1', undefined],
      ['suite1:baz:batched:batch__=0', undefined],
      ['suite1:baz:batched:batch__=1', undefined],
    ]
  );

  // Expectations lists that have no effect
  await testIterateCollapsed(
    t,
    1,
    ['suite1:foo:*'],
    ['suite1:foo:*', 'suite1:bar,buzz,buzz:*', 'suite1:baz:*']
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:bar,buzz,buzz:*'],
    ['suite1:foo:*', 'suite1:bar,buzz,buzz:*', 'suite1:baz:*']
  );
  await testIterateCollapsed(
    t,
    2,
    ['suite1:baz:wye:*'],
    [
      'suite1:foo:hello:*',
      'suite1:foo:bonjour:*',
      'suite1:foo:hola:*',
      'suite1:bar,buzz,buzz:zap:*',
      'suite1:baz:wye:*',
      'suite1:baz:zed:*',
      'suite1:baz:batched:*',
    ]
  );
  // Test with includeEmptySubtrees=true
  await testIterateCollapsed(
    t,
    1,
    [],
    [
      'suite1:foo:*',
      'suite1:bar,biz:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:*',
      'suite1:empty,*',
    ],
    true
  );
  await testIterateCollapsed(
    t,
    2,
    [],
    [
      'suite1:foo:hello:*',
      'suite1:foo:bonjour:*',
      'suite1:foo:hola:*',
      'suite1:bar,biz:*',
      'suite1:bar,buzz,buzz:zap:*',
      'suite1:baz:wye:*',
      'suite1:baz:zed:*',
      'suite1:baz:batched:*',
      'suite1:empty,*',
    ],
    true
  );

  // Expectations lists that have some effect
  await testIterateCollapsed(
    t,
    1,
    ['suite1:baz:wye:*'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye:*',
      'suite1:baz:zed,*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:baz:zed:*'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye,*',
      'suite1:baz:zed:*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:baz:wye:*', 'suite1:baz:zed:*'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye:*',
      'suite1:baz:zed:*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:baz:wye:'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1;*',
      'suite1:baz:zed,*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:baz:wye:x=1'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1',
      'suite1:baz:zed,*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    1,
    ['suite1:foo:*', 'suite1:baz:wye:'],
    [
      'suite1:foo:*',
      'suite1:bar,buzz,buzz:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1;*',
      'suite1:baz:zed,*',
      'suite1:baz:batched,*',
    ]
  );
  await testIterateCollapsed(
    t,
    2,
    ['suite1:baz:wye:'],
    [
      'suite1:foo:hello:*',
      'suite1:foo:bonjour:*',
      'suite1:foo:hola:*',
      'suite1:bar,buzz,buzz:zap:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1;*',
      'suite1:baz:zed:*',
      'suite1:baz:batched:*',
    ]
  );
  await testIterateCollapsed(
    t,
    2,
    ['suite1:baz:wye:x=1'],
    [
      'suite1:foo:hello:*',
      'suite1:foo:bonjour:*',
      'suite1:foo:hola:*',
      'suite1:bar,buzz,buzz:zap:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1',
      'suite1:baz:zed:*',
      'suite1:baz:batched:*',
    ]
  );
  await testIterateCollapsed(
    t,
    2,
    ['suite1:foo:hello:*', 'suite1:baz:wye:'],
    [
      'suite1:foo:hello:*',
      'suite1:foo:bonjour:*',
      'suite1:foo:hola:*',
      'suite1:bar,buzz,buzz:zap:*',
      'suite1:baz:wye:',
      'suite1:baz:wye:x=1;*',
      'suite1:baz:zed:*',
      'suite1:baz:batched:*',
    ]
  );

  // Invalid expectation queries
  await testIterateCollapsed(t, 1, ['*'], 'throws');
  await testIterateCollapsed(t, 1, ['garbage'], 'throws');
  await testIterateCollapsed(t, 1, ['garbage*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:foo*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:foo:he*'], 'throws');

  // Valid expectation queries but they don't match anything
  await testIterateCollapsed(t, 1, ['garbage:*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:doesntexist:*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite2:foo:*'], 'throws');
  // Can't expand subqueries bigger than one file.
  await testIterateCollapsed(t, 1, ['suite1:*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:bar,*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:bar:hello,*'], 'throws');
  await testIterateCollapsed(t, 1, ['suite1:baz,*'], 'throws');
});
