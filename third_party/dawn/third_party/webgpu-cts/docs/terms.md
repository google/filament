# Terminology

Each test suite is organized as a tree, both in the filesystem and further within each file.

- _Suites_, e.g. `src/webgpu/`.
  - _READMEs_, e.g. `src/webgpu/README.txt`.
  - _Test Spec Files_, e.g. `src/webgpu/examples.spec.ts`.
    Identified by their file path.
    Each test spec file provides a description and a _Test Group_.
    A _Test Group_ defines a test fixture, and contains multiple:
    - _Tests_.
      Identified by a comma-separated list of parts (e.g. `basic,async`)
      which define a path through a filesystem-like tree (analogy: `basic/async.txt`).
      Defines a _test function_ and contains multiple:
      - _Test Cases_.
        Identified by a list of _Public Parameters_ (e.g. `x` = `1`, `y` = `2`).
        Each Test Case has the same test function but different Public Parameters.

## Test Tree

A _Test Tree_ is a tree whose leaves are individual Test Cases.

A Test Tree can be thought of as follows:

- Suite, which is the root of a tree with "leaves" which are:
  - Test Spec Files, each of which is a tree with "leaves" which are:
    - Tests, each of which is a tree with leaves which are:
      - Test Cases.

(In the implementation, this conceptual tree of trees is decomposed into one big tree
whose leaves are Test Cases.)

**Type:** `TestTree`

## Suite

A suite of tests.
A single suite has a directory structure, and many _test spec files_
(`.spec.ts` files containing tests) and _READMEs_.
Each member of a suite is identified by its path within the suite.

**Example:** `src/webgpu/`

### README

**Example:** `src/webgpu/README.txt`

Describes (in prose) the contents of a subdirectory in a suite.

READMEs are only processed at build time, when generating the _Listing_ for a suite.

**Type:** `TestSuiteListingEntryReadme`

## Queries

A _Query_ is a structured object which specifies a subset of cases in exactly one Suite.
A Query can be represented uniquely as a string.
Queries are used to:

- Identify a subtree of a suite (by identifying the root node of that subtree).
- Identify individual cases.
- Represent the list of tests that a test runner (standalone, wpt, or cmdline) should run.
- Identify subtrees which should not be "collapsed" during WPT `cts.https.html` generation,
  so that that cts.https.html "variants" can have individual test expectations
  (i.e. marked as "expected to fail", "skip", etc.).

There are four types of `TestQuery`:

- `TestQueryMultiFile` represents any subtree of the file hierarchy:
  - `suite:*`
  - `suite:path,to,*`
  - `suite:path,to,file,*`
- `TestQueryMultiTest` represents any subtree of the test hierarchy:
  - `suite:path,to,file:*`
  - `suite:path,to,file:path,to,*`
  - `suite:path,to,file:path,to,test,*`
- `TestQueryMultiCase` represents any subtree of the case hierarchy:
  - `suite:path,to,file:path,to,test:*`
  - `suite:path,to,file:path,to,test:my=0;*`
  - `suite:path,to,file:path,to,test:my=0;params="here";*`
- `TestQuerySingleCase` represents as single case:
  - `suite:path,to,file:path,to,test:my=0;params="here"`

Test Queries are a **weakly ordered set**: any query is
_Unordered_, _Equal_, _StrictSuperset_, or _StrictSubset_ relative to any other.
This property is used to construct the complete tree of test cases.
In the examples above, every example query is a StrictSubset of the previous one
(note: even `:*` is a subset of `,*`).

In the WPT and standalone harnesses, the query is stored in the URL, e.g.
`index.html?q=q:u,e:r,y:*`.

Queries are selectively URL-encoded for readability and compatibility with browsers
(see `encodeURIComponentSelectively`).

**Type:** `TestQuery`

## Listing

A listing of the **test spec files** in a suite.

This can be generated only in Node, which has filesystem access (see `src/tools/crawl.ts`).
As part of the build step, a _listing file_ is generated (see `src/tools/gen.ts`) so that the
Test Spec Files can be discovered by the web runner (since it does not have filesystem access).

**Type:** `TestSuiteListing`

### Listing File

Each Suite has one Listing File (`suite/listing.[tj]s`), containing a list of the files
in the suite.

In `src/suite/listing.ts`, this is computed dynamically.
In `out/suite/listing.js`, the listing has been pre-baked (by `tools/gen_listings_and_webworkers`).

**Type:** Once `import`ed, `ListingFile`

