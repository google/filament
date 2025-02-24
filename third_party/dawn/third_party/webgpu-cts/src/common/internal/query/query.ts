import { TestParams } from '../../framework/fixture.js';
import { optionWorkerMode } from '../../runtime/helper/options.js';
import { assert, unreachable } from '../../util/util.js';
import { Expectation } from '../logging/result.js';

import { compareQueries, Ordering } from './compare.js';
import { encodeURIComponentSelectively } from './encode_selectively.js';
import { parseQuery } from './parseQuery.js';
import { kBigSeparator, kPathSeparator, kWildcard } from './separators.js';
import { stringifyPublicParams } from './stringify_params.js';

/**
 * Represents a test query of some level.
 *
 * TestQuery types are immutable.
 */
export type TestQuery =
  | TestQuerySingleCase
  | TestQueryMultiCase
  | TestQueryMultiTest
  | TestQueryMultiFile;

/**
 * - 1 = MultiFile.
 * - 2 = MultiTest.
 * - 3 = MultiCase.
 * - 4 = SingleCase.
 */
export type TestQueryLevel = 1 | 2 | 3 | 4;

export interface TestQueryWithExpectation {
  query: TestQuery;
  expectation: Expectation;
}

/**
 * A multi-file test query, like `s:*` or `s:a,b,*`.
 *
 * Immutable (makes copies of constructor args).
 */
export class TestQueryMultiFile {
  readonly level: TestQueryLevel = 1;
  readonly isMultiFile: boolean = true;
  readonly suite: string;
  readonly filePathParts: readonly string[];

  constructor(suite: string, file: readonly string[]) {
    this.suite = suite;
    this.filePathParts = [...file];
  }

  get depthInLevel() {
    return this.filePathParts.length;
  }

  toString(): string {
    return encodeURIComponentSelectively(this.toStringHelper().join(kBigSeparator));
  }

  protected toStringHelper(): string[] {
    return [this.suite, [...this.filePathParts, kWildcard].join(kPathSeparator)];
  }
}

/**
 * A multi-test test query, like `s:f:*` or `s:f:a,b,*`.
 *
 * Immutable (makes copies of constructor args).
 */
export class TestQueryMultiTest extends TestQueryMultiFile {
  override readonly level: TestQueryLevel = 2;
  override readonly isMultiFile = false as const;
  readonly isMultiTest: boolean = true;
  readonly testPathParts: readonly string[];

  constructor(suite: string, file: readonly string[], test: readonly string[]) {
    super(suite, file);
    assert(file.length > 0, 'multi-test (or finer) query must have file-path');
    this.testPathParts = [...test];
  }

  override get depthInLevel() {
    return this.testPathParts.length;
  }

  protected override toStringHelper(): string[] {
    return [
      this.suite,
      this.filePathParts.join(kPathSeparator),
      [...this.testPathParts, kWildcard].join(kPathSeparator),
    ];
  }
}

/**
 * A multi-case test query, like `s:f:t:*` or `s:f:t:a,b,*`.
 *
 * Immutable (makes copies of constructor args), except for param values
 * (which aren't normally supposed to change; they're marked readonly in TestParams).
 */
export class TestQueryMultiCase extends TestQueryMultiTest {
  override readonly level: TestQueryLevel = 3;
  override readonly isMultiTest = false as const;
  readonly isMultiCase: boolean = true;
  readonly params: TestParams;

  constructor(suite: string, file: readonly string[], test: readonly string[], params: TestParams) {
    super(suite, file, test);
    assert(test.length > 0, 'multi-case (or finer) query must have test-path');
    this.params = { ...params };
  }

  override get depthInLevel() {
    return Object.keys(this.params).length;
  }

  protected override toStringHelper(): string[] {
    return [
      this.suite,
      this.filePathParts.join(kPathSeparator),
      this.testPathParts.join(kPathSeparator),
      stringifyPublicParams(this.params, true),
    ];
  }
}

/**
 * A multi-case test query, like `s:f:t:` or `s:f:t:a=1,b=1`.
 *
 * Immutable (makes copies of constructor args).
 */
export class TestQuerySingleCase extends TestQueryMultiCase {
  override readonly level: TestQueryLevel = 4;
  override readonly isMultiCase = false as const;

