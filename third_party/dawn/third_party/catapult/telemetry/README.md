<!-- Copyright 2015 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# Telemetry

Telemetry is the performance testing framework used by Chrome.  It allows you
to perform arbitrary actions on a set of web pages (or any android application!)
and report metrics about it.  The framework abstracts:

*   Launching a browser with arbitrary flags on any platform.
*   Opening a tab and navigating to the page under test.
*   Launching an Android application with intents through ADB.
*   Fetching data via the Inspector timeline and traces.
*   Using [Web Page Replay](../web_page_replay_go/README.md) to
    cache real-world websites so they don’t change when used in benchmarks.

## How to run the unit tests

Run
`catapult/telemetry/bin/run_tests --help`
and see the usage info at the top.

### Running tests on ChromeOS?

See [this page](https://www.chromium.org/chromium-os/developer-library/guides/containers/cros-vm/#run-telemetry-tests).

## Design Principles

*   Write one performance test that runs on major platforms - Windows, Mac,
    Linux, ChromeOS, and Android for both Chrome and ContentShell.
*   Run on browser binaries, without a full Chromium checkout, and without
    having to build the browser yourself.
*   Use Web Page Replay to get repeatable test results.
*   Clean architecture for writing benchmarks that keeps measurements and use
    cases separate.

**Telemetry is designed for measuring performance rather than checking
  correctness. If you want to check for correctness,
  [browser tests](http://www.chromium.org/developers/testing/browser-tests) are
  your friend.**

**If you are a Chromium developer looking to add a new Telemetry benchmark to
[`src/tools/perf/`](https://source.chromium.org/chromium/chromium/src/+/main:tools/perf/),
please make sure to read our
[Benchmark Policy](https://docs.google.com/document/d/1ni2MIeVnlH4bTj4yvEDMVNxgL73PqK_O9_NUm3NW3BA/preview)
first.**

## Code Concepts

Telemetry provides two major functionality groups: those that provide test
automation, and those that provide the capability to collect data.

### Test Automation

The test automation facilities of Telemetry provide Python wrappers for a number
of different system concepts.

*   _Platforms_ use a variety of libraries & tools to abstract away the OS
    specific logic.
*   _Browser_ wraps Chrome's
    [DevTools Remote Debugging Protocol](https://developer.chrome.com/devtools/docs/remote-debugging)
    to perform actions and extract information from the browser.
*   _Android App_ is a Python wrapper around
    [`adb shell`](http://developer.android.com/tools/help/adb.html).

The Telemetry framework lives in
[`src/third_party/catapult/telemetry/`](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/)
and performance benchmarks that use Telemetry live in
[`src/tools/perf/`](https://code.google.com/p/chromium/codesearch#chromium/src/tools/perf/).

### Data Collection

Telemetry offers a framework for collecting metrics that quantify the
performance of automated actions in terms of benchmarks, measurements, and story
sets.

*   A
    [_benchmark_](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/benchmark.py)
    combines a _measurement_ together with a _story set_, and optionally a set
    of browser options.
    *   We strongly discourage benchmark authors from using command-line flags
        to specify the behavior of benchmarks, since benchmarks should be
        cross-platform.
    *   Benchmarks are discovered and run by the
        [benchmark runner](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/benchmark_runner.py),
        which is wrapped by scripts like
        [`run_benchmark`](https://code.google.com/p/chromium/codesearch#chromium/src/tools/perf/run_benchmark)
        in `tools/perf`.
*   A _measurement_ (called
    [`StoryTest`](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/web_perf/story_test.py)
    in the code) is responsible for setting up and tearing down the testing
    platform, and for collecting _metrics_ that quantify the application
    scenario under test.
    *   Measurements need to work with all story sets, to provide consistency
        and prevent benchmark rot.
    *   You probably don't need to override `StoryTest` (see "Timeline Based
        Measurement" below). If you think you do, please talk to us.
*   A
    [_story set_](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/story/story_set.py)
    is a set of _stories_ together with a
    [_shared state_](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/story/shared_state.py)
    that describes application-level configuration options.
*   A
    [_story_](https://cs.chromium.org/chromium/src/third_party/catapult/telemetry/telemetry/story/story.py)
    is an application scenario and a set of actions to run in that scenario. In
    the typical Chromium use case, this will be a web page together with actions
    like scrolling, clicking, or executing JavaScript.
*   There are two major ways to collect data (often referred to as
    _measurements_ or _metrics_) about the stories:
    * _Ad hoc measurements:_  These are measurements that do not require traces,
        for example when a metric is calculated directly in the test page in
        Javascript and we simply want to extract and report this number.
        Currently `PressBenchmark` and associated `PressStory` subclasses are
        examples that use ad hoc measurements. (In reality, PressBenchmark uses
        `DualMetricMeasurement` StoryTest, where you can have both ad hoc and
        timeline based metrics.)
    *  _Timeline Based Measurements:_ These are measurements that require
       recording a timeline of events, for example a [chrome
       trace](https://www.chromium.org/developers/how-tos/trace-event-profiling-tool).
       Telemetry collects traces and other artifacts as it interacts with the
       page and stores them in the form of test results, then [Results Processor](https://source.chromium.org/chromium/chromium/src/+/master:tools/perf/core/results_processor/README.md)
       computes metrics using these test results. New metrics should generally
       be timeline based measurements: Computing metrics on the trace makes it
       possible to compute many different metrics from the same run easily, and
       the collected trace is useful for debugging metrics values.
        *  The current supported programming model is known as [Timeline Based Measurements v2](https://github.com/catapult-project/catapult/blob/master/docs/how-to-write-metrics.md)
           **This is the current recommended method of adding metrics to telemetry**.
        *  [TBMv3](https://source.chromium.org/chromium/chromium/src/+/master:tools/perf/core/tbmv3/),
           a new version of Timeline Based Measurement based on Perfetto is
           currently under development. It is not ready for general use yet but
           there are active experiments on the FYI bots. Ideally this will
           eventually replace TBMv2, but this will only happen once have an easy
           migration path of current TBMv2 metrics to TBMv3.

## Next Steps

*   [Run Telemetry benchmarks locally](/telemetry/docs/run_benchmarks_locally.md)
*   [Record a story set](https://sites.google.com/a/chromium.org/dev/developers/telemetry/record_a_page_set)
    with Web Page Replay
*   [Feature guidelines](https://sites.google.com/a/chromium.org/dev/developers/telemetry/telemetry-feature-guidelines)
*   [Profile generation](https://sites.google.com/a/chromium.org/dev/developers/telemetry/telemetry-profile-generation)
*   [Telemetry unittests](/telemetry/docs/run_telemetry_tests.md)

## Frequently Asked Questions

### I get an error when I try to use recorded story sets.

The recordings are not included in the Chromium source tree. If you are a Google
partner, run `gsutil config` to authenticate, then try running the test again.
If you don't have `gsutil` installed on your machine, you can find it in
`build/third_party/gsutil/gsutil`.

If you are not a Google partner, you can run on live sites with
--use-live-sites` or
[record your own](http://dev.chromium.org/developers/telemetry/record_a_page_set)
story set archive.

### I get mysterious errors about device\_forwarder failing.

Your forwarder binary may be outdated. If you have built the forwarder in
src/out that one will be used. if there isn't anything there Telemetry will
default to downloading a pre-built binary. Try re-building the forwarder, or
alternatively wiping the contents of `src/out/` and running `run_benchmark`,
which should download the latest binary.

### I'm having problems with keychain prompts on Mac.

Make sure that your keychain is
[correctly configured](https://sites.google.com/a/chromium.org/dev/developers/telemetry/telemetry-mac-keychain-setup).
