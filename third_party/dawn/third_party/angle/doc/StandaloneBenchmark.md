# ANGLE Standalone Benchmark

This option builds the trace tests apk to run the selected traces from restricted traces in "src/tests/restricted_traces/". Running traces this way won't require additional trace data setup. Simply clicking on the apk or running the app activity will run the traces. Additionally, it will also dump the fps values of the traces it runs and also the histogram of the diff of the screenshot of the KeyFrame of the trace if golden images are provided.

Setup is similar to ANGLE Restricted Traces and requires some additional GN args.

## Accessing the traces

Follow the `Accessing the traces` in [restricted_traces/README.md](../src/tests/restricted_traces/README.md#accessing-the-traces) and then return here.

## Building the Standalone Benchmark apk

This option is available only for Android. Follow the steps in [DevSetupAndroid.md](DevSetupAndroid.md) (Recommend using the [`Performance`](DevSetupAndroid.md#performance-config) arguments for best performance)

When that is working, add the following GN arg to your setup:
```
build_angle_trace_perf_tests = true
```
### Selecting which traces to build

Since the traces are numerous and trace data is huge in size, you should limit the compilation to a subset to keep the apk file size within limits.
```
angle_restricted_traces = ["pokemon_go 5", "car_chase 1"]
angle_standalone_benchmark_traces = ["pokemon_go", "car_chase"]
angle_standalone_benchmark_goldens_dir = "<Path to golden images of the traces if present>"
```

Names of the png files of golden images should match \<trace name\>_golden.png

To build the apk:
```
autoninja -C out/<config> angle_trace_tests
```

## Installing the apk
```
adb install out/<config>/angle_trace_tests_apk/angle_trace_tests-debug.apk
```

### Set permissions for the app
To allow the results to be available in the external storage, we need to provide appropriate permission.
```
adb shell "appops set com.android.angle.test MANAGE_EXTERNAL_STORAGE allow || true"
```

## Running the apk
You can run the app either by clicking on the "ANGLEBench" app icon or by running the following command (after unlocking the phone).

Note: It is important to unlock the device before running the adb command.
```
adb shell am start -n com.android.angle.test/com.android.angle.test.StandaloneBenchmarkActivity
```

## Run configs
Below configs for the trace runs are set to hardcoded values
```
        gVerboseLogging  = true;
        gScreenshotDir   = "<Application_Dir>/files";
        gSaveScreenshots = true;
        gUseANGLE        = "vulkan";
```

However, other configs can be set using `org.chromium.native_test.NativeTest.CommandLineFlags`. Example:
```
adb shell am start -n com.android.angle.test/com.android.angle.test.StandaloneBenchmarkActivity -e org.chromium.native_test.NativeTest.CommandLineFlags '--trials=1\ --trial-time=60'
```

## Results
Once all the traces listed in `angle_standalone_benchmark_traces` are finished, the app will generate `traces_fps.txt`.

If `angle_standalone_benchmark_goldens_dir` is specified in the GN args, it will also generate `traces_img_comp.txt`.

Output files will be placed on the device in `chromium_tests_root` dir in external storage. For example: `/sdcard/chromium_tests_root/`

### traces_fps.txt
```
<trace 1 name> <FPS value>
<trace 2 name> <FPS value>
...
```

### traces_img_comp.txt
```
<trace 1 name>:
Diff in pixel value:    0-20, 20-40, 40-70, 70-100, 100-150, 150-255
Number of pixels:       <#>, <#>, <#>, <#>, <#>, <#>,

<trace 2 name>:
Diff in pixel value:    0-20, 20-40, 40-70, 70-100, 100-150, 150-255
Number of pixels:       <#>, <#>, <#>, <#>, <#>, <#>,
...
```
