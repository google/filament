# ANGLE + dEQP

drawElements (dEQP) is a very robust and comprehensive set of open-source
tests for GLES2, GLES3+ and EGL. They provide a huge net of coverage for
almost every GL API feature. ANGLE by default builds dEQP testing targets for
testing against GLES 2, GLES 3, EGL, and GLES 3.1 (on supported platforms).

## How to build dEQP

You should have dEQP as a target if you followed the [DevSetup](DevSetup.md)
instructions. Some of the current targets are:

  * `angle_deqp_egl_tests` for EGL 1.x tests
  * `angle_deqp_gles2_tests` for GLES 2.0 tests
  * `angle_deqp_gles3_tests` for GLES 3.0 tests
  * `angle_deqp_gles31_tests` for GLES 3.1 tests

## How to use dEQP

Note:
To run an individual test, use the `--gtest_filter` argument.
It supports simple wildcards. For example: `--gtest_filter=dEQP-GLES2.functional.shaders.linkage.*`.

The tests lists are sourced from the Android CTS mains in
`third_party/VK-GL-CTS/src/android/cts/main`. See `gles2-main.txt`,
`gles3-main.txt`, `gles31-main.txt` and `egl-main.txt`.

If you're running a full test suite, it might take very long time. Running in
Debug is only useful to isolate and fix particular failures, Release will give
a better sense of total passing rate.

### Choosing a Renderer

By default ANGLE tests with Vulkan, except on Apple platforms where OpenGL is used.
To specify the exact platform for ANGLE + dEQP, use the arguments:

  * `--deqp-egl-display-type=angle-d3d11` for D3D11 (highest available feature level)
  * `--deqp-egl-display-type=angle-d3d9` for D3D9
  * `--deqp-egl-display-type=angle-d3d11-fl93` for D3D11 Feature level 9_3
  * `--deqp-egl-display-type=angle-gl` for OpenGL Desktop (OSX, Linux and Windows)
  * `--deqp-egl-display-type=angle-gles` for OpenGL ES (Android/ChromeOS, some Windows platforms)
  * `--deqp-egl-display-type=angle-metal` for Metal (Mac)
  * `--deqp-egl-display-type=angle-swiftshader` for Vulkan with SwiftShader as driver (Android, Linux, Mac, Windows)
  * `--deqp-egl-display-type=angle-vulkan` for Vulkan (Android, Linux, Windows)

The flag `--use-angle=X` has the same effect as `--deqp-egl-display-type=angle-X`.

### Check your results

If run from Visual Studio 2015, dEQP generates a test log to
`out/sln/obj/src/tests/TestResults.qpa`. To view the test log information, you'll need to
use the open-source GUI
[Cherry](https://android.googlesource.com/platform/external/cherry). ANGLE
checks out a copy of Cherry to `angle/third_party/cherry` when you sync with
gclient. Note, if you are using ninja or another build system, the qpa file
will be located in your working directory.

See the [official Cherry README](https://android.googlesource.com/platform/external/cherry/+/master/README)
for instructions on how to run Cherry on Linux or Windows.

### GoogleTest, ANGLE and dEQP

ANGLE also supports the same set of targets built with GoogleTest, for running
on the bots. We don't currently recommend using these for local debugging, but
we do maintain lists of test expectations in `src/tests/deqp_support` (see
[Handling Test Failures](TestingAndProcesses.md)). When
you fix tests, please remove the suppression(s) from the relevant files!

### Running dEQP on Android

When you only need to run a few tests with `--gtest_filter` you can use Android wrappers such as `angle_deqp_egl_tests` directly but beware that Android test runner wipes data by default (try `--skip-clear-data`).

Running the tests not using the test runner is tricky, but is necessary in order to get a complete TestResults.qpa from the dEQP tests when running many tests (since the runner shards the tests, only the results of the last shard will be available when using the test runner). First, use the runner to install the APK, test data and test expectations on the device. After the tests start running, the test runner can be stopped with Ctrl+C. Then, run
```
adb shell am start -a android.intent.action.MAIN -n org.chromium.native_test/.NativeUnitTestNativeActivity -e org.chromium.native_test.NativeTest.StdoutFile /sdcard/chromium_tests_root/out.txt
```
After the tests finish, get the results with (requires `adb root`)
```
adb pull /data/data/com.android.angle.test/TestResults.qpa .
```
Note: this location might change, one can double-check with `adb logcat -d | grep qpa`.
