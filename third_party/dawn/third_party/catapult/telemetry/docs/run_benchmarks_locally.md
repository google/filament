<!-- Copyright 2015 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->

# Telemetry: Run Benchmarks Locally

## Set Up

If you just want Telemetry, cloning
[catapult](https://github.com/catapult-project/catapult) repository should be
enough. If you also want the Chrome benchmarks built with Telemetry, get the
[latest Chromium checkout](https://www.chromium.org/developers/how-tos/get-the-code).
If you're running on Mac OS X, you're all set! For Windows, Linux, Android, or
ChromeOS, read on.

#### Windows

Some benchmarks require you to have
[pywin32](http://sourceforge.net/projects/pywin32/files/pywin32/Build%20219/).
Be sure to install a version that matches the version and bitness of the Python
you have installed.

#### Linux

Telemetry on Linux tries to scan for attached Android devices with
[adb](https://developer.android.com/tools/help/adb.html).
The included adb binary is 32-bit. On 64-bit machines, you need to install the
libstdc++6:i386 package.

#### Android

Running on Android is supported with a Linux host. Windows and Mac OS X are not
yet supported. There are also a few additional steps to set up:

  1. Telemetry requires [adb](http://developer.android.com/tools/help/adb.html).
     If you're running from the zip archive, adb is already included. But if
     you're running with a Chromium checkout, ensure your .gclient file contains
     target\_os = ['android'], then resync your code.
  2. Telemetry only supports devices with ADB Daemon running as root.
     First, you need to install a "userdebug" build of Android on your device.
     Then run `adb root`. Sometimes you may also need to run `adb remount`.
     If you are unable to install "userdebug" build of Android, you can try
     running benchmarks with `--compatibility-mode=dont-require-rooted-device`
     switch, however this configuration may not be supported and you may run
     into errors.
  3. Enable [debugging over USB](http://developer.android.com/tools/device.html)
     on your device.
  4. You can get the name of your device with `adb devices` and use it with
     Telemetry via --device=<device\_name>.

#### ChromeOS

See [Running Telemetry on ChromeOS](https://www.chromium.org/chromium-os/developer-library/guides/containers/cros-vm/#run-telemetry-tests).

## Benchmark Commands

Telemetry benchmarks can be run with run\_benchmark.

In the Chromium source tree, this is located at `src/tools/perf/run_benchmark`.

#### Running a benchmark

List the available benchmarks with `telemetry/run_benchmark list`.

Here's an example for running a particular benchmark:

`src/tools/perf/run_benchmark blink_perf.css --browser=stable`

#### Running on another browser

To list available browsers, use:

`src/tools/perf/run_benchmark --browser=list`

For ease of use, you can use default system browsers on desktop:

`src/tools/perf/run_benchmark blink_perf.css --browser=system`

and on Android:

`src/tools/perf/run_benchmark blink_perf.css --browser=android-system-chrome`

If you're running telemetry from within a Chromium checkout, the release and
debug browsers are what's built in out/Release and out/Debug, respectively.

To run a specific browser executable:

`src/tools/perf/run_benchmark blink_perf.css --browser=exact --browser-executable=/path/to/binary`

To run on a Chromebook:

`src/tools/perf/run_benchmark blink_perf.css --browser=cros-chrome --remote=[ip_address]`

#### Options

To see all options, run:

`src/tools/perf/run_benchmark run --help`

Use --pageset-repeat to run the benchmark repeatedly. For example:

`src/tools/perf/run_benchmark blink_perf.css --pageset-repeat=30`

Use --run-abridged-story-set to run a shortened version of a benchmark with a
representative subset of the stories included (Note that some benchmarks do not
have an abridged version yet. Instructions for abridging a benchmark are
[here](https://docs.google.com/document/d/1GOl4QiOQ0hjwBa0xzvIkgp5wmepqdNpQ6iBybvKENIE/edit#heading=h.x7s3eoyaqdu)). Example:

`src/tools/perf/run_benchmark rendering.desktop --run-abridged-story-set`

If you want to re-generate HTML results and add label, you can do this locally
by using the parameters `--reset-results --results-label="foo"`

`src/tools/perf/run_benchmark blink_perf.css --reset-results --results-label="foo"`

#### Comparing Two Runs

`src/tools/perf/run_benchmark some_benchmark --browser-executable=path/to/version/1
--reset-results --results-label="Version 1"`

`src/tools/perf/run_benchmark some_benchmark --browser-executable=path/to/version/2
--results-label="Version 2"`

The results will be written to in the `results.html` file in the same location
of the `run_benchmark` script.
