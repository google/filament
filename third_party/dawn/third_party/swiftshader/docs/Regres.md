# Regres - SwiftShader automated testing

## Introduction

Regres is a collection of tools to perform [dEQP](https://github.com/KhronosGroup/VK-GL-CTS)
presubmit and continuous integration testing and code coverage evaluation for
SwiftShader.

Regres provides:

* [Presubmit testing](#presubmit-testing) - An automatic Vulkan
  dEQP test run for each Gerrit patchset put up for review.
* [Continuous integration testing](#daily-run-continuous-integration-testing) -
  A Vulkan dEQP test run performed against the `master` branch each night. \
  This nightly run also produces code coverage information which can be viewed at
  [swiftshader-regres.github.io/swiftshader-coverage](https://swiftshader-regres.github.io/swiftshader-coverage/).
* [Local dEQP test runner](#local-dEQP-test-runner) Provides a local tool for
  efficiently running a number of dEQP tests based wildcard or regex name
  matching.

The Regres source root directory is at [`<swiftshader>/tests/regres/`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/).

## Presubmit testing

Regres monitors changes that have been [put up for review with Gerrit](https://swiftshader-review.googlesource.com/q/status:open).

Once a new [qualifying](#qualifying) patchset has been found, regres will
checkout, build and test the change against the parent changelist. \
Any differences in results are reported as a review comment on the change
[[example]](https://swiftshader-review.googlesource.com/c/SwiftShader/+/46369/5#message-4f09ea3e6d01ed94ae26183c8b6c547c90492c12).

### Qualifying

As Regres may be running externally authored code on Google hardware,
Regres will only test a change if it is authored by or reviewed by a Googler.

Only the most recent patchset of a change will be tested. If a new patchset is
pushed while the previous is currently being tested, then testing will continue
to completion and the previous patchsets will be posted, and the new patchset
will be queued for testing.

### Prioritization

At the time of writing a Regres presubmit run takes a little over 20 minutes to
complete, and there is a single Regres machine servicing all changes.
To keep Regres responsive, changes are prioritized based on their 'readiness to
land', which is determined by the change's `Kokoro-Presubmit`, `Code-Review` and
`Presubmit-Ready` Gerrit labels.

### Test Filtering

By default, Regres will run all the test lists declared in the
[`<swiftshader>/tests/regres/ci-tests.json`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/ci-tests.json) file.\
As new functionally is being implemented, the test lists in `ci-tests.json` may
reference known-passing test lists updated by the [daily run](#daily-run-continuous-integration-testing),
so that failing tests for incomplete functionality are skipped, but tests that
pass for new functionality *are tested* to ensure they do not regres.

Additional tests names found in the files referenced by
[`<swiftshader>/tests/regres/full-tests.json`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/full-tests.json)
can be explicitly included in the change's presubmit run
by including a line in the change description with the signature:

```text
Test: <dEQP-test-pattern>
```

`<dEQP-test-pattern>` can be a single dEQP test name, or you can use wildcards
[as documented here](https://golang.org/pkg/path/filepath/#Match).

You can repeat `Test:` as many times as you like. `Tests:` is also acccepted.

[For example](https://swiftshader-review.googlesource.com/c/SwiftShader/+/26574):

```text
Add support for OpLogicalEqual, OpLogicalNotEqual

Test: dEQP-VK.glsl.operator.bool_compare.*
Test: dEQP-VK.glsl.operator.binary_operator.equal.*
Test: dEQP-VK.glsl.operator.binary_operator.not_equal.*
Bug: b/126870789
Change-Id: I9d33444d67792274d8027b7d1632235533cfc079
```

## Daily-run continuous integration testing

Once a day, regres will also test another set of tests from [`<swiftshader>/tests/regres/full-tests.json`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/full-tests.json),
and post the test result lists as a Gerrit changelist
[[example]](https://swiftshader-review.googlesource.com/c/SwiftShader/+/46448).

The daily run also performs code coverage instrumentation per dEQP test,
automatically uploading the results of all the dEQP tests to the viewer at
[swiftshader-regres.github.io/swiftshader-coverage](https://swiftshader-regres.github.io/swiftshader-coverage/).

## Local dEQP test runner

Regres also provides a multi-threaded, [process sandboxed](#process-sandboxing),
local dEQP test runner with a wild-card / regex based test name matcher.

The local test runner can be run with:

[`<swiftshader>/tests/regres/run_testlist.sh`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/run_testlist.sh) `--deqp-vk=<path to deqp-vk> [--filter=<test name filter>]`

`<test name filter>` can be a single dEQP test name, or you can use wildcards
[as documented here](https://golang.org/pkg/path/filepath/#Match).
Alternatively, start with a `/` to use a regex filter.

Other useful flags:

```text
  -limit int
        only run a maximum of this number of tests
  -no-results
        disable generation of results.json file
  -output string
        path to an output JSON results file (default "results.json")
  -shuffle
        shuffle tests
  -test-list string
        path to a test list file (default "vk-master-PASS.txt")
```

Run [`<swiftshader>/tests/regres/run_testlist.sh`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/run_testlist.sh) with `--help` to see all available flags.

## Process sandboxing

Regres will run each dEQP test in a separate process to prevent state
leakage between tests.

Tests are run concurrently, and crashing processes will not take down the test
runner.

Some dEQP tests are known to perform excessive memory allocations (i.e. keep
allocating until no more can be claimed from the OS). \
In order to prevent a single test starving other test processes of memory, each
process is restricted to a fraction of the system's memory using [linux resource limits](https://man7.org/linux/man-pages/man2/getrlimit.2.html).

Tests may also deadlock, so each test process has a time limit before they are
automatically killed.

## Implementation details

### Presubmit & daily run process

Regres runs until stopped, and will:

* Download a known compatible version of Clang to a cache directory. This will
  be used for all compilation stages below.
* Periodically poll Gerrit for recently opened changes
* Periodically query Gerrit for details about each tracked change, determining
  [whether it should be tested](#qualifying), and determine its current
  [priority](#prioritization).
* A qualifying change with the highest priority will be picked, and the
  following is performed for the change:
  1. The change is `git fetch`ed into a temporary directory.
  2. If not already cached, the dEQP version described in the
     change's [`<swiftshader>/tests/regres/deqp.json`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/deqp.json) file is downloaded and built the into a cached directory.
  3. The source for the change is built into a temporary build directory.
  4. The built dEQP binaries are used to test the change. The full test results
     are stored in a cached directory.
  5. If the parent change's test results aren't already cached, then steps 3 and
     4 are repeated for the parent change.
  6. The results of the two changes are diffed, and the results of the diff are
     posted to the change as a Gerrit review comment.
* The above is repeated until it is time to perform a daily run, upon which:
  1. The `HEAD` change of `master` is fetched into a temporary directory.
  2. If not already cached, the dEQP version described in the
     change's [`<swiftshader>/tests/regres/deqp.json`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/deqp.json) file is downloaded and built the into a cached directory.
  3. The `HEAD` change is built into a temporary directory, optionally with code
     coverage instrumenting.
  4. The build dEQP binaries are used to test the change.  The full test results
     are stored in a cached directory, and the each test is binned by status and
     written to the [`<swiftshader>/tests/regres/testlists`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/testlists) directory.
  5. A new Gerrit change is created containing the updated test lists and put up
     for review, along with a summary of test result changes [[example]](https://swiftshader-review.googlesource.com/c/SwiftShader/+/46448).
     If there's an existing daily test change up for review then this is reused
     instead of creating another.
  6. If the build included code coverage instrumentation, then the coverage
     results are collated from all test runs, processed and compressed, and
     uploaded to [github.com/swiftshader-regres/swiftshader-coverage](https://github.com/swiftshader-regres/swiftshader-coverage)
     which is immediately reflected at [swiftshader-regres.github.io/swiftshader-coverage](https://swiftshader-regres.github.io/swiftshader-coverage).
     This process is [described in more detail below](#code-coverage).
  7. Stages 3 - 5 are repeated for both the LLVM and Subzero backends.

### Caching

The cache directory is heavily used to avoid duplicated work. For example, it
is common for patchsets to be repeatedly pushed with the same parent change, so
the test results of the parent can be calculated once and stored. A tested
patchset that is merged into master would also be cached when used as a parent
of another change.

The cache needs to consider more than just the change identifier as the
cache-key for storing and retrieving data. Both the test lists and version of
dEQP used are dictated by the change being tested, and so both used as part of
the cache key.

### Vulkan Loader usage

Applications make use of the Vulkan API by loading the [Vulkan Loader](https://github.com/KhronosGroup/Vulkan-Loader)
library (`libvulkan.so.1` on Linux), which enumerates available Vulkan
implementations (typically GPUs and their drivers) before an actual 'instance'
is created to communicate with a specific Installable Client Driver (ICD).

However, SwiftShader can build into libvulkan.so.1 itself, which implements the
same API entry functions as the Vulkan Loader. Regres by default will make dEQP
load this SwiftShader library instead of the system's Vulkan Loader. It ensures
test results are independent of the system's Vulkan setup.

To override this, one can set LD_LIBRARY_PATH to point to the location of a
Loader's libvulkan.so.1.

### Code coverage

The [daily run](#daily-run-continuous-integration-testing) produces code
coverage information that can be examined for each individual dEQP test at
[swiftshader-regres.github.io/swiftshader-coverage](https://swiftshader-regres.github.io/swiftshader-coverage/).

The process for generating this information is complex, and is described in
detail below:

#### Per-test generation

Code coverage instrumentation is generated with
[clang's `--coverage`](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)
functionality. This compiler option is enabled by using SwiftShader's
`SWIFTSHADER_EMIT_COVERAGE` CMake flag.

Each dEQP test process is run with a unique `LLVM_PROFILE_FILE` environment
variable value which dictates where the process writes its raw coverage profile
file. Each process gets a different path so that we can emit coverage from
multiple, concurrent dEQP test processes.

#### Parsing

[Clang provides two tools](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#creating-coverage-reports) for processing coverage data:

* `llvm-profdata` indexes the raw `.profraw` coverage profile file and emits a
  `.profdata` file.
* `llvm-cov` further processes the `.profdata` file into something human
  readable or machine parsable.

`llvm-cov` provides many options, including emitting an pretty HTML file, but is
remarkably slow at producing easily machine-parsable data. Fortunately the core
of `llvm-cov` is [a few hundreds of lines of code](https://github.com/llvm/llvm-project/tree/master/llvm/tools/llvm-cov), as it relies on LLVM libraries to do the heavy lifting. Regres
replaces `llvm-cov` with ["`turbo-cov`"](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/cov/turbo-cov/) which efficiently converts a `.profdata` into a simple binary stream which can
be consumed by Regres.

#### Processing

At the time of writing there are over 560,000 individual dEQP tests, and around
176,000 lines of C++ code in [`<swiftshader>/src`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:src/).
If you used 1 bit for each source line, per-line source coverage for all dEQP
tests would require over 11GiB of storage. That's just for one snapshot.

The processing and compression schemes described below reduces this down to
around 10 MiB (~1100x reduction in size), and supports sub-line coverage scopes.

##### Spans

Code coverage information is described in spans.

A span is a described as an interval of source locations, where a location is a
line-column pair:

```go
type Location struct {
    Line, Column int
}

type Span struct {
    Start, End Location
}
```

##### Test tree construction

Each dEQP test is uniquely identified by a fully qualified name.
Each test belongs to a group, and that group may be nested within any number of
parent groups. The groups are described in the test name, using dots (`.`) to
delimit the groups and leaf test name.

For example, the fully qualified test name:

`dEQP-VK.fragment_shader_interlock.basic.discard.ssbo.sample_unordered.4xaa.sample_shading.16x16`

Can be broken down into the following groups and test name:

```text
dEQP-VK                       <-- root group name
╰ fragment_shader_interlock
  ╰ basic.discard
    ╰ ssbo
      ╰ sample_unordered
        ╰ 4xaa
          ╰ sample_shading
            ╰ 16x16           <-- leaf test name
```

Breaking down fully qualified test names into groups provide a natural way to
structure coverage data, as tests of the same group are likely to have similar
coverage spans.

So, for each source file in the codebase, we create a tree with test groups as
non-leaf nodes, and tests as leaf nodes.

For example, given the following test list:

```text
a.b.d.h
a.b.d.i.n
a.b.d.i.o
a.b.e.j
a.b.e.k.p
a.b.e.k.q
a.c.f
a.c.g.l.r
a.c.g.m
```

We would construct the following tree:

```text
               a
        ╭──────┴──────╮
        b             c
    ╭───┴───╮     ╭───┴───╮
    d       e     f       g
  ╭─┴─╮   ╭─┴─╮         ╭─┴─╮
  h   i   j   k         l   m
     ╭┴╮     ╭┴╮        │
     n o     p q        r

```

Each leaf node in this tree (`h`, `n`, `o`, `j`, `p`, `q`, `f`, `r`, `m`)
represent a test, and non-leaf nodes (`a`, `b`, `c`, `d`, `e`, `g`, `i`, `k`,
`l`) are a groups.

To begin, we create a test tree structure, and associate the full list of test
coverage spans with every leaf node (test) in this tree.

This data structure hasn't given us any compression benefits yet, but we can
now do a few tricks to dramatically reduce number of spans needed to describe
the graph:

##### Optimization 1: Common span promotion

The first compression scheme is to promote common spans up the tree when they
are common for all children. This will reduce the number of spans needed to be
encoded in the final file.

For example, if the test group `a` has 4 children that all share the same span
`X`:

```text
          a
    ╭───┬─┴─┬───╮
    b   c   d   e
 [X,Y] [X] [X] [X,Z]
```

Then span `X` can be promoted up to `a`:

```text
         [X]
          a
    ╭───┬─┴─┬───╮
    b   c   d   e
   [Y] []   [] [Z]
```

##### Optimization 2: Span XOR promotion

This idea can be extended further, by not requiring all the children to share
the same span before promotion. If **most** child nodes share the same span, we
can still promote the span, but this time we **remove** the span from the
children **if they had it**, and **add** the span to children **if they didn't
have it**.

For example, if the test group `a` has 4 children with 3 that share the span
`X`:

```text
          a
    ╭───┬─┴─┬───╮
    b   c   d   e
 [X,Y] [X]  [] [X,Z]
```

Then span `X` can be promoted up to `a` by flipping the presence of `X` on the
child nodes:

```text
         [X]
          a
    ╭───┬─┴─┬───╮
    b   c   d   e
   [Y] []  [X] [Z]
```

This process repeats up the tree.

With this optimization applied, we now need to traverse the tree from root to
leaf in order to know whether a given span is in use for the leaf node (test):

* If the span is encountered an **odd** number of times during traversal, then
  the span is **covered**.
* If the span is encountered an **even** number of times during traversal, then
  the span is **not covered**.

See [`tests/regres/cov/coverage_test.go`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/cov/coverage_test.go) for more examples of this optimization.

##### Optimization 3: Common span grouping

With real world data, we encounter groups of spans that are commonly found
together. To further reduce coverage data, the whole graph is scanned for common
span patterns, and are indexed by each tree node.
The XOR'ing of spans as described above is performed as if the spans were not
grouped.

##### Optimization 4: Lookup tables

All spans, span-groups and strings are stored in de-duplicated tables, and are
indexed wherever possible.

The final serialization is performed by [`tests/regres/cov/serialization.go`](https://cs.opensource.google/swiftshader/SwiftShader/+/master:tests/regres/cov/serialization.go).

##### Optimization 5: zlib compression

The coverage data is encoded into JSON for parsing by the web page.

Before writing the JSON file, the text data is zlib compressed.

#### Presentation

The zlib-compressed JSON coverage data is decompressed using
[`pako`](https://github.com/nodeca/pako), and consumed by some
[vanilla JavaScript](https://github.com/swiftshader-regres/swiftshader-coverage/blob/gh-pages/index.html).

[`codemirror`](https://codemirror.net/) is used to perform coverage span and C++
syntax highlighting
