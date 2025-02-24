# ANGLE Test Harness

The ANGLE test harness is a harness around GoogleTest that provides functionality similar to the
[Chromium test harness][BaseTest]. It features:

 * splitting a test set into shards
 * catching and reporting crashes and timeouts
 * outputting to the Chromium [JSON test results format][JSONFormat]
 * multi-process execution

## Command-Line Arguments

The ANGLE test harness accepts all standard GoogleTest arguments. The harness also accepts the
following additional command-line arguments:

 * `--batch-size` limits the number of tests to run in each batch
 * `--batch-timeout` limits the amount of time spent in each batch
 * `--bot-mode` enables multi-process execution and test batching
 * `--debug-test-groups` dumps the test config categories when using `bot-mode`
 * `--filter-file` allows passing a larger `gtest_filter` via a file
 * `--histogram-json-file` outputs a [formatted JSON file][HistogramSet] for perf dashboards
 * `--max-processes` limits the number of simuntaneous processes
 * `--results-directory` specifies a directory to write test results to
 * `--results-file` specifies a location for the JSON test result output
 * `--shard-count` and `--shard-index` control the test sharding
 * `--test-timeout` limits the amount of time spent in each test
 * `--flaky-retries` allows for tests to fail a fixed number of times and still pass
 * `--disable-crash-handler` forces off OS-level crash handling
 * `--isolated-outdir` specifies a test artifacts directory
 * `--max-failures` specifies a count of failures after which the harness early exits.

`--isolated-script-test-output` and `--isolated-script-perf-test-output` mirror `--results-file`
and `--histogram-json-file` respectively.

As well as the custom command-line arguments we support a few standard GoogleTest arguments:

 * `gtest_filter` works as it normally does with GoogleTest
 * `gtest_also_run_disabled_tests` works as it normally does as well

Other GoogleTest arguments are not supported although they may work.

## Implementation Notes

 * The test harness only requires `angle_common` and `angle_util`.
 * It does not depend on any Chromium browser code. This allows us to compile on other non-Clang platforms.
 * It uses rapidjson to read and write JSON files.
 * Test execution is not currently deterministic in multi-process mode.

## Normal Mode vs Bot Mode

The test runner has two main modes of operation: normal and *bot mode*.

During normal mode:

 * Tests are executed single-process and single-thread.
 * The test runner executes the GoogleTest Run function.
 * We use a `TestEventListener` to record test results for our output JSON file.
 * A *watchdog thread* will force a fast exit if no test results get recorded after a timeout.
 * Crashes are handled via ANGLE's test crash handling code.

During bot mode:

 * Tests are run in multiple processes depending on the system processor count.
 * A server process records the child processes' stdout and stderr.
 * The server terminates a child process if there's no progress after a timeout.
 * The server sorts work into batches according to the back-end configuration.
 * This prevents driver errors from using multiple back-ends in the same process.
 * Batches are striped to help split up slow groups of tests.
 * The server passes test batches to child processes via a `gtest_filter` file.
 * Bot mode does not work on Android or Fuchsia.

See the source code for more details: [TestSuite.h](TestSuite.h) and [TestSuite.cpp](TestSuite.cpp).

## Potential Areas of Improvement

 * Deterministic test execution.
 * Using sockets to communicate with test children. Similar to dEQP's test harness.
 * Closer integration with ANGLE's test expectations and system config libraries.
 * Supporting a GoogleTest-free integration.

[BaseTest]: https://chromium.googlesource.com/chromium/src/+/refs/heads/main/base/test/
[JSONFormat]: https://chromium.googlesource.com/chromium/src/+/main/docs/testing/json_test_results_format.md
[HistogramSet]: https://chromium.googlesource.com/catapult/+/HEAD/docs/histogram-set-json-format.md
