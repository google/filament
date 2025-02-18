# Running the WebGPU CTS Locally with Chrome

Running the WebGPU CTS locally with Chrome requires a Chromium checkout.

Follow [these instructions](https://www.chromium.org/developers/how-tos/get-the-code/) for checking out
and building Chrome. You'll also need to build the `telemetry_gpu_integration_test` target.

At the root of a Chromium checkout, run:
`./content/test/gpu/run_gpu_integration_test.py webgpu_cts --browser=exact --browser-executable=path/to/your/chrome-executable`

If you don't want to build Chrome, you can still run the CTS, by passing the path to an existing Chrome executable to the `--browser-executable` argument. However, if you would like to use all harness functionality (symbolizing stack dumps, etc.). You will still need to build the `telemetry_gpu_integration_test` target.

Useful command-line arguments:
 - `--help`: See more options and argument documentation.
 - `-l`: List all tests that would be run.
 - `--test-filter`: Filter tests.
 - `--passthrough --show-stdout`: Show browser output. See also `--browser-logging-verbosity`.
 - `--extra-browser-args`: Pass extra args to the browser executable.
 - `--jobs=N`: Run with multiple parallel browser instances.
 - `--stable-jobs`: Assign tests to each job in a stable order. Used on the bots for consistency and ease of reproduction.
 - `--enable-dawn-backend-validation`: Enable Dawn's backend validation.
 - `--use-webgpu-adapter=[default,swiftshader,compat]`: Forwarded to the browser to select a particular WebGPU adapter.

## Running the CTS locally on Android

If you want to run the full CTS on Android with expectations locally, some additional setup is required.

First, as explained in the [GPU Testing doc](https://source.chromium.org/chromium/chromium/src/+/main:docs/gpu/gpu_testing.md) you need to build the `telemetry_gpu_integration_test_android_chrome` target. This target _must_ be built in either the `out/Release` or `out/Debug` directories, as those are the only two that can be targeted by the test runner when executing tests on Android.

When executing the tests, use `--browser=android-chromium` (which will look in `out/Release` by default) and limit the tests to one job at a time with `--jobs=1`.

An example of a known-working command line is:

```sh
./content/test/gpu/run_gpu_integration_test.py webgpu_cts --show-stdout --browser=android-chromium --stable-jobs --jobs=1 --extra-browser-args="--enable-logging=stderr --js-flags=--expose-gc --force_high_performance_gpu --use-webgpu-power-preference=default-high-performance"
```

Be aware that running the tests locally on Android is *SLOW*. Expect it to take 4 hrs+.

### Running without root

Typically you want to run the CTS on a device which has root, which generally means flashing a userdebug image onto the device. If this isn't an option, you can try running with the `--compatibility-mode=dont-require-rooted-device` flag as described on [this page](https://chromium.googlesource.com/catapult/+/HEAD/telemetry/docs/run_benchmarks_locally.md), though this is not a supported configuration and you may run into errors.

This mode has been observed to fail if another version of Chrome besides the `chrome_public_apk` target is currently running on the device, so it's suggested to manually close all Chrome variants before starting the test.

### Android Proxy errors

When running the tests on Android devices with the above commands, some devices have been observed to start displaying an `ERR_PROXY_CONNECTION_FAILED` error when attempting to browse with Chrome/Chromium. This is the result of command line proxy settings used by the test runner accidentally not getting cleaned up, likely because the script was terminated early. Should it happen to you the command line used by Chrome can be cleared by running the following command from the root of a Chromium checkout:

```sh
build/android/adb_chrome_public_command_line ""
```

# Running a local CTS build on Swarming
Often, it's useful to test changes on Chrome's infrastructure if it's difficult to reproduce a bug locally. To do that, we can package our local build as an "isolate" and upload it to Swarming to run there. This is often much faster than uploading your CL to Gerrit and triggering tryjobs.

Note that since you're doing a local build, you need to be on the same type of machine as the job you'd like to trigger in swarming. To run a job on a Windows bot, you need to build the isolate on Windows.

1. Build the isolate

   `vpython3 tools/mb/mb.py isolate out/Release telemetry_gpu_integration_test`
2. Upload the isolate

   `./tools/luci-go/isolate archive -cas-instance chromium-swarm -i out/Release/telemetry_gpu_integration_test.isolate`

   This will output a hash like:
   95199eb624d8ddb6ffdfe7a2fc41bc08573aebe3d17363a119cb1e9ca45761ae/734

   Save this hash for use in the next command.
3. Trigger the swarming job.

   The command structure is as follows:

   `./tools/luci-go/swarming trigger -S https://chromium-swarm.appspot.com <dimensions...> -digest <YOUR_ISOLATE_HASH> -- <command> ...`

   Say you want to trigger a job on a Linux Intel bot. It's easiest to check an existing task to see the right args you would use.
   For example: https://chromium-swarm.appspot.com/task?id=5d552b8def31ab11.

   In the table on the left hand side, you can see the bot's **Dimensions**.
   In the **Raw Output** on the right or below the table, you can see the commands run on this bot. Copying those, you would use:
   ```
   ./tools/luci-go/swarming trigger -S https://chromium-swarm.appspot.com -d "pool=chromium.tests.gpu" -d "cpu=x86-64" -d "gpu=8086:9bc5-20.0.8" -d "os=Ubuntu-18.04.6" -digest 95199eb624d8ddb6ffdfe7a2fc41bc08573aebe3d17363a119cb1e9ca45761ae/734 -- vpython3 testing/test_env.py testing/scripts/run_gpu_integration_test_as_googletest.py content/test/gpu/run_gpu_integration_test.py --isolated-script-test-output=${ISOLATED_OUTDIR}/output.json webgpu_cts --browser=release --passthrough -v --show-stdout --extra-browser-args="--enable-logging=stderr --js-flags=--expose-gc --force_high_performance_gpu --enable-features=Vulkan" --total-shards=14 --shard-index=0 --jobs=4 --stable-jobs
   ```

   The command will output a link to the Swarming task for you to see the results.