  override get depthInLevel() {
    return 0;
  }

  protected override toStringHelper(): string[] {
    return [
      this.suite,
      this.filePathParts.join(kPathSeparator),
      this.testPathParts.join(kPathSeparator),
      stringifyPublicParams(this.params),
    ];
  }
}

/**
 * Parse raw expectations input into TestQueryWithExpectation[], filtering so that only
 * expectations that are relevant for the provided query and wptURL.
 *
 * `rawExpectations` should be @type {{ query: string, expectation: Expectation }[]}
 *
 * The `rawExpectations` are parsed and validated that they are in the correct format.
 * If `wptURL` is passed, the query string should be of the full path format such
 * as `path/to/cts.https.html?worker=0&q=suite:test_path:test_name:foo=1;bar=2;*`.
 * If `wptURL` is `undefined`, the query string should be only the query
 * `suite:test_path:test_name:foo=1;bar=2;*`.
 */
export function parseExpectationsForTestQuery(
  rawExpectations:
    | unknown
    | {
        query: string;
        expectation: Expectation;
      }[],
  query: TestQuery,
  wptURL?: URL
) {
  if (!Array.isArray(rawExpectations)) {
    unreachable('Expectations should be an array');
  }
  const expectations: TestQueryWithExpectation[] = [];
  for (const entry of rawExpectations) {
    assert(typeof entry === 'object');
    const rawExpectation = entry as { query?: string; expectation?: string };
    assert(rawExpectation.query !== undefined, 'Expectation missing query string');
    assert(rawExpectation.expectation !== undefined, 'Expectation missing expectation string');

    let expectationQuery: TestQuery;
    if (wptURL !== undefined) {
      const expectationURL = new URL(`${wptURL.origin}/${entry.query}`);
      if (expectationURL.pathname !== wptURL.pathname) {
        continue;
      }
      assert(
        expectationURL.pathname === wptURL.pathname,
        `Invalid expectation path ${expectationURL.pathname}
Expectation should be of the form path/to/cts.https.html?debug=0&q=suite:test_path:test_name:foo=1;bar=2;...
        `
      );

      const params = expectationURL.searchParams;
      if (optionWorkerMode('worker', params) !== optionWorkerMode('worker', wptURL.searchParams)) {
        continue;
      }

      const qs = params.getAll('q');
      assert(qs.length === 1, 'currently, there must be exactly one ?q= in the expectation string');
      expectationQuery = parseQuery(qs[0]);
    } else {
      expectationQuery = parseQuery(entry.query);
    }

    // Strip params from multicase expectations so that an expectation of foo=2;*
    // is stored if the test query is bar=3;*
    const queryForFilter =
      expectationQuery instanceof TestQueryMultiCase
        ? new TestQueryMultiCase(
            expectationQuery.suite,
            expectationQuery.filePathParts,
            expectationQuery.testPathParts,
            {}
          )
        : expectationQuery;

    if (compareQueries(query, queryForFilter) === Ordering.Unordered) {
      continue;
    }

    switch (entry.expectation) {
      case 'pass':
      case 'skip':
      case 'fail':
        break;
      default:
        unreachable(`Invalid expectation ${entry.expectation}`);
    }

    expectations.push({
      query: expectationQuery,
      expectation: entry.expectation,
    });
  }
  return expectations;
}

/**
 * For display purposes only, produces a "relative" query string from parent to child.
 * Used in the wpt runtime to reduce the verbosity of logs.
 */
export function relativeQueryString(parent: TestQuery, child: TestQuery): string {
  const ordering = compareQueries(parent, child);
  if (ordering === Ordering.Equal) {
    return '';
  } else if (ordering === Ordering.StrictSuperset) {
    const parentString = parent.toString();
    assert(parentString.endsWith(kWildcard));
    const childString = child.toString();
    assert(
      childString.startsWith(parentString.substring(0, parentString.length - 2)),
      'impossible?: childString does not start with parentString[:-2]'
    );
    return childString.substring(parentString.length - 2);
  } else {
    unreachable(
      `relativeQueryString arguments have invalid ordering ${ordering}:\n${parent}\n${child}`
    );
  }
}