**Example:** `out/webgpu/listing.js`

## Test Spec File

A Test Spec File has a `description` and a Test Group (under which tests and cases are defined).

**Type:** Once `import`ed, `SpecFile`

**Example:** `src/webgpu/**/*.spec.ts`

## Test Group

A subtree of tests. There is one Test Group per Test Spec File.

The Test Fixture used for tests is defined at TestGroup creation.

**Type:** `TestGroup`

## Test

One test. It has a single _test function_.

It may represent multiple _test cases_, each of which runs the same Test Function with different
Parameters.

A test is named using `TestGroup.test()`, which returns a `TestBuilder`.
`TestBuilder.params()`/`.paramsSimple()`/`.paramsSubcasesOnly()`
can optionally be used to parametrically generate instances (cases and subcases) of the test.
Finally, `TestBuilder.fn()` provides the Test Function
(or, a test can be marked unimplemented with `TestBuilder.unimplemented()`).

### Test Function

When a test subcase is run, the Test Function receives an instance of the
Test Fixture provided to the Test Group, producing test results.

**Type:** `TestFn`

## Test Case / Case

A single case of a test. It is identified by a `TestCaseID`: a test name, and its parameters.

Each case appears as an individual item (tree leaf) in `/standalone/`,
and as an individual "step" in WPT.

If `TestBuilder.params()`/`.paramsSimple()`/`.paramsSubcasesOnly()` are not used,
there is exactly one case with one subcase, with parameters `{}`.

**Type:** During test run time, a case is encapsulated as a `RunCase`.

## Test Subcase / Subcase

A single "subcase" of a test. It can also be identified by a `TestCaseID`, though
not all contexts allow subdividing cases into subcases.

All of the subcases of a case will run _inside_ the case, essentially as a for-loop wrapping the
test function. They do _not_ appear individually in `/standalone/` or WPT.

If `CaseParamsBuilder.beginSubcases()` is not used, there is exactly one subcase per case.

## Test Parameters / Params

Each Test Subcase has a (possibly empty) set of Test Parameters,
The parameters are passed to the Test Function `f(t)` via `t.params`.

A set of Public Parameters identifies a Test Case or Test Subcase within a Test.

There are also Private Parameters: any parameter name beginning with an underscore (`_`).
These parameters are not part of the Test Case identification, but are still passed into
the Test Function. They can be used, e.g., to manually specify expected results.

**Type:** `TestParams`

## Test Fixture / Fixture

_Test Fixtures_ provide helpers for tests to use.
A new instance of the fixture is created for every run of every test case.

There is always one fixture class for a whole test group (though this may change).

The fixture is also how a test gets access to the _case recorder_,
which allows it to produce test results.

They are also how tests produce results: `.skip()`, `.fail()`, etc.

**Type:** `Fixture`

### `UnitTest` Fixture

Provides basic fixture utilities most useful in the `unittests` suite.

### `GPUTest` Fixture

Provides utilities useful in WebGPU CTS tests.

# Test Results

## Logger

A logger logs the results of a whole test run.

It saves an empty `LiveTestSpecResult` into its results map, then creates a
_test spec recorder_, which records the results for a group into the `LiveTestSpecResult`.

**Type:** `Logger`

### Test Case Recorder

Refers to a `LiveTestCaseResult` created by the logger.
Records the results of running a test case (its pass-status, run time, and logs) into it.

**Types:** `TestCaseRecorder`, `LiveTestCaseResult`

#### Test Case Status

The `status` of a `LiveTestCaseResult` can be one of:

- `'running'` (only while still running)
- `'pass'`
- `'skip'`
- `'warn'`
- `'fail'`

The "worst" result from running a case is always reported (fail > warn > skip > pass).
Note this means a test can still fail if it's "skipped", if it failed before
`.skip()` was called.

**Type:** `Status`

## Results Format

The results are returned in JSON format.

They are designed to be easily merged in JavaScript:
the `"results"` can be passed into the constructor of `Map` and merged from there.

(TODO: Write a merge tool, if needed.)

```js
{
  "version": "bf472c5698138cdf801006cd400f587e9b1910a5-dirty",
  "results": [
    [
      "unittests:async_mutex:basic:",
      { "status": "pass", "timems": 0.286, "logs": [] }
    ],
    [
      "unittests:async_mutex:serial:",
      { "status": "pass", "timems": 0.415, "logs": [] }
    ]
  ]
}
```
